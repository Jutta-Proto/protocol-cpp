cmake_minimum_required(VERSION 3.16)

include(Catch)

add_executable(proto_tests Tests.cpp)

set_target_properties(proto_tests PROPERTIES UNITY_BUILD OFF)
target_link_libraries(proto_tests PRIVATE catch2)

catch_discover_tests(proto_tests)

