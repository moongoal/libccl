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

    /**
     * Atomic data type wrapper. Operations performed on atomic objects
     * are meant to happen atomically in a concurrent context. Specifying
     * the memory ordering constraint also allows to synchronise operations
     * on shared, atomic data among threads.
     */
    template<typename T>
    class atomic {
        public:
            using value_type = T;
            using reference = T&;

        private:
            value_type value;

        public:
            /**
             * True if the implementation for the given data type is always lock-free.
             */
            static constexpr bool is_always_lock_free = __atomic_always_lock_free(sizeof(T), 0);

            /**
             * Initialise the atomic object. This operation is not atomic.
             */
            constexpr atomic() noexcept(std::is_nothrow_default_constructible_v<T>): value{} {};

            /**
             * Initialise the atomic object. This operation is not atomic.
             *
             * @param value The value to assign to this object.
             */
            constexpr atomic(value_type value) noexcept : value{value} {}
            constexpr atomic(const atomic& other) = delete;

            /**
             * True if this object is lock-free.
             */
            constexpr bool CCLINLINE is_lock_free() const noexcept {
                return __atomic_is_lock_free(sizeof(T), std::addressof(value));
            }

            /**
             * Load the value of this object.
             *
             * @param order The memory ordering constraint.
             *
             * @return The value of this object.
             */
            T CCLINLINE load(const memory_order order = memory_order_seq_cst) const noexcept {
                return __atomic_load_n(std::addressof(value), order);
            }

            /**
             * Store a value to this object.
             *
             * @param value The value to store.
             * @param order The memory ordering constraint.
             *
             */
            void CCLINLINE store(value_type value, const memory_order order = memory_order_seq_cst) noexcept {
                __atomic_store(
                    std::addressof(this->value),
                    std::addressof(value),
                    order
                );
            }

            /**
             * Store a new value to this object and return the old value.
             *
             * @param desired The value to store.
             * @param order The memory ordering constraint.
             *
             * @return The old value of this object.
             */
            value_type CCLINLINE exchange(value_type desired, const memory_order order = memory_order_seq_cst) noexcept {
                return __atomic_exchange_n(
                    std::addressof(value),
                    desired,
                    order
                );
            }

            /**
             * Weakly compare the value of this object with an expected value
             * and if they match, store a desired value to this object.
             *
             * @param expected The expected value to test against.
             * @param desired The value to store.
             * @param success The memory ordering constraint when this operation succeeds.
             * @param failure The memory ordering constraint when this operation fails.
             *
             * @return True if the exchange has happened, false if it didn't.
             */
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

            /**
             * Weakly compare the value of this object with an expected value
             * and if they match, store a desired value to this object.
             *
             * @param expected The expected value to test against.
             * @param desired The value to store.
             * @param order The memory ordering constraint for both success and failure.
             *
             * @return True if the exchange has happened, false if it didn't.
             */
            bool CCLINLINE compare_exchange_weak(
                reference expected,
                value_type desired,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return compare_exchange_weak(expected, desired, order, order);
            }

            /**
             * Strongly compare the value of this object with an expected value
             * and if they match, store a desired value to this object.
             *
             * @param expected The expected value to test against.
             * @param desired The value to store.
             * @param success The memory ordering constraint when this operation succeeds.
             * @param failure The memory ordering constraint when this operation fails.
             *
             * @return True if the exchange has happened, false if it didn't.
             */
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

            /**
             * Strongly compare the value of this object with an expected value
             * and if they match, store a desired value to this object.
             *
             * @param expected The expected value to test against.
             * @param desired The value to store.
             * @param order The memory ordering constraint for both success and failure.
             *
             * @return True if the exchange has happened, false if it didn't.
             */
            bool CCLINLINE compare_exchange_strong(
                reference expected,
                value_type desired,
                const memory_order order = memory_order_seq_cst
            ) noexcept {
                return compare_exchange_strong(expected, desired, order, order);
            }

            /**
             * Add a value to this object, then store and return the result.
             *
             * @param value The value to add.
             * @param order The memory ordering constraint.
             *
             * @return The result of the addition.
             */
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

            /**
             * Subtract a value to this object, then store and return the result.
             *
             * @param value The value to subtract.
             * @param order The memory ordering constraint.
             *
             * @return The result of the subtraction.
             */
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

            /**
             * Perform the AND operation between a value and this object,
             * then store and return the result.
             *
             * @param value The second operand.
             * @param order The memory ordering constraint.
             *
             * @return The result of the AND operation.
             */
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

            /**
             * Perform the XOR operation between a value and this object,
             * then store and return the result.
             *
             * @param value The second operand.
             * @param order The memory ordering constraint.
             *
             * @return The result of the XOR operation.
             */
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

            /**
             * Perform the OR operation between a value and this object,
             * then store and return the result.
             *
             * @param value The second operand.
             * @param order The memory ordering constraint.
             *
             * @return The result of the OR operation.
             */
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

            /**
             * Perform the NAND operation between a value and this object,
             * then store and return the result.
             *
             * @param value The second operand.
             * @param order The memory ordering constraint.
             *
             * @return The result of the NAND operation.
             */
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

            /**
             * Perform the addition between a value and this object,
             * then store the result and return the old value stored
             * in this object.
             *
             * @param value The second operand.
             * @param order The memory ordering constraint.
             *
             * @return The result of the addition.
             */
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

            /**
             * Perform the subtraction between a value and this object,
             * then store the result and return the old value stored
             * in this object.
             *
             * @param value The second operand.
             * @param order The memory ordering constraint.
             *
             * @return The result of the subtraction.
             */
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

            /**
             * Perform the AND operation between a value and this object,
             * then store the result and return the old value stored
             * in this object.
             *
             * @param value The second operand.
             * @param order The memory ordering constraint.
             *
             * @return The result of AND operation.
             */
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

            /**
             * Perform the XOR operation between a value and this object,
             * then store the result and return the old value stored
             * in this object.
             *
             * @param value The second operand.
             * @param order The memory ordering constraint.
             *
             * @return The result of XOR operation.
             */
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

            /**
             * Perform the OR operation between a value and this object,
             * then store the result and return the old value stored
             * in this object.
             *
             * @param value The second operand.
             * @param order The memory ordering constraint.
             *
             * @return The result of OR operation.
             */
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

            /**
             * Perform the NAND operation between a value and this object,
             * then store the result and return the old value stored
             * in this object.
             *
             * @param value The second operand.
             * @param order The memory ordering constraint.
             *
             * @return The result of NAND operation.
             */
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

    /**
     * Atomic flag. This object is meant to be faster than `atomic<bool>` and
     * can only be set, tested or cleared. This is guaranteed to always be lock-free.
     */
    class atomic_flag {
        uint8_t value = 0;

        public:
            /**
             * Initialise a cleared atomic flag.
             * This operation is not atomic.
             */
            constexpr atomic_flag() noexcept = default;

            /**
             * Test the flag and then set it.
             *
             * @param order The memory ordering constraint.
             *
             * @return True if the flag was set, false if the flag was clear.
             */
            bool CCLINLINE test_and_set(const memory_order order = memory_order_seq_cst) noexcept {
                return __atomic_test_and_set(
                    &value,
                    order
                );
            }

            /**
             * Clear the flag.
             *
             * @param order The memory ordering constraint.
             */
            void CCLINLINE clear(const memory_order order = memory_order_seq_cst) noexcept {
                __atomic_clear(&value, order);
            }

            /**
             * Test the flag.
             *
             * @param order The memory ordering constraint.
             *
             * @return True if the flag is set, false if the flag is clear.
             */
            bool CCLINLINE test(const memory_order order = memory_order_seq_cst) const noexcept {
                return __atomic_load_n(&value, order);
            }
    };

    /**
     * Insert a synchronization fence between threads based on the specified memory order.
     *
     * @param order The memory ordering constraint.
     */
    inline void CCLINLINE atomic_thread_fence(const memory_order order = memory_order_seq_cst) noexcept {
        __atomic_thread_fence(order);
    }

    /**
     * Insert a synchronization fence between a thread and signal handlers based in the same thread.
     *
     * @param order The memory ordering constraint.
     */
    inline void CCLINLINE atomic_signal_fence(const memory_order order = memory_order_seq_cst) noexcept {
        __atomic_signal_fence(order);
    }
}

#endif // CCL_ATOMIC_HPP
