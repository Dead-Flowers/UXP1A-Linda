#include <argparse/argparse.hpp>

#include "linda/linda.h"

int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("linda-server");
    parser.add_description("Hosts the global Linda tuple space.");
    parser.parse_args(argc, argv);

    TupleSpaceHost host;
    host.init("./key.k", 2137);
    host.runServer();
    puts("done\n");
    return 0;
}
