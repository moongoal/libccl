/**
 * @file
 *
 * Type traits library.
 */
#ifndef CCL_TYPE_TRAITS_HPP
#define CCL_TYPE_TRAITS_HPP

#include <ccl/api.hpp>
#include <type_traits>

namespace ccl {
    template<typename T>
    struct pointer_traits {};

    template<typename T>
    struct pointer_traits<T*> {
        using element_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;
        using const_reference = const T&;
        using difference_type = ptrdiff_t;
    };
}

#endif // CCL_TYPE_TRAITS_HPP
