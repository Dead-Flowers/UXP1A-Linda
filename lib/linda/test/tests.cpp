#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "linda/linda.h"
#include <fstream>
#include <pthread.h>

static std::function<void()> runServer;

class TestService {
public:
    TestService(int projectId);
    ~TestService();
    void closeTest();
    TuplePattern parsePattern(const char* pattern);
    Tuple parseTuple(const char*);
    TupleSpaceHost host;
    TupleSpace client;
    pthread_t hostThread;
    std::string fileName;
};

void *threadFunc(void *ptr) {
    runServer();
};


TestService::TestService(int projectId) : client(), host(){
    std::ofstream f {"/tmp/test.k"};
    f.close();

    this->host.init(this->fileName.c_str(), projectId);

    key_t key = ftok(this->fileName.c_str(), projectId);
    this->client.open(key);

    runServer = [this](){this->host.runServer();};
    auto ret = pthread_create(&hostThread, NULL, threadFunc, NULL);

}

void TestService::closeTest() {
    this->host.close();
}

TuplePattern TestService::parsePattern(const char* pattern) {
    auto lexer = linda::modules::Lexer(pattern);
    auto parser = linda::modules::PatternParser(lexer);

    return parser.parse();
}

Tuple TestService::parseTuple(const char* pattern) {
    auto lexer = linda::modules::Lexer(pattern);
    auto parser = linda::modules::TupleParser(lexer);

    return parser.parse();
}

TestService::~TestService() {
    std::remove(this->fileName.c_str());
}

TEST_SUITE("TEST") {
    TestService service(123);

    TEST_CASE("equal on types") {
        std::cout << "start1";
        service.host.reset();
        service.client.output("(123, 456.789, \"unix\")");
        service.client.output("(420, 69.789, \"test\")");
        service.client.output("(123, 69.789, \"unix\")");
        service.client.output("(123, 69.789, \"unixx\")");
        service.client.output("(124, 69.789, \"unixx\")");
        sleep(1);
        CHECK(service.host.spaceSize() == 5);
        CHECK(service.host.contains(service.parsePattern("(integer:123, float:>456, string:\"unix\")")));
        std::cout << "end1";

    }

    TEST_CASE("test read") {
        std::cout << "start2";
        service.host.reset();
        service.client.output("(123, 456.789, \"unix\")");
        service.client.output("(420, 69.789, \"test\")");
        service.client.output("(123, 69.789, \"unix\")");
        service.client.output("(123, 69.789, \"unixx\")");
        service.client.output("(124, 69.789, \"unixx\")");
        sleep(1);
        std::cout << "end";
        CHECK(service.host.spaceSize() == 5);
        auto t = service.client.read("(integer:123, float:>456.0, string:\"unix\")", 300);
        CHECK(service.host.spaceSize() == 5);
        CHECK(t.has_value());
        auto b = t.value() == service.parseTuple("(123, 456.789, \"unix\")");
        CHECK(b);
        CHECK(service.host.contains(service.parsePattern("(integer:123, float:>456, string:\"unix\")")));
        std::cout << "end2";

    }

    TEST_CASE("test input") {
        service.host.reset();
        service.client.output("(123, 456.789, \"unix\")");
        service.client.output("(420, 69.789, \"test\")");
        service.client.output("(123, 69.789, \"unix\")");
        service.client.output("(123, 69.789, \"unixx\")");
        service.client.output("(124, 69.789, \"unixx\")");
        sleep(1);
        CHECK(service.host.spaceSize() == 5);
        auto t = service.client.input("(integer:123, float:>456, string:\"unix\")", 3);
        CHECK(service.host.spaceSize() == 4);
        CHECK(t.has_value());
        auto b = t.value() == service.parseTuple("(123, 456.789, \"unix\")");
        CHECK(b);
        CHECK(!service.host.contains(service.parsePattern("(integer:123, float:>456, string:\"unix\")")));
    }


    TEST_CASE("too big tuple") {
        service.host.reset();
        CHECK_THROWS(service.client.output("(123, 456.789, \"unix\", 123, 456.789, \"unix\", 123, 456.789, \"unix\", 123, 456.789, \"unix\",123, 456.789, \"unix\",123, 456.789, \"unix\",123, 456.789, \"unix\" )"));
    }

    TEST_CASE("equal on float") {
        service.host.reset();
        service.client.output("(123, 456.789, \"unix\")");
        CHECK(service.host.spaceSize() == 1);
        CHECK_THROWS(service.client.input("(integer:123, float:456, string:\"unix\")", 3));
    }

    TEST_CASE("incorrect syntax") {
        service.host.reset();
        CHECK_THROWS(service.client.output("(123, 456.789, "));
    }

    TEST_CASE("random text") {
        service.host.reset();
        CHECK_THROWS(service.client.output("fafaffafaffa "));
    }

    TEST_CASE("random chines letters") {
        service.host.reset();
        CHECK_THROWS(service.client.output("象形字象形字象形字象形字象形字象形字象形字象形字"));
    }

    TEST_CASE("chines letters in tuple") {
        service.host.reset();
        service.client.output("(123, \"象形字象形字象形字象形字象形字象形字象形字象形字\")");
        CHECK(service.host.spaceSize() == 1);
    }

    TEST_CASE("tuple not exists") {
        service.host.reset();
        service.client.output("(123, 456.789)");
        sleep(1);
        CHECK(service.host.spaceSize() == 1);
        CHECK(service.host.contains(service.parsePattern("(integer:*, float:*)")));

        auto t = service.client.read("(integer:123, float:>500)", 5);
        CHECK(!t.has_value());
    }

    TEST_CASE("test string") {
        service.host.reset();
        service.client.output("(\"aaaaaa\")");
        service.client.output("(\"abbbbb\")");
        service.client.output("(\"accccc\")");
        service.client.output("(\"aaabbbb\")");
        service.client.output("(\"aaabcccc\")");
        sleep(1);
        CHECK(service.host.spaceSize() == 5);
        auto t = service.client.read("(string:>\"aaa\")", 3);
        CHECK(t.has_value());
        auto b = t.value() == service.parseTuple("(\"aaaaaa\")");
        CHECK(b);

        t = service.client.read("(string:>\"bbb\")", 3);
        CHECK(!t.has_value());

        t = service.client.read("(string:>\"aaab\")", 3);
        CHECK(t.has_value());
        b = t.value() == service.parseTuple("(\"abbbbb\")");
        CHECK(b);
    }

    // last test, don't change order
    TEST_CASE("basic") {
        service.host.reset();
        service.client.output("(123, 456.789)");
        sleep(1);
        CHECK(service.host.spaceSize() == 1);
        CHECK(service.host.contains(service.parsePattern("(integer:*, float:*)")));
        service.closeTest();
    }

}







