#ifndef CCL_POINTER_UNIQUE_HPP
#define CCL_POINTER_UNIQUE_HPP

#include <ccl/api.hpp>
#include <ccl/concepts.hpp>
#include <ccl/pointer/base.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/internal/optional-allocator.hpp>

namespace ccl {
    namespace internal {
        template<typename T>
        void default_unique_ptr_deleter(std::decay_t<std::remove_all_extents_t<T>>* const ptr) {
            static constexpr bool is_array = std::is_array_v<T>;

            if constexpr(is_array) {
                delete[] ptr;
            } else {
                delete ptr;
            }
        }
    };

    template<typename T, deleter<T> Deleter = internal::default_unique_ptr_deleter, typed_allocator<T> Allocator = allocator>
    class unique_ptr : public base_ptr<T>, private internal::with_optional_allocator<Allocator> {
        using alloc = internal::with_optional_allocator<Allocator>;

        public:
            using allocator_type = Allocator;
            using pointer = base_ptr<T>::pointer;
            using const_pointer = base_ptr<T>::const_pointer;

        private:
            pointer ptr;
    };
}

#endif // CCL_POINTER_UNIQUE_HPP
