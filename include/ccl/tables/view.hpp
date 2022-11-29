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
    template<
        basic_allocator Allocator,
        typename ...ColumnTypes
    > class table;

    template<typename ...Ts>
    using view_iterator = std::function<void(const Ts&...)>;

    template<typename Table, typename ...Ts>
    class view {
        const Table *table;

        public:
            explicit constexpr view(const Table& table) noexcept : table{&table} {}

            template<typename T>
            constexpr const auto& get() const noexcept {
                static_assert(is_type_in_pack<T, Ts...>(), "Type does not belong to view.");

                return table->template get<T>();
            }

            constexpr void each(const view_iterator<Ts...> iter) const {
                each<Ts...>(iter);
            }

            template<typename ...Types>
            constexpr void each(const view_iterator<Types...> iter) const {
                using first_type = first_type_t<Types...>;
                const auto& v = get<first_type>();
                const size_t v_size = v.size();

                for(size_t i = 0; i < v_size; ++i) {
                    iter(get<Types>()[i]...);
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
