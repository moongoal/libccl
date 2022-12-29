/**
 * @file
 *
 * Ccl API definitions.
 */
#ifndef CCL_API_HPP
#define CCL_API_HPP

#include <cstdint>
#include <ccl/features.hpp>
#include <ccl/definitions.hpp>

#define CCLNODISCARD [[nodiscard]]
#define CCLUNUSED [[maybe_unused]]
#define CCLINLINE __attribute__((always_inline))

#ifdef _MSC_VER
    #define CCLZEROSIZE [[msvc::no_unique_address]]
#else // _MSC_VER
    #define CCLZEROSIZE [[no_unique_address]]
#endif // _MSC_VER

#endif // CCL_API_HPP
