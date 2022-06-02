#include <unordered_map>
#include <string>

#ifndef PARSER_TOKEN_TYPE_H
#define PARSER_TOKEN_TYPE_H

namespace linda::modules::utils
{
    enum class TokenType: unsigned int
    {
        Unknown             = 0,
        ParenthOpen         = 1,
        ParenthClose        = 2,
        IntLiteral          = 3,
        FloatLiteral        = 4,
        StringLiteral       = 5,
        IntType             = 6,
        FloatType           = 7,
        StringType          = 8,
        Asterisk            = 9,
        EqualOp             = 10,
        NotEqualOp          = 11,
        GreaterOp           = 12,
        GreaterOrEqualOp    = 13,
        LessOp              = 14,
        LessOrEqualOp       = 15,
        Comma               = 16,
        Colon               = 17,
        ETX                 = 18
    };

    const std::unordered_map<unsigned int, std::string> tokenTypeNames = {
            { 0,  "ParenthOpen"},
            { 1,  "ParenthClose"},
            { 2,  "IntLiteral"},
            { 3,  "FloatLiteral"},
            { 4,  "StringLiteral"},
            { 5,  "IntType"},
            { 6,  "FloatType"},
            { 7,  "StringType"},
            { 8,  "Asterisk"},
            { 9,  "EqualOp"},
            { 10, "NotEqualOp"},
            { 11, "GreaterOp"},
            { 12, "GreaterOrEqualOp"},
            { 13, "LessOp"},
            { 14, "LessOrEqualOp"},
            { 15,  "Comma"},
            { 16,  "Colon"},
            { 17,  "ETX"}
    };

    inline const std::string getTokenTypeName(const TokenType& tokenType)
    {
        return tokenTypeNames.at(static_cast<unsigned int>(tokenType));
    };
}

#endif //PARSER_TOKEN_TYPE_H
