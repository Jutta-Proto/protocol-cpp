cmake_minimum_required(VERSION 3.16)

set(PROJECT_DESCRIPTION "An unofficial serial (UART) protocol library implementation to control JURA coffee makers.")

project("Jutta Protocol Library" 
            VERSION 0.0.0
            DESCRIPTION "${PROJECT_DESCRIPTION}"
            HOMEPAGE_URL "https://github.com/Jutta-Proto/protocol-cpp")
set(VERSION_NAME "dev")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CMAKE_UNITY_BUILD "Enable unity build" ON)
option(ENABLE_LINTING "Enables linting while building" ON)
option(BUILD_TESTING "Build tests" ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(sanitizer)
include(gcc_analyze)

if (${ENABLE_LINTING})
    message(STATUS "Enabling linting")
    include(clang-tidy)
else()
    message(STATUS "Linting is disabled")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
endif()

get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (NOT isMultiConfig)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${ALLOWED_BUILD_TYPES}")
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
    elseif(NOT CMAKE_BUILD_TYPE IN_LIST ALLOWED_BUILD_TYPES)
        message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
    endif()
else ()
    unset(CMAKE_BUILD_TYPE)
    foreach(TYPE ${ALLOWED_BUILD_TYPES})
    if (NOT ${TYPE} IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES ${TYPE})
    endif()  
    endforeach()  
endif()

if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
                 "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

set(CONAN_CONFIGS"Release;Debug;RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE IN_LIST CONAN_CONFIGS)
    set(CONAN_BUILD_TYPE "Debug")
else()
    set(CONAN_BUILD_TYPE ${CMAKE_BUILD_TYPE})
endif()

conan_cmake_run(REQUIRES catch2/2.13.3
                         spdlog/1.8.0
                SETTINGS compiler.cppstd=${CMAKE_CXX_STANDARD}
                         os=${CMAKE_SYSTEM_NAME}
                OPTIONS  spdlog:header_only=False
                BUILD    missing
                BUILD_TYPE ${CONAN_BUILD_TYPE})

conan_basic_setup(TARGETS NO_OUTPUT_DIRS)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_subdirectory(src)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    if (CMAKE_FIND_ROOT_PATH)
        set(MINGW_ROOT ${CMAKE_FIND_ROOT_PATH})
    else()
        set(MINGW_ROOT ${CMAKE_INSTALL_PREFIX})
        message(STATUS "Guessing \"CMAKE_INSTALL_PREFIX\" as mingw root (${CMAKE_INSTALL_PREFIX})")
    endif()

    if (NOT SHARE_INSTALL_PREFIX)
        set(SHARE_INSTALL_PREFIX "${MINGW_ROOT}/share")
    endif()

    if (NOT CMAKE_INSTALL_LIBDIR)
        set(CMAKE_INSTALL_LIBDIR "${MINGW_ROOT}/lib")
    endif()
    install(DIRECTORY ${MINGW_ROOT}/bin/ DESTINATION bin
            FILES_MATCHING PATTERN "*.dll")
    install(FILES ${MINGW_ROOT}/bin/gdbus.exe DESTINATION bin)
endif()

set(CPACK_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_VENDOR "Jutta")
set(CPACK_PACKAGE_CONTACT "jutta@uwpx.org")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION}-${VERSION_NAME}")
# NSIS stuff
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/resources/papersearch.ico")
set(CPACK_NSIS_DISPLAY_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_NSIS_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
include (CPack)

# Testing
if(${BUILD_TESTING})
    message(STATUS "Testing is enabled")
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "Testing is disabled")
endif()

