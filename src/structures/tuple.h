#ifndef LINDA_TUPLE_H
#define LINDA_TUPLE_H
#include <sys/ipc.h>
#include <cstdint>
#include <vector>
#include <variant>
#include <string>

const int MAX_TUPLE_LENGTH = 1024;

using TupleItem = std::variant<int64_t, float, std::string>;
using Tuple = std::vector<TupleItem>;

enum class TupleDataType {
    String,
    Integer,
    Float
};

enum class TupleOperator {
    Equal, /* default operator */
    Less,
    LessEqual,
    Greater,
    GreaterEqual
};

struct TupleItemPattern {
    TupleDataType type;
    TupleOperator op;
    TupleItem value;
};

// pending request for read/input
struct PendingTupleRequest {
    key_t responseQueueKey; // key of communicates queue server -> client
    uint32_t requestId; // unique(for client) request id
    std::vector<TupleItemPattern> itemPatterns;
};

struct TupleSpaceContainer {
    std::vector<Tuple> data;
    std::vector<PendingTupleRequest> pendingRequests;
};

#endif //LINDA_TUPLE_H
