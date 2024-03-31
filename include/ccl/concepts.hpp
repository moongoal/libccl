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
        const uint32_t flags,
        void * const ptr
    ) {
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

    /**
     * A type decaying to an integral type.
     */
    template<typename T>
    concept integral_decaying = std::is_integral_v<std::decay_t<T>>;

    template<typename T, typename S>
    concept streamable_out = requires(S s, T t) {
        s << t;
    };

    template<typename T, typename S>
    concept streamable_in = requires(S s, T t) {
        s >> t;
    };

    template<typename T, typename S>
    concept streamable = requires(S s, T t) {
        requires streamable_in<T, S> && streamable_out<T, S>;
    };

    /**
     * A deleter type.
     *
     * @tparam T The deleted type.
     * @tparam C The control block type.
     * @tparam D The deleter type.
     */
    template<typename D, typename T, typename C>
    concept deleter = requires(D &d, T * t, C &c) {
        d(t, c);
    };

    /**
     * A singleton objct.
     */
    template<typename T>
    concept singleton = requires() {
        { T::instance() } -> std::convertible_to<T&>;
    };

    template<typename CharTraitsType, typename CharType>
    concept char_traits_impl = requires(const CharType c, CharType &ref, const CharType &const_ref, CharType * const str, int str_sz) {
        std::integral<typename CharTraitsType::char_type>;
        std::integral<typename CharTraitsType::int_type>;

        std::is_same_v<CharType, typename CharTraitsType::char_type>;

        std::convertible_to<typename CharTraitsType::char_type, typename CharTraitsType::int_type>;
        std::convertible_to<typename CharTraitsType::int_type, typename CharTraitsType::char_type>;

        { CharTraitsType::assign(ref, const_ref) };
        { CharTraitsType::assign(str, str_sz, c) };
    };
}

#endif // CCL_CONCEPTS_HPP
