#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include <argparse/argparse.hpp>
#include <spdlog/spdlog.h>

#include "linda/linda.h"
#include "spdlog/cfg/env.h"

int main(int argc, char **argv) {
    spdlog::set_level(spdlog::level::level_enum::err);
    spdlog::cfg::load_env_levels();


    auto parser = argparse::ArgumentParser("linda-client");
    parser.add_description(
        "Executes a given Linda command on the global tuple space.");
    parser.add_argument("action").help("one of `input`, `read`, `output`").required();
    parser.add_argument("data")
        .help("tuple or template data for the action")
        .remaining().required();
    parser.add_argument("-t", "--timeout")
        .help("number of seconds to wait before returning an error")
        .scan<'i', int>()
        .default_value(10);
    parser.add_argument("-k", "--key-path")
            .help("path to be used as a key to identify the Linda Host queue")
            .default_value(std::string(DEFAULT_KEY_FILE_PATH));
    parser.add_argument("-p", "--project-id")
            .help("project ID to be used along with --key-path to identify the Linda Host queue")
            .scan<'i', int>()
            .default_value( DEFAULT_PROJECT_ID);
    parser.add_epilog(
        "\nAvailable actions:\n"
        "input \t-\tconsumes the first tuple that matches template provided in "
        "<data>\n"
        "read  \t-\treads (without consuming) the first tuple that matches "
        "template provided in <data>\n"
        "output\t-\tadds to the tuple space a tuple provided in <data>\n");

    try {
        parser.parse_args(argc, argv);
    } catch(std::runtime_error& e){
        std::cerr << "Error: " << e.what() << std::endl;
        std::cout << parser;
        return 1;
    }

    std::string action;
    std::vector<std::string> dataArgs;

    try {
        action = parser.get<std::string>("action");
        dataArgs = parser.get<std::vector<std::string>>("data");
    } catch(std::logic_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cout << parser;
        return 2;
    }

    TupleSpace client;
    client.open(parser.get<std::string>("key-path").c_str(), parser.get<int>("project-id"));



    std::ostringstream data_stream;
    std::copy(dataArgs.begin(), dataArgs.end(),
              std::ostream_iterator<std::string>(data_stream, " "));
    auto data = data_stream.str();

    if (action == "input" || action == "read") {
        auto timeout = parser.get<int>("timeout");
        std::optional<Tuple> result;

        try {
            result = action == "input"
                     ? client.input(data, timeout)
                     : client.read(data, timeout);
        } catch(std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        if (!result.has_value()) {
            return 3;
        }
        std::cout << tupleToString(result.value()) << std::endl;
    } else if (action == "output") {
        try {
            client.output(data);
        } catch(std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    } else {
        std::cerr << "Unknown action: " << action << std::endl;
    }

    return 0;
}