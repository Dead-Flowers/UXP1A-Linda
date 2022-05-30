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

    std::optional<Tuple> requestTuple(std::string tupleTemplate, int timeout, bool pop);

    inline static std::atomic_flag LindaInitialized = ATOMIC_FLAG_INIT;
    static void init() {
        if (LindaInitialized.test_and_set())
            return;
        signal(SIGALRM, SIG_IGN);
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
private:
    std::optional<TupleResponse> processRequest(TupleRequest);
    std::optional<TupleResponse> processReadOrInput(TupleRequest, bool pop);
    void processOutput(TupleRequest);
    void insertTuple(Tuple);
    void notifyPendingRequests(Tuple tuple);
    std::optional<Tuple> searchSpace(const TuplePattern& tuplePattern, bool pop);
    void insertPendingRequest(uint32_t requestId, key_t responseQueueKey, const TuplePattern& tuplePattern);
    bool compareValue(TupleItem, TupleItemPattern); //TODO: consider removing out of class
    Tuple parseTuple(const char*); //TODO: implement
    std::vector<TupleItemPattern> parsePattern(const char*); //TODO: implement
    void printSpace();
    bool patternMatchesTuple(const TuplePattern& pattern, const Tuple& tuple);
    TupleSpaceContainer space;
    int mainQueueId;
    int run;
    std::shared_ptr<spdlog::logger> _logger;
};




#endif //LINDA_LINDA_H
