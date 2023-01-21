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
        using difference_type = std::ptrdiff_t;
    };

    template<typename T>
    struct is_boolean { static constexpr bool value = false; };

    template<>
    struct is_boolean<bool> { static constexpr bool value = true; };

    template<>
    struct is_boolean<const bool> { static constexpr bool value = true; };

    template<typename T>
    static constexpr decltype(auto) is_boolean_v = is_boolean<T>::value;
}

#endif // CCL_TYPE_TRAITS_HPP
