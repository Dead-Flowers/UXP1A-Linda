#include "linda/linda.h"
#include "spdlog/cfg/env.h"

int main() {
    spdlog::cfg::load_env_levels();

    TupleSpace client;
    client.open("/tmp/linda.key", 1234);
    client.output("(123, 456.789");



    puts("done\n");
    return 0;
}