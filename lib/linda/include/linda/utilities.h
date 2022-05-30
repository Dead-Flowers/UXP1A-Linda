#ifndef LINDA_UTILITIES_H
#define LINDA_UTILITIES_H
#include <string>
#include <sys/msg.h>
#include <random>
#include <exception>

key_t createQueueWithRandomKey(int flags, int* pMsgId = nullptr) {
    int msgId;
    key_t key;
    do {
        key = std::rand();
        msgId = msgget(key == 0 ? 1 : key, flags);
    } while (msgId == -1 && errno == EEXIST);

    if (msgId == -1) {
        throw std::exception(); // TODO: exceptions
    }
    if (pMsgId)
        *pMsgId = msgId;
    return key;
}

bool writeStringToCharArray(std::string str, char* pDest, size_t size) {
    auto sizeWritten = str.copy(pDest, size);
    return sizeWritten == str.length();
}

template <class Comparable>
bool compareTuple(Comparable left, TupleOperator op, Comparable right) {
    switch (op) {
        case TupleOperator::Equal: {return left == right;}
        case TupleOperator::Less: {return left < right;}
        case TupleOperator::LessEqual: {return left <= right;}
        case TupleOperator::Greater: {return left > right;}
        case TupleOperator::GreaterEqual: {return left >= right;}
        default: {throw "bad operator";}
    }
}

template <class T>
bool compareTupleToPattern(TupleItem item, TupleItemPattern pattern) {
    auto value = std::get<T>(item);
    auto patternValue = std::get<T>(pattern.value);
    return compareTuple(value, pattern.op, patternValue);
}

#endif