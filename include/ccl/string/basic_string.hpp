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
#include <ccl/vector.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/internal/optional-allocator.hpp>
#include <ccl/string/char_traits.hpp>
#include <ccl/contiguous-iterator.hpp>

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
                std::initializer_list<CharType> values,
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) : _data{values, allocator, alloc_flags} {}

            template<std::ranges::input_range InputRange>
            constexpr basic_string(
                const InputRange& input,
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) : _data{input, allocator, alloc_flags} {}

            constexpr auto capacity() const noexcept {
                return _data.capacity();
            }

            constexpr auto size() const noexcept {
                return _data.size();
            }

            constexpr auto data() const noexcept {
                return _data.data();
            }

            constexpr void push_back(const value_type c) {
                return _data.push_back(c);
            }
    };
}

#endif // CCL_STRING_BASE_STRING_HPP
