/**
 * @file
 *
 * A vector.
 */
#ifndef CCL_VECTOR_HPP
#define CCL_VECTOR_HPP

#include <type_traits>
#include <memory>
#include <cstring>
#include <cassert>
#include <functional>
#include <iterator>
#include <algorithm>
#include <charcoal/api.hpp>
#include <charcoal/allocator.hpp>
#include <charcoal/concepts.hpp>
#include <charcoal/util.hpp>

namespace ccl {
    template<typename Vector>
    struct vector_iterator {
        using iterator_category = std::contiguous_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = typename Vector::value_type;
        using pointer = typename Vector::pointer;
        using reference = typename Vector::reference;

        constexpr vector_iterator(const pointer ptr = nullptr) noexcept : ptr{ptr} {}
        constexpr vector_iterator(const vector_iterator &other) noexcept : ptr{other.ptr} {}
        constexpr vector_iterator(vector_iterator &&other) noexcept : ptr{std::move(other.ptr)} {}

        constexpr vector_iterator& operator =(const vector_iterator &other) noexcept {
            ptr = other.ptr;

            return *this;
        }

        constexpr vector_iterator& operator =(vector_iterator &&other) noexcept {
            ptr = std::move(other.ptr);

            return *this;
        }

        constexpr reference operator*() const noexcept { return *ptr; }
        constexpr reference operator->() const noexcept { return *ptr; }

        constexpr vector_iterator& operator +=(const difference_type n) noexcept {
            ptr += n;
            return *this;
        }

        constexpr vector_iterator& operator -=(const difference_type n) noexcept {
            ptr -= n;
            return *this;
        }

        constexpr vector_iterator operator +(const difference_type n) const noexcept {
            return ptr + n;
        }

        constexpr vector_iterator operator -(const difference_type n) const noexcept {
            return ptr - n;
        }

        constexpr difference_type operator -(const vector_iterator other) const noexcept {
            return ptr - other.ptr;
        }

        constexpr reference operator[](const difference_type i) const noexcept {
            return ptr[i];
        }

        constexpr bool operator ==(const vector_iterator other) const noexcept { return ptr == other.ptr; }
        constexpr bool operator !=(const vector_iterator other) const noexcept { return ptr != other.ptr; }
        constexpr bool operator >(const vector_iterator other) const noexcept { return ptr > other.ptr; }
        constexpr bool operator <(const vector_iterator other) const noexcept { return ptr < other.ptr; }
        constexpr bool operator >=(const vector_iterator other) const noexcept { return ptr >= other.ptr; }
        constexpr bool operator <=(const vector_iterator other) const noexcept { return ptr <= other.ptr; }

        constexpr vector_iterator& operator --() const noexcept {
            --ptr;
            return *this;
        }

        constexpr vector_iterator operator --(int) const noexcept {
            return ptr--;
        }

        constexpr vector_iterator& operator ++() const noexcept {
            ++ptr;
            return *this;
        }

        constexpr vector_iterator operator ++(int) const noexcept {
            return ptr++;
        }

        private:
            pointer ptr;
    };

    template<typename Vector>
    constexpr vector_iterator<Vector> operator +(
        const typename vector_iterator<Vector>::difference_type n,
        const vector_iterator<Vector> it
    ) noexcept {
        return vector_iterator<Vector>{it.get_data()};
    }

    template<
        typename T,
        typed_allocator<T> Allocator = allocator
    >
    class vector {
        static_assert(std::is_default_constructible_v<T>);

        public:
            using size_type = size_t;
            using value_type = T;
            using pointer = T*;
            using reference = T&;
            using const_reference = const T&;
            using allocator_type = Allocator;
            using iterator = vector_iterator<vector>;
            using const_iterator = vector_iterator<vector<const value_type, allocator_type>>;

        private:
            size_type length = 0;
            size_type capacity = 0;
            value_type * data = nullptr;
            allocator_type * allocator = nullptr;

            void insert_w_assign(
                const iterator it,
                reference item,
                std::function<void(reference item)> assign
            ) noexcept {
                assert(it >= begin() && it <= end());
                reserve(capacity += 1);

                if(it < end()) {
                    std::move_backward(
                        it + 1,
                        end() - 1,
                        end()
                    );
                }

                assign(item);
                length += 1;
            }

        public:
            explicit constexpr vector(
                allocator_type * const allocator = nullptr
            ) noexcept : allocator{allocator ? allocator : get_default_allocator<allocator_type>()}
            {}

            // TODO: Use iterators
            // constexpr vector(const vector &other)
            // : size{other.size}, allocator{other.allocator} {
            //     reserve(other.capacity);
            // }

            constexpr size_type get_length() const noexcept { return length; }
            constexpr size_type get_capacity() const noexcept { return capacity; }
            constexpr void* get_data() const noexcept { return data; }

            void reserve(const size_type new_capacity) noexcept {
                if(new_capacity > capacity) {
                    const size_type actual_new_capacity = increase_capacity(capacity, new_capacity);
                    value_type * const new_data = allocator->template allocate<value_type>(actual_new_capacity);

                    std::uninitialized_move(begin(), last(), new_data);
                    allocator->free(data);

                    data = new_data;
                    capacity = actual_new_capacity;
                }
            }

            void insert(const iterator where, const T& item) noexcept {
                insert_w_assign(
                    where,
                    item,
                    [this, where](reference item) { *where = item; }
                );
            }

            void insert(const iterator where, T&& item) noexcept {
                insert_w_assign(
                    where,
                    item,
                    [this, where](reference item) { *where = std::move(item); }
                );
            }

            void prepend(const T& item) noexcept { insert(begin(), item); }
            void prepend(T&& item) noexcept { insert(begin(), std::move(item)); }

            void append(const T& item) noexcept { insert(end(), item); }
            void append(T&& item) noexcept { insert(end(), std::move(item)); }

            constexpr value_type& operator[](const size_type index) noexcept {
                assert(index >= 0 && index < length);

                return data[index];
            }

            constexpr const value_type& operator[](const size_type index) const noexcept {
                assert(index >= 0 && index < length);

                return data[index];
            }

            constexpr void clear() noexcept {
                if constexpr(std::is_destructible_v<T>) {
                    std::for_each(begin(), end(), [](reference item) { item.~T(); });
                }

                length = 0;
            }

            constexpr iterator begin() const noexcept { return data; }
            constexpr const_iterator cbegin() const noexcept { return data; }
            constexpr iterator end() const noexcept { return data + length; }
            constexpr const_iterator cend() const noexcept { return data + length; }
    };
}

#endif // CCL_VECTOR_HPP
