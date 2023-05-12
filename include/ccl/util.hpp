/**
 * @file
 *
 * General utilities.
 */
#ifndef CCL_UTIL_HPP
#define CCL_UTIL_HPP

#include <cmath>
#include <utility>
#include <ccl/api.hpp>
#include <ccl/debug.hpp>
#include <ccl/concepts.hpp>
#include <ccl/exceptions.hpp>
#include <ccl/type-traits.hpp>
#include <ccl/either.hpp>

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
    template<integral_decaying T, integral_decaying U, typename W = std::common_type_t<std::decay_t<T>, std::decay_t<U>>>
    constexpr W choose(const T a, const U b, const bool cond) noexcept {
        using Z = either_or_t<W, unsigned, !is_boolean_v<W>>;

        const Z mask = (~static_cast<Z>(0)) & (static_cast<Z>(cond) - 1);
        const Z first = a & ~mask;
        const Z second = b & mask;

        return first | second;
    }

    template<typename T>
    constexpr T* choose(const T* a, const T* b, const bool cond) noexcept {
        const std::intptr_t mask = (~static_cast<std::intptr_t>(0)) & (static_cast<std::intptr_t>(cond) - 1);
        const std::intptr_t first = reinterpret_cast<std::intptr_t>(a) & ~mask;
        const std::intptr_t second = reinterpret_cast<std::intptr_t>(b) & mask;

        return reinterpret_cast<T*>(first | second);
    }

    template<typename T, typename U, typename Z = std::common_type_t<std::decay_t<T>, std::decay_t<U>>>
    constexpr Z choose(const T a, const U b, const bool cond) noexcept {
        const Z choices[2] { b, a };

        return choices[cond];
    }

    /**
     * Find the maximum among the arguments.
     *
     * @tparam FirstArg The type of the first argument.
     * @tparam Ts The type of all the other arguments.
     *
     * @param arg1 The first argument.
     * @param args The other arguments.
     *
     * @return The argument with the max value.
     */
    template<typename FirstArg, typename ...Ts>
    constexpr std::common_type_t<std::decay_t<FirstArg>, std::decay_t<Ts>...> max(FirstArg&& arg1, Ts&& ...args) noexcept {
        if constexpr(sizeof...(args) > 0) {
            const auto arg2 = max(std::forward<Ts>(args)...);

            return choose(
                arg1,
                arg2,
                arg1 > arg2
            );
        } else {
            return arg1;
        }
    }

    /**
     * Find the minimum among the arguments.
     *
     * @tparam FirstArg The type of the first argument.
     * @tparam Ts The type of all the other arguments.
     *
     * @param arg1 The first argument.
     * @param args The other arguments.
     *
     * @return The argument with the min value.
     */
    template<typename FirstArg, typename ...Ts>
    constexpr std::common_type_t<std::decay_t<FirstArg>, std::decay_t<Ts>...> min(FirstArg&& arg1, Ts&& ...args) noexcept {
        if constexpr(sizeof...(args) > 0) {
            const auto arg2 = max(std::forward<Ts>(args)...);

            return choose(
                arg1,
                arg2,
                arg1 < arg2
            );
        } else {
            return arg1;
        }
    }

    /**
     * Clamp a value.
     *
     * @param value The value to clamp.
     * @param low The lower bound.
     * @param high The upper bound.
     *
     * @return `low` if `value <= low`, `high` if `value >= high` or `value` for all other cases.
     */
    template<typename Value, typename Low, typename High>
    constexpr std::common_type_t<std::decay_t<Value>, std::decay_t<Low>, std::decay_t<High>> clamp(Value&& value, Low&& low, High&& high) noexcept {
        CCL_ASSERT(high >= low);

        return max(std::forward<Low>(low), min(std::forward<High>(high), std::forward<Value>(value)));
    }

    /**
     * Double capacity until it reaches or surpasses the value of
     * `threshold`.
     *
     * @tparam T The capacity type.
     *
     * @param capacity The initial capacity value.
     * @param threshold The minimum target capacity to reach. Must be >= capacity.
     *
     * @return The new capacity.
     */
    template<std::integral T>
    constexpr T increase_capacity(T capacity, const T threshold) {
        CCL_ASSERT(capacity >= 0);

        capacity = max(static_cast<T>(1), capacity);

        CCL_THROW_IF(!is_power_2(capacity), std::invalid_argument{"Capacity must be a power of two."});

        while(capacity < threshold) {
            capacity <<= 1;
        }

        return capacity;
    }

    /**
     * Increase capacity by the given size of the page until it reaches or
     * surpasses the value of `threshold`.
     *
     * @tparam T The capacity type.
     *
     * @param capacity The initial capacity value. Must be a multiple of page_size.
     * @param threshold The minimum target capacity to reach.
     * @param page_size The size of the page. Must be > 0 and a power of 2.
     *
     * @return The new capacity.
     */
    template<std::integral T>
    constexpr T increase_paged_capacity(T capacity, const T threshold, const T page_size) {
        CCL_ASSERT(capacity >= 0);
        CCL_ASSERT(page_size >= 0);

        CCL_THROW_IF(!is_power_2(page_size), std::invalid_argument{"Page size must be a power of two."});
        CCL_THROW_IF(capacity % page_size, std::invalid_argument{"Capacity must be a multiple of the page size."});

        while(capacity < threshold) {
            capacity += page_size;
        }

        return capacity;
    }

    /**
     * Count the number of places required to get to the highest
     * set bit in a give number.
     *
     * @tparam T The type of number to compute the bit count for.
     *
     * @param n The number to count the bits for.
     *
     * @return The total bit count of the value to the highest set bit.
     */
    template<std::integral T>
    constexpr std::size_t bitcount(T n) noexcept {
        std::size_t count = 0;

        while(n) {
            n >>= 1;
            ++count;
        }

        return count;
    }

    /**
     * An empty type.
     */
    struct empty {};

    /**
     * Unpack the first type of a pack.
     */
    template<typename First, typename ...Rest>
    struct first_type {
        using type = First;
    };

    /**
     * Unpack the first type of a pack.
     */
    template<typename ...Ts>
    using first_type_t = typename first_type<Ts...>::type;

    /**
     * Check whether a given type is in a pack of types.
     *
     * @tparam T The type to check.
     * @tparam First The first type in the pack.
     * @tparam Rest The remaining types in the pack.
     *
     * @return True if T is one of the types in the pack, false otherwise.
     */
    template<typename T, typename First, typename ...Rest>
    consteval bool is_type_in_pack() {
        if constexpr(std::is_same_v<T, First>) {
            return true;
        }

        if constexpr(sizeof...(Rest) == 0) {
            return false;
        } else {
            return is_type_in_pack<T, Rest...>();
        }
    }

    /**
     * Align a size to some alignment constraint.
     *
     * @param orig_size The original size.
     * @param alignment The alignment constraint - must be a power of 2.
     *
     * @return The aligned size. Aligned sizes are always >= than `orig_size`.
     */
    constexpr std::size_t align_size(const std::size_t orig_size, const std::size_t alignment) noexcept {
        CCL_ASSERT(is_power_2(alignment));

        std::size_t const mask = alignment - 1;

        return (orig_size + mask) & ~mask;
    }

    /**
     * Align an address to some alignment constraint.
     *
     * @param orig_address The address to align.
     * @param alignment The alignment constraint - must be a power of 2.
     *
     * @return The aligned pointer. Aligned addresses are always >= than `orig_address`.
     */
    template<typename T>
    constexpr T* align_address(const T* const orig_address, const uintptr_t alignment) noexcept {
        CCL_ASSERT(is_power_2(alignment));

        uintptr_t const int_addr = (uintptr_t)orig_address;
        uintptr_t const mask = alignment - 1;

        return (T*)((int_addr + mask) & ~mask);
    }

    /**
     * Return the size in bytes of one or more objects of the given type.
     *
     * @tparam T The type of the object.
     * @param n The number of objects.
     *
     * @return The size in bytes of a hypothetical array of `n` objects of type `T`.
     */
    template<typename T>
    constexpr std::size_t size_of(const std::size_t n = 1) noexcept {
        return sizeof(T) * n;
    }

    template<typename T>
    static constexpr T* null_v = nullptr;

    /**
     * Resolves to a typed null pointer value container.
     *
     * @tparam The null pointer type.
     */
    template<typename T>
    struct null_t {
        using pointer = T*;

        static constexpr pointer value = nullptr;
    };

    /**
     * Resolves to a typed null pointer.
     *
     * @tparam The null pointer type.
     */
    template<typename T>
    static constexpr T* null = null_t<T>::value;

    /**
     * Test whether a given pointer is aligned.
     *
     * @tparam T The value type.
     * @tparam Alignment The alignment of the value.
     *
     * @param ptr The pointer to check for alignment.
     *
     * @return True if the pointer is aligned on the given boundary, false if not.
     */
    template<typename T, std::size_t Alignment = alignof(T)>
    constexpr bool is_address_aligned(const T * const ptr) noexcept {
        return ptr == align_address(ptr, Alignment);
    }

    /**
     * Swap two values.
     *
     * @tparam T The type of the variables to swap.
     *
     * @param a The first variable to swap.
     * @param b The second variable to swap.
     */
    template<typename T>
    constexpr void swap(T& a, T& b) noexcept {
        const T temp = a;

        a = b;
        b = temp;
    }
}

#endif // CCL_UTIL_HPP
