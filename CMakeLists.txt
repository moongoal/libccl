cmake_minimum_required(VERSION 3.24)
project(ccl)

include(cmake/features.cmake)
enable_testing()

if(CCL_FEATURE_BUILD_SHARED)
    set(CCL_BUILD_MODE SHARED)
else()
    set(CCL_BUILD_MODE STATIC)
endif()

add_library(
    ccl
    ${CCL_BUILD_MODE}
        src/allocator.cpp
)

target_compile_features(
    ccl
    PUBLIC
        cxx_std_20
)

target_compile_definitions(
    ccl
    PRIVATE
        CCL_IMPL
)

target_include_directories(
    ccl
    PUBLIC
        include
        build/include
)

include(cmake/test.cmake)

configure_file(
    include/ccl/features.hpp.in
    include/ccl/features.hpp
    NEWLINE_STYLE UNIX
)
