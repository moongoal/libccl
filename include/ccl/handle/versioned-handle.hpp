/**
 * @file
 *
 * Versioned handle definition.
 */
#ifndef CCL_HANDLE_VERSIONED_HANDLE_HPP
#define CCL_HANDLE_VERSIONED_HANDLE_HPP

#include <ccl/api.hpp>
#include <ccl/packed-integer.hpp>
#include <ccl/hash.hpp>
#include <ccl/handle/raw-handle.hpp>

namespace ccl {
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
    class versioned_handle {
        public:
            using underlying_type = packed_integer<handle_t, CCL_HANDLE_VALUE_WIDTH>;

        private:
            underlying_type _value;

        public:
            using object_type = ObjectType;
            using value_type = underlying_type::value_type;

            static constexpr handle_t max_generation = underlying_type::high_part_max;
            static constexpr handle_t max_value = underlying_type::low_part_max;
            static constexpr handle_t invalid_handle_value = max_value;

            constexpr versioned_handle& operator=(const versioned_handle& other) {
                _value = other._value;

                return *this;
            }

            constexpr versioned_handle& operator=(versioned_handle&& other) {
                _value = std::move(other._value);

                return *this;
            }

            constexpr versioned_handle() : _value{invalid_handle_value} {}

            constexpr versioned_handle(const versioned_handle&) = default;

            constexpr versioned_handle(versioned_handle&& other) : _value{invalid_handle_value} {
                swap(*this, other);

            }

            explicit constexpr versioned_handle(const handle_t raw) : _value{raw} {}

            constexpr auto generation() const { return _value.high(); }
            constexpr auto value() const { return _value.low(); }
            constexpr auto raw() const { return _value.get(); }
            constexpr bool is_null() const { return value() == invalid_handle_value; }

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
                constexpr versioned_handle(const underlying_type value) : _value{value} {}
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

    template<typename HandleType>
    struct hash<versioned_handle<HandleType>> {
        constexpr hash_t operator()(const versioned_handle<HandleType>& h) const {
            return hash<typename versioned_handle<HandleType>::underlying_type>{}(h.raw());
        }
    };

    template<typename HandleTypeTo, typename HandleTypeFrom>
    constexpr versioned_handle<HandleTypeTo> static_handle_cast(const versioned_handle<HandleTypeFrom> handle) {
        static_assert(std::is_base_of_v<HandleTypeTo, HandleTypeFrom>);

        return versioned_handle<HandleTypeTo>::make(handle.generation(), handle.value());
    }

    template<typename HandleTypeTo, typename HandleTypeFrom>
    constexpr versioned_handle<HandleTypeTo> reinterpret_handle_cast(const versioned_handle<HandleTypeFrom> handle) {
        return versioned_handle<HandleTypeTo>::make(handle.generation(), handle.value());
    }
}

#endif // CCL_HANDLE_VERSIONED_HANDLE_HPP
