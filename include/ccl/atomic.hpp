/**
 * @file
 *
 * Atomic data types.
 */
#ifndef CCL_ATOMIC_HPP
#define CCL_ATOMIC_HPP

#include <utility>
#include <ccl/api.hpp>

namespace ccl {
    /**
     * Memory ordering constraints.
     */
    enum memory_order {
        /**
         * Implies no inter-thread ordering constraints.
         */
        memory_order_relaxed = __ATOMIC_RELAXED,

        /**
         * This is currently implemented using the stronger `memory_order_acquire`
         * memory order because of a deficiency in C++11’s semantics for
         * memory_order_consume.
         *
         * Use of consume semantics is discouraged.
         */
        memory_order_consume = __ATOMIC_CONSUME,

        /**
         * Creates an inter-thread happens-before constraint from the release
         * (or stronger) semantic store to this acquire load. Can prevent
         * hoisting of code to before the operation.
         */
        memory_order_acquire = __ATOMIC_ACQUIRE,

        /**
         * Creates an inter-thread happens-before constraint to acquire
         * (or stronger) semantic loads that read from this release store.
         * Can prevent sinking of code to after the operation.
         */
        memory_order_release = __ATOMIC_RELEASE,

        /**
         * Combines the effects of both `memory_order_acquire` and `memory_order_acq_rel`.
         */
        memory_order_acq_rel = __ATOMIC_ACQ_REL,

        /**
         * Enforces total ordering with all other `memory_order_seq_cst` operations.
         */
        memory_order_seq_cst = __ATOMIC_SEQ_CST
    };

    // https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    template<typename T>
    class atomic {
        public:
            using value_type = T;
            using reference = T&;

        private:
            value_type value;

        public:
            static constexpr bool is_always_lock_free = __atomic_always_lock_free(sizeof(T), 0);

            constexpr atomic() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
            constexpr atomic(value_type value) noexcept : value{value} {}
            constexpr atomic(const atomic& other) = delete;

            constexpr bool CCLINLINE is_lock_free() noexcept {
                return __atomic_is_lock_free(sizeof(T), std::addressof(value));
            }

            T CCLINLINE load(const memory_order order = memory_order_seq_cst) noexcept {
                return __atomic_load_n(std::addressof(value), order);
            }

            void CCLINLINE store(value_type value, const memory_order order = memory_order_seq_cst) noexcept {
                __atomic_store(
                    std::addressof(this->value),
                    std::addressof(value),
                    order
                );
            }

            value_type CCLINLINE exchange(value_type desired, const memory_order order = memory_order_seq_cst) noexcept {
                return __atomic_exchange_n(
                    std::addressof(value),
                    desired,
                    order
                );
            }

            bool CCLINLINE compare_exchange_weak(
                reference expected,
                value_type desired,
                const memory_order success,
                const memory_order failure
            ) noexcept {
                return __atomic_compare_exchange(
                    std::addressof(value),
                    std::addressof(expected),
                    std::addressof(desired),
                    true,
                    success,
                    failure
                );
            }

            bool CCLINLINE compare_exchange_weak(
                reference expected,
                value_type desired,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return compare_exchange_weak(expected, desired, order, order);
            }

            bool CCLINLINE compare_exchange_strong(
                reference expected,
                value_type desired,
                const memory_order success,
                const memory_order failure
            ) noexcept {
                return __atomic_compare_exchange(
                    std::addressof(value),
                    std::addressof(expected),
                    std::addressof(desired),
                    false,
                    success,
                    failure
                );
            }

            bool CCLINLINE compare_exchange_strong(
                reference expected,
                value_type desired,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return compare_exchange_strong(expected, desired, order, order);
            }

            value_type add_fetch(
                const value_type value,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return __atomic_add_fetch(
                    std::addressof(this->value),
                    value,
                    order
                );
            }

            value_type sub_fetch(
                const value_type value,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return __atomic_sub_fetch(
                    std::addressof(this->value),
                    value,
                    order
                );
            }

            value_type and_fetch(
                const value_type value,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return __atomic_and_fetch(
                    std::addressof(this->value),
                    value,
                    order
                );
            }

            value_type xor_fetch(
                const value_type value,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return __atomic_xor_fetch(
                    std::addressof(this->value),
                    value,
                    order
                );
            }

            value_type or_fetch(
                const value_type value,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return __atomic_or_fetch(
                    std::addressof(this->value),
                    value,
                    order
                );
            }

            value_type nand_fetch(
                const value_type value,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return __atomic_nand_fetch(
                    std::addressof(this->value),
                    value,
                    order
                );
            }

            value_type fetch_add(
                const value_type value,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return __atomic_fetch_add(
                    std::addressof(this->value),
                    value,
                    order
                );
            }

            value_type fetch_sub(
                const value_type value,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return __atomic_fetch_sub(
                    std::addressof(this->value),
                    value,
                    order
                );
            }

            value_type fetch_and(
                const value_type value,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return __atomic_fetch_and(
                    std::addressof(this->value),
                    value,
                    order
                );
            }

            value_type fetch_xor(
                const value_type value,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return __atomic_fetch_xor(
                    std::addressof(this->value),
                    value,
                    order
                );
            }

            value_type fetch_or(
                const value_type value,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return __atomic_fetch_or(
                    std::addressof(this->value),
                    value,
                    order
                );
            }

            value_type fetch_nand(
                const value_type value,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return __atomic_fetch_nand(
                    std::addressof(this->value),
                    value,
                    order
                );
            }
    };

    class atomic_flag {
        uint8_t value = 0;

        public:
            static constexpr bool is_always_lock_free = __atomic_always_lock_free(sizeof(uint8_t), 0);

            constexpr atomic_flag() noexcept = default;

            constexpr bool CCLINLINE is_lock_free() noexcept {
                return __atomic_is_lock_free(sizeof(uint8_t), std::addressof(value));
            }

            bool CCLINLINE test_and_set(const memory_order order = memory_order_seq_cst) noexcept {
                return __atomic_test_and_set(
                    &value,
                    order
                );
            }

            void CCLINLINE clear(const memory_order order = memory_order_seq_cst) noexcept {
                __atomic_clear(&value, order);
            }

            bool CCLINLINE test(const memory_order order = memory_order_seq_cst) noexcept {
                return __atomic_load_n(&value, order);
            }
    };

    inline void CCLINLINE atomic_thread_fence(const memory_order order = memory_order_seq_cst) noexcept {
        __atomic_thread_fence(order);
    }

    inline void CCLINLINE atomic_signal_fence(const memory_order order = memory_order_seq_cst) noexcept {
        __atomic_signal_fence(order);
    }
}

#endif // CCL_ATOMIC_HPP
