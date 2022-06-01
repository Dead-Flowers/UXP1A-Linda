#include "linda/linda.h"
#include "spdlog/cfg/env.h"

int main() {
    spdlog::cfg::load_env_levels();

    TupleSpaceHost host;
    host.init("/tmp/linda.key", 1234);
    host.runServer();
    puts("done\n");
    return 0;
}