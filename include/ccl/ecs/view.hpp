/**
 * @file
 *
 * ECS archetype view definition.
 */
#ifndef CCL_ECS_VIEW_HPP
#define CCL_ECS_VIEW_HPP

#include <array>
#include <ccl/api.hpp>
#include <ccl/ecs/component.hpp>
#include <ccl/ecs/archetype.hpp>

namespace ccl::ecs {
    /**
     * Read-only access to a subset of components of an archetype
     * for iterating.
     *
     * @tparam Allocator The allocator class of the archetype.
     * @tparam Components The components to add to the view.
     */
    template<typename Allocator, typename ...Components>
    class view {
        public:
            using archetype_type = archetype<Allocator>;

            static constexpr size_t component_count = sizeof...(Components);

        private:
            size_t index = 0; // Ref iteration index
            std::array<generic_component*, component_count> refs;

            template<typename CurrentComponent, typename ...RestComponents>
            static void set_view_components(view& v, const archetype_type& arch, const size_t index = 0) {
                v.refs[index] = arch.template get_component<CurrentComponent>();

                if constexpr(sizeof...(RestComponents)) {
                    set_view_components<RestComponents...>(v, arch, index + 1);
                }
            }

            template<typename T, typename CurrentComponent, typename ...RestComponents>
            const generic_component* get_generic_component(const size_t i = 0) const {
                if constexpr(std::is_same_v<T, CurrentComponent>) {
                    return refs[i];
                }

                static_assert(sizeof...(RestComponents) > 0, "Component not found.");

                return get_generic_component<T, RestComponents...>(i + 1);
            }

        public:
            static view create(const archetype_type& arch) {
                view v;

                set_view_components<Components...>(v, arch);

                return v;
            }

            /**
             * Access the current element from a given component.
             *
             * @tparam T The component type to access.
             *
             * @return A reference to the requested component for the current item.
             */
            template<typename T>
            constexpr const typename component<T, Allocator>::value_type& get() const {
                const generic_component * const g = get_generic_component<T, Components...>();

                return g->template cast<T, Allocator>()[index];
            }

            /**
             * Advance to the next entity in the view.
             */
            constexpr void next() {
                index += 1;
            }

            /**
             * Reset the view.
             */
            constexpr void reset() {
                index = 0;
            }

            constexpr size_t size() const {
                return refs[0]->size();
            }
    };
}

#endif // CCL_ECS_VIEW_HPP