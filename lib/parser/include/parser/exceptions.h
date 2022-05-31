#ifndef PARSER_EXCEPTIONS_H
#define PARSER_EXCEPTIONS_H

#include <exception>
#include <string>
#include <cstring>

struct LexerParsingException : public std::exception {
    const char* errorMsg;

    LexerParsingException(const char* msg) {
        errorMsg = msg;
    }

    virtual const char* what() const throw() {
        return errorMsg;
    }
};

struct PatternParsingException : public std::exception {
    const char* errorMsg;

    PatternParsingException(const char* msg) {
        errorMsg = msg;
    }

    virtual const char* what() const throw() {
        return errorMsg;
    }
};

struct TupleParsingException : public std::exception {
    const char* errorMsg;

    TupleParsingException(const char* msg) {
        errorMsg = msg;
    }

    virtual const char* what() const throw() {
        return errorMsg;
    }
};

#endif
