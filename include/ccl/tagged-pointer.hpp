/**
 * @file
 *
 * Tagged pointer implementation.
 */
#ifndef CCL_TAGGED_POINTER_HPP
#define CCL_TAGGED_POINTER_HPP

#include <ccl/api.hpp>
#include <ccl/exceptions.hpp>
#include <ccl/debug.hpp>
#include <ccl/util.hpp>

namespace ccl {
    /**
     * A pointer object with a tag value stored in
     * its unused LSBs. The size of the tag area depends
     * on the alignment of the pointer.
     *
     * @tparam T The value type.
     * @tparam Alignment The value alignment boundary.
     */
    template<typename T, uint32_t Alignment = alignof(T)>
    class tagged_pointer {
        static_assert(Alignment > 0);
        static_assert(is_power_2(Alignment));

        public:
            using value_type = T;
            using pointer = T*;

            /**
             * Data alignment, in bytes.
             */
            static constexpr uint32_t alignment = Alignment;

            /**
             * Tag mask.
             */
            static constexpr std::uintptr_t tag_mask = alignment - 1;

            /**
             * Address mask.
             */
            static constexpr std::uintptr_t address_mask = ~tag_mask;

            /**
             * Max value settable to the tag.
             */
            static constexpr uint32_t max_tag_value = tag_mask;

        private:
            std::uintptr_t ptr = 0;

        public:
            /**
             * Initialise a null pointer.
             */
            constexpr tagged_pointer() = default;

            /**
             * Initialise a pointer by specifying its components.
             *
             * @param address The pointer to the value.
             * @param tag The tag value.
             */
            constexpr tagged_pointer(const pointer address, const uint32_t tag) {
                set_address(address);
                set_tag(tag);
            }

            /**
             * Set the address portion of the pointer.
             *
             * @param value The new address.
             */
            constexpr void set_address(const pointer value) {
                CCL_THROW_IF(!is_address_aligned(value), std::invalid_argument{"Tagged pointer not aligned."});

                ptr = reinterpret_cast<std::uintptr_t>(value) | tag();
            }

            /**
             * Set the tag portion of the pointer.
             *
             * @param value The new tag.
             */
            constexpr void set_tag(const uint32_t value) {
                CCL_THROW_IF(value > max_tag_value, std::out_of_range{"Tag value too large"});

                ptr = reinterpret_cast<std::uintptr_t>(address()) | value;
            }

            /**
             * Get the address portion of the pointer.
             *
             * @return The address.
             */
            constexpr pointer address() const {
                return reinterpret_cast<pointer>(ptr & address_mask);
            }

            /**
             * Get the tag portion of the pointer.
             *
             * @return The tag.
             */
            constexpr uint32_t tag() const {
                return ptr & tag_mask;
            }

            /**
             * Get the raw tagged pointer value comprising both
             * the address and the tag, packed together.
             *
             * @return The raw tagged pointer value.
             */
            constexpr std::uintptr_t raw() const {
                return ptr;
            }

            explicit constexpr operator pointer() const {
                return address();
            }

            constexpr pointer operator->() const {
                return address();
            }

            constexpr T& operator*() const {
                return *address();
            }
    };

    template<typename T, uint32_t Alignment = alignof(T)>
    static constexpr bool operator==(
        const tagged_pointer<T, Alignment> a,
        const tagged_pointer<T, Alignment> b
    ) {
        return a.raw() == b.raw();
    }

    template<typename T, uint32_t Alignment = alignof(T)>
    static constexpr bool operator>(
        const tagged_pointer<T, Alignment> a,
        const tagged_pointer<T, Alignment> b
    ) {
        return a.raw() > b.raw();
    }

    template<typename T, uint32_t Alignment = alignof(T)>
    static constexpr bool operator<(
        const tagged_pointer<T, Alignment> a,
        const tagged_pointer<T, Alignment> b
    ) {
        return a.raw() < b.raw();
    }

    template<typename T, uint32_t Alignment = alignof(T)>
    static constexpr bool operator>=(
        const tagged_pointer<T, Alignment> a,
        const tagged_pointer<T, Alignment> b
    ) {
        return a.raw() >= b.raw();
    }

    template<typename T, uint32_t Alignment = alignof(T)>
    static constexpr bool operator<=(
        const tagged_pointer<T, Alignment> a,
        const tagged_pointer<T, Alignment> b
    ) {
        return a.raw() <= b.raw();
    }
}

#endif // CCL_TAGGED_POINTER_HPP
