/**
 * @file
 *
 * A pool of raw memory for objects of a given type.
 */
#ifndef CCL_MEMORY_POOL_HPP
#define CCL_MEMORY_POOL_HPP

#include <functional>
#include <ccl/api.hpp>
#include <ccl/debug.hpp>
#include <ccl/exceptions.hpp>
#include <ccl/concepts.hpp>
#include <ccl/paged-vector.hpp>
#include <ccl/memory/allocator.hpp>

namespace ccl {
    /**
     * A pool of memory for items of the same type.
     *
     * This class keeps a collection of items and allows referencing them
     * via pointers. This class only manages raw memory, no lifetime management
     * for the stored objects is provided. Memory pools guarantee pointer
     * stability, hence all returned pointers are guaranteed to be valid for the
     * lifetime of the pool object.
     */
    template<
        typename T,
        allocation_flags AllocationFlags = 0,
        basic_allocator Allocator = allocator
    > class memory_pool {
        public:
            static constexpr size_t object_size = sizeof(T);
            static constexpr size_t object_alignment = alignof(T);
            static constexpr allocation_flags allocation_flags = AllocationFlags;

            using value_type = T;
            using pointer = T*;
            using reference = T&;
            using const_reference = const T&;
            using const_pointer = const T*;
            using allocator_type = Allocator;
            using memory_vector_type = paged_vector<uint8_t, allocation_flags, uint8_t*, allocator_type>;
            using free_stack_type = vector<pointer, allocation_flags, allocator_type>;

            static_assert(sizeof(T) <= memory_vector_type::page_size, "Object type too large.");

            /**
             * Number of objects that can be hosted in on individual page.
             */
            static constexpr size_t object_page_count = memory_vector_type::page_size / object_size;

        private:
            memory_vector_type data;
            free_stack_type free_items;

            /**
             * Add pointers to the items of a given page to the free item stack.
             */
            void add_memory_page_items(const pointer page) {
                const pointer end_ptr = page + object_page_count;

                for(pointer ptr = end_ptr - 1; ptr >= page; --ptr) {
                    free_items.push_back(ptr);
                }
            }

            /**
             * Add a new page to store more items.
             */
            void add_memory_page() {
                const size_t new_page_index = data.size();

                data.resize(data.size() + memory_vector_type::page_size);
                add_memory_page_items(
                    reinterpret_cast<pointer>(
                        &data[new_page_index]
                    )
                );
            }

        public:
            explicit constexpr memory_pool(allocator_type * const allocator = nullptr) : data{allocator} {}

            constexpr memory_pool(const memory_pool& other) = default;
            constexpr memory_pool(memory_pool&& other) noexcept = default;

            memory_pool& operator=(const memory_pool& other) = default;
            memory_pool& operator=(memory_pool&& other) noexcept = default;

            virtual ~memory_pool() = default;

            /**
             * Acquire uninitialized memory for a new item from the pool.
             *
             * @return The item pointer.
             */
            CCLNODISCARD pointer acquire() {
                if(!free_items.is_empty()) {
                    auto it = free_items.end() - 1;
                    const pointer ptr = *it;

                    free_items.erase(it);

                    return ptr;
                }

                add_memory_page();

                return acquire();
            }

            /**
             * Release memory for an item to the pool. This will not free any memory or
             * perform any object destruction. The memory will be left available for re-use
             * as-is.
             *
             * @param ptr The pointer to the item to release the memory for.
             */
            void release(const pointer ptr) {
                CCL_THROW_IF(!ptr, std::invalid_argument{"ptr can't be null."});

                free_items.push_back(ptr);
            }
    };
}

#endif // CCL_MEMORY_POOL_HPP
