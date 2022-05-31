#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>

#include "linda/linda.h"

int main(int argc, char **argv) {
    spdlog::set_level(spdlog::level::level_enum::err);

    auto parser = argparse::ArgumentParser("linda-client");
    parser.add_description(
        "Executes a given Linda command on the global tuple space.");
    parser.add_argument("action").help("one of `input`, `read`, `output`");
    parser.add_argument("data")
        .help("tuple or template data for the action")
        .remaining();
    parser.add_argument("-t", "--timeout")
        .help("number of milliseconds to wait before returning an error")
        .scan<'i', int>()
        .default_value(10);
    parser.add_epilog(
        "\nAvailable actions:\n"
        "`input` \tconsumes the first tuple that matches template provided in "
        "<data>\n"
        "`read`  \treads (without consuming) the first tuple that matches "
        "template provided in <data>\n"
        "`output`\tadds to the tuple space a tuple provided in <data>\n");
    parser.parse_args(argc, argv);

    key_t key = ftok("./key.k", 2137);
    TupleSpace client;
    client.open(key);

    auto action = parser.get<std::string>("action");

    auto data_args = parser.get<std::vector<std::string>>("data");
    std::ostringstream data_stream;
    std::copy(data_args.begin(), data_args.end(),
              std::ostream_iterator<std::string>(data_stream, " "));
    auto data = data_stream.str();

    if (action == "input") {
        std::cout << TupleToString(client.input(data, parser.get<int>("timeout")).value()) << std::endl;
    } else if (action == "read") {
        std::cout << TupleToString(client.read(data, parser.get<int>("timeout")).value()) << std::endl;
    } else if (action == "output") {
        client.output(data);
    } else {
        std::cerr << "Unknown action: " << action << std::endl;
    }

    return 0;
}