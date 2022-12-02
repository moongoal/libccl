cmake_minimum_required(VERSION 3.23)

project(ccl)

set(CCL_CONAN_PATHS_FILE ${CMAKE_BINARY_DIR}/conan_paths.cmake)

include(cmake/features.cmake)
include(cmake/definitions.cmake)

if(EXISTS ${CCL_CONAN_PATHS_FILE})
    message(STATUS "Conan paths file found.")
    include(${CCL_CONAN_PATHS_FILE})
else()
    message(WARNING "Conan paths file NOT found.")
endif()

enable_testing()

if(NOT ${CMAKE_BUILD_TYPE} STREQUAL "")
    set(${CMAKE_BUILD_TYPE} "Debug")
endif()

find_package(xxHash REQUIRED)

add_library(
    ccl
    INTERFACE
)

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

target_link_libraries(
    ccl
    INTERFACE
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

install(
    TARGETS ccl
    EXPORT ccl
    ARCHIVE
    PUBLIC_HEADER
)

install(
    EXPORT ccl
    DESTINATION "$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}>"
)
