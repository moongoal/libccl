/**
 * @file
 *
 * A vector whose data is split into pages.
 * Certain operations of this vector do not guarantee item ordering preservation.
 */
#ifndef CCL_UNORDERED_PAGED_VECTOR_HPP
#define CCL_UNORDERED_PAGED_VECTOR_HPP

#include <ccl/api.hpp>
#include <ccl/debug.hpp>
#include <ccl/vector.hpp>
#include <ccl/definitions.hpp>
#include <ccl/type-traits.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/internal/optional-allocator.hpp>
#include <ccl/concepts.hpp>
#include <ccl/util.hpp>

namespace ccl {
    template<typename Vector>
    struct paged_vector_iterator {
        using iterator_category = std::contiguous_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = typename Vector::value_type;
        using pointer = typename Vector::pointer;
        using reference = typename Vector::reference;
        using size_type = typename Vector::size_type;
        using vector_type = Vector;

        explicit constexpr paged_vector_iterator(
            const vector_type &vector,
            const size_type index = 0
        ) noexcept
            : vector{&vector},
            index{index}
        {}

        constexpr paged_vector_iterator(const paged_vector_iterator &other) noexcept
            : vector{other.vector},
            index{other.index}
        {}

        constexpr paged_vector_iterator& operator =(const paged_vector_iterator &other) noexcept {
            vector = other.vector;
            index = other.index;

            return *this;
        }

        constexpr reference operator*() const noexcept { return vector[index]; }
        constexpr pointer operator->() const noexcept { return &vector[index]; }

        constexpr paged_vector_iterator& operator +=(const difference_type n) noexcept {
            index += n;
            return *this;
        }

        constexpr paged_vector_iterator& operator -=(const difference_type n) noexcept {
            index -= n;
            return *this;
        }

        constexpr paged_vector_iterator operator +(const difference_type n) const noexcept {
            return { vector, index + n };
        }

        constexpr paged_vector_iterator operator -(const difference_type n) const noexcept {
            return { vector, index - n };
        }

        constexpr difference_type operator -(const paged_vector_iterator other) const noexcept {
            return index - other.index;
        }

        constexpr reference operator[](const difference_type i) const noexcept {
            return vector[i + index];
        }

        constexpr paged_vector_iterator& operator --() noexcept {
            --index;
            return *this;
        }

        constexpr paged_vector_iterator operator --(int) noexcept {
            return { vector, index-- };
        }

        constexpr paged_vector_iterator& operator ++() noexcept {
            ++index;
            return *this;
        }

        constexpr paged_vector_iterator operator ++(int) noexcept {
            return { vector, index++ };
        }

        private:
            vector_type *vector;
            size_type index;
    };

    template<typename Vector>
    constexpr bool operator ==(const paged_vector_iterator<Vector> &a, const paged_vector_iterator<Vector> &b) noexcept {
        return a.ptr == b.ptr;
    }

    template<typename Vector>
    constexpr bool operator !=(const paged_vector_iterator<Vector> &a, const paged_vector_iterator<Vector> &b) noexcept {
        return a.ptr != b.ptr;
    }

    template<typename Vector>
    constexpr bool operator >(const paged_vector_iterator<Vector> &a, const paged_vector_iterator<Vector> &b) noexcept {
        return a.ptr > b.ptr;
    }

    template<typename Vector>
    constexpr bool operator <(const paged_vector_iterator<Vector> &a, const paged_vector_iterator<Vector> &b) noexcept {
        return a.ptr < b.ptr;
    }

    template<typename Vector>
    constexpr bool operator >=(const paged_vector_iterator<Vector> &a, const paged_vector_iterator<Vector> &b) noexcept {
        return a.ptr >= b.ptr;
    }

    template<typename Vector>
    constexpr bool operator <=(const paged_vector_iterator<Vector> &a, const paged_vector_iterator<Vector> &b) noexcept {
        return a.ptr <= b.ptr;
    }

    template<typename Vector>
    static constexpr paged_vector_iterator<Vector> operator +(
        const typename paged_vector_iterator<Vector>::difference_type n,
        const paged_vector_iterator<Vector> it
    ) noexcept {
        return paged_vector_iterator<Vector>{it.get_data() + n};
    }

    template<typename T, typename Ptr, typed_allocator<T> Allocator>
    struct page_cloner {};

    template<typename T, typed_allocator<T> Allocator>
    struct page_cloner<T, T*, Allocator> : public internal::with_optional_allocator<Allocator> {
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using allocator_type = Allocator;

        private:
            using alloc = internal::with_optional_allocator<Allocator>;

        public:
            explicit constexpr page_cloner(allocator_type * const allocator = null<allocator_type>) : alloc{allocator ? allocator : get_default_allocator()} {}

            pointer clone(const pointer page) const {
                const pointer new_page = alloc::get_allocator();

                std::uninitialized_copy(page, page + CCL_PAGE_SIZE, new_page);

                return new_page;
            }
    };

    template<typename T, typename Ptr = T*, typed_allocator<T> Allocator = allocator>
    class unordered_paged_vector : public internal::with_optional_allocator<Allocator> {
        static_assert(is_power_2(CCL_PAGE_SIZE));

        public:
            using value_type = T;
            using pointer_traits = ccl::pointer_traits<Ptr>;
            using pointer = typename pointer_traits::pointer;
            using const_pointer = typename pointer_traits::const_pointer;
            using reference = typename pointer_traits::reference;
            using const_reference = typename pointer_traits::const_reference;
            using size_type = size_t;
            using allocator_type = Allocator;
            using cloner = page_cloner<value_type, pointer, allocator_type>;
            using iterator = paged_vector_iterator<unordered_paged_vector>;
            using const_iterator = paged_vector_iterator<const unordered_paged_vector>;

            static constexpr size_type page_size = CCL_PAGE_SIZE;
            static constexpr size_type page_size_shift_width = bitcount(page_size) - 1;

        private:
            using alloc = internal::with_optional_allocator<Allocator>;
            using page_vector = vector<pointer, allocator>;

            page_vector pages;
            size_type last_item_index; // Within last page

            constexpr void clone_pages_from(const page_vector& v) {
                cloner cloner{alloc::get_allocator()};

                clear();

                pages.reserve(v.size());

                for(const auto p : v) {
                    pages.push_back(cloner.clone(p));
                }
            }

            constexpr void clear() {
                for(const auto p : pages) {
                    if constexpr(std::is_destructible_v<value_type>) {
                        std::destroy(p, p + page_size);
                    }

                    alloc::get_allocator()->deallocate(p);
                }

                pages.clear();
                last_item_index = 0;
            }

            constexpr void update_last_page_size_from_total_size(const size_type total_size) {
                last_item_index = total_size & (page_size - 1);
            }

            constexpr pointer get_pages() { return pages; }
            constexpr const_pointer get_pages() const { return pages; }

        public:
            constexpr unordered_paged_vector() noexcept : last_item_index{0} {}

            constexpr unordered_paged_vector(const unordered_paged_vector& other) {
                clone_pages_from(other.pages);

                last_item_index = other.last_item_index;
            }

            constexpr unordered_paged_vector(unordered_paged_vector &&other) : pages{std::move(other.pages)}, last_item_index{other.last_item_index} {}

            constexpr ~unordered_paged_vector() {
                clear();
            }

            template<typename Other>
            constexpr unordered_paged_vector& operator=(const Other& other) {
                size_type total_size = 0;

                clear();

                for(const auto &it : other) {
                    push_back(it);

                    total_size++;
                }

                update_last_page_size_from_total_size(total_size);

                return *this;
            }

            template<>
            constexpr unordered_paged_vector& operator=<unordered_paged_vector>(const unordered_paged_vector& other) {
                clone_pages_from(other.pages);
                last_item_index = other.last_item_index;

                return *this;
            }

            constexpr unordered_paged_vector& operator=(unordered_paged_vector &&other) {
                pages = std::move(other.pages);
                last_item_index = std::move(other.last_item_index);

                return *this;
            }

            constexpr size_type size() const noexcept {
                const size_type page_count = pages.size();

                return choose(
                    (page_count - 1) * page_size,
                    0ULL,
                    page_count > 0
                ) + last_item_index;
            }

            constexpr size_type capacity() const noexcept {
                return pages.capacity() * page_size;
            }

            constexpr iterator begin() noexcept { return iterator{pages, 0}; }
            constexpr iterator end() noexcept { return iterator{pages, size()}; }
            constexpr const_iterator begin() const noexcept { return const_iterator{pages, 0}; }
            constexpr const_iterator end() const noexcept { return const_iterator{pages, size()}; }

            constexpr const_iterator cbegin() const noexcept { return const_iterator{pages, 0}; }
            constexpr const_iterator cend() const noexcept { return const_iterator{pages, size()}; }

            constexpr size_type item_page(const size_type index) const {
                return index >> page_size_shift_width;
            }

            constexpr size_type item_page(const iterator it) const {
                return item_page(it.index);
            }

            constexpr size_type item_page(const const_iterator it) const {
                return item_page(it.index);
            }

            constexpr size_type index_in_page(const size_type index) const {
                return index & (page_size - 1);
            }

            constexpr size_type index_in_page(const iterator it) const {
                return index_in_page(it.index);
            }

            constexpr size_type index_in_page(const const_iterator it) const {
                return index_in_page(it.index);
            }

            constexpr reference operator[](const size_type index) {
                const size_type page_index = item_page(index);
                const size_type item_index = index_in_page(index);

                return get(page_index, item_index);
            }

            constexpr const_reference operator[](const size_type index) const {
                const size_type page_index = item_page(index);
                const size_type item_index = index_in_page(index);

                return get(page_index, item_index);
            }

            constexpr reference operator[](const iterator it) const {
                return operator[](it.index);
            }

            constexpr const_reference operator[](const const_iterator it) const {
                return operator[](it.index);
            }

            constexpr reference get(const size_type page_index, const size_type item_index) {
                CCL_THROW_IF(page_index >= pages.size() || item_index > last_item_index, std::out_of_range{"Index out of bounds."});

                return pages[page_index][item_index];
            }

            constexpr const_reference get(const size_type page_index, const size_type item_index) const {
                CCL_THROW_IF(page_index >= pages.size() || item_index > last_item_index, std::out_of_range{"Index out of bounds."});

                return pages[page_index][item_index];
            }

            constexpr void reserve(const size_type new_capacity) {
                const size_type current_capacity = capacity();

                if(new_capacity > current_capacity) {
                    const size_type actual_new_capacity = increase_capacity(current_capacity, new_capacity);
                    const size_type new_page_count = max(1ULL, actual_new_capacity >> page_size_shift_width);

                    for(size_t i = 0; i < new_page_count; ++i) {
                        const pointer new_page = alloc::get_allocator()->template allocate<value_type>(page_size);
                        pages.push_back(new_page);
                    }
                }
            }

            constexpr void push_back(const_reference value) {
                reserve(size() + 1);

                reference new_item = get(pages.size() - 1, last_item_index);
                std::uninitialized_copy(&value, &value + 1, &new_item);

                last_item_index = (last_item_index + 1) & (page_size - 1);
            }
    };
}

#endif // CCL_UNORDERED_PAGED_VECTOR_HPP
