#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "parser/Lexer.h"
#include "parser/TupleParser.h"
#include "parser/PatternsParser.h"


using Lexer = linda::modules::Lexer;
using TupleParser = linda::modules::TupleParser;
using PatternParser = linda::modules::PatternParser;

TEST_CASE("Test parsing tuple with multiple elements") {

    auto tuple = "(123, 69.789, \"unixx\")";
    Lexer lexer(tuple);
    TupleParser p(lexer);
    auto res = p.parse();

    CHECK(res.size() == 3);
}

TEST_CASE("Test parsing multiple patterns without asterisks") {
    auto patternInput = "(integer:>0, float:*, string:\"abc\")";
}