/**
 * @file
 *
 * XXHash64 implementation.
 */
#ifndef CCL_XXHASH64_HPP
#define CCL_XXHASH64_HPP

#include <ccl/api.hpp>
#include <ccl/concepts.hpp>
#include <ccl/debug.hpp>

namespace ccl {
    namespace xx64_internal {
        static constexpr uint64_t prime_1 = 0x9E3779B185EBCA87ULL;
        static constexpr uint64_t prime_2 = 0xC2B2AE3D27D4EB4FULL;
        static constexpr uint64_t prime_3 = 0x165667B19E3779F9ULL;
        static constexpr uint64_t prime_4 = 0x85EBCA77C2B2AE63ULL;
        static constexpr uint64_t prime_5 = 0x27D4EB2F165667C5ULL;

        template<std::integral T>
        constexpr T rotl64(const T x, const T r) noexcept {
            return ((x << r) | (x >> (64 - r)));
        }

        template<typename Target>
        constexpr Target convert(const auto * const ptr) noexcept
        {
            union u { const decltype(ptr) source; const Target *target; } x { .source = ptr };

            return *x.target;
        }

        constexpr uint64_t round(uint64_t acc, const uint64_t input)
        {
            acc += input * prime_2;
            acc  = rotl64(acc, static_cast<decltype(acc)>(31));
            acc *= prime_1;

            return acc;
        }

        constexpr uint64_t merge_round(uint64_t acc, uint64_t val)
        {
            val = round(0, val);
            acc ^= val;
            acc = acc * prime_1 + prime_4;

            return acc;
        }

        constexpr uint64_t avalanche(uint64_t hash)
        {
            hash ^= hash >> 33;
            hash *= prime_2;
            hash ^= hash >> 29;
            hash *= prime_3;
            hash ^= hash >> 32;

            return hash;
        }

        constexpr uint64_t
        finalize(uint64_t hash, const uint8_t * ptr, size_t len)
        {
            if (ptr==NULL) CCL_ASSERT(len == 0);
            len &= 31;
            while (len >= 8) {
                uint64_t const k1 = round(0, convert<uint64_t>(ptr));
                ptr += 8;
                hash ^= k1;
                hash  = rotl64(hash, static_cast<decltype(hash)>(27)) * prime_1 + prime_4;
                len -= 8;
            }
            if (len >= 4) {
                hash ^= (uint64_t)(convert<uint32_t>(ptr)) * prime_1;
                ptr += 4;
                hash = rotl64(hash, static_cast<decltype(hash)>(23)) * prime_2 + prime_3;
                len -= 4;
            }
            while (len > 0) {
                hash ^= (*ptr++) * prime_5;
                hash = rotl64(hash, static_cast<decltype(hash)>(11)) * prime_1;
                --len;
            }
            return  avalanche(hash);
        }
    }

    constexpr uint64_t xxh64(const uint8_t* input, const size_t len, const uint64_t seed)
    {
        using namespace xx64_internal;

        uint64_t h64;
        if (input==NULL) CCL_ASSERT(len == 0);

        if (len >= 32) {
            const uint8_t* const bEnd = input + len;
            const uint8_t* const limit = bEnd - 31;
            uint64_t v1 = seed + prime_1 + prime_2;
            uint64_t v2 = seed + prime_2;
            uint64_t v3 = seed + 0;
            uint64_t v4 = seed - prime_1;

            do {
                v1 = round(v1, convert<uint64_t>(input));
                input+=8;

                v2 = round(v2, convert<uint64_t>(input));
                input+=8;

                v3 = round(v3, convert<uint64_t>(input));
                input+=8;

                v4 = round(v4, convert<uint64_t>(input));
                input+=8;
            } while (input < limit);

            h64 = rotl64(v1, static_cast<decltype(v1)>(1))
                + rotl64(v2, static_cast<decltype(v2)>(7))
                + rotl64(v3, static_cast<decltype(v3)>(12))
                + rotl64(v4, static_cast<decltype(v4)>(18));

            h64 = merge_round(h64, v1);
            h64 = merge_round(h64, v2);
            h64 = merge_round(h64, v3);
            h64 = merge_round(h64, v4);
        } else {
            h64  = seed + prime_5;
        }

        h64 += static_cast<uint64_t>(len);

        return finalize(h64, input, len);
    }

    constexpr uint64_t myh = xxh64(xx64_internal::convert<uint8_t*>("asd"), 3, 0);
}

#endif // CCL_XXHASH64_HPP
