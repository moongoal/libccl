/**
 * @file
 *
 * Shared pointer.
 */
#ifndef CCL_POINTER_SHARED_HPP
#define CCL_POINTER_SHARED_HPP

#include <atomic>
#include <utility>
#include <memory>
#include <ccl/api.hpp>
#include <ccl/debug.hpp>
#include <ccl/concepts.hpp>
#include <ccl/util.hpp>
#include <ccl/either.hpp>
#include <ccl/type-traits.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/pointer/base.hpp>
#include <ccl/internal/optional-allocator.hpp>

namespace ccl {
    class shared_ptr_ctrl_block_base {
        public:
            using counter_base_type = uint64_t;
            using counter_type = std::atomic<counter_base_type>;

            static constexpr counter_base_type shared_ref_count_mask = 0xffffffff;
            static constexpr counter_base_type weak_ref_count_mask =   0xffffffff'00000000;
            static constexpr counter_base_type weak_ref_count_shift_width = (sizeof(counter_base_type) * 8) >> 1;
            static constexpr counter_base_type shared_ref_unit_value = 1;
            static constexpr counter_base_type weak_ref_unit_value = 0x0000'0001'0000'0000;

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
                : counters{other.counters.load(std::memory_order_relaxed)}
            {}

            explicit shared_ptr_ctrl_block_base(const counter_base_type counters = 1) : counters{counters} {}
            virtual ~shared_ptr_ctrl_block_base() = default;

            virtual void invoke_deleter() = 0;

            counter_base_type decr_shared_ref_count() {
                return counters.fetch_sub(shared_ref_unit_value, std::memory_order_relaxed) - shared_ref_unit_value;
            }

            counter_base_type incr_shared_ref_count() {
                return counters.fetch_add(shared_ref_unit_value, std::memory_order_relaxed) + shared_ref_unit_value;
            }

            counter_base_type decr_weak_ref_count() {
                return counters.fetch_sub(weak_ref_unit_value, std::memory_order_relaxed) - weak_ref_unit_value;
            }

            counter_base_type incr_weak_ref_count() {
                return counters.fetch_add(weak_ref_unit_value, std::memory_order_relaxed) + weak_ref_unit_value;
            }

            counter_base_type get_counters() {
                return counters.load(std::memory_order_relaxed);
            }

            bool compare_exchange_counters(counter_base_type &expected, const counter_base_type new_value) noexcept {
                return counters.compare_exchange_weak(expected, new_value);
            }

            virtual void *get_allocator() noexcept = 0;
    };

    template<typename T, deleter<std::decay_t<std::remove_all_extents_t<T>>, shared_ptr_ctrl_block_base> D, basic_allocator Allocator = allocator>
    class shared_ptr_ctrl_block final : public shared_ptr_ctrl_block_base, private internal::with_optional_allocator<Allocator> {
        using alloc = internal::with_optional_allocator<Allocator>;

        public:
            using pointer = std::decay_t<std::remove_all_extents_t<T>>*;
            using deleter_type = D;
            using allocator_type = Allocator;

        private:
            deleter_type deleter;
            pointer ptr;

        public:
            shared_ptr_ctrl_block() = delete;

            shared_ptr_ctrl_block(
                const pointer ptr,
                const deleter_type deleter,
                allocator_type * const allocator
            ) : alloc{allocator}, deleter{deleter}, ptr{ptr} {}

            shared_ptr_ctrl_block(const shared_ptr_ctrl_block &) = delete;

            void invoke_deleter() override {
                CCL_ASSERT(ptr);

                deleter(ptr, *this);
                ptr = nullptr;
            }

            virtual void *get_allocator() noexcept override { return alloc::get_allocator(); }
    };

    template<typename T, basic_allocator Allocator = allocator>
    class shared_ptr final : public base_ptr<T> {
        public:
            using value_type = std::decay_t<std::remove_all_extents_t<T>>;
            using pointer = value_type*;
            using const_pointer = const value_type*;
            using reference = value_type&;
            using const_reference = const value_type&;
            using ctrl_block_type = shared_ptr_ctrl_block_base;
            using allocator_type = Allocator;

            struct new_tag_t {};

            /**
             * Use this tag when constructing a shared pointer to
             * indicate the memory about to become owned by the
             * shared pointer object was allocated via `new` and
             * must thus be deleted via `delete`.
             */
            static constexpr new_tag_t new_tag;

        private:
            mutable ctrl_block_type *ctrl_block = nullptr;

            static void default_deleter(const pointer ptr, ctrl_block_type &ctrl_block) {
                std::destroy_at(ptr);

                allocator_type * const allocator = reinterpret_cast<allocator_type*>(
                    ctrl_block.get_allocator()
                );

                allocator->deallocate(ptr);
            }

            static void default_delete_deleter(const pointer ptr, ctrl_block_type &) {
                std::destroy_at(ptr);

                if constexpr(std::is_array_v<T>) {
                    delete [] ptr;
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

                allocator_type * const allocator = reinterpret_cast<allocator_type*>(
                    ctrl_block->get_allocator()
                );

                ctrl_block->~ctrl_block_type();
                allocator->deallocate(ctrl_block);

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

            template<deleter<value_type, ctrl_block_type> Deleter>
            void create_ctrl_block(
                allocator_type * allocator,
                const allocation_flags alloc_flags,
                const Deleter d
            ) {
                using ctrl_block_type = shared_ptr_ctrl_block<T, Deleter, allocator_type>;

                if(!allocator) {
                    allocator = get_default_allocator<allocator_type>();
                }

                ctrl_block = new (
                    allocator->allocate(sizeof(ctrl_block_type), alignof(ctrl_block_type), alloc_flags)
                ) ctrl_block_type{base_ptr<T>::get(), d, allocator};
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

            template<deleter<value_type, ctrl_block_type> Deleter>
            shared_ptr(
                const pointer ptr,
                const Deleter d,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS,
                allocator_type * const allocator = nullptr
            )
                : base_ptr<T>{ptr}
            {
                create_ctrl_block(allocator, alloc_flags, d);
            }

            shared_ptr(const pointer ptr) : shared_ptr{ptr, default_deleter} {}
            shared_ptr(const pointer ptr, new_tag_t) : shared_ptr{ptr, default_delete_deleter} {}

            // NOTE: This is used internally by the weak pointer implementation
            // and expects the control block to be already up to date with a new
            // shared reference when this constructor is called.
            shared_ptr(
                const pointer ptr,
                ctrl_block_type &ctrl_block
            )
                : base_ptr<T>{ptr},
                ctrl_block{&ctrl_block}
            {}

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

                ctrl_block = other.ctrl_block;

                incr_ref_count();

                return *this;
            }

            template<typename U>
            shared_ptr& operator=(shared_ptr<U, Allocator> &&other) {
                base_ptr<T>::operator=(std::move(other));
                ccl::swap(ctrl_block, other.ctrl_block);

                return *this;
            }

            constexpr void swap(shared_ptr &other) noexcept {
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
    constexpr auto operator <=>(const shared_ptr<T, Allocator> &a, const shared_ptr<T, Allocator> &b) noexcept {
        return a.get() <=> b.get();
    }

    template<typename T, typename Allocator>
    constexpr bool operator ==(const shared_ptr<T, Allocator> &a, const shared_ptr<T, Allocator> &b) noexcept {
        return a.get() == b.get();
    }

    template<typename T, typename Allocator>
    constexpr bool operator !=(const shared_ptr<T, Allocator> &a, const shared_ptr<T, Allocator> &b) noexcept {
        return a.get() != b.get();
    }

    /**
     * Create a new shared pointer by allocating memory via the default allocator
     * of the given type.
     *
     * @tparam T the value type
     * @tparam Allocator The allocator type. The default allocator of this type will be
     *  used to allocate both the control block and the data.
     *
     * @param alloc_flags Allocation flags for the new value. This does not affect allocation
     *  of the control block, which will be allocated with the default allocation flags.
     * @param args The arguments to pass to the constructor.
     *
     * @return A new shared pointer.
     */
    template<typename T, basic_allocator Allocator = allocator, typename ...Args>
    shared_ptr<T, Allocator> make_shared_default(const allocation_flags alloc_flags, Args&& ...args) noexcept {
        return shared_ptr<T, Allocator>{
            new (get_default_allocator<Allocator>()->allocate(sizeof(T), alignof(T), alloc_flags))
            T(std::forward<Args>(args)...)
        };
    }

    /**
     * Create a new shared pointer by allocating memory via `new`.
     *
     * @tparam T the value type
     * @tparam Allocator The allocator type. The default allocator of this type will be
     *  used to allocate the control block.
     * @param args The arguments to pass to the constructor.
     *
     * @return A new shared pointer.
     */
    template<typename T, basic_allocator Allocator = allocator, typename ...Args>
    shared_ptr<T, Allocator> make_shared_new(Args&& ...args) noexcept {
        return shared_ptr<T, Allocator>{
            new T(std::forward<Args>(args)...),
            shared_ptr<T, Allocator>::new_tag
        };
    }
}

#endif // CCL_POINTER_SHARED_HPP
