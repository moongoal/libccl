/**
 * @file
 *
 * A packed integer is made of two integers fitting the same
 * memory location.
 */
#ifndef CCL_PACKED_INTEGER_HPP
#define CCL_PACKED_INTEGER_HPP

#include <ccl/api.hpp>
#include <ccl/util.hpp>
#include <ccl/concepts.hpp>
#include <ccl/exceptions.hpp>
#include <ccl/hash.hpp>

namespace ccl {
    /**
     * A single integer packing two values as "high" part
     * and "low" part.
     *
     * @tparam T The underlying unsigned integer type.
     * @tparam LowPartSize The size in bits of the lower part.
     */
    template<std::unsigned_integral T, std::size_t LowPartSize = sizeof(T) * 4>
    class packed_integer {
        static_assert(LowPartSize > 0 && LowPartSize < sizeof(T) * 8, "LowPartSize out of bounds.");

        public:
            using value_type = T;

            /**
             * The size of the low part, in bits.
             */
            static constexpr std::size_t low_part_size = LowPartSize;

            /**
             * The number of bits to shift the value to retrieve the high part.
             */
            static constexpr std::size_t high_part_shift_bits = LowPartSize;

            /**
             * The mask to apply to the value to retrieve the low part.
             */
            static constexpr value_type low_part_mask = ~(~static_cast<value_type>(0) >> high_part_shift_bits << high_part_shift_bits);

            /**
             * Maximum value for the low part.
             */
            static constexpr value_type low_part_max = low_part_mask;

            /**
             * Maximum value for the high part.
             */
            static constexpr value_type high_part_max = (~low_part_max) >> high_part_shift_bits;

        private:
            value_type value;

        public:
            constexpr packed_integer() : value{static_cast<T>(0)} {}
            constexpr packed_integer(const packed_integer &) = default;
            constexpr packed_integer(const value_type value) : value{value} {}

            constexpr operator value_type() const { return value; }
            constexpr packed_integer& operator =(const packed_integer &) = default;
            constexpr packed_integer& operator =(packed_integer &&) = default;

            constexpr packed_integer& operator =(const value_type value) {
                this->value = value;

                return *this;
            }

            /**
             * Get the high part.
             *
             * @return The value of the high part.
             */
            constexpr value_type high() const {
                return value >> high_part_shift_bits;
            }

            /**
             * Get the low part.
             *
             * @return The value of the low part.
             */
            constexpr value_type low() const {
                return value & low_part_mask;
            }

            /**
             * Get the whole value.
             *
             * @return The full value.
             */
            constexpr value_type get() const {
                return value;
            }

            CCLNODISCARD static constexpr packed_integer make(const value_type high, const value_type low) {
                CCL_THROW_IF(high > high_part_max, std::out_of_range{"High value too large."});
                CCL_THROW_IF(low > low_part_max, std::out_of_range{"Low value too large."});

                return (high << high_part_shift_bits) | low;
            }
    };

    template<typename ...Args>
    constexpr bool operator==(const packed_integer<Args...> a, const packed_integer<Args...> b) {
        return a.get() == b.get();
    }

    template<std::unsigned_integral T, std::size_t LowPartSize>
    struct hash<packed_integer<T, LowPartSize>> {
        constexpr hash_t operator()(const packed_integer<T, LowPartSize>& value) const {
            return hash<typename packed_integer<T, LowPartSize>::value_type>{}(value.get());
        }
    };
}

#endif // CCL_PACKED_INTEGER_HPP
