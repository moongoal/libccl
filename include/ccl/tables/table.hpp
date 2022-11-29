/**
 * @file
 *
 * SoA-based table implementation.
 */
#ifndef CCL_TABLES_TABLE_HPP
#define CCL_TABLES_TABLE_HPP

#include <functional>
#include <utility>
#include <ccl/api.hpp>
#include <ccl/debug.hpp>
#include <ccl/allocator.hpp>
#include <ccl/vector.hpp>
#include <ccl/tables/view.hpp>

namespace ccl {
    /**
     * A single column of data.
     *
     * This is represented by a linear, contiguous collection of items of a specified type.
     *
     * @tparam T The value type of this column's data.
     * @tparam Allocator The allocator used for the underlying collection memory management.
     */
    template<
        typename T,
        typed_allocator<T> Allocator = allocator
    >
    class column {
        public:
            using value_type = T;
            using allocator_type = Allocator;
            using value_collection_type = vector<value_type, allocator_type>;
            using reference_collection_type = vector<value_type, allocator_type>&;
            using const_reference_collection_type = const vector<value_type, allocator_type>&;

            constexpr reference_collection_type get() noexcept { return data; }
            constexpr const_reference_collection_type get() const noexcept { return data; }

            template<typename ...Args>
            constexpr decltype(auto) emplace(Args&& ...args) {
                return get().emplace(std::forward<Args>(args)...);
            }

            constexpr typename value_collection_type::reference operator[](const typename value_collection_type::size_type index) {
                return data[index];
            }

            constexpr typename value_collection_type::const_reference operator[](const typename value_collection_type::size_type index) const {
                return data[index];
            }

        private:
            value_collection_type data;
    };

    /**
     * A functor to operate on a column.
     *
     * This is effectively a wrapper around a std::function object to expose
     * extra type information.
     */
    template<typename T, typename Allocator>
    struct op_functor {
        using value_type = T;
        using column_type = column<T, Allocator>;
        using column_reference_type = column<T, Allocator>&;
        using function_type = std::function<void(column_reference_type)>;

        private:
            function_type function;

        public:
            template<typename F>
            op_functor(F&& f) : function{f} {}

            constexpr decltype(auto) operator ()(column_reference_type c) {
                return function(c);
            }
    };

    /**
     * A SoA table. Columns are specified at compile time and are operated upon
     * always together. Querying can instead be done on a subset of columns.
     *
     * This is a low level structure and it's the user's responsibility to ensure
     * the columns are always of the same size and the data of each row is stored
     * at the correct indices.
     *
     * @tparam Allocator The allocator type for the table's data.
     * @tparam ColumnTypes The data type for each column. Each type must be different.
     */
    template<
        basic_allocator Allocator,
        typename ...ColumnTypes
    > class table : private column<ColumnTypes, Allocator>... {
        public:
            using allocator_type = Allocator;
            using size_type = size_t;

            template<typename T>
            using column_type = column<T, allocator_type>;

            template<typename T>
            using column_collection_type = typename column_type<T>::reference_collection_type;

            template<typename T>
            using const_column_collection_type = typename column_type<T>::const_reference_collection_type;

            /**
             * Get a column.
             *
             * @tparam T The column data type.
             *
             * @return The column having this data type.
             */
            template<typename T>
            constexpr column_collection_type<T> get() noexcept {
                return column_type<T>::get();
            }

            /**
             * Get a column.
             *
             * @tparam T The column data type.
             *
             * @return The column having this data type.
             */
            template<typename T>
            constexpr const_column_collection_type<T> get() const noexcept {
                return column_type<T>::get();
            }

            /**
             * Apply an operation to each column.
             *
             * @param ops The operation functors, one per column.
             */
            constexpr void apply(op_functor<ColumnTypes, allocator_type>&& ...ops) {
                (ops(*this),...);
            }

            /**
             * Apply an operation to some columns.
             *
             * @param ops The operation functors, one per column.
             */
            template<typename ...Ts>
            constexpr void apply(op_functor<Ts, allocator_type>&& ...ops) {
                (ops(*this),...);
            }

            /**
             * Apply an operation to a single column.
             *
             * @param op The operation functor.
             */
            template<typename T>
            constexpr void apply_one(op_functor<T, allocator_type> op) {
                op(*this);
            }

            /**
             * Reserve room for rows.
             *
             * @param new_capacity The new capacity, as rows.
             */
            constexpr void reserve(const size_type new_capacity) {
                (get<ColumnTypes>().reserve(new_capacity),...);
            }

            /**
             * Emplace a new row, default-constructing all columns.
             *
             * @return The index of the new row.
             */
            constexpr size_type emplace() {
                (apply_one<ColumnTypes>([] (auto &x) { x.emplace(); }),...);

                return get<first_type_t<ColumnTypes...>>().size() - 1;
            }

            /**
             * Create a view of the entire table.
             *
             * @return A new view object.
             */
            constexpr ccl::view<table, ColumnTypes...> view() const {
                return ccl::view<table, ColumnTypes...>{
                    *this
                };
            }

            /**
             * Create a view of the entire table.
             *
             * @tparam Ts The column value types to create the view for.
             *
             * @return A new view object.
             */
            template<typename ...Ts>
            constexpr ccl::view<table, Ts...> view() const {
                return ccl::view<table, Ts...>{
                    *this
                };
            }

            constexpr void each(const view_iterator<ColumnTypes...> iter) const {
                each<ColumnTypes...>(iter);
            }

            template<typename First, typename ...Rest>
            constexpr void each(const view_iterator<First, Rest...> iter) const {
                const auto& v = get<First>();
                const size_t v_size = v.size();

                for(size_t i = 0; i < v_size; ++i) {
                    iter(v[i], get<Rest>()[i]...);
                }
            }
    };
}

#endif // CCL_TABLES_TABLE_HPP
