/**
 * @file
 *
 * Character traits.
 */
#ifndef CCL_STRING_CHAR_TRAITS_HPP
#define CCL_STRING_CHAR_TRAITS_HPP

#include <cstdio>
#include <cstring>
#include <ccl/api.hpp>
#include <ccl/type-traits.hpp>
#include <ccl/util.hpp>

namespace ccl {
    template<typename CharType>
    struct char_traits;

    /**
     * Basic char traits.
     *
     * @see https://en.cppreference.com/w/cpp/named_req/CharTraits
     */
    template<typename CharType, typename IntType, typename PosType>
    struct base_char_traits {
        using char_type = CharType;
        using int_type = IntType;
        using pos_type = PosType;

        static constexpr int_type nul() noexcept {
            return '\0';
        }

        static constexpr void assign(char_type& r, const char_type &a) noexcept {
            r = a;
        }

        static constexpr void assign(char_type * const r, const pos_type count, const char_type a) {
            for(pos_type i = 0; i < count; ++i) {
                r[i] = a;
            }
        }

        static constexpr bool eq(const char_type a, const char_type b) noexcept {
            return a == b;
        }

        static constexpr bool lt(const char_type a, const char_type b) noexcept {
            return a < b;
        }

        static constexpr char_type* move(char_type* const dest, const char_type* const src, const pos_type count) {
            if constexpr(std::is_trivial_v<char_type>) {
                ::memmove(dest, src, count);
            } else {
                for(pos_type i = count; i > 0; --i) {
                    assign(dest[i], src[i]);
                }

                assign(dest[0], src[0]);
            }

            return dest;
        }

        static constexpr char_type* copy(char_type* const dest, const char_type* const src, const pos_type count) {
            if constexpr(std::is_trivial_v<char_type>) {
                ::memcpy(dest, src, count);
            } else {
                for(pos_type i = 0; i < count; ++i) {
                    assign(dest[i], src[i]);
                }
            }

            return dest;
        }

        static constexpr int compare(const char_type* const s1, const char_type* const s2, const pos_type count) {
            bool full_equality = false;

            for(pos_type i = 0; i < count; ++i) {
                const char_type &c1 = s1[i];
                const char_type &c2 = s2[i];

                if(and_(full_equality, lt(c1, c2))) {
                    return -1;
                }

                full_equality &= eq(c1, c2);
            }

            return 1 - static_cast<int>(full_equality);
        }

        static constexpr std::size_t length(const char_type* s) {
            pos_type i = 0;

            for(; !eq(s[i], char_type{}); ++i);

            return i;
        }

        static constexpr const char_type* find(const char_type* const ptr, const pos_type count, const char_type& ch) {
            for(pos_type i = 0; i < count; ++i) {
                const char_type &cur = &ptr[count];

                if(eq(cur, ch) == 0) {
                    return &cur;
                }
            }

            return nullptr;
        }

        static constexpr char_type to_char_type(const int_type c) noexcept {
            return static_cast<char_type>(c);
        }

        static constexpr int_type to_int_type(const char_type c) noexcept {
            return static_cast<int_type>(c);
        }

        static constexpr bool eq_int_type(const int_type c1, const int_type c2) noexcept {
            return c1 == c2;
        }

        static constexpr int_type eof() noexcept {
            return EOF;
        }

        static constexpr int_type not_eof(const int_type e) noexcept {
            return choose(
                e,
                EOF - 1,
                eq_int_type(e, eof())
            );
        }
    };

    template<>
    struct char_traits<char> : public base_char_traits<char, int, CCL_CHAR_TRAITS_DEFAULT_POS_TYPE> {
        using base = base_char_traits<char, int, CCL_CHAR_TRAITS_DEFAULT_POS_TYPE>;

        using typename base::char_type;
        using typename base::int_type;
        using typename base::pos_type;
    };
}

#endif // CCL_STRING_CHAR_TRAITS_HPP
