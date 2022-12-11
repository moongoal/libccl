/**
 * @file
 *
 * A vector.
 */
#ifndef CCL_VECTOR_HPP
#define CCL_VECTOR_HPP

#include <type_traits>
#include <ranges>
#include <memory>
#include <cstring>
#include <functional>
#include <iterator>
#include <algorithm>
#include <initializer_list>
#include <ccl/api.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/concepts.hpp>
#include <ccl/util.hpp>
#include <ccl/internal/optional-allocator.hpp>

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

        constexpr vector_iterator& operator =(const vector_iterator &other) noexcept {
            ptr = other.ptr;

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

        pointer ptr;
    };

    template<typename Vector>
    constexpr bool operator ==(const vector_iterator<Vector> &a, const vector_iterator<Vector> &b) noexcept {
        return a.ptr == b.ptr;
    }

    template<typename Vector>
    constexpr bool operator !=(const vector_iterator<Vector> &a, const vector_iterator<Vector> &b) noexcept {
        return a.ptr != b.ptr;
    }

    template<typename Vector>
    constexpr bool operator >(const vector_iterator<Vector> &a, const vector_iterator<Vector> &b) noexcept {
        return a.ptr > b.ptr;
    }

    template<typename Vector>
    constexpr bool operator <(const vector_iterator<Vector> &a, const vector_iterator<Vector> &b) noexcept {
        return a.ptr < b.ptr;
    }

    template<typename Vector>
    constexpr bool operator >=(const vector_iterator<Vector> &a, const vector_iterator<Vector> &b) noexcept {
        return a.ptr >= b.ptr;
    }

    template<typename Vector>
    constexpr bool operator <=(const vector_iterator<Vector> &a, const vector_iterator<Vector> &b) noexcept {
        return a.ptr <= b.ptr;
    }

    template<typename Vector>
    static constexpr vector_iterator<Vector> operator +(
        const typename vector_iterator<Vector>::difference_type n,
        const vector_iterator<Vector> it
    ) noexcept {
        return vector_iterator<Vector>{it.get_data() + n};
    }

    template<
        typename T,
        typed_allocator<T> Allocator = allocator
    >
    class vector : private internal::with_optional_allocator<Allocator> {
        using alloc = internal::with_optional_allocator<Allocator>;

        struct value_init_tag_t {};
        static constexpr value_init_tag_t value_init_tag{};

        public:
            using size_type = size_t;
            using value_type = T;
            using pointer = T*;
            using reference = T&;
            using const_reference = const T&;
            using allocator_type = Allocator;
            using iterator = vector_iterator<vector>;
            using const_iterator = vector_iterator<const vector>;

        private:
            size_type _size = 0;
            size_type _capacity = 0;
            value_type * _data = nullptr;

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

                const size_type index = std::to_address(it) - _data;
                reserve(_size + n);
                it = { _data + index };

                if(it < end()) {
                    std::move_backward(
                        it,
                        end(),
                        end() + n
                    );
                }

                _size += n;

                return it;
            }

        public:
            explicit constexpr vector(
                allocator_type * const allocator = nullptr
            ) : alloc{allocator}
            {}

            constexpr vector(const vector &other) : vector{other.get_allocator()} {
                reserve(other._size);
                std::uninitialized_copy(other.begin(), other.end(), begin());
                _size = other._size;
            }

            constexpr vector(vector &&other)
                : alloc{other.get_allocator()},
                _size{other._size},
                _capacity{other._capacity},
                _data{other._data}
            {
                other._data = nullptr;
                other._size = 0;
                other._capacity = 0;
            }

            constexpr vector(
                std::initializer_list<T> values,
                allocator_type * const allocator = nullptr
            ) : alloc{allocator} {
                reserve(values.size());
                std::uninitialized_copy(values.begin(), values.end(), begin());
                _size = values.size();
            }

            template<std::ranges::input_range InputRange>
            constexpr vector(const InputRange& input, allocator_type * const allocator = nullptr)
            : vector{allocator} {
                const size_type input_size = std::abs(std::ranges::distance(input));

                if(input_size > 0) {
                    reserve(input_size);
                    _size = input_size;
                    std::ranges::uninitialized_copy(input, *this);
                }
            }

            ~vector() {
                destroy();
            }

            void destroy() noexcept {
                clear();

                if(_data) {
                    alloc::get_allocator()->deallocate(_data);
                }

                _capacity = 0;
                _data = nullptr;
            }

            constexpr vector& operator =(const vector &other) {
                if(other._size > _size) {
                    destroy();
                    reserve(other._size);
                    std::uninitialized_copy(other.begin(), other.end(), begin());
                } else {
                    std::copy(other.begin(), other.end(), begin());
                    std::destroy(begin() + other.size(), end());
                }

                _size = other.size();

                return *this;
            }

            constexpr vector& operator =(vector &&other) {
                destroy();

                internal::with_optional_allocator<Allocator>::operator =(std::move(other));

                _size = other._size;
                _capacity = other._capacity;
                _data = other._data;

                other._data = nullptr;
                other._size = 0;
                other._capacity = 0;

                return *this;
            }

            constexpr size_type size() const noexcept { return _size; }
            constexpr size_type capacity() const noexcept { return _capacity; }
            constexpr pointer data() const noexcept { return _data; }

            constexpr void reserve(const size_type new_capacity) {
                if(new_capacity > _capacity) {
                    const size_type actual_new_capacity = increase_capacity(_capacity, new_capacity);
                    value_type * const new_data = alloc::get_allocator()->template allocate<value_type>(actual_new_capacity);

                    if(_data) {
                        std::uninitialized_move(begin(), end(), new_data);
                        alloc::get_allocator()->deallocate(_data);
                    }

                    _data = new_data;
                    _capacity = actual_new_capacity;
                }
            }

            constexpr void insert(iterator where, const_reference item) {
                CCL_THROW_IF(where < begin() || where > end(), std::out_of_range{"Iterator out of range."});

                const bool must_assign = where != end();
                where = make_room(where);

                CCL_ASSERT(where >= begin() || where <= end());

                if(must_assign) {
                    *where = item;
                } else {
                    std::construct_at(std::to_address(where), std::move(item));
                }
            }

            template <std::ranges::input_range InputRange>
            constexpr void insert(iterator where, const InputRange& input) {
                CCL_THROW_IF(where < begin() || where > end(), std::out_of_range{"Iterator out of range."});

                const size_type input_size = std::abs(std::ranges::distance(input));

                if(input_size > 0) {
                    where = make_room(where, input_size);

                    std::ranges::copy(input, where);
                }
            }

            template<typename ...Args>
            constexpr reference emplace_at(iterator where, Args&& ...args) {
                CCL_THROW_IF(where < begin() || where > end(), std::out_of_range{"Iterator out of range."});

                const bool must_destroy = where != end();
                where = make_room(where);

                CCL_ASSERT(where >= begin() || where <= end());

                if(must_destroy) {
                    std::destroy_at(std::to_address(where));
                }

                std::construct_at(
                    std::to_address(where),
                    std::forward<Args>(args)...
                );

                return *where;
            }

            constexpr void prepend(const_reference item) { insert(begin(), item); }
            constexpr void append(const_reference item) { insert(end(), item); }
            constexpr void push_back(const_reference item) { append(item); }

            template<typename ...Args>
            constexpr reference emplace(Args&& ...args) { return emplace_at(end(), std::forward<Args>(args)...); }

            template<typename ...Args>
            constexpr reference emplace_back(Args&& ...args) { return emplace_back(std::forward<Args>(args)...); }

            template<typename ...Args>
            constexpr reference  prepend_emplace(Args&& ...args) { return emplace_at(begin(), std::forward<Args>(args)...); }

            constexpr reference operator[](const size_type index) {
                CCL_THROW_IF(index >= _size, std::out_of_range{"Index out of range."});

                return _data[index];
            }

            constexpr const_reference operator[](const size_type index) const {
                CCL_THROW_IF(index >= _size, std::out_of_range{"Index out of range."});

                return _data[index];
            }

            constexpr void clear() noexcept {
                if constexpr(!std::is_trivially_destructible_v<T>) {
                    std::destroy(begin(), end());
                }

                _size = 0;
            }

            template<typename X>
            constexpr void resize(const size_type new_length, const X& value) {
                static_assert(
                    (std::is_same_v<value_init_tag_t, X> && std::is_default_constructible_v<T>)
                    || !std::is_same_v<value_init_tag_t, X>,
                    "Vector item is not default-constructible."
                );

                if(new_length > _size) {
                    reserve(new_length);

                    const auto start = begin() + _size;
                    const auto finish = begin() + new_length;

                    if constexpr(std::is_same_v<value_init_tag_t, X>) {
                        std::uninitialized_default_construct(start, finish);
                    } else {
                        std::uninitialized_fill(start, finish, value);
                    }
                } else if(new_length == 0) {
                    clear();
                } else if(new_length < _size) {
                    std::destroy(
                        begin() + new_length,
                        begin() + _size
                    );
                }

                _size = new_length;
            }

            constexpr void resize(const size_type new_length) {
                resize(new_length, value_init_tag);
            }

            constexpr void erase(const iterator start, const iterator finish) {
                CCL_THROW_IF(std::to_address(start) < _data || std::to_address(start) > _data + _size, std::out_of_range{"Invalid start iterator."});
                CCL_THROW_IF(std::to_address(finish) < _data || std::to_address(finish) > _data + _size, std::out_of_range{"Invalid finish iterator."});

                static_assert(std::is_move_assignable_v<T>);

                std::move(finish, end(), start);
                std::destroy(finish, end());

                _size -= finish - start;
            }

            constexpr void erase(const iterator it) {
                erase(it, it + 1);
            }

            constexpr bool is_empty() const noexcept { return _size == 0; }

            constexpr iterator begin() noexcept { return _data; }
            constexpr iterator end() noexcept { return _data + _size; }

            constexpr const_iterator begin() const noexcept { return _data; }
            constexpr const_iterator end() const noexcept { return _data + _size; }

            constexpr const_iterator cbegin() const noexcept { return _data; }
            constexpr const_iterator cend() const noexcept { return _data + _size; }
    };
}

#endif // CCL_VECTOR_HPP
