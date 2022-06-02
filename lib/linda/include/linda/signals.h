#ifndef LINDA_SIGNALS_H
#define LINDA_SIGNALS_H

#include <csignal>

class SignalWrapper {
public:
    SignalWrapper(int signum, __sighandler_t handler);
    ~SignalWrapper();
private:
    __sighandler_t _previous;
    int _signum;
};

#endif //LINDA_SIGNALS_H
