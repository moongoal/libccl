/**
 * @file
 *
 * Concurrent FIFO queue implementation based on ring buffer.
 */
#ifndef CCL_CONCURRENT_FIFO_HPP
#define CCL_CONCURRENT_FIFO_HPP

#include <ccl/api.hpp>
#include <ccl/concepts.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/atomic.hpp>
#include <ccl/internal/optional-allocator.hpp>

namespace ccl::concurrent {
    template<typename T, typed_allocator<T> Allocator = allocator>
    class fifo : private internal::with_optional_allocator<Allocator> {
        using alloc = internal::with_optional_allocator<Allocator>;

        public:
            using value_type = T;
            using size_type = count_t;
            using allocator_type = Allocator;

        private:
            atomic<T> *_data;
            size_type _size;
            atomic<size_type> read_index, write_index;
            allocation_flags alloc_flags;

        public:
            constexpr fifo()
                : alloc{},
                _data{nullptr},
                _size{0},
                read_index{0},
                write_index{0},
                alloc_flags{CCL_ALLOCATOR_DEFAULT_FLAGS}
            {}

            constexpr fifo(const fifo &other)
                : alloc{other},
                _data{alloc::get_allocator()->template allocate<atomic<T>>(other._size, other.alloc_flags)},
                _size{other._size},
                read_index{other.read_index},
                write_index{other.write_index},
                alloc_flags{other.alloc_flags}
            {
                if(_size == 0) {
                    return;
                }

                for(
                    size_type i = read_index.load(memory_order_relaxed);
                    i != write_index.load(memory_order_relaxed);
                    i = (i + 1) % _size
                ) {
                    _data[i].store(other._data[i].load(memory_order_relaxed), memory_order_relaxed);
                }
            }

            constexpr fifo(fifo &&other)
                : alloc{std::move(other)},
                _data{other._data},
                _size{other._size},
                read_index{other.read_index},
                write_index{other.write_index},
                alloc_flags{other.alloc_flags}
            {
                other._data = nullptr;
                other._size = 0;
                other.read_index = other.write_index = 0;
            }

            constexpr fifo(
                const size_type length,
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) : alloc{allocator},
                _data{alloc::get_allocator()->template allocate<atomic<T>>(length, alloc_flags)},
                _size{length},
                read_index{0},
                write_index{0},
                alloc_flags{alloc_flags}
            {}

            ~fifo() { destroy(); }

            constexpr fifo& operator=(const fifo &other) {
                alloc::operator=(other);

                _data = alloc::get_allocator()->template allocate<atomic<T>>(other._size, other.alloc_flags);
                _size = other._size;
                read_index = other.read_index;
                write_index = other.write_index;
                alloc_flags = other.alloc_flags;

                if(_size == 0) {
                    return;
                }

                for(
                    size_type i = read_index.load(memory_order_relaxed);
                    i != write_index.load(memory_order_relaxed);
                    i = (i + 1) % _size
                ) {
                    _data[i].store(other._data[i].load(memory_order_relaxed), memory_order_relaxed);
                }
            }

            void destroy() {
                if(_data) {
                    alloc::get_allocator()->deallocate(_data);
                    _data = nullptr;
                }
            }
    };
}

#endif // CCL_CONCURRENT_FIFO_HPP
