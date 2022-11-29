/**
 * @file
 *
 * Read-only subset of table columns.
 */
#ifndef CCL_TABLES_VIEW_HPP
#define CCL_TABLES_VIEW_HPP

#include <functional>
#include <ccl/api.hpp>
#include <ccl/vector.hpp>

namespace ccl {
    template<typename T, basic_allocator Allocator>
    class column_view {
        public:
            using value_type = T;
            using allocator_type = Allocator;
            using const_vector_type = const vector<T, allocator_type>;
            using const_vector_pointer_type = const_vector_type*;
            using const_vector_reference_type = const_vector_type&;

        private:
            const_vector_pointer_type ptr;

        protected:
            explicit constexpr column_view(const_vector_reference_type ptr) : ptr{&ptr} {}
            constexpr column_view(const column_view &) = default;
            constexpr column_view(column_view &&) = default;

            constexpr column_view& operator =(const column_view &) = default;
            constexpr column_view& operator =(column_view &&) = default;

            ~column_view() = default;

            constexpr const_vector_reference_type get() const noexcept { return *ptr; }

            constexpr decltype(auto) begin() const noexcept { return ptr->begin(); }
            constexpr decltype(auto) end() const noexcept { return ptr->end(); }
    };

    template<
        basic_allocator Allocator,
        typename ...ColumnTypes
    > class table;

    template<typename ...Ts>
    using view_iterator = std::function<void(const Ts&...)>;

    template<basic_allocator Allocator, typename ...Ts>
    class view : private column_view<Ts, Allocator>... {
        public:
            explicit constexpr view(const vector<Ts, Allocator>& ...column_collections) noexcept : column_view<Ts, Allocator>(column_collections)... {}

            template<typename T>
            constexpr const auto& get() const noexcept {
                return column_view<T, Allocator>::get();
            }

            constexpr void each(const view_iterator<Ts...> iter) const {
                each<Ts...>(iter);
            }

            template<typename First, typename ...Rest>
            constexpr void each(const view_iterator<First, Rest...> iter) const {
                const auto& v = get<First>();
                const size_t v_size = v.size();

                for(size_t i = 0; i < v_size; ++i) {
                    iter(v[i], get<Rest>()[i]...);
                }
            }

            constexpr size_t size() const noexcept {
                return size<Ts...>();
            }

        private:
            template<typename First, typename ...Rest>
            constexpr size_t size() const noexcept {
                return get<First>().size();
            }
    };
}

#endif // CCL_TABLES_VIEW_HPP
