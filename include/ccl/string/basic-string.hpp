/**
 * @file
 *
 * Generic string of characters.
 */
#ifndef CCL_STRING_BASE_STRING_HPP
#define CCL_STRING_BASE_STRING_HPP

#include <iterator>
#include <span>
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
        char_traits_impl<CharType> CharTraits = char_traits<CharType>,
        typed_allocator<CharType> Allocator = allocator
    > class basic_string : internal::with_optional_allocator<Allocator> {
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
            pointer _data = nullptr;
            size_type _length = 0;
            allocation_flags _alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS;

        public:
             explicit constexpr basic_string(
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS,
                allocator_type * const allocator = nullptr
            ) : alloc{allocator}, _alloc_flags{alloc_flags}
            {}

            constexpr basic_string(
                const basic_string &other
            ) :
                alloc{other.get_allocator()},
                _data{other._length ? alloc::get_allocator()->template allocate<value_type>(size_of<value_type>(other._length), other._alloc_flags) : nullptr},
                _length{other._length},
                _alloc_flags{other._alloc_flags}
            {
                char_traits::copy(_data, other._data, _length);
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
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS,
                allocator_type * const allocator = nullptr
            ) :
                alloc{allocator},
                _data{length ? alloc::get_allocator()->template allocate<value_type>(size_of<value_type>(length), alloc_flags) : nullptr},
                _length{length},
                _alloc_flags{alloc_flags}
            {
                char_traits::copy(_data, raw, _length);
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
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS,
                allocator_type * const allocator = nullptr
            ): basic_string{raw, N - 1, alloc_flags, allocator}
            {}

            constexpr basic_string(basic_string &&other) {
                swap(other);
            }

            ~basic_string() {
                destroy();
            }

            constexpr auto swap(basic_string &other) noexcept {
                alloc::swap(other);

                ccl::swap(_data, other._data);
                ccl::swap(_length, other._length);
                ccl::swap(_alloc_flags, other._alloc_flags);
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
                return _data;
            }

            constexpr bool operator==(const basic_string& rhs) const {
                if(_length != rhs._length) {
                    return false;
                }

                for(size_type i = 0; i < _length; ++i) {
                    if(_data[i] != rhs._data[i]) {
                        return false;
                    }
                }

                return true;
            }

            /**
             * Compares this string with a NUL-terminated string.
             */
            constexpr bool operator==(const_pointer rhs) const {
                if(!_data) {
                    return char_traits::eq(rhs[0], char_traits::nul());
                }

                const size_t rhs_len = ::strlen(rhs);

                if(rhs_len == _length) {
                    return char_traits::compare(rhs, _data, _length) == 0;
                }

                return false;
            }

            /**
             * Compares this string with a NUL-terminated string.
             */
            constexpr bool operator!=(const_pointer rhs) const {
                return !operator==(rhs);
            }

            constexpr bool operator!=(const basic_string& rhs) const {
                return !basic_string::operator==(rhs);
            }

            constexpr bool operator<(const basic_string& rhs) const {
                const size_type sz = min(_length, rhs._length);

                for(size_type i = 0; i < sz; ++i) {
                    if(!char_traits::eq(_data[i], rhs._data[i])) {
                        return char_traits::lt(_data[i], rhs._data[i]);
                    }
                }

                return false;
            }

            constexpr bool operator<=(const basic_string& rhs) const {
                const size_type sz = min(_length, rhs._length);

                for(size_type i = 0; i < sz; ++i) {
                    if(!char_traits::eq(_data[i], rhs._data[i])) {
                        return char_traits::lt(_data[i], rhs._data[i]);
                    }
                }

                return true;
            }

            constexpr bool operator>(const basic_string& rhs) const {
                const size_type sz = min(_length, rhs._length);

                for(size_type i = 0; i < sz; ++i) {
                    if(!char_traits::eq(_data[i], rhs._data[i])) {
                        return char_traits::lt(rhs._data[i], _data[i]);
                    }
                }

                return false;
            }

            constexpr bool operator>=(const basic_string& rhs) const {
                const size_type sz = min(_length, rhs._length);

                for(size_type i = 0; i < sz; ++i) {
                    if(!char_traits::eq(_data[i], rhs._data[i])) {
                        return char_traits::lt(rhs._data[i], _data[i]);
                    }
                }

                return true;
            }

            constexpr basic_string& operator=(const basic_string& rhs) {
                destroy();

                alloc::operator=(rhs.get_allocator());
                _alloc_flags = rhs.get_allocation_flags();

                if(rhs._data) {
                    _data = alloc::get_allocator()->template allocate<value_type>(
                        size_of<value_type>(rhs.length()),
                        _alloc_flags
                    );

                    _length = rhs._length;

                    char_traits::copy(_data, rhs._data, _length);
                }

                return *this;
            }

            constexpr basic_string& operator=(basic_string&& rhs) {
                swap(rhs);

                return *this;
            }

            constexpr const_reference operator[](const size_type index) const {
                return _data[index];
            }

            constexpr hash_t hash() const {
                return fnv1a_hash(
                    size_of<value_type>(_length),
                    reinterpret_cast<const uint8_t*>(_data)
                );
            }

            constexpr const_iterator begin() const noexcept { return _data; }
            constexpr const_iterator end() const noexcept { return _data + _length; }

            constexpr const_iterator cbegin() const noexcept { return _data; }
            constexpr const_iterator cend() const noexcept { return _data + _length; }

            constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{_data + _length}; }
            constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{_data}; }

            constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{_data + _length}; }
            constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{_data}; }

            void destroy() {
                if(_data) {
                    alloc::get_allocator()->deallocate(_data);
                    _data = nullptr;
                    _length = 0;
                }
            }

            constexpr bool is_empty() const noexcept { return _length == 0; }
            constexpr allocator_type* get_allocator() const noexcept { return alloc::get_allocator(); }
            constexpr allocation_flags get_allocation_flags() const noexcept { return _alloc_flags; }

            /**
             * Copy the contents of this string to a NUL-terminated output.
             *
             * @param out The output value. If too short, the string will be truncated.
             */
            constexpr void to_nul_terminated(std::span<value_type> out) const {
                const size_type end = min(_length, out.size() - 1);

                for(size_type i = 0; i < end; ++i) {
                    char_traits::copy(out.data(), _data, end);
                }

                out[end] = char_traits::nul();
            }

            template<
                typename OtherAllocator
            > constexpr bool starts_with(const basic_string<value_type, char_traits, OtherAllocator> &other) const {
                if(_length < other._length) {
                    return false;
                }

                return 0 == char_traits::compare(_data, other._data, other._length);
            }

            constexpr bool starts_with(const const_pointer other, const size_type other_length) const {
                if(_length < other_length) {
                    return false;
                }

                return 0 == char_traits::compare(_data, other, other_length);
            }

            template<size_type N>
            constexpr bool starts_with(const CharType (&other)[N]) const {
                if(_length < N - 1) {
                    return false;
                }

                return 0 == char_traits::compare(_data, other, N - 1);
            }

            template<
                typename OtherAllocator
            > constexpr bool ends_with(const basic_string<value_type, char_traits, OtherAllocator> &other) const {
                if(_length < other._length) {
                    return false;
                }

                const size_type delta = _length - other._length;

                return 0 == char_traits::compare(_data + delta, other._data, other._length);
            }

            constexpr bool ends_with(const const_pointer other, const size_type other_length) const {
                if(_length < other_length) {
                    return false;
                }

                const size_type delta = _length - other_length;

                return 0 == char_traits::compare(_data + delta, other, other_length);
            }

            template<size_type N>
            constexpr bool ends_with(const value_type (&other)[N]) const {
                constexpr size_type other_length = N - 1;

                if(_length < other_length) {
                    return false;
                }

                const size_type delta = _length - other_length;

                return 0 == char_traits::compare(_data + delta, other, other_length);
            }

            template<
                typename OtherAllocator
            > constexpr int compare(const basic_string<value_type, char_traits, OtherAllocator> &other) const {
                const int result = char_traits::compare(
                    _data,
                    other._data,
                    min(_length, other._length)
                );

                return choose(
                    result,
                    choose(1, -1, _length > other._length),
                    or_(_length == other._length, result != 0)
                );
            }

            static basic_string from_nul_terminated(
                const const_pointer str,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS,
                allocator_type * const allocator = nullptr
            ) {
                const count_t len = static_cast<count_t>(::strlen(str));

                return basic_string{str, len, alloc_flags, allocator};
            }
    };
}

#endif // CCL_STRING_BASE_STRING_HPP
