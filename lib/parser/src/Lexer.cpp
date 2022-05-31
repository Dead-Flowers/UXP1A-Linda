
#include "parser/Lexer.h"

#include <cctype>
#include <algorithm>
#include <cmath>
#include "spdlog/sinks/stdout_color_sinks.h"
#include "parser/exceptions.h"

using Lexer = linda::modules::Lexer;
namespace utils = linda::modules::utils;

Lexer::Lexer(const std::string &input) : reader(StringReader(input)) {
    this->currentSign = this->reader.getNextCharacter();
}

Token Lexer::nextToken() {
    Token token;

    while(std::isspace(currentSign)) {
        currentSign = this->reader.getNextCharacter();
    }

    if (currentSign == (char)-1) {
        token.type = TokenType ::ETX;
        return token;
    }

    if (currentSign == '\"') {
        currentSign = this->reader.getNextCharacter();
        std::string buffer;

        while(currentSign != '\"') {
            if (currentSign == (char)-1) {
                throw LexerParsingException("Unexpected end of text");
            }
            buffer.push_back(currentSign);
            currentSign = this->reader.getNextCharacter();
        }

        token.type = TokenType ::StringLiteral;
        this->currentSign = this->reader.getNextCharacter();
        token.value = buffer;
        return token;
    } else if (std::isdigit(currentSign)) {

        long integerPart = 0;

        integerPart += (currentSign - '0');
        currentSign = this->reader.getNextCharacter();
        while(std::isdigit(currentSign)) {
            integerPart = (integerPart * 10) + (currentSign - '0');
            currentSign = this->reader.getNextCharacter();
        }
        if (currentSign == '.') {
            int fractionPart = 0;
            int decimalPlaces = 0;
            currentSign = this->reader.getNextCharacter();
            while (std::isdigit(currentSign)) {
                fractionPart = (fractionPart * 10) + (currentSign - '0');
                decimalPlaces++;
                currentSign = this->reader.getNextCharacter();
            }
            float finalValue = integerPart + fractionPart / pow(10, decimalPlaces);
            token.type = TokenType::FloatLiteral;
            token.value = finalValue;
            return token;
        }
        token.type = TokenType::IntLiteral;
        token.value = integerPart;
        return token;
    } else if (std::isalpha(currentSign)) {
        std::string buffer;
        while (std::isalpha(currentSign)) {
            buffer.push_back(currentSign);
            currentSign = this->reader.getNextCharacter();
        }
        if (buffer == "integer") token.type = TokenType::IntType;
        else if (buffer == "float") token.type = TokenType::FloatType;
        else if (buffer == "string") token.type = TokenType::StringType;
        else {
            throw LexerParsingException("Unrecognized data type");
        }
        token.value = buffer;
        return token;
    } else if (currentSign == ',') {
        token.type = TokenType::Comma;
        currentSign = this->reader.getNextCharacter();
        return token;
    } else if (currentSign == ':') {
        token.type = TokenType::Colon;
        currentSign = this->reader.getNextCharacter();
        return token;
    } else if (currentSign == '(') {
        token.type = TokenType::ParenthOpen;
        currentSign = this->reader.getNextCharacter();
        return token;
    } else if (currentSign == ')') {
        token.type = TokenType::ParenthClose;
        currentSign = this->reader.getNextCharacter();
        return token;
    } else if (currentSign == '<' || currentSign == '>' || currentSign == '=') {
        char firstSign = currentSign;
        currentSign = this->reader.getNextCharacter();
        if (currentSign == '=') {
            std::string op;
            op.push_back(firstSign);
            op.push_back(currentSign);
            if (op == "<=") token.type = TokenType::LessOrEqualOp;
            else if (op == ">=") token.type = TokenType::GreaterOrEqualOp;
            else if (op == "==") token.type = TokenType::EqualOp;
            currentSign = this->reader.getNextCharacter();
            return token;
        }
        if (firstSign == '<') token.type = TokenType::LessOp;
        else if (firstSign == '>') token.type = TokenType::GreaterOp;
        else {
            throw LexerParsingException("Unrecognized operator");
        }
        return token;
    } else if (currentSign == '*') {
        token.type = TokenType::Asterisk;
        currentSign = this->reader.getNextCharacter();
        return token;
    } else if (currentSign == '!') {
        currentSign = this->reader.getNextCharacter();
        if (currentSign == '=') {
            token.type = TokenType::NotEqualOp;
            return token;
        }
        throw LexerParsingException("Unrecognized operator");
    } else {
        throw LexerParsingException("Unrecognized character");
    }
}
