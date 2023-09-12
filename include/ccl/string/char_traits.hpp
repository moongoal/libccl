/**
 * @file
 *
 * Character traits.
 */
#ifndef CCL_STRING_CHAR_TRAITS_HPP
#define CCL_STRING_CHAR_TRAITS_HPP

#include <cstring>
#include <ccl/api.hpp>
#include <ccl/type-traits.hpp>

namespace ccl {
    template<typename CharType>
    struct char_traits;

    template<typename CharType, typename IntType, typename PosType>
    struct base_char_traits {
        using char_type = CharType;
        using int_type = IntType;
        using pos_type = PosType;

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
                    dest[i] = src[i];
                }

                dest[0] = src[0];
            }

            return dest;
        }

        static constexpr char_type* copy(char_type* const dest, const char_type* const src, const pos_type count) {
            if constexpr(std::is_trivial_v<char_type>) {
                ::memcpy(dest, src, count);
            } else {
                for(pos_type i = 0; i < count; ++i) {
                    dest[i] = src[i];
                }
            }

            return dest;
        }

        static constexpr int compare(const char_type* const s1, const char_type* const s2, const pos_type count) {
            if constexpr(std::is_trivial_v<char_type>) {
                return ::strncmp(s1, s2, count);
            } else {
                for(pos_type i = 0; i < count; ++i) {
                    if(s1[i] != s2[i]) {
                        return false;
                    }
                }

                return true;
            }
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
