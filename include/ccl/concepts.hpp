/**
 * @file
 *
 * Concept definitions.
 */
#ifndef CCL_CONCEPTS_HPP
#define CCL_CONCEPTS_HPP

#include <ccl/api.hpp>
#include <type_traits>
#include <concepts>

namespace ccl {
    /**
     * An allocator able to perform byte allocate/free operations.
     *
     * This concept does not requried the templated `allocate()`
     * member function to be defined.
     *
     * @tparam Allocator The allocator type.
     */
    template<typename Allocator>
    concept basic_allocator = requires(
        Allocator alloc,
        const std::size_t n_bytes,
        const std::size_t alignment,
        const int flags,
        void * const ptr
    ) {
        { alloc.allocate(n_bytes, flags) } -> std::convertible_to<void*>;
        { alloc.allocate(n_bytes, alignment, flags) } -> std::convertible_to<void*>;

        alloc.deallocate(ptr);
    };

    /**
     * An allocator able to perform typed allocate/free operations.
     *
     * This concept is similar to base_allocator but does also requrie
     * the templated `allocate()` member function to be defined.
     *
     * @tparam Allocator The allocator type.
     * @tparam T The allocatable type.
     */
    template<typename Allocator, typename T>
    concept typed_allocator = requires(Allocator allocator, const std::size_t n, const int flags) {
        requires basic_allocator<Allocator>;

        { allocator.template allocate<T>(n, flags) } -> std::convertible_to<T*>;
    };
}

#endif // CCL_CONCEPTS_HPP
