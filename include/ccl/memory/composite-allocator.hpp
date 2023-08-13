/**
 * @file
 *
 * Allocator composed by two sub-allocators.
 */
#ifndef CCL_COMPOSITE_ALLOCATOR_HPP
#define CCL_COMPOSITE_ALLOCATOR_HPP

#include <stdexcept>
#include <ccl/api.hpp>
#include <ccl/debug.hpp>
#include <ccl/util.hpp>
#include <ccl/concepts.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/compressed-pair.hpp>
#include <ccl/memory/local-allocator.hpp>

namespace ccl {
    /**
     * Meta-allocator dispatching to sub allocators.
     *
     * Allocations will first be dispatched to the first allocator and
     * if those fail, to the second allocator. Deallocations are dispatched
     * to the owning allocator only.
     *
     * Allocator1 must support ownership queries and return `nullptr` upon
     * allocation failure.
     *
     * @tparam Allocator1 The first sub allocator.
     * @tparam Allocator2 The second sub allocator.
     */
    template<basic_allocator Allocator1, basic_allocator Allocator2>
    class composite_allocator {
        public:
            using allocator1_type = Allocator1;
            using allocator2_type = Allocator2;

        private:
            compressed_pair<allocator1_type*, allocator2_type*> allocators;

        public:
            composite_allocator(allocator1_type &allocator1, allocator2_type &allocator2) : allocators{&allocator1, &allocator2} {
                CCL_ASSERT(static_cast<void*>(&allocator1) != static_cast<void*>(&allocator2));

                CCL_THROW_IF(
                    !(allocator1.get_features() & CCL_ALLOCATOR_FEATURE_OWNERSHIP_QUERY_BIT),
                    std::invalid_argument{"Allocator 1 must support ownership queries."}
                );
            }

            /**
             * Allocate memory.
             *
             * @param n_bytes Number of bytes to allocate.
             * @param alignment The alignment constraint.
             * @param flags Optional allocation flags.
             *
             * @return A pointer to the newly allocated memory.
             */
            CCLNODISCARD void* allocate(const std::size_t n_bytes, const std::size_t alignment, const allocation_flags flags = 0) {
                void * const ptr1 = allocators.first()->allocate(n_bytes, alignment, flags);

                if(!ptr1) {
                    return allocators.second()->allocate(n_bytes, alignment, flags);
                }

                return ptr1;
            }

            /**
             * Typed allocate with default alignment constraint.
             *
             * @param n Number of objects to allocate.
             * @param flags Optional allocation flags.
             *
             * @return A pointer to the newly allocated memory.
             */
            template<typename T>
            CCLNODISCARD T* allocate(const std::size_t n, const allocation_flags flags = 0) {
                return reinterpret_cast<T*>(allocate(size_of<T>(n), alignof(T), flags));
            }

            /**
             * Get information about a memory allocation.
             *
             * @param ptr The pointer to the allocation to retrieve information for.
             *
             * @return A structure containing the information held by the allocator for this allocation.
             */
            allocation_info get_allocation_info(const void* const ptr CCLUNUSED) const { return {}; }

            /**
             * Free memory.
             *
             * @param ptr A pointer allocated with this allocator.
             */
            void deallocate(void * const ptr) {
                if(allocators.first()->owns(ptr)) {
                    allocators.first()->deallocate(ptr);
                } else {
                    allocators.second()->deallocate(ptr);
                }
            }

            /**
             * Return the feature set supported by this allocator.
             *
             * @return An integer representing the set of supported allocator features.
             */
            allocator_feature_flags get_features() const noexcept { return 0; }

            /**
             * Return a boolean value stating whether the allocator owns
             * the memory represented by the pointer.
             *
             * If the allocator does not support ownership queries, this function
             * must return false.
             *
             * @param ptr A pointer.
             *
             * @return True if the memory pointed at by `ptr` is owned by this allocator.
             */
            bool owns(const void * const ptr CCLUNUSED) const { return false; }
    };
}

#endif // CCL_COMPOSITE_ALLOCATOR_HPP
