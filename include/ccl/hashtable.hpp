/**
 * @file
 *
 * Open-addressed hash table.
 */
#ifndef CCL_HASHTABLE_HPP
#define CCL_HASHTABLE_HPP

#include <algorithm>
#include <ccl/api.hpp>
#include <ccl/allocator.hpp>
#include <ccl/concepts.hpp>
#include <ccl/debug.hpp>
#include <ccl/compressed-pair.hpp>
#include <ccl/hash.hpp>

namespace ccl {
    template<typename Hashtable>
    struct hashtable_iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;

        using key_type = typename Hashtable::key_type;
        using value_type = typename Hashtable::value_type;

        using key_pointer = typename Hashtable::key_pointer;
        using value_pointer = typename Hashtable::value_pointer;

        using key_reference = typename Hashtable::key_reference;
        using value_reference = typename Hashtable::value_reference;

        using const_key_reference = typename Hashtable::const_key_reference;
        using const_value_reference = typename Hashtable::const_value_reference;

        using key_value_pair = compressed_pair<key_reference, value_reference>;
        using const_key_value_pair = compressed_pair<const_key_reference, const_value_reference>;

        constexpr hashtable_iterator() noexcept : key{nullptr}, value{nullptr} {}
        constexpr hashtable_iterator(const key_pointer key, const value_pointer value) noexcept : key{key}, value{value} {}
        constexpr hashtable_iterator(const hashtable_iterator &other) noexcept : key{other.key}, value{other.value} {}
        constexpr hashtable_iterator(hashtable_iterator &&other) noexcept : key{std::move(other.key)}, value{std::move(other.value)} {}

        constexpr hashtable_iterator& operator =(const hashtable_iterator &other) noexcept {
            key = other.key;
            value = other.value;

            return *this;
        }

        constexpr hashtable_iterator& operator =(hashtable_iterator &&other) noexcept {
            key = std::move(other.key);
            value = std::move(other.value);

            return *this;
        }

        constexpr key_value_pair operator*() noexcept { return key_value_pair{ *key, *value }; }
        constexpr const_key_value_pair operator*() const noexcept { return const_key_value_pair{ *key, *value }; }
        constexpr value_pointer operator->() const noexcept { return value; } // TODO: What value should this be?

        constexpr bool operator ==(const hashtable_iterator other) const noexcept { return key == other.key; }
        constexpr bool operator !=(const hashtable_iterator other) const noexcept { return key != other.key; }
        constexpr bool operator >(const hashtable_iterator other) const noexcept { return key > other.key; }
        constexpr bool operator <(const hashtable_iterator other) const noexcept { return key < other.key; }
        constexpr bool operator >=(const hashtable_iterator other) const noexcept { return key >= other.key; }
        constexpr bool operator <=(const hashtable_iterator other) const noexcept { return key <= other.key; }

        constexpr hashtable_iterator& operator --() noexcept {
            --key;
            --value;
            return *this;
        }

        constexpr hashtable_iterator operator --(int) noexcept {
            return {key--, value--};
        }

        constexpr hashtable_iterator& operator ++() noexcept {
            ++key;
            ++value;
            return *this;
        }

        constexpr hashtable_iterator operator ++(int) noexcept {
            return {key++, value++};
        }

        private:
            key_pointer key;
            value_pointer value;
    };

    template<typename Hashtable>
    static constexpr hashtable_iterator<Hashtable> operator +(
        const typename hashtable_iterator<Hashtable>::difference_type n,
        const hashtable_iterator<Hashtable> it
    ) noexcept {
        return hashtable_iterator<Hashtable>{it.get_data() + n};
    }

    /**
     * An open-addressed hash table.
     *
     * @tparam K Key type. Must be hashable.
     * @tparam V Value type.
     * @tparam HashFunction The function used to compute the key hashes.
     * @tparam Allocator The allocator type.
     */
    template<
        typename K,
        typename V,
        typename HashFunction = hash<K>,
        typename Allocator = allocator
    >
    requires typed_allocator<Allocator, K> && typed_allocator<Allocator, V>
    class hashtable {
        public:
            using size_type = size_t;

            using key_type = K;
            using value_type = V;
            using hash_type = typename HashFunction::hash_type;
            using hash_function_type = HashFunction;

            using hash_pointer = hash_type*;
            using key_pointer = K*;
            using value_pointer = V*;

            using key_reference = K&;
            using value_reference = V&;

            using rvalue_key_reference = K&&;
            using rvalue_value_reference = V&&;

            using const_key_reference = const K&;
            using const_value_reference = const V&;

            using allocator_type = Allocator;

            using iterator = hashtable_iterator<hashtable>;
            using const_iterator = hashtable_iterator<hashtable<const key_type, const value_type, allocator_type>>;

            static constexpr hash_type invalid_hash = 0;

            explicit constexpr hashtable(
                allocator_type * const allocator = nullptr
            ) : _capacity{0}, hashes{nullptr}, keys{nullptr}, values{nullptr}, allocator{allocator}
            {}

            constexpr hashtable(const hashtable &other)
                : _capacity{other._capacity}, keys{other.keys}, values{other.values}, allocator{other.allocator}
            {}

            constexpr hashtable(hashtable &&other)
                : _capacity{std::move(other._capacity)},
                keys{std::move(other.keys)},
                values{std::move(other.values)},
                allocator{std::move(other.allocator)}
            {}

            // template<typename Pair>
            // constexpr hashtable(
            //     std::initializer_list<Pair> values,
            //     allocator_type * const allocator = nullptr
            // ) : hashtable{allocator} {
            //     std::for_each(
            //         values.begin(),
            //         values.end(),
            //         [this] (const Pair &pair) {
            //             (*this)[pair.first] = pair.second;
            //         }
            //     );
            // }

            // template<typename InputRange>
            // requires std::ranges::input_range<InputRange>
            // constexpr hashtable(
            //     InputRange input,
            //     allocator_type * const allocator = nullptr
            // ) : hashtable{allocator} {
            //     std::for_each(
            //         input.begin(),
            //         input.end(),
            //         [this] (const auto &pair) {
            //             (*this)[pair.first] = pair.second;
            //         }
            //     );
            // }

            ~hashtable() {
                destroy();
            }

            void destroy() noexcept {
                allocator->deallocate(hashes);
                allocator->deallocate(keys);
                allocator->deallocate(values);

                _capacity = 0;
                hashes = nullptr;
                keys = nullptr;
                values = nullptr;
            }

            constexpr size_type size() const noexcept {
                return _capacity;
            }

            constexpr hashtable& operator =(const hashtable &other) {
                keys = other.keys;
                values = other.values;

                return *this;
            }

            constexpr hashtable& operator =(hashtable &&other) {
                keys = std::move(other.keys);
                values = std::move(other.values);

                return *this;
            }

            constexpr size_type capacity() const noexcept { return _capacity; }

            constexpr void resize(const size_type new_capacity) {
                if(new_capacity <= _capacity) {
                    return;
                }

                const hash_pointer new_hashes = allocator->template allocate<hash_type>(new_capacity);
                const key_pointer new_keys = allocator->template allocate<key_type>(new_capacity);
                const value_pointer new_values = allocator->template allocate<value_type>(new_capacity);

                std::memset(new_hashes, 0, sizeof(hash_type) * new_capacity);

                for(size_type i = 0; i < _capacity; ++i) {
                    const hash_type current_hash = hashes[i];

                    if(current_hash != invalid_hash) {
                        const_key_reference current_key = keys[i];
                        const_value_reference current_value = values[i];

                        // TODO: Rehash
                    }
                }

                // ...
                _capacity = new_capacity;
            }

        private:
            static void insert(
                const size_type capacity,
                const key_pointer keys,
                const value_pointer values,
                const_key_reference key,
                const_value_reference value
            ) {
                const hash_type key_hash = hash(key);
                size_type key_index = key_hash % capacity; // TODO: Find faster way of computing this

                for(; key_index < key_slot_count; ++key_index) {

                }
            }

            static hash_type hash(const_key_reference x) {
                return hash_function_type{}(x);
            }

            size_type _capacity = 0;
            hash_pointer hashes = nullptr;
            key_pointer keys = nullptr;
            value_pointer values = nullptr;
            allocator_type *allocator = nullptr;
    };
}

#endif // CCL_HASHTABLE_HPP
