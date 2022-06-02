
#include "parser/lexer.h"

#include <cctype>
#include <algorithm>
#include <cmath>
#include "spdlog/sinks/stdout_color_sinks.h"
#include "parser/exceptions.h"
#include "parser/consts.h"

using Lexer = linda::modules::Lexer;
namespace utils = linda::modules::utils;

Lexer::Lexer(const std::string &input) : reader(StringReader(input)) {
    this->currentSign = this->reader.getNextCharacter();
}

Token Lexer::nextToken() {
    this->token = Token();

    while(std::isspace(currentSign)) {
        currentSign = this->reader.getNextCharacter();
    }

    if (currentSign == (char)-1) {
        token.type = TokenType::ETX;
        return token;
    }

    if (tryBuildStringLiteral()) return token;
    if (tryBuildNumberLiteral()) return token;
    if (tryBuildType()) return token;
    if (tryBuildSingleCharacterKeyword()) return token;
    if (tryBuildMultipleCharacterOperator()) return token;
    throw LexerParsingException("Unrecognized character");
}

bool Lexer::tryBuildStringLiteral() {
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
        return true;
    } else {
        return false;
    }
}

bool Lexer::tryBuildNumberLiteral() {
    if (std::isdigit(currentSign) || currentSign == '-') {

        bool belowZero = false;
        belowZero = currentSign == '-';
        if (belowZero) {
            currentSign = this->reader.getNextCharacter();
            if (!std::isdigit(currentSign)) throw LexerParsingException("Could not find number after minus sign");
        }

        long integerPart = 0;

        int digitCount = 0;
        integerPart += (currentSign - '0');
        currentSign = this->reader.getNextCharacter();
        digitCount++;
        while(std::isdigit(currentSign)) {
            if (digitCount >= LONG_MAX_DIGIT_COUNT) {
                throw LexerParsingException("Number overflow");
            }
            integerPart = (integerPart * 10) + (currentSign - '0');
            digitCount++;
            currentSign = this->reader.getNextCharacter();
        }
        if (currentSign == '.') {
            int fractionPart = 0;
            int decimalPlaces = 0;
            currentSign = this->reader.getNextCharacter();
            while (std::isdigit(currentSign)) {
                if (decimalPlaces > FLOAT_DECIMAL_PRECISION) {
                    throw LexerParsingException("Decimal part overflow");
                }
                fractionPart = (fractionPart * 10) + (currentSign - '0');
                decimalPlaces++;
                currentSign = this->reader.getNextCharacter();
            }
            float finalValue = integerPart + fractionPart / pow(10, decimalPlaces);
            token.type = TokenType::FloatLiteral;
            token.value = belowZero ? -finalValue : finalValue;
            return true;
        }
        token.type = TokenType::IntLiteral;
        token.value = belowZero ? -integerPart : integerPart;
        return true;
    } else {
        return false;
    }
}

bool Lexer::tryBuildType() {
    if (std::isalpha(currentSign)) {
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
        return true;
    } else {
        return false;
    }
}

bool Lexer::tryBuildSingleCharacterKeyword() {
    if (currentSign == ',') {
        token.type = TokenType::Comma;
        currentSign = this->reader.getNextCharacter();
        return true;
    } else if (currentSign == ':') {
        token.type = TokenType::Colon;
        currentSign = this->reader.getNextCharacter();
        return true;
    } else if (currentSign == '(') {
        token.type = TokenType::ParenthOpen;
        currentSign = this->reader.getNextCharacter();
        return true;
    } else if (currentSign == ')') {
        token.type = TokenType::ParenthClose;
        currentSign = this->reader.getNextCharacter();
        return true;
    } else if (currentSign == '*') {
        token.type = TokenType::Asterisk;
        currentSign = this->reader.getNextCharacter();
        return true;
    } else {
        return false;
    }
}

bool linda::modules::Lexer::tryBuildMultipleCharacterOperator() {
    if (currentSign == '<' || currentSign == '>' || currentSign == '=') {
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
            return true;
        }
        if (firstSign == '<') token.type = TokenType::LessOp;
        else if (firstSign == '>') token.type = TokenType::GreaterOp;
        else {
            throw LexerParsingException("Unrecognized operator");
        }
        return true;
    } else if (currentSign == '!') {
        currentSign = this->reader.getNextCharacter();
        if (currentSign == '=') {
            token.type = TokenType::NotEqualOp;
            currentSign = this->reader.getNextCharacter();
            return true;
        }
        throw LexerParsingException("Unrecognized operator");
    }
}
