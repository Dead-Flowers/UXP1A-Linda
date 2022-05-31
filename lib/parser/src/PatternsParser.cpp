
#include "parser/PatternsParser.h"
#include "parser/exceptions.h"

using PatternParser = linda::modules::PatternParser;

std::vector<TupleItemPattern> PatternParser::parse() {
    std::vector<TupleItemPattern> patterns;
    currentToken = lexer.nextToken();
    if (!checkAndConsume(TokenType::ParenthOpen)) {
        throw PatternParsingException("Missing opening parenthesis");
    }
    auto tuplePattern = tryParseTuplePattern();
    if (tuplePattern == std::nullopt) {
        return patterns;
    }
    patterns.push_back(tuplePattern.value());
    while (checkAndConsume(TokenType::Comma)) {
        tuplePattern = tryParseTuplePattern();
        if (tuplePattern == std::nullopt) {
            throw PatternParsingException("Missing pattern definition");
        }
        patterns.push_back(tuplePattern.value());
    }
    if (!checkAndConsume(TokenType::ParenthClose)) {
        throw PatternParsingException("Missing closing parenthesis");
    }
    return patterns;
}

PatternParser::PatternParser(const Lexer &lexer) : lexer(lexer) {}

std::optional<TupleItemPattern> PatternParser::tryParseTuplePattern() {
    TupleItemPattern pattern;
    auto dataType = tryParseDataType();
    if (dataType == std::nullopt) {
        return std::nullopt;
    } else {
        pattern.type = dataType.value();
    }
    if (!checkAndConsume(TokenType::Colon)) {
        throw PatternParsingException("Missing colon");
    }
    auto tupleOperator = tryParseOperator();
    if (tupleOperator == std::nullopt) {
        pattern.op = TupleOperator::Equal;
    } else {
        pattern.op = tupleOperator.value();
    }
    auto conditionValue = tryParseValue();
    if (conditionValue == std::nullopt) {
        if (currentToken.type != TokenType::Asterisk) {
            throw PatternParsingException("Missing condition");
        } else {
            if(!checkAndConsume(TokenType::Asterisk)) {
                throw PatternParsingException("Missing asterisk");
            }
        }
    } else {
        pattern.value = conditionValue.value();
    }

    if (conditionValue.has_value() && dataType.value() == TupleDataType::Float && std::holds_alternative<int64_t>(conditionValue.value())) {
        pattern.value = (float)get<int64_t>(conditionValue.value());
    }
    if (!checkCombination(pattern.type, pattern.op, pattern.value)) {
        throw PatternParsingException("Invalid pattern type and value combination");
    }
    return pattern;
}

std::optional<TupleDataType> PatternParser::tryParseDataType() {
    TupleDataType dataType;
    if (checkAndConsume(TokenType::IntType)) dataType = TupleDataType::Integer;
    else if (checkAndConsume(TokenType::FloatType)) dataType = TupleDataType::Float;
    else if (checkAndConsume(TokenType::StringType)) dataType = TupleDataType::String;
    else {
        return std::nullopt;
    }
    return dataType;
}

std::optional<TupleOperator> PatternParser::tryParseOperator() {
    TupleOperator tupleOperator;
    if (checkAndConsume(TokenType::EqualOp)) tupleOperator = TupleOperator::Equal;
    else if (checkAndConsume(TokenType::NotEqualOp)) tupleOperator = TupleOperator::NotEqual;
    else if (checkAndConsume(TokenType::LessOp)) tupleOperator = TupleOperator::Less;
    else if (checkAndConsume(TokenType::LessOrEqualOp)) tupleOperator = TupleOperator::LessEqual;
    else if (checkAndConsume(TokenType::GreaterOp)) tupleOperator = TupleOperator::Greater;
    else if (checkAndConsume(TokenType::GreaterOrEqualOp)) tupleOperator = TupleOperator::GreaterEqual;
    else {return std::nullopt;}
    return tupleOperator;
}

std::optional<TupleItem> PatternParser::tryParseValue() {
    TupleItem value;
    if (currentToken.type == TokenType::IntLiteral
        || currentToken.type == TokenType::FloatLiteral
        || currentToken.type == TokenType:: StringLiteral) {
        value = currentToken.value;
        currentToken = this->lexer.nextToken();
        return value;
    }
    return std::nullopt;
}

bool PatternParser::checkAndConsume(TokenType type) {
    if (currentToken.type == type) {
        currentToken = this->lexer.nextToken();
        return true;
    } else {
        return false;
    }
}

bool PatternParser::checkCombination(TupleDataType dataType, TupleOperator op, const std::optional<TupleItem>& optionalValue) {
    if (!optionalValue.has_value()){
        if (op != TupleOperator::Equal) {
            throw PatternParsingException("Invalid operator for wildcard value");
        }
        return true;
    }
    auto& value = optionalValue.value();
    if (dataType == TupleDataType::String && !holds_alternative<std::string>(value)) {
        return false;
    }
    if (dataType == TupleDataType::Float && !std::holds_alternative<float>(value)) {
        return false;
    }
    if (dataType == TupleDataType::Integer && !std::holds_alternative<int64_t>(value)) {
        return false;
    }
    return true;
}



