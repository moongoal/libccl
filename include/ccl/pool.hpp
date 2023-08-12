/**
 * @file
 *
 * A pool of objects of a given type.
 */
#ifndef CCL_POOL_HPP
#define CCL_POOL_HPP

#include <functional>
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
        typename Handle = versioned_handle<T>,
        typed_allocator<T> Allocator = allocator,
        allocation_flags AllocationFlags = 0
    > class pool {
        public:
            using value_type = T;
            using pointer = T*;
            using reference = T&;
            using const_reference = const T&;
            using const_pointer = const T*;
            using handle_type = Handle;
            using allocator_type = Allocator;
            using object_vector_type = paged_vector<value_type, pointer, allocator_type>;
            using handle_manager_type = handle_manager<T, HandleExpiryPolicy, handle_type, allocator_type>;
            using handle_object_callback = std::function<void(const handle_type, reference)>;

            static constexpr allocation_flags allocation_flags = AllocationFlags;

        private:
            handle_manager_type handle_manager;
            object_vector_type data;
            value_type default_value;

        public:
            explicit constexpr pool(
                const_reference default_value = T{},
                allocator_type * const allocator = nullptr
            ) noexcept :
                handle_manager{allocator},
                data{allocator},
                default_value{default_value}
            {}

            constexpr pool(const pool& other) = default;
            constexpr pool(pool&& other) noexcept = default;

            pool& operator=(const pool& other) = default;
            pool& operator=(pool&& other) noexcept = default;

            virtual ~pool() = default;

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

                set(handle, default_value);

                return handle;
            }

            /**
             * Release an item in the pool. This will not destroy the item.
             *
             * @param handle The handle of the item to release.
             */
            void release(const handle_type &handle) {
                set(handle, default_value);
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
            void reset() {
                handle_manager.reset();
                std::fill(data.begin(), data.end(), default_value);
            }

            /**
             * @see handle_manager::is_valid()
             */
            bool is_valid(const handle_type &handle) const { return handle_manager.is_valid(handle); }

            /**
             * Get an item from the pool. An old (used in the past but free now) handle
             * will return the default value. A handle never acquired via this pool
             * will generate undefined behaviour. Setting the value of a reference
             * obtained via an old handle will produce undefined behaviour.
             *
             * @param handle The handle of the item to get.
             *
             * @return A reference to the item.
             */
            CCLNODISCARD reference get(const handle_type &handle) {
                return data[handle.value()];
            }

            /**
             * Get an item from the pool. An old (used in the past but free now) handle
             * will return the default value. A handle never acquired via this pool
             * will generate undefined behaviour.
             *
             * @param handle The handle of the item to get.
             *
             * @return A reference to the item.
             */
            CCLNODISCARD const_reference get(const handle_type &handle) const {
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
            reference set(const handle_type &handle, const T& value) {
                CCL_THROW_IF(!is_valid(handle), std::invalid_argument{"Invalid handle."});

                return data[handle.value()] = value;
            }

            /**
             * Invoke a given callback for all acquired handles and items.
             *
             * @param callback The callback to invoke.
             */
            void for_each(const handle_object_callback callback) {
                handle_manager.for_each([this, callback] (const handle_type &handle) {
                    callback(handle, get(handle));
                });
            }

            /**
             * Invoke a given callback for all acquired handles and items.
             *
             * @param callback The callback to invoke.
             */
            void for_each(const handle_object_callback callback) const {
                handle_manager.for_each([this, callback] (const handle_type &handle) {
                    callback(handle, get(handle));
                });
            }
    };
}

#endif // CCL_POOL_HPP
