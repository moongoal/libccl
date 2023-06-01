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
#define CCLUNLIKELY [[unlikely]]
#define CCLLIKELY [[likely]]

#ifdef _MSC_VER
    #define CCLZEROSIZE [[msvc::no_unique_address]]
#else // _MSC_VER
    #define CCLZEROSIZE [[no_unique_address]]
#endif // _MSC_VER

namespace ccl {
    /**
     * A raw, numeric handle type.
     */
    using handle_t = uint32_t;
}

#endif // CCL_API_HPP
