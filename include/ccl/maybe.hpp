/**
 * @file
 *
 * Conditional memory residency.
 */
#ifndef CCL_MAYBE_HPP
#define CCL_MAYBE_HPP

#include <utility>
#include <ccl/api.hpp>

namespace ccl {
    namespace internal {
        template<typename T, bool TEmpty>
        struct maybe_switch;

        static constexpr int maybe_type_empty = 0;
        static constexpr int maybe_type_present = 1;

        template<typename T>
        struct maybe_switch<T, true> { static constexpr int value = maybe_type_empty; };

        template<typename T>
        struct maybe_switch<T, false> { static constexpr int value = maybe_type_present; };

        template<typename T, bool TEmpty>
        static constexpr int maybe_switch_v = maybe_switch<T, TEmpty>::value;

        template<typename T, int MaybeType>
        struct maybe_impl;

        template<typename T>
        struct maybe_impl<T, maybe_type_empty> : T {
            using value_type = T;
            using reference = T&;
            using const_reference = const T&;
            using rvalue_reference = T&&;

            constexpr maybe_impl() = default;
            constexpr maybe_impl(const maybe_impl &) = default;
            constexpr maybe_impl(maybe_impl &&) = default;
            constexpr maybe_impl(const_reference) {}
            constexpr maybe_impl(rvalue_reference) {}

            constexpr maybe_impl& operator =(const_reference) { return *this; }
            constexpr maybe_impl& operator =(rvalue_reference) { return *this; }

            constexpr reference operator *() noexcept { return *this; }
            constexpr const_reference operator *() const noexcept { return *this; }

            constexpr reference get() noexcept { return *this; }
            constexpr const_reference get() const noexcept { return *this; }
        };

        template<typename T>
        struct maybe_impl<T, maybe_type_present> {
            using value_type = T;
            using reference = T&;
            using const_reference = const T&;
            using rvalue_reference = T&&;

            constexpr maybe_impl() = default;
            constexpr maybe_impl(const maybe_impl &) = default;
            constexpr maybe_impl(maybe_impl &&) = default;
            constexpr maybe_impl(const_reference x) : value{x} {}
            constexpr maybe_impl(rvalue_reference x) : value{std::move(x)} {}

            constexpr maybe_impl& operator =(const_reference other) {
                value = other.value;
                return *this;
            }

            constexpr maybe_impl& operator =(rvalue_reference other) {
                value = std::move(other.value);
                return *this;
            }

            constexpr reference operator *() noexcept { return value; }
            constexpr const_reference operator *() const noexcept { return value; }

            constexpr reference get() noexcept { return value; }
            constexpr const_reference get() const noexcept { return value; }

            private:
                value_type value;
        };
    }

    template<typename T>
    struct maybe : internal::maybe_impl<
        T,
        internal::maybe_switch_v<T, std::is_empty_v<T>>
    > {
        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        using rvalue_reference = T&&;

        constexpr maybe() = default;
        constexpr maybe(const maybe &other) : impl{other} {}
        constexpr maybe(maybe &&other) : impl{std::move(other)} {}
        constexpr maybe(const_reference x) : impl{x} {}
        constexpr maybe(rvalue_reference x) : impl{std::move(x)} {}

        constexpr maybe& operator =(const_reference other) {
            impl::operator=(other);
            return *this;
        }

        constexpr maybe& operator =(rvalue_reference other) {
            impl::operator=(std::move(other));
            return *this;
        }

        constexpr reference operator *() noexcept { return impl::get(); }
        constexpr const_reference operator *() const noexcept { return impl::get(); }

        constexpr reference get() noexcept { return impl::get(); }
        constexpr const_reference get() const noexcept { return impl::get(); }

        private:
            using impl = internal::maybe_impl<T, internal::maybe_switch_v<T, std::is_empty_v<T>>>;
    };
}

#endif // CCL_MAYBE_HPP
