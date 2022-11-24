/**
 * @file
 *
 * Hash function
 */
#ifndef CCL_HASH_HPP
#define CCL_HASH_HPP

#include <ccl/api.hpp>
#include <xxhash.h>

namespace ccl {
    template<typename T>
    struct hash {
        using hash_value = uint64_t;

        static constexpr hash_value seed = 137438953471; // Mersenne, p=37

        constexpr hash_value operator()(const T& value) {
            return XXH64(&value, sizeof(value), seed);
        }
    };
}

#endif // CCL_HASH_HPP
