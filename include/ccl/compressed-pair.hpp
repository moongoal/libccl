/**
 * @file
 *
 * Compressed pair implementation.
 */
#ifndef CCL_COMPRESSED_PAIR_HPP
#define CCL_COMPRESSED_PAIR_HPP

#include <type_traits>
#include <utility>
#include <ccl/api.hpp>

namespace ccl {
    namespace internal {
        template<
            typename T1,
            typename T2,
            bool SameTypes,
            bool T1Empty,
            bool T2Empty
        > struct compressed_pair_switch;

        static constexpr int compressed_pair_type_diff_non_empty = 0; // Different types, both non-empty
        static constexpr int compressed_pair_type_same_non_empty = 1; // Same types, both non-empty
        static constexpr int compressed_pair_type_diff_one_empty = 2; // Different types, first empty
        static constexpr int compressed_pair_type_diff_two_empty = 3; // Different types, second empty
        static constexpr int compressed_pair_type_diff_all_empty = 4; // Different types, both empty
        static constexpr int compressed_pair_type_same_all_empty = 5; // Same types, both empty

        template<typename T1, typename T2>
        struct compressed_pair_switch<T1, T2, false, false, false> {
            static const int value = compressed_pair_type_diff_non_empty;
        };

        template<typename T1, typename T2>
        struct compressed_pair_switch<T1, T2, true, false, false> {
            static const int value = compressed_pair_type_same_non_empty;
        };

        template<typename T1, typename T2>
        struct compressed_pair_switch<T1, T2, false, true, false> {
            static const int value = compressed_pair_type_diff_one_empty;
        };

        template<typename T1, typename T2>
        struct compressed_pair_switch<T1, T2, false, false, true> {
            static const int value = compressed_pair_type_diff_two_empty;
        };

        template<typename T1, typename T2>
        struct compressed_pair_switch<T1, T2, false, true, true> {
            static const int value = compressed_pair_type_diff_all_empty;
        };

        template<typename T1, typename T2>
        struct compressed_pair_switch<T1, T2, true, true, true> {
            static const int value = compressed_pair_type_same_all_empty;
        };

        template<typename T1, typename T2, bool SameTypes, bool T1Empty, bool T2Empty>
        static constexpr int compressed_pair_switch_v = compressed_pair_switch<T1, T2, SameTypes, T1Empty, T2Empty>::value;

        template<typename T1, typename T2>
        struct comparessed_pair;

        template<typename T1, typename T2, int PairType>
        struct compressed_pair_impl;

        template<typename T1, typename T2>
        struct compressed_pair_impl<T1, T2, compressed_pair_type_diff_non_empty> {
            using first_type = T1;
            using second_type = T2;
            using first_reference = T1&;
            using second_reference = T2&;
            using first_const_reference = const T1&;
            using second_const_reference = const T2&;

            constexpr compressed_pair_impl() = default;
            constexpr compressed_pair_impl(first_const_reference x) : _first{x} {}
            constexpr compressed_pair_impl(second_const_reference x) : _second{x} {}
            constexpr compressed_pair_impl(first_const_reference x, second_const_reference y) : _first{x}, _second{y} {}
            constexpr compressed_pair_impl(const compressed_pair_impl &other) = default;
            constexpr compressed_pair_impl(compressed_pair_impl &&other) = default;

            constexpr compressed_pair_impl& operator=(const compressed_pair_impl &other) = default;
            constexpr compressed_pair_impl& operator=(compressed_pair_impl &&other) = default;

            constexpr bool operator==(const compressed_pair_impl &other) const {
                return _first == other._first && _second == other._second;
            }

            constexpr bool operator!=(const compressed_pair_impl &other) const {
                return _first != other._first || _second != other._second;
            }

            constexpr first_reference first() noexcept { return _first; }
            constexpr first_const_reference first() const noexcept { return _first; }

            constexpr second_reference second() noexcept { return _second; }
            constexpr second_const_reference second() const noexcept { return _second; }

            private:
                T1 _first;
                T2 _second;
        };

        template<typename T>
        struct compressed_pair_impl<T, T, compressed_pair_type_same_non_empty> {
            using first_type = T;
            using second_type = T;
            using first_reference = T&;
            using second_reference = T&;
            using first_const_reference = const T&;
            using second_const_reference = const T&;

            constexpr compressed_pair_impl() = default;
            constexpr compressed_pair_impl(first_const_reference x, second_const_reference y) : _first{x}, _second{y} {}
            constexpr compressed_pair_impl(const compressed_pair_impl &other) = default;
            constexpr compressed_pair_impl(compressed_pair_impl &&other) = default;

            constexpr compressed_pair_impl& operator=(const compressed_pair_impl &other) = default;
            constexpr compressed_pair_impl& operator=(compressed_pair_impl &&other) = default;

            constexpr bool operator==(const compressed_pair_impl &other) const {
                return _first == other._first && _second == other._second;
            }

            constexpr bool operator!=(const compressed_pair_impl &other) const {
                return _first != other._first || _second != other._second;
            }

            constexpr first_reference first() noexcept { return _first; }
            constexpr first_const_reference first() const noexcept { return _first; }

            constexpr second_reference second() noexcept { return _second; }
            constexpr second_const_reference second() const noexcept { return _second; }

            private:
                T _first;
                T _second;
        };

        template<typename T1, typename T2>
        struct compressed_pair_impl<T1, T2, compressed_pair_type_diff_one_empty> : private T1 {
            using first_type = T1;
            using second_type = T2;
            using first_reference = T1&;
            using second_reference = T2&;
            using first_const_reference = const T1&;
            using second_const_reference = const T2&;

            constexpr compressed_pair_impl() = default;
            constexpr compressed_pair_impl(second_const_reference x) : _second{x} {}
            constexpr compressed_pair_impl(first_const_reference, second_const_reference y) : _second{y} {}
            constexpr compressed_pair_impl(const compressed_pair_impl &other) = default;
            constexpr compressed_pair_impl(compressed_pair_impl &&other) = default;

            constexpr compressed_pair_impl& operator=(const compressed_pair_impl &other) = default;
            constexpr compressed_pair_impl& operator=(compressed_pair_impl &&other) = default;

            constexpr bool operator==(const compressed_pair_impl &other) const {
                return _second == other._second;
            }

            constexpr bool operator!=(const compressed_pair_impl &other) const {
                return _second != other._second;
            }

            constexpr first_reference first() noexcept { return *this; }
            constexpr first_const_reference first() const noexcept { return *this; }

            constexpr second_reference second() noexcept { return _second; }
            constexpr second_const_reference second() const noexcept { return _second; }

            private:
                T2 _second;
        };

        template<typename T1, typename T2>
        struct compressed_pair_impl<T1, T2, compressed_pair_type_diff_two_empty> : private T2 {
            using first_type = T1;
            using second_type = T2;
            using first_reference = T1&;
            using second_reference = T2&;
            using first_const_reference = const T1&;
            using second_const_reference = const T2&;

            constexpr compressed_pair_impl() = default;
            constexpr compressed_pair_impl(first_const_reference x) : _first{x} {}
            constexpr compressed_pair_impl(first_const_reference x, second_const_reference) : _first{x}{}
            constexpr compressed_pair_impl(const compressed_pair_impl &other) = default;
            constexpr compressed_pair_impl(compressed_pair_impl &&other) = default;

            constexpr compressed_pair_impl& operator=(const compressed_pair_impl &other) = default;
            constexpr compressed_pair_impl& operator=(compressed_pair_impl &&other) = default;

            constexpr bool operator==(const compressed_pair_impl &other) const {
                return _first == other._first;
            }

            constexpr bool operator!=(const compressed_pair_impl &other) const {
                return _first != other._first;
            }

            constexpr first_reference first() noexcept { return _first; }
            constexpr first_const_reference first() const noexcept { return _first; }

            constexpr second_reference second() noexcept { return *this; }
            constexpr second_const_reference second() const noexcept { return *this; }

            private:
                T1 _first;
        };

        template<typename T1, typename T2>
        struct compressed_pair_impl<T1, T2, compressed_pair_type_diff_all_empty> : private T1, private T2 {
            using first_type = T1;
            using second_type = T2;
            using first_reference = T1&;
            using second_reference = T2&;
            using first_const_reference = const T1&;
            using second_const_reference = const T2&;

            constexpr compressed_pair_impl() = default;
            constexpr compressed_pair_impl(first_const_reference) {}
            constexpr compressed_pair_impl(second_const_reference) {}
            constexpr compressed_pair_impl(first_const_reference, second_const_reference) {}
            constexpr compressed_pair_impl(const compressed_pair_impl &other) = default;
            constexpr compressed_pair_impl(compressed_pair_impl &&other) = default;

            constexpr compressed_pair_impl& operator=(const compressed_pair_impl &other) = default;
            constexpr compressed_pair_impl& operator=(compressed_pair_impl &&other) = default;

            constexpr bool operator==(const compressed_pair_impl&) const {
                return true;
            }

            constexpr bool operator!=(const compressed_pair_impl&) const {
                return false;
            }

            constexpr first_reference first() noexcept { return *this; }
            constexpr first_const_reference first() const noexcept { return *this; }

            constexpr second_reference second() noexcept { return *this; }
            constexpr second_const_reference second() const noexcept { return *this; }
        };

        template<typename T>
        struct compressed_pair_impl<T, T, compressed_pair_type_same_all_empty> : private T {
            using first_type = T;
            using second_type = T;
            using first_reference = T&;
            using second_reference = T&;
            using first_const_reference = const T&;
            using second_const_reference = const T&;

            constexpr compressed_pair_impl() = default;
            constexpr compressed_pair_impl(first_const_reference, second_const_reference) {}
            constexpr compressed_pair_impl(const compressed_pair_impl &other) = default;
            constexpr compressed_pair_impl(compressed_pair_impl &&other) = default;

            constexpr compressed_pair_impl& operator=(const compressed_pair_impl &other) = default;
            constexpr compressed_pair_impl& operator=(compressed_pair_impl &&other) = default;

            constexpr bool operator==(const compressed_pair_impl&) const {
                return true;
            }

            constexpr bool operator!=(const compressed_pair_impl&) const {
                return false;
            }

            constexpr first_reference first() noexcept { return *this; }
            constexpr first_const_reference first() const noexcept { return *this; }

            constexpr second_reference second() noexcept { return *this; }
            constexpr second_const_reference second() const noexcept { return *this; }
        };
    }

    template<typename T1, typename T2>
        struct compressed_pair : private internal::compressed_pair_impl<
            T1, T2,
            internal::compressed_pair_switch_v<
                T1, T2,
                std::is_same_v<std::remove_cv_t<T1>, std::remove_cv_t<T2>>,
                std::is_empty_v<T1>, std::is_empty_v<T2>
            >
        > {
            using first_type = T1;
            using second_type = T2;
            using first_reference = T1&;
            using second_reference = T2&;
            using first_const_reference = const T1&;
            using second_const_reference = const T2&;

            constexpr compressed_pair() = default;
            constexpr compressed_pair(first_const_reference x) : impl{x} {}
            constexpr compressed_pair(second_const_reference x) : impl{x} {}
            constexpr compressed_pair(first_const_reference x, second_const_reference y) : impl{x, y} {}
            constexpr compressed_pair(const compressed_pair &other) = default;
            constexpr compressed_pair(compressed_pair &&other) = default;

            constexpr compressed_pair& operator=(const compressed_pair &other) = default;
            constexpr compressed_pair& operator=(compressed_pair &&other) = default;

            constexpr bool operator==(const compressed_pair &other) const {
                return impl::operator==(other);
            }

            constexpr bool operator!=(const compressed_pair &other) const {
                return impl::operator==(other);
            }

            constexpr first_reference first() noexcept { return impl::first(); }
            constexpr first_const_reference first() const noexcept { return impl::first(); }

            constexpr second_reference second() noexcept { return impl::second(); }
            constexpr second_const_reference second() const noexcept { return impl::second(); }

            private:
                using impl = internal::compressed_pair_impl<
                    T1, T2,
                    internal::compressed_pair_switch_v<
                        T1, T2,
                        std::is_same_v<std::remove_cv_t<T1>, std::remove_cv_t<T2>>,
                        std::is_empty_v<T1>, std::is_empty_v<T2>
                    >
                >;
        };

    template<typename T>
    struct compressed_pair<T, T> : private internal::compressed_pair_impl<
        T, T,
        internal::compressed_pair_switch_v<
            T, T,
            true,
            std::is_empty_v<T>, std::is_empty_v<T>
        >
    > {
        using first_type = T;
        using second_type = T;
        using first_reference = T&;
        using second_reference = T&;
        using first_const_reference = const T&;
        using second_const_reference = const T&;

        constexpr compressed_pair() = default;
        constexpr compressed_pair(first_const_reference x, second_const_reference y) : impl{x, y} {}
        constexpr compressed_pair(const compressed_pair &other) = default;
        constexpr compressed_pair(compressed_pair &&other) = default;

        constexpr compressed_pair& operator=(const compressed_pair &other) = default;
        constexpr compressed_pair& operator=(compressed_pair &&other) = default;

        constexpr bool operator==(const compressed_pair &other) const {
            return impl::operator==(other);
        }

        constexpr bool operator!=(const compressed_pair &other) const {
            return impl::operator==(other);
        }

        constexpr first_reference first() noexcept { return impl::first(); }
        constexpr first_const_reference first() const noexcept { return impl::first(); }

        constexpr second_reference second() noexcept { return impl::second(); }
        constexpr second_const_reference second() const noexcept { return impl::second(); }

        private:
            using impl = internal::compressed_pair_impl<
                T, T,
                internal::compressed_pair_switch_v<
                    T, T,
                    std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<T>>,
                    std::is_empty_v<T>, std::is_empty_v<T>
                >
            >;
    };

    template<typename X, typename Y>
    constexpr compressed_pair<X, Y> make_compressed_pair(X&& x, Y&& y) {
        return compressed_pair<X, Y>{std::forward<X>(x), std::forward<Y>(y)};
    }
}

#endif // CCL_COMPRESSED_PAIR_HPP
