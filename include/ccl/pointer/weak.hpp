#ifndef CCL_POINTER_WEAK_HPP
#define CCL_POINTER_WEAK_HPP

#include <optional>
#include <ccl/api.hpp>
#include <ccl/pointer/base.hpp>
#include <ccl/pointer/shared.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/internal/optional-allocator.hpp>

namespace ccl {
    template<typename T, typed_allocator<T> Allocator = allocator>
    class weak_ptr final : public base_ptr<T> {
        public:
            using value_type = T;
            using allocator_type = Allocator;
            using ctrl_block_type = shared_ptr_ctrl_block_base;
            using shared_ptr_type = shared_ptr<value_type, allocator_type>;

        private:
            mutable ctrl_block_type *ctrl_block = nullptr;

            void delete_ctrl_block() {
                CCL_ASSERT(ctrl_block);

                allocator_type * const allocator = reinterpret_cast<allocator_type*>(
                    ctrl_block->get_allocator()
                );

                ctrl_block->~ctrl_block_type();
                allocator->deallocate(ctrl_block);

                ctrl_block = nullptr;
            }

            void decr_ref_count_and_maybe_delete() {
                const ctrl_block_type::counter_base_type counters = ctrl_block->decr_weak_ref_count();

                if(counters == 0) {
                    delete_ctrl_block();
                }
            }

        public:
            constexpr weak_ptr() noexcept = default;

            constexpr weak_ptr(std::nullptr_t) noexcept : weak_ptr{} {}

            constexpr weak_ptr(const weak_ptr &other) noexcept
                : base_ptr<T>{other},
                ctrl_block{other.ctrl_block}
            {
                if(ctrl_block) {
                    ctrl_block->incr_weak_ref_count();
                }
            }

            constexpr weak_ptr(weak_ptr &&other) noexcept
                : base_ptr<T>{std::move(other)},
                ctrl_block{other.ctrl_block}
            {
                other.ctrl_block = nullptr;
            }

            weak_ptr(shared_ptr_type &sptr) noexcept
                : base_ptr<T>{sptr.get()},
                ctrl_block{sptr.get_ctrl_block()}
            {
                if(ctrl_block) {
                    ctrl_block->incr_weak_ref_count();
                }
            }

            ~weak_ptr() {
                if(ctrl_block) {
                    decr_ref_count_and_maybe_delete();
                }
            }

            shared_ptr_type lock() noexcept(!exceptions_enabled) {
                if(!ctrl_block) {
                    return shared_ptr_type{};
                }

                ctrl_block_type::counter_base_type counter_value = ctrl_block->get_counters();
                ctrl_block_type::counter_base_type new_counter_value;

                do {
                    if(ctrl_block_type::extract_shared_ref_count(counter_value) == 0) CCLUNLIKELY {
                        return shared_ptr_type{};
                    }

                    new_counter_value = counter_value + ctrl_block_type::shared_ref_unit_value;
                } while(ctrl_block->compare_exchange_counters(counter_value, new_counter_value));

                return shared_ptr_type{base_ptr<T>::get(), *ctrl_block};
            }

            constexpr weak_ptr& operator=(const weak_ptr& other) noexcept {
                base_ptr<T>::operator=(other);

                ctrl_block = other.ctrl_block;

                if(ctrl_block) {
                    ctrl_block->incr_weak_ref_count();
                }

                return *this;
            }

            constexpr weak_ptr& operator=(weak_ptr&& other) noexcept {
                swap(other);

                return *this;
            }

            constexpr void swap(weak_ptr &other) noexcept {
                base_ptr<T>::swap(other);
                ccl::swap(ctrl_block, other.ctrl_block);
            }

            /**
             * @internal
             *
             * Get the control block.
             */
            CCLNODISCARD ctrl_block_type* get_ctrl_block() const {
                return ctrl_block;
            }
    };

    template<typename T, typename Allocator>
    constexpr auto operator <=>(const weak_ptr<T, Allocator> &a, const weak_ptr<T, Allocator> &b) noexcept {
        return a.get() <=> b.get();
    }

    template<typename T, typename Allocator>
    constexpr bool operator ==(const weak_ptr<T, Allocator> &a, const weak_ptr<T, Allocator> &b) noexcept {
        return a.get() == b.get();
    }

    template<typename T, typename Allocator>
    constexpr bool operator !=(const weak_ptr<T, Allocator> &a, const weak_ptr<T, Allocator> &b) noexcept {
        return a.get() != b.get();
    }
}

#endif // CCL_POINTER_WEAK_HPP
