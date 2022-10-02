/**
 * @file
 *
 * A vector.
 */
#ifndef CCL_VECTOR_HPP
#define CCL_VECTOR_HPP

#include <type_traits>
#include <memory>
#include <cassert>
#include <functional>
#include <charcoal/api.hpp>
#include <charcoal/allocator.hpp>
#include <charcoal/concepts.hpp>

namespace ccl {
    template<
        typename T,
        typed_allocator<T> Allocator
    >
    class vector {
        static_assert(std::is_default_constructible_v<T>);

        public:
            using size_type = size_t;
            using value_type = T;
            using allocator_type = Allocator;

        private:
            size_type length = 0;
            size_type capacity = 0;
            value_type * data = nullptr;
            allocator_type * allocator = nullptr;

            static void move_or_copy_item(T& destination, T& source) {
                if constexpr(std::is_move_assignable_v<T>) {
                    destination = std::move(source);
                } else {
                    destination = source;
                }
            }

            static void move_or_copy_slice(T* const destination, T* const source, const size_type count) {
                if constexpr(std::is_trivially_copyable_v<T>) {
                    ::memmove(destination, source, sizeof(T) * count);
                } else {
                    for(
                        T* ptr_src = source + count - 1, ptr_dst = destination + count - 1;
                        ptr_src >= source;
                        --ptr_src, --ptr_dst
                    ) {
                        *ptr_dst = *ptr_src;
                    }
                }
            }

            template<typename Tref>
            void insert_w_assign(
                const size_type index,
                Tref item,
                std::function<void()> assign
            ) {
                assert(index >= 0 && index <= length);
                reserve(capacity += 1);

                if(index < length) {
                    move_or_copy_slice(
                        data + index + 1,
                        data + index,
                        length - index
                    );
                }

                assign();
                length += 1;
            }

        public:
            explicit constexpr vector(
                allocator_type * const allocator = nullptr
            ) : allocator{allocator ? allocator : get_default_allocator<value_type>()}
            {}

            // TODO: Use iterators
            // constexpr vector(const vector &other)
            // : size{other.size}, allocator{other.allocator} {
            //     reserve(other.capacity);
            // }

            constexpr size_type get_length() const { return length; }
            constexpr size_type get_capacity() const { return capacity; }
            constexpr void* get_data() const { return data; }

            void reserve(const size_type new_capacity) {
                if(new_capacity > capacity) {
                    value_type * const new_data = allocator->template allocate<value_type>(new_capacity);

                    for(size_t i = 0; i < length; ++i) {
                        move_or_copy_item(new_data[i], data[i]);
                    }

                    allocator->free(data);
                    data = new_data;
                }
            }

            void insert(const size_type index, const T& item) {
                insert_w_assign(
                    index,
                    item,
                    [this, index, &item]() { data[index] = item; }
                );
            }

            void insert(const size_type index, T&& item) {
                insert_w_assign(
                    index,
                    item,
                    [this, index, &item]() { data[index] = std::move(item); }
                );
            }

            void append(const T& item) { insert(length, item); }
            void append(T&& item) { insert(length, std::move(item)); }

            constexpr value_type& operator[](const size_type index) {
                assert(index >= 0 && index < length);

                return data[index];
            }

            constexpr const value_type& operator[](const size_type index) const {
                assert(index >= 0 && index < length);

                return data[index];
            }
    };
}

#endif // CCL_VECTOR_HPP
