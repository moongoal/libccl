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

    /**
     * A variable sequence of bits.
     */
    template<typed_allocator<uint64_t> Allocator = allocator>
    class bitset {
        public:
            using cluster_type = uint64_t;
            using size_type = size_t;
            using allocator_type = Allocator;
            using iterator = bitset_iterator<allocator_type>;
            using const_iterator = const bitset_iterator<allocator_type>;

            class bit_proxy {
                bitset *set;
                size_type index;

                public:
                    bit_proxy() = delete;
                    bit_proxy(bitset &set, const size_type index) : set{&set}, index{index} {}
                    bit_proxy(const bit_proxy &) = default;
                    bit_proxy(bit_proxy &&) = default;

                    constexpr bit_proxy& operator =(const bool value) {
                        set->assign(index, value);

                        return *this;
                    }

                    constexpr operator bool() const noexcept {
                        return set->get(index);
                    }
            };

            static constexpr size_t cluster_size_bitcount = bitcount(sizeof(cluster_type));
            static constexpr size_t bits_per_cluster = sizeof(cluster_type) * 8;

            explicit constexpr bitset(allocator_type * const allocator = nullptr) : clusters{allocator}, _size_bits{0} {}
            constexpr bitset(const bitset &other) : clusters{other.clusters}, _size_bits{other._size_bits} {}
            constexpr bitset(bitset &&other) : clusters{std::move(other.clusters)}, _size_bits{std::move(other._size_bits)} {}

            ~bitset() {
                destroy();
            }

            bitset& operator =(const bitset &other) {
                clusters = other.clusters;
                _size_bits = other._size_bits;

                return *this;
            }

            bitset& operator =(bitset &&other) {
                clusters = std::move(other.clusters);
                _size_bits = std::move(other._size_bits);

                return *this;
            }

            /**
             * Clear this bitset, removing all bits.
             */
            void clear() {
                clusters.clear();
                _size_bits = 0;
            }

            /**
             * Release any resources.
             */
            void destroy() noexcept {
                clusters.destroy();
                _size_bits = 0;
            }

            /**
             * Get the size of the bitset.
             *
             * @return The size as number of bits.
             */
            constexpr size_type size_bits() const noexcept {
                return _size_bits;
            }

            /**
             * Get the size of the bitset.
             *
             * @return The size as number of clusters.
             */
            constexpr size_type size() const noexcept {
                return clusters.size();
            }

            /**
             * Get the capacity of the bitset.
             *
             * @return The capacity as number of clusters.
             */
            constexpr size_type capacity() const noexcept {
                return clusters.capacity();
            }

            /**
             * Reserve some capacity.
             *
             * @param The new suggested capacity, in bits.
             */
            constexpr void reserve(const size_type new_capacity) {
                #pragma clang diagnostic push
                #pragma clang diagnostic ignored "-Wshift-count-overflow"
                clusters.reserve(new_capacity / bits_per_cluster);
                #pragma clang diagnostic pop
            }

            /**
             * Resize the bitset. When growing, new bits have
             * undefined value.
             *
             * @param new_size The new size of the collection, in bits.
             */
            constexpr void resize(const size_type new_size) {
                #pragma clang diagnostic push
                #pragma clang diagnostic ignored "-Wshift-count-overflow"
                clusters.resize(new_size / bits_per_cluster);
                #pragma clang diagnostic pop

                _size_bits = new_size;
            }

            /**
             * Append a new bit.
             *
             * @param value The to assign.
             */
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

            /**
             * Append a new set bit.
             */
            constexpr void push_back_set() {
                const auto [target_cluster_index, internal_bit_index] = locate_bit(_size_bits);
                const size_type required_cluster_size = target_cluster_index + 1;

                clusters.resize(required_cluster_size);
                cluster_type &target_cluster = clusters[target_cluster_index];
                target_cluster |= static_cast<cluster_type>(1) << internal_bit_index;

                _size_bits++;
            }

            /**
             * Append a new clear bit.
             */
            constexpr void push_back_clear() {
                const auto [target_cluster_index, internal_bit_index] = locate_bit(_size_bits);
                const size_type required_cluster_size = target_cluster_index + 1;

                clusters.resize(required_cluster_size);
                cluster_type &target_cluster = clusters[target_cluster_index];
                target_cluster &= ~(static_cast<cluster_type>(1) << internal_bit_index);

                _size_bits++;
            }

            /**
             * Access a bit given its index.
             *
             * @param index The index of the bit to access.
             *
             * @return The value of the bit at that index.
             */
            constexpr bit_proxy operator[](const size_type index) {
                CCL_THROW_IF(index >= _size_bits, std::out_of_range{"Index out of range."});

                return {*this, index};
            }

            /**
             * Access a bit given its index.
             *
             * @param index The index of the bit to access.
             *
             * @return The value of the bit at that index.
             */
            constexpr const bit_proxy operator[](const size_type index) const {
                CCL_THROW_IF(index >= _size_bits, std::out_of_range{"Index out of range."});

                return {*this, index};
            }

            /**
             * Access a bit given its index.
             *
             * @param index The index of the bit to access.
             *
             * @return The value of the bit at that index.
             */
            constexpr bool get(const size_type index) const {
                CCL_THROW_IF(index >= _size_bits, std::out_of_range{"Index out of range."});

                const auto [target_cluster_index, internal_bit_index] = locate_bit(index);
                const cluster_type &target_cluster = clusters[target_cluster_index];

                return target_cluster & static_cast<cluster_type>(1) << internal_bit_index;
            }

            /**
             * Set a bit given its index.
             *
             * @param index The index of the bit to set.
             */
            constexpr void set(const size_type index) {
                CCL_THROW_IF(index >= _size_bits, std::out_of_range{"Index out of range."});

                const auto [target_cluster_index, internal_bit_index] = locate_bit(index);
                cluster_type &target_cluster = clusters[target_cluster_index];
                target_cluster |= static_cast<cluster_type>(1) << internal_bit_index;
            }

            /**
             * Clear a bit given its index.
             *
             * @param index The index of the bit to clear.
             */
            constexpr void clear(const size_type index) {
                CCL_THROW_IF(index >= _size_bits, std::out_of_range{"Index out of range."});

                const auto [target_cluster_index, internal_bit_index] = locate_bit(index);
                cluster_type &target_cluster = clusters[target_cluster_index];
                target_cluster &= ~(static_cast<cluster_type>(1) << internal_bit_index);
            }

            /**
             * Clear all bits without changing the length of the set.
             */
            constexpr void zero() {
                std::for_each(
                    clusters.begin(),
                    clusters.end(),
                    [] (cluster_type &c) {
                        c = 0;
                    }
                );
            }

            /**
             * Assign a value to a bit, given its index.
             *
             * @param index The index of the bit to assign.
             * @param value The value to assign the index.
             */
            constexpr void assign(const size_type index, const bool value) {
                CCL_THROW_IF(index >= _size_bits, std::out_of_range{"Index out of range."});

                const auto [target_cluster_index, internal_bit_index] = locate_bit(index);
                cluster_type &target_cluster = clusters[target_cluster_index];

                if(value) {
                    target_cluster |= static_cast<cluster_type>(1) << internal_bit_index;
                } else {
                    target_cluster &= ~(static_cast<cluster_type>(1) << internal_bit_index);
                }
            }

            constexpr auto& get_clusters() const noexcept {
                return clusters;
            }

            private:
                /**
                 * The sequence of clusters containing the bits.
                 */
                vector<cluster_type, allocator_type> clusters;

                /**
                 * Bit count.
                 */
                size_type _size_bits;

                /**
                 * Representation of the location of a bit within `clusters`.
                 */
                struct bit_location {
                    /**
                     * The index of the cluster where the bit is located.
                     */
                    size_type target_cluster_index;

                    /**
                     * The bit location within the cluster.
                     */
                    size_type internal_bit_index;
                };

                /**
                 * Compute the location of a bit in terms of cluster index
                 * and bit index within the cluster.
                 *
                 * @param The bit index.
                 *
                 * @return The bit location coordinates within `clusters`.
                 */
                constexpr bit_location locate_bit(const size_type index) const {
                    return {
                        index / bits_per_cluster,
                        index & (bits_per_cluster - 1)
                    };
                }
    };
}

#endif // CCL_BITSET_HPP
