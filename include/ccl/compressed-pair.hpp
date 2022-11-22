/**
 * @file
 *
 * Compressed pair implementation.
 */
#ifndef CCL_COMPRESSED_PAIR_HPP
#define CCL_COMPRESSED_PAIR_HPP

#include <type_traits>
#include <ccl/api.hpp>

namespace ccl {
    template<typename T>
    struct compressed_pair_first_entry {
        CCLZEROSIZE T first;
    };

    template<typename T>
    struct compressed_pair_second_entry {
        CCLZEROSIZE T second;
    };

    template<typename T1, typename T2>
    struct compressed_pair : public compressed_pair_first_entry<T1>, public compressed_pair_second_entry<T2>{
        using first_type = T1;
        using second_type = T2;

        constexpr compressed_pair() = default;
        constexpr compressed_pair(const T1 &x, const T2 &y) : compressed_pair_first_entry<T1>{x}, compressed_pair_second_entry<T2>{y} {}

        template<typename U1 = T1, typename U2 = T2>
        constexpr compressed_pair(U1&& x, U2&& y) : compressed_pair_first_entry<T1>{std::move(x)}, compressed_pair_second_entry<T2>{std::move(y)} {}

        template<typename U1 = T1, typename U2 = T2>
        constexpr compressed_pair(const compressed_pair<U1, U2>& p) : compressed_pair_first_entry<T1>{p.first}, compressed_pair_second_entry<T2>{p.second} {}

        template<typename U1 = T1, typename U2 = T2>
        constexpr compressed_pair(compressed_pair<U1, U2>&& p) : compressed_pair_first_entry<T1>{std::move(p.first)}, compressed_pair_second_entry<T2>{std::move(p.second)} {}
    };
}

#endif // CCL_COMPRESSED_PAIR_HPP
