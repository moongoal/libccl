/**
 * @file
 *
 * Open-addressed hash table.
 */
#ifndef CCL_HASHTABLE_HPP
#define CCL_HASHTABLE_HPP

#include <algorithm>
#include <cstring>
#include <ccl/api.hpp>
#include <ccl/definitions.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/concepts.hpp>
#include <ccl/debug.hpp>
#include <ccl/compressed-pair.hpp>
#include <ccl/hash.hpp>
#include <ccl/util.hpp>
#include <ccl/bitset.hpp>
#include <ccl/internal/optional-allocator.hpp>
#include <ccl/pair.hpp>

namespace ccl {
    template<typename Hashtable>
    struct hashtable_iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept = iterator_category;
        using difference_type = std::ptrdiff_t;

        using key_type = either_or_t<
            const typename Hashtable::key_type,
            typename Hashtable::key_type,
            std::is_const_v<Hashtable>
        >;

        using value_type = either_or_t<
            const typename Hashtable::value_type,
            typename Hashtable::value_type,
            std::is_const_v<Hashtable>
        >;

        using pointer = value_type*;
        using reference = value_type&;
        using key_value_pair = pair<key_type*, value_type*>;

        using hashtable_type = Hashtable;
        using size_type = typename Hashtable::size_type;

        constexpr hashtable_iterator() noexcept : hashtable{nullptr}, index{0} {}

        explicit constexpr hashtable_iterator(hashtable_type& hashtable, const size_type item_index) noexcept : hashtable{&hashtable}, index{item_index} {
            // Ensure we are actually pointing at an existing value or at the end.
            // Useful for `begin()` iterators.
            for(; index < hashtable._capacity; ++index) {
                if(hashtable.slot_map[index]) {
                    return;
                }
            }
        }

        constexpr hashtable_iterator(const hashtable_iterator &other) noexcept : hashtable{other.hashtable}, index{other.index} {}

        constexpr hashtable_iterator& operator =(const hashtable_iterator &other) noexcept {
            hashtable = other.hashtable;
            index = other.index;

            return *this;
        }

        constexpr const key_value_pair operator*() const noexcept { return key_value_pair{ &hashtable->keys[index], &hashtable->values[index] }; }

        constexpr const key_value_pair* operator->() const noexcept {
            pair = key_value_pair{ &hashtable->keys[index], &hashtable->values[index] };

            return &pair;
        }

        constexpr auto& operator --() noexcept {
            do {
                index--;
            } while(!hashtable->slot_map[index]);

            return *this;
        }

        constexpr auto operator --(int) noexcept {
            const size_type old_index = this->index;

            do {
                if(index == 0) {
                    break;
                }

                index--;
            } while(!hashtable->slot_map[index]);

            return hashtable_iterator{*hashtable, old_index};
        }

        constexpr auto& operator ++() noexcept {
            for(index += 1; index < hashtable->_capacity && !hashtable->slot_map[index]; ++index);

            return *this;
        }

        constexpr auto operator ++(int) noexcept {
            const size_type old_index = this->index;

            for(index += 1; index < hashtable->_capacity && !hashtable->slot_map[index]; ++index);

            return hashtable_iterator{*hashtable, old_index};
        }

        hashtable_type *hashtable;
        mutable size_type index;
        mutable key_value_pair pair;
    };

    template<typename Hashtable>
    constexpr bool operator ==(const hashtable_iterator<Hashtable> a, const hashtable_iterator<Hashtable> b) {
        return a.hashtable == b.hashtable && a.index == b.index;
    }

    template<typename Hashtable>
    constexpr bool operator !=(const hashtable_iterator<Hashtable> a, const hashtable_iterator<Hashtable> b) {
        return a.hashtable != b.hashtable || a.index != b.index;
    }

    template<typename Hashtable>
    constexpr bool operator >(const hashtable_iterator<Hashtable> a, const hashtable_iterator<Hashtable> b) {
        CCL_THROW_IF(a.hashtable != b.hashtable, std::runtime_error{"Comparing iterators from different hashtables."});

        return a.index > b.index;
    }

    template<typename Hashtable>
    constexpr bool operator <(const hashtable_iterator<Hashtable> a, const hashtable_iterator<Hashtable> b) {
        CCL_THROW_IF(a.hashtable != b.hashtable, std::runtime_error{"Comparing iterators from different hashtables."});

        return a.index < b.index;
    }

    template<typename Hashtable>
    constexpr bool operator >=(const hashtable_iterator<Hashtable> a, const hashtable_iterator<Hashtable> b) {
        CCL_THROW_IF(a.hashtable != b.hashtable, std::runtime_error{"Comparing iterators from different hashtables."});

        return a.index >= b.index;
    }

    template<typename Hashtable>
    constexpr bool operator <=(const hashtable_iterator<Hashtable> a, const hashtable_iterator<Hashtable> b) {
        CCL_THROW_IF(a.hashtable != b.hashtable, std::runtime_error{"Comparing iterators from different hashtables."});

        return a.index <= b.index;
    }

    /**
     * An open-addressed hash table.
     *
     * @tparam K Key type.
     * @tparam V Value type.
     * @tparam HashFunction The function used to compute the key hashes.
     * @tparam Allocator The allocator type.
     */
    template<
        std::equality_comparable K,
        typename V,
        typed_hash_function<K> HashFunction = hash<K>,
        typename Allocator = allocator
    >
    requires typed_allocator<Allocator, K> && typed_allocator<Allocator, V>
    class hashtable : private internal::with_optional_allocator<Allocator> {
        friend struct hashtable_iterator<hashtable>;
        friend struct hashtable_iterator<const hashtable>;

        using alloc = internal::with_optional_allocator<Allocator>;

        public:
            using size_type = count_t;

            using key_type = K;
            using value_type = V;
            using hash_type = hash_t;
            using hash_function_type = HashFunction;

            using key_pointer = K*;
            using value_pointer = V*;

            using key_reference = K&;
            using value_reference = V&;

            using const_key_reference = const K&;
            using const_value_reference = const V&;

            using allocator_type = Allocator;

            using iterator = hashtable_iterator<hashtable>;
            using const_iterator = hashtable_iterator<const hashtable>;

            static constexpr size_type minimum_capacity = CCL_HASHTABLE_MINIMUM_CAPACITY;

            static constexpr size_type default_chunk_size = min(
                minimum_capacity,
                max<size_type>(
                    CCL_HASHTABLE_MINIMUM_CHUNK_SIZE,
                    1u << (
                        bitcount(std::hardware_constructive_interference_size / sizeof(K))
                        + (std::hardware_constructive_interference_size % sizeof(K) > 0)
                        - 1
                    )
                )
            );

            explicit constexpr hashtable(
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS,
                allocator_type * const allocator = nullptr
            ) : alloc{allocator}, _capacity{0}, keys{nullptr}, values{nullptr}, alloc_flags{alloc_flags}
            {
                reserve(minimum_capacity);
            }

            constexpr hashtable(const hashtable &other)
                : alloc{other},
                _capacity{other._capacity},
                slot_map{other.slot_map},
                keys{other.keys},
                values{other.values}
            {}

            constexpr hashtable(hashtable &&other)
                : alloc{std::move(other)},
                _capacity{std::move(other._capacity)},
                slot_map{std::move(other.slot_map)},
                keys{std::move(other.keys)},
                values{std::move(other.values)}
            {
                other.keys = nullptr;
                other.values = nullptr;
            }

            template<typename InputRange>
            requires std::ranges::input_range<InputRange>
            constexpr hashtable(
                InputRange&& input,
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS,
                allocator_type * const allocator = nullptr
            ) : hashtable{alloc_flags, allocator} {
                std::for_each(
                    input.begin(),
                    input.end(),
                    [this] (const auto &pair) {
                        insert(pair.first, pair.second);
                    }
                );
            }

            ~hashtable() {
                destroy();
            }

            void destroy() noexcept {
                if constexpr(
                    !std::is_trivially_destructible_v<K>
                    || !std::is_trivially_destructible_v<V>
                ) {
                    const size_type end = slot_map.size_bits();

                    for(size_type i = 0; i < end; i+= 1) {
                        const bool is_set = slot_map[i];

                        if(is_set) {
                            if constexpr(!std::is_trivially_destructible_v<K>) {
                                std::destroy_at(&keys[i]);
                            }

                            if constexpr(!std::is_trivially_destructible_v<V>) {
                                std::destroy_at(&values[i]);
                            }
                        }
                    }
                }

                if(keys) {
                    alloc::get_allocator()->deallocate(keys);
                    alloc::get_allocator()->deallocate(values);
                }

                slot_map.destroy();

                _capacity = 0;
                keys = nullptr;
                values = nullptr;
            }

            constexpr size_type capacity() const noexcept {
                return _capacity;
            }

            constexpr hashtable& operator =(const hashtable &other) {
                destroy();
                alloc::operator =(other);
                reserve(other._capacity);

                if constexpr(
                    std::is_trivially_copyable_v<K>
                    && std::is_trivially_copyable_v<V>
                ) { // Both trivially copiable
                    ::memcpy(keys, other.keys, sizeof(K) * _capacity);
                    ::memcpy(values, other.values, sizeof(V) * _capacity);

                    slot_map = other.slot_map;
                } else {
                    for(const auto& pair : other) {
                        insert(*pair.first, *pair.second);
                    }
                }

                return *this;
            }

            constexpr hashtable& operator =(hashtable &&other) {
                destroy();

                alloc::operator =(std::move(other));
                slot_map = std::move(other.slot_map);
                keys = std::move(other.keys);
                values = std::move(other.values);
                _capacity = other._capacity;

                other.keys = nullptr;
                other.values = nullptr;
                other._capacity = 0;

                return *this;
            }

            constexpr void reserve(size_type new_capacity) {
                if(new_capacity <= _capacity) {
                    return;
                }

                bool done;
                new_capacity = increase_capacity(_capacity, new_capacity);
                bitset<allocator_type> new_slot_map;
                const auto finish = end();
                key_pointer new_keys;
                value_pointer new_values;

                do {
                    bool keep_iterating = true;
                    done = true;
                    new_keys = alloc::get_allocator()->template allocate<key_type>(new_capacity, alloc_flags);
                    new_values = alloc::get_allocator()->template allocate<value_type>(new_capacity, alloc_flags);

                    new_slot_map.resize(new_capacity);
                    new_slot_map.zero();

                    for(auto it = begin(); it < finish && keep_iterating; ++it) {
                        const size_type new_index = compute_key_index(*it->first, new_capacity);
                        const size_type last_chunk_index = wrap_index(new_index + chunk_size, new_capacity);
                        bool item_added = false;

                        for(size_type i = new_index; i != last_chunk_index; i = wrap_index(++i, new_capacity)) {
                            if(!new_slot_map[i]) {
                                std::construct_at(&new_keys[i], std::move(*it->first));
                                std::construct_at(&new_values[i], std::move(*it->second));

                                std::destroy_at(&keys[it.index]);
                                std::destroy_at(&values[it.index]);

                                new_slot_map.set(i);

                                item_added = true;
                                break;
                            }
                        }

                        // If the chunk is full because of too many close-by
                        // elements we have to start over and increase the capacity once more.
                        if(!item_added) {
                            alloc::get_allocator()->deallocate(new_keys);
                            alloc::get_allocator()->deallocate(new_values);
                            new_capacity <<= 1;
                            done = false;
                            break;
                        }
                    }
                } while(!done);

                if(keys) {
                    alloc::get_allocator()->deallocate(keys);
                    alloc::get_allocator()->deallocate(values);
                }

                _capacity = new_capacity;
                slot_map = std::move(new_slot_map);
                keys = new_keys;
                values = new_values;
            }

            constexpr void insert(const_key_reference key, const_value_reference value) {
                const size_type index = compute_key_index(key, _capacity);
                const size_type last_chunk_index = wrap_index(index + chunk_size, _capacity);
                size_type first_empty = invalid_size;

                // Check all items in a chunk. If we find the exact key,
                // nothing needs to be done. Item is already there. Otherwise
                // find the first available slot in the chunk and add the item.
                for(size_type i = index; i != last_chunk_index; i = wrap_index(++i, _capacity)) {
                    if(slot_map[i] && key == keys[i]) {
                        return;
                    }

                    if(!slot_map[i] && first_empty == invalid_size) {
                        first_empty = i;
                    }
                }

                if(first_empty != invalid_size) {
                    std::construct_at(&keys[first_empty], key);
                    std::construct_at(&values[first_empty], value);
                    slot_map[first_empty] = true;
                    return;
                }

                // No slots available in the chunk. Reserve and
                // rehash.
                rehash();
                insert(key, value);
            }

            template<typename ...Args>
            constexpr value_reference emplace(const_key_reference key, Args&& ...args) {
                const size_type index = compute_key_index(key, _capacity);
                const size_type last_chunk_index = wrap_index(index + chunk_size, _capacity);
                size_type first_empty = invalid_size;

                for(size_type i = index; i != last_chunk_index; i = wrap_index(++i, _capacity)) {
                    if(slot_map[i] && key == keys[i]) {
                        return values[i];
                    }

                    if(!slot_map[i] && first_empty == invalid_size) {
                        first_empty = i;
                    }
                }

                if(first_empty != invalid_size) {
                    std::construct_at(&keys[first_empty], key);
                    std::construct_at(&values[first_empty], std::forward<Args>(args)...);
                    slot_map[first_empty] = true;
                    return values[first_empty];
                }

                // No slots available
                rehash();
                return emplace(key, std::forward<Args>(args)...);
            }

            constexpr void erase(const_key_reference key) {
                const size_type index = compute_key_index(key, _capacity);
                const size_type last_chunk_index = wrap_index(index + chunk_size, _capacity);

                for(size_type i = index; i != last_chunk_index; i = wrap_index(++i, _capacity)) {
                    if(slot_map[i] && keys[i] == key) {
                        std::destroy_at(&keys[i]);
                        std::destroy_at(&values[i]);
                        slot_map[i] = false;
                        return;
                    }
                }
            }

            template<typename Iterator>
            constexpr void erase(const Iterator& it) {
                const size_type i = it.index;

                std::destroy_at(&keys[i]);
                std::destroy_at(&values[i]);
                slot_map[i] = false;
            }

            CCLNODISCARD constexpr auto& at(const_key_reference key) const {
                const size_type index = compute_key_index(key, _capacity);
                const size_type last_chunk_index = wrap_index(index + chunk_size, _capacity);

                for(size_type i = index; i != last_chunk_index; i = wrap_index(++i, _capacity)) {
                    if(slot_map[i] && keys[i] == key) {
                        return values[i];
                    }
                }

                CCL_THROW(std::out_of_range{"Key not present."});
            }

            constexpr value_reference operator [](const_key_reference key) {
                static_assert(std::is_default_constructible_v<V>);

                const size_type index = compute_key_index(key, _capacity);
                const size_type last_chunk_index = wrap_index(index + chunk_size, _capacity);

                for(size_type i = index; i != last_chunk_index; i = wrap_index(++i, _capacity)) {
                    if(slot_map[i] && keys[i] == key) {
                        return values[i];
                    }
                }

                return emplace(key);
            }

            constexpr void clear() {
                if constexpr(!std::is_trivially_destructible_v<K>) {
                    for(size_type i = 0; i < _capacity; ++i) {
                        if(slot_map[i]) {
                            std::destroy_at(&keys[i]);
                        }
                    }
                }

                if constexpr(!std::is_trivially_destructible_v<V>) {
                    for(size_type i = 0; i < _capacity; ++i) {
                        if(slot_map[i]) {
                            std::destroy_at(&values[i]);
                        }
                    }
                }

                slot_map.zero();
            }

            constexpr iterator find(const_key_reference key) {
                const size_type index = compute_key_index(key, _capacity);
                const size_type last_chunk_index = wrap_index(index + chunk_size, _capacity);

                for(size_type i = index; i != last_chunk_index; i = wrap_index(++i, _capacity)) {
                    if(slot_map[i] && keys[i] == key) {
                        return iterator { *this, i };
                    }
                }

                return end();
            }

            constexpr const_iterator find(const_key_reference key) const {
                const size_type index = compute_key_index(key, _capacity);
                const size_type last_chunk_index = wrap_index(index + chunk_size, _capacity);

                for(size_type i = index; i != last_chunk_index; i = wrap_index(++i, _capacity)) {
                    if(slot_map[i] && keys[i] == key) {
                        return const_iterator { *this, i };
                    }
                }

                return end();
            }

            constexpr bool contains(const_key_reference key) const {
                return find(key) != end();
            }

            constexpr iterator begin() { return iterator{ *this, 0 }; }
            constexpr iterator end() { return iterator{ *this, _capacity }; }

            constexpr const_iterator begin() const { return const_iterator{ *this, 0 }; }
            constexpr const_iterator end() const { return const_iterator{ *this, _capacity }; }

            constexpr const_iterator cbegin() const { return const_iterator{ *this, 0 }; }
            constexpr const_iterator cend() const { return const_iterator{ *this, _capacity }; }

            constexpr allocator_type* get_allocator() const noexcept { return alloc::get_allocator(); }
            constexpr allocation_flags get_allocation_flags() const noexcept { return alloc_flags; }
            constexpr size_type get_chunk_size() const noexcept { return chunk_size; }

        private:
            void rehash() {
                chunk_size <<= 1;
                reserve(max<size_type>(1, _capacity << 1));
            }

            static hash_type hash(const_key_reference x) {
                return hash_function_type{}(x);
            }

            static constexpr size_type wrap_index(const size_type index, const size_type capacity) {
                CCL_ASSERT(is_power_2(capacity));
                CCL_ASSERT(capacity);

                return index & (capacity - 1);
            }

            static constexpr size_type compute_key_index(const_key_reference x, const size_type capacity) {
                return wrap_index(hash(x), capacity);
            }

            size_type _capacity = 0;
            size_type chunk_size = default_chunk_size;
            bitset<allocator_type> slot_map; // Slot availability bit map
            key_pointer keys = nullptr;
            value_pointer values = nullptr;
            allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS;

            static constexpr size_type invalid_size = ~static_cast<size_type>(0);
    };
}

#endif // CCL_HASHTABLE_HPP
