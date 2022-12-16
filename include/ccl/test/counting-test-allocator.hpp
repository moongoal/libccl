/**
 * @file
 *
 * A test allocator keeping track of the number of allocations.
 *
 * This allocator will crash the application if the amount of
 * allocations is not the same of the amount of deallocations
 * when it's destroyed.
 */
#ifndef CCL_TEST_COUNTING_TEST_ALLOCATOR_HPP
#define CCL_TEST_COUNTING_TEST_ALLOCATOR_HPP

#include <cstdlib>
#include <ccl/api.hpp>
#include <ccl/memory/allocator.hpp>

namespace ccl {
    class counting_test_allocator {
        std::size_t count = 0;

        public:
            ~counting_test_allocator() {
                if(count) { // Memory leak detected
                    std::abort();
                }
            }

            CCLNODISCARD void* allocate(const std::size_t n_bytes, const int flags = 0) {
                count++;
                return get_default_allocator()->allocate(n_bytes, flags);
            }

            CCLNODISCARD void* allocate(const std::size_t n_bytes, const std::size_t alignment, const int flags = 0) {
                count++;
                return get_default_allocator()->allocate(n_bytes, alignment, flags);
            }

            template<typename T>
            CCLNODISCARD T* allocate(const std::size_t n, const int flags = 0) {
                return reinterpret_cast<T*>(allocate(size_of<T>(n), alignof(T), flags));
            }

            allocation_info get_allocation_info(const void* const ptr) const {
                return get_default_allocator()->get_allocation_info(ptr);
            }

            void deallocate(void * const ptr) {
                count--;
                return get_default_allocator()->deallocate(ptr);
            }

            CCLNODISCARD bool owns(const void * const ptr) const {
                return get_default_allocator()->owns(ptr);
            }

            allocator_feature_flags get_features() const noexcept {
                return get_default_allocator()->get_features();
            }

            static counting_test_allocator* get_default();
    };

    #ifdef CCL_ALLOCATOR_IMPL
        static counting_test_allocator s_counting_allocator;

        template<>
        counting_test_allocator* get_default_allocator() {
            return &s_counting_allocator;
        }
    #endif // CCL_ALLOCATOR_IMPL
}

#endif // CCL_TEST_COUNTING_TEST_ALLOCATOR_HPP
