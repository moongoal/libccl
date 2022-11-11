/**
 * @file
 *
 * General utilities.
 */
#ifndef CCL_UTIL_HPP
#define CCL_UTIL_HPP

#include <cassert>
#include <concepts>
#include <charcoal/api.hpp>

namespace ccl {
    /**
     * Increase capacity until it reaches or surpasses the value of
     * `threshold`.
     *
     * @tparam T The capacity type.
     *
     * @param capacity The initial capacity value. Must be > 0 and a power of 2.
     * @param threshold The minimum target capacity to reach. Can be any value.
     *
     * @return The new capacity.
     */
    template<typename T>
    requires std::integral<T>
    constexpr T increase_capacity(T capacity, const T threshold) {
        assert(capacity > 0);

        // TODO: Add exception to ensure capacity is a power of 2

        while(capacity < threshold) {
            capacity <<= 1;
        }

        return capacity;
    }
}

#endif // CCL_UTIL_HPP
