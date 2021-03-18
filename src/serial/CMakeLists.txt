cmake_minimum_required(VERSION 3.16)

add_library(serial SHARED SerialConnection.cpp)
target_link_libraries(serial PRIVATE logger)

install(TARGETS serial)
