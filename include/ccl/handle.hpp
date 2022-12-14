/**
 * @file
 *
 * Versioned handle definition.
 */
#ifndef CCL_HANDLE_HPP
#define CCL_HANDLE_HPP

#include <ccl/api.hpp>
#include <ccl/packed-integer.hpp>
#include <ccl/hash.hpp>

namespace ccl {
    /**
     * A raw, numeric handle type.
     */
    using handle_t = uint32_t;

    /**
     * A handle possessing a value and a generation.
     *
     * Two versioned handle are only equal if both value and generation
     * match.
     *
     * Comparing two handles is similar to comparing two integers
     * (value only), with the exception that equality comparison
     * also accounts for the generation.
     *
     * @tparam ObjectType The type of object this handle represents.
     */
    template<typename ObjectType>
    class versioned_handle : protected packed_integer<handle_t, CCL_HANDLE_VALUE_WIDTH> {
        public:
            using underlying_type = packed_integer<handle_t, CCL_HANDLE_VALUE_WIDTH>;
            using object_type = ObjectType;
            using underlying_type::value_type;
            using packed_integer::operator typename underlying_type::value_type;

            constexpr versioned_handle& operator=(const versioned_handle& other) {
                underlying_type::operator=(other);

                return *this;
            }

            constexpr versioned_handle() = default;
            constexpr versioned_handle(const versioned_handle&) = default;
            explicit constexpr versioned_handle(const handle_t raw) : underlying_type{raw} {}

            constexpr auto generation() const { return high(); }
            constexpr auto value() const { return low(); }
            constexpr auto raw() const { return underlying_type::get(); }

            /**
             * Make a handle from its components.
             *
             * @param generation The handle generation.
             * @param value The handle value.
             */
            CCLNODISCARD static constexpr versioned_handle make(
                const underlying_type::value_type generation,
                const underlying_type::value_type value
            ) {
                return underlying_type::make(generation, value);
            }

            private:
                constexpr versioned_handle(const underlying_type value) : underlying_type{value} {}
    };

    template<typename T>
    constexpr bool operator>(const versioned_handle<T> a, const versioned_handle<T> b) {
        return a.value() > b.value();
    }

    template<typename T>
    constexpr bool operator<(const versioned_handle<T> a, const versioned_handle<T> b) {
        return a.value() < b.value();
    }

    template<typename T>
    constexpr bool operator<=(const versioned_handle<T> a, const versioned_handle<T> b) {
        return choose(
            true,
            a.raw() == b.raw(),
            a.value() < b.value()
        );
    }

    template<typename T>
    constexpr bool operator>=(const versioned_handle<T> a, const versioned_handle<T> b) {
        return choose(
            true,
            a.raw() == b.raw(),
            a.value() > b.value()
        );
    }

    template<typename T>
    constexpr bool operator==(const versioned_handle<T> a, const versioned_handle<T> b) {
        return a.raw() == b.raw();
    }

    /**
     * A generic versioned handle.
     */
    using untyped_versioned_handle = versioned_handle<void>;

    template<typename HandleType>
    struct hash<versioned_handle<HandleType>> {
        constexpr hash_t operator()(const versioned_handle<HandleType>& h) const {
            return hash<typename versioned_handle<HandleType>::underlying_type>{}(h.raw());
        }
    };
}

#endif // CCL_HANDLE_HPP
