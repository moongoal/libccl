/**
 * @file
 *
 * Ccl API definitions.
 */
#ifndef CCL_API_HPP
#define CCL_API_HPP

#include <cstdint>
#include <ccl/features.hpp>

#ifdef CCL_FEATURE_BUILD_SHARED
    #ifdef CCL_IMPL
        #define CCLAPI __attribute__((dllexport))
    #else
        #define CCLAPI __attribute__((dllimport))
    #endif // CCL_IMPL
    #else // CCL_FEATURE_BUILD_SHARED
#endif // CCL_FEATURE_BUILD_SHARED

#define CCLAPI
#define CCLNODISCARD [[nodiscard]]

#ifdef _MSC_VER
    #define CCLZEROSIZE [[msvc::no_unique_address]]
#else // _MSC_VER
    #define CCLZEROSIZE [[no_unique_address]]
#endif // _MSC_VER

#endif // CCL_API_HPP
