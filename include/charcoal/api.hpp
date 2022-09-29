/**
 * @file
 *
 * Charcoal API definitions.
 */
#ifndef CCL_API_HPP
#define CCL_API_HPP

#include <cstdint>

#ifdef CCL_IMPL
    #define CCLAPI __attribute__((dllexport))
#else
    #define CCLAPI __attribute__((dllimport))
#endif // CCL_IMPL

#define CCLNODISCARD [[nodiscard]]

#endif // CCL_API_HPP
