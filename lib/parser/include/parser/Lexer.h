#ifndef PARSER_LEXER_H


#include <string>
#include "Token.h"
#include "StringReader.h"
#include "spdlog/logger.h"

using Token = linda::modules::utils::Token;

namespace linda::modules {
    class Lexer {
    public:
        Lexer(const std::string &input);
        ~Lexer() = default;

        Token nextToken();
    private:
        StringReader reader;
        char currentSign;
    };

}

#define PARSER_LEXER_H

#endif //PARSER_LEXER_H
