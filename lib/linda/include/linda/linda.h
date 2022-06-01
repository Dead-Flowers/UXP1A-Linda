#ifndef LINDA_LINDA_H
#define LINDA_LINDA_H

#include <atomic>
#include <csignal>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "tuple.h"
#include "message.h"
#include <functional>
#include <optional>
#include <iostream>
#include <cstring>
#include <memory>
#include "parser/TupleParser.h"
#include "parser/Lexer.h"
#include "parser/PatternsParser.h"
#include "spdlog/logger.h"
#include "consts.h"

void clientSigHandler(int signum);

class TupleSpace {
public:
    TupleSpace();
    ~TupleSpace();
    // timeout is in seconds, 0 - wait for sever response indefinitely
    void open(const char* keyPath, int projectId, int clientChmod = DEFAULT_CHMOD);
    std::optional<Tuple> input(std::string tupleTemplate, int timeout);
    std::optional<Tuple> read(std::string tupleTemplate, int timeout);
    void output(std::string tuple);
    void close();
private:
    std::shared_ptr<spdlog::logger> _logger;
    int _hostQueueId;
    int _clientQueueId;
    key_t _clientQueueKey;
    std::atomic_int _lastRequestId;
    std::atomic_flag _closed = ATOMIC_FLAG_INIT;

    std::optional<TupleResponse> waitForResponse(uint32_t requestId, int timeout);
    std::optional<Tuple> requestTuple(std::string tupleTemplate, int timeout, bool pop);


    inline static std::atomic_flag LindaInitialized = ATOMIC_FLAG_INIT;
    static void init() {
        if (LindaInitialized.test_and_set())
            return;
        signal(SIGALRM, clientSigHandler);
        std::srand(time(nullptr));
    }
};


static std::function<void(int)> closeProgram;

void sigHandler(int signum);

class TupleSpaceHost {
public:
    TupleSpaceHost();
    ~TupleSpaceHost();

    void init(const char* keyPath, int projectId, int chmod = DEFAULT_CHMOD);
    void runServer();
    void close();
    int spaceSize();
    bool contains(const TuplePattern& tuplePattern);
    void reset();
private:
    std::optional<TupleResponse> processRequest(TupleRequest);
    std::optional<TupleResponse> processReadOrInput(TupleRequest, bool pop);
    std::optional<TupleResponse> processOutput(TupleRequest);
    bool trySendResponse(key_t responseQueueKey, const TupleResponse& tupleResponse);
    void insertTuple(const Tuple& tuple);
    bool tryMatchPendingRequests(const Tuple& tuple);
    std::optional<Tuple> searchSpace(const TuplePattern& tuplePattern, bool pop);
    void insertPendingRequest(uint32_t requestId, key_t responseQueueKey, const TuplePattern& tuplePattern);
    bool compareValue(TupleItem, TupleItemPattern);
    Tuple parseTuple(const char*);
    std::vector<TupleItemPattern> parsePattern(const char*);
    bool patternMatchesTuple(const TuplePattern& pattern, const Tuple& tuple);
    TupleSpaceContainer space;
    int mainQueueId;
    int run;
    std::shared_ptr<spdlog::logger> _logger;
    std::atomic_flag _closed = ATOMIC_FLAG_INIT;
};




#endif //LINDA_LINDA_H
