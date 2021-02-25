cmake_minimum_required(VERSION 3.16)

include(Catch)

add_executable(tests Tests.cpp)

set_target_properties(tests PROPERTIES UNITY_BUILD OFF)
target_link_libraries(tests PRIVATE CONAN_PKG::catch2)

catch_discover_tests(tests)

