#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "parser/Lexer.h"
#include "parser/TupleParser.h"
#include "parser/exceptions.h"


using Lexer = linda::modules::Lexer;
using TupleParser = linda::modules::TupleParser;

TEST_CASE("Test parsing tuple with int only") {

    auto tuple = "(123)";
    Lexer lexer(tuple);
    TupleParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 1);
    CHECK(std::get<int64_t>(res[0]) == 123);
}

TEST_CASE("Test parsing tuple with float only") {

    auto tuple = "(69.789)";
    Lexer lexer(tuple);
    TupleParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 1);
    CHECK(std::abs(std::get<float>(res[0]) - 69.789) < 1e-5);
}

TEST_CASE("Test parsing tuple with string only") {

    auto tuple = "(\"unixx\")";
    Lexer lexer(tuple);
    TupleParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 1);
    CHECK(std::get<std::string>(res[0]) == "unixx");
}

TEST_CASE("Test parsing tuple with multiple elements") {

    auto tuple = "(123, 69.789, \"unixx\")";
    Lexer lexer(tuple);
    TupleParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 3);
    CHECK(std::get<int64_t>(res[0]) == 123);
    CHECK(std::abs(std::get<float>(res[1]) - 69.789) < 1e-5);
    CHECK(std::get<std::string>(res[2]) == "unixx");
}

TEST_CASE("Test failed parsing tuple - lexer") {

    auto tuple = "(\"abc)";
    Lexer lexer(tuple);
    TupleParser p(lexer);
    CHECK_THROWS_AS(p.parse(), LexerParsingException);
}

TEST_CASE("Test failed parsing tuple") {

    auto tuple = "\"abc\")";
    Lexer lexer(tuple);
    TupleParser p(lexer);
    CHECK_THROWS_AS(p.parse(), TupleParsingException);
}
