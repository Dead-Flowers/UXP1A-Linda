#include <cerrno>
#include "linda/signals.h"
#include "linda/exceptions.h"

SignalWrapper::SignalWrapper(int signum, __sighandler_t handler) {
    _signum = signum;
    _previous = std::signal(signum, handler);
    if (_previous == SIG_ERR)
        throw LindaSyscallException(errno);
}

SignalWrapper::~SignalWrapper() {
    if (_previous != SIG_ERR)
        std::signal(_signum, _previous);
}