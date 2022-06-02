FetchContent_Declare(argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse
    GIT_TAG v2.5)
FetchContent_MakeAvailable(argparse)

add_executable(client linda-client/main.cpp)
target_link_libraries(client linda parser argparse)
target_include_directories(client PUBLIC ${LINDA_INCLUDE})
target_include_directories(client PUBLIC ${PARSER_INCLUDE})

add_executable(server linda-server/main.cpp)
target_link_libraries(server linda argparse)
target_include_directories(server PUBLIC ${LINDA_INCLUDE})