/**
 * @file
 *
 * Either-or type selector.
 */
#ifndef CCL_EITHER_HPP
#define CCL_EITHER_HPP

#include <ccl/api.hpp>

namespace ccl {
    template<typename T1, typename T2, bool Predicate>
    struct either_or;

    template<typename T1, typename T2>
    struct either_or<T1, T2, true> {
        using type = T1;
    };

    template<typename T1, typename T2>
    struct either_or<T1, T2, false> {
        using type = T2;
    };

    template<typename T1, typename T2, bool Predicate>
    using either_or_v = typename either_or<T1, T2, Predicate>::type;
}

#endif // CCL_EITHER_HPP
