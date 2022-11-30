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
#include <ccl/util.hpp>
#include <ccl/bitset.hpp>
#include <ccl/internal/optional-allocator.hpp>

namespace ccl {
    template<typename Hashtable>
    struct hashtable_iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;

        using key_type = typename Hashtable::key_type;
        using value_type = typename Hashtable::value_type;

        using key_value_pair = compressed_pair<key_type, value_type>;
        using const_key_value_pair = compressed_pair<const key_type, const value_type>;

        using hashtable_type = Hashtable;
        using size_type = typename Hashtable::size_type;

        constexpr hashtable_iterator() noexcept : hashtable{nullptr}, index{0} {}

        explicit constexpr hashtable_iterator(hashtable_type& hashtable, const size_type item_index = 0) noexcept : hashtable{&hashtable}, index{item_index} {
            // Ensure we are actually pointing at an existing value or at the end.
            // Useful for `begin()` iterators.
            for(; index < hashtable._capacity; ++index) {
                if(hashtable.availability_map[index]) {
                    return;
                }
            }
        }

        constexpr hashtable_iterator(const hashtable_iterator &other) noexcept : hashtable{other.hashtable}, index{other.index} {}
        constexpr hashtable_iterator(hashtable_iterator &&other) noexcept : hashtable{std::move(other.hashtable)}, index{std::move(other.index)} {}

        constexpr hashtable_iterator& operator =(const hashtable_iterator &other) noexcept {
            hashtable = other.hashtable;
            index = other.index;

            return *this;
        }

        constexpr hashtable_iterator& operator =(hashtable_iterator &&other) noexcept {
            hashtable = std::move(other.hashtable);
            index = std::move(other.index);

            return *this;
        }

        constexpr key_value_pair operator*() noexcept { return key_value_pair{ hashtable->keys[index], hashtable->values[index] }; }
        constexpr const_key_value_pair operator*() const noexcept { return const_key_value_pair{ hashtable->keys[index], hashtable->values[index] }; }

        constexpr key_value_pair* operator->() const noexcept {
            pair = key_value_pair{ hashtable->keys[index], hashtable->values[index] };

            return &pair;
        }

        constexpr bool operator ==(const hashtable_iterator other) const noexcept { return hashtable == other.hashtable && index == other.index; }
        constexpr bool operator !=(const hashtable_iterator other) const noexcept { return hashtable != other.hashtable || index != other.index; }

        constexpr bool operator >(const hashtable_iterator other) const noexcept {
            CCL_THROW_IF(hashtable != other.hashtable, std::runtime_error{"Comparing iterators from different hashtables."});

            return index > other.index;
        }

        constexpr bool operator <(const hashtable_iterator other) const noexcept {
            CCL_THROW_IF(hashtable != other.hashtable, std::runtime_error{"Comparing iterators from different hashtables."});

            return index < other.index;
        }

        constexpr bool operator >=(const hashtable_iterator other) const noexcept {
            CCL_THROW_IF(hashtable != other.hashtable, std::runtime_error{"Comparing iterators from different hashtables."});

            return index >= other.index;
        }

        constexpr bool operator <=(const hashtable_iterator other) const noexcept {
            CCL_THROW_IF(hashtable != other.hashtable, std::runtime_error{"Comparing iterators from different hashtables."});

            return index <= other.index;
        }

        constexpr auto& operator --() const noexcept {
            do {
                index--;
            } while(!hashtable->availability_map[index]);

            return *this;
        }

        constexpr auto operator --(int) const noexcept {
            const size_type old_index = this->index;

            do {
                if(index == 0) {
                    break;
                }

                index--;
            } while(!hashtable->availability_map[index]);

            return hashtable_iterator{*hashtable, old_index};
        }

        constexpr auto& operator ++() const noexcept {
            for(index += 1; index < hashtable->_capacity && !hashtable->availability_map[index]; ++index);

            return *this;
        }

        constexpr auto operator ++(int) const noexcept {
            const size_type old_index = this->index;

            for(; index < hashtable->_capacity && !hashtable->availability_map[index]; ++index);

            return hashtable_iterator{*hashtable, old_index};
        }

        private:
            hashtable_type *hashtable;
            mutable size_type index;
            mutable key_value_pair pair;
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
    class hashtable : private internal::with_optional_allocator<Allocator> {
        friend struct hashtable_iterator<hashtable>;

        using alloc = internal::with_optional_allocator<Allocator>;

        public:
            using size_type = size_t;

            using key_type = K;
            using value_type = V;
            using hash_type = typename HashFunction::hash_value_type;
            using hash_function_type = HashFunction;

            using key_pointer = K*;
            using value_pointer = V*;

            using key_reference = K&;
            using value_reference = V&;

            using const_key_reference = const K&;
            using const_value_reference = const V&;

            using allocator_type = Allocator;

            using iterator = hashtable_iterator<hashtable>;
            using const_iterator = hashtable_iterator<hashtable<const key_type, const value_type, allocator_type>>;

            static constexpr size_type minimum_capacity = CCL_HASHTABLE_MINIMUM_CAPACITY;

            explicit constexpr hashtable(
                allocator_type * const allocator = nullptr
            ) : alloc{allocator}, _capacity{0}, keys{nullptr}, values{nullptr}
            {
                reserve(minimum_capacity);
            }

            constexpr hashtable(const hashtable &other)
                : alloc{other},
                _capacity{other._capacity},
                availability_map{other.availability_map},
                keys{other.keys},
                values{other.values}
            {}

            constexpr hashtable(hashtable &&other)
                : alloc{std::move(other)},
                _capacity{std::move(other._capacity)},
                availability_map{std::move(other.availability_map)},
                keys{std::move(other.keys)},
                values{std::move(other.values)}
            {}

            template<typename InputRange>
            requires std::ranges::input_range<InputRange>
            constexpr hashtable(
                InputRange&& input,
                allocator_type * const allocator = nullptr
            ) : hashtable{allocator} {
                std::for_each(
                    input.begin(),
                    input.end(),
                    [this] (const auto &pair) {
                        insert(pair.first(), pair.second());
                    }
                );
            }

            ~hashtable() {
                destroy();
            }

            void destroy() noexcept {
                availability_map.destroy();
                alloc::get_allocator()->deallocate(keys);
                alloc::get_allocator()->deallocate(values);

                _capacity = 0;
                keys = nullptr;
                values = nullptr;
            }

            constexpr size_type capacity() const noexcept {
                return _capacity;
            }

            constexpr hashtable& operator =(const hashtable &other) {
                alloc::operator =(other);
                availability_map = other.availability_map;
                keys = other.keys;
                values = other.values;

                return *this;
            }

            constexpr hashtable& operator =(hashtable &&other) {
                alloc::operator =(std::move(other));
                availability_map = std::move(other.availability_map);
                keys = std::move(other.keys);
                values = std::move(other.values);

                return *this;
            }

            constexpr void reserve(const size_type new_capacity) {
                if(new_capacity <= _capacity) {
                    return;
                }

                const size_type actual_new_capacity = increase_capacity(_capacity, new_capacity);

                bitset<allocator_type> new_availability_map;
                const key_pointer new_keys = alloc::get_allocator()->template allocate<key_type>(actual_new_capacity);
                const value_pointer new_values = alloc::get_allocator()->template allocate<value_type>(actual_new_capacity);

                new_availability_map.resize(actual_new_capacity);
                new_availability_map.zero();

                for(size_type i = 0; i < _capacity; ++i) {
                    const bool is_available = availability_map[i];

                    if(is_available) {
                        key_reference current_key = keys[i];
                        value_reference current_value = values[i];

                        const size_type new_index = compute_key_index(i, actual_new_capacity);

                        std::construct_at(&new_keys[new_index], std::move(current_key));
                        std::construct_at(&new_values[new_index], std::move(current_value));

                        std::destroy_at(&current_key);
                        std::destroy_at(&current_value);

                        new_availability_map.set(new_index);
                    }
                }

                alloc::get_allocator()->deallocate(keys);
                alloc::get_allocator()->deallocate(values);

                _capacity = actual_new_capacity;
                availability_map = std::move(new_availability_map);
                keys = new_keys;
                values = new_values;
            }

            constexpr void insert(const_key_reference key, const_value_reference value) {
                const size_type index = compute_key_index(key, _capacity);

                for(size_type i = index; i < _capacity; ++i) {
                    if(!availability_map[i] || key == keys[i]) {
                        std::construct_at(&keys[i], key);
                        std::construct_at(&values[i], value);
                        availability_map[i] = true;
                        return;
                    }
                }

                for(size_type i = 0; i < index; ++i) {
                    if(!availability_map[i] || key == keys[i]) {
                        std::construct_at(&keys[i], key);
                        std::construct_at(&values[i], value);
                        availability_map[i] = true;
                        return;
                    }
                }

                // No slots available
                reserve(max<size_type>(1, _capacity << 1));
                insert(key, value);
            }

            template<typename ...Args>
            constexpr void emplace(const_key_reference key, Args&& ...args) {
                const size_type index = compute_key_index(key, _capacity);

                for(size_type i = index; i < _capacity; ++i) {
                    if(!availability_map[i]) {
                        std::construct_at(&keys[i], key);
                        std::construct_at(&values[i], std::forward<Args>(args)...);
                        availability_map[i] = true;
                        return;
                    }
                }

                for(size_type i = 0; i < index; ++i) {
                    if(!availability_map[i]) {
                        std::construct_at(&keys[i], key);
                        std::construct_at(&values[i], std::forward<Args>(args)...);
                        availability_map[i] = true;
                        return;
                    }
                }

                // No slots available
                reserve(_capacity << 1);
                emplace(key, std::forward<Args>(args)...);
            }

            constexpr void erase(const_key_reference key) {
                const size_type index = compute_key_index(key, _capacity);

                for(size_type i = index; i < _capacity; ++i) {
                    if(keys[i] == key) {
                        CCL_ASSERT(availability_map[i]);

                        std::destroy_at(&keys[i]);
                        std::destroy_at(&values[i]);
                        availability_map[i] = false;
                        return;
                    }
                }

                for(size_type i = 0; i < index; ++i) {
                    if(keys[i] == key) {
                        CCL_ASSERT(availability_map[i]);

                        std::destroy_at(&keys[i]);
                        std::destroy_at(&values[i]);
                        availability_map[i] = false;
                        return;
                    }
                }
            }

            constexpr value_reference operator [](const_key_reference key) {
                const size_type index = compute_key_index(key, _capacity);

                CCL_THROW_IF(index >= _capacity, std::invalid_argument{"Invalid key."});

                for(size_type i = index; i < _capacity; ++i) {
                    if(availability_map[i] && keys[i] == key) {
                        return values[i];
                    }
                }

                for(size_type i = 0; i < index; ++i) {
                    if(availability_map[i] && keys[i] == key) {
                        return values[i];
                    }
                }

                CCL_THROW(std::invalid_argument{"Invalid key."});
            }

            constexpr iterator begin() { return hashtable_iterator<hashtable>{ *this, 0 }; }
            constexpr iterator end() { return hashtable_iterator<hashtable>{ *this, _capacity }; }

            constexpr const_iterator begin() const { return hashtable_iterator<hashtable>{ *this, 0 }; }
            constexpr const_iterator end() const { return hashtable_iterator<hashtable>{ *this, _capacity }; }

            constexpr const_iterator cbegin() const { return hashtable_iterator<hashtable>{ *this, 0 }; }
            constexpr const_iterator cend() const { return hashtable_iterator<hashtable>{ *this, _capacity }; }

        private:
            static hash_type hash(const_key_reference x) {
                return hash_function_type{}(x);
            }

            static size_type compute_key_index(const_key_reference x, const size_type capacity) {
                CCL_ASSERT(is_power_2(capacity));

                return hash(x) & (capacity - 1);
            }

            size_type _capacity = 0;
            bitset<allocator_type> availability_map; // Slot availability bit map
            key_pointer keys = nullptr;
            value_pointer values = nullptr;
    };
}

#endif // CCL_HASHTABLE_HPP
