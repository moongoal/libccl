/**
 * @file
 *
 * Memory allocator.
 */
#ifndef CCL_ALLOCATOR_HPP
#define CCL_ALLOCATOR_HPP

#include <ccl/api.hpp>
#include <ccl/util.hpp>
#include <ccl/concepts.hpp>

#ifndef CCL_USER_DEFINED_ALLOCATOR
    #include <memory>
#endif // CCL_USER_DEFINED_ALLOCATOR

#ifdef CCL_ALLOCATOR_EXPORTER
    #define CCL_ALLOCAPI __attribute__ ((dllexport))
#else // CCL_ALLOCATOR_EXPORTER
    #ifdef CCL_ALLOCATOR_IMPORTER
        #define CCL_ALLOCAPI __attribute__ ((dllimport))
    #else // CCL_ALLOCATOR_IMPORTER
        #define CCL_ALLOCAPI
    #endif // CCL_ALLOCATOR_IMPORTER
#endif // CCL_ALLOCATOR_EXPORTER

namespace ccl {
    class allocator;

    allocator* CCL_ALLOCAPI get_default_allocator();

    /**
     * User-defined allocation flags.
     */
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
        std::size_t size = 0;

        /**
         * Alignment constraint used when allocating.
         */
        std::size_t alignment = 0;

        /**
         * Set of flags applying to the allocaiton.
         */
        allocation_flags flags = 0;
    };

    class CCL_ALLOCAPI allocator {
        public:
            /**
             * Allocate with default alignment constraint.
             *
             * @param n_bytes Number of bytes to allocate.
             * @param flags Optional allocation flags.
             *
             * @return A pointer to the newly allocated memory.
             */
            CCLNODISCARD void* allocate(const std::size_t n_bytes, const allocation_flags flags = 0);

            /**
             * Allocate memory.
             *
             * @param n_bytes Number of bytes to allocate.
             * @param alignment The alignment constraint.
             * @param flags Optional allocation flags.
             *
             * @return A pointer to the newly allocated memory.
             */
            CCLNODISCARD void* allocate(const std::size_t n_bytes, const std::size_t alignment, const allocation_flags flags = 0);

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
            allocation_info get_allocation_info(const void* const ptr) const;

            /**
             * Free memory.
             *
             * @param ptr A pointer allocated with this allocator.
             */
            void deallocate(void * const ptr);

            /**
             * Free memory.
             *
             * @param ptr A pointer allocated with this allocator.
             */
            template<typename T>
            void deallocate(T * const ptr) {
                deallocate(
                    reinterpret_cast<void*>(
                        const_cast<std::decay_t<T>*>(ptr)
                    )
                );
            }

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
    };

    /**
     * An allocator that does nothing.
     */
    class null_allocator {
        public:
            CCLNODISCARD void* allocate(const std::size_t n_bytes CCLUNUSED, const allocation_flags flags CCLUNUSED = 0) { return nullptr; }
            CCLNODISCARD void* allocate(const std::size_t n_bytes CCLUNUSED, const std::size_t alignment CCLUNUSED, const allocation_flags flags CCLUNUSED = 0) { return nullptr; }
            void deallocate(void * const ptr CCLUNUSED) {}

            template<typename T>
            CCLNODISCARD T* allocate(const std::size_t n CCLUNUSED, const allocation_flags flags CCLUNUSED = 0) { return nullptr; }

            allocator_feature_flags get_features() const { return 0; }
            CCLNODISCARD bool owns(const void * const ptr CCLUNUSED) const { return false; }
            allocation_info get_allocation_info(const void* const ptr CCLUNUSED) const { return {}; }
    };

    #ifndef CCL_USER_DEFINED_ALLOCATOR
        inline void *allocator::allocate(const std::size_t n_bytes, const allocation_flags flags CCLUNUSED) {
            return ::malloc(n_bytes);
        }

        inline void *allocator::allocate(const std::size_t n_bytes, const std::size_t alignment CCLUNUSED, const allocation_flags flags CCLUNUSED) {
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

    /**
     * Set the default memory allocator.
     *
     * @param allocator The new memory allocator.
     */
    void CCL_ALLOCAPI set_default_allocator(allocator &allocator);

    /**
     * Get the default memory allocator.
     */
    allocator * get_default_allocator();

    /**
     * Get a default allocator for a given type.
     * Override this function to return the allocator for the object type.
     */
    template<basic_allocator Allocator>
    Allocator * get_default_allocator() {
        return nullptr;
    }

    template<>
    inline allocator * get_default_allocator<allocator>() {
        return get_default_allocator();
    }
}

#ifdef CCL_ALLOCATOR_IMPL
    #include <ccl/memory/default-allocator-impl.hpp>
#endif // CCL_ALLOCATOR_IMPL

#endif // CCL_ALLOCATOR_HPP
