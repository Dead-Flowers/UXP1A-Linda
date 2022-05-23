#include "linda.h"

int main() {
    TupleSpaceHost host;
    host.init();
    host.runServer();
    puts("done\n");
    return 0;
}