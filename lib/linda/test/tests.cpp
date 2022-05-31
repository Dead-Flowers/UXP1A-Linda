#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "linda/linda.h"
#include <fstream>
#include <pthread.h>

static std::function<void()> runServer;

class TestService {
public:
    TestService();
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


TestService::TestService() : client(), host(){
    std::ofstream f {"test.k"};
    f.close();

    this->host.init(this->fileName.c_str(), 2137);

    key_t key = ftok(this->fileName.c_str(), 2137);
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
    TestService service;

    TEST_CASE("equal on types") {
        service.host.reset();
        service.client.output("(123, 456.789, \"unix\")");
        service.client.output("(420, 69.789, \"test\")");
        service.client.output("(123, 69.789, \"unix\")");
        service.client.output("(123, 69.789, \"unixx\")");
        service.client.output("(124, 69.789, \"unixx\")");
        sleep(1);
        CHECK(service.host.spaceSize() == 5);
        CHECK(service.host.contains(service.parsePattern("(integer:123, float:>456, string:\"unix\")")));
    }

    TEST_CASE("test read") {
        service.host.reset();
        service.client.output("(123, 456.789, \"unix\")");
        service.client.output("(420, 69.789, \"test\")");
        service.client.output("(123, 69.789, \"unix\")");
        service.client.output("(123, 69.789, \"unixx\")");
        service.client.output("(124, 69.789, \"unixx\")");
        sleep(1);
        CHECK(service.host.spaceSize() == 5);
        auto t = service.client.read("(integer:123, float:>456, string:\"unix\")", 3);
        CHECK(service.host.spaceSize() == 5);
        CHECK(t.has_value());
        auto b = t.value() == service.parseTuple("(123, 456.789, \"unix\")");
        CHECK(b);
        CHECK(service.host.contains(service.parsePattern("(integer:123, float:>456, string:\"unix\")")));
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







