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
#include <ccl/memory/allocator.hpp>
#include <ccl/util.hpp>

namespace ccl {
    enum class local_allocator_policy {
        /**
         * Throw an exception when allocation fails.
         */
        throw_exception,

        /**
         * Return `nullptr` when allocation fails.
         */
        return_nullptr
    };

    /**
     * An allocator that only uses local memory. Useful to store a
     * known, small amount of data without accessing the heap.
     *
     * @tparam Size The size of the local memory area in bytes.
     * @tparam Policy The allocator policy.
     */
    template<std::size_t Size, local_allocator_policy Policy = local_allocator_policy::throw_exception>
    class local_allocator {
        public:
            static constexpr std::size_t memory_size = Size;
            static constexpr local_allocator_policy policy = Policy;

        private:
            alignas(CCL_ALLOCATOR_DEFAULT_ALIGNMENT) uint8_t memory[memory_size];
            std::size_t used_size;

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
            CCLNODISCARD void* allocate(const std::size_t n_bytes, const allocation_flags flags = 0) {
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
            CCLNODISCARD void* allocate(const std::size_t n_bytes, const std::size_t alignment, const allocation_flags flags CCLUNUSED = 0) {
                uint8_t * const aligned_memory = align_address(memory, alignment);
                const std::size_t padding = aligned_memory - memory;
                const std::size_t allocation_size = n_bytes + padding;
                const std::size_t remaining_size = memory_size - used_size;

                if(allocation_size > remaining_size) {
                    if constexpr (policy == local_allocator_policy::throw_exception) {
                        CCL_THROW(std::bad_alloc{});
                    } else {
                        return nullptr;
                    }
                }

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
            void deallocate(void * const ptr CCLUNUSED) {}

            /**
             * Return the feature set supported by this allocator.
             *
             * @return An integer representing the set of supported allocator features.
             */
            allocator_feature_flags get_features() const noexcept { return CCL_ALLOCATOR_FEATURE_OWNERSHIP_QUERY_BIT; }

            /**
             * Free any allocated memory and allow reuse.
             */
            void clear() noexcept { used_size = 0; }

            bool owns(const void * const ptr CCLUNUSED) const {
                return ptr >= memory && ptr < (memory + memory_size);
            }

            /**
             * Return the amount of used local memory.
             */
            constexpr std::size_t get_used_memory_size() const noexcept { return used_size; }
    };

    /**
     * A local allocator specialisation made to allocate items of a specific type
     * in a local buffer, returning `nullptr` whenever the allocation fails.
     * This allocator is the preferred choice to build a PMR-like `composite_allocator`
     * specialisation.
     *
     * @tparam T The type of object to store in the buffer.
     * @tparam BufferLength The length of the internal buffer as number of elements.
     */
    template<typename T, std::size_t BufferLength>
    using local_buffering_allocator = local_allocator<sizeof(T) * BufferLength, local_allocator_policy::return_nullptr>;
}

#endif // CCL_LOCAL_ALLOCATOR_HPP
