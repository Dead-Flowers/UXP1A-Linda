#include "linda/linda.h"
#include "linda/utilities.h"
#include <csignal>
#include <sys/msg.h>
#include <optional>
#include <memory>
#include <utility>
#include <unistd.h>

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
    TupleSpace::init();
    _clientQueueId = _hostQueueId = 0;
    _clientQueueKey = 0;
}

TupleSpace::~TupleSpace() {
    this->close();
}

void TupleSpace::open(key_t tupleHostKey) {
    if (_clientQueueKey) {
        throw std::exception(); //TODO: exceptions
    }
    _hostQueueId = msgget(tupleHostKey, 0);
    if (_hostQueueId == -1) {
        throw std::exception(); //TODO: exceptions
    }
    _clientQueueKey = createQueueWithRandomKey(0666 | IPC_CREAT | IPC_EXCL,&_clientQueueId);
}

void TupleSpace::close(){
    if (_closed.test_and_set())
        return;
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
    if ((result = msgsnd(_hostQueueId, (void*)&request, sizeof(request), 0)) != 0) {
        throw std::exception(); //TODO: exceptions
    }
    // wait for response
    bool isExpectedResponse;
    do {
        alarm(timeout);
        if ((result = msgrcv(_clientQueueId, (void *) &response, sizeof(response),
                             static_cast<long>(MessageType::Input), 0)) != 0) {
            auto saved_errno = errno;
            alarm(0);
            if (saved_errno == EINTR) return std::nullopt;
            throw std::exception(); //TODO: exceptions
        }
        isExpectedResponse = response.requestId == reqId;
        if (!isExpectedResponse) {
            std::cout << "Received unexpected response: expected "
                      << reqId << ", got " << response.requestId << std::endl;
        }
    } while (response.requestId != reqId);
    alarm(0);

    std::cout << "Received tuple: ";
    std::cout.write(response.tuple, sizeof(response.tuple));
    std::cout << std::endl;

    return std::nullopt;
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
        throw std::exception(); //TODO: exceptions
    }
}


void TupleSpaceHost::init() {
    auto key = ftok(KET_FILE_PATH, PROJECT_ID);

    mainQueueId = msgget(key, 0666 | IPC_CREAT  );
    auto err = std::strerror(errno);
    if(mainQueueId < 0)
        throw "pipe with this key exists"; //TODO: EXPRECTIONS

}

void TupleSpaceHost::runServer() {
    run = true;
    while(run) {
        auto req = TupleRequest{};
        if(auto res = msgrcv(mainQueueId, (void*)&req, sizeof(TupleRequest), 0, 0) == -1 ) {
            auto err = std::strerror(errno);
            throw "problem with receiving a message" ; //TODO: EXPRECTIONS
        }

        std::cout << "message received: " << req << std::endl;

        auto response = this->processRequest(req);

        std::cout << "attemting to send respone" << std::endl;
        if(response.has_value()) {
            std::cout << "sending response " << response.value() << std::endl;

            auto responseQueueId = msgget(req.responseQueueKey, 0 );

            if (auto res = msgsnd(responseQueueId, (void *) &response, sizeof(TupleResponse), 0) == -1) {
                std::cout << "papierz" << std::endl;
                throw "problem with sending a message"; //TODO: EXPRECTIONS
            }

            std::cout << "komandos " << std::endl;
        }

    }
}

void TupleSpaceHost::close() {
    run = false;
    //TODO: ingore res ?
    auto res = msgctl(mainQueueId,IPC_RMID,NULL);
}

TupleSpaceHost::TupleSpaceHost() {
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
        case MessageType::Output: {processOutput(request); return std::nullopt;};
    }

    return response;
}

std::optional<TupleResponse> TupleSpaceHost::processReadOrInput(TupleRequest request, bool pop) {
    auto lexer = linda::modules::Lexer(request.tuple);
    auto parser = linda::modules::PatternParser(lexer);

    auto patterns = parser.parse();

    auto tuple = this->searchSpace(patterns, pop);
    if(!tuple.has_value()) {
        this->insertPendingRequest(request.requestId, request.responseQueueKey, patterns);
        return std::nullopt;
    }

    auto response = TupleResponse();
    response.requestId = request.requestId;
    strcpy(response.tuple, TupleToString(tuple.value()).c_str());

    response.messageType = MessageType::Read;
    //TODO: shoude it be here ?
    if(pop)
        response.messageType = MessageType::Input;

    return response;
}


void TupleSpaceHost::processOutput(TupleRequest request) {
    auto lexer = linda::modules::Lexer(request.tuple);
    auto parser = linda::modules::TupleParser(lexer);

    std::cout << "attempt to parse: "<< request.tuple << std::endl ;

    auto tuple = parser.parse();

    this->insertTuple(tuple);

    this->notifyPendingRequests(tuple);
    this->printSpace();
}

void TupleSpaceHost::insertTuple(Tuple tuple) {
    space.data.push_back(tuple);
    return;
}

void TupleSpaceHost::insertPendingRequest(uint32_t requestId, key_t responseQueueKey, std::vector<TupleItemPattern> itemPatterns) {
    auto req = PendingTupleRequest{};
    req.requestId = requestId;
    req.responseQueueKey = responseQueueKey;
    req.itemPatterns = itemPatterns;
    space.pendingRequests.push_back(req);
    return;
}

std::optional<Tuple> TupleSpaceHost::searchSpace(std::vector<TupleItemPattern> itemPattern, bool pop) {
    std::cout << "dipa" << std::endl;
    for (auto iter = space.data.begin(); iter != space.data.end(); iter++) {
        bool tupleMatched = false;
        auto tuple = *iter;
        for(auto item: tuple){
            std::cout << "dipa2" << std::endl;
            auto pattern = getPattern(itemPattern, static_cast<TupleDataType>(item.index()));
            std::cout << "dipa3" << std::endl;
            if(pattern.has_value())
                tupleMatched = compareValue(item, pattern.value());
            if(!tupleMatched)
                continue;
            std::cout << "dipa4" << std::endl;
        }

        if(tupleMatched) {
            if (pop)
                space.data.erase(iter);
            std::cout << "found tuple" << std::endl;
            return tuple;
        }
    }
    return std::nullopt;
}

bool TupleSpaceHost::compareValue(TupleItem item, TupleItemPattern pattern) {
    switch (pattern.type) {
        case TupleDataType::Integer: {
            return compareTupleToPattern<int64_t>(item, pattern);
        }
        case TupleDataType::Float: {
            if (pattern.op == TupleOperator::Equal)
                throw "Not allowed";
            return compareTupleToPattern<float>(item, pattern);
        }
        case TupleDataType::String: {
            return compareTupleToPattern<std::string>(item, pattern);
        }
        default: throw "bad data type";
        }
    }

void TupleSpaceHost::notifyPendingRequests(Tuple tuple) {
    for(auto iter = space.pendingRequests.begin(); iter != space.pendingRequests.end(); iter++) {
        bool matched_tuple = true;
        for(auto item: tuple) {
            auto pattern = getPattern(iter->itemPatterns, static_cast<TupleDataType>(item.index()));
            if(pattern.has_value()) {
                matched_tuple = compareValue(item, pattern.value());
            }
        }
        auto response = TupleResponse{};
        if(iter->pop)
            response.messageType = MessageType::Input;
        else
            response.messageType = MessageType::Read;
        response.requestId = iter->requestId;
        strcpy(response.tuple, TupleToString(tuple).c_str());
        if(auto res = msgsnd(iter->responseQueueKey, (void*)&response, sizeof(TupleResponse), 0) == -1) {
            throw "problem with sending a message" ; //TODO: EXPRECTIONS
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


