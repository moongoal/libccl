#ifndef CCL_POINTER_BASE_HPP
#define CCL_POINTER_BASE_HPP

#include <cstddef>
#include <type_traits>
#include <ccl/api.hpp>
#include <ccl/util.hpp>

namespace ccl {
    template<typename T>
    class base_ptr {
        public:
            using pointer = std::decay_t<std::remove_all_extents_t<T>>*;
            using const_pointer = const std::decay_t<std::remove_all_extents_t<T>>*;

        private:
            pointer ptr;

        protected:
            static constexpr bool is_array = std::is_array_v<T>;

            void set(const pointer value) { ptr = value; }

            base_ptr() noexcept : ptr{nullptr} {}
            base_ptr(std::nullptr_t) : ptr{nullptr} {}
            base_ptr(const pointer ptr) : ptr{ptr} {}
            base_ptr(const base_ptr&) = default;
            base_ptr(base_ptr&&) = default;

            template<typename U>
            base_ptr(
                std::decay_t<std::remove_all_extents_t<T>>* const ptr
            ) : ptr{static_cast<pointer>(ptr)} {}

            template<typename U>
            base_ptr(
                const base_ptr<U> &ptr
            ) : ptr{
                const_cast<pointer>(
                    static_cast<const_pointer>(ptr.get())
                )
            } {}

            template<typename U>
            base_ptr(
                base_ptr<U> &&ptr
            ) : ptr{static_cast<pointer>(ptr.get())} {}

            base_ptr& operator=(const base_ptr&) = default;

            base_ptr& operator=(base_ptr&& other) {
                swap(other);

                return *this;
            }

            constexpr void swap(base_ptr &other) noexcept {
                ccl::swap(ptr, other.ptr);
            }

        public:
            pointer get() { return ptr; }
            const_pointer get() const { return ptr; }
    };
}

#endif // CCL_POINTER_BASE_HPP
