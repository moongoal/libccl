/**
 * @file
 *
 * General utilities.
 */
#ifndef CCL_UTIL_HPP
#define CCL_UTIL_HPP

#include <cmath>
#include <charcoal/api.hpp>
#include <charcoal/debug.hpp>
#include <charcoal/concepts.hpp>
#include <charcoal/exceptions.hpp>

namespace ccl {
    /**
     * Check whether a number is a power of 2.
     *
     * @param n The integral value to check.
     *
     * @return True if the number is a power of two, false if not.
     */
    constexpr bool is_power_2(std::integral auto n) noexcept {
        CCL_ASSERT(n >= 0);

        return !(n & (n - 1));
    }

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
        CCL_ASSERT(capacity > 0);
        THROW_IF(!is_power_2(capacity), std::invalid_argument{"Capacity must be a power of two."});

        while(capacity < threshold) {
            capacity <<= 1;
        }

        return capacity;
    }
}

#endif // CCL_UTIL_HPP
