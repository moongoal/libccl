/**
 * @file
 *
 * A pool of items of the same type.
 */
#ifndef CCL_POOL_HPP
#define CCL_POOL_HPP

#include <utility>
#include <ccl/api.hpp>
#include <ccl/concepts.hpp>
#include <ccl/exceptions.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/paged-vector.hpp>

namespace ccl {
    /**
     * A pool of items stored in semi-contiguous memory.
     *
     * @tparam T The value type.
     * @tparam Allocator The allocator type.
     */
    template<typename T, typed_allocator<T> Allocator = allocator>
    requires (
        std::is_default_constructible_v<T>,
        typed_allocator<Allocator, T*>
    ) class pool final {
        public:
            using value_type = T;
            using pointer = T*;
            using allocator_type = Allocator;
            using available_type = vector<pointer, Allocator>;
            using item_collection_type = paged_vector<T, T*, Allocator>;

        private:
            available_type available;
            item_collection_type items;

        public:
            pool(
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS,
                allocator_type * const allocator = nullptr
            ) noexcept : available{alloc_flags, allocator}, items{alloc_flags, allocator}
            {}

            void destroy() {
                items.destroy();
                available.destroy();
            }

            /**
             * Acquire an item. The new item will not be newly constructed.
             * This function will never return a null pointer.
             *
             * @return A pointer to the newly acquired item.
             */
            pointer acquire() noexcept(!exceptions_enabled) {
                if(available.is_empty()) CCLUNLIKELY {
                    return &items.emplace_back();
                }

                auto ptr_it = available.end() - 1;
                pointer const ptr = *ptr_it;

                available.erase(ptr_it);

                return ptr;
            }

            /**
             * Release an item. The item will not be destroyed but its
             * memory location will be made available for re-use.
             *
             * @param ptr The pointer to the item to release.
             */
            void release(const pointer ptr) noexcept(!exceptions_enabled) {
                available.push_back(ptr);
            }
    };
}

#endif // CCL_POOL_HPP
