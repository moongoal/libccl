#ifndef CCL_HANDLE_MANAGER_HPP
#define CCL_HANDLE_MANAGER_HPP

#include <algorithm>
#include <type_traits>
#include <ccl/api.hpp>
#include <ccl/debug.hpp>
#include <ccl/exceptions.hpp>
#include <ccl/hash.hpp>
#include <ccl/handle.hpp>
#include <ccl/paged-vector.hpp>
#include <ccl/util.hpp>
#include <ccl/memory/allocator.hpp>

namespace ccl {
    /**
     * Policy to decide how to deal with a handle reaching
     * its maximum genereation.
     */
    enum class handle_manager_expiry_policy {
        /**
         * Set the generation to 0 and recycle the handle slot.
         */
        recycle,

        /**
         * Disable the handle slot and do not use it until the
         * manager is reset.
         */
        discard
    };

    /**
     * The default handle manager expiry policy value.
     */
    static constexpr handle_manager_expiry_policy default_handle_manager_expiry_policy = handle_manager_expiry_policy::discard;

    /**
     * Facility for managing handles of a given type. This class
     * allows acquiring, validating and relating handles.
     *
     * This manager defines an "expired" handle as a handle whose generation
     * has reached its maximum value.
     *
     * @tparam ObjectType The type of object handles acquired via this manager represent.
     * @tparam ExpiryPolicy Policy to deal with expired handles.
     * @tparam Allocator The allocator.
     */
    template<
        typename ObjectType,
        handle_manager_expiry_policy ExpiryPolicy = default_handle_manager_expiry_policy,
        typed_allocator<ObjectType> Allocator = allocator
    > class handle_manager {
        public:
            using object_type = ObjectType;
            using handle_type = versioned_handle<object_type>;
            using allocator_type = Allocator;
            using value_type = typename handle_type::value_type;
            using vector_type = paged_vector<value_type, value_type*, allocator_type>;

            static constexpr value_type value_unused_mask = bitcount(static_cast<value_type>(0) - 1) - 1;
            static constexpr handle_manager_expiry_policy expiry_policy = ExpiryPolicy;

        private:
            /**
             * A vector of available slots. Each value of these contains the current generation of the handle
             * and its MSB is set to 1 if the handle slot is currently not in used.
             */
            vector_type handles;

            /**
             * Last acquired handle slot. This is used to improve acquisition times
             * by avoiding having to start search for a new handle always at location 0.
             */
            size_t last_slot_index = 0;

            /**
             * Test whether a handle slot is used.
             *
             * @param value The slot value to test.
             *
             * @return True if the slot is in use, false if it's available.
             */
            static constexpr bool is_slot_used(const value_type value) noexcept {
                return ~value & value_unused_mask;
            }

            /**
             * Get the generation value from a slot.
             *
             * @param value The slot value to test.
             *
             * @return The slot generation.
             */
            static constexpr value_type get_slot_generation(const value_type value) noexcept {
                const bool is_used = is_slot_used(value);

                return choose(value, value & ~value_unused_mask, is_used);
            }

            /**
             * Add a new page of slots and initialise
             * its values.
             */
            void add_page() {
                handles.resize(handles.size() + vector_type::page_size);
                auto * const last_page = *(handles.pages().end() - 1);
                std::fill(last_page, last_page + vector_type::page_size, value_unused_mask);
            }

            /**
             * Find the next available slot. If none is available,
             * return `handles.end()`.
             */
            auto find_next_slot() {
                const auto is_available_slot = [] (const value_type slot) {
                    if constexpr(expiry_policy == handle_manager_expiry_policy::recycle) {
                        return !is_slot_used(slot);
                    } else {
                        return !is_slot_used(slot) && get_slot_generation(slot) < handle_type::max_generation;
                    }
                };

                const auto result = std::find_if(
                    handles.begin() + last_slot_index,
                    handles.end(),
                    is_available_slot
                );

                if(result != handles.end()) {
                    return result;
                }

                return std::find_if(
                    handles.begin(),
                    handles.begin() + last_slot_index,
                    is_available_slot
                );
            }

        public:
            /**
             * Acquire a new handle.
             *
             * @return The newly acquired handle.
             */
            CCLNODISCARD handle_type acquire() {
                typename vector_type::iterator result = find_next_slot();

                if(result == handles.end()) {
                    const size_t size = handles.size();

                    add_page();
                    result = handles.begin() + size;
                }

                const value_type generation = get_slot_generation(*result);
                const value_type index = last_slot_index;

                last_slot_index = (result - handles.begin() + 1) & (vector_type::page_size - 1);
                *result = generation;

                return handle_type::make(generation, index);
            }

            /**
             * Validate a handle.
             *
             * A handle is invalid if:
             * * It's not within boundary of the currently allocated handle-space
             * * It's not in use
             * * Its generation is different from the one in the manager
             *
             * @param handle The handle to validate.
             *
             * @return True if the handle is valid, false if it's invalid.
             */
            bool validate_handle(const handle_type handle) const {
                const auto index = handle.value();
                const auto generation = handle.generation();

                if(index < handles.size()) {
                    const auto slot = handles[index];

                    return (
                        is_slot_used(slot)
                        && get_slot_generation(slot) == generation
                    );
                }

                return false;
            }

            /**
             * Release a handle. After being released, the handle becomes
             * invalid.
             *
             * @param handle The handle to release.
             */
            void release(const handle_type handle) {
                CCL_THROW_IF(!validate_handle(handle), std::invalid_argument{"Invalid handle."});

                const auto index = handle.value();
                const auto generation = handle.generation();

                if constexpr(expiry_policy == handle_manager_expiry_policy::recycle) {
                    handles[index] = (generation + 1) & handle_type::max_generation;
                } else {
                    handles[index] = generation + 1;
                }
            }

            /**
             * Resets all expired handles to the 0th generation so they can be used again.
             */
            std::enable_if<expiry_policy == handle_manager_expiry_policy::discard>
            reset_expired() {
                std::for_each(handles.begin(), handles.end(), [] (value_type& slot) {
                    if(!is_slot_used(slot) && get_slot_generation(slot) == handle_type::max_generation) {
                        slot = value_unused_mask;
                    }
                });
            }
    };
}

#endif // CCL_HANDLE_MANAGER_HPP
