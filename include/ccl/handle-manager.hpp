#ifndef CCL_HANDLE_MANAGER_HPP
#define CCL_HANDLE_MANAGER_HPP

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

        public:
            handle_type acquire();
            void release(const handle_t handle);
    };
}

#endif // CCL_HANDLE_MANAGER_HPP
