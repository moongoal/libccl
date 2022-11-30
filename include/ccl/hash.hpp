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
        using hash_value_type = uint64_t;

        constexpr hash_value_type operator()(const T& value) {
            return XXH3_64bits(&value, sizeof(value));
        }
    };
}

#endif // CCL_HASH_HPP
