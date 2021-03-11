cmake_minimum_required(VERSION 3.16)

add_library(jutta_proto SHARED CoffeeMaker.cpp
                               JuttaConnection.cpp)

target_link_libraries(jutta_proto PRIVATE serial)

# Set version for shared libraries.
set_target_properties(jutta_proto
     PROPERTIES
     VERSION ${${PROJECT_NAME}_VERSION}
     SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR})

install(TARGETS jutta_proto)
