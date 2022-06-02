#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "parser/Lexer.h"
#include "parser/PatternsParser.h"
#include "linda/tuple.h"
#include "parser/exceptions.h"

using Lexer = linda::modules::Lexer;
using PatternParser = linda::modules::PatternParser;

TEST_CASE("Test parsing integer pattern without asterisks") {
    auto patternInput = "(integer:>0)";
    Lexer lexer(patternInput);
    PatternParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 1);
    CHECK(res[0].type == TupleDataType::Integer);
    CHECK(res[0].op == TupleOperator::Greater);
    CHECK(std::get<int64_t>(res[0].value.value()) == 0);
}

TEST_CASE("Test parsing float pattern without asterisks") {
    auto patternInput = "(float:<5.5)";
    Lexer lexer(patternInput);
    PatternParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 1);
    CHECK(res[0].type == TupleDataType::Float);
    CHECK(res[0].op == TupleOperator::Less);
    CHECK(std::abs(std::get<float>(res[0].value.value()) - 5.5) < 1e-5);
}

TEST_CASE("Test parsing int as float pattern without asterisks") {
    auto patternInput = "(float:<5)";
    Lexer lexer(patternInput);
    PatternParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 1);
    CHECK(res[0].type == TupleDataType::Float);
    CHECK(res[0].op == TupleOperator::Less);
    CHECK(std::abs(std::get<float>(res[0].value.value()) - 5) < 1e-5);
}

TEST_CASE("Test parsing multiple patterns without asterisks") {
    auto patternInput = "(string:\"test\")";
    Lexer lexer(patternInput);
    PatternParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 1);
    CHECK(res[0].type == TupleDataType::String);
    CHECK(res[0].op == TupleOperator::Equal);
    CHECK(std::get<std::string>(res[0].value.value()) == "test");
}

TEST_CASE("Test parsing greater equal operator") {
    auto patternInput = "(integer:>=5)";
    Lexer lexer(patternInput);
    PatternParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 1);
    CHECK(res[0].type == TupleDataType::Integer);
    CHECK(res[0].op == TupleOperator::GreaterEqual);
    CHECK(std::get<int64_t>(res[0].value.value()) == 5);
}

TEST_CASE("Test parsing less operator") {
    auto patternInput = "(integer:<0)";
    Lexer lexer(patternInput);
    PatternParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 1);
    CHECK(res[0].type == TupleDataType::Integer);
    CHECK(res[0].op == TupleOperator::Less);
    CHECK(std::get<int64_t>(res[0].value.value()) == 0);
}

TEST_CASE("Test parsing less equal equal operator") {
    auto patternInput = "(integer:<=0)";
    Lexer lexer(patternInput);
    PatternParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 1);
    CHECK(res[0].type == TupleDataType::Integer);
    CHECK(res[0].op == TupleOperator::LessEqual);
    CHECK(std::get<int64_t>(res[0].value.value()) == 0);
}

TEST_CASE("Test parsing equal operator") {
    auto patternInput = "(integer:==0)";
    Lexer lexer(patternInput);
    PatternParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 1);
    CHECK(res[0].type == TupleDataType::Integer);
    CHECK(res[0].op == TupleOperator::Equal);
    CHECK(std::get<int64_t>(res[0].value.value()) == 0);
}

TEST_CASE("Test parsing not equal operator") {
    auto patternInput = "(integer:!=0)";
    Lexer lexer(patternInput);
    PatternParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 1);
    CHECK(res[0].type == TupleDataType::Integer);
    CHECK(res[0].op == TupleOperator::NotEqual);
    CHECK(std::get<int64_t>(res[0].value.value()) == 0);
}

TEST_CASE("Test parsing integer pattern with asterisks") {
    auto patternInput = "(integer:*)";
    Lexer lexer(patternInput);
    PatternParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 1);
    CHECK(res[0].type == TupleDataType::Integer);
    CHECK(res[0].op == TupleOperator::Equal);
    CHECK(res[0].value == std::nullopt);
}

TEST_CASE("Test parsing float pattern with asterisks") {
    auto patternInput = "(float:*)";
    Lexer lexer(patternInput);
    PatternParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 1);
    CHECK(res[0].type == TupleDataType::Float);
    CHECK(res[0].op == TupleOperator::Equal);
    CHECK(res[0].value == std::nullopt);
}

TEST_CASE("Test parsing multiple patterns with asterisks") {
    auto patternInput = "(string:*)";
    Lexer lexer(patternInput);
    PatternParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 1);
    CHECK(res[0].type == TupleDataType::String);
    CHECK(res[0].op == TupleOperator::Equal);
    CHECK(res[0].value == std::nullopt);
}

TEST_CASE("Test failed parsing pattern - lexer") {

    auto tuple = "(string:\"abc)";
    Lexer lexer(tuple);
    PatternParser p(lexer);
    CHECK_THROWS_AS(p.parse(), LexerParsingException);
}

TEST_CASE("Test failed parsing pattern - parser") {

    auto tuple = "string:\"abc\")";
    Lexer lexer(tuple);
    PatternParser p(lexer);
    CHECK_THROWS_AS(p.parse(), PatternParsingException);
}

TEST_CASE("Test failed parsing wildcard") {

    auto tuple = "(integer:>*)";
    Lexer lexer(tuple);
    PatternParser p(lexer);
    CHECK_THROWS_AS(p.parse(), PatternParsingException);
}

TEST_CASE("Test failed invalid combination") {

    auto tuple = "(string:123)";
    Lexer lexer(tuple);
    PatternParser p(lexer);
    CHECK_THROWS_AS(p.parse(), PatternParsingException);
}

TEST_CASE("Test parsing negative integer pattern") {
    auto patternInput = "(integer:<-5)";
    Lexer lexer(patternInput);
    PatternParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 1);
    CHECK(res[0].type == TupleDataType::Integer);
    CHECK(res[0].op == TupleOperator::Less);
    CHECK(std::get<int64_t>(res[0].value.value()) == -5);
}

TEST_CASE("Test parsing multiple patterns") {
    auto patternInput = "(integer:>0, float:<10.5, string:*)";
    Lexer lexer(patternInput);
    PatternParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 3);
    CHECK(res[0].type == TupleDataType::Integer);
    CHECK(res[0].op == TupleOperator::Greater);
    CHECK(std::get<int64_t>(res[0].value.value()) == 0);

    CHECK(res[1].type == TupleDataType::Float);
    CHECK(res[1].op == TupleOperator::Less);
    CHECK(std::abs(std::get<float>(res[1].value.value()) - 10.5) < 1e-5);

    CHECK(res[2].type == TupleDataType::String);
    CHECK(res[2].op == TupleOperator::Equal);
    CHECK(res[2].value == std::nullopt);
}

TEST_CASE("Test parsing overflow float pattern without asterisks") {
    auto patternInput = "(float:<5.5235345345345)";
    Lexer lexer(patternInput);
    PatternParser p(lexer);

    CHECK_THROWS_AS(p.parse(), LexerParsingException);
}

TEST_CASE("Test parsing overflow integer pattern without asterisks") {
    auto patternInput = "(integer:3454564523549876283463247583767234587247528937645982475)";
    Lexer lexer(patternInput);
    PatternParser p(lexer);

    CHECK_THROWS_AS(p.parse(), LexerParsingException);
}