/**
 * @file
 *
 * Allocator class using local memory.
 */
#ifndef CCL_LOCAL_ALLOCATOR_HPP
#define CCL_LOCAL_ALLOCATOR_HPP

#include <stdexcept>
#include <ccl/api.hpp>
#include <ccl/definitions.hpp>
#include <ccl/allocator.hpp>
#include <ccl/util.hpp>

namespace ccl {
    template<size_t Size>
    class local_allocator {
        public:
            static constexpr size_t memory_size = Size;

        private:
            uint8_t memory[memory_size];
            size_t used_size;

        public:
            local_allocator() : used_size{0} {}

            /**
             * Allocate with default alignment constraint.
             *
             * @param n_bytes Number of bytes to allocate.
             * @param flags Optional allocation flags.
             *
             * @return A pointer to the newly allocated memory.
             */
            CCLNODISCARD void* allocate(const size_t n_bytes, const int flags = 0) {
                return allocate(n_bytes, CCL_ALLOCATOR_DEFAULT_ALIGNMENT, flags);
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
            CCLNODISCARD void* allocate(const size_t n_bytes, const size_t alignment, const int flags CCLUNUSED = 0) {
                uint8_t * const aligned_memory = align_address(memory, alignment);
                const size_t padding = aligned_memory - memory;
                const size_t allocation_size = n_bytes + padding;
                const size_t remaining_size = memory_size - used_size;

                CCL_THROW_IF(allocation_size > remaining_size, std::bad_alloc{});

                used_size += allocation_size;

                return aligned_memory;
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
            CCLNODISCARD T* allocate(const size_t n, const int flags = 0) {
                return reinterpret_cast<T*>(allocate(sizeof(T) * n, alignof(T), flags));
            }

            /**
             * Get information about a memory allocation.
             *
             * @param ptr The pointer to the allocation to retrieve information for.
             *
             * @return A structure containing the information held by the allocator for this allocation.
             */
            allocation_info get_allocation_info(void* const ptr CCLUNUSED) const { return {}; }

            /**
             * Free memory.
             *
             * @param ptr A pointer allocated with this allocator.
             */
            void deallocate(void * const ptr CCLUNUSED) {}

            /**
             * Return the feature set supported by this allocator.
             *
             * @return An integer representing the set of supported allocator features.
             */
            allocator_feature_flags get_features() const noexcept { return 0; }

            /**
             * Free any allocated memory and allow reuse.
             */
            void clear() noexcept { used_size = 0; }
    };
}

#endif // CCL_LOCAL_ALLOCATOR_HPP
