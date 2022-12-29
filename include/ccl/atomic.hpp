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
            using pointer = T*;
            using const_pointer = const T*;

        private:
            value_type value;

        public:
            constexpr atomic() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
            constexpr atomic(T value) noexcept : value{value} {}
            constexpr atomic(const atomic& other) = delete;

            constexpr bool CCLINLINE is_lock_free() noexcept {
                return __atomic_is_lock_free(sizeof(T), std::addressof(value));
            }

            T CCLINLINE load(const memory_order order = memory_order_seq_cst) noexcept {
                return __atomic_load(std::addressof(value), order);
            }

            void CCLINLINE store(T value, const memory_order order = memory_order_seq_cst) noexcept {
                __atomic_store(
                    std::addressof(this->value),
                    std::addressof(value),
                    order
                );
            }

            T CCLINLINE exchange(T desired, const memory_order order = memory_order_seq_cst) noexcept {
                return __atomic_exchange(
                    std::addressof(value),
                    std::addressof(desired),
                    order
                );
            }

            bool CCLINLINE compare_exchange_weak(
                T& expected,
                T desired,
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
                T& expected,
                T desired,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return compare_exchange_weak(expected, desired, order, order);
            }

            bool CCLINLINE compare_exchange_strong(
                T& expected,
                T desired,
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
                T& expected,
                T desired,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return compare_exchange_weak(expected, desired, order, order);
            }
    };
}

#endif // CCL_ATOMIC_HPP
