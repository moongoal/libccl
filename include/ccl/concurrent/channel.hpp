/**
 * @file
 *
 * Communication channel between two threads.
 */
#ifndef CCL_CONCURRENT_CHANNEL_HPP
#define CCL_CONCURRENT_CHANNEL_HPP

#include <optional>
#include <ccl/api.hpp>
#include <ccl/concepts.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/atomic.hpp>
#include <ccl/internal/optional-allocator.hpp>

namespace ccl::concurrent {
    /**
     * A communication channel that can be shared between two threads.
     *
     * The channel is uni-directional, that is one thread must only be
     * allowed to read and one thread must only be allowed to write.
     *
     * @tparam T The type of the value to exchange between the two threads.
     * @tparam Allocator The allocator type.
     */
    template<typename T, typed_allocator<T> Allocator = allocator>
    class channel : private internal::with_optional_allocator<Allocator> {
        using alloc = internal::with_optional_allocator<Allocator>;

        public:
            using value_type = T;
            using size_type = count_t;
            using allocator_type = Allocator;

        private:
            /**
             * The ring buffer data.
             */
            T *_data;

            /**
             * The capacity of the ring buffer.
             */
            size_type _capacity;

            /**
             * The index for reading from the ring buffer.
             */
            atomic<size_type> read_index;

            /**
             * The index for writing to the ring buffer.
             */
            atomic<size_type> write_index;

            /**
             * Allocation flags.
             */
            allocation_flags alloc_flags;

        public:
            channel() = delete;
            channel(const channel &other) = delete;

            constexpr channel(channel &&other)
                : alloc{std::move(other)},
                _data{other._data},
                _capacity{other._capacity},
                read_index{std::move(other.read_index)},
                write_index{std::move(other.write_index)},
                alloc_flags{other.alloc_flags}
            {
                other._data = nullptr;
            }

            /**
             * Initialise a new channel.
             *
             * @param capacity The capacity of the internal buffer.
             * @param allocator The optional allocator.
             * @param alloc_flags The optional allocator flags.
             */
            explicit constexpr channel(
                const size_type capacity,
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) : alloc{allocator},
                _data{alloc::get_allocator()->template allocate<T>(capacity, alloc_flags)},
                _capacity{capacity},
                read_index{0},
                write_index{0},
                alloc_flags{alloc_flags}
            {
                CCL_THROW_IF(capacity == 0, std::invalid_argument{"Length must be a positive value."});

                std::uninitialized_default_construct(_data, _data + _capacity);
            }

            ~channel() { destroy(); }

            constexpr channel& operator=(channel &&other) {
                alloc::operator=(std::move(other));

                ccl::swap(_data, other._data);
                ccl::swap(_capacity, other._capacity);

                const uint32_t my_read_index = read_index.load(memory_order_relaxed);
                const uint32_t other_read_index = other.read_index.load(memory_order_relaxed);
                const uint32_t my_write_index = write_index.load(memory_order_relaxed);
                const uint32_t other_write_index = other.write_index.load(memory_order_relaxed);

                read_index.store(other_read_index, memory_order_relaxed);
                write_index.store(other_write_index, memory_order_relaxed);
                other.read_index.store(my_read_index, memory_order_relaxed);
                other.write_index.store(my_write_index, memory_order_relaxed);

                ccl::swap(alloc_flags, other.alloc_flags);

                return *this;
            }

            /**
             * Destroy a channel. The channel must not be used after this function returns.
             */
            void destroy() {
                if(_data) {
                    alloc::get_allocator()->deallocate(_data);
                    _data = nullptr;
                }
            }

            /**
             * Tell whether the channel's buffer is full.
             *
             * @return True if the channel's buffer is full, false if it is not.
             */
            bool is_full() const {
                const size_type cur_write_index = write_index.load(memory_order_relaxed);
                const size_type cur_read_index = read_index.load(memory_order_relaxed);
                const size_type start_index = min(cur_write_index, cur_read_index);
                const size_type end_index = max(cur_write_index, cur_read_index);
                const size_type length = end_index - start_index;

                return length == _capacity;
            }

            /**
             * Tell whether the channel's buffer is empty.
             *
             * @return True if the channel's buffer is empty, false if it is not.
             */
            bool is_empty() const {
                const size_type cur_write_index = write_index.load(memory_order_relaxed);
                const size_type cur_read_index = read_index.load(memory_order_relaxed);

                return cur_read_index == cur_write_index;
            }

            /**
             * Get the channel's buffer capacity.
             *
             * @return The channel's buffer capacity.
             */
            constexpr size_type get_capacity() const {
                return _capacity;
            }

            /**
             * Add an item to the channel's buffer.
             *
             * @param item The item to add.
             *
             * @return True if the item was successfully added, false if the buffer is full.
             */
            CCLNODISCARD bool send(const T& item) {
                const size_type cur_write_index = write_index.load(memory_order_relaxed);

                if(is_full()) { CCLUNLIKELY
                    return false;
                }

                _data[cur_write_index % _capacity] = item;
                write_index.store(cur_write_index + 1, memory_order_relaxed);

                return true;
            }

            /**
             * Extract the next available message from the channel.
             *
             * @return The next available message, if any, or `std::nullopt`
             *  if the channel's buffer is empty.
             */
            CCLNODISCARD std::optional<T> recv() {
                if(is_empty()) {
                    return std::nullopt;
                }

                const size_type cur_read_index = read_index.load(memory_order_relaxed);

                do_not_optimize(read_index);
                const T value = std::move(_data[cur_read_index % _capacity]);
                do_not_optimize(read_index);

                read_index.store(cur_read_index + 1, memory_order_relaxed);

                return value;
            }
    };
}

#endif // CCL_CONCURRENT_CHANNEL_HPP
