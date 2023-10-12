/**
 * @file
 *
 * Generic string of characters.
 */
#ifndef CCL_STRING_BASE_STRING_HPP
#define CCL_STRING_BASE_STRING_HPP

#include <iterator>
#include <ranges>
#include <ccl/api.hpp>
#include <ccl/concepts.hpp>
#include <ccl/hash.hpp>
#include <ccl/vector.hpp>
#include <ccl/contiguous-iterator.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/internal/optional-allocator.hpp>
#include <ccl/string/char-traits.hpp>

namespace ccl {
    template<
        typename CharType,
        char_traits_impl<CharType> CharTraits,
        typed_allocator<CharType> Allocator
    > class basic_string_builder;

    template<
        typename CharType,
        char_traits_impl<CharType> CharTraits = char_traits<CharType>,
        typed_allocator<CharType> Allocator = allocator
    > class basic_string : internal::with_optional_allocator<Allocator> {
        friend class basic_string_builder<CharType, CharTraits, Allocator>;

        using alloc = internal::with_optional_allocator<Allocator>;

        public:
            using value_type = CharType;
            using reference = CharType&;
            using const_reference = const CharType&;
            using pointer = CharType*;
            using const_pointer = const CharType*;
            using size_type = count_t;
            using allocator_type = Allocator;
            using iterator = contiguous_iterator<CharType>;
            using const_iterator = contiguous_iterator<const CharType>;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            using char_traits = CharTraits;

        private:
            pointer data = nullptr;
            size_type _length = 0;
            allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS;

        public:
             explicit constexpr basic_string(
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) : alloc{allocator}, alloc_flags{alloc_flags}
            {}

            constexpr basic_string(
                const basic_string &other
            ) :
                alloc{other.get_allocator()},
                data{alloc::get_allocator()->template allocate<value_type>(size_of<value_type>(other._length), other.alloc_flags)},
                _length{other._length},
                alloc_flags{other.alloc_flags}
            {
                char_traits::copy(data, other.data, _length);
            }

            /**
             * Create a new string from its raw representation.
             *
             * @param raw The raw string data.
             * @param length The length of the string.
             * @param allocator The allocator object.
             * @param alloc_flags Flags to pass to the allocator.
             */
            constexpr basic_string(
                const_pointer raw,
                const size_type length,
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) :
                alloc{allocator},
                data{alloc::get_allocator()->template allocate<value_type>(size_of<value_type>(length), alloc_flags)},
                _length{length},
                alloc_flags{alloc_flags}
            {
                char_traits::copy(data, raw, _length);
            }

            /**
             * Create a new string from a literal.
             *
             * @param raw The literal string data.
             * @param allocator The allocator object.
             * @param alloc_flags Flags to pass to the allocator.
             */
            template<size_type N>
            constexpr basic_string(
                const CharType (&raw)[N],
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ): basic_string{raw, N - 1, allocator, alloc_flags}
            {}

            constexpr basic_string(basic_string &&other) {
                swap(other);
            }

            ~basic_string() {
                destroy();
            }

            constexpr auto swap(basic_string &other) noexcept {
                alloc::swap(other);

                std::swap(data, other.data);
                std::swap(_length, other._length);
                std::swap(alloc_flags, other.alloc_flags);
            }

            /**
             * The length of the string, in characters.
             */
            constexpr auto length() const noexcept {
                return _length;
            }

            /**
             * Return the raw string data.
             *
             * This function does **not** return a nul-terminated string.
             */
            constexpr const_pointer raw() const noexcept {
                return data;
            }

            constexpr bool operator==(const basic_string& rhs) const {
                if(_length != rhs._length) {
                    return false;
                }

                for(size_type i = 0; i < _length; ++i) {
                    if(data[i] != rhs.data[i]) {
                        return false;
                    }
                }

                return true;
            }

            constexpr bool operator!=(const basic_string& rhs) const {
                return !basic_string::operator==(rhs);
            }

            constexpr bool operator<(const basic_string& rhs) const {
                const size_type sz = min(_length, rhs._length);

                for(size_type i = 0; i < sz; ++i) {
                    if(!char_traits::eq(data[i], rhs.data[i])) {
                        return char_traits::lt(data[i], rhs.data[i]);
                    }
                }

                return false;
            }

            constexpr bool operator<=(const basic_string& rhs) const {
                const size_type sz = min(_length, rhs._length);

                for(size_type i = 0; i < sz; ++i) {
                    if(!char_traits::eq(data[i], rhs.data[i])) {
                        return char_traits::lt(data[i], rhs.data[i]);
                    }
                }

                return true;
            }

            constexpr bool operator>(const basic_string& rhs) const {
                const size_type sz = min(_length, rhs._length);

                for(size_type i = 0; i < sz; ++i) {
                    if(!char_traits::eq(data[i], rhs.data[i])) {
                        return char_traits::lt(rhs.data[i], data[i]);
                    }
                }

                return false;
            }

            constexpr bool operator>=(const basic_string& rhs) const {
                const size_type sz = min(_length, rhs._length);

                for(size_type i = 0; i < sz; ++i) {
                    if(!char_traits::eq(data[i], rhs.data[i])) {
                        return char_traits::lt(rhs.data[i], data[i]);
                    }
                }

                return true;
            }

            constexpr const_reference operator[](const size_type index) const {
                return data[index];
            }

            constexpr hash_t hash() const {
                return fnv1a_hash(
                    size_of<value_type>(_length),
                    reinterpret_cast<const uint8_t*>(data)
                );
            }

            constexpr const_iterator begin() const noexcept { return data; }
            constexpr const_iterator end() const noexcept { return data + _length; }

            constexpr const_iterator cbegin() const noexcept { return data; }
            constexpr const_iterator cend() const noexcept { return data + _length; }

            constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{data + _length}; }
            constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{data}; }

            constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{data + _length}; }
            constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{data}; }

            void destroy() {
                if(data) {
                    alloc::get_allocator()->deallocate(data);
                    data = nullptr;
                    _length = 0;
                }
            }

            constexpr bool is_empty() const noexcept { return _length == 0; }
            constexpr allocator_type* get_allocator() const noexcept { return alloc::get_allocator(); }
            constexpr allocation_flags get_allocation_flags() const noexcept { return alloc_flags; }
    };
}

#endif // CCL_STRING_BASE_STRING_HPP
