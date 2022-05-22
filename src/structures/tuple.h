//
// Created by powerofdark on 22/05/22.
//

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
    Equal, /* operator domyślny */
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

// struktura reprezentująca oczekującą operację read/input
struct PendingTupleRequest {
    key_t responseQueueKey; // klucz kolejki komunikatów serwer->klient
    uint32_t requestId; // unikalny (w ramach klienta) id. zapytania
    std::vector<TupleItemPattern> itemPatterns; // wzorce elementów krotki
};

struct TupleSpaceContainer {
    std::vector<Tuple> data;
    std::vector<PendingTupleRequest> pendingRequests;
};

#endif //LINDA_TUPLE_H
