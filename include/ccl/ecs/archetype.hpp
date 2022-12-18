/**
 * @file
 *
 * ECS Archetype definition.
 */
#ifndef CCL_ARCHETYPE_HPP
#define CCL_ARCHETYPE_HPP

#include <typeinfo>
#include <memory>
#include <optional>
#include <ccl/api.hpp>
#include <ccl/concepts.hpp>
#include <ccl/hashtable.hpp>
#include <ccl/dense-map.hpp>
#include <ccl/ecs/entity.hpp>
#include <ccl/ecs/component.hpp>

namespace ccl::ecs {
    template<basic_allocator Allocator>
    class archetype {
        public:
            using entity_type = entity_t;
            using entity_id = entity_id_t;
            using allocator_type = Allocator;
            using size_type = uint32_t;
            using entity_index_collection = dense_map<entity_type, size_type, allocator_type>;
            using component_pointer = std::unique_ptr<component_i>;
            using component_collection = hashtable<std::size_t, component_pointer, hash<std::size_t>, allocator_type>;

        private:
            /**
             * The ID of the archetype, computed by aggregating
             * its component IDs.
             */
            hash_t archetype_id;

            /**
             * Every entity in this archetype is listed here
             * and its associated value is an index into every
             * component's collection for that entity.
             */
            entity_index_collection entity_index_map;

            /**
             * Collection of components for this archetype.
             */
            component_collection components;

            /**
             * Set the components of this archetype up.
             *
             * @tparam The component parameter types.
             */
            template<typename ...Components>
            constexpr void setup_components() {
                (components.emplace(
                    component<Components, Allocator>::make_id(),
                    std::make_unique<component<Components, Allocator>>()
                ), ...);
            }

            constexpr archetype(hash_t id) : archetype_id{id} {}

        public:
            constexpr archetype(const archetype& other) = delete;
            constexpr archetype(archetype&& other) = default;

            /**
             * Create a new archetype from its component type set.
             *
             * @tparam Components Component types part of the archetype.
             *  A component of type `archetype::entity_type` is always
             *  added automatically to every archetype to track entity IDs.
             */
            template<typename ...Components>
            static archetype make() {
                const hash_t id = make_id<Components...>();
                archetype arch{id};

                arch.setup_components<entity_type, Components...>();

                return arch;
            }

            /**
             * @return The archetype ID.
             */
            constexpr hash_t id() const noexcept { return archetype_id; }

            /**
             * Check whether an entity is present in this archetype.
             *
             * @param e The entity.
             *
             * @return True if the entity is located in this archetype, false if not.
             */
            constexpr bool has_entity(const entity_type e) const {
                return entity_index_map.contains(e);
            }

            /**
             * Check whether the archetype contains a given component.
             *
             * @tparam T The component type.
             *
             * @return True if the component is present in the archetype, false if not.
             */
            template<typename T>
            constexpr bool has_component() const {
                const std::size_t component_hash = component<T, Allocator>::make_id();

                return components.contains(component_hash);
            }

            /**
             * Get an existing component collection.
             *
             * @param component_hash The component type hash.
             *
             * @return The component collection.
             */
            constexpr const component_i* get_component(const std::size_t component_hash) const {
                const auto component_it = components.find(component_hash);

                CCL_THROW_IF(component_it == components.end(), std::out_of_range{"Component not present in archetype."});

                return &*component_it;
            }

            /**
             * Get a component collection.
             *
             * @param component_hash The component type hash.
             *
             * @return The optional component collection.
             */
            constexpr const std::optional<const component_i*> get_optional_component(const std::size_t component_hash) const {
                const auto component_it = components.find(component_hash);

                if(component_it == components.end()) {
                    return {};
                }

                return &*component_it;
            }

            /**
             * Get a component collection.
             *
             * @param component_hash The component type hash.
             *
             * @return The optional component collection.
             */
            constexpr std::optional<component_i*> get_optional_component(const std::size_t component_hash) {
                const auto component_it = components.find(component_hash);

                if(component_it == components.end()) {
                    return {};
                }

                return &*component_it;
            }

            /**
             * Get an existing component collection.
             *
             * @param component_hash The component type hash.
             *
             * @return The component collection.
             */
            constexpr component_i* get_component(const std::size_t component_hash) {
                const auto component_it = components.find(component_hash);

                CCL_THROW_IF(component_it == components.end(), std::out_of_range{"Component not present in archetype."});

                return component_it->second()->get();
            }

            /**
             * Get an existing component collection.
             *
             * @tparam T The component type.
             *
             * @return The component.
             */
            template<typename T>
            constexpr const component<T, Allocator>& get_component() const {
                const std::size_t component_hash = component<T, Allocator>::make_id();

                return get_component(component_hash)->template cast<T, Allocator>();
            }

            /**
             * Get an existing component collection.
             *
             * @tparam T The component type.
             *
             * @return The component.
             */
            template<typename T>
            constexpr component_i* get_component() {
                const std::size_t component_hash = component<T, Allocator>::make_id();

                return get_component(component_hash);
            }

            /**
             * Get an existing component for an existing entity.
             *
             * @tparam T The component type.
             *
             * @param e The entity.
             *
             * @return A const reference to the entity's component.
             */
            template<typename T>
            constexpr const component<T, Allocator>& get_entity_component(const entity_type e) const {
                const std::size_t component_hash = component<T, Allocator>::make_id();
                const auto component_it = components.find(component_hash);
                const auto entity_it = entity_index_map.find(e);

                CCL_THROW_IF(component_it == components.end(), std::out_of_range{"Component not present in archetype."});
                CCL_THROW_IF(entity_it == entity_it.end(), std::out_of_range{"Entity not present in archetype."});

                const auto entity_component = component_it->operator[](*entity_it);

                return entity_component.template cast<T, Allocator>();
            }

            /**
             * Set an existing component for an existing entity.
             *
             * @tparam T The component type.
             *
             * @param e The entity.
             * @param value The new component value.
             */
            template<typename T>
            constexpr void set_entity_component(const entity_type e, const T& value) {
                get_entity_component<T>(e) = value;
            }

            /**
             * Compute an archetype ID.
             *
             * @tparam Components The component types.
             * @param components The sequence of component types.
             *
             * @return The computed archetype ID.
             */
            template<typename ...Components>
            static constexpr hash_t make_id() {
                return (typeid(component<Components, Allocator>).hash_code() ^...);
            }

            /**
             * Extend an archetype ID with new components.
             *
             * @tparam Components The component types.
             *
             * @return The archetype ID of the current archetype's component extended with the
             *  provided ones.
             */
            template<typename ...Components>
            constexpr hash_t extend_id() {
                return archetype_id ^ (typeid(component<Components, Allocator>).hash_code() ^...);
            }

            /**
             * Add a new entity to the archetype.
             *
             * @param entity The entity handle.
             *
             * @return The entity index within this archetype.
             */
            constexpr size_type add_entity(const entity_type entity) {
                component<entity_t, Allocator>& entity_component = get_component<entity_t>()->template cast<entity_t, Allocator>();
                const size_type entity_index = entity_component.size();
                const auto entity_component_id = component<entity_t, Allocator>::make_id();

                entity_component.push_back(entity);

                for(const auto& pair : components) {
                    if(*pair.first() != entity_component_id) {
                        (*pair.second())->emplace_empty();
                    }
                }

                return entity_index;
            }

            /**
             * Copy the common components for a given entity from another archetype.
             *
             * @param entity The entity.
             * @param source The source component.
             */
            constexpr void copy_entity_components_from(const entity_type entity, const archetype<Allocator>& source) {
                const size_type index_to = entity_index_map[entity];

                for(const auto& c : source.components) {
                    const std::size_t source_component_id = *c.first();
                    const component_i * const source_component = *c.second();
                    std::optional<component_i*> dest_component = get_optional_component(source_component_id);

                    if(dest_component.has_value()) {
                        const size_type index_from = source.entity_index_map[entity];

                        dest_component.value()->move_from(*source_component, index_from, index_to);
                    }
                }
            }

            /**
             * Remove an entity and all of its associated components.
             *
             * @param entity The entity.
             */
            constexpr void remove_entity(const entity_type entity) {
                auto entity_index_it = entity_index_map.find(entity);
                const size_type entity_index = *entity_index_it;

                auto last_index_it = entity_index_map.end_values() - 1;

                if(*last_index_it == entity_index) { // Removing last element
                    --last_index_it;

                    for(const auto& c : components) {
                        component_i * const component = *c.second();

                        component->erase(entity_index);
                    }
                } else { // Removing not last element
                    const size_type source_entity_index = *last_index_it;
                    const entity_type source_entity = get_component<entity_type>().get()[source_entity_index];

                    // Keep the array compact by swapping with last item
                    // and then removing it.
                    for(const auto& c : components) {
                        component_i * const component = *c.second();

                        component->move(source_entity_index, entity_index);
                        component->erase(source_entity_index);
                    }

                    entity_index_map[source_entity] = entity_index;
                }

                entity_index_map.remove(entity_index_it);
            }
    };
}

#endif // CCL_ARCHETYPE_HPP
