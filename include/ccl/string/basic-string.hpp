/**
 * @file
 *
 * Generic string of characters.
 */
#ifndef CCL_STRING_BASE_STRING_HPP
#define CCL_STRING_BASE_STRING_HPP

#include <iterator>
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
    > class basic_string {
        using vec = vector<CharType, Allocator>;

        public:
            using value_type = typename vec::value_type;
            using reference = typename vec::reference;
            using const_reference = typename vec::const_reference;
            using pointer = typename vec::pointer;
            using const_pointer = const value_type*;
            using size_type = typename vec::size_type;
            using allocator_type = typename vec::allocator_type;
            using iterator = typename vec::iterator;
            using const_iterator = typename vec::const_iterator;
            using reverse_iterator = typename vec::reverse_iterator;
            using const_reverse_iterator = typename vec::const_reverse_iterator;

            using char_traits = CharTraits;

        private:
            vec _data;

        public:
             explicit constexpr basic_string(
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) : _data{allocator, alloc_flags}
            {}

            constexpr basic_string(const basic_string &other) : _data{other._data} {}
            constexpr basic_string(basic_string &&other) : _data{std::move(other._data)} {}

            constexpr basic_string(
                const const_pointer cstr,
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) : _data{allocator, alloc_flags} {
                const size_type sz = char_traits::length(cstr);

                _data.resize(sz + 1);
                char_traits::copy(_data.data(), cstr, sz);
                _data[sz] = char_traits::to_char_type(char_traits::nul());
            }

            constexpr basic_string(
                std::initializer_list<CharType> values,
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) : _data{allocator, alloc_flags} {
                _data.resize(values.size() + 1);
                char_traits::copy(_data.data(), values.begin(), values.size());
                _data[values.size()] = char_traits::to_char_type(char_traits::nul());
            }

            constexpr auto capacity() const noexcept {
                return choose(0, _data.capacity() - 1, _data.is_empty());
            }

            constexpr auto size() const noexcept {
                return choose(0, _data.size() - 1, _data.is_empty());
            }

            constexpr auto data() const noexcept {
                return _data.data();
            }

            constexpr const_pointer c_str() const noexcept {
                return _data.data();
            }

            constexpr void push_back(const value_type c) {
                if(_data.is_empty()) {
                    _data.reserve(2);
                    _data.push_back(c);
                } else {
                    _data[_data.size() - 1] = c;
                }

                _data.push_back(char_traits::to_char_type(char_traits::nul()));
            }

            constexpr bool operator==(const basic_string& rhs) const {
                if(_data.size() != rhs._data.size()) {
                    return false;
                }

                for(size_type i = 0; i < _data.size(); ++i) {
                    if(_data[i] != rhs._data[i]) {
                        return false;
                    }
                }

                return true;
            }

            constexpr bool operator!=(const basic_string& rhs) const {
                return !basic_string::operator==(rhs);
            }

            constexpr bool operator<(const basic_string& rhs) const {
                const size_type sz = min(_data.size(), rhs._data.size());

                for(size_type i = 0; i < sz; ++i) {
                    if(!char_traits::eq(_data[i], rhs._data[i])) {
                        return char_traits::lt(_data[i], rhs._data[i]);
                    }
                }

                return false;
            }

            constexpr bool operator<=(const basic_string& rhs) const {
                const size_type sz = min(_data.size(), rhs._data.size());

                for(size_type i = 0; i < sz; ++i) {
                    if(!char_traits::eq(_data[i], rhs._data[i])) {
                        return char_traits::lt(_data[i], rhs._data[i]);
                    }
                }

                return true;
            }

            constexpr bool operator>(const basic_string& rhs) const {
                const size_type sz = min(_data.size(), rhs._data.size());

                for(size_type i = 0; i < sz; ++i) {
                    if(!char_traits::eq(_data[i], rhs._data[i])) {
                        return char_traits::lt(rhs._data[i], _data[i]);
                    }
                }

                return false;
            }

            constexpr bool operator>=(const basic_string& rhs) const {
                const size_type sz = min(_data.size(), rhs._data.size());

                for(size_type i = 0; i < sz; ++i) {
                    if(!char_traits::eq(_data[i], rhs._data[i])) {
                        return char_traits::lt(rhs._data[i], _data[i]);
                    }
                }

                return true;
            }

            constexpr decltype(auto) operator[](const size_type index) {
                return _data[index];
            }

            constexpr decltype(auto) operator[](const size_type index) const {
                return _data[index];
            }

            constexpr hash_t hash() const {
                return fnv1a_hash(
                    size_of<value_type>(_data.size()),
                    reinterpret_cast<const uint8_t*>(_data.data())
                );
            }

            constexpr bool empty() const {
                return _data.size() == 0;
            }

            constexpr decltype(auto) begin() noexcept { return _data.begin(); }

            constexpr decltype(auto) end() noexcept {
                auto it = _data.end();

                // Account for trailing '\0'
                if(it != _data.begin()) {
                    --it;
                }

                return it;
            }

            constexpr decltype(auto) begin() const noexcept { return _data.begin(); }

            constexpr decltype(auto) end() const noexcept {
                auto it = _data.end();

                // Account for trailing '\0'
                if(it != _data.begin()) {
                    --it;
                }

                return it;
            }

            constexpr decltype(auto) cbegin() const noexcept { return _data.begin(); }

            constexpr decltype(auto) cend() const noexcept {
                auto it = _data.end();

                // Account for trailing '\0'
                if(it != _data.begin()) {
                    --it;
                }

                return it;
            }

            constexpr decltype(auto) rbegin() noexcept { return reverse_iterator{end()}; }
            constexpr decltype(auto) rend() noexcept { return reverse_iterator{begin()}; }

            constexpr decltype(auto) rbegin() const noexcept { return const_reverse_iterator{cend()}; }
            constexpr decltype(auto) rend() const noexcept { return const_reverse_iterator{cbegin()}; }

            constexpr decltype(auto) crbegin() const noexcept { return const_reverse_iterator{cend()}; }
            constexpr decltype(auto) crend() const noexcept { return const_reverse_iterator{cbegin()}; }

            constexpr void shrink_to_fit() { _data.shrink_to_fit(); }
            constexpr void clear() noexcept { _data.clear(); }
    };
}

#endif // CCL_STRING_BASE_STRING_HPP
