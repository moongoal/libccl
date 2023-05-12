cmake_minimum_required(VERSION 3.25)

include(version.cmake)

project(
    ccl
    VERSION ${CCL_VERSION_MAJOR}.${CCL_VERSION_MINOR}.${CCL_VERSION_PATCH}
)

include(CMakePackageConfigHelpers)

enable_testing()

option(CCL_ENABLE_TESTS "Enable unit tests" ON)

add_library(ccl INTERFACE)

target_compile_features(
    ccl
    INTERFACE
        cxx_std_23
)

set(CCL_INCLUDE_PATH include)
set(CCL_FULL_BUILD_INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/include)

file(REAL_PATH ${CCL_INCLUDE_PATH} CCL_FULL_INCLUDE_PATH)
file(RELATIVE_PATH CCL_BUILD_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CCL_FULL_BUILD_INCLUDE_PATH})

target_include_directories(
    ccl
    INTERFACE
        $<INSTALL_INTERFACE:${CCL_INCLUDE_PATH}> $<BUILD_INTERFACE:${CCL_FULL_INCLUDE_PATH}>
        $<INSTALL_INTERFACE:${CCL_BUILD_INCLUDE_PATH}> $<BUILD_INTERFACE:${CCL_FULL_BUILD_INCLUDE_PATH}>
)

if(CCL_ENABLE_TESTS)
    include(cmake/test.cmake)
endif()

configure_file(
    include/ccl/version.hpp.in
    include/ccl/version.hpp
    NEWLINE_STYLE UNIX
)

install(
    TARGETS ccl
    EXPORT ccl
    PUBLIC_HEADER
)

file(
    GLOB_RECURSE
    CCL_HEADER_FILES
    LIST_DIRECTORIES false
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    include/*.hpp
)

install(
    DIRECTORY
        include/
        ${CMAKE_CURRENT_BINARY_DIR}/include/
    TYPE INCLUDE
    FILES_MATCHING
    PATTERN *.hpp
)
