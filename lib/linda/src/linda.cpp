#include "linda/linda.h"
#include "linda/utilities.h"
#include <csignal>
#include <sys/msg.h>
#include <optional>
#include <memory>
#include <utility>
#include <unistd.h>
#include "spdlog/spdlog.h"
#include "linda/exceptions.h"
#include "spdlog/sinks/stdout_color_sinks.h"

void sigHandler(int signum) {
    closeProgram(signum);
};

std::optional<TupleItemPattern> getPattern(std::vector<TupleItemPattern> itemPatterns, TupleDataType patternType) {
    for (auto pattern: itemPatterns) {
        if(pattern.type == patternType)
            return pattern;
    }

    return std::nullopt;
}


TupleSpace::TupleSpace() {
    _logger = spdlog::stdout_color_mt("TupleSpace");
    TupleSpace::init();
    _clientQueueId = _hostQueueId = 0;
    _clientQueueKey = 0;
}

TupleSpace::~TupleSpace() {
    this->close();
}

void TupleSpace::open(key_t tupleHostKey) {
    if (_clientQueueKey) {
        throw LindaException("null client queue key");
    }
    _hostQueueId = msgget(tupleHostKey, 0);
    if (_hostQueueId == -1) {
        throw LindaSyscallException(errno);
    }
    _clientQueueKey = createQueueWithRandomKey(0666 | IPC_CREAT | IPC_EXCL,&_clientQueueId);
}

void TupleSpace::close(){
    if (_closed.test_and_set())
        return;
    _logger->info("Closing");
    if (_clientQueueId) {
        auto ret = msgctl(_clientQueueId, IPC_RMID, nullptr);
        // TODO:: should we ignore ret?
        _clientQueueId = 0;
        _hostQueueId = 0;
    }
}

std::optional<Tuple> TupleSpace::input(std::string tupleTemplate, int timeout) {
    return requestTuple(std::move(tupleTemplate), timeout, true);
}

std::optional<Tuple> TupleSpace::read(std::string tupleTemplate, int timeout) {
    return requestTuple(std::move(tupleTemplate), timeout, false);
}

std::optional<Tuple> TupleSpace::requestTuple(std::string tupleTemplate, int timeout, bool pop) {
    uint32_t reqId = std::rand();

    TupleResponse response = {};
    TupleRequest request = {
            pop ? MessageType::Input : MessageType::Read,
            _clientQueueKey,
            reqId,
    };
    writeStringToCharArray(std::move(tupleTemplate), request.tuple, sizeof(request.tuple));

    int result;
    if ((result = msgsnd(_hostQueueId, (void*)&request, sizeof(request), 0)) < 0) {
        throw LindaSyscallException(errno);
    }
    // wait for response
    bool isExpectedResponse;
    do {
        alarm(timeout);
        if ((result = msgrcv(_clientQueueId, (void *) &response, sizeof(response),
                             static_cast<long>(MessageType::Input), 0)) < 0) {
            auto saved_errno = errno;
            alarm(0);
            if (saved_errno == EINTR)
                return std::nullopt;

            throw LindaSyscallException(saved_errno);
        }
        isExpectedResponse = response.requestId == reqId;
        if (!isExpectedResponse) {
            std::cout << "Received unexpected response: expected "
                      << reqId << ", got " << response.requestId << std::endl;
        }
    } while (response.requestId != reqId);
    alarm(0);

    std::cout << "Received tuple: ";
    std::cout.write(response.tuple, (ssize_t) strlen(response.tuple));
    std::cout << std::endl;

    auto lexer = linda::modules::Lexer(response.tuple);
    auto parser = linda::modules::TupleParser(lexer);

    return parser.parse();
}

void TupleSpace::output(std::string tuple) {
    uint32_t reqId = std::rand();
    TupleRequest request = {
            MessageType::Output,
            _clientQueueKey,
            reqId,
    };
    writeStringToCharArray(std::move(tuple), request.tuple, sizeof(request.tuple));

    int result;
    if ((result = msgsnd(_hostQueueId, (void*)&request, sizeof(request), 0)) != 0) {
        throw LindaSyscallException(errno);
    }
}


void TupleSpaceHost::init(const char* inputKeyFile, int inputProjectID) {
    auto key_file_path = DEFAULT_KEY_FILE_PATH;
    if(inputKeyFile != "") {
        key_file_path = inputKeyFile;
    }

    auto proj_id = DEFAULT_PROJECT_ID;
    if (inputProjectID != 0) {
        proj_id = inputProjectID;
    }

    auto key = ftok(key_file_path, proj_id);

    mainQueueId = msgget(key, 0666 | IPC_CREAT  );
    auto err = std::strerror(errno);
    if(mainQueueId < 0)
        throw LindaSyscallException(errno);

}

void TupleSpaceHost::runServer() {
    run = true;
    while(run) {
        auto req = TupleRequest{};
        if(auto res = msgrcv(mainQueueId, (void*)&req, sizeof(TupleRequest), 0, 0) == -1 ) {
            throw LindaSyscallException(errno);
        }

        spdlog::info("Received request #{0} of type {1}", req.requestId, MsgTypeToString(req.messageType));
        spdlog::debug("Content of request #{0}: {1}", req.requestId, req.tuple);

        auto response = this->processRequest(req);

        if (response.has_value()) {
            spdlog::info("Immediately fulfilled request #{0}", req.requestId);

            auto responseQueueId = msgget(req.responseQueueKey, 0);

            if (responseQueueId == -1 || msgsnd(responseQueueId, (void *) &response, sizeof(TupleResponse), 0) < 0) {
                spdlog::warn("Error while sending response to #{0}: {1}", req.requestId, std::strerror(errno));
            }

        }

    }
}

void TupleSpaceHost::close() {
    run = false;
    //TODO: ignore res ?
    auto res = msgctl(mainQueueId,IPC_RMID,NULL);
}

TupleSpaceHost::TupleSpaceHost() {
    _logger = spdlog::stdout_color_mt("TupleSpaceHost");
    _logger->set_level(spdlog::level::debug);
    closeProgram = [this](int signum){this->close();};
    auto res = std::signal(SIGINT, sigHandler);
}

TupleSpaceHost::~TupleSpaceHost() {
    this->close();
}

std::optional<TupleResponse> TupleSpaceHost::processRequest(TupleRequest request) {
    auto response = TupleResponse{};

    switch (request.messageType) {
        case MessageType::Read: return processReadOrInput(request, false);
        case MessageType::Input: return processReadOrInput(request, true);
        case MessageType::Output: {processOutput(request); return std::nullopt;}
    }

    return response;
}

std::optional<TupleResponse> TupleSpaceHost::processReadOrInput(TupleRequest request, bool pop) {
    auto patterns = this->parsePattern(request.tuple);

    auto tuple = this->searchSpace(patterns, pop);
    if (!tuple.has_value()) {
        this->insertPendingRequest(request.requestId, request.responseQueueKey, patterns);
        return std::nullopt;
    }

    auto response = TupleResponse();
    response.requestId = request.requestId;
    response.messageType = pop ? MessageType::Input : MessageType::Read;

    writeStringToCharArray(TupleToString(tuple.value()), response.tuple, sizeof(response.tuple));
    return response;
}

void TupleSpaceHost::processOutput(TupleRequest request) {
    auto tuple = this->parseTuple(request.tuple);

    this->insertTuple(tuple);

    this->notifyPendingRequests(tuple);
    this->printSpace();
}

Tuple TupleSpaceHost::parseTuple(const char* tuple) {
    auto lexer = linda::modules::Lexer(tuple);
    auto parser = linda::modules::TupleParser(lexer);

    return parser.parse();
}

std::vector<TupleItemPattern> TupleSpaceHost::parsePattern(const char* pattern) {
    auto lexer = linda::modules::Lexer(pattern);
    auto parser = linda::modules::PatternParser(lexer);

    return parser.parse();
}

void TupleSpaceHost::insertTuple(Tuple tuple) {
    space.data.push_back(tuple);
}

void TupleSpaceHost::insertPendingRequest(uint32_t requestId, key_t responseQueueKey,
                                          const TuplePattern& tuplePattern) {
    auto req = PendingTupleRequest{};
    req.requestId = requestId;
    req.responseQueueKey = responseQueueKey;
    req.itemPatterns = tuplePattern;
    space.pendingRequests.push_back(req);
}

bool TupleSpaceHost::patternMatchesTuple(const TuplePattern& pattern, const Tuple& tuple) {
    for(auto item: tuple) {
        auto pat = getPattern(pattern, static_cast<TupleDataType>(item.index()));
        if (!pat.has_value()) {
            return false;
        }
        if(!compareValue(item, pat.value())) {
            return false;
        }
    }

    return true;
}

std::optional<Tuple> TupleSpaceHost::searchSpace(const TuplePattern& tuplePattern, bool pop) {
    _logger->debug("Searching tuple space");
    auto& data = space.data;
    for (auto iter = data.begin(); iter != data.end(); iter++) {
        auto tuple = *iter;

        if(this->patternMatchesTuple(tuplePattern, tuple)) {
            if (pop) {
                *iter = data.back();
                data.pop_back();
            }
            _logger->debug("Found a matching tuple (pop={0})", pop);
            return tuple;
        }
    }
    _logger->debug("Found no matches");
    return std::nullopt;
}

bool TupleSpaceHost::compareValue(TupleItem item, TupleItemPattern pattern) {
    switch (pattern.type) {
        case TupleDataType::Integer: {
            return compareTupleToPattern<int64_t>(item, pattern);
        }
        case TupleDataType::Float: {
            if (pattern.value.has_value() && pattern.op == TupleOperator::Equal)
                throw LindaException("Not allowed operator equal on float");
            return compareTupleToPattern<float>(item, pattern);
        }
        case TupleDataType::String: {
            return compareTupleToPattern<std::string>(item, pattern);
        }
        default: throw LindaException("Bad data type");
        }
    }

void TupleSpaceHost::notifyPendingRequests(Tuple tuple) {
    _logger->debug("Notifying pending requests");
    for(auto iter = space.pendingRequests.begin(); iter != space.pendingRequests.end(); iter++) {
        if(this->patternMatchesTuple(iter->itemPatterns, tuple)) {
            auto response = TupleResponse{};
            response.messageType = MessageType::Output;
            response.requestId = iter->requestId;
            writeStringToCharArray(TupleToString(tuple), response.tuple, sizeof(response.tuple));

            if (msgsnd(iter->responseQueueKey, (void *) &response, sizeof(TupleResponse), 0) == 0) {
                // success, stop matching if popped
                if (iter->pop){
                    *iter = space.pendingRequests.back();
                    space.pendingRequests.pop_back();
                    return;
                }
            } else {
                throw LindaSyscallException(errno);
            }
        }
    }

}

void TupleSpaceHost::printSpace() {
    std::cout << "SPACE: " << std::endl;
    for(auto t: space.data) {
        std::cout << "Tuple: ";
        for (auto i: t) {
            switch (i.index()) {
                case 0: { std::cout << std::get<int64_t>(i); break;}
                case 1: { std::cout << std::get<float>(i); break;}
                case 2: { std::cout << std::get<std::string>(i); break;}
            }
            std::cout << " , ";
        }
        std::cout << std::endl;
    }
}

bool TupleSpaceHost::contains(const TuplePattern& tuplePattern) {
    auto val = searchSpace(tuplePattern, false);
    return val.has_value();
}

int TupleSpaceHost::spaceSize() {
    return space.data.size();
}

void TupleSpaceHost::reset() {
    space.data.clear();
}



