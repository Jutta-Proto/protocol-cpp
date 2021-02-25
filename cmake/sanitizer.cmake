include(CheckCXXCompilerFlag)
include(CheckCXXSourceRuns)

set(ALLOWED_BUILD_TYPES Debug Release RelWithDebInfo MinSizeRel)
set(ALLSAN_FLAGS "")

# ThreadSanitizer
set(THREAD_SAN_FLAGS "-fsanitize=thread")
set(PREV_FLAG ${CMAKE_REQUIRED_FLAGS})
set(CMAKE_REQUIRED_FLAGS "${THREAD_SAN_FLAGS}")
check_cxx_source_runs("int main() { return 0; }" THREAD_SANITIZER_AVAILABLE)
set(CMAKE_REQUIRED_FLAGS ${PREV_FLAG})
if(THREAD_SANITIZER_AVAILABLE)
    list(APPEND ALLOWED_BUILD_TYPES ThreadSan)
    # Do not add Thread Sanitizer to all Sanitizers because it is incompatible with other Sanitizers
endif()
set(CMAKE_C_FLAGS_THREADSAN "${CMAKE_C_FLAGS_DEBUG} ${THREAD_SAN_FLAGS}" CACHE INTERNAL "Flags used by the C compiler during Thread Sanitizer builds." FORCE)
set(CMAKE_CXX_FLAGS_THREADSAN "${CMAKE_CXX_FLAGS_DEBUG} ${THREAD_SAN_FLAGS}" CACHE INTERNAL "Flags used by the C++ compiler during Thread Sanitizer builds." FORCE)

# AddressSanitizer
set(ADDR_SAN_FLAGS "-fsanitize=address")
set(PREV_FLAG ${CMAKE_REQUIRED_FLAGS})
set(CMAKE_REQUIRED_FLAGS "${ADDR_SAN_FLAGS}")
check_cxx_source_runs("int main() { return 0; }" ADDRESS_SANITIZER_AVAILABLE)
set(CMAKE_REQUIRED_FLAGS ${PREV_FLAG})
if(ADDRESS_SANITIZER_AVAILABLE)
    list(APPEND ALLOWED_BUILD_TYPES AddrSan)
    set(ALLSAN_FLAGS "${ALLSAN_FLAGS} ${ADDR_SAN_FLAGS}")
endif()
set(CMAKE_C_FLAGS_ADDRSAN "${CMAKE_C_FLAGS_DEBUG} ${ADDR_SAN_FLAGS} -fno-omit-frame-pointer -fno-optimize-sibling-calls" CACHE INTERNAL "Flags used by the C compiler during AddressSanitizer builds." FORCE)
set(CMAKE_CXX_FLAGS_ADDRSAN "${CMAKE_CXX_FLAGS_DEBUG} ${ADDR_SAN_FLAGS} -fno-omit-frame-pointer -fno-optimize-sibling-calls" CACHE INTERNAL "Flags used by the C++ compiler during AddressSanitizer builds." FORCE)

# LeakSanitizer
set(LEAK_SAN_FLAGS "-fsanitize=leak")
check_cxx_compiler_flag(${LEAK_SAN_FLAGS} LEAK_SANITIZER_AVAILABLE)
if(LEAK_SANITIZER_AVAILABLE)
    list(APPEND ALLOWED_BUILD_TYPES LeakSan)
    set(ALLSAN_FLAGS "${ALLSAN_FLAGS} ${LEAK_SAN_FLAGS}")
endif()
set(CMAKE_C_FLAGS_LEAKSAN "${CMAKE_C_FLAGS_DEBUG} ${LEAK_SAN_FLAGS} -fno-omit-frame-pointer" CACHE INTERNAL "Flags used by the C compiler during LeakSanitizer builds." FORCE)
set(CMAKE_CXX_FLAGS_LEAKSAN "${CMAKE_CXX_FLAGS_DEBUG} ${LEAK_SAN_FLAGS} -fno-omit-frame-pointer" CACHE INTERNAL "Flags used by the C++ compiler during LeakSanitizer builds." FORCE)

# UndefinedBehaviour
set(UDEF_SAN_FLAGS "-fsanitize=undefined")
check_cxx_compiler_flag(${UDEF_SAN_FLAGS} UNDEFINED_BEHAVIOUR_SANITIZER_AVAILABLE)
if(UNDEFINED_BEHAVIOUR_SANITIZER_AVAILABLE)
    list(APPEND ALLOWED_BUILD_TYPES UdefSan)
    set(ALLSAN_FLAGS "${ALLSAN_FLAGS} ${UDEF_SAN_FLAGS}")
endif()
set(CMAKE_C_FLAGS_UDEFSAN "${CMAKE_C_FLAGS_DEBUG} ${UDEF_SAN_FLAGS}" CACHE INTERNAL "Flags used by the C compiler during Undefined_BehaviourSanitizer builds." FORCE)
set(CMAKE_CXX_FLAGS_UDEFSAN "${CMAKE_CXX_FLAGS_DEBUG} ${UDEF_SAN_FLAGS}" CACHE INTERNAL "Flags used by the C++ compiler during Undefined_BehaviourSanitizer builds." FORCE)

# AllSanetizer
if(NOT ALLSAN_FLAGS STREQUAL "")
    set(PREV_FLAG ${CMAKE_REQUIRED_FLAGS})
    set(CMAKE_REQUIRED_FLAGS "${ALLSAN_FLAGS}")
    check_cxx_source_runs("int main() { return 0; }" ALL_SANITIZERS_AVAILABLE)
    set(CMAKE_REQUIRED_FLAGS ${PREV_FLAG})
    if(ALL_SANITIZERS_AVAILABLE)
        list(APPEND ALLOWED_BUILD_TYPES AllSan)
    endif()
endif()

set(CMAKE_C_FLAGS_ALLSAN "${CMAKE_C_FLAGS_DEBUG} ${ALLSAN_FLAGS} -fno-omit-frame-pointer -fno-optimize-sibling-calls" CACHE INTERNAL "Flags used by the C compiler during All Possible Sanetizer builds." FORCE)
set(CMAKE_CXX_FLAGS_ALLSAN "${CMAKE_CXX_FLAGS_DEBUG} ${ALLSAN_FLAGS} -fno-omit-frame-pointer -fno-optimize-sibling-calls" CACHE INTERNAL "Flags used by the C++ compiler during All Possible Sanetizer builds." FORCE)
