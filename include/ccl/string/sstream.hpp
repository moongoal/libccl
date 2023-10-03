/**
 * @file
 *
 * Basic string stream.
 */
#ifndef CCL_STRING_SSTREAM_HPP
#define CCL_STRING_SSTREAM_HPP

#include <ccl/api.hpp>
#include <ccl/vector.hpp>
#include <ccl/concepts.hpp>
#include <ccl/string/basic-string.hpp>
#include <ccl/string/char-traits.hpp>

namespace ccl {
    template<
        typename CharType,
        char_traits_impl<CharType> CharTraits,
        typed_allocator<CharType> Allocator
    > class basic_sstream {
        public:
            using value_type = CharType;
            using char_traits = CharTraits;
            using allocator_type = Allocator;
            using string_type = basic_string<value_type, char_traits, allocator_type>;
            using char_vector_type = vector<CharType, Allocator>;

        private:
            char_vector_type _data;

        public:
            explicit constexpr basic_sstream(
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) : _data{allocator, alloc_flags}
            {}

            explicit constexpr basic_sstream(
                const allocation_flags alloc_flags
            ) : basic_sstream{nullptr, alloc_flags}
            {}

            constexpr basic_sstream(const basic_sstream &other) : _data{other._data} {}
            constexpr basic_sstream(basic_sstream &&other) : _data{std::move(other._data)} {}
    };
}

#endif // CCL_STRING_SSTREAM_HPP
