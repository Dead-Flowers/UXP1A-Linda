cmake_minimum_required(VERSION 3.22)
project(linda)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb")

set(LINDA_INCLUDE "${PROJECT_SOURCE_DIR}/lib/linda/include")
set(PARSER_INCLUDE "${PROJECT_SOURCE_DIR}/lib/parser/include")



add_subdirectory (src)
add_subdirectory (lib)