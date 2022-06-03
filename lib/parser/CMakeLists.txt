file(GLOB PARSER_SOURCES
        include/*.h
        src/*.cpp)

file(GLOB TUPLE_PARSER_UNIT_TEST_SOURCES
        test/tuple-parser-test.cpp)

file(GLOB PATTERN_PARSER_UNIT_TEST_SOURCES
        test/pattern-parser-test.cpp)

FetchContent_Declare(spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.x)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.8)
FetchContent_MakeAvailable(doctest)

add_library(parser ${PARSER_SOURCES} include/parser/consts.h)
target_include_directories(parser PUBLIC ${LINDA_INCLUDE})
target_include_directories(parser PUBLIC ${PARSER_INCLUDE})

target_link_libraries(parser spdlog)

add_executable(tupleParserUnitTests ${TUPLE_PARSER_UNIT_TEST_SOURCES} test/tuple-parser-test.cpp)
add_executable(patternParserUnitTests ${PATTERN_PARSER_UNIT_TEST_SOURCES} test/pattern-parser-test.cpp)
target_link_libraries(tupleParserUnitTests parser doctest)
target_link_libraries(patternParserUnitTests parser doctest)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(tupleParserUnitTests patternParserUnitTests)