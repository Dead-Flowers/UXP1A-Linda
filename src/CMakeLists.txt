FetchContent_Declare(argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse
    GIT_TAG v2.5)
FetchContent_MakeAvailable(argparse)

add_executable(linda-client linda-client/main.cpp)
target_link_libraries(linda-client linda parser argparse)
target_include_directories(linda-client PUBLIC ${LINDA_INCLUDE})
target_include_directories(linda-client PUBLIC ${PARSER_INCLUDE})

add_executable(linda-server linda-server/main.cpp)
target_link_libraries(linda-server linda argparse)
target_include_directories(linda-server PUBLIC ${LINDA_INCLUDE})