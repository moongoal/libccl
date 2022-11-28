/**
 * @file
 *
 * Read-only subset of table columns.
 */
#ifndef CCL_TABLES_VIEW_HPP
#define CCL_TABLES_VIEW_HPP

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

    template<basic_allocator Allocator, typename ...Ts>
    class view : private column_view<Ts, Allocator>... {
        public:
            explicit constexpr view(const vector<Ts, Allocator>& ...column_collections) noexcept : column_view<Ts, Allocator>(column_collections)... {}
    };
}

#endif // CCL_TABLES_VIEW_HPP
