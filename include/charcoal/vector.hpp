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
#include <vector>
#include <charcoal/api.hpp>
#include <charcoal/allocator.hpp>
#include <charcoal/concepts.hpp>

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

        constexpr bool operator ==(const vector_iterator other) const noexcept { return ptr == other.ptr;}
        constexpr bool operator !=(const vector_iterator other) const noexcept { return ptr != other.ptr;}
        constexpr bool operator >(const vector_iterator other) const noexcept { return ptr > other.ptr;}
        constexpr bool operator <(const vector_iterator other) const noexcept { return ptr < other.ptr;}
        constexpr bool operator >=(const vector_iterator other) const noexcept { return ptr >= other.ptr;}
        constexpr bool operator <=(const vector_iterator other) const noexcept { return ptr <= other.ptr;}

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

            static void move_or_copy_item(T& destination, T& source) noexcept {
                if constexpr(std::is_move_assignable_v<T>) {
                    destination = std::move(source);
                } else {
                    destination = source;
                }
            }

            static void move_or_copy_slice(T* const destination, T* const source, const size_type count) noexcept {
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
            ) noexcept {
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
                    value_type * const new_data = allocator->template allocate<value_type>(new_capacity);

                    for(size_t i = 0; i < length; ++i) {
                        move_or_copy_item(new_data[i], data[i]);
                    }

                    allocator->free(data);
                    data = new_data;
                }
            }

            void insert(const size_type index, const T& item) noexcept {
                insert_w_assign(
                    index,
                    item,
                    [this, index, &item]() { data[index] = item; }
                );
            }

            void insert(const size_type index, T&& item) noexcept {
                insert_w_assign(
                    index,
                    item,
                    [this, index, &item]() { data[index] = std::move(item); }
                );
            }

            void append(const T& item) noexcept { insert(length, item); }
            void append(T&& item) noexcept { insert(length, std::move(item)); }

            constexpr value_type& operator[](const size_type index) noexcept {
                assert(index >= 0 && index < length);

                return data[index];
            }

            constexpr const value_type& operator[](const size_type index) const noexcept {
                assert(index >= 0 && index < length);

                return data[index];
            }

            constexpr iterator begin() const noexcept { return data; }
            constexpr const_iterator cbegin() const noexcept { return data; }
            constexpr iterator end() const noexcept { return data + length; }
            constexpr const_iterator cend() const noexcept { return data + length; }
    };
}

#endif // CCL_VECTOR_HPP
