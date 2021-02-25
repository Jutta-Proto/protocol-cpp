cmake_minimum_required(VERSION 3.16)

add_library(jutta SHARED CoffeeMaker.hpp
                         CoffeeMaker.cpp
                         JuttaCommands.hpp
                         JuttaConnection.hpp
                         JuttaConnection.cpp)

target_link_libraries(jutta PUBLIC serial)

# Set version for shared libraries.
set_target_properties(jutta
     PROPERTIES
     VERSION ${${PROJECT_NAME}_VERSION}
     SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR})
