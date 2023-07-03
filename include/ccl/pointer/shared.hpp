/**
 * @file
 *
 * Shared pointer.
 */
#ifndef CCL_POINTER_SHARED_HPP
#define CCL_POINTER_SHARED_HPP

#include <utility>
#include <ccl/api.hpp>
#include <ccl/debug.hpp>
#include <ccl/atomic.hpp>
#include <ccl/concepts.hpp>
#include <ccl/util.hpp>
#include <ccl/either.hpp>
#include <ccl/type-traits.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/pointer/base.hpp>
#include <ccl/internal/optional-allocator.hpp>

namespace ccl {
    namespace internal {
        class shared_ptr_ctrl_block_base {
            public:
                using counter_base_type = uint64_t;
                using counter_type = atomic<counter_base_type>;

                static constexpr counter_base_type shared_ref_count_mask = 0xffffffff;
                static constexpr counter_base_type weak_ref_count_mask = 0x00000000'ffffffff;
                static constexpr counter_base_type weak_ref_count_shift_width = (sizeof(counter_base_type) * 8) >> 1;

                static constexpr counter_base_type extract_shared_ref_count(const counter_base_type counters) {
                    return counters & shared_ref_count_mask;
                }

                static constexpr counter_base_type extract_weak_ref_count(const counter_base_type counters) {
                    return (counters & weak_ref_count_mask) >> weak_ref_count_shift_width;
                }

            private:
                /**
                 * An atomic counter merging together shared and weak
                 * pointer reference counts.
                 *
                 * The high 32 bits of this counter will contain the counter
                 * of weak references, while the low 32 bits will contain the
                 * counter of shared references.
                 *
                 * Since the control block is created with the first shared pointer,
                 * the counter is initialised to 1 shared / 0 weak references.
                 */
                counter_type counters;

            public:
                shared_ptr_ctrl_block_base(const shared_ptr_ctrl_block_base &other)
                    : counters{other.counters.load(memory_order_relaxed)}
                {}

                explicit shared_ptr_ctrl_block_base(const counter_base_type counters = 1) : counters{counters} {}
                virtual ~shared_ptr_ctrl_block_base() = default;

                virtual void invoke_deleter() = 0;

                uint32_t decr_shared_ref_count() {
                    return counters.sub_fetch(1, memory_order_relaxed);
                }

                uint32_t incr_shared_ref_count() {
                    return counters.add_fetch(1, memory_order_relaxed);
                }

                uint32_t decr_weak_ref_count() {
                    return counters.sub_fetch(0x0000'0001'0000'0000, memory_order_relaxed);
                }

                uint32_t incr_weak_ref_count() {
                    return counters.add_fetch(0x0000'0001'0000'0000, memory_order_relaxed);
                }

                size_t get_counters() {
                    return counters.load(memory_order_relaxed);
                }
        };

        template<typename T, deleter<std::decay_t<std::remove_all_extents_t<T>>> D>
        class shared_ptr_ctrl_block final : public shared_ptr_ctrl_block_base {
            public:
                using pointer = std::decay_t<std::remove_all_extents_t<T>>*;
                using deleter_type = D;

            private:
                deleter_type deleter;
                pointer ptr;

            public:
                shared_ptr_ctrl_block() = delete;
                shared_ptr_ctrl_block(const pointer ptr, const deleter_type deleter) : deleter{deleter}, ptr{ptr} {}
                shared_ptr_ctrl_block(const shared_ptr_ctrl_block &other)
                    : shared_ptr_ctrl_block_base{other},
                    deleter{other.deleter},
                    ptr{other.ptr}
                {}

                void invoke_deleter() override {
                    CCL_ASSERT(ptr);

                    deleter(ptr);
                    ptr = nullptr;
                }
        };
    }

    template<typename T, typed_allocator<T> Allocator = allocator>
    class shared_ptr : public base_ptr<T>, private internal::with_optional_allocator<Allocator> {
        using alloc = internal::with_optional_allocator<Allocator>;

        public:
            using value_type = std::decay_t<std::remove_all_extents_t<T>>;
            using pointer = value_type*;
            using const_pointer = const value_type *;
            using reference = value_type&;
            using const_reference = const value_type&;
            using ctrl_block_type = internal::shared_ptr_ctrl_block_base;
            using allocator_type = Allocator;

        private:
            mutable ctrl_block_type *ctrl_block = nullptr;

            static constexpr bool is_array = std::is_array_v<T>;

            static void default_deleter(const pointer ptr) {
                if constexpr(is_array) {
                    delete[] ptr;
                } else {
                    delete ptr;
                }
            }

            void decr_ref_count() {
                CCL_ASSERT(ctrl_block);
                ctrl_block->decr_shared_ref_count();
            }

            void incr_ref_count() {
                CCL_ASSERT(ctrl_block);
                ctrl_block->incr_shared_ref_count();
            }

            void invoke_deleter() {
                CCL_ASSERT(ctrl_block);
                ctrl_block->invoke_deleter();
            }

            void delete_ctrl_block() {
                CCL_ASSERT(ctrl_block);

                ctrl_block->~ctrl_block_type();
                alloc::get_allocator()->deallocate(ctrl_block);

                ctrl_block = nullptr;
            }

            void decr_ref_count_and_maybe_delete() {
                const ctrl_block_type::counter_base_type counters = ctrl_block->decr_shared_ref_count();

                if(ctrl_block_type::extract_shared_ref_count(counters) == 0) {
                    invoke_deleter();
                }

                if(counters == 0) {
                    delete_ctrl_block();
                }
            }

            template<deleter<value_type> Deleter>
            void create_ctrl_block(const Deleter d = default_deleter) {
                using ctrl_block_type = internal::shared_ptr_ctrl_block<T, Deleter>;

                ctrl_block = alloc::get_allocator()->template allocate<ctrl_block_type>(1, 0);

                new (ctrl_block) ctrl_block_type{base_ptr<T>::get(), d};
            }

        public:
            shared_ptr() = default;
            shared_ptr(std::nullptr_t) noexcept {}

            shared_ptr(const shared_ptr &other) : base_ptr<T>(other), ctrl_block{other.ctrl_block} {
                if(ctrl_block) {
                    incr_ref_count();
                }
            }

            template<typename U>
            shared_ptr(const shared_ptr<U, Allocator> &other) : base_ptr<T>(other), ctrl_block{other.get_ctrl_block()} {
                if(ctrl_block) {
                    incr_ref_count();
                }
            }

            shared_ptr(shared_ptr &&other)
                : base_ptr<T>(std::move(other))
            {
                ccl::swap(ctrl_block, other.ctrl_block);
            }

            template<typename U>
            shared_ptr(shared_ptr<U, Allocator> &&other)
                : base_ptr<T>(std::move(other))
            {
                ctrl_block = other.get_ctrl_block();
                other.set_ctrl_block(nullptr);
            }

            template<deleter<value_type> Deleter>
            shared_ptr(const pointer ptr, const Deleter d, allocator_type * const allocator = nullptr)
                : base_ptr<T>{ptr},
                alloc{allocator}
            {
                create_ctrl_block(d);
            }

            shared_ptr(const pointer ptr) : shared_ptr{ptr, default_deleter} {}

            ~shared_ptr() {
                if(ctrl_block) { decr_ref_count_and_maybe_delete(); }
            }

            reference operator *() {
                return *base_ptr<T>::get();
            }

            const_reference operator *() const {
                return *base_ptr<T>::get();
            }

            pointer operator ->() {
                return base_ptr<T>::get();
            }

            const_pointer operator ->() const {
                return base_ptr<T>::get();
            }

            reference operator[](const size_t index) {
                return base_ptr<T>::get()[index];
            }

            const_reference operator[](const size_t index) const {
                return base_ptr<T>::get()[index];
            }

            size_t use_count() const noexcept {
                if(ctrl_block) {
                    return ctrl_block_type::extract_shared_ref_count(ctrl_block->get_counters());
                }

                return 0;
            }

            bool unique() const noexcept {
                return use_count() == 1;
            }

            operator bool() const noexcept {
                return ctrl_block != nullptr;
            }

            template<class U>
            bool owner_before(const shared_ptr<U>& other) const noexcept {
                return ctrl_block < other.ctrl_block;
            }

            void reset() {
                if(ctrl_block) {
                    decr_ref_count_and_maybe_delete();
                }

                base_ptr<T>::set(nullptr);
                ctrl_block = nullptr;
            }

            shared_ptr& operator=(const shared_ptr &other) {
                if(ctrl_block) {
                    decr_ref_count_and_maybe_delete();
                }

                base_ptr<T>::set(const_cast<pointer>(other.get()));
                alloc::operator=(other);
                ctrl_block = other.ctrl_block;

                incr_ref_count();

                return *this;
            }

            template<typename U>
            shared_ptr& operator=(const shared_ptr<U, Allocator> &other) {
                if(ctrl_block) {
                    decr_ref_count_and_maybe_delete();
                }

                base_ptr<T>::set(
                    const_cast<pointer>(
                        static_cast<const_pointer>(other.get())
                    )
                );

                alloc::operator=(other);
                ctrl_block = other.ctrl_block;

                incr_ref_count();

                return *this;
            }

            template<typename U>
            shared_ptr& operator=(shared_ptr<U, Allocator> &&other) {
                base_ptr<T>::operator=(std::move(other));
                alloc::operator=(std::move(other));

                ccl::swap(ctrl_block, other.ctrl_block);

                return *this;
            }

            template<typename U>
            void reset(U * const ptr) {
                *this = shared_ptr{ptr};
            }

            constexpr void swap(shared_ptr &other) noexcept {
                base_ptr<T>::swap(other);
                alloc::swap(other);
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

            /**
             * @internal
             *
             * Set the control block.
             *
             * @param value The new control block value.
             */
            void set_ctrl_block(ctrl_block_type * const value) const {
                ctrl_block = value;
            }
    };

    template<typename T, typename Allocator>
    auto operator <=>(const shared_ptr<T, Allocator> &a, const shared_ptr<T, Allocator> &b) {
        return a.get() <=> b.get();
    }

    template<typename T, typename Allocator>
    bool operator ==(const shared_ptr<T, Allocator> &a, const shared_ptr<T, Allocator> &b) {
        return a.get() == b.get();
    }

    template<typename T, typename Allocator>
    bool operator !=(const shared_ptr<T, Allocator> &a, const shared_ptr<T, Allocator> &b) {
        return a.get() != b.get();
    }
}

#endif // CCL_POINTER_SHARED_HPP
