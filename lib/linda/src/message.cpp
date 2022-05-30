#include "linda/message.h"

const char* MsgTypeToString(MessageType type) {
    switch (type) {
        case MessageType::Input: return "INPUT";
        case MessageType::Output: return "OUTPUT";
        case MessageType::Read: return "READ";
    }
}

std::ostream& operator<< (std::ostream& os, const TupleRequest& req) {
    os << "Type: " << MsgTypeToString(req.messageType) << " responseQueueKey: " << req.responseQueueKey
       << " requestId: " << req.requestId << " \n TUPLE: " << req.tuple;
    return  os;
}

std::ostream& operator<< (std::ostream& os, const TupleResponse& req) {

    os << "Type: " << MsgTypeToString(req.messageType)
       << " requestId: " << req.requestId << " \n TUPLE: ";
    for(auto c: req.tuple) {
        os << c;
    }
    os << std::endl;
    return  os;
}

std::string TupleToString(Tuple tuple) {
    std::string str = "";
    for(auto item: tuple) {
        switch (static_cast<TupleDataType>(item.index())) {
            case TupleDataType::Integer : {
                str += std::get<int64_t>(item);
            }
            case TupleDataType::Float : {
                str += std::get<float>(item);
            }
            case TupleDataType::String : {
                str += std::get<std::string>(item);
            }
        }
        str += ",";
    }

    return str;
}