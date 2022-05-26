add_executable(lindaTestClient test-client/main.cpp)
target_link_libraries(lindaTestClient linda parser)
target_include_directories(lindaTestClient PUBLIC ${LINDA_INCLUDE})
target_include_directories(lindaTestClient PUBLIC ${PARSER_INCLUDE})

add_executable(lindaTestHost test-host/main.cpp)
target_link_libraries(lindaTestHost linda)
target_include_directories(lindaTestHost PUBLIC ${LINDA_INCLUDE})