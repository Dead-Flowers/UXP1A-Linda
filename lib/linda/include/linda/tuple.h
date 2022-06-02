#ifndef LINDA_TUPLE_H
#define LINDA_TUPLE_H
#include <sys/ipc.h>
#include <cstdint>
#include <vector>
#include <variant>
#include <string>
#include <optional>



using TupleItem = std::variant<int64_t, float, std::string>;
using Tuple = std::vector<TupleItem>;

std::string tupleToString(const Tuple& tuple);

// This has to have the same order as variant because of variant index() usage
enum class TupleDataType {
    Integer,
    Float,
    String
};

enum class TupleOperator {
    Equal, /* default operator */
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual
};

struct TupleItemPattern {
    TupleDataType type;
    TupleOperator op;
    std::optional<TupleItem> value;
};


// pending request for read/input
struct PendingTupleRequest {
    key_t responseQueueKey; // key of communicates queue server -> client
    bool pop;
    uint32_t requestId; // unique(for client) request id
    std::vector<TupleItemPattern> itemPatterns;
};
struct TupleSpaceContainer {
    std::vector<Tuple> data;
    std::vector<PendingTupleRequest> pendingRequests;
};

using TuplePattern = std::vector<TupleItemPattern>;

#endif //LINDA_TUPLE_H
