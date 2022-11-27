/**
 * @file
 *
 * SoA-based table implementation.
 */
#ifndef CCL_TABLE_HPP
#define CCL_TABLE_HPP

#include <type_traits>
#include <functional>
#include <utility>
#include <ccl/api.hpp>
#include <ccl/debug.hpp>
#include <ccl/allocator.hpp>
#include <ccl/vector.hpp>

namespace ccl {
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
            constexpr auto emplace(Args&& ...args) {
                return get().emplace(std::forward<Args>(args)...);
            }

        private:
            value_collection_type data;
    };

    template<typename T, typename Allocator>
    struct emplacer {
        using value_type = T;
        using column_type = column<T, Allocator>;
        using column_reference_type = column<T, Allocator>&;
        using function_type = std::function<void(column_reference_type)>;

        private:
            function_type function;

        public:
            template<typename F>
            emplacer(F&& f) : function{f} {}

            constexpr decltype(auto) operator ()(column_reference_type c) {
                return function(c);
            }
    };

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

            template<typename T>
            constexpr column_collection_type<T> get() noexcept {
                return column_type<T>::get();
            }

            template<typename T>
            constexpr const_column_collection_type<T> get() const noexcept {
                return column_type<T>::get();
            }

            constexpr void emplace_row(emplacer<ColumnTypes, allocator_type>&& ...emplacers) {
                (emplacers(*this),...);
            }

            constexpr void reserve(const size_type new_capacity) {
                (get<ColumnTypes>().reserve(new_capacity),...);
            }
    };
}

#endif // CCL_TABLE_HPP
