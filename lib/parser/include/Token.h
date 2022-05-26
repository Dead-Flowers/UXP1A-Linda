#ifndef PARSER_TOKEN_H
#define PARSER_TOKEN_H

#include <string>
#include <variant>

#include "TokenType.h"

using TokenType = linda::modules::utils::TokenType;

namespace linda::modules::utils
{
    class Token
    {
    public:
        TokenType type;
        std::variant<int64_t, float, std::string> value;

        Token() = default;
        ~Token() = default;
    };
}

#endif //PARSER_TOKEN_H
