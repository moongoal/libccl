/**
 * @file
 *
 * Generic string of characters.
 */
#ifndef CCL_STRING_BASE_STRING_HPP
#define CCL_STRING_BASE_STRING_HPP

#include <ccl/api.hpp>
#include <ccl/concepts.hpp>
#include <ccl/vector.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/string/char_traits.hpp>

namespace ccl {
    template<
        typename CharType,
        char_traits_impl<CharType> CharTraits,
        typed_allocator<CharType> allocator
    > class basic_string : public vector<CharType, allocator> {
        using vec = vector<CharType, allocator>;

        public:
            using typename vec::value_type;
            using typename vec::reference;
            using typename vec::const_reference;
            using typename vec::pointer;
            using typename vec::size_type;
            using typename vec::allocator_type;
            using typename vec::iterator;
            using typename vec::const_iterator;
            using typename vec::reverse_iterator;
            using typename vec::const_reverse_iterator;

            using char_traits = CharTraits;
    };
}

#endif // CCL_STRING_BASE_STRING_HPP
