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
        char_traits_impl<CharType> CharTraits,
        typed_allocator<CharType> Allocator
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
            size_type _size = 0;
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
                data{alloc::get_allocator()->template allocate<value_type>(other._size, other.alloc_flags)},
                _size{other._size},
                alloc_flags{other.alloc_flags}
            {
                char_traits::copy(data, other.data, _size);
            }

            constexpr basic_string(
                const_pointer raw,
                const size_type size,
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) :
                alloc{allocator},
                data{alloc::get_allocator()->template allocate<value_type>(size, alloc_flags)},
                _size{size},
                alloc_flags{alloc_flags}
            {
                char_traits::copy(data, raw, _size);
            }

            constexpr basic_string(
                const_pointer raw,
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ): basic_string{raw, static_cast<size_type>(char_traits::length(raw)), allocator, alloc_flags}
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
                std::swap(_size, other._size);
                std::swap(alloc_flags, other.alloc_flags);
            }

            constexpr auto size() const noexcept {
                return _size;
            }

            constexpr const_pointer raw() const noexcept {
                return data;
            }

            constexpr bool operator==(const basic_string& rhs) const {
                if(_size != rhs._size) {
                    return false;
                }

                for(size_type i = 0; i < _size; ++i) {
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
                const size_type sz = min(_size, rhs._size);

                for(size_type i = 0; i < sz; ++i) {
                    if(!char_traits::eq(data[i], rhs.data[i])) {
                        return char_traits::lt(data[i], rhs.data[i]);
                    }
                }

                return false;
            }

            constexpr bool operator<=(const basic_string& rhs) const {
                const size_type sz = min(_size, rhs._size);

                for(size_type i = 0; i < sz; ++i) {
                    if(!char_traits::eq(data[i], rhs.data[i])) {
                        return char_traits::lt(data[i], rhs.data[i]);
                    }
                }

                return true;
            }

            constexpr bool operator>(const basic_string& rhs) const {
                const size_type sz = min(_size, rhs._size);

                for(size_type i = 0; i < sz; ++i) {
                    if(!char_traits::eq(data[i], rhs.data[i])) {
                        return char_traits::lt(rhs.data[i], data[i]);
                    }
                }

                return false;
            }

            constexpr bool operator>=(const basic_string& rhs) const {
                const size_type sz = min(_size, rhs._size);

                for(size_type i = 0; i < sz; ++i) {
                    if(!char_traits::eq(data[i], rhs.data[i])) {
                        return char_traits::lt(rhs.data[i], data[i]);
                    }
                }

                return true;
            }

            constexpr decltype(auto) operator[](const size_type index) const {
                return data[index];
            }

            constexpr hash_t hash() const {
                return fnv1a_hash(
                    size_of<value_type>(_size),
                    reinterpret_cast<const uint8_t*>(data)
                );
            }

            constexpr iterator begin() noexcept { return data; }
            constexpr iterator end() noexcept { return data + _size; }

            constexpr const_iterator begin() const noexcept { return data; }
            constexpr const_iterator end() const noexcept { return data + _size; }

            constexpr const_iterator cbegin() const noexcept { return data; }
            constexpr const_iterator cend() const noexcept { return data + _size; }

            constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{data + _size}; }
            constexpr reverse_iterator rend() noexcept { return reverse_iterator{data}; }

            constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{data + _size}; }
            constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{data}; }

            constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{data + _size}; }
            constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{data}; }

            void destroy() {
                if(data) {
                    alloc::get_allocator()->deallocate(data);
                    data = nullptr;
                    _size = 0;
                }
            }

            constexpr bool is_empty() const noexcept { return _size == 0; }
    };
}

#endif // CCL_STRING_BASE_STRING_HPP
