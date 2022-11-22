/**
 * @file
 *
 * Open-addressed hash table.
 */
#ifndef CCL_HASHTABLE_HPP
#define CCL_HASHTABLE_HPP

#include <ccl/api.hpp>
#include <ccl/allocator.hpp>
#include <ccl/concepts.hpp>
#include <ccl/debug.hpp>

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

        constexpr hashtable_iterator() noexcept : keys{nullptr}, values{nullptr} {}
        constexpr hashtable_iterator(const key_pointer keys, const value_pointer values) noexcept : keys{keys}, values{values} {}
        constexpr hashtable_iterator(const hashtable_iterator &other) noexcept : keys{other.keys}, values{other.values} {}
        constexpr hashtable_iterator(hashtable_iterator &&other) noexcept : keys{std::move(other.keys)}, values{std::move(other.values)} {}

        constexpr hashtable_iterator& operator =(const hashtable_iterator &other) noexcept {
            keys = other.keys;
            values = other.values;

            return *this;
        }

        constexpr hashtable_iterator& operator =(hashtable_iterator &&other) noexcept {
            keys = std::move(other.keys);
            values = std::move(other.values);

            return *this;
        }

        constexpr value_reference operator*() const noexcept { return *values; }
        constexpr value_pointer operator->() const noexcept { return values; }

        constexpr hashtable_iterator& operator +=(const difference_type n) noexcept {
            ptr += n;
            return *this;
        }

        constexpr hashtable_iterator& operator -=(const difference_type n) noexcept {
            ptr -= n;
            return *this;
        }

        constexpr hashtable_iterator operator +(const difference_type n) const noexcept {
            return ptr + n;
        }

        constexpr hashtable_iterator operator -(const difference_type n) const noexcept {
            return ptr - n;
        }

        constexpr difference_type operator -(const hashtable_iterator other) const noexcept {
            return ptr - other.ptr;
        }

        constexpr reference operator[](const difference_type i) const noexcept {
            return ptr[i];
        }

        constexpr bool operator ==(const hashtable_iterator other) const noexcept { return ptr == other.ptr; }
        constexpr bool operator !=(const hashtable_iterator other) const noexcept { return ptr != other.ptr; }
        constexpr bool operator >(const hashtable_iterator other) const noexcept { return ptr > other.ptr; }
        constexpr bool operator <(const hashtable_iterator other) const noexcept { return ptr < other.ptr; }
        constexpr bool operator >=(const hashtable_iterator other) const noexcept { return ptr >= other.ptr; }
        constexpr bool operator <=(const hashtable_iterator other) const noexcept { return ptr <= other.ptr; }

        constexpr hashtable_iterator& operator --() noexcept {
            --ptr;
            return *this;
        }

        constexpr hashtable_iterator operator --(int) noexcept {
            return ptr--;
        }

        constexpr hashtable_iterator& operator ++() noexcept {
            ++ptr;
            return *this;
        }

        constexpr hashtable_iterator operator ++(int) noexcept {
            return ptr++;
        }

        private:
            key_pointer keys;
            value_pointer values;
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
     */
    template<
        typename K,
        typename V,
        typename Allocator = allocator
    >
    requires typed_allocator<Allocator, K> && typed_allocator<Allocator, V>
    class hashtable {
        public:
            using size_type = size_t;

            using key_type = K;
            using value_type = V;

            using key_pointer = K*;
            using value_pointer = V*;

            using key_reference = K&;
            using value_reference = V&;

            using rvalue_key_reference = K&&;
            using rvalue_value_reference = V&&;

            using const_key_reference = const K&;
            using const_value_reference = const V&;

            using allocator_type = Allocator;

            using iterator = hashtable_iterator<hashtable>;
            using const_iterator = hashtable_iterator<hashtable<const key_type, const value_type, allocator_type>>;
    };
}

#endif // CCL_HASHTABLE_HPP
