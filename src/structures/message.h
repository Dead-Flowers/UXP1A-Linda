//
// Created by powerofdark on 22/05/22.
//

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
    MessageType messageType; // wymagane pole `long mtype`
    key_t responseQueueKey; // klucz kolejki komunikatów serwer->klient
    uint32_t requestId; // unikalny (w ramach klienta) id. zapytania
    char tuple[MAX_TUPLE_LENGTH]; // krotka lub wzorzec krotki
};

struct TupleResponse {
    MessageType messageType; // wymagane pole `long mtype`
    uint32_t requestId;
    char tuple[MAX_TUPLE_LENGTH];
};

#endif //LINDA_MESSAGE_H
