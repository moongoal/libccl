/**
 * @file
 *
 * Memory allocator.
 */
#ifndef CCL_ALLOCATOR_HPP
#define CCL_ALLOCATOR_HPP

#include <ccl/api.hpp>
#include <ccl/util.hpp>

#ifndef CCL_USER_DEFINED_ALLOCATOR
    #include <memory>
#endif // CCL_USER_DEFINED_ALLOCATOR

namespace ccl {
    class allocator;

    allocator* CCLAPI get_default_allocator();

    enum allocation_flag_bits {
        #define CCL_VALUE(name, bitno) CCL_ALLOCATION_ ## name ## _BIT = 1 << bitno

        /**
         * Allocate space in permanent memory. This cannot be freed.
         */
        CCL_VALUE(PERMANENT, 0),

        /**
         * Allocate space for a short-lived object.
         */
        CCL_VALUE(TEMPORARY, 1)

        #undef CCL_VALUE
    };
    typedef uint32_t allocation_flags;

    enum allocator_feature_flag_bits {
        #define CCL_VALUE(name, bitno) CCL_ALLOCATOR_FEATURE_ ## name ## _BIT = 1 << bitno

        /**
         * The return value of `allocator::get_allocation_info()` is meaningful.
         */
        CCL_VALUE(ALLOCATION_INFO, 0),

        /**
         * The return value of `allocator::owns()` is meaningful.
         */
        CCL_VALUE(OWNERSHIP_QUERY, 1)

        #undef CCL_VALUE
    };
    typedef uint32_t allocator_feature_flags;

    /**
     * Information about a memory allocation.
     */
    struct allocation_info {
        /**
         * Total size of the allocation.
         */
        size_t size;

        /**
         * Alignment constraint used when allocating.
         */
        size_t alignment;
    };

    class CCLAPI allocator {
        public:
            /**
             * Allocate with default alignment constraint.
             *
             * @param n_bytes Number of bytes to allocate.
             * @param flags Optional allocation flags.
             *
             * @return A pointer to the newly allocated memory.
             */
            CCLNODISCARD void* allocate(const size_t n_bytes, const int flags = 0);

            /**
             * Allocate memory.
             *
             * @param n_bytes Number of bytes to allocate.
             * @param alignment The alignment constraint.
             * @param flags Optional allocation flags.
             *
             * @return A pointer to the newly allocated memory.
             */
            CCLNODISCARD void* allocate(const size_t n_bytes, const size_t alignment, const int flags = 0);

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
                return reinterpret_cast<T*>(allocate(size_of<T>(n), alignof(T), flags));
            }

            /**
             * Get information about a memory allocation.
             *
             * @param ptr The pointer to the allocation to retrieve information for.
             *
             * @return A structure containing the information held by the allocator for this allocation.
             */
            allocation_info get_allocation_info(const void* const ptr) const;

            /**
             * Free memory.
             *
             * @param ptr A pointer allocated with this allocator.
             */
            void deallocate(void * const ptr);

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
            CCLNODISCARD bool owns(const void * const ptr) const;

            /**
             * Return the feature set supported by this allocator.
             *
             * @return An integer representing the set of supported allocator features.
             */
            allocator_feature_flags get_features() const noexcept;

            /**
             * Return the default allocator for this allocator type.
             *
             * @return The default allocator instance for this allocator.
             */
            static allocator* get_default() noexcept { return get_default_allocator(); }
    };

    /**
     * An allocator that does nothing.
     */
    class null_allocator {
        public:
            CCLNODISCARD void* allocate(const size_t n_bytes CCLUNUSED, const int flags CCLUNUSED = 0) { return nullptr; }
            CCLNODISCARD void* allocate(const size_t n_bytes CCLUNUSED, const size_t alignment CCLUNUSED, const int flags CCLUNUSED = 0) { return nullptr; }
            void deallocate(void * const ptr CCLUNUSED) {}

            template<typename T>
            CCLNODISCARD T* allocate(const size_t n CCLUNUSED, const int flags CCLUNUSED = 0) { return nullptr; }

            allocator_feature_flags get_features() const { return 0; }
            CCLNODISCARD bool owns(const void * const ptr CCLUNUSED) const { return false; }
            allocation_info get_allocation_info(const void* const ptr CCLUNUSED) const { return {}; }

            static null_allocator* get_default() noexcept { return nullptr; }
    };

    #ifndef CCL_USER_DEFINED_ALLOCATOR
        inline void *allocator::allocate(const size_t n_bytes, const int flags CCLUNUSED) {
            return ::malloc(n_bytes);
        }

        inline void *allocator::allocate(const size_t n_bytes, const size_t alignment CCLUNUSED, const int flags CCLUNUSED) {
            return ::malloc(n_bytes);
        }

        inline void allocator::deallocate(void * const ptr) {
            ::free(ptr);
        }

        inline allocator_feature_flags allocator::get_features() const noexcept {
            return 0;
        }

        inline allocation_info allocator::get_allocation_info(const void* const ptr CCLUNUSED) const {
            return {};
        }

        inline bool allocator::owns(const void * const ptr CCLUNUSED) const {
            return false;
        }
    #endif // CCL_USER_DEFINED_ALLOCATOR

    #ifdef CCL_ALLOCATOR_IMPL
        static allocator *s_allocator = nullptr;

        void CCLAPI set_default_allocator(allocator &allocator) {
            s_allocator = &allocator;
        }

        allocator * CCLAPI get_default_allocator() {
            return s_allocator;
        }
    #endif // CCL_ALLOCATOR_IMPL

    /**
     * Set the default memory allocator.
     *
     * @param allocator The new memory allocator.
     */
    void CCLAPI set_default_allocator(allocator &allocator);

    /**
     * Get the default memory allocator.
     */
    allocator * CCLAPI get_default_allocator();

    /**
     * Get a default allocator for a given type.
     * Override this function to return the allocator for the object type.
     */
    template<typename Allocator>
    Allocator * get_default_allocator() {
        return nullptr;
    }
}

#endif // CCL_ALLOCATOR_HPP
