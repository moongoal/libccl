include_guard()

set(XXH_INLINE_ALL OFF)
set(XXH_VECTOR XXH_SSE2)
set(XXH_NO_STREAM ON)
set(XXH_SIZE_OPT 0)
set(XXH_NO_STDLIB ON)
set(XXH_CPU_LITTLE_ENDIAN ON)

set(
    CCL_XXH_PUBLIC_COMPILE_DEFINITIONS
        -DXXH_VECTOR=XXH_SSE2
        -DXXH_NO_STREAM
        -DXXH_SIZE_OPT=0
        -DXXH_NO_STDLIB
        -DXXH_CPU_LITTLE_ENDIAN
)

add_subdirectory(contrib/xxhash/cmake_unofficial xxhash)
