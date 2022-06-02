file(GLOB LINDA_SOURCES
        include/*.h
        src/*.cpp)

file(GLOB LINDA_UNIT_TEST_SOURCES
        test/*.cpp)

add_library(linda ${LINDA_SOURCES})

FetchContent_Declare(spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.x)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.8)
FetchContent_MakeAvailable(doctest)


target_include_directories(linda PRIVATE ${LINDA_INCLUDE} ${PARSER_INCLUDE})
target_link_libraries(linda parser spdlog)

add_executable(lindaUnitTests ${LINDA_UNIT_TEST_SOURCES})
target_link_libraries(lindaUnitTests linda doctest)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(lindaUnitTests)