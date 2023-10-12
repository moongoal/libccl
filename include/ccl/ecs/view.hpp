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
#include <ccl/type-traits.hpp>
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
            using archetype_paged_iterator = std::function<void(const typename component::template item_collection<Components>& ...components)>;

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
            /**
             * Add a new archetype to this view.
             *
             * @param arch The archetype to add.
             */
            constexpr void add_archetype(const archetype& arch) {
                CCL_THROW_IF(archetype_count == max_archetype_count, std::out_of_range{"Too many archetypes in view."});

                #ifdef CCL_FEATURE_ECS_CHECK_ARCHETYPE_COMPONENTS
                CCL_ASSERT(arch.template has_component<Components>() && ...);
                #endif // CCL_FEATURE_ECS_CHECK_ARCHETYPE_COMPONENTS

                archetypes[archetype_count++] = &arch;
            }

            /**
             * Compute the size of this view as number of entities to
             * iterate.
             *
             * @return The number of entities that will be iterate by this view.
             */
            constexpr std::size_t size() const {
                std::size_t total_size = 0;

                for(std::size_t i = 0; i < archetype_count; ++i) {
                    const archetype * const a = archetypes[i];
                    total_size += a->size();
                }

                return total_size;
            }

            /**
             * Iterate this view. The given iterator function will have
             * the same types of the view in the same order and will
             * accept constant references (or values) to items of the
             * specified types in the same order as arguments.
             *
             * @example
             *  const auto my_view = registry.view<int, float>();
             *  view.iterate([] (const int, const float) { ... });
             *
             * @param iterator The iterator function.
             */
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

            /**
             * Iterate this view. The given iterator function will have
             * accept references to the underlying archetype components'
             * data and will be called once per matching archetype.
             *
             * @example
             *  const auto my_view = registry.view<int, float>();
             *  view.iterate_archetypes([] (const auto& int_vec, const auto& float_vec) { ... });
             *
             * @param iterator The iterator function.
             */
            constexpr void iterate_archetypes(const archetype_paged_iterator iterator) const {
                if constexpr(component_count > 0) {
                    component_array components;

                    for(std::size_t i = 0; i < archetype_count; ++i) {
                        const archetype& current_archetype = *archetypes[i];

                        set_archetype_components<Components...>(current_archetype, components);

                        iterator(
                            get_component<Components, Components...>(components)
                                ->template get<Components>()
                            ...
                        );
                    }
                }
            }
    };
}

#endif // CCL_ECS_VIEW_HPP
