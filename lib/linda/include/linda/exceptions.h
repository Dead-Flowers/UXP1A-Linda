#ifndef LINDA_EXCEPTIONS_H
#define LINDA_EXCEPTIONS_H

#include <exception>
#include <string>
#include <cstring>

struct LindaSyscallException : public std::exception
{
    const char* sysErrorMsg;

    LindaSyscallException(int errNumber) {
        sysErrorMsg = std::strerror(errNumber);
    }
    virtual const char* what() const throw()
    {
        return sysErrorMsg;
    }
};

struct LindaException : public std::exception
{
    const char* errorMsg;
    LindaException(const char* msg) {
        errorMsg = msg;
    }
    virtual const char* what() const throw()
    {
        return errorMsg;
    }
};


#endif
