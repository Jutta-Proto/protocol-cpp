cmake_minimum_required(VERSION 3.16)

target_include_directories(serial PUBLIC  
    $<INSTALL_INTERFACE:include>    
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_sources(serial PRIVATE
     # Header files (useful in IDEs)
     serial/SerialConnection.hpp)

target_include_directories(jutta_proto PUBLIC  
    $<INSTALL_INTERFACE:include>    
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_sources(jutta_proto PRIVATE
     # Header files (useful in IDEs)
    jutta_proto/CoffeeMaker.hpp
    jutta_proto/JuttaConnection.hpp
    jutta_proto/JuttaCommands.hpp)

install(DIRECTORY cpr DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})