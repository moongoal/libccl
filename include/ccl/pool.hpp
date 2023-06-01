/**
 * @file
 *
 * A pool of objects of a given type.
 */
#ifndef CCL_POOL_HPP
#define CCL_POOL_HPP

#include <ccl/api.hpp>
#include <ccl/debug.hpp>
#include <ccl/exceptions.hpp>
#include <ccl/concepts.hpp>
#include <ccl/handle-manager.hpp>
#include <ccl/paged-vector.hpp>
#include <ccl/memory/allocator.hpp>

namespace ccl {
    /**
     * A pool of items of the same type.
     *
     * This class keeps a collection of items and allows referencing them
     * via handles. Items are only destroyed when the pool is, so destructors
     * are not called when a handle is released. This class guarantees pointer
     * stability, so all returned pointers are guaranteed to be valid for the
     * lifetime of the pool object.
     */
    template<
        typename T,
        handle_expiry_policy HandleExpiryPolicy,
        typed_allocator<T> ObjectAllocator = allocator,
        handle_manager_slot_allocator<T> HandleManagerAllocator = allocator
    > class pool {
        public:
            using value_type = T;
            using pointer = T*;
            using reference = T&;
            using const_reference = const T&;
            using const_pointer = const T*;
            using handle_type = versioned_handle<T>;
            using object_allocator_type = ObjectAllocator;
            using handle_manager_allocator_type = HandleManagerAllocator;
            using object_vector_type = paged_vector<value_type, pointer, object_allocator_type>;
            using handle_manager_type = handle_manager<T, HandleExpiryPolicy, handle_manager_allocator_type>;

        private:
            handle_manager_type handle_manager;
            object_vector_type data;

        public:
            explicit constexpr pool(
                object_allocator_type * const object_allocator = nullptr,
                handle_manager_allocator_type * const handle_manager_allocator = nullptr
            ) noexcept : handle_manager{handle_manager_allocator}, data{object_allocator}
            {}

            constexpr pool(const pool& other) = default;
            constexpr pool(pool&& other) noexcept = default;

            pool& operator=(const pool& other) = default;
            pool& operator=(pool&& other) noexcept = default;

            /**
             * Acquire a new item in the pool.
             *
             * @return The item handle.
             */
            CCLNODISCARD handle_type acquire() {
                const auto handle = handle_manager.acquire();

                if(handle.value() >= data.size()) {
                    data.resize(handle.value() + 1);
                }

                return handle;
            }

            /**
             * Release an item in the pool. This will not destroy the item.
             *
             * @param handle The handle of the item to release.
             */
            void release(const handle_type handle) {
                handle_manager.release(handle);
            }

            /**
             * @see handle_manager::reset_expired()
             */
            template<bool Enable = HandleExpiryPolicy == handle_expiry_policy::discard>
            std::enable_if_t<Enable>
            reset_expired() { handle_manager.reset_expired(); }

            /**
             * @see handle_manager::reset()
             */
            void reset() { handle_manager.reset(); }

            /**
             * @see handle_manager::is_valid_handle()
             */
            bool is_valid_handle(const handle_type handle) const { return handle_manager.is_valid_handle(handle); }

            /**
             * Get an item from the pool. It's undefined behaviour to provide
             * an invalid handle.
             *
             * @param handle The handle of the item to get.
             *
             * @return A reference to the item.
             */
            CCLNODISCARD reference get(const handle_type handle) {
                return data[handle.value()];
            }

            /**
             * Get an item from the pool. It's undefined behaviour to provide
             * an invalid handle.
             *
             * @param handle The handle of the item to get.
             *
             * @return A reference to the item.
             */
            CCLNODISCARD const_reference get(const handle_type handle) const {
                return data[handle.value()];
            }

            /**
             * Set the value for an item from the pool.
             *
             * @param handle The handle of the item to get.
             * @param value The new value to set.
             *
             * @return A reference to the item.
             */
            reference set(const handle_type handle, T&& value) {
                CCL_THROW_IF(!handle_manager.is_valid_handle(handle), std::invalid_argument{"Invalid handle."});

                return data[handle.value()] = std::forward<T&&>(value);
            }
    };
}

#endif // CCL_POOL_HPP
