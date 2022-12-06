/**
 * @file
 *
 * Sparse set.
 */
#ifndef CCL_DENSE_MAP_HPP
#define CCL_DENSE_MAP_HPP

#include <utility>
#include <ccl/api.hpp>
#include <ccl/hashtable.hpp>
#include <ccl/vector.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/concepts.hpp>
#include <ccl/hash.hpp>
#include <ccl/internal/optional-allocator.hpp>
#include <ccl/compressed-pair.hpp>

namespace ccl {
template<typename Map>
    struct dense_map_iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;

        using key_type = typename Map::key_type;
        using value_type = typename Map::value_type;

        using key_value_pair = compressed_pair<key_type*, value_type*>;
        using const_key_value_pair = compressed_pair<const key_type*, const value_type*>;

        using map_type = Map;
        using size_type = typename Map::size_type;

        using index_map_iterator = hashtable_iterator<typename map_type::index_map_type>;

        constexpr dense_map_iterator() = default;

        constexpr dense_map_iterator(map_type& dense_map, index_map_iterator position) noexcept
            : map{&dense_map},
            index_iterator{position}
        {}

        constexpr dense_map_iterator(const dense_map_iterator &other) noexcept
            : map{other.map},
            index_iterator{*other.index_iterator.hashtable, other.index_iterator.index}
        {}
        constexpr dense_map_iterator(dense_map_iterator &&other) noexcept
            : map{std::move(other.map)},
            index_iterator{std::move(other.index_iterator)}
        {}

        constexpr dense_map_iterator& operator =(const dense_map_iterator &other) noexcept {
            map = other. dense_map;
            index_iterator = other.index_iterator;

            return *this;
        }

        constexpr key_value_pair operator*() noexcept {
            const auto& key_index_pair = (*index_iterator);
            const uint32_t index = *key_index_pair.second();

            return key_value_pair{ key_index_pair.first(), &map->data[index] };
        }

        constexpr const_key_value_pair operator*() const noexcept {
            const auto& key_index_pair = (*index_iterator);
            const uint32_t index = key_index_pair.second();

            return key_value_pair{ key_index_pair.first(), map->data[index] };
        }

        constexpr key_value_pair operator->() const noexcept {
            const auto& key_index_pair = (*index_iterator);
            const uint32_t index = key_index_pair.second();

            return key_value_pair{ key_index_pair.first(), map->data[index] };
        }

        constexpr auto& operator --() const noexcept {
            --index_iterator;

            return *this;
        }

        constexpr auto operator --(int) const noexcept {
            return dense_map_iterator{*map, index_iterator--};
        }

        constexpr auto& operator ++() const noexcept {
            ++index_iterator;

            return *this;
        }

        constexpr auto operator ++(int) const noexcept {
            return dense_map_iterator{*map, index_iterator++};
        }

        map_type *map;
        mutable index_map_iterator index_iterator;
    };

    template<typename Map>
    constexpr bool operator ==(const dense_map_iterator<Map> a, const dense_map_iterator<Map> b) {
        return a.map == b.map && a.index_iterator == b.index_iterator;
    }

    template<typename Map>
    constexpr bool operator !=(const dense_map_iterator<Map> a, const dense_map_iterator<Map> b) {
        return a.map != b.map || a.index_iterator != b.index_iterator;
    }

    template<typename Map>
    constexpr bool operator >(const dense_map_iterator<Map> a, const dense_map_iterator<Map> b) {
        CCL_THROW_IF(a.map != b.map, std::runtime_error{"Comparing iterators from different dense maps."});

        return a.index_iterator > b.index_iterator;
    }

    template<typename Map>
    constexpr bool operator <(const dense_map_iterator<Map> a, const dense_map_iterator<Map> b) {
        CCL_THROW_IF(a.map != b.map, std::runtime_error{"Comparing iterators from different dense maps."});

        return a.index_iterator < b.index_iterator;
    }

    template<typename Map>
    constexpr bool operator >=(const dense_map_iterator<Map> a, const dense_map_iterator<Map> b) {
        CCL_THROW_IF(a.map != b.map, std::runtime_error{"Comparing iterators from different dense maps."});

        return a.index_iterator >= b.index_iterator;
    }

    template<typename Map>
    constexpr bool operator <=(const dense_map_iterator<Map> a, const dense_map_iterator<Map> b) {
        CCL_THROW_IF(a.map != b.map, std::runtime_error{"Comparing iterators from different dense maps."});

        return a.index_iterator <= b.index_iterator;
    }

    template<typename K, typename V, typed_allocator<K> Allocator = allocator, typed_hash_function<K> Hash = hash<K>>
    class dense_map : internal::with_optional_allocator<Allocator> {
        friend struct dense_map_iterator<dense_map>;
        friend struct dense_map_iterator<const dense_map>;

        public:
            using key_type = K;
            using value_type = V;
            using const_key_reference = const K&;
            using const_value_reference = const V&;
            using allocator_type = Allocator;
            using hash_function_type = Hash;
            using hash_type = hash_t;
            using size_type = uint32_t;
            using data_vector_type = vector<V, allocator_type>;
            using index_map_type = hashtable<K, size_type, hash_function_type, allocator_type>;
            using value_iterator = typename data_vector_type::iterator;
            using const_value_iterator = typename data_vector_type::const_iterator;
            using iterator = dense_map_iterator<dense_map>;
            using const_iterator = dense_map_iterator<const dense_map>;

        private:
            data_vector_type data;
            index_map_type index_map;

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

        public:
            constexpr dense_map(
                allocator_type * const allocator = nullptr
            ) : internal::with_optional_allocator<Allocator>(
                allocator ? allocator : get_default_allocator()
            ) {}

            constexpr dense_map(const dense_map &other)
                : internal::with_optional_allocator<Allocator>(other.get_allocator()),
                data{other.data},
                index_map{other.index_map}
            {}

            constexpr dense_map(dense_map &&other)
                : internal::with_optional_allocator<Allocator>(other.get_allocator()),
                data{std::move(other.data)},
                index_map{std::move(other.index_map)}
            {}

            constexpr void insert(const_key_reference key, const_value_reference value) {
                auto it = index_map.find(key);

                if(it == index_map.end()) {
                    const size_type item_index = static_cast<size_type>(data.size());

                    data.push_back(value);
                    index_map.insert(key, item_index);
                } else {
                    data[*it->second()] = value;
                }
            }

            constexpr void remove(const_key_reference key) {
                const auto it = index_map.find(key);

                if(it != index_map.end()) {
                    const size_type index = static_cast<size_type>(*it->second());

                    data.erase(data.begin() + index);
                    index_map.erase(key);
                }
            }

            constexpr bool contains(const_key_reference item) const {
                return index_map.contains(item);
            }

            constexpr decltype(auto) begin_values() { return data.begin(); }
            constexpr decltype(auto) begin_values() const { return data.begin(); }
            constexpr decltype(auto) end_values() { return data.end(); }
            constexpr decltype(auto) end_values() const { return data.end(); }

            constexpr decltype(auto) cbegin_values() const { return data.cbegin(); }
            constexpr decltype(auto) cend_values() const { return data.cend(); }

            constexpr decltype(auto) begin() { return dense_map_iterator{*this, index_map.begin()}; }
            constexpr decltype(auto) begin() const { return dense_map_iterator{*this, index_map.begin()}; }
            constexpr decltype(auto) end() { return dense_map_iterator{*this, index_map.end()}; }
            constexpr decltype(auto) end() const { return dense_map_iterator{*this, index_map.end()}; }

            constexpr decltype(auto) cbegin() const { return dense_map_iterator{*this, index_map.begin()}; }
            constexpr decltype(auto) cend() const { return dense_map_iterator{*this, index_map.end()}; }

            constexpr size_type size() const noexcept {
                return static_cast<size_type>(data.size());
            }
    };
}

#endif // CCL_DENSE_MAP_HPP