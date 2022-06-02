#ifndef LINDA_MESSAGE_H
#define LINDA_MESSAGE_H
#include <sys/ipc.h>
#include <cstdint>
#include "tuple.h"
#include "consts.h"
#include <iostream>


enum class MessageType : long {
    Input = 0x01,
    Output = 0x02,
    Read = 0x03,
    Error = 0x04,
};

const char* messageTypeToString(MessageType type);

struct TupleRequest {
    MessageType messageType; // required filed `long mtype`
    key_t responseQueueKey; // key of communicates queue server -> client
    uint32_t requestId; // unique(for client) request id
    char tuple[MAX_TUPLE_LENGTH]; // tuple or pattern of tuple
    friend std::ostream& operator<< (std::ostream& buf, const TupleRequest& req);
};

std::ostream& operator<< (std::ostream& os, const TupleRequest& req);

struct TupleResponse {
    MessageType messageType; // required filed `long mtype`
    uint32_t requestId;
    char tuple[MAX_TUPLE_LENGTH];
    friend std::ostream& operator<< (std::ostream& buf, const TupleResponse& req);
};

std::ostream& operator<< (std::ostream& os, const TupleResponse& req);

#endif //LINDA_MESSAGE_H
