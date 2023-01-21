/**
 * @file
 *
 * Double-ended queue implementation. This is implemented with a linear, contiguous
 * chunk of memory and reallocated whenever necessary. Unlike in a vector, allocating
 * at the beginning of the collection does not necessarily require moving all elements
 * forward. Contiguous memory helps increasing cache usage efficiency during iteration.
 */
#ifndef CCL_DEQUE_HPP
#define CCL_DEQUE_HPP

#include <ccl/api.hpp>
#include <ccl/internal/optional-allocator.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/concepts.hpp>
#include <ccl/debug.hpp>
#include <ccl/util.hpp>
#include <ccl/contiguous-iterator.hpp>

namespace ccl {
    template<typename T, typed_allocator<T> Allocator>
    class deque : private internal::with_optional_allocator<Allocator> {
        using alloc = internal::with_optional_allocator<Allocator>;

        public:
            using value_type = T;
            using reference = T&;
            using const_reference = const T&;
            using pointer = T*;
            using const_pointer = const T*;
            using allocator_type = Allocator;
            using size_type = std::size_t;
            using iterator = contiguous_iterator<T>;
            using const_iterator = contiguous_iterator<const T>;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            static constexpr size_type minimum_capacity = CCL_DEQUE_MIN_CAPACITY;

        private:
            size_type first = 0;
            size_type last = 0;
            pointer _data = nullptr;
            size_type _capacity = 0;

            /**
             * Recenter first and last indices.
             */
            void recenter() noexcept {
                first = last = max(_capacity, 1ULL) >> 1;
            }

        public:
            constexpr deque(allocator_type * const allocator = nullptr)
                : alloc{allocator}
            {}

            constexpr deque(const deque& other)
                : alloc{other},
                first{other.first},
                last{other.last},
                _data{other._data},
                _capacity{other._capacity}
            {}

            constexpr deque(deque&& other)
                : alloc{other},
                first{other.first},
                last{other.last},
                _data{other._data},
                _capacity{other._capacity}
            {
                other._data = nullptr;
            }

            ~deque() { destroy(); }

            constexpr size_type size() const noexcept { return last - first; }
            constexpr size_type capacity() const noexcept { return _capacity; }

            void destroy() noexcept {
                if(_data) {
                    std::destroy_n(_data + first, size());

                    alloc::get_allocator()->deallocate(_data);
                    _data = nullptr;
                }
            }

            constexpr pointer data() noexcept {
                return _data;
            }

            constexpr const_pointer data() const noexcept {
                return _data;
            }

            constexpr bool is_empty() const noexcept { return first == last; }

            CCLNODISCARD constexpr reference front() {
                CCL_THROW_IF(is_empty(), std::out_of_range{"Deque is empty."});
                return *begin();
            }

            CCLNODISCARD constexpr reference back() {
                CCL_THROW_IF(is_empty(), std::out_of_range{"Deque is empty."});
                return *(end() - 1);
            }

            CCLNODISCARD constexpr const_reference cfront() const {
                CCL_THROW_IF(is_empty(), std::out_of_range{"Deque is empty."});
                return *cbegin();
            }

            CCLNODISCARD constexpr const_reference cback() const {
                CCL_THROW_IF(is_empty(), std::out_of_range{"Deque is empty."});
                return *(cend() - 1);
            }

            constexpr iterator begin() noexcept { return _data + first; }
            constexpr iterator end() noexcept { return _data + last; }

            constexpr const_iterator begin() const noexcept { return _data + first; }
            constexpr const_iterator end() const noexcept { return _data + last; }

            constexpr const_iterator cbegin() const noexcept { return _data + first; }
            constexpr const_iterator cend() const noexcept { return _data + last; }

            constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{_data + last}; }
            constexpr reverse_iterator rend() noexcept { return reverse_iterator{_data + first}; }

            constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{_data + last}; }
            constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{_data + first}; }

            constexpr const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{_data + last}; }
            constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator{_data + first}; }

            constexpr void reserve(const size_type new_capacity) {
                if(new_capacity > capacity_front() || new_capacity > capacity_back()) {
                    const size_type actual_new_capacity = max(
                        increase_capacity(_capacity, new_capacity),
                        minimum_capacity
                    );

                    value_type * const new_data = alloc::get_allocator()->template allocate<value_type>(actual_new_capacity);
                    const size_type old_size = size();
                    const size_type new_first = (max(actual_new_capacity, 1ULL) >> 1) - old_size / 2;

                    if(_data) {
                        std::uninitialized_move(begin(), end(), new_data + new_first);
                        alloc::get_allocator()->deallocate(_data);
                    }

                    first = new_first;
                    last = first + old_size;
                    _data = new_data;
                    _capacity = actual_new_capacity;
                }
            }

            constexpr deque& operator=(const deque& other) {
                if(other.size() > size() || !alloc::is_allocator_stateless()) {
                    destroy();
                    alloc::operator=(other);
                    reserve(other._capacity);
                    std::uninitialized_copy(other.begin(), other.end(), begin());
                } else {
                    std::copy(other.begin(), other.end(), begin());
                    std::destroy(begin() + other.size(), end());
                }

                first = other.first;
                last = other.last;
                _capacity = other._capacity;

                return *this;
            }

            constexpr deque& operator =(deque &&other) {
                alloc::operator =(std::move(other));

                swap(first, other.first);
                swap(last, other.last);
                swap(_capacity, other._capacity);
                swap(_data, other._data);

                return *this;
            }

            constexpr size_type capacity_back() const noexcept {
                return _capacity - last;
            }

            constexpr size_type capacity_front() const noexcept {
                return first;
            }

            constexpr void push_back(const_reference item) {
                if(!capacity_back()) { CCLUNLIKELY
                    reserve(_capacity + 1);
                }

                std::uninitialized_copy(&item, &item + 1, _data + last);
                last += 1;
            }

            template<typename ...Args>
            constexpr void emplace_back(Args&& ...args) {
                if(!capacity_back()) { CCLUNLIKELY
                    reserve(_capacity + 1);
                }

                std::construct_at(_data + last, std::forward<Args>(args)...);
                last += 1;
            }

            constexpr void push_front(const_reference item) {
                if(!capacity_front()) { CCLUNLIKELY
                    reserve(_capacity + 1);
                }

                first -= first != 0; // Decrease by one only if > 0
                std::uninitialized_copy(&item, &item + 1, _data + first);
            }

            template<typename ...Args>
            constexpr void emplace_front(Args&& ...args) {
                if(!capacity_front()) { CCLUNLIKELY
                    reserve(_capacity + 1);
                }

                first -= first != 0; // Decrease by one only if > 0
                std::construct_at(_data + first, std::forward<Args>(args)...);
            }

            void clear() {
                if(_data) {
                    std::destroy_n(_data + first, size());
                    recenter();
                }
            }

            void pop_back() {
                const bool is_empty = this->is_empty();

                CCL_THROW_IF(is_empty, std::out_of_range{"Deque is empty."});

                std::destroy_at(_data + last - 1);
                last -= 1;

                if(!is_empty) { return; }

                recenter();
            }

            void pop_front() {
                const bool is_empty = this->is_empty();

                CCL_THROW_IF(is_empty, std::out_of_range{"Deque is empty."});

                std::destroy_at(_data + first);
                first += 1;

                if(!is_empty) { return; }

                recenter();
            }
    };
}

#endif // CCL_DEQUE_HPP