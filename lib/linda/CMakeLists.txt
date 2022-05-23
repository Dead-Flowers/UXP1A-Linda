file(GLOB LINDA_SOURCES
        include/*.h
        src/*.cpp)

add_library(linda ${LINDA_SOURCES})
target_include_directories(linda PRIVATE ${LINDA_INCLUDE})