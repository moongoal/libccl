/**
 * @file
 *
 * ANSI string inmplementation.
 */
#ifndef CCL_STRING_ANSI_STRING_HPP
#define CCL_STRING_ANSI_STRING_HPP

#include <ccl/api.hpp>
#include <ccl/string/basic-string.hpp>
#include <ccl/memory/allocator.hpp>

namespace ccl {
    template<typed_allocator<char> Allocator = allocator>
    using ansi_string = basic_string<char, base_char_traits<char, int, uint32_t>, Allocator>;
}

#endif // CCL_STRING_ANSI_STRING_HPP
