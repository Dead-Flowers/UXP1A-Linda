
#ifndef LINDA_TUPLEPARSER_H
#define LINDA_TUPLEPARSER_H

#include "linda/tuple.h"
#include "Lexer.h"

namespace linda::modules {
    class TupleParser {
    public:
        TupleParser(const Lexer& lexer);
        ~TupleParser() = default;

        Tuple parse();

    private:
        Lexer lexer;
        Token currentToken;
        std::optional<TupleItem> tryParseTupleItem();
        bool checkAndConsume(TokenType type);
    };
}

#endif //LINDA_TUPLEPARSER_H
