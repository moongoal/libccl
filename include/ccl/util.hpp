/**
 * @file
 *
 * General utilities.
 */
#ifndef CCL_UTIL_HPP
#define CCL_UTIL_HPP

#include <cmath>
#include <ccl/api.hpp>
#include <ccl/debug.hpp>
#include <ccl/concepts.hpp>
#include <ccl/exceptions.hpp>

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
     * Choose between two values.
     *
     * @param a The first value.
     * @param b The second value.
     * @param cond The condition.
     *
     * @return The value of `a` if `cond` evaluates to true or that of `b` if it evaluates to false.
     */
    template<typename T>
    constexpr T choose(const T a, const T b, const bool cond) noexcept {
        const T choices[2] { b, a };

        return choices[cond];
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
        CCL_ASSERT(capacity >= 0);

        capacity = choose<T>(1, capacity, capacity == 0);

        CCL_THROW_IF(!is_power_2(capacity), std::invalid_argument{"Capacity must be a power of two."});

        while(capacity < threshold) {
            capacity <<= 1;
        }

        return capacity;
    }
}

#endif // CCL_UTIL_HPP
