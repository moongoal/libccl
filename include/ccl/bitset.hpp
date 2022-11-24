/**
 * @file
 *
 * Vector of bits.
 */
#ifndef CCL_BITSET_HPP
#define CCL_BITSET_HPP

#include <ccl/api.hpp>
#include <ccl/debug.hpp>
#include <ccl/vector.hpp>

namespace ccl {
    template<typed_allocator<uint64_t> Allocator>
    struct bitset_iterator;

    template<typed_allocator<uint64_t> Allocator = allocator>
    class bitset {
        public:
            using cluster_type = uint64_t;
            using size_type = size_t;
            using allocator_type = Allocator;
            using iterator = bitset_iterator<allocator_type>;
            using const_iterator = const bitset_iterator<allocator_type>;

            explicit constexpr bitset(allocator_type * const allocator = nullptr) : clusters{allocator}, _size_bits{0} {}
            constexpr bitset(const bitset &other) : clusters{other.clusters}, _size_bits{other._size_bits} {}
            constexpr bitset(bitset &&other) : clusters{std::move(other.clusters)}, _size_bits{std::move(other._size_bits)} {}

            ~bitset() {
                destroy();
            }

            void clear() {
                clusters.clear();
                _size_bits = 0;
            }

            void destroy() noexcept {
                clusters.destroy();
                _size_bits = 0;
            }

            constexpr size_type size_bits() const noexcept {
                return _size_bits;
            }

            constexpr size_type size() const noexcept {
                return clusters.size();
            }

            constexpr size_type capacity() const noexcept {
                return clusters.capacity();
            }

            constexpr void push_back(const bool value) {
                const auto [target_cluster_index, internal_bit_index] = locate_bit(_size_bits);
                const size_type required_cluster_size = target_cluster_index + 1;

                clusters.resize(required_cluster_size);
                cluster_type &target_cluster = clusters[target_cluster_index];

                if(value) {
                    target_cluster |= static_cast<cluster_type>(1) << internal_bit_index;
                } else {
                    target_cluster &= ~(static_cast<cluster_type>(1) << internal_bit_index);
                }

                _size_bits++;
            }

            constexpr void push_back_true() {
                const auto [target_cluster_index, internal_bit_index] = locate_bit(_size_bits);
                const size_type required_cluster_size = target_cluster_index + 1;

                clusters.resize(required_cluster_size);
                cluster_type &target_cluster = clusters[target_cluster_index];
                target_cluster |= static_cast<cluster_type>(1) << internal_bit_index;

                _size_bits++;
            }

            constexpr void push_back_false() {
                const auto [target_cluster_index, internal_bit_index] = locate_bit(_size_bits);
                const size_type required_cluster_size = target_cluster_index + 1;

                clusters.resize(required_cluster_size);
                cluster_type &target_cluster = clusters[target_cluster_index];
                target_cluster &= ~(static_cast<cluster_type>(1) << internal_bit_index);

                _size_bits++;
            }

            constexpr bool operator[](const size_type index) const {
                CCL_THROW_IF(index >= _size_bits, std::out_of_range{"Index out of range."});

                const auto [target_cluster_index, internal_bit_index] = locate_bit(index);
                const cluster_type &target_cluster = clusters[target_cluster_index];

                return target_cluster & static_cast<cluster_type>(1) << internal_bit_index;
            }

            constexpr void set(const size_type index) {
                CCL_THROW_IF(index >= _size_bits, std::out_of_range{"Index out of range."});

                const auto [target_cluster_index, internal_bit_index] = locate_bit(index);
                cluster_type &target_cluster = clusters[target_cluster_index];
                target_cluster |= static_cast<cluster_type>(1) << internal_bit_index;
            }

            constexpr void clear(const size_type index) {
                CCL_THROW_IF(index >= _size_bits, std::out_of_range{"Index out of range."});

                const auto [target_cluster_index, internal_bit_index] = locate_bit(index);
                cluster_type &target_cluster = clusters[target_cluster_index];
                target_cluster &= ~(static_cast<cluster_type>(1) << internal_bit_index);
            }

            constexpr void assign(const size_type index, const bool value) {
                CCL_THROW_IF(index >= _size_bits, std::out_of_range{"Index out of range."});

                const auto [target_cluster_index, internal_bit_index] = locate_bit(index);
                cluster_type &target_cluster = clusters[target_cluster_index];
                target_cluster &= ~(static_cast<cluster_type>(1) << internal_bit_index);

                if(value) {
                    target_cluster |= static_cast<cluster_type>(1) << internal_bit_index;
                } else {
                    target_cluster &= ~(static_cast<cluster_type>(1) << internal_bit_index);
                }
            }

            private:
                vector<cluster_type, allocator_type> clusters;
                size_type _size_bits;

                struct bit_location {
                    size_type target_cluster_index;
                    size_type internal_bit_index;
                };

                static constexpr bit_location locate_bit(const size_type index) {
                    return {
                        index / sizeof(cluster_type),
                        index % sizeof(cluster_type)
                    };
                }
    };
}

#endif // CCL_BITSET_HPP
