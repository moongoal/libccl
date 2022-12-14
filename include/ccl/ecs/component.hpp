/**
 * @file
 *
 * Type-erased ECS component definition.
 */
#ifndef CCL_ECS_COMPONENT_HPP
#define CCL_ECS_COMPONENT_HPP

#include <ccl/api.hpp>
#include <ccl/concepts.hpp>
#include <ccl/paged-vector.hpp>

namespace ccl::ecs {
    template<typename T, typed_allocator<T> Allocator>
    class component;

    struct generic_component {
        virtual ~generic_component() = default;

        virtual size_t size() const = 0;

        template<typename T, typed_allocator<T> Allocator>
        constexpr component<T, Allocator>& cast() {
            return *

            #ifdef CCL_FEATURE_TYPECHECK_CASTS
                dynamic_cast
            #else // CCL_FEATURE_TYPECHECK_CASTS
                reinterpret_cast
            #endif // CCL_FEATURE_TYPECHECK_CASTS

            <component<T, Allocator>*>(this);
        }

        template<typename T, typed_allocator<T> Allocator>
        constexpr const component<T, Allocator>& cast() const {
            return *

            #ifdef CCL_FEATURE_TYPECHECK_CASTS
                dynamic_cast
            #else // CCL_FEATURE_TYPECHECK_CASTS
                reinterpret_cast
            #endif // CCL_FEATURE_TYPECHECK_CASTS

            <const component<T, Allocator>*>(this);
        }
    };

    template<typename T, typed_allocator<T> Allocator>
    class component : public generic_component {
        public:
            using value_type = T;
            using allocator_type = Allocator;
            using item_collection = paged_vector<T, T*, Allocator>;

        private:
            item_collection items;

        public:
            constexpr auto& get() { return items; }
            constexpr const auto& get() const { return items; }

            virtual size_t size() const override {
                return items.size();
            }
    };
}

#endif // CCL_ECS_COMPONENT_HPP
