/**
 * @file
 *
 * Optional allocator.
 */
#ifndef CCL_INTERNAL_OPTIONAL_ALLOCATOR_HPP
#define CCL_INTERNAL_OPTIONAL_ALLOCATOR_HPP

#include <type_traits>
#include <ccl/api.hpp>
#include <ccl/allocator.hpp>
#include <ccl/concepts.hpp>

namespace ccl::internal {
    template<typename Allocator, bool IsAllocatorEmpty = std::is_empty_v<Allocator>>
    struct with_optional_allocator {
        static_assert(basic_allocator<Allocator>);

        using allocator_type = Allocator;

        protected:
            constexpr with_optional_allocator(allocator_type * const allocator = nullptr) noexcept : allocator{allocator} {}
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
    struct with_optional_allocator<Allocator, false> : private Allocator {
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

            allocator_type *get_allocator() const noexcept { return this; }
    };
}

#endif // CCL_INTERNAL_OPTIONAL_ALLOCATOR_HPP
