/**
 * @file
 *
 * Exception handling.
 */
#ifndef CCL_EXCEPTIONS_HPP
#define CCL_EXCEPTIONS_HPP

#include <ccl/api.hpp>
#include <stdexcept>

namespace ccl {
    static constexpr bool exceptions_enabled = (
        #ifdef CCL_FEATURE_EXCEPTIONS
            true
        #else // CCL_FEATURE_EXCEPTIONS
            false
        #endif // CCL_FEATURE_EXCEPTIONS
    );
}

#endif // CCL_EXCEPTIONS_HPP
