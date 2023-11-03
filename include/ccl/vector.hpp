/**
 * @file
 *
 * A vector.
 */
#ifndef CCL_VECTOR_HPP
#define CCL_VECTOR_HPP

#include <type_traits>
#include <memory>
#include <algorithm>
#include <initializer_list>
#include <ccl/api.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/concepts.hpp>
#include <ccl/util.hpp>
#include <ccl/internal/optional-allocator.hpp>
#include <ccl/either.hpp>
#include <ccl/contiguous-iterator.hpp>

namespace ccl {
    template<
        typename T,
        typed_allocator<T> Allocator = allocator
    >
    class vector : private internal::with_optional_allocator<Allocator> {
        using alloc = internal::with_optional_allocator<Allocator>;

        struct value_init_tag_t {};
        static constexpr value_init_tag_t value_init_tag{};

        public:
            using size_type = count_t;
            using value_type = T;
            using pointer = T*;
            using reference = T&;
            using const_reference = const T&;
            using allocator_type = Allocator;
            using iterator = contiguous_iterator<T>;
            using const_iterator = contiguous_iterator<const T>;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        private:
            size_type _size = 0;
            size_type _capacity = 0;
            value_type * _data = nullptr;
            allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS;

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
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) : alloc{allocator}, alloc_flags{alloc_flags}
            {}

            constexpr vector(const vector &other) : vector{other.get_allocator(), other.alloc_flags} {
                alloc_flags = other.alloc_flags;
                reserve(other._size);
                std::uninitialized_copy(other.begin(), other.end(), begin());
                _size = other._size;
            }

            constexpr vector(vector &&other)
                : alloc{other.get_allocator()},
                _size{other._size},
                _capacity{other._capacity},
                _data{other._data},
                alloc_flags{other.alloc_flags}
            {
                other._data = nullptr;
                other._size = 0;
                other._capacity = 0;
            }

            constexpr vector(
                std::initializer_list<T> values,
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) : alloc{allocator} {
                this->alloc_flags = alloc_flags;
                reserve(values.size());
                std::uninitialized_copy(values.begin(), values.end(), begin());
                _size = values.size();
            }

            template<std::ranges::input_range InputRange>
            constexpr vector(
                const InputRange& input,
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) : vector{allocator, alloc_flags} {
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
                if(other._size > _size || !alloc::is_allocator_stateless()) {
                    destroy();
                    alloc::operator=(other);
                    alloc_flags = other.alloc_flags;
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
                swap(other);

                return *this;
            }

            constexpr void swap(vector &other) {
                alloc::swap(other);

                ccl::swap(_size, other._size);
                ccl::swap(_capacity, other._capacity);
                ccl::swap(_data, other._data);
                ccl::swap(alloc_flags, other.alloc_flags);
            }

            constexpr size_type size() const noexcept { return _size; }
            constexpr size_type capacity() const noexcept { return _capacity; }
            constexpr pointer data() const noexcept { return _data; }

            constexpr void reserve(const size_type new_capacity) {
                if(new_capacity > _capacity) {
                    const size_type actual_new_capacity = increase_capacity(_capacity, new_capacity);

                    value_type * const new_data = alloc::get_allocator()->template allocate<value_type>(
                        actual_new_capacity,
                        alloc_flags
                    );

                    if(_data) {
                        std::uninitialized_move(begin(), end(), new_data);
                        alloc::get_allocator()->deallocate(_data);
                    }

                    _data = new_data;
                    _capacity = actual_new_capacity;
                }
            }

            constexpr void shrink_to_fit() {
                if(_size > 0) {
                    const size_type new_capacity = increase_capacity<decltype(_capacity)>(1, _size);
                    value_type * const new_data = alloc::get_allocator()->template allocate<value_type>(
                        new_capacity,
                        alloc_flags
                    );

                    if(_data) {
                        std::uninitialized_move(begin(), end(), new_data);
                        alloc::get_allocator()->deallocate(_data);
                    }

                    _data = new_data;
                    _capacity = new_capacity;
                } else {
                    destroy();
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
            constexpr void insert_range(iterator where, const InputRange& input) {
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
            constexpr reference emplace(iterator where, Args&& ...args) { return emplace_at(where, std::forward<Args>(args)...); }

            template<typename ...Args>
            constexpr reference emplace_back(Args&& ...args) { return emplace_at(end(), std::forward<Args>(args)...); }

            template<typename ...Args>
            constexpr reference prepend_emplace(Args&& ...args) { return emplace_at(begin(), std::forward<Args>(args)...); }

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

            constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{_data + _size}; }
            constexpr reverse_iterator rend() noexcept { return reverse_iterator{_data}; }

            constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{_data + _size}; }
            constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{_data}; }

            constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{_data + _size}; }
            constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{_data}; }

            constexpr allocator_type* get_allocator() const noexcept { return alloc::get_allocator(); }
            constexpr allocation_flags get_allocation_flags() const noexcept { return alloc_flags; }
    };
}

#endif // CCL_VECTOR_HPP
