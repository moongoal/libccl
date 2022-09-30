/**
 * @file
 *
 * A vector.
 */
#ifndef CCL_VECTOR_HPP
#define CCL_VECTOR_HPP

#include <charcoal/api.hpp>
#include <charcoal/allocator.hpp>
#include <charcoal/concepts.hpp>

namespace ccl {
    template<
        typename T,
        typed_allocator<T> Allocator
    >
    class vector {
        public:
            using size_type = size_t;
            using value_type = T;
            using allocator_type = Allocator;

        private:
            size_type size = 0;
            size_type capacity = 0;
            value_type * data = nullptr;
            allocator_type * allocator = nullptr;

        public:
            explicit constexpr vector(
                allocator_type * const allocator = nullptr
            ) : allocator{allocator ? allocator : get_default_allocator<value_type>()}
            {}

            void reserve(const size_t new_capacity) {
                if(new_capacity > capacity) {
                    value_type * const new_data = allocator->template allocate<value_type>(new_capacity);

                    for(size_t i = 0; i < size; ++i) {
                        new_data[i] = std::move(data[i]);
                    }

                    allocator->free(data);
                    data = new_data;
                }
            }
    };
}

#endif // CCL_VECTOR_HPP
