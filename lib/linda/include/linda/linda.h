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

#define DEFAULT_KEY_FILE_PATH "./key.k"
#define DEFAULT_PROJECT_ID 2137

#define MAX_TUPLE_LEN 15
#define TOO_LONG_TUPLE_ERROR "too long tuple"


void clientSigHandler(int signum);


class TupleSpace {
public:
    TupleSpace();
    ~TupleSpace();
    // timeout is in seconds, 0 - wait for sever response indefinitely
    void open(key_t tupleHostKey);
    void close();
    std::optional<Tuple> input(std::string tupleTemplate, int timeout);
    std::optional<Tuple> read(std::string tupleTemplate, int timeout);
    void output(std::string tuple);
private:
    std::shared_ptr<spdlog::logger> _logger;
    int _hostQueueId;
    int _clientQueueId;
    key_t _clientQueueKey;
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

    void init(const char* keyPath, int projectId);
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
    void insertTuple(Tuple);
    void notifyPendingRequests(const Tuple& tuple);
    std::optional<Tuple> searchSpace(const TuplePattern& tuplePattern, bool pop);
    void insertPendingRequest(uint32_t requestId, key_t responseQueueKey, const TuplePattern& tuplePattern);
    bool compareValue(TupleItem, TupleItemPattern); //TODO: consider removing out of class
    std::optional<Tuple> parseTuple(const char*); //TODO: implement
    std::vector<TupleItemPattern> parsePattern(const char*); //TODO: implement
    bool patternMatchesTuple(const TuplePattern& pattern, const Tuple& tuple);
    std::optional<TupleResponse> checkTuple(TupleRequest);
    std::optional<TupleResponse> checkPattern(TupleRequest);
    TupleSpaceContainer space;
    int mainQueueId;
    int run;
    std::shared_ptr<spdlog::logger> _logger;
    std::atomic_flag _closed = ATOMIC_FLAG_INIT;
};




#endif //LINDA_LINDA_H
