/**
 * @file
 *
 * Optionally resident allocator.
 *
 * This mixin is meant to be used by containers and other allocator-aware
 * facilities and avoids allocating extra space for non-stateful allocators.
 */
#ifndef CCL_INTERNAL_OPTIONAL_ALLOCATOR_HPP
#define CCL_INTERNAL_OPTIONAL_ALLOCATOR_HPP

#include <type_traits>
#include <ccl/api.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/concepts.hpp>

namespace ccl::internal {
    template<typename Allocator, bool IsAllocatorEmpty = std::is_empty_v<Allocator>>
    struct with_optional_allocator {
        static_assert(basic_allocator<Allocator>);

        using allocator_type = Allocator;

        protected:
            constexpr with_optional_allocator(allocator_type * const allocator = nullptr) noexcept : allocator{allocator ? allocator : get_default_allocator<allocator_type>()} {}
            constexpr with_optional_allocator(const with_optional_allocator &) = default;
            constexpr with_optional_allocator(with_optional_allocator &&) = default;
            ~with_optional_allocator() = default;

            constexpr with_optional_allocator& operator =(const with_optional_allocator &) noexcept = default;
            constexpr with_optional_allocator& operator =(with_optional_allocator &&) noexcept = default;

            constexpr with_optional_allocator& operator =(allocator_type * const allocator) noexcept {
                this->allocator = allocator;

                return *this;
            }

            allocator_type *get_allocator() const noexcept { return allocator; }

        private:
            allocator_type *allocator = nullptr;
    };

    template<typename Allocator>
    struct with_optional_allocator<Allocator, true> : private Allocator {
        static_assert(basic_allocator<Allocator>);

        using allocator_type = Allocator;

        protected:
            constexpr with_optional_allocator(allocator_type * const allocator CCLUNUSED = nullptr) noexcept {}
            constexpr with_optional_allocator(const with_optional_allocator &) = default;
            constexpr with_optional_allocator(with_optional_allocator &&) = default;
            ~with_optional_allocator() = default;

            constexpr with_optional_allocator& operator =(const with_optional_allocator &) noexcept = default;
            constexpr with_optional_allocator& operator =(with_optional_allocator &&) noexcept = default;

            constexpr with_optional_allocator& operator =(allocator_type * const allocator CCLUNUSED) noexcept {
                return *this;
            }

            allocator_type *get_allocator() const noexcept {
                return const_cast<allocator_type*>(
                    static_cast<const allocator_type*>(this)
                );
            }

            allocator_type *get_allocator() noexcept {
                return static_cast<allocator_type*>(this);
            }
    };
}

#endif // CCL_INTERNAL_OPTIONAL_ALLOCATOR_HPP
