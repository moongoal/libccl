/**
 * @file
 *
 * Hash function
 */
#ifndef CCL_HASH_HPP
#define CCL_HASH_HPP

#include <ccl/api.hpp>
#include <ccl/xxhash64.hpp>

namespace ccl {
    template<typename T>
    struct hash {
        using hash_value = uint64_t;

        constexpr hash_value operator()(const T& value) {
            return xxh64(&value, sizeof(value), 0);
        }
    };
}

#endif // CCL_HASH_HPP
