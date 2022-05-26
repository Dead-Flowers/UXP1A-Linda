file(GLOB PARSER_SOURCES
        include/*.h
        src/*.cpp)

add_library(parser ${PARSER_SOURCES})
target_include_directories(parser PUBLIC ${LINDA_INCLUDE})
target_include_directories(parser PUBLIC ${PARSER_INCLUDE})

