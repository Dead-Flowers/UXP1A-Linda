file(GLOB PARSER_SOURCES
        include/*.h
        src/*.cpp)

file(GLOB PARSER_UNIT_TEST_SOURCES
        test/*.cpp)

FetchContent_Declare(spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.x)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.8)
FetchContent_MakeAvailable(doctest)

add_library(parser ${PARSER_SOURCES})
target_include_directories(parser PUBLIC ${LINDA_INCLUDE})
target_include_directories(parser PUBLIC ${PARSER_INCLUDE})

target_link_libraries(parser spdlog)

add_executable(parserUnitTests ${PARSER_UNIT_TEST_SOURCES})
target_link_libraries(parserUnitTests parser doctest)