#include <argparse/argparse.hpp>

#include "linda/linda.h"

int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("linda-server");
    parser.add_description("Hosts the global Linda tuple space.");
    parser.add_argument("-k", "--key-path")
        .help("path to be used as a key to identify the Linda Host queue")
        .default_value(std::string("/tmp/linda.key"));
    parser.add_argument("-p", "--project-id")
        .help("project ID to be used along with --key-path to identify the Linda Host queue")
        .scan<'i', int>()
        .default_value((int) 1);
    parser.parse_args(argc, argv);

    TupleSpaceHost host;
    host.init(parser.get<std::string>("key-path").c_str(), parser.get<int>("project-id"));
    host.runServer();

    return 0;
}
