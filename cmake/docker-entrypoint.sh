#!/bin/sh

export ASAN_OPTIONS="halt_on_error=false"

cmake \
    -S . \
    -B build \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER=$(which clang++) \
    -DCMAKE_C_COMPILER=$(which clang) \
    # -DCCL_FEATURE_SANITIZE_MEMORY:BOOL=ON \
    || exit 1

cmake --build build || exit 2
ctest --test-dir build --output-on-failure --stop-on-failure || exit 3
