cmake_minimum_required(VERSION 3.24)
project(Charcoal)

enable_testing()

add_library(
    libcharcoal
        STATIC
        src/allocator.cpp
)

target_compile_features(
    libcharcoal
    PUBLIC
        cxx_std_20
)

target_compile_definitions(
    libcharcoal
    PRIVATE
        CCL_IMPL
)

target_include_directories(libcharcoal PUBLIC include)

include(cmake/test.cmake)
