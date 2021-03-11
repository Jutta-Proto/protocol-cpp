cmake_minimum_required(VERSION 3.16)

add_library(jutta_proto STATIC CoffeeMaker.hpp
                         CoffeeMaker.cpp
                         JuttaCommands.hpp
                         JuttaConnection.hpp
                         JuttaConnection.cpp)

target_link_libraries(jutta_proto PRIVATE serial)
target_include_directories(jutta_proto PRIVATE .)

# Set version for shared libraries.
set_target_properties(jutta_proto
     PROPERTIES
     VERSION ${${PROJECT_NAME}_VERSION}
     SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR})
