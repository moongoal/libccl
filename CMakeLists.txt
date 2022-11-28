cmake_minimum_required(VERSION 3.23)
project(ccl)

include(cmake/features.cmake)
include(cmake/definitions.cmake)
include(cmake/build-xxhash.cmake)
enable_testing()

if(CCL_FEATURE_BUILD_SHARED)
    set(CCL_BUILD_MODE SHARED)
else()
    set(CCL_BUILD_MODE STATIC)
endif()

if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(${CMAKE_BUILD_TYPE} "Debug")
endif()

add_library(
    ccl
    ${CCL_BUILD_MODE}
        src/allocator.cpp
)

target_compile_options(
    ccl
    PUBLIC
        -Wall -Wextra -pedantic -Werror
        $<$<BOOL:${CCL_FEATURE_SANITIZE_MEMORY}>:-fsanitize=memory>
        $<$<BOOL:${CCL_FEATURE_SANITIZE_ADDRESS}>:-fsanitize=address>
        $<$<BOOL:${CCL_FEATURE_SANITIZE_ADDRESS}>:-fno-omit-frame-pointer>
        $<$<BOOL:${CCL_FEATURE_SANITIZE_ADDRESS}>:-fsanitize-recover=address>
        $<$<BOOL:${CCL_FEATURE_SANITIZE_UNDEFINED_BEHAVIOR}>:-fsanitize=undefined>
        $<$<BOOL:${CCL_FEATURE_SANITIZE_STACK}>:-fsanitize=safe-stack>
)

target_link_options(
    ccl
    PUBLIC
        $<$<BOOL:${CCL_FEATURE_SANITIZE_MEMORY}>:-fsanitize=memory>
        $<$<BOOL:${CCL_FEATURE_SANITIZE_ADDRESS}>:-fsanitize=address>
        $<$<BOOL:${CCL_FEATURE_SANITIZE_UNDEFINED_BEHAVIOR}>:-fsanitize=undefined>
        $<$<BOOL:${CCL_FEATURE_SANITIZE_STACK}>:-fsanitize=safe-stack>
)

target_compile_features(
    ccl
    PUBLIC
        cxx_std_23
)

target_compile_definitions(
    ccl
    PRIVATE
        CCL_IMPL
    PUBLIC
        ${CCL_XXH_PUBLIC_COMPILE_DEFINITIONS}
)

target_compile_options(
    ccl
    PUBLIC
        -march=x86-64
)

target_link_options(
    ccl
    PUBLIC
        -march=x86-64
)

target_include_directories(
    ccl
    PUBLIC
        include
        build/include
)

target_link_libraries(
    ccl
    PUBLIC
        xxHash::xxhash
)

include(cmake/test.cmake)

configure_file(
    include/ccl/features.hpp.in
    include/ccl/features.hpp
    NEWLINE_STYLE UNIX
)

configure_file(
    include/ccl/definitions.hpp.in
    include/ccl/definitions.hpp
    NEWLINE_STYLE UNIX
)
