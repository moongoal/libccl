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
#include <ccl/memory/allocator.hpp>

namespace ccl {
    /**
     * A variable sequence of bits.
     */
    template<
        typed_allocator<uint64_t> Allocator = allocator
    > class bitset {
        public:
            using cluster_type = uint64_t;
            using size_type = std::size_t;
            using allocator_type = Allocator;

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

            class const_bit_proxy {
                const bitset *set;
                size_type index;

                public:
                    const_bit_proxy() = delete;
                    const_bit_proxy(const bitset &set, const size_type index) : set{&set}, index{index} {}
                    const_bit_proxy(const const_bit_proxy &) = default;
                    const_bit_proxy(const_bit_proxy &&) = default;

                    constexpr bit_proxy& operator =(const bool value) = delete;

                    constexpr operator bool() const noexcept {
                        return set->get(index);
                    }
            };

            static constexpr std::size_t bits_per_cluster = sizeof(cluster_type) * 8;
            static constexpr std::size_t cluster_size_bitcount = bitcount(bits_per_cluster) - 1;

            explicit constexpr bitset(
                const allocation_flags alloc_flags = CCL_ALLOCATOR_DEFAULT_FLAGS,
                allocator_type * const allocator = nullptr
            ) : clusters{alloc_flags, allocator}, _size_bits{0} {}

            constexpr bitset(const bitset &other) : clusters{other.clusters}, _size_bits{other._size_bits} {}
            constexpr bitset(bitset &&other) : clusters{std::move(other.clusters)}, _size_bits{std::move(other._size_bits)} {
                other._size_bits = 0;
            }

            ~bitset() {
                destroy();
            }

            constexpr bitset& operator =(const bitset &other) {
                clusters = other.clusters;
                _size_bits = other._size_bits;

                return *this;
            }

            constexpr bitset& operator =(bitset &&other) {
                clusters = std::move(other.clusters);
                _size_bits = std::move(other._size_bits);

                other._size_bits = 0;

                return *this;
            }

            /**
             * Clear this bitset, removing all bits.
             */
            constexpr void clear() {
                clusters.clear();
                _size_bits = 0;
            }

            /**
             * Release any resources.
             */
            constexpr void destroy() noexcept {
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
                clusters.reserve(new_capacity >> cluster_size_bitcount);
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
                clusters.resize(new_size >> cluster_size_bitcount);
                #pragma clang diagnostic pop

                _size_bits = new_size;
            }

            /**
             * Append a new bit. If the value of the bit to add is known,
             * prefer `push_back_set()` or `push_back_clear()`.
             *
             * @param value The to assign.
             */
            constexpr void push_back(const bool value) {
                const auto [target_cluster_index, internal_bit_index] = locate_bit(_size_bits);
                const size_type required_cluster_size = target_cluster_index + 1;

                clusters.resize(required_cluster_size);
                cluster_type &target_cluster = clusters[target_cluster_index];

                const cluster_type cluster_w_enabled = target_cluster | static_cast<cluster_type>(1) << internal_bit_index;
                const cluster_type cluster_w_disabled = target_cluster & ~(static_cast<cluster_type>(1) << internal_bit_index);

                target_cluster = choose(cluster_w_enabled, cluster_w_disabled, value);

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
            constexpr const const_bit_proxy operator[](const size_type index) const {
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

                return target_cluster & (static_cast<cluster_type>(1) << internal_bit_index);
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
                std::fill(
                    clusters.begin(),
                    clusters.end(),
                    static_cast<cluster_type>(0)
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

                const cluster_type cluster_w_enabled = target_cluster | (static_cast<cluster_type>(1) << internal_bit_index);
                const cluster_type cluster_w_disabled = target_cluster & ~(static_cast<cluster_type>(1) << internal_bit_index);

                target_cluster = choose(cluster_w_enabled, cluster_w_disabled, value);
            }

            /**
             * Get the underlying data structure where the bit clusters
             * are stored.
             *
             * @return The internal bit storage.
             */
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
                        index >> cluster_size_bitcount,
                        index & (bits_per_cluster - 1)
                    };
                }

                constexpr allocator_type* get_allocator() const noexcept { return clusters.get_allocator(); }
                constexpr allocation_flags get_allocation_flags() const noexcept { return clusters.get_allocation_flags(); }
    };
}

#endif // CCL_BITSET_HPP
