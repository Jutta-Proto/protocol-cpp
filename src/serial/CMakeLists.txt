cmake_minimum_required(VERSION 3.16)

add_library(serial SHARED SerialConnection.cpp)

install(TARGETS serial)
