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
#include <functional>
#include <iterator>
#include <algorithm>
#include <initializer_list>
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
        constexpr pointer operator->() const noexcept { return ptr; }

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

        constexpr vector_iterator& operator --() noexcept {
            --ptr;
            return *this;
        }

        constexpr vector_iterator operator --(int) noexcept {
            return ptr--;
        }

        constexpr vector_iterator& operator ++() noexcept {
            ++ptr;
            return *this;
        }

        constexpr vector_iterator operator ++(int) noexcept {
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
            using rvalue_reference = T&&;
            using const_reference = const T&;
            using allocator_type = Allocator;
            using iterator = vector_iterator<vector>;
            using const_iterator = vector_iterator<vector<const value_type, allocator_type>>;

        private:
            size_type length = 0;
            size_type capacity = 0;
            value_type * data = nullptr;
            allocator_type * allocator = nullptr;

            /**
             * Make room for insertion.
             *
             * @return The iterator where to place the new item.
             */
            constexpr iterator make_room(iterator it) {
                CCL_ASSERT(it >= begin() || it <= end());

                const size_type index = std::to_address(it) - data;
                reserve(length + 1);
                it = { data + index };

                if(it < end()) {
                    std::move_backward(
                        it,
                        end(),
                        end() + 1
                    );
                }

                length += 1;

                return it;
            }

        public:
            explicit constexpr vector(
                allocator_type * const allocator = nullptr
            ) : allocator{allocator ? allocator : get_default_allocator<allocator_type>()}
            {}

            constexpr vector(const vector &other) : vector{other.allocator} {
                const auto end_it = other.end();

                reserve(other.length);
                std::uninitialized_copy(other.begin(), other.end(), begin());
                length = other.length;
            }

            constexpr vector(vector &&other)
                : length{other.length},
                capacity{other.capacity},
                data{other.data},
                allocator{other.allocator}
            {
                other.data = nullptr;
            }

            explicit vector(
                std::initializer_list<T> values,
                allocator_type * const allocator = nullptr
            ) : vector{allocator} {
                reserve(values.size());
                std::uninitialized_copy(values.begin(), values.end(), begin());
                length = values.size();
            }

            ~vector() {
                if constexpr(std::is_destructible_v<T>) {
                    const auto it_end = end();

                    if(data) {
                        std::destroy(begin(), end());
                    }
                }

                allocator->free(data);
            }

            constexpr size_type get_length() const noexcept { return length; }
            constexpr size_type get_capacity() const noexcept { return capacity; }
            constexpr pointer get_data() const noexcept { return data; }

            constexpr void reserve(const size_type new_capacity) {
                if(new_capacity > capacity) {
                    const size_type actual_new_capacity = increase_capacity(capacity, new_capacity);
                    value_type * const new_data = allocator->template allocate<value_type>(actual_new_capacity);

                    std::uninitialized_move(begin(), end(), new_data);
                    allocator->free(data);

                    data = new_data;
                    capacity = actual_new_capacity;
                }
            }

            constexpr void insert(iterator where, const_reference item) {
                CCL_THROW_IF(where < begin() || where > end(), std::out_of_range{"Iterator out of range."});

                where = make_room(where);

                CCL_ASSERT(where >= begin() || where <= end());

                std::construct_at(
                    std::to_address(where),
                    item
                );
            }

            constexpr void insert(iterator where, rvalue_reference item) {
                CCL_THROW_IF(where < begin() || where > end(), std::out_of_range{"Iterator out of range."});

                where = make_room(where);

                CCL_ASSERT(where >= begin() || where <= end());

                std::construct_at(
                    std::to_address(where),
                    std::move(item)
                );
            }

            constexpr void prepend(const_reference item) { insert(begin(), item); }
            constexpr void prepend(rvalue_reference item) { insert(begin(), std::move(item)); }

            constexpr void append(const_reference item) { insert(end(), item); }
            constexpr void append(rvalue_reference item) { insert(end(), std::move(item)); }

            constexpr reference operator[](const size_type index) {
                CCL_THROW_IF(index < 0 || index >= length, std::out_of_range{"Index out of range."});

                return data[index];
            }

            constexpr const_reference operator[](const size_type index) const {
                CCL_THROW_IF(index < 0 || index >= length, std::out_of_range{"Index out of range."});

                return data[index];
            }

            constexpr void clear() noexcept {
                if constexpr(std::is_destructible_v<T>) {
                    std::for_each(begin(), end(), [](reference item) { item.~T(); });
                }

                length = 0;
            }

            constexpr void resize(const size_type new_length) {
                CCL_THROW_IF(new_length < 1, std::invalid_argument{"Size can't be less than 1."});

                if(new_length > length) {
                    reserve(new_length);

                    std::uninitialized_default_construct(
                        begin() + length,
                        begin() + new_length
                    );
                } else if(new_length < length) {
                    std::destroy(
                        begin() + new_length,
                        begin() + length
                    );
                }

                length = new_length;
            }

            constexpr iterator begin() const noexcept { return data; }
            constexpr const_iterator cbegin() const noexcept { return data; }
            constexpr iterator end() const noexcept { return data + length; }
            constexpr const_iterator cend() const noexcept { return data + length; }
    };
}

#endif // CCL_VECTOR_HPP
