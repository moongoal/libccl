#ifndef CCL_HANDLE_MANAGER_HPP
#define CCL_HANDLE_MANAGER_HPP

#include <algorithm>
#include <ccl/api.hpp>
#include <ccl/hash.hpp>
#include <ccl/handle.hpp>
#include <ccl/paged-vector.hpp>
#include <ccl/util.hpp>
#include <ccl/memory/allocator.hpp>

namespace ccl {
    template<typename ObjectType, typed_allocator<ObjectType> Allocator>
    class handle_manager {
        public:
            using object_type = ObjectType;
            using handle_type = versioned_handle<object_type>;
            using handle_ptr = handle_type*;
            using allocator_type = Allocator;
            using value_type = typename handle_type::value_type;
            using vector_type = paged_vector<value_type, handle_ptr, allocator_type>;

            static constexpr value_type value_used_mask = bitcount(static_cast<value_type>(0) - 1) - 1;

        private:
            /**
             * A vector of available slots. Each value of these contains the current generation of the handle
             * and its MSB is set to 1 if the handle slot is currently not in used.
             */
            vector_type handles;

            size_t last_slot_index = 0;

            /**
             * Test whether a handle slot is used.
             *
             * @param value The slot value to test.
             *
             * @return True if the slot is in use, false if it's available.
             */
            static constexpr bool is_slot_used(const value_type value) noexcept {
                return ~value & value_used_mask;
            }

            /**
             * Get the generation value from a slot.
             *
             * @param value The slot value to test.
             *
             * @return The slot generation.
             */
            static constexpr bool get_slot_generation(const value_type value) noexcept {
                const bool is_used = is_slot_used(value);

                return choose(is_used, value, value | ~value_used_mask);
            }

            /**
             * Add a new page of slots and initialise
             * its values.
             */
            void add_page() {
                handles.resize(handles.size() + vector_type::page_size);
                auto * const last_page = *(handles.pages().end() - 1);
                std::fill(last_page, last_page + vector_type::page_size, value_used_mask);
            }

            /**
             * Find the next available slot. If none is available,
             * return `handles.end()`.
             */
            auto find_next_slot() {
                auto result = std::find_if(handles.begin() + last_slot_index, handles.end(), [] (const value_type slot) {
                    return !is_slot_used(slot);
                });

                if(result) {
                    return result;
                }

                result = std::find_if(handles.begin(), handles.begin() + last_slot_index, [] (const value_type slot) {
                    return !is_slot_used(slot);
                });

                return result;
            }

        public:
            /**
             * Acquire a new handle.
             *
             * @return The newly acquired handle.
             */
            handle_type acquire() {
                auto result = find_next_slot();

                if(result != handles.end()) {
                    const size_t size = handles.size();

                    add_page();
                    result = handles.begin() + size;
                }

                last_slot_index = result - handles.begin();
                *result &= ~value_used_mask;

                return handle_type::make(get_slot_generation(*result), last_slot_index);
            }

            void release(const handle_t handle);
    };
}

#endif // CCL_HANDLE_MANAGER_HPP
