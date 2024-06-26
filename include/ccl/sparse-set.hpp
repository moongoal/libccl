/**
 * @file
 *
 * Sparse set.
 */
#ifndef CCL_SPARSE_SET_HPP
#define CCL_SPARSE_SET_HPP

#include <utility>
#include <ccl/api.hpp>
#include <ccl/hashtable.hpp>
#include <ccl/vector.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/concepts.hpp>
#include <ccl/hash.hpp>
#include <ccl/internal/optional-allocator.hpp>

namespace ccl {
    template<
        typename T,
        typed_hash_function<T> Hash = hash<T>,
        typed_allocator<T> Allocator = allocator
    > class sparse_set : internal::with_optional_allocator<Allocator> {
        public:
            using value_type = T;
            using reference_type = T&;
            using pointer = T*;
            using const_pointer = const T*;
            using const_reference_type = const T&;
            using allocator_type = Allocator;
            using hash_function_type = Hash;
            using hash_type = hash_t;
            using size_type = uint32_t;

        private:
            using alloc = internal::with_optional_allocator<Allocator>;
            using data_vector_type = vector<T, allocator_type>;
            using index_map_type = hashtable<T, size_type, hash_function_type, allocator_type>;

            data_vector_type _data;
            index_map_type index_map;

            static hash_type hash(const_reference_type x) {
                return hash_function_type{}(x);
            }

            static constexpr size_type wrap_index(const size_type index, const size_type capacity) {
                CCL_ASSERT(is_power_2(capacity));
                CCL_ASSERT(capacity);

                return index & (capacity - 1);
            }

            static constexpr size_type compute_key_index(const_reference_type x, const size_type capacity) {
                return wrap_index(hash(x), capacity);
            }

        public:
            constexpr sparse_set(
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS,
                allocator_type * const allocator = nullptr
            ) : alloc{allocator}, _data{alloc_flags, allocator}, index_map{alloc_flags, allocator}
            {}

            constexpr sparse_set(const sparse_set &other)
                : alloc{other.get_allocator()},
                _data{other._data},
                index_map{other.index_map}
            {}

            constexpr sparse_set(sparse_set &&other)
                : alloc{other.get_allocator()},
                _data{std::move(other._data)},
                index_map{std::move(other.index_map)}
            {}

            constexpr void insert(const_reference_type item) {
                if(!index_map.contains(item)) {
                    const size_type item_index = static_cast<size_type>(_data.size());

                    _data.push_back(item);
                    index_map.insert(item, item_index);
                }
            }

            constexpr void remove(const_reference_type item) {
                const auto it = index_map.find(item);

                if(it != index_map.end()) {
                    const size_type index = static_cast<size_type>(*it->second);

                    _data.erase(_data.begin() + index);
                    index_map.erase(item);
                }
            }

            constexpr bool contains(const_reference_type item) const {
                return index_map.contains(item);
            }

            constexpr decltype(auto) begin() { return _data.begin(); }
            constexpr decltype(auto) begin() const { return _data.begin(); }
            constexpr decltype(auto) end() { return _data.end(); }
            constexpr decltype(auto) end() const { return _data.end(); }

            constexpr decltype(auto) cbegin() const { return _data.cbegin(); }
            constexpr decltype(auto) cend() const { return _data.cend(); }

            constexpr size_type size() const noexcept {
                return static_cast<size_type>(_data.size());
            }

            constexpr pointer data() noexcept {
                return _data.data();
            }

            constexpr const_pointer data() const noexcept {
                return _data.data();
            }

            constexpr allocator_type* get_allocator() const noexcept { return _data.get_allocator(); }
            constexpr allocation_flags get_allocation_flags() const noexcept { return _data.get_allocation_flags(); }
    };
}

#endif // CCL_SPARSE_SET_HPP
