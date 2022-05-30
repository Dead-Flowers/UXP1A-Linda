#include "linda/linda.h"

int main() {
    TupleSpaceHost host;
    host.init("./key.k", 2137);
    host.runServer();
    puts("done\n");
    return 0;
}