#include <sstream>
#include "linda/message.h"

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

const char* messageTypeToString(MessageType type) {
    switch (type) {
        case MessageType::Input: return "INPUT";
        case MessageType::Output: return "OUTPUT";
        case MessageType::Read: return "READ";
        case MessageType::Error: return "ERROR";
        default: return "UNKNOWN";
    }
}

std::ostream& operator<< (std::ostream& os, const TupleRequest& req) {
    os << "Type: " << messageTypeToString(req.messageType) << " responseQueueKey: " << req.responseQueueKey
       << " requestId: " << req.requestId << " \n TUPLE: " << req.tuple;
    return  os;
}

std::ostream& operator<< (std::ostream& os, const TupleResponse& req) {

    os << "Type: " << messageTypeToString(req.messageType)
       << " requestId: " << req.requestId << " \n TUPLE: ";
    for(auto c: req.tuple) {
        os << c;
    }
    os << std::endl;
    return  os;
}

std::string tupleToString(const Tuple& tuple) {
    std::ostringstream ss;
    ss << "(";
    for(auto item: tuple) {
        std::visit(overloaded {
                [&](const std::string value) { ss << "\"" << value << "\""; },
                [&](const auto value) { ss << value; },
        }, item);

        ss << ",";
    }
    if (!tuple.empty()) {
        // trim comma
        ss.seekp(-1, std::ios_base::cur);
    }
    ss << ")";

    return ss.str();
}