/**
 * @file
 *
 * Typed handle definition.
 */
#ifndef CCL_HANDLE_TYPED_HANDLE_HPP
#define CCL_HANDLE_TYPED_HANDLE_HPP

#include <ccl/api.hpp>
#include <ccl/packed-integer.hpp>
#include <ccl/hash.hpp>

namespace ccl {
    /**
     * A numeric, typed handle.
     *
     * Two versioned handle are only equal if both value and generation
     * match.
     *
     * Comparing two handles is equivalent to comparing two integers.
     *
     * @tparam ObjectType The type of object this handle represents.
     */
    template<typename ObjectType>
    class typed_handle {
        public:
            using object_type = ObjectType;

            static constexpr handle_t max_value = ~static_cast<handle_t>(0);

        private:
            handle_t _value;

        public:
            constexpr typed_handle() noexcept : _value{0} {}
            constexpr typed_handle(const typed_handle&) = default;
            explicit constexpr typed_handle(const handle_t raw) : _value{raw} {}
            constexpr typed_handle& operator=(const typed_handle& other) = default;

            constexpr auto value() const { return _value; }
    };

    template<typename T>
    constexpr bool operator>(const typed_handle<T> a, const typed_handle<T> b) {
        return a.value() > b.value();
    }

    template<typename T>
    constexpr bool operator<(const typed_handle<T> a, const typed_handle<T> b) {
        return a.value() < b.value();
    }

    template<typename T>
    constexpr bool operator<=(const typed_handle<T> a, const typed_handle<T> b) {
        return a.value() <= b.value();
    }

    template<typename T>
    constexpr bool operator>=(const typed_handle<T> a, const typed_handle<T> b) {
        return a.value() >= b.value();
    }

    template<typename T>
    constexpr bool operator==(const typed_handle<T> a, const typed_handle<T> b) {
        return a.value() == b.value();
    }

    template<typename HandleType>
    struct hash<typed_handle<HandleType>> {
        constexpr hash_t operator()(const typed_handle<HandleType>& h) const {
            return hash<handle_t>{}(h.value());
        }
    };

    template<typename HandleTypeTo, typename HandleTypeFrom>
    constexpr typed_handle<HandleTypeTo> static_handle_cast(const typed_handle<HandleTypeFrom> handle) {
        static_assert(std::is_base_of_v<HandleTypeTo, HandleTypeFrom>);

        return typed_handle<HandleTypeTo>{handle.value()};
    }
}

#endif // CCL_HANDLE_TYPED_HANDLE_HPP
