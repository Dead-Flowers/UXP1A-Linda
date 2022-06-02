
#ifndef LINDA_TUPLE_PARSER_H
#define LINDA_TUPLE_PARSER_H

#include "linda/tuple.h"
#include "lexer.h"

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

#endif //LINDA_TUPLE_PARSER_H
