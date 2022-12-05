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

namespace ccl {
    template<typename Vector>
    struct paged_vector_iterator;

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
            explicit constexpr page_cloner(allocator_type * const allocator = nullptr) : alloc{allocator ? allocator : get_default_allocator()} {}

            pointer clone(const pointer page) const {
                const pointer new_page = alloc::get_allocator();

                std::uninitialized_copy(page, page + CCL_PAGE_SIZE, new_page);

                return new_page;
            }
    };

    template<typename T, typename Ptr = T*, typed_allocator<T> Allocator = allocator>
    class unordered_paged_vector : public internal::with_optional_allocator<Allocator> {
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

        private:
            using alloc = internal::with_optional_allocator<Allocator>;
            using page_vector = vector<pointer, allocator>;

            page_vector pages;
            size_type last_page_size;

            constexpr void clone_pages_from(const page_vector& v) {
                cloner cloner{alloc::get_allocator()};

                clear_all_pages();

                pages.reserve(v.size());

                for(const auto p : v) {
                    pages.push_back(cloner.clone(p));
                }
            }

            constexpr void clear_all_pages() {
                for(const auto p : pages) {
                    alloc::get_allocator()->deallocate(p);
                }

                pages.clear();
            }

            constexpr void update_last_page_size_from_total_size(const size_type total_size) {
                last_page_size = total_size % page_size;
            }

            constexpr pointer get_pages() { return pages; }
            constexpr const_pointer get_pages() const { return pages; }

        public:
            constexpr unordered_paged_vector() noexcept : pages{nullptr}, last_page_size{0} {}

            constexpr unordered_paged_vector(const unordered_paged_vector& other) : pages{nullptr}, last_page_size{other.last_page_size} {
                clone_pages_from(other.pages);
            }

            constexpr unordered_paged_vector(unordered_paged_vector &&other) : pages{std::move(other.pages)}, last_page_size{other.last_page_size} {}

            constexpr ~unordered_paged_vector() {
                clear_all_pages();
            }

            template<std::ranges::input_range Other>
            constexpr unordered_paged_vector& operator=(const Other& other) {
                size_type total_size = 0;

                clear_all_pages();

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
                last_page_size = other.last_page_size;

                return *this;
            }

            constexpr unordered_paged_vector& operator=(unordered_paged_vector &&other) {
                pages = std::move(other.pages);
                last_page_size = std::move(other.last_page_size);

                return *this;
            }

            constexpr size_type size() const noexcept {
                return pages.size() * page_size + last_page_size;
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
                return index / page_size;
            }

            constexpr size_type item_page(const iterator it) const {
                return item_page(it);
            }

            constexpr size_type item_page(const const_iterator it) const {
                return item_page(it);
            }

            constexpr size_type index_in_page(const size_type index) const {
                return index % page_size;
            }

            constexpr size_type index_in_page(const iterator it) const {
                return index_in_page(it.index);
            }

            constexpr size_type index_in_page(const const_iterator it) const {
                return index_in_page(it.index);
            }

            constexpr reference operator[](const size_type index) const {
                const size_type page_index = item_page(index);
                const size_type item_index = index_in_page(index);

                return get(page_index, item_index);
            }

            constexpr reference get(const size_type page_index, const size_type item_index) const {
                CCL_THROW_IF(page_index >= pages.size() || item_index >= last_page_size, std::out_of_range{"Index out of bounds."});

                return pages[page_index][item_index];
            }
    };
}

#endif // CCL_UNORDERED_PAGED_VECTOR_HPP
