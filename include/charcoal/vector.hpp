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
             * Make room for insertion by displacing existing items forward.
             *
             * @param it Iterator of the first item to displace forward.
             * @param n The number of items to displace.
             *
             * @return The iterator where to place the new item.
             */
            constexpr iterator make_room(iterator it, const size_type n = 1) {
                CCL_ASSERT(it >= begin() || it <= end());
                CCL_ASSERT(n >= 1);

                const size_type index = std::to_address(it) - data;
                reserve(length + n);
                it = { data + index };

                if(it < end()) {
                    std::move_backward(
                        it,
                        end(),
                        end() + n
                    );
                }

                length += n;

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
                other.length = 0;
                other.capacity = 0;
            }

            constexpr vector(
                std::initializer_list<T> values,
                allocator_type * const allocator = nullptr
            ) : vector{allocator} {
                reserve(values.size());
                std::uninitialized_copy(values.begin(), values.end(), begin());
                length = values.size();
            }

            template<typename InputIterator>
            requires std::input_iterator<InputIterator>
            constexpr vector(InputIterator input_begin, InputIterator input_end, allocator_type * const allocator = nullptr)
            : vector{allocator} {
                const size_type input_length = ::abs(std::distance(input_begin, input_end));

                if(input_length > 0) {
                    reserve(input_length);
                    std::uninitialized_copy(input_begin, input_end, begin());
                    length = input_length;
                }
            }

            ~vector() {
                destroy();
            }

            void destroy() noexcept {
                clear();

                allocator->free(data);
                capacity = 0;
                data = nullptr;
            }

            constexpr vector& operator =(const vector &other) {
                if(other.length > length) {
                    destroy();
                    reserve(other.length);
                    std::uninitialized_copy(other.begin(), other.end(), begin());
                } else {
                    std::copy(other.begin(), other.end(), begin());
                    std::destroy(begin() + other.get_length(), end());
                }

                length = other.get_length();

                return *this;
            }

            constexpr vector& operator =(vector &&other) {
                clear();

                length = other.length;
                capacity = other.capacity;
                data = other.data;
                allocator = other.allocator;

                other.data = nullptr;
                other.length = 0;
                other.capacity = 0;

                return *this;
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

            template <typename InputIterator>
            requires std::input_iterator<InputIterator>
            constexpr void insert(iterator where, InputIterator input_begin, InputIterator input_end) {
                CCL_THROW_IF(where < begin() || where > end(), std::out_of_range{"Iterator out of range."});

                const size_type input_length = ::abs(std::distance(input_begin, input_end));

                if(input_length > 0) {
                    where = make_room(where, input_length);

                    std::copy(input_begin, input_end, begin());
                }
            }

            template<typename ...Args>
            constexpr void emplace(iterator where, Args&& ...args) {
                CCL_THROW_IF(where < begin() || where > end(), std::out_of_range{"Iterator out of range."});

                where = make_room(where);

                CCL_ASSERT(where >= begin() || where <= end());

                std::construct_at(
                    std::to_address(where),
                    std::forward<Args...>(args...)
                );
            }

            constexpr void prepend(const_reference item) { insert(begin(), item); }
            constexpr void prepend(rvalue_reference item) { insert(begin(), std::move(item)); }

            constexpr void append(const_reference item) { insert(end(), item); }
            constexpr void append(rvalue_reference item) { insert(end(), std::move(item)); }

            template<typename ...Args>
            constexpr void append_emplace(Args&& ...args) { emplace(end(), std::forward<Args...>(args...)); }

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
                if(new_length > length) {
                    reserve(new_length);

                    std::uninitialized_default_construct(
                        begin() + length,
                        begin() + new_length
                    );
                } else if(new_length == 0) {
                    clear();
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
