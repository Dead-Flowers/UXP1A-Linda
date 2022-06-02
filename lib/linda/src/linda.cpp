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
#include "parser/exceptions.h"
#include "linda/consts.h"
#include "linda/signals.h"


void hostSigHandler(int signum) {
    killRunServer(signum);
};

void clientSigHandler(int signum) {
    spdlog::trace("Client received signal: {0}", signum);
}

TupleSpace::TupleSpace() {
    _logger = getLogger("TupleSpace");
    _logger->set_level(spdlog::level::trace);

    TupleSpace::init();
    _clientQueueId = _hostQueueId = 0;
    _clientQueueKey = 0;
    _lastRequestId = std::rand(); // for easier debugging
}

TupleSpace::~TupleSpace() {
    this->close();
}

void TupleSpace::open(const char* keyPath, int projectId, int clientChmod) {
    if (_clientQueueKey) {
        throw LindaException("null client queue key");
    }
    _hostQueueId = msgget(ftok(keyPath, projectId), 0);
    if (_hostQueueId < 0) {
        _logger->error("Error while opening host queue");
        throw LindaSyscallException(errno);
    }
    _clientQueueKey = createQueueWithRandomKey(clientChmod | IPC_CREAT | IPC_EXCL,&_clientQueueId);
    if(_clientQueueId < 0){
        _logger->error("Error while creating client queue");
        throw LindaSyscallException(errno);
    }
    _logger->info("Successfully created client queue with key {0}", _clientQueueKey);

}

void TupleSpace::close(){
    if (_closed.test_and_set())
        return;
    _logger->info("Closing");
    if (_clientQueueId) {
        if (msgctl(_clientQueueId, IPC_RMID, nullptr) < 0) {
            _logger->warn("Error while closing client queue ({0})", std::strerror(errno));
        }
        _clientQueueId = 0;
        _hostQueueId = 0;
    }
}

std::optional<Tuple> TupleSpace::input(const std::string& tupleTemplate, int timeout) {
    return requestTuple(std::move(tupleTemplate), timeout, true);
}

std::optional<Tuple> TupleSpace::read(const std::string& tupleTemplate, int timeout) {
    return requestTuple(std::move(tupleTemplate), timeout, false);
}

std::optional<Tuple> TupleSpace::requestTuple(const std::string& tupleTemplate, int timeout, bool pop) {
    if (tupleTemplate.size() > MAX_TUPLE_LENGTH) {
        throw LindaException(TOO_LONG_TUPLE_ERROR);
    }
    uint32_t reqId = _lastRequestId++;
    TupleRequest request = {
            pop ? MessageType::Input : MessageType::Read,
            _clientQueueKey,
            reqId,
    };
    writeStringToCharArray(std::move(tupleTemplate), request.tuple, sizeof(request.tuple));
    _logger->debug("Request #{0}, matching pattern: {1} (pop={2})", request.requestId, request.tuple, pop);


    if (msgsnd(_hostQueueId, (void*)&request, sizeof(request), 0) < 0) {
        throw LindaSyscallException(errno);
    }

    auto optionalResponse = waitForResponse(reqId, timeout);
    if (optionalResponse.has_value()){
        auto lexer = linda::modules::Lexer(optionalResponse.value().tuple);
        auto parser = linda::modules::TupleParser(lexer);
        return parser.parse();
    }

    return std::nullopt;
}

void TupleSpace::output(const std::string& tuple, int timeout) {
    if (tuple.size() > MAX_TUPLE_LENGTH) {
        throw LindaException(TOO_LONG_TUPLE_ERROR);
    }

    uint32_t reqId = ++_lastRequestId;
    TupleRequest request = {
            MessageType::Output,
            _clientQueueKey,
            reqId,
    };
    writeStringToCharArray(std::move(tuple), request.tuple, sizeof(request.tuple));
    _logger->debug("Request #{0}, outputting tuple: {1}", request.requestId, request.tuple);
    int result;
    if ((result = msgsnd(_hostQueueId, (void*)&request, sizeof(request), 0)) < 0) {
        throw LindaSyscallException(errno);
    }

    // catch any error or get acknowledgement
    waitForResponse(reqId, timeout);
}

std::optional<TupleResponse> TupleSpace::waitForResponse(uint32_t requestId, int timeout) {
    TupleResponse response{};
    int result;
    bool isExpectedResponse;

    do {
        alarm(timeout);
        if ((result = msgrcv(_clientQueueId, (void *) &response, sizeof(response),0, 0)) < 0) {
            auto saved_errno = errno;
            alarm(0);
            if (saved_errno == EINTR)
                return std::nullopt;

            throw LindaSyscallException(saved_errno);
        }
        isExpectedResponse = response.requestId == requestId;
        if (!isExpectedResponse) {
            _logger->warn("Received unexpected response: expected #{0}, got #{1}", requestId, response.requestId);
        }
    } while (response.requestId != requestId);
    alarm(0);

    if (response.messageType == MessageType::Error) {
        _logger->error("Received error response for #{0}: {1}", response.requestId, response.tuple);
        throw LindaException(response.tuple);
    }
    _logger->debug("Received valid response for #{0}: {1}", response.requestId, response.tuple);
    return std::make_optional(response);
}


void TupleSpaceHost::init(const char* inputKeyFile, int inputProjectId, bool rm, int chmod) {
    auto key_file_path = DEFAULT_KEY_FILE_PATH;
    if (inputKeyFile) {
        key_file_path = inputKeyFile;
    }

    auto proj_id = inputProjectId == 0 ? DEFAULT_PROJECT_ID : inputProjectId;
    _logger->info("Initializing tuple space at {0} with project id {1}", inputKeyFile, inputProjectId);

    auto key = ftok(key_file_path, proj_id);
    if (rm && (_mainQueueId = msgget(key, 0)) >= 0) {
        _logger->info("Removing existing host queue");
        if (msgctl(_mainQueueId, IPC_RMID, nullptr) < 0) {
            _logger->warn("Unable to close host queue");
        }
    }

    if((_mainQueueId = msgget(key, chmod | IPC_CREAT | IPC_EXCL)) < 0) {
        auto err = LindaSyscallException(errno);
        _logger->error("Unable to create host queue: {0}", err.what());
        throw err;
    }
    _logger->info("Tuple space host is ready");
}

bool TupleSpaceHost::trySendResponse(const key_t responseQueueKey, const TupleResponse& tupleResponse) {
    int msgQueueId;
    if ((msgQueueId = msgget(responseQueueKey, 0)) < 0) {
        return false;
    }
    auto result = msgsnd(msgQueueId, (void *) &tupleResponse, sizeof(TupleResponse), 0);
    return result == 0;
}

void TupleSpaceHost::runServer() {
    killRunServer = [this](int signum){this->close();};
    SignalWrapper sigIntWrapper(SIGINT, hostSigHandler);
    SignalWrapper sigTermWrapper(SIGTERM, hostSigHandler);

    _run = true;
    while(_run) {
        auto req = TupleRequest{};
        if(msgrcv(_mainQueueId, (void*)&req, sizeof(TupleRequest), 0, 0) < 0) {
            if (!_run) return;
            if (errno == EINTR) continue;
            throw LindaSyscallException(errno);
        }

        _logger->info("Received request #{0} from Q{1} of type {2}", req.requestId,
                      req.responseQueueKey, messageTypeToString(req.messageType));
        _logger->debug("Content of request #{0}: {1}", req.requestId, req.tuple);

        auto response = this->processRequest(req);

        if (response.has_value()) {
            response->requestId = req.requestId;
            _logger->info("Immediately fulfilled request #{0}", req.requestId);

            if (!trySendResponse(req.responseQueueKey, response.value())) {
                _logger->warn("Error while sending response to #{0} Q{1}: {2}", req.requestId,
                              req.responseQueueKey, std::strerror(errno));
                if (req.messageType == MessageType::Input) {
                    _logger->warn("Re-adding tuple from unhandled response #{0}", req.requestId);
                    this->_space.data.push_back(parseTuple(response->tuple).value());
                }
            }
        }
    }
}

void TupleSpaceHost::close() {
    if (_closed.test_and_set())
        return;
    _logger->info("Closing");
    _run = false;
    if (msgctl(_mainQueueId, IPC_RMID, nullptr) < 0) {
        _logger->warn("Unable to close host queue");
    }
}

TupleSpaceHost::TupleSpaceHost() {
    _logger = getLogger("TupleSpaceHost");
    _logger->set_level(spdlog::level::trace);
}

TupleSpaceHost::~TupleSpaceHost() {
    this->close();
}

std::optional<TupleResponse> TupleSpaceHost::processRequest(TupleRequest request) {
    auto response = TupleResponse{};

    switch (request.messageType) {
        case MessageType::Read: return processReadOrInput(request, false);
        case MessageType::Input: return processReadOrInput(request, true);
        case MessageType::Output: return processOutput(request);
        case MessageType::Error:
        {
            response.messageType = MessageType::Error;
            writeStringToCharArray("Bad request message type", response.tuple, sizeof(response.tuple));
            break;
        }
    }

    return response;
}

std::optional<TupleResponse> TupleSpaceHost::processReadOrInput(TupleRequest request, bool pop) {
    auto response = TupleResponse();
    response.requestId = request.requestId;

    auto check = this->checkPattern(request);
    if(check.has_value()) {
        response = check.value();
        _logger->warn("Bad tuple, requestId: #{0}, reason: {1}", request.requestId, response.tuple);
        return response;
    }

    auto patterns = this->parsePattern(request.tuple);

    if (patterns.size() > MAX_TUPLE_ITEMS) {
        response.messageType = MessageType::Error;
        writeStringToCharArray(TOO_LONG_TUPLE_ERROR, response.tuple, sizeof(response.tuple));
        return response;
    }

    auto tuple = this->searchSpace(patterns, pop);
    if (!tuple.has_value()) {
        this->insertPendingRequest(request.requestId, request.responseQueueKey, patterns, pop);
        return std::nullopt;
    }

    response.messageType = pop ? MessageType::Input : MessageType::Read;

    writeStringToCharArray(tupleToString(tuple.value()), response.tuple, sizeof(response.tuple));
    return response;
}

std::optional<TupleResponse> TupleSpaceHost::processOutput(TupleRequest request) {
    auto response = TupleResponse();
    response.requestId = request.requestId;
    response.messageType = MessageType::Output;

    auto check = this->checkTuple(request);
    if (check.has_value()) {
        response = check.value();
        _logger->warn("Bad tuple, requestId: #{0}, reason: {1}", request.requestId, response.tuple);
        return response;
    }

    // can do this safely, check tuple would have return an error
    auto tupleOptional = this->parseTuple(request.tuple);
    if (!tupleOptional.has_value()) {
        response.messageType = MessageType::Error;
        writeStringToCharArray("Unexpected parsing error", response.tuple, sizeof(response.tuple));
    }

    this->insertTuple(tupleOptional.value());
    return response;
}

std::optional<Tuple> TupleSpaceHost::parseTuple(const char* tuple) {
    auto lexer = linda::modules::Lexer(tuple);
    auto parser = linda::modules::TupleParser(lexer);

    return parser.parse();
}

std::vector<TupleItemPattern> TupleSpaceHost::parsePattern(const char* pattern) {
    auto lexer = linda::modules::Lexer(pattern);
    auto parser = linda::modules::PatternParser(lexer);

    return parser.parse();
}

void TupleSpaceHost::insertTuple(const Tuple& tuple) {
    if(!this->tryMatchPendingRequests(tuple)) {
        _space.data.push_back(tuple);
    }
}

void TupleSpaceHost::insertPendingRequest(uint32_t requestId, key_t responseQueueKey,
                                          const TuplePattern& tuplePattern, bool pop) {
    auto req = PendingTupleRequest{};
    req.requestId = requestId;
    req.responseQueueKey = responseQueueKey;
    req.itemPatterns = tuplePattern;
    req.pop = pop;
    _space.pendingRequests.push_back(req);
}

bool TupleSpaceHost::patternMatchesTuple(const TuplePattern& pattern, const Tuple& tuple) {
    if (tuple.size() != pattern.size())
        return false;

    for(int i = 0; i < tuple.size(); i++) {
        auto& tupleItem = tuple[i];
        auto& patternItem = pattern[i];
        if (!compareValue(tupleItem, patternItem))
            return false;
    }

    return true;
}

std::optional<Tuple> TupleSpaceHost::searchSpace(const TuplePattern& tuplePattern, bool pop) {
    _logger->debug("Searching tuple space");
    auto& data = _space.data;
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

bool TupleSpaceHost::tryMatchPendingRequests(const Tuple& tuple) {
    _logger->debug("Notifying pending requests");
    auto iter = _space.pendingRequests.begin();
    while(iter != _space.pendingRequests.end()) {
        if (!this->patternMatchesTuple(iter->itemPatterns, tuple)) {
            iter++;
            continue;
        }
        auto response = TupleResponse{};
        response.messageType = MessageType::Output;
        response.requestId = iter->requestId;
        writeStringToCharArray(tupleToString(tuple), response.tuple, sizeof(response.tuple));

        if (!trySendResponse(iter->responseQueueKey, response)) {
            _logger->warn("Error while notifying/sending response to #{0}: {1}", iter->requestId,
                         std::strerror(errno));
        } else {
            // success, stop matching if popped
            _logger->debug("Notifying request id {0}", iter->requestId);
            *iter = _space.pendingRequests.back();
            _space.pendingRequests.pop_back();
            if (iter->pop) {
                _logger->debug("stopping further iteration");
                return true;
            }
            continue; // don't advance iter
        }
        iter++;

    }
    return false;
}

bool TupleSpaceHost::contains(const TuplePattern& tuplePattern) {
    auto val = searchSpace(tuplePattern, false);
    return val.has_value();
}

int TupleSpaceHost::spaceSize() {
    return _space.data.size();
}

void TupleSpaceHost::reset() {
    _space.data.clear();
    _space.pendingRequests.clear();
}

std::optional<TupleResponse> TupleSpaceHost::checkPattern(TupleRequest request) {
    std::vector<TupleItemPattern> patterns;
    auto response = TupleResponse();
    response.messageType = MessageType::Error;

    try {
        patterns = this->parsePattern(request.tuple);
    } catch (std::exception& e) {
        std::string errMsg = e.what();
        writeStringToCharArray(errMsg, response.tuple, sizeof(response.tuple));
        return response;
    }
    if (patterns.size() > MAX_TUPLE_ITEMS) {
        writeStringToCharArray(TOO_LONG_TUPLE_ERROR, response.tuple, sizeof(response.tuple));
        return response;
    }

    for(auto& pattern: patterns) {
        if (pattern.type == TupleDataType::Float && pattern.value.has_value() && pattern.op == TupleOperator::Equal) {
            writeStringToCharArray("Operator == is not allowed for float type", response.tuple, sizeof(response.tuple));
            return response;
        }
    }
    return std::nullopt;
}

std::optional<TupleResponse> TupleSpaceHost::checkTuple(TupleRequest request) {
    std::optional<Tuple> tuple;
    auto response = TupleResponse();
    response.messageType = MessageType::Error;
    response.requestId = request.requestId;

    try {
        tuple = this->parseTuple(request.tuple);
    } catch (std::exception& e) {
        std::string errMsg = e.what();
        writeStringToCharArray(errMsg, response.tuple, sizeof(response.tuple));
        return response;
    }

    if(!tuple.has_value()) {
        writeStringToCharArray("invalid syntax", response.tuple, sizeof(response.tuple));
        return response;
    }

    if (tuple.value().size() > MAX_TUPLE_ITEMS) {
        writeStringToCharArray(TOO_LONG_TUPLE_ERROR, response.tuple, sizeof(response.tuple));
        return response;
    }

    return std::nullopt;
}



