
#ifndef PARSER_PATTERNS_H
#define PARSER_PATTERNS_H


#include "linda/tuple.h"
#include "Lexer.h"

namespace linda::modules {
    class PatternParser {
    public:
        PatternParser(const Lexer& lexer);
        ~PatternParser() = default;

        std::vector<TupleItemPattern> parse();

    private:
        Lexer lexer;
        Token currentToken;
        std::optional<TupleItemPattern> tryParseTuplePattern();
        std::optional<TupleDataType> tryParseDataType();
        std::optional<TupleOperator> tryParseOperator();
        std::optional<TupleItem> tryParseValue();
        bool checkAndConsume(TokenType type);
        bool checkCombination(TupleDataType dataType, TupleOperator op, const std::optional<TupleItem>& value);
    };
}

#endif //PARSER_PATTERNS_H
