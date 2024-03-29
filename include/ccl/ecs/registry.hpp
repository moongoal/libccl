/**
 * @file
 *
 * ECS registry definition.
 */
#ifndef CCL_ECS_REGISTRY_HPP
#define CCL_ECS_REGISTRY_HPP

#include <ccl/api.hpp>
#include <ccl/debug.hpp>
#include <ccl/exceptions.hpp>
#include <ccl/util.hpp>
#include <ccl/ecs/archetype.hpp>
#include <ccl/ecs/entity.hpp>
#include <ccl/ecs/view.hpp>
#include <ccl/concepts.hpp>
#include <ccl/dense-map.hpp>
#include <ccl/internal/optional-allocator.hpp>

namespace ccl::ecs {
    namespace internal {
        template<basic_allocator Allocator>
        class registry_editor;
    }

    template<basic_allocator Allocator>
    class registry : ccl::internal::with_optional_allocator<Allocator> {
        friend class internal::registry_editor<Allocator>;

        using alloc = ccl::internal::with_optional_allocator<Allocator>;

        public:
            using allocator_type = Allocator;
            using archetype = ccl::ecs::archetype<Allocator>;

            static constexpr entity_id_t max_entity_id = entity_t::underlying_type::low_part_max;

        private:
            entity_id_t current_generation;
            entity_id_t next_entity_id;
            dense_map<hash_t, archetype, hash<hash_t>, allocator_type> archetype_map;

        public:
            explicit constexpr registry(allocator_type * const allocator = nullptr)
                : alloc{allocator},
                current_generation{0},
                next_entity_id{0},
                archetype_map{allocator}
            {}

            constexpr registry(const registry& other) = delete;

            constexpr registry(registry&& other)
                : alloc{std::move(other)},
                current_generation{other.current_generation},
                next_entity_id{other.next_entity_id},
                archetype_map{std::move(other.archetype_map)}
            {}

            /**
             * Add a new entity to this registry. The returned entity is guaranteed
             * To be new, unique and of the 0th generation. Will throw an `std::out_of_range`
             * exception if the maximum allowed number of entities is reached.
             *
             * Calling this function creates an entity with 0 components. This entity will
             * never be iterated unless components are assigned to it. To make this iterable
             * without assigning any components, explicitly call `add_components()` with no
             * component types.
             *
             * @example
             *  registry.add_components<>(registry.add_entity());
             *
             * @return The newly created entity.
             */
            CCLNODISCARD entity_t add_entity() {
                CCL_THROW_IF(
                    next_entity_id >= max_entity_id,
                    std::out_of_range{"Maximum number of entities reached."}
                );

                return entity_t::make(current_generation, next_entity_id++);
            }

            template<typename ...Components>
            void add_components(
                const entity_t entity,
                Components&& ...components
            ) {
                archetype * old_arch = get_entity_archetype(entity);
                archetype * new_arch;

                #ifdef CCL_FEATURE_ECS_CHECK_ARCHETYPE_COMPONENTS
                CCL_THROW_IF(
                    old_arch && ((old_arch->template has_component<Components>()) || ...),
                    std::invalid_argument{"Attempting to add one or more already existing components."}
                );
                #endif // CCL_FEATURE_ECS_CHECK_ARCHETYPE_COMPONENTS

                const hash_t new_archetype_id = old_arch
                    ? old_arch->template extend_id<Components...>()
                    : archetype::template make_id<Components...>();

                auto new_arch_it = archetype_map.find(new_archetype_id);

                if(new_arch_it == archetype_map.end()) {
                    if(old_arch) {
                        new_arch = &archetype_map.emplace(
                            new_archetype_id,
                            archetype::make_from_template(*old_arch)
                        );
                    } else {
                        new_arch = &archetype_map.emplace(
                            new_archetype_id,
                            archetype::template make<Components...>()
                        );
                    }
                } else {
                    new_arch = new_arch_it->second;
                }

                CCL_ASSERT(new_arch);

                if(old_arch) {
                    // Adding a new archetype may trigger the vector
                    // reallocation and invalidate the pointer.
                    old_arch = get_entity_archetype(entity);
                }

                new_arch->add_entity(entity);

                if(old_arch) {
                    new_arch->copy_entity_components_from(entity, *old_arch);
                    old_arch->remove_entity(entity);
                }

                new_arch->template add_components<Components...>();
                (new_arch->set_entity_component(entity, components), ...);
            }

            archetype* get_entity_archetype(const entity_t entity) {
                const auto archetype_end = archetype_map.end_values();

                for(auto it = archetype_map.begin_values(); it != archetype_end; ++it) {
                    if(it->has_entity(entity)) {
                        return &(*it);
                    }
                }

                return nullptr;
            }

            const archetype* get_entity_archetype(const entity_t entity) const {
                const auto archetype_end = archetype_map.end_values();

                for(auto it = archetype_map.begin_values(); it != archetype_end; ++it) {
                    if(it->has_entity(entity)) {
                        return &(*it);
                    }
                }

                return nullptr;
            }

            template<typename ...Components>
            void remove_components(const entity_t entity) {
                archetype * old_arch = get_entity_archetype(entity);
                archetype * new_arch = nullptr;

                #ifdef CCL_FEATURE_ECS_CHECK_ARCHETYPE_COMPONENTS
                CCL_THROW_IF(
                    old_arch && ((!old_arch->template has_component<Components>()) || ...),
                    std::out_of_range{"One or more components missing from entity's archetype."}
                );
                #endif // CCL_FEATURE_ECS_CHECK_ARCHETYPE_COMPONENTS

                const hash_t new_archetype_id = old_arch->template extend_id<Components...>();

                auto new_arch_it = archetype_map.find(new_archetype_id);

                if(new_arch_it == archetype_map.end()) {
                    if(old_arch) {
                        new_arch = &archetype_map.emplace(
                            new_archetype_id,
                            archetype::make_from_template(*old_arch)
                        );
                    }
                } else {
                    new_arch = new_arch_it->second;
                }

                if(new_arch) {
                    new_arch->add_entity(entity);

                    if(old_arch) {
                        // Removing an archetype may trigger an item
                        // relocation and invalidate the pointer.
                        old_arch = get_entity_archetype(entity);
                        new_arch->copy_entity_components_from(entity, *old_arch);
                        old_arch->remove_entity(entity);
                    }
                }

            }

            template<typename ...Components>
            constexpr const ccl::ecs::view<allocator_type, Components...> view() const {
                const auto archetype_map_end = archetype_map.end_values();
                ccl::ecs::view<allocator_type, Components...> view_object;

                for(auto it = archetype_map.begin_values(); it != archetype_map_end; ++it) {
                    const bool has_all_components = (it->template has_component<Components>() && ...);

                    if(has_all_components) {
                        view_object.add_archetype(*it);
                    }
                }

                return view_object;
            }

            /**
             * Remove all entities and advance to the next generation.
             */
            void clear() {
                archetype_map.clear();

                current_generation = choose(current_generation + 1, 0U, current_generation < max_entity_id);
                next_entity_id = 0;
            }

            /**
             * Check whether an entity is present in this registry.
             *
             * @param entity The entity to check for presence.
             *
             * @return True if the entity is present in this registry, false if not.
             */
            constexpr bool has_entity(const entity_t entity) const {
                const auto archetype_end = archetype_map.end_values();

                for(auto it = archetype_map.begin_values(); it != archetype_end; ++it) {
                    if(it->has_entity(entity)) {
                        return true;
                    }
                }

                return false;
            }

            /**
             * Remove an entity, if present.
             *
             * @param entity The entity to remove.
             */
            void remove_entity(const entity_t entity) {
                archetype * const arch = get_entity_archetype(entity);

                if(arch) {
                    arch->remove_entity(entity);
                }
            }

            /**
             * Remove an entity. If the entity is not present within this
             * registry, calling this function will cause undefined behaviour.
             *
             * This is faster than `remove_entity()` as this always assumes
             * the entity is present, thus avoiding one condition.
             *
             * @param entity The entity to remove.
             */
            void unsafe_remove_entity(const entity_t entity) {
                #ifdef CCL_FEATURE_ECS_CHECK_UNSAFE_REMOVE_ENTITY
                CCL_ASSERT(has_entity(entity));
                #endif // CCL_FEATURE_ECS_CHECK_UNSAFE_REMOVE_ENTITY
                get_entity_archetype(entity)->remove_entity(entity);
            }

            /**
             * Tell whether a given entity has all the given components.
             *
             * @tparam Components The components to test.
             *
             * @param entity The entity to test.
             *
             * @return True if the given entity has all the given components, false if not.
             */
            template<typename ...Components>
            CCLNODISCARD bool has_components(const entity_t entity) const {
                const archetype * const arch = get_entity_archetype(entity);

                if(arch) {
                    return (arch->template has_component<Components>() && ...);
                }

                return false;
            }

            /**
             * Tell whether a given entity has any of the given components.
             *
             * @tparam Components The components to test.
             *
             * @param entity The entity to test.
             *
             * @return True if the given entity has any of the given components, false if not.
             */
            template<typename ...Components>
            CCLNODISCARD bool has_any_components(const entity_t entity) const {
                const archetype * const arch = get_entity_archetype(entity);

                if(arch) {
                    return (arch->template has_component<Components>() || ...);
                }

                return false;
            }

            /**
             * Get the component value of an entity.
             *
             * @tparam Component The component type.
             *
             * @param entity The entity.
             *
             * @return The component value for this entity.
             */
            template<typename Component>
            CCLNODISCARD constexpr Component& get_entity_component(const entity_t entity) {
                archetype * const arch = get_entity_archetype(entity);

                CCL_THROW_IF(!arch, std::out_of_range{"Entity not present in registry."});

                return arch->template get_entity_component<Component>(entity);
            }

            /**
             * Get the component value of an entity.
             *
             * @tparam Component The component type.
             *
             * @param entity The entity.
             *
             * @return The component value for this entity.
             */
            template<typename Component>
            CCLNODISCARD constexpr const Component& get_entity_component(const entity_t entity) const {
                const archetype * const arch = get_entity_archetype(entity);

                CCL_THROW_IF(!arch, std::out_of_range{"Entity not present in registry."});

                return arch->template get_entity_component<Component>(entity);
            }
    };
}
#endif // CCL_ECS_REGISTRY_HPP
