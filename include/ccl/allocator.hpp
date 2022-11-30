/**
 * @file
 *
 * Memory allocator.
 */
#ifndef CCL_ALLOCATOR_HPP
#define CCL_ALLOCATOR_HPP

#include <ccl/api.hpp>

#ifndef CCL_USER_DEFINED_ALLOCATOR
    #include <memory>
#endif // CCL_USER_DEFINED_ALLOCATOR

namespace ccl {
    enum allocation_flags {
        #define CCL_VALUE(name, bitno) CCL_ALLOCATION_FLAGS ## name = 1 << bitno

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
                return reinterpret_cast<T*>(allocate(sizeof(T) * n, alignof(T), flags));
            }

            /**
             * Free memory.
             *
             * @param ptr A pointer allocated with this allocator.
             */
            void deallocate(void * const ptr);
    };

    /**
     * An allocator that does nothing.
     */
    class null_allocator {
        public:
            CCLNODISCARD void* allocate(const size_t n_bytes CCLUNUSED, const int flags CCLUNUSED = 0) { return nullptr; }
            CCLNODISCARD void* allocate(const size_t n_bytes CCLUNUSED, const size_t alignment CCLUNUSED, const int flags CCLUNUSED = 0) { return nullptr; }
            void free(void * const ptr CCLUNUSED) {}
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
