/**
 * @file
 *
 * A facility to manage handle creation and and deletion for a specific type.
 */
#ifndef CCL_HANDLE_MANAGER_HPP
#define CCL_HANDLE_MANAGER_HPP

#include <algorithm>
#include <functional>
#include <type_traits>
#include <ccl/api.hpp>
#include <ccl/debug.hpp>
#include <ccl/exceptions.hpp>
#include <ccl/hash.hpp>
#include <ccl/handle/versioned-handle.hpp>
#include <ccl/paged-vector.hpp>
#include <ccl/util.hpp>
#include <ccl/memory/allocator.hpp>

namespace ccl {
    /**
     * Policy to decide how to deal with a handle reaching
     * its maximum genereation.
     */
    enum class handle_expiry_policy {
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
     * An allocator suitable for managing handle slot memory.
     *
     * @tparam Allocator The allocator type.
     * @tparam HandleType The handle type of the manager.
     */
    template<typename Allocator, typename HandleType>
    concept handle_manager_slot_allocator = requires(Allocator allocator) {
        typed_allocator<Allocator, typename HandleType::value_type>;
    };

    /**
     * The default handle manager expiry policy value.
     */
    static constexpr handle_expiry_policy default_handle_manager_expiry_policy = handle_expiry_policy::discard;

    /**
     * Facility for managing handles of a given type. This class
     * allows acquiring, validating and relating handles.
     *
     * This manager defines an "expired" handle as a handle whose generation
     * has reached its maximum value.
     *
     * @tparam ObjectType The type of object handles acquired via this manager represent.
     * @tparam ExpiryPolicy Policy to deal with expired handles.
     * @tparam Handle The handle type.
     * @tparam Allocator The allocator.
     */
    template<
        typename ObjectType,
        handle_expiry_policy ExpiryPolicy = default_handle_manager_expiry_policy,
        typename Handle = versioned_handle<ObjectType>,
        handle_manager_slot_allocator<Handle> Allocator = allocator
    > class handle_manager {
        public:
            using object_type = ObjectType;
            using handle_type = Handle;
            using allocator_type = Allocator;
            using value_type = typename handle_type::value_type;
            using vector_type = paged_vector<value_type, value_type*, allocator_type>;
            using handle_callback = std::function<void(const handle_type&)>;

            static constexpr value_type value_unused_mask = handle_type::max_generation + 1;
            static constexpr handle_expiry_policy expiry_policy = ExpiryPolicy;

        private:
            /**
             * A vector of available slots. Each value of these contains the current generation of the handle
             * and its MSB is set to 1 if the handle slot is currently not in used.
             */
            vector_type handle_slots;

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
                handle_slots.resize(handle_slots.size() + vector_type::page_size);
                auto * const last_page = *(handle_slots.pages().end() - 1);
                std::fill(last_page, last_page + vector_type::page_size, value_unused_mask);
            }

            /**
             * Find the next available slot. If none is available,
             * return `handles.end()`.
             */
            auto find_next_slot() {
                const auto is_available_slot = [] (const value_type slot) {
                    if constexpr(expiry_policy == handle_expiry_policy::recycle) {
                        return !is_slot_used(slot);
                    } else {
                        return !is_slot_used(slot) && get_slot_generation(slot) < handle_type::max_generation;
                    }
                };

                auto result = std::find_if(
                    handle_slots.begin() + last_slot_index,
                    handle_slots.end(),
                    is_available_slot
                );

                if(result != handle_slots.end()) {
                    return result;
                }

                const auto new_end = handle_slots.begin() + last_slot_index;

                result = std::find_if(
                    handle_slots.begin(),
                    new_end,
                    is_available_slot
                );

                return result != new_end ? result : handle_slots.end();
            }

        public:
            explicit constexpr handle_manager(
                allocator_type * const allocator = nullptr,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS
            ) noexcept : handle_slots{allocator, alloc_flags} {}

            constexpr handle_manager(const handle_manager &other) = default;
            constexpr handle_manager(handle_manager &&other) noexcept = default;
            virtual ~handle_manager() = default;

            handle_manager& operator=(const handle_manager &other) = default;
            handle_manager& operator=(handle_manager &&other) noexcept = default;

            /**
             * Acquire a new handle.
             *
             * @return The newly acquired handle.
             */
            CCLNODISCARD handle_type acquire() {
                typename vector_type::iterator result = find_next_slot();

                if(result == handle_slots.end()) {
                    last_slot_index = handle_slots.size();

                    add_page();
                    result = handle_slots.begin() + last_slot_index;
                }

                const value_type generation = get_slot_generation(*result);
                const value_type index = last_slot_index;

                last_slot_index = (result - handle_slots.begin() + 1) & (handle_slots.size() - 1);
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
            bool is_valid(const handle_type handle) const {
                const auto index = handle.value();
                const auto generation = handle.generation();

                if(index < handle_slots.size()) {
                    const auto slot = handle_slots[index];

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
                CCL_THROW_IF(!is_valid(handle), std::invalid_argument{"Invalid handle."});

                const auto index = handle.value();
                const auto generation = handle.generation();

                if constexpr(expiry_policy == handle_expiry_policy::recycle) {
                    handle_slots[index] = choose(
                        generation + 1,
                        0,
                        generation < handle_type::max_generation - 1
                    ); // Avoid invalid handle value
                } else {
                    handle_slots[index] = generation + 1;
                }

                handle_slots[index] |= value_unused_mask;
            }

            /**
             * Resets all expired handles to the 0th generation so they can be used again.
             */
            template<bool Enable = expiry_policy == handle_expiry_policy::discard>
            std::enable_if_t<Enable>
            reset_expired() {
                std::for_each(handle_slots.begin(), handle_slots.end(), [] (value_type& slot) {
                    if(!is_slot_used(slot) && get_slot_generation(slot) == handle_type::max_generation) {
                        slot = value_unused_mask;
                    }
                });
            }

            /**
             * Reset all handles to the 0th generation. This is equivalent to
             * creating a new handle manager.
             */
            void reset() {
                std::fill(handle_slots.begin(), handle_slots.end(), value_unused_mask);
                last_slot_index = 0;
            }

            /**
             * Invoke a given callback for all acquired handles.
             *
             * @param callback The callback to invoke.
             */
            void for_each(const handle_callback callback) const {
                const size_t slot_count = handle_slots.size();

                for(value_type i = 0; i < slot_count; ++i) {
                    const value_type slot = handle_slots[i];
                    bool is_slot_in_use = is_slot_used(slot);

                    if constexpr(expiry_policy == handle_expiry_policy::discard) {
                        is_slot_in_use = is_slot_in_use && get_slot_generation(slot) < handle_type::max_generation;
                    }

                    if(is_slot_used(slot)) {
                        const value_type generation = get_slot_generation(slot);
                        const handle_type handle = handle_type::make(generation, i);

                        callback(handle);
                    }
                }
            }

            constexpr allocator_type* get_allocator() const noexcept { return handle_slots.get_allocator(); }
            constexpr allocation_flags get_allocation_flags() const noexcept { return handle_slots.get_allocation_flags(); }
    };
}

#endif // CCL_HANDLE_MANAGER_HPP
