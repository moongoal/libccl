/**
 * @file
 *
 * A vector whose data is split into pages.
 * Certain operations of this vector do not guarantee item ordering preservation.
 */
#ifndef CCL_PAGED_VECTOR_HPP
#define CCL_PAGED_VECTOR_HPP

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
            return paged_vector_iterator{ *vector, index + n };
        }

        constexpr paged_vector_iterator operator -(const difference_type n) const noexcept {
            return paged_vector_iterator{ *vector, index - n };
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
            return paged_vector_iterator{ *vector, index-- };
        }

        constexpr paged_vector_iterator& operator ++() noexcept {
            ++index;
            return *this;
        }

        constexpr paged_vector_iterator operator ++(int) noexcept {
            return paged_vector_iterator{ *vector, index++ };
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

    template<typename ...Args>
    static constexpr paged_vector_iterator<Args...> operator +(
        const typename paged_vector_iterator<Args...>::difference_type n,
        const paged_vector_iterator<Args...> it
    ) noexcept {
        return paged_vector_iterator<Args...>{*it.vector, n};
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

            constexpr pointer clone(const pointer page, const std::size_t page_size) const {
                CCL_THROW_IF(!page, std::invalid_argument{"Page must not be null."});
                CCL_THROW_IF(!page_size, std::invalid_argument{"Page size must not be 0."});

                const pointer new_page = alloc::get_allocator()->template allocate<value_type>(page_size);

                std::uninitialized_copy(page, page + page_size, new_page);

                return new_page;
            }
    };

    template<typename T, typename Ptr = T*, typed_allocator<T> Allocator = allocator>
    class paged_vector : public internal::with_optional_allocator<Allocator> {
        static_assert(is_power_2(CCL_PAGE_SIZE));

        public:
            using value_type = T;
            using pointer_traits = ccl::pointer_traits<Ptr>;
            using pointer = typename pointer_traits::pointer;
            using const_pointer = typename pointer_traits::const_pointer;
            using reference = typename pointer_traits::reference;
            using const_reference = typename pointer_traits::const_reference;
            using size_type = std::size_t;
            using allocator_type = Allocator;
            using cloner = page_cloner<value_type, pointer, allocator_type>;
            using iterator = paged_vector_iterator<paged_vector>;
            using const_iterator = paged_vector_iterator<const paged_vector>;

            static constexpr size_type page_size = CCL_PAGE_SIZE;
            static constexpr size_type page_size_shift_width = bitcount(page_size) - 1;

        private:
            using alloc = internal::with_optional_allocator<Allocator>;
            using page_vector = vector<pointer, allocator_type>;

            page_vector _pages;
            size_type _size; // Item count

            constexpr void clone_pages_from(const paged_vector& v) {
                cloner cloner{alloc::get_allocator()};

                destroy();

                if(v.size()) {
                    _pages.reserve(v.size());

                    const auto start = v._pages.begin();
                    const auto last = v._pages.end() - 1;

                    for(auto it = start; it < last; ++it) {
                        _pages.push_back(cloner.clone(*it));
                    }

                    _pages.push_back(
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

            constexpr void grow(const size_type new_size) {
                const size_type current_size = size();
                const size_type current_page_count = _pages.size();

                reserve(new_size);

                const size_type first_page_size_to_fill = min(page_size, new_size) - next_item_index();
                const size_type size_delta = new_size - current_size; // Precondition: new size > current size
                const size_type intermediate_page_count = (size_delta - first_page_size_to_fill) >> page_size_shift_width;
                const size_type last_page_size = compute_last_page_size(new_size);
                const bool is_single_page_grow = intermediate_page_count == 0;

                // Default-construct first page items
                const size_type first_page_index = choose<size_type>(
                    current_page_count - 1,
                    0,
                    current_page_count
                );

                const pointer first_page = _pages[first_page_index];
                const pointer first_page_start = first_page + next_item_index();
                const pointer first_page_end = first_page + choose(next_item_index() + size_delta, page_size, is_single_page_grow);

                std::uninitialized_default_construct(first_page_start, first_page_end);

                // Default-construct full page items
                const size_type intermediate_page_base = first_page_index + 1;
                const size_type intermediate_page_ceil = intermediate_page_count + intermediate_page_base;

                for(size_type i = intermediate_page_base; i < intermediate_page_ceil; ++i) {
                    const pointer page = _pages[i];

                    std::uninitialized_default_construct(page, page + page_size);
                }

                // Default-construct last page items
                const bool should_construct_last_page = (
                    intermediate_page_count > 0
                    || size_delta > page_size
                );

                if(should_construct_last_page) {
                    const pointer last_page = _pages[_pages.size() - 1];

                    std::uninitialized_default_construct(last_page, last_page + last_page_size);
                }

                _size = new_size;
            }

            constexpr void shrink(const size_type new_size CCLUNUSED) {
                auto page = _pages.end() - 1;
                size_type actual_size = size();

                if(actual_size > page_size) {
                    // Remove items from last page
                    if(next_item_index()) {
                        if constexpr(!std::is_trivially_destructible_v<value_type>) {
                            std::destroy(*page, *page + next_item_index());
                        }

                        actual_size -= next_item_index();

                        --page;
                    }

                    // Remove items from intermediate pages
                    const size_type intermediate_page_count = (actual_size >> page_size_shift_width) - 1;

                    if constexpr(!std::is_trivially_destructible_v<value_type>) {
                        for(size_type i = 0; i < intermediate_page_count; ++i, --page) {
                            std::destroy(*page, *page + page_size);
                        }
                    }

                    actual_size -= page_size * intermediate_page_count;

                    // Remove items from first affected page
                    if constexpr(!std::is_trivially_destructible_v<value_type>) {
                        if(actual_size) {
                            std::destroy(*page + new_size, *page + page_size);
                        }
                    }
                } else {
                    // Remove items from first affected page
                    if constexpr(!std::is_trivially_destructible_v<value_type>) {
                        std::destroy(*page + new_size, *page + next_item_index());
                    }
                }

                _size = new_size;
            }

            constexpr iterator make_room(iterator it, const size_type n = 1) {
                CCL_ASSERT(it >= begin() || it <= end());
                CCL_ASSERT(n >= 1);

                reserve(_size + n);

                const auto old_end = end();
                _size += n;

                if(it < old_end) {
                    std::move_backward(
                        it,
                        old_end,
                        end()
                    );
                }

                return it;
            }

        public:
            constexpr paged_vector(
                allocator_type * const allocator = nullptr
            ) noexcept : alloc{allocator}, _size{0} {}

            constexpr paged_vector(const paged_vector& other) : alloc{other.get_allocator()} {
                clone_pages_from(other);
                _size = other._size;
            }

            constexpr paged_vector(paged_vector &&other)
                : alloc{std::move(other.get_allocator())},
                _pages{std::move(other._pages)},
                _size{other._size}
            {}

            constexpr paged_vector(
                std::initializer_list<T> values,
                allocator_type * const allocator = nullptr
            ) : alloc{allocator} {
                reserve(values.size());
                _size = values.size();
                std::uninitialized_copy(values.begin(), values.end(), begin());
            }

            template<std::ranges::input_range InputRange>
            constexpr paged_vector(const InputRange& input, allocator_type * const allocator = nullptr)
            : paged_vector{allocator} {
                const size_type input_size = std::abs(std::ranges::distance(input));

                if(input_size > 0) {
                    reserve(input_size);
                    _size = input_size;
                    std::uninitialized_copy(input.begin(), input.end(), begin());
                }
            }

            constexpr ~paged_vector() {
                destroy();
            }

            template<typename Other>
            constexpr paged_vector& operator=(const Other& other) {
                clear();

                for(const auto &it : other) {
                    push_back(it);
                }

                return *this;
            }

            constexpr paged_vector& operator=(const paged_vector& other) {
                alloc::operator=(other);

                clone_pages_from(other);
                _size = other._size;

                return *this;
            }

            constexpr paged_vector& operator=(paged_vector &&other) {
                destroy();
                alloc::operator=(std::move(other));

                _pages = std::move(other._pages);
                _size = std::move(other._size);

                return *this;
            }

            constexpr void clear() {
                if constexpr(!std::is_trivially_destructible_v<value_type>) {
                    if(!_pages.is_empty()) {
                        size_type remaining_size = size();
                        auto it = _pages.begin();

                        const auto recycle = [](const pointer page, const size_type page_size) {
                            std::destroy(page, page + page_size);
                        };

                        for(; remaining_size > page_size; remaining_size -= page_size, ++it) {
                            recycle(*it, page_size);
                        }

                        recycle(*it, remaining_size);
                    }
                }

                _size = 0;
            }

            /**
             * Like clear but also release all allocated memory.
             */
            constexpr void destroy() {
                if(_pages.data()) {
                    clear();

                    for(const auto p : _pages) {
                        alloc::get_allocator()->deallocate(p);
                    }

                    _pages.destroy();
                }
            }

            constexpr size_type size() const noexcept {
                return _size;
            }

            constexpr size_type capacity() const noexcept {
                return _pages.size() * page_size;
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

            constexpr reference operator[](const iterator it) {
                return operator[](it.index);
            }

            constexpr const_reference operator[](const size_type index) const {
                return get(index);
            }

            constexpr const_reference operator[](const iterator it) const {
                return operator[](it.index);
            }

            constexpr const_reference operator[](const const_iterator it) const {
                return operator[](it.index);
            }

            constexpr reference get(const size_type index) {
                const size_type page_index = item_page(index);
                const size_type item_index = index_in_page(index);

                CCL_THROW_IF(index >= _size, std::out_of_range{"Index out of bounds."});

                return _pages[page_index][item_index];
            }

            constexpr const_reference get(const size_type index) const {
                const size_type page_index = item_page(index);
                const size_type item_index = index_in_page(index);

                CCL_THROW_IF(index >= _size, std::out_of_range{"Index out of bounds."});

                return _pages[page_index][item_index];
            }

            constexpr void reserve(const size_type new_capacity) {
                const size_type current_capacity = capacity();

                if(new_capacity > current_capacity) {
                    const size_type actual_new_capacity = increase_paged_capacity(current_capacity, new_capacity, page_size);
                    const size_type new_page_count = max(1ULL, actual_new_capacity >> page_size_shift_width);
                    const size_type page_to_add_count = new_page_count - _pages.size();

                    for(std::size_t i = 0; i < page_to_add_count; ++i) {
                        const pointer new_page = alloc::get_allocator()->template allocate<value_type>(page_size);
                        _pages.push_back(new_page);
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

            template<typename ...Args>
            constexpr void emplace(Args&& ...args) {
                const size_type old_size = _size;

                reserve(size() + 1);
                _size += 1;

                reference new_item = get(old_size);
                std::construct_at(&new_item, std::forward<Args>(args)...);
            }

            constexpr void resize(const size_type new_size) {
                const size_type current_size = size();

                if(new_size > current_size) {
                    grow(new_size);
                } else if(new_size < current_size) {
                    shrink(new_size);
                }
            }

            constexpr void insert(iterator where, const_reference value) {
                CCL_THROW_IF(where < begin() || where > end(), std::out_of_range{"Iterator out of range."});

                const bool must_assign = where != end();
                where = make_room(where);
                CCL_ASSERT(where >= begin() || where <= end());

                if(must_assign) {
                    *where = value;
                } else {
                    std::construct_at(std::to_address(where), value);
                }
            }

            constexpr void insert(iterator where, T&& value) {
                CCL_THROW_IF(where < begin() || where > end(), std::out_of_range{"Iterator out of range."});

                const bool must_assign = where != end();
                where = make_room(where);
                CCL_ASSERT(where >= begin() || where <= end());

                if(must_assign) {
                    *where = value;
                } else {
                    std::construct_at(std::to_address(where), std::move(value));
                }
            }

            template<std::ranges::input_range Range>
            constexpr void insert(iterator where, const Range &range) {
                CCL_THROW_IF(where < begin() || where > end(), std::out_of_range{"Iterator out of range."});

                const size_type input_size = std::abs(std::ranges::distance(range));

                if(input_size > 0) {
                    where = make_room(where, input_size);

                    std::ranges::copy(range, where);
                }
            }

            constexpr page_vector& pages() { return _pages; }
            constexpr const page_vector& pages() const { return _pages; }

            template<typename ...Args>
            constexpr reference emplace_at(iterator where, Args&& ...args) {
                CCL_THROW_IF(where < begin() || where > end(), std::out_of_range{"Iterator out of range."});

                const bool must_destroy = where != end();
                where = make_room(where);

                CCL_ASSERT(where >= begin() || where <= end());

                if(must_destroy) {
                    std::destroy_at(std::to_address(where));
                }

                std::construct_at(
                    std::to_address(where),
                    std::forward<Args>(args)...
                );

                return *where;
            }

            constexpr void erase(const iterator start, const iterator finish) {
                CCL_THROW_IF(start < begin() || start > end(), std::out_of_range{"Iterator out of range."});
                CCL_THROW_IF(finish < begin() || finish > end(), std::out_of_range{"Iterator out of range."});

                static_assert(std::is_move_assignable_v<T>);

                std::move(finish, end(), start);
                std::destroy(finish, end());

                _size -= finish - start;
            }

            constexpr void erase(const iterator it) {
                erase(it, it + 1);
            }
    };
}

#endif // CCL_PAGED_VECTOR_HPP
