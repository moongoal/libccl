/**
 * @file
 *
 * Pair implementation.
 */
#ifndef CCL_PAIR_HPP
#define CCL_PAIR_HPP

#include <utility>
#include <ccl/api.hpp>

namespace ccl {
    template<typename T1, typename T2>
    struct pair {
        using first_type = std::decay_t<T1>;
        using second_type = std::decay_t<T2>;

        first_type first;
        second_type second;

        constexpr pair() = default;
        constexpr pair(const pair& other) : first{other.first}, second{other.second} {}
        constexpr pair(pair&& other) : first{std::move(other.first)}, second{std::move(other.second)} {}

        constexpr pair(const first_type& first, const second_type& second)
            : first{first},
            second{second}
        {}

        constexpr pair(first_type&& first, second_type&& second)
            : first{std::move(first)},
            second{std::move(second)}
        {}

        constexpr pair& operator=(const pair& other) {
            first = other.first;
            second = other.second;

            return *this;
        }

        constexpr pair& operator=(pair&& other) {
            first = std::move(other.first);
            second = std::move(other.second);

            return *this;
        }
    };

    template<typename T1, typename T2>
    static constexpr bool operator==(const pair<T1, T2>& a, const pair<T1, T2>& b) {
        return a.first == b.first && a.second == b.second;
    }

    template<typename T1, typename T2>
    constexpr pair<T1, T2> make_pair(T1&& first, T2&& second) {
        return pair<T1, T2>{ std::forward<T1>(first), std::forward<T2>(second) };
    }
}

#endif // CCL_PAIR_HPP
