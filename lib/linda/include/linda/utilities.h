#ifndef LINDA_UTILITIES_H
#define LINDA_UTILITIES_H
#include <string>
#include <sys/msg.h>
#include <random>
#include <exception>
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

key_t createQueueWithRandomKey(int flags, int* pMsgId = nullptr) {
    int msgId;
    key_t key;
    do {
        key = std::rand();
        msgId = msgget(key == 0 ? 1 : key, flags);
    } while (msgId == -1 && errno == EEXIST);

    if (msgId == -1) {
        return -1;
    }
    if (pMsgId)
        *pMsgId = msgId;
    return key;
}

bool writeStringToCharArray(const std::string& str, char* pDest, size_t size) {
    auto sizeWritten = str.copy(pDest, size);
    pDest[std::min(size - 1, sizeWritten)] = 0;
    return sizeWritten == str.length();
}

template <class Comparable>
bool compareTuple(const Comparable& left, TupleOperator op, const Comparable& right) {
    switch (op) {
        case TupleOperator::Equal: {return left == right;}
        case TupleOperator::Less: {return left < right;}
        case TupleOperator::LessEqual: {return left <= right;}
        case TupleOperator::Greater: {return left > right;}
        case TupleOperator::GreaterEqual: {return left >= right;}
        default: {throw std::invalid_argument("bad operator"); }
    }
}

TupleDataType getTupleDataType(const TupleItem& item) {
    if (holds_alternative<float>(item)) return TupleDataType::Float;
    if (holds_alternative<int64_t>(item)) return TupleDataType::Integer;
    if (holds_alternative<std::string>(item)) return TupleDataType::String;
    throw std::invalid_argument("Invalid item type");
}

template <class T>
bool compareTupleToPattern(const TupleItem& item, const TupleItemPattern& pattern) {
    if (getTupleDataType(item) != pattern.type)
        return false;
    if (!pattern.value.has_value()) {
        return true; // in case the operand is a wildcard
    }
    auto value = std::get<T>(item);
    auto patternValue = std::get<T>(pattern.value.value());
    return compareTuple(value, pattern.op, patternValue);
}

std::shared_ptr<spdlog::logger> getLogger(const std::string& name) {
    auto logger = spdlog::get(name);
    return logger == nullptr ? spdlog::stderr_color_mt(name) : logger;
}

#endif