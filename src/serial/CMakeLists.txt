cmake_minimum_required(VERSION 3.16)

add_library(serial STATIC SerialConnection.hpp
                          SerialConnection.cpp)
