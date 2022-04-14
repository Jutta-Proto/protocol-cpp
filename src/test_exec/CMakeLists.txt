cmake_minimum_required(VERSION 3.16)

if(JUTTA_PROTO_BUILD_TEST_EXEC)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

    # Handshake Test:
    set(EXECUTABLE_NAME "handshake_test")
    set(EXECUTABLE_MAIN "handshake_test.cpp")

    add_executable(${EXECUTABLE_NAME} ${EXECUTABLE_MAIN})
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE logger jutta_proto)
    set_property(SOURCE ${EXECUTABLE_MAIN} PROPERTY COMPILE_DEFINITIONS)
endif()
