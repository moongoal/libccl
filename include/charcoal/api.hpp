/**
 * @file
 *
 * Charcoal API definitions.
 */
#ifndef CCL_API_HPP
#define CCL_API_HPP

#include <cstdint>
#include <charcoal/features.hpp>

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

#endif // CCL_API_HPP
