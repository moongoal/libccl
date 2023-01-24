/**
 * @file
 *
 * Unordered set.
 */
#ifndef CCL_SET_HPP
#define CCL_SET_HPP

#include <initializer_list>
#include <algorithm>
#include <ccl/api.hpp>
#include <ccl/definitions.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/concepts.hpp>
#include <ccl/internal/optional-allocator.hpp>
#include <ccl/hash.hpp>
#include <ccl/bitset.hpp>
#include <ccl/util.hpp>

namespace ccl {
    template<typename Set>
    struct set_iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using iterator_concept = iterator_category;
        using difference_type = std::ptrdiff_t;

        using value_type = typename Set::key_type;
        using reference = typename Set::key_reference;
        using pointer = typename Set::key_pointer;
        using const_reference = typename Set::const_key_reference;
        using const_pointer = const value_type*;

        using set_type = Set;
        using size_type = typename Set::size_type;

        constexpr set_iterator() noexcept : set{nullptr}, index{0} {}

        explicit constexpr set_iterator(set_type& set, const size_type item_index) noexcept : set{&set}, index{item_index} {
            // Ensure we are actually pointing at an existing value or at the end.
            // Useful for `begin()` iterators.
            for(; index < set._capacity; ++index) {
                if(set.slot_map[index]) {
                    return;
                }
            }
        }

        constexpr set_iterator(const set_iterator &other) noexcept : set{other.set}, index{other.index} {}
        constexpr set_iterator(set_iterator &&other) noexcept : set{std::move(other.set)}, index{std::move(other.index)} {}

        constexpr set_iterator& operator =(const set_iterator &other) noexcept {
            set = other.set;
            index = other.index;

            return *this;
        }

        constexpr const_reference operator*() const noexcept { return set->keys[index]; }

        constexpr const_pointer operator->() const noexcept {
            return &set->keys[index];
        }

        constexpr auto& operator --() noexcept {
            do {
                index--;
            } while(!set->slot_map[index]);

            return *this;
        }

        constexpr auto operator --(int) noexcept {
            const size_type old_index = this->index;

            do {
                if(index == 0) {
                    break;
                }

                index--;
            } while(!set->slot_map[index]);

            return set_iterator{*set, old_index};
        }

        constexpr auto& operator ++() noexcept {
            for(index += 1; index < set->_capacity && !set->slot_map[index]; ++index);

            return *this;
        }

        constexpr auto operator ++(int) noexcept {
            const size_type old_index = this->index;

            for(; index < set->_capacity && !set->slot_map[index]; ++index);

            return set_iterator{*set, old_index};
        }

        set_type *set;
        mutable size_type index;
    };

    template<typename Set>
    constexpr bool operator ==(const set_iterator<Set> a, const set_iterator<Set> b) {
        return a.set == b.set && a.index == b.index;
    }

    template<typename Set>
    constexpr bool operator !=(const set_iterator<Set> a, const set_iterator<Set> b) {
        return a.set != b.set || a.index != b.index;
    }

    template<typename Set>
    constexpr bool operator >(const set_iterator<Set> a, const set_iterator<Set> b) {
        CCL_THROW_IF(a.set != b.set, std::runtime_error{"Comparing iterators from different sets."});

        return a.index > b.index;
    }

    template<typename Set>
    constexpr bool operator <(const set_iterator<Set> a, const set_iterator<Set> b) {
        CCL_THROW_IF(a.set != b.set, std::runtime_error{"Comparing iterators from different sets."});

        return a.index < b.index;
    }

    template<typename Set>
    constexpr bool operator >=(const set_iterator<Set> a, const set_iterator<Set> b) {
        CCL_THROW_IF(a.set != b.set, std::runtime_error{"Comparing iterators from different sets."});

        return a.index >= b.index;
    }

    template<typename Set>
    constexpr bool operator <=(const set_iterator<Set> a, const set_iterator<Set> b) {
        CCL_THROW_IF(a.set != b.set, std::runtime_error{"Comparing iterators from different sets."});

        return a.index <= b.index;
    }

    /**
     * An unordered set.
     *
     * @tparam K Key type.
     * @tparam HashFunction The function used to compute the key hashes.
     * @tparam Allocator The allocator type.
     */
    template<
        typename K,
        typename HashFunction = hash<K>,
        typename Allocator = allocator
    >
    requires typed_allocator<Allocator, K> && std::equality_comparable<K>
    class set : private internal::with_optional_allocator<Allocator> {
        friend struct set_iterator<set>;
        friend struct set_iterator<const set>;

        using alloc = internal::with_optional_allocator<Allocator>;

        public:
            using size_type = std::size_t;

            using key_type = K;
            using key_pointer = K*;
            using key_reference = K&;

            using const_key_reference = const K&;
            using hash_type = hash_t;
            using hash_function_type = HashFunction;

            using allocator_type = Allocator;

            using iterator = set_iterator<set>;
            using const_iterator = set_iterator<const set>;

            static constexpr size_type minimum_capacity = CCL_SET_MINIMUM_CAPACITY;

            explicit constexpr set(
                allocator_type * const allocator = nullptr
            ) : alloc{allocator}, _capacity{0}, keys{nullptr}
            {
                reserve(minimum_capacity);
            }

            constexpr set(const set &other)
                : alloc{other},
                _capacity{other._capacity},
                slot_map{other.slot_map},
                keys{other.keys}
            {}

            constexpr set(set &&other)
                : alloc{std::move(other)},
                _capacity{std::move(other._capacity)},
                slot_map{std::move(other.slot_map)},
                keys{std::move(other.keys)}
            {}

            template<typename InputRange>
            requires std::ranges::input_range<InputRange>
            constexpr set(
                InputRange&& input,
                allocator_type * const allocator = nullptr
            ) : set{allocator} {
                insert(input);
            }

            ~set() {
                destroy();
            }

            void destroy() noexcept {
                slot_map.destroy();

                if(keys) {
                    alloc::get_allocator()->deallocate(keys);
                }

                _capacity = 0;
                keys = nullptr;
            }

            constexpr size_type capacity() const noexcept {
                return _capacity;
            }

            constexpr set& operator =(const set &other) {
                alloc::operator =(other);
                slot_map = other.slot_map;
                keys = other.keys;

                return *this;
            }

            constexpr set& operator =(set &&other) {
                destroy();

                alloc::operator =(std::move(other));
                slot_map = std::move(other.slot_map);
                keys = std::move(other.keys);

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

                do {
                    bool keep_iterating = true;
                    done = true;
                    new_keys = alloc::get_allocator()->template allocate<key_type>(new_capacity);

                    new_slot_map.resize(new_capacity);
                    new_slot_map.zero();

                    for(auto it = begin(); it < finish && keep_iterating; ++it) {
                        const size_type new_index = compute_key_index(*it, new_capacity);
                        const size_type last_chunk_index = wrap_index(new_index + CCL_SET_KEY_CHUNK_SIZE, new_capacity);
                        bool item_added = false;

                        for(size_type i = new_index; i != last_chunk_index; i = wrap_index(++i, new_capacity)) {
                            if(!new_slot_map[i]) {
                                std::construct_at(&new_keys[i], std::move(*it));
                                std::destroy_at(std::to_address(it));
                                new_slot_map.set(i);

                                item_added = true;
                                break;
                            }
                        }

                        // If the chunk is full because of too many close-by
                        // elements we have to start over and increase the capacity once more.
                        if(!item_added) {
                            alloc::get_allocator()->deallocate(new_keys);
                            new_capacity <<= 1;
                            done = false;
                            break;
                        }
                    }
                } while(!done);

                if(keys) {
                    alloc::get_allocator()->deallocate(keys);
                }

                _capacity = new_capacity;
                slot_map = std::move(new_slot_map);
                keys = new_keys;
            }

            constexpr void insert(const_key_reference key) {
                const size_type index = compute_key_index(key, _capacity);
                const size_type last_chunk_index = wrap_index(index + CCL_SET_KEY_CHUNK_SIZE, _capacity);
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
                    slot_map[first_empty] = true;
                    return;
                }

                // No slots available in the chunk. Reserve and
                // rehash.
                reserve(max<size_type>(1, _capacity << 1));
                insert(key);
            }

            constexpr void insert(key_type&& key) {
                const size_type index = compute_key_index(key, _capacity);
                const size_type last_chunk_index = wrap_index(index + CCL_SET_KEY_CHUNK_SIZE, _capacity);
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
                    std::construct_at(&keys[first_empty], std::move(key));
                    slot_map[first_empty] = true;
                    return;
                }

                // No slots available in the chunk. Reserve and
                // rehash.
                reserve(max<size_type>(1, _capacity << 1));
                insert(std::move(key));
            }

            template<typename Iterator>
            constexpr void insert(Iterator&& start, Iterator&& finish) {
                for(auto it = start; it != finish; ++it) {
                    insert(*it);
                }
            }

            template<std::ranges::range InputRange>
            constexpr void insert(InputRange&& input) {
                for(auto it = input.begin(); it != input.end(); ++it) {
                    insert(*it);
                }
            }

            constexpr void insert(std::initializer_list<key_type> input) {
                for(auto it = input.begin(); it != input.end(); ++it) {
                    insert(*it);
                }
            }

            constexpr void erase(const_key_reference key) {
                const size_type index = compute_key_index(key, _capacity);
                const size_type last_chunk_index = wrap_index(index + CCL_SET_KEY_CHUNK_SIZE, _capacity);

                for(size_type i = index; i != last_chunk_index; i = wrap_index(++i, _capacity)) {
                    if(slot_map[i] && keys[i] == key) {
                        std::destroy_at(&keys[i]);
                        slot_map[i] = false;
                        return;
                    }
                }
            }

            constexpr void clear() {
                if constexpr(!std::is_trivially_destructible_v<K>) {
                    for(size_type i = 0; i < _capacity; ++i) {
                        if(slot_map[i]) {
                            std::destroy_at(&keys[i]);
                        }
                    }
                }

                slot_map.zero();
            }

            constexpr iterator find(const_key_reference key) {
                const size_type index = compute_key_index(key, _capacity);
                const size_type last_chunk_index = wrap_index(index + CCL_SET_KEY_CHUNK_SIZE, _capacity);

                for(size_type i = index; i != last_chunk_index; i = wrap_index(++i, _capacity)) {
                    if(slot_map[i] && keys[i] == key) {
                        return iterator { *this, i };
                    }
                }

                return end();
            }

            constexpr bool contains(const_key_reference key) const {
                const size_type index = compute_key_index(key, _capacity);
                const size_type last_chunk_index = wrap_index(index + CCL_SET_KEY_CHUNK_SIZE, _capacity);

                for(size_type i = index; i != last_chunk_index; i = wrap_index(++i, _capacity)) {
                    if(slot_map[i] && keys[i] == key) {
                        return true;
                    }
                }

                return false;
            }

            constexpr iterator begin() { return iterator{ *this, 0 }; }
            constexpr iterator end() { return iterator{ *this, _capacity }; }

            constexpr const_iterator begin() const { return const_iterator{ *this, 0 }; }
            constexpr const_iterator end() const { return const_iterator{ *this, _capacity }; }

            constexpr const_iterator cbegin() const { return const_iterator{ *this, 0 }; }
            constexpr const_iterator cend() const { return const_iterator{ *this, _capacity }; }

        private:
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
            bitset<allocator_type> slot_map; // Slot availability bit map. true is filled, false is empty
            key_pointer keys = nullptr;

            static constexpr size_type invalid_size = ~static_cast<size_type>(0);
    };
}

#endif // CCL_SET_HPP
