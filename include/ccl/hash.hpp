/**
 * @file
 *
 * Hash function
 */
#ifndef CCL_HASH_HPP
#define CCL_HASH_HPP

#include <concepts>
#include <utility>
#include <span>
#include <ccl/api.hpp>

namespace ccl {
    using hash_t = std::size_t;

    template<typename T>
    struct hash;

    /**
     * Hashable by implementation of a `hash<T>` specialisation.
     */
    template<typename T>
    concept externally_hashable = requires(T object) {
        { hash<T>{}(object) } -> std::convertible_to<hash_t>;
    };

    /**
     * Hashable by implementation of a `hash()` member function.
     */
    template<typename T>
    concept internally_hashable = requires(T object) {
        { object.hash() } -> std::convertible_to<hash_t>;
    };

    /**
     * Either externally or internally hashable.
     */
    template<typename T>
    concept hashable = internally_hashable<T> || externally_hashable<T>;

    /**
     * FNV-1A prime number.
     */
    static constexpr hash_t fnv1a_prime = 0x100000001B3ULL;

    /**
     * FNV-1A initial hash value.
     */
    static constexpr hash_t fnv1a_basis = 0xCBF29CE484222325ULL;

    /**
     * FNV-1A non cryptographic hashing function.
     *
     * @param size Length of the array to hash.
     * @param data Array to hash
     * @param initial Initial hash value. Can be used to resume hashing.
     *
     * @return The hash of `size` bytes from `data`. Can  be passed as the
     *  `initial` parameter of a following call to this function to resume
     *  hashing.
     *
     * @see https://datatracker.ietf.org/doc/html/draft-eastlake-fnv-18
     */
    constexpr hash_t fnv1a_hash(
        const std::size_t size,
        const uint8_t * const data,
        const hash_t initial = fnv1a_basis
    ) noexcept {
        hash_t result = initial;

        for(std::size_t i = 0; i < size; ++i) {
            result ^= data[i];
            result *= fnv1a_prime;
        }

        return result;
    }

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

            return static_cast<hash_t>(x.bits ? x.bits : 0.0);
        }
    };

    template<>
    struct hash<double> {
        constexpr hash_t operator()(const double n) {
            union pun { double n; uint64_t bits; } x;

            x.n = n;

            return static_cast<hash_t>(n ? x.bits : 0ULL);
        }
    };

    template<>
    struct hash<long double> {
        /**
         * @internal
         */
        hash_t _compute_hash_slow(const long double &n) {
            const long double k = n ? n : 0.0;

            return fnv1a_hash(sizeof(long double), &reinterpret_cast<const uint8_t&>(k));
        }

        constexpr hash_t operator()(const long double &n) {
            if constexpr(sizeof(double) != sizeof(long double)) {
                return _compute_hash_slow(n);
            } else {
                return hash<double>{}(static_cast<double>(n));
            }
        }
    };

    /**
     * General hash implementation used for internally hashable and (scoped) enumerations.
     *
     * @param T The type to hash.
     */
    template<typename T>
    struct hash {
        static_assert(internally_hashable<T> || std::is_enum_v<T> || std::is_scoped_enum_v<T>);

        constexpr hash_t operator()(const T& value) {
            if constexpr(internally_hashable<T>) {
                return value.hash();
            } else {
                using U = std::underlying_type_t<T>;

                return hash<U>{}(static_cast<U>(value));
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
