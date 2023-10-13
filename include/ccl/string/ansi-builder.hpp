/**
 * @file
 *
 * ANSI string builder specialisation.
 */
#ifndef CCL_STRING_ANSI_BUILDER_HPP
#define CCL_STRING_ANSI_BUILDER_HPP

#include <ccl/api.hpp>
#include <ccl/string/builder.hpp>
#include <ccl/string/ansi-string.hpp>

namespace ccl {
    template<
        typed_allocator<char> Allocator = allocator
    > using ansi_string_builder = basic_string_builder<
        typename ansi_string<Allocator>::value_type,
        typename ansi_string<Allocator>::char_traits,
        Allocator
    >;
}

#endif // CCL_STRING_ANSI_BUILDER_HPP
