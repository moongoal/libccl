cmake_minimum_required(VERSION 3.24)
project(Charcoal)

include(cmake/features.cmake)
enable_testing()

if(CCL_FEATURE_BUILD_SHARED)
    set(CCL_BUILD_MODE SHARED)
else()
    set(CCL_BUILD_MODE STATIC)
endif()

add_library(
    charcoal
    ${CCL_BUILD_MODE}
        src/allocator.cpp
)

target_compile_features(
    charcoal
    PUBLIC
        cxx_std_20
)

target_compile_definitions(
    charcoal
    PRIVATE
        CCL_IMPL
)

target_include_directories(
    charcoal
    PUBLIC
        include
        build/include
)

include(cmake/test.cmake)

configure_file(
    include/charcoal/features.hpp.in
    include/charcoal/features.hpp
    NEWLINE_STYLE UNIX
)
