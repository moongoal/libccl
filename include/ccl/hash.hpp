/**
 * @file
 *
 * Hash function
 */
#ifndef CCL_HASH_HPP
#define CCL_HASH_HPP

#include <concepts>
#include <utility>
#include <ccl/api.hpp>

namespace ccl {
    using hash_t = size_t;

    template<typename T>
    struct hash;

    template<typename T>
    concept externally_hashable = requires(T object) {
        { hash<T>{}(object) } -> std::convertible_to<hash_t>;
    };

    template<typename T>
    concept internally_hashable = requires(T object) {
        { object.hash() } -> std::convertible_to<hash_t>;
    };

    template<typename T>
    concept hashable = internally_hashable<T> || externally_hashable<T>;

    // Basic types
    #define CCL__DECL_HASH(T) template<> struct hash<T> { constexpr hash_t operator()(const T value) { return static_cast<T>(value); } }

        CCL__DECL_HASH(bool);

        CCL__DECL_HASH(uint8_t);
        CCL__DECL_HASH(uint16_t);
        CCL__DECL_HASH(uint32_t);
        CCL__DECL_HASH(uint64_t);

        CCL__DECL_HASH(int8_t);
        CCL__DECL_HASH(int16_t);
        CCL__DECL_HASH(int32_t);
        CCL__DECL_HASH(int64_t);

        CCL__DECL_HASH(char8_t);
        CCL__DECL_HASH(char16_t);
        CCL__DECL_HASH(char32_t);
        CCL__DECL_HASH(char);
        CCL__DECL_HASH(wchar_t);

    #undef CCL__DECL_HASH

    template<>
    struct hash<std::nullptr_t> {
        constexpr hash_t operator()(const std::nullptr_t&) {
            return 0;
        }
    };

    template<typename T>
    struct hash<T*> {
        constexpr hash_t operator()(T* const value) {
            return reinterpret_cast<hash_t>(value);
        }
    };

    template<>
    struct hash<float> {
        constexpr hash_t operator()(const float n) {
            union pun { float n; uint32_t bits; } x;

            x.n = n;

            return static_cast<hash_t>(x.bits);
        }
    };

    template<>
    struct hash<double> {
        constexpr hash_t operator()(const double n) {
            union pun { double n; uint64_t bits; } x;

            x.n = n;

            return static_cast<hash_t>(x.bits);
        }
    };

    template<>
    struct hash<long double> {
        constexpr hash_t operator()(const long double n) {
            static_assert(sizeof(long double) <= sizeof(double) * 2, "This compiler or architecture requires a different hash implementation for long double.");

            if constexpr (sizeof(long double) == sizeof(double)) {
                return hash<double>{}(static_cast<double>(n));
            } else {
                union pun { long double n; uint64_t bits[2]; } x;

                x.n = n;

                return static_cast<hash_t>(x.bits[0] ^ x.bits[1]);
            }
        }
    };

    template<typename T>
    struct hash {
        constexpr hash_t operator()(const T& value) {
            static_assert(internally_hashable<T> || std::is_enum_v<T> || std::is_scoped_enum_v<T>);

            if constexpr(internally_hashable<T>) {
                return value.hash();
            } else {
                return hash<size_t>{}(static_cast<size_t>(value));
            }
        }
    };

    /**
     * A hash function capable of hashing a given type.
     *
     * @tparam Function The function type.
     * @tparam T The hashable type.
     */
    template<typename Function, typename T>
    concept typed_hash_function = requires(Function f, T object) {
        { f(object) } -> std::convertible_to<hash_t>;
    };
}

#endif // CCL_HASH_HPP
