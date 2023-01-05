/**
 * @file
 *
 * STL compatibility.
 */
#ifndef CCL_COMPAT_HPP
#define CCL_COMPAT_HPP

#include <typeindex>
#include <memory>
#include <string>
#include <ccl/api.hpp>
#include <ccl/hash.hpp>

namespace ccl {
    template<>
    struct hash<std::type_index> {
        constexpr hash_t operator()(const std::type_index& x) const {
            return x.hash_code();
        }
    };

    template<typename T>
    struct hash<std::shared_ptr<T>> {
        constexpr hash_t operator()(const std::shared_ptr<T>& x) const {
            return hash<T*>{}(std::to_address(x));
        }
    };

    template<typename T>
    struct hash<std::unique_ptr<T>> {
        constexpr hash_t operator()(const std::unique_ptr<T>& x) const {
            return hash<T*>{}(std::to_address(x));
        }
    };

    template<
        class CharT,
        class Traits,
        class Allocator
    > struct hash<std::basic_string<CharT, Traits, Allocator>> {
        using S = std::basic_string<CharT, Traits, Allocator>;

        constexpr hash_t operator()(const S& x) const {
            return fnv1a_hash(sizeof(CharT) * x.size(), reinterpret_cast<const uint8_t*>(x.data()));
        }
    };
}

#endif // CCL_COMPAT_HPP
