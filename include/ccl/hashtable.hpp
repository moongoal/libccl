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
#include <ccl/vector.hpp>
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

            using key_pointer = K*;
            using value_pointer = V*;

            using key_reference = K&;
            using value_reference = V&;

            using rvalue_key_reference = K&&;
            using rvalue_value_reference = V&&;

            using const_key_reference = const K&;
            using const_value_reference = const V&;

            using allocator_type = Allocator;

            using key_vector_type = vector<key_type, allocator_type>;
            using value_vector_type = vector<value_type, allocator_type>;

            using iterator = hashtable_iterator<hashtable>;
            using const_iterator = hashtable_iterator<hashtable<const key_type, const value_type, allocator_type>>;

            explicit constexpr hashtable(
                allocator_type * const allocator = nullptr
            ) : keys{allocator}, values{allocator}
            {}

            constexpr hashtable(const hashtable &other)
                : keys{other.keys}, values{other.values}
            {}

            constexpr hashtable(hashtable &&other)
                : keys{std::move(other.keys)}, values{std::move(other.values)}
            {}

            template<typename Pair>
            constexpr hashtable(
                std::initializer_list<Pair> values,
                allocator_type * const allocator = nullptr
            ) : hashtable{allocator} {
                std::for_each(
                    values.begin(),
                    values.end(),
                    [this] (const Pair &pair) {
                        (*this)[pair.first] = pair.second;
                    }
                );
            }

            template<typename InputRange>
            requires std::ranges::input_range<InputRange>
            constexpr hashtable(
                InputRange input,
                allocator_type * const allocator = nullptr
            ) : hashtable{allocator} {
                std::for_each(
                    input.begin(),
                    input.end(),
                    [this] (const auto &pair) {
                        (*this)[pair.first] = pair.second;
                    }
                );
            }

            ~hashtable() {
                destroy();
            }

            void destroy() noexcept {
                keys.destroy();
                values.destroy();
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

            constexpr size_type capacity() const noexcept { return keys.capacity(); }

            constexpr void reserve(const size_type new_capacity) {
                key_vector_type new_keys;
                value_vector_type new_values;

                new_keys.reserve(new_capacity);
                new_values.reserve(new_capacity);

                for(const pair)
            }

        private:
            key_vector_type keys;
            value_vector_type values;
    };
}

#endif // CCL_HASHTABLE_HPP
