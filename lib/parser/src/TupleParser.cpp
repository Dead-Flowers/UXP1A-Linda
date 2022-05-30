#include "parser/TupleParser.h"

using TupleParser = linda::modules::TupleParser;


TupleParser::TupleParser(const Lexer &lexer) : lexer(lexer) {}

Tuple TupleParser::parse() {
    std::vector<TupleItem> tuples;
    currentToken = lexer.nextToken();
    if (!checkAndConsume(TokenType::ParenthOpen)) {
        // TODO: error missing opening parenth
    }
    auto tuple = tryParseTupleItem();
    if (tuple == std::nullopt) {
        return tuples;
    }
    tuples.push_back(tuple.value());
    while(checkAndConsume(TokenType::Comma)) {
        tuple = tryParseTupleItem();
        if (tuple == std::nullopt) {
            // TODO: error missing tuple definition
        }
        tuples.push_back(tuple.value());
    }
    if (!checkAndConsume(TokenType::ParenthClose)) {
        // TODO: error missing closing parenth
    }
    return tuples;
}

std::optional<TupleItem> TupleParser::tryParseTupleItem() {
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

bool TupleParser::checkAndConsume(TokenType type) {
    if (currentToken.type == type) {
        currentToken = this->lexer.nextToken();
        return true;
    } else {
        return false;
    }
}


