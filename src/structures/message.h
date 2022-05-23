#ifndef LINDA_MESSAGE_H
#define LINDA_MESSAGE_H
#include <sys/ipc.h>
#include <cstdint>
#include "tuple.h"


enum class MessageType : long {
    Input = 0x01,
    Output = 0x02,
    Read = 0x03,
};

struct TupleRequest {
    MessageType messageType; // required filed `long mtype`
    key_t responseQueueKey; // key of communicates queue server -> client
    uint32_t requestId; // unique(for client) request id
    char tuple[MAX_TUPLE_LENGTH]; // tuple or pattern of tuple 
};

struct TupleResponse {
    MessageType messageType; // required filed `long mtype`
    uint32_t requestId;
    char tuple[MAX_TUPLE_LENGTH];
};

#endif //LINDA_MESSAGE_H
