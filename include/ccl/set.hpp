/**
 * @file
 *
 * Unordered set.
 */
#ifndef CCL_SET_HPP
#define CCL_SET_HPP

#include <algorithm>
#include <ccl/api.hpp>
#include <ccl/definitions.hpp>
#include <ccl/allocator.hpp>
#include <ccl/concepts.hpp>
#include <ccl/internal/optional-allocator.hpp>
#include <ccl/hash.hpp>
#include <ccl/bitset.hpp>

namespace ccl {
    template<typename Set>
    struct set_iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;

        using key_type = typename Set::key_type;
        using const_key_reference = typename Set::const_key_reference;
        using const_key_pointer = const key_type*;

        using set_type = Set;
        using size_type = typename Set::size_type;

        constexpr set_iterator() noexcept : set{nullptr}, index{0} {}

        explicit constexpr set_iterator(set_type& set, const size_type item_index) noexcept : set{&set}, index{item_index} {
            // Ensure we are actually pointing at an existing value or at the end.
            // Useful for `begin()` iterators.
            for(; index < set._capacity; ++index) {
                if(set.availability_map[index]) {
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

        constexpr const_key_reference operator*() const noexcept { return set->keys[index]; }

        constexpr const_key_pointer operator->() const noexcept {
            return &set->keys[index];
        }

        constexpr auto& operator --() const noexcept {
            do {
                index--;
            } while(!set->availability_map[index]);

            return *this;
        }

        constexpr auto operator --(int) const noexcept {
            const size_type old_index = this->index;

            do {
                if(index == 0) {
                    break;
                }

                index--;
            } while(!set->availability_map[index]);

            return set_iterator{*set, old_index};
        }

        constexpr auto& operator ++() const noexcept {
            for(index += 1; index < set->_capacity && !set->availability_map[index]; ++index);

            return *this;
        }

        constexpr auto operator ++(int) const noexcept {
            const size_type old_index = this->index;

            for(; index < set->_capacity && !set->availability_map[index]; ++index);

            return set_iterator{*set, old_index};
        }

        set_type *set;
        mutable size_type index;
    };

    template<typename Set>
    constexpr bool operator ==(const set_iterator<Set> a, const set_iterator<Set> b) noexcept {
        return a.set == b.set && a.index == b.index;
    }

    template<typename Set>
    constexpr bool operator !=(const set_iterator<Set> a, const set_iterator<Set> b) noexcept {
        return a.set != b.set || a.index != b.index;
    }

    template<typename Set>
    constexpr bool operator >(const set_iterator<Set> a, const set_iterator<Set> b) noexcept {
        CCL_THROW_IF(a.set != b.set, std::runtime_error{"Comparing iterators from different sets."});

        return a.index > b.index;
    }

    template<typename Set>
    constexpr bool operator <(const set_iterator<Set> a, const set_iterator<Set> b) noexcept {
        CCL_THROW_IF(a.set != b.set, std::runtime_error{"Comparing iterators from different sets."});

        return a.index < b.index;
    }

    template<typename Set>
    constexpr bool operator >=(const set_iterator<Set> a, const set_iterator<Set> b) noexcept {
        CCL_THROW_IF(a.set != b.set, std::runtime_error{"Comparing iterators from different sets."});

        return a.index >= b.index;
    }

    template<typename Set>
    constexpr bool operator <=(const set_iterator<Set> a, const set_iterator<Set> b) noexcept {
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
    requires typed_allocator<Allocator, K>
    class set : private internal::with_optional_allocator<Allocator> {
        friend struct set_iterator<set>;
        friend struct set_iterator<const set>;

        using alloc = internal::with_optional_allocator<Allocator>;

        public:
            using size_type = size_t;

            using key_type = K;
            using key_pointer = K*;
            using key_reference = K&;

            using const_key_reference = const K&;
            using hash_type = typename HashFunction::hash_value_type;
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
                availability_map{other.availability_map},
                keys{other.keys}
            {}

            constexpr set(set &&other)
                : alloc{std::move(other)},
                _capacity{std::move(other._capacity)},
                availability_map{std::move(other.availability_map)},
                keys{std::move(other.keys)}
            {}

            template<typename InputRange>
            requires std::ranges::input_range<InputRange>
            constexpr set(
                InputRange&& input,
                allocator_type * const allocator = nullptr
            ) : set{allocator} {
                std::for_each(
                    input.begin(),
                    input.end(),
                    [this] (const auto &key) {
                        insert(key);
                    }
                );
            }

            ~set() {
                destroy();
            }

            void destroy() noexcept {
                availability_map.destroy();
                alloc::get_allocator()->deallocate(keys);

                _capacity = 0;
                keys = nullptr;
            }

            constexpr size_type capacity() const noexcept {
                return _capacity;
            }

            constexpr set& operator =(const set &other) {
                alloc::operator =(other);
                availability_map = other.availability_map;
                keys = other.keys;

                return *this;
            }

            constexpr set& operator =(set &&other) {
                alloc::operator =(std::move(other));
                availability_map = std::move(other.availability_map);
                keys = std::move(other.keys);

                return *this;
            }

            constexpr void reserve(const size_type new_capacity) {
                if(new_capacity <= _capacity) {
                    return;
                }

                const size_type actual_new_capacity = increase_capacity(_capacity, new_capacity);

                bitset<allocator_type> new_availability_map;
                const key_pointer new_keys = alloc::get_allocator()->template allocate<key_type>(actual_new_capacity);

                new_availability_map.resize(actual_new_capacity);
                new_availability_map.zero();

                for(size_type i = 0; i < _capacity; ++i) {
                    const bool is_available = availability_map[i];

                    if(is_available) {
                        key_reference current_key = keys[i];

                        const size_type new_index = compute_key_index(keys[i], actual_new_capacity);

                        std::construct_at(&new_keys[new_index], std::move(current_key));
                        std::destroy_at(&current_key);

                        new_availability_map.set(new_index);
                    }
                }

                alloc::get_allocator()->deallocate(keys);

                _capacity = actual_new_capacity;
                availability_map = std::move(new_availability_map);
                keys = new_keys;
            }

            constexpr void insert(const_key_reference key) {
                const size_type index = compute_key_index(key, _capacity);

                for(size_type i = index; i < _capacity; ++i) {
                    if(!availability_map[i] || key == keys[i]) {
                        std::construct_at(&keys[i], key);
                        availability_map[i] = true;
                        return;
                    }
                }

                for(size_type i = 0; i < index; ++i) {
                    if(!availability_map[i] || key == keys[i]) {
                        std::construct_at(&keys[i], key);
                        availability_map[i] = true;
                        return;
                    }
                }

                // No slots available
                reserve(max<size_type>(1, _capacity << 1));
                insert(key);
            }

            constexpr void erase(const_key_reference key) {
                const size_type index = compute_key_index(key, _capacity);

                for(size_type i = index; i < _capacity; ++i) {
                    if(keys[i] == key) {
                        CCL_ASSERT(availability_map[i]);

                        std::destroy_at(&keys[i]);
                        availability_map[i] = false;
                        return;
                    }
                }

                for(size_type i = 0; i < index; ++i) {
                    if(keys[i] == key) {
                        CCL_ASSERT(availability_map[i]);

                        std::destroy_at(&keys[i]);
                        availability_map[i] = false;
                        return;
                    }
                }
            }

            constexpr void clear() {
                for(size_type i = 0; i < _capacity; ++i) {
                    if constexpr(std::is_destructible_v<K>) {
                        if(availability_map[i]) {
                            std::destroy_at(&keys[i]);
                        }
                    }
                }

                std::fill(
                    availability_map.get_clusters().begin(),
                    availability_map.get_clusters().end(),
                    static_cast<typename decltype(availability_map)::cluster_type>(0)
                );
            }

            constexpr iterator find(const_key_reference key) {
                const size_type index = compute_key_index(key, _capacity);

                for(size_type i = index; i < _capacity; ++i) {
                    if(availability_map[i] && keys[i] == key) {
                        return iterator { *this, i };
                    }
                }

                for(size_type i = 0; i < index; ++i) {
                    if(availability_map[i] && keys[i] == key) {
                        return iterator { *this, i };
                    }
                }

                return end();
            }

            constexpr bool contains(const_key_reference key) const {
                const size_type index = compute_key_index(key, _capacity);

                for(size_type i = index; i < _capacity; ++i) {
                    if(availability_map[i] && keys[i] == key) {
                        return true;
                    }
                }

                for(size_type i = 0; i < index; ++i) {
                    if(availability_map[i] && keys[i] == key) {
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

            static size_type compute_key_index(const_key_reference x, const size_type capacity) {
                CCL_ASSERT(is_power_2(capacity));
                CCL_ASSERT(capacity);

                return hash(x) & (capacity - 1);
            }

            size_type _capacity = 0;
            bitset<allocator_type> availability_map; // Slot availability bit map
            key_pointer keys = nullptr;
    };
}

#endif // CCL_SET_HPP
