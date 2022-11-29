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
#include <ccl/util.hpp>

namespace ccl {
    template<typename ...Ts>
    using view_iterator = std::function<void(const Ts&...)>;

    template<typename Table, typename ...Ts>
    class view {
        const Table *table;

        public:
            explicit constexpr view(const Table& table) noexcept : table{&table} {}

            /**
             * Get a column.
             *
             * @tparam T The column data type.
             *
             * @return The column having this data type.
             */
            template<typename T>
            constexpr const auto& get() const noexcept {
                static_assert(is_type_in_pack<T, Ts...>(), "Type does not belong to view.");

                return table->template get<T>();
            }

            /**
             * Iterate over the entire collection of items.
             *
             * @param iter The iterator function.
             */
            constexpr void each(const view_iterator<Ts...> iter) const {
                each<Ts...>(iter);
            }

            /**
             * Get the size of the collection.
             *
             * @return The number of items in a column.
             */
            constexpr size_t size() const noexcept {
                using first_type = first_type_t<Ts...>;
                return get<first_type>().size();
            }

        private:
            template<typename ...Types>
            constexpr void each(const view_iterator<Types...> iter) const {
                using first_type = first_type_t<Types...>;
                const auto& v = get<first_type>();
                const size_t v_size = v.size();

                for(size_t i = 0; i < v_size; ++i) {
                    iter(get<Types>()[i]...);
                }
            }
    };
}

#endif // CCL_TABLES_VIEW_HPP
