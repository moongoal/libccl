/**
 * @file
 *
 * Ring buffer.
 */
#ifndef CCL_RING_HPP
#define CCL_RING_HPP

#include <ccl/api.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/concepts.hpp>
#include <ccl/internal/optional-allocator.hpp>

namespace ccl {
    template<
        typename T,
        typed_allocator<T> Allocator = allocator
    > class ring : private internal::with_optional_allocator<Allocator> {
        using alloc = internal::with_optional_allocator<Allocator>;

        public:
            using size_type = count_t;
            using value_type = T;
            using pointer = T*;
            using reference = T&;
            using rvalue_reference = T&&;
            using const_reference = const T&;
            using allocator_type = Allocator;

        private:
            size_type _read_index;
            size_type _size;
            size_type _capacity;
            value_type * _data;
            allocation_flags _alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS;

            constexpr size_type get_write_index() const {
                CCL_THROW_IF(is_full(), std::out_of_range{"Ring is full."});

                return (_read_index + _size) % _capacity;
            }

        public:
            explicit constexpr ring(
                const size_type capacity,
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) : alloc{allocator},
                _read_index{0},
                _size{0},
                _capacity{capacity},
                _data{alloc::get_allocator()->template allocate<value_type>(_capacity, alloc_flags)},
                _alloc_flags{alloc_flags}
            {}

            constexpr ring(const ring &other)
                : alloc{other.get_allocator()},
                _read_index{other._read_index},
                _size{other._size},
                _capacity{other._capacity},
                _data{alloc::get_allocator()->template allocate<value_type>(_capacity, other._alloc_flags)},
                _alloc_flags{other._alloc_flags}
            {
                std::uninitialized_copy_n(other._data, other._size, _data);
            }

            constexpr ring(ring &&other) noexcept
                : alloc{other.get_allocator()},
                _read_index{other._read_index},
                _size{other._size},
                _capacity{other._capacity},
                _data{other._data},
                _alloc_flags{other._alloc_flags}
            {
                other._data = nullptr;
            }

            template<std::ranges::input_range InputRange>
            constexpr ring(
                const InputRange& input,
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) : ring{
                    static_cast<size_type>(std::abs(std::ranges::distance(input))),
                    allocator,
                    alloc_flags
                }
            {
                if(_capacity > 0) {
                    _size = _capacity;

                    std::ranges::uninitialized_copy(
                        input.begin(),
                        input.end(),
                        _data,
                        _data + _size
                    );
                }
            }

            ~ring() {
                destroy();
            }

            void destroy() noexcept {
                clear();

                alloc::get_allocator()->deallocate(_data);
                _data = nullptr;
            }

            void clear() noexcept {
                if constexpr(!std::is_trivially_destructible_v<T>) {
                    if(_data && _size > 0) {
                        const pointer read_ptr = _data + _read_index;
                        const pointer end_ptr = _data + (_read_index + _size) % _capacity;
                        const pointer destroy_start = ccl::min(read_ptr, end_ptr);
                        const pointer destroy_end = ccl::max(read_ptr, end_ptr);

                        std::destroy(destroy_start, destroy_end);
                    }
                }

                _size = 0;
            }

            constexpr ring& operator =(const ring &other) {
                if(other._capacity > _capacity || !alloc::is_allocator_stateless()) {
                    destroy();
                    alloc::operator=(other);
                    _size = other._size;
                    _capacity = other._capacity;
                    _data = alloc::get_allocator()->template allocate<value_type>(
                        other._capacity,
                        other._alloc_flags
                    );

                    std::uninitialized_copy(other._data, other._data + other._capacity, _data);
                } else {
                    std::copy(other._data, other._data + other._capacity, _data);
                    std::destroy(_data + other.size(), _data + _capacity);
                    _size = other.size();
                }

                _read_index = other._read_index;
                _alloc_flags = other._alloc_flags;

                return *this;
            }

            constexpr ring& operator =(ring &&other) {
                swap(other);

                return *this;
            }

            constexpr void swap(ring &other) {
                alloc::swap(other);

                ccl::swap(_read_index, other._read_index);
                ccl::swap(_size, other._size);
                ccl::swap(_capacity, other._capacity);
                ccl::swap(_data, other._data);
                ccl::swap(_alloc_flags, other._alloc_flags);
            }

            constexpr size_type read_index() const noexcept { return _read_index; }
            constexpr size_type size() const noexcept { return _size; }
            constexpr size_type capacity() const noexcept { return _capacity; }
            constexpr pointer data() const noexcept { return _data; }
            constexpr bool is_empty() const noexcept { return _size == 0; }
            constexpr bool is_full() const noexcept { return _size == _capacity; }
            constexpr allocator_type* get_allocator() const noexcept { return alloc::get_allocator(); }
            constexpr allocation_flags get_allocation_flags() const noexcept { return _alloc_flags; }

            constexpr void enqueue(const_reference item) noexcept(!exceptions_enabled) {
                const size_type write_index = get_write_index();

                std::construct_at(&_data[write_index], item);
                _size += 1;
            }

            constexpr void emplace_enqueue(rvalue_reference item) noexcept(!exceptions_enabled) {
                const size_type write_index = get_write_index();

                std::construct_at(&_data[write_index], std::move(item));
                _size += 1;
            }

            constexpr void dequeue() noexcept(!exceptions_enabled) {
                CCL_THROW_IF(is_empty(), std::out_of_range{"Ring is empty."});

                const pointer item = &_data[_read_index];
                std::destroy(item, item + 1);
                _read_index = (_read_index + 1) % _capacity;
                _size -= 1;
            }

            constexpr reference operator*() noexcept(!exceptions_enabled) {
                CCL_THROW_IF(is_empty(), std::out_of_range{"Ring is empty."});

                return _data[_read_index];
            }

            constexpr const_reference operator*() const noexcept(!exceptions_enabled) {
                CCL_THROW_IF(is_empty(), std::out_of_range{"Ring is empty."});

                return _data[_read_index];
            }
    };
}

#endif // CCL_RING_HPP
