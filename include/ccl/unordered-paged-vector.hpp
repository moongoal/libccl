/**
 * @file
 *
 * A vector whose data is split into pages.
 * Certain operations of this vector do not guarantee item ordering preservation.
 */
#ifndef CCL_UNORDERED_PAGED_VECTOR_HPP
#define CCL_UNORDERED_PAGED_VECTOR_HPP

#include <memory>
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
        using const_pointer = typename Vector::const_pointer;
        using const_reference = typename Vector::const_reference;
        using size_type = typename Vector::size_type;
        using vector_type = Vector;

        explicit constexpr paged_vector_iterator(
            vector_type &vector,
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

        constexpr reference operator*() noexcept { return (*vector)[index]; }
        constexpr pointer operator->() noexcept { return &(*vector)[index]; }

        constexpr const_reference operator*() const noexcept { return (*vector)[index]; }
        constexpr const_pointer operator->() const noexcept { return &(*vector)[index]; }

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
            return (*vector)[i + index];
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

        vector_type *vector;
        size_type index;
    };

    template<typename Vector>
    constexpr bool operator ==(const paged_vector_iterator<Vector> &a, const paged_vector_iterator<Vector> &b) noexcept {
        return a.index == b.index;
    }

    template<typename Vector>
    constexpr bool operator !=(const paged_vector_iterator<Vector> &a, const paged_vector_iterator<Vector> &b) noexcept {
        return a.index != b.index;
    }

    template<typename Vector>
    constexpr bool operator >(const paged_vector_iterator<Vector> &a, const paged_vector_iterator<Vector> &b) noexcept {
        return a.index > b.index;
    }

    template<typename Vector>
    constexpr bool operator <(const paged_vector_iterator<Vector> &a, const paged_vector_iterator<Vector> &b) noexcept {
        return a.index < b.index;
    }

    template<typename Vector>
    constexpr bool operator >=(const paged_vector_iterator<Vector> &a, const paged_vector_iterator<Vector> &b) noexcept {
        return a.index >= b.index;
    }

    template<typename Vector>
    constexpr bool operator <=(const paged_vector_iterator<Vector> &a, const paged_vector_iterator<Vector> &b) noexcept {
        return a.index <= b.index;
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
            explicit constexpr page_cloner(allocator_type * const allocator = null<allocator_type>) : alloc{allocator} {}

            constexpr pointer clone(const pointer page) const {
                return clone(page, CCL_PAGE_SIZE);
            }

            constexpr pointer clone(const pointer page, const size_t page_size) const {
                CCL_THROW_IF(!page, std::invalid_argument{"Page must not be null."});
                CCL_THROW_IF(!page_size, std::invalid_argument{"Page size must not be 0."});

                const pointer new_page = alloc::get_allocator()->template allocate<value_type>(page_size);

                std::uninitialized_copy(page, page + page_size, new_page);

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
            size_type _size; // Item count

            constexpr void clone_pages_from(const unordered_paged_vector& v) {
                cloner cloner{alloc::get_allocator()};

                clear();

                if(v.size()) {
                    pages.reserve(v.size());

                    const auto start = v.pages.begin();
                    const auto last = v.pages.end() - 1;

                    for(auto it = start; it < last; ++it) {
                        pages.push_back(cloner.clone(*it));
                    }

                    pages.push_back(
                        cloner.clone(*last, v.next_item_index())
                    );
                }
            }

            static constexpr size_type compute_last_page_size(const size_type total_size) {
                return total_size & (page_size - 1);
            }

            constexpr size_type next_item_index() const {
                return compute_last_page_size(_size);
            }

            constexpr void resize_grow(const size_type new_size) {
                const size_type current_size = size();
                const size_type current_page_count = pages.size();

                reserve(new_size);

                const size_type first_page_size_to_fill = min<size_type>(page_size, new_size) - next_item_index();
                const size_type size_delta = new_size - current_size; // Precondition: new size > current size
                const size_type intermediate_page_count = (size_delta - first_page_size_to_fill) >> page_size_shift_width;
                const size_type last_page_size = compute_last_page_size(new_size);
                const bool is_single_page_grow = intermediate_page_count == 0;

                // Default-construct first page items
                const size_type first_page_index = choose(
                    current_page_count - 1,
                    static_cast<size_type>(0),
                    current_page_count
                );

                const pointer first_page = pages[first_page_index];
                const pointer first_page_start = first_page + next_item_index();
                const pointer first_page_end = first_page + choose(next_item_index() + size_delta, page_size, is_single_page_grow);

                std::uninitialized_default_construct(first_page_start, first_page_end);

                // Default-construct full page items
                const size_type intermediate_page_base = first_page_index + 1;
                const size_type intermediate_page_ceil = intermediate_page_count + intermediate_page_base;

                for(size_type i = intermediate_page_base; i < intermediate_page_ceil; ++i) {
                    const pointer page = pages[i];

                    std::uninitialized_default_construct(page, page + page_size);
                }

                // Default-construct last page items
                const bool should_construct_last_page = (
                    intermediate_page_count > 0
                    || size_delta > page_size
                );

                if(should_construct_last_page) {
                    const pointer last_page = pages[pages.size() - 1];

                    std::uninitialized_default_construct(last_page, last_page + last_page_size);
                }

                _size = new_size;
            }

            constexpr void resize_shrink(const size_type new_size CCLUNUSED) {
                auto page = pages.end() - 1;
                size_type actual_size = size();

                if(actual_size > page_size) {
                    // Remove items from last page
                    if(next_item_index()) {
                        if constexpr(std::is_destructible_v<value_type>) {
                            std::destroy(*page, *page + next_item_index());
                        }

                        actual_size -= next_item_index();

                        --page;
                    }

                    // Remove items from intermediate pages
                    const size_type intermediate_page_count = (actual_size >> page_size_shift_width) - 1;

                    if constexpr(std::is_destructible_v<value_type>) {
                        for(size_type i = 0; i < intermediate_page_count; ++i, --page) {
                            std::destroy(*page, *page + page_size);
                        }
                    }

                    actual_size -= page_size * intermediate_page_count;

                    // Remove items from first affected page
                    if constexpr(std::is_destructible_v<value_type>) {
                        if(actual_size) {
                            std::destroy(*page + new_size, *page + page_size);
                        }
                    }
                } else {
                    // Remove items from first affected page
                    if constexpr(std::is_destructible_v<value_type>) {
                        std::destroy(*page + new_size, *page + next_item_index());
                    }
                }

                _size = new_size;
            }

        public:
            constexpr unordered_paged_vector(
                allocator_type * const allocator = nullptr
            ) noexcept : alloc{allocator}, _size{0} {}

            constexpr unordered_paged_vector(const unordered_paged_vector& other) : alloc{other.get_allocator()} {
                clone_pages_from(other);
                _size = other._size;
            }

            constexpr unordered_paged_vector(unordered_paged_vector &&other)
                : alloc{std::move(other.get_allocator())},
                pages{std::move(other.pages)},
                _size{other._size}
            {}

            constexpr ~unordered_paged_vector() {
                clear();
            }

            template<typename Other>
            constexpr unordered_paged_vector& operator=(const Other& other) {
                clear();

                for(const auto &it : other) {
                    push_back(it);

                    _size++;
                }

                return *this;
            }

            template<>
            constexpr unordered_paged_vector& operator=<unordered_paged_vector>(const unordered_paged_vector& other) {
                alloc::operator=(other);

                clone_pages_from(other.pages);
                next_item_index() = other.next_item_index();

                return *this;
            }

            constexpr unordered_paged_vector& operator=(unordered_paged_vector &&other) {
                alloc::operator=(std::move(other));

                pages = std::move(other.pages);
                next_item_index() = std::move(other.next_item_index());

                return *this;
            }

            constexpr void clear() {
                const auto begin = pages.begin();
                const auto end = pages.end();

                if(begin != end) {
                    const auto recycle = [this](const pointer page, const size_type page_size) {
                        if constexpr(std::is_destructible_v<value_type>) {
                            std::destroy(page, page + page_size);
                        }

                        alloc::get_allocator()->deallocate(page);
                    };

                    const auto last = end - 1;

                    for(auto it = begin; it != last; ++it) {
                        recycle(*it, page_size);
                    }

                    recycle(*last, next_item_index());

                    pages.clear();
                    _size = 0;
                }
            }

            constexpr size_type size() const noexcept {
                return _size;
            }

            constexpr size_type capacity() const noexcept {
                return pages.size() * page_size;
            }

            constexpr iterator begin() noexcept { return iterator{*this, 0}; }
            constexpr iterator end() noexcept { return iterator{*this, size()}; }
            constexpr const_iterator begin() const noexcept { return const_iterator{*this, 0}; }
            constexpr const_iterator end() const noexcept { return const_iterator{*this, size()}; }

            constexpr const_iterator cbegin() const noexcept { return const_iterator{*this, 0}; }
            constexpr const_iterator cend() const noexcept { return const_iterator{*this, size()}; }

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
                return get(index);
            }

            constexpr const_reference operator[](const size_type index) const {
                return get(index);
            }

            constexpr reference operator[](const iterator it) const {
                return operator[](it.index);
            }

            constexpr const_reference operator[](const const_iterator it) const {
                return operator[](it.index);
            }

            constexpr reference get(const size_type index) {
                const size_type page_index = item_page(index);
                const size_type item_index = index_in_page(index);

                CCL_THROW_IF(index >= _size, std::out_of_range{"Index out of bounds."});

                return pages[page_index][item_index];
            }

            constexpr const_reference get(const size_type index) const {
                const size_type page_index = item_page(index);
                const size_type item_index = index_in_page(index);

                CCL_THROW_IF(index >= _size, std::out_of_range{"Index out of bounds."});

                return pages[page_index][item_index];
            }

            constexpr void reserve(const size_type new_capacity) {
                const size_type current_capacity = capacity();

                if(new_capacity > current_capacity) {
                    const size_type actual_new_capacity = increase_paged_capacity(current_capacity, new_capacity, page_size);
                    const size_type new_page_count = max(1ULL, actual_new_capacity >> page_size_shift_width);
                    const size_type page_to_add_count = new_page_count - pages.size();

                    for(size_t i = 0; i < page_to_add_count; ++i) {
                        const pointer new_page = alloc::get_allocator()->template allocate<value_type>(page_size);
                        pages.push_back(new_page);
                    }
                }
            }

            constexpr void push_back(T&& value) {
                const size_type old_size = _size;

                reserve(size() + 1);
                _size += 1;

                reference new_item = get(old_size);
                std::uninitialized_move(&value, &value + 1, &new_item);
            }

            constexpr void push_back(const_reference value) {
                const size_type old_size = _size;

                reserve(size() + 1);
                _size += 1;

                reference new_item = get(old_size);
                std::uninitialized_copy(&value, &value + 1, &new_item);
            }

            constexpr void resize(const size_type new_size) {
                const size_type current_size = size();

                if(new_size > current_size) {
                    resize_grow(new_size);
                } else if(new_size < current_size) {
                    resize_shrink(new_size);
                }
            }

            constexpr page_vector& get_pages() { return pages; }
            constexpr const page_vector& get_pages() const { return pages; }
    };
}

#endif // CCL_UNORDERED_PAGED_VECTOR_HPP
