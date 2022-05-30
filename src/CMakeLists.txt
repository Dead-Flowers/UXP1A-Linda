FetchContent_Declare(argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse
    GIT_TAG v2.5)
FetchContent_MakeAvailable(argparse)

add_executable(lindaTestClient test-client/main.cpp)
target_link_libraries(lindaTestClient linda parser argparse)
target_include_directories(lindaTestClient PUBLIC ${LINDA_INCLUDE})
target_include_directories(lindaTestClient PUBLIC ${PARSER_INCLUDE})

add_executable(lindaTestHost test-host/main.cpp)
target_link_libraries(lindaTestHost linda argparse)
target_include_directories(lindaTestHost PUBLIC ${LINDA_INCLUDE})