/**
 * @file
 *
 * ECS archetype view definition.
 */
#ifndef CCL_ECS_VIEW_HPP
#define CCL_ECS_VIEW_HPP

#include <array>
#include <functional>
#include <ccl/api.hpp>
#include <ccl/debug.hpp>
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
            using archetype = ccl::ecs::archetype<Allocator>;
            using component = ccl::ecs::component<Allocator>;
            using archetype_iterator = std::function<void(const Components& ...components)>;

            static constexpr std::size_t max_archetype_count = CCL_ECS_VIEW_MAX_ARCHETYPE_COUNT;
            static constexpr std::size_t component_count = sizeof...(Components);

            using component_array = std::array<const component*, component_count>;

        private:
            std::size_t archetype_count = 0;
            std::array<const archetype*, max_archetype_count> archetypes;

            template<typename Component, typename ...Rest>
            static constexpr void set_archetype_components(
                const archetype& archetype,
                component_array& out_components,
                const uint32_t component_index = 0
            ) {
                out_components[component_index] = &archetype.template get_component<Component>();

                if constexpr(sizeof...(Rest) > 0) {
                    set_archetype_components<Rest...>(archetype, out_components, component_index + 1);
                }
            }

            template<typename TargetComponent, typename CurrentComponent, typename ...Rest>
            static constexpr auto get_component(component_array& components, const uint32_t component_index = 0) {
                if constexpr(std::is_same_v<TargetComponent, CurrentComponent>) {
                    return components[component_index];
                } else {
                    return get_component<TargetComponent, Rest...>(components, component_index + 1);
                }
            }

        public:
            constexpr void add_archetype(const archetype& arch) {
                CCL_THROW_IF(archetype_count == max_archetype_count, std::out_of_range{"Too many archetypes in view."});

                archetypes[archetype_count++] = &arch;
            }

            constexpr std::size_t size() const {
                std::size_t total_size = 0;

                for(archetype * const a : archetypes) {
                    total_size += a->size();
                }

                return total_size;
            }

            constexpr void iterate(const archetype_iterator iterator) const {
                if constexpr(component_count > 0) {
                    component_array components;

                    for(std::size_t i = 0; i < archetype_count; ++i) {
                        const archetype& current_archetype = *archetypes[i];

                        set_archetype_components<Components...>(current_archetype, components);
                        const uint32_t entity_count = components[0]->size();

                        for(uint32_t j = 0; j < entity_count; ++j) {
                            iterator(
                                get_component<Components, Components...>(components)
                                    ->template get<Components>(j)
                                ...
                            );
                        }
                    }
                }
            }
    };
}

#endif // CCL_ECS_VIEW_HPP
