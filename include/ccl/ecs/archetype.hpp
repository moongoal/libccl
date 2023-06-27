/**
 * @file
 *
 * ECS Archetype definition.
 */
#ifndef CCL_ARCHETYPE_HPP
#define CCL_ARCHETYPE_HPP

#include <typeinfo>
#include <memory>
#include <algorithm>
#include <ccl/api.hpp>
#include <ccl/concepts.hpp>
#include <ccl/hashtable.hpp>
#include <ccl/dense-map.hpp>
#include <ccl/ecs/entity.hpp>
#include <ccl/ecs/component.hpp>
#include <ccl/either.hpp>
#include <ccl/internal/optional-allocator.hpp>

namespace ccl::ecs {
    template<basic_allocator Allocator>
    class archetype : internal::with_optional_allocator<Allocator> {
        public:
            using entity_type = entity_t;
            using entity_id = entity_id_t;
            using allocator_type = Allocator;
            using size_type = uint32_t;
            using entity_index_collection = dense_map<entity_type, size_type, allocator_type>;
            using component = ccl::ecs::component<allocator_type>;
            using component_collection = hashtable<std::size_t, component, hash<std::size_t>, allocator_type>;

            static constexpr hash_t invalid_id = ~static_cast<hash_t>(0);

        private:
            using alloc = internal::with_optional_allocator<Allocator>;

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
                    component::template make_id<Components>(),
                    component::template make<Components>(alloc::get_allocator())
                ), ...);
            }

        public:
            constexpr archetype(
                const hash_t id = invalid_id,
                allocator_type * const allocator = nullptr
            ) : alloc{allocator},
                archetype_id{id},
                entity_index_map{allocator},
                components{allocator}
            {}

            constexpr archetype(const archetype& other) = delete;

            constexpr archetype(archetype&& other)
                : alloc{std::move(other)},
                archetype_id{other.archetype_id},
                entity_index_map{std::move(other.entity_index_map)},
                components{std::move(other.components)} {
                    other.archetype_id = invalid_id;
            }

            constexpr archetype& operator=(const archetype&) = delete;

            constexpr archetype& operator=(archetype&& other) {
                alloc::operator =(std::move(other));

                archetype_id = other.archetype_id;
                entity_index_map = std::move(other.entity_index_map);
                components = std::move(other.components);

                other.archetype_id = invalid_id;

                return *this;
            }

            /**
             * Create a new archetype from its component type set.
             *
             * @tparam Components Component types part of the archetype.
             *  A component of type `archetype::entity_type` is always
             *  added automatically to every archetype to track entity IDs.
             */
            template<typename ...Components>
            static archetype make(allocator_type * const allocator = nullptr) {
                const hash_t id = make_id<Components...>();
                archetype arch{id, allocator};

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
                const std::size_t component_id = component::template make_id<T>();

                return components.contains(component_id);
            }

            /**
             * Get an existing component collection.
             *
             * @param component_id The component type ID.
             *
             * @return The component collection.
             */
            constexpr const component& get_component_by_id(const std::size_t component_id) const {
                const auto component_it = components.find(component_id);

                CCL_THROW_IF(component_it == components.end(), std::out_of_range{"Component not present in archetype."});

                return *component_it->second;
            }

            /**
             * Get an existing component collection.
             *
             * @param component_id The component type ID.
             *
             * @return The component collection.
             */
            constexpr component& get_component_by_id(const std::size_t component_id) {
                const auto component_it = components.find(component_id);

                CCL_THROW_IF(component_it == components.end(), std::out_of_range{"Component not present in archetype."});

                return *component_it->second;
            }

            /**
             * Get a component collection.
             *
             * @param component_id The component type ID.
             *
             * @return The optional component collection, or nullptr.
             */
            constexpr component* get_optional_component_by_id(const std::size_t component_id) {
                const auto component_it = components.find(component_id);

                if(component_it != components.end()) {
                    return component_it->second;
                }

                return nullptr;
            }

            /**
             * Get a component collection.
             *
             * @param component_id The component type hash.
             *
             * @return The optional component collection, or nullptr.
             */
            constexpr const component* get_optional_component_by_id(const std::size_t component_id) const {
                const auto component_it = components.find(component_id);

                if(component_it != components.end()) {
                    return component_it->second;
                }

                return nullptr;
            }

            /**
             * Get a component collection.
             *
             * @tparam T The component value type.
             *
             * @return The optional component collection or nullptr if not present.
             */
            template<typename T>
            constexpr component* get_optional_component() {
                const std::size_t component_id = component::template make_id<T>();

                return get_optional_component_by_id(component_id);
            }

            /**
             * Get a component collection.
             *
             * @tparam T The component value type.
             *
             * @return The optional component collection or nullptr if not present.
             */
            template<typename T>
            constexpr const component* get_optional_component() const {
                const std::size_t component_id = component::template make_id<T>();
                auto * const component = get_optional_component_by_id(component_id);

                return get_optional_component_by_id(component_id);
            }

            /**
             * Get an existing component collection.
             *
             * @tparam T The component type.
             *
             * @return The component.
             */
            template<typename T>
            constexpr const component& get_component() const {
                const std::size_t component_id = component::template make_id<T>();

                return get_component_by_id(component_id);
            }

            /**
             * Get an existing component collection.
             *
             * @tparam T The component type.
             *
             * @return The component.
             */
            template<typename T>
            constexpr component& get_component() {
                const std::size_t component_id = component::template make_id<T>();

                return get_component_by_id(component_id);
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
            constexpr const T& get_entity_component(const entity_type e) const {
                const component& component = get_component<T>();
                const auto entity_it = entity_index_map.find(e);

                CCL_THROW_IF(entity_it == entity_index_map.end(), std::out_of_range{"Entity not present in archetype."});

                const auto entity_index = *entity_it->second;

                return component.template get<T>(entity_index);
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
            constexpr T& get_entity_component(const entity_type e) {
                component& component = get_component<T>();
                const auto entity_it = entity_index_map.find(e);

                CCL_THROW_IF(entity_it == entity_index_map.end(), std::out_of_range{"Entity not present in archetype."});

                const auto entity_index = *entity_it->second;

                return component.template get<T>(entity_index);
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
                return (component::template make_id<Components>() ^...);
            }

            template<>
            static constexpr hash_t make_id() {
                return 0;
            }

            /**
             * Extend an archetype ID with new components.
             * This function can also be used to subtract components from the ID, by
             * providing existing components as `Components`.
             *
             * @tparam Components The component types.
             *
             * @return The archetype ID of the current archetype's component extended with the
             *  provided ones.
             */
            template<typename ...Components>
            constexpr hash_t extend_id() {
                return archetype_id ^ (component::template make_id<Components>() ^...);
            }

            template<>
            constexpr hash_t extend_id() {
                return archetype_id;
            }

            /**
             * Add a new entity to the archetype.
             *
             * @param entity The entity handle.
             *
             * @return The entity index within this archetype.
             */
            constexpr size_type add_entity(const entity_type entity) {
                component& entity_component = get_component<entity_t>();
                const size_type entity_index = entity_component.size();
                const auto entity_component_id = entity_component.id();

                entity_component.push_back(entity);

                for(const auto& pair : components) {
                    if(*pair.first != entity_component_id) {
                        pair.second->emplace_empty();
                    }
                }

                entity_index_map.insert(entity, entity_index);

                return entity_index;
            }

            /**
             * Copy the common components for a given entity from another archetype.
             *
             * @param entity The entity.
             * @param source The source component.
             */
            constexpr void copy_entity_components_from(const entity_type entity, const archetype<Allocator>& source) {
                const size_type index_to = entity_index_map.at(entity);

                for(const auto& c : source.components) {
                    const std::size_t source_component_id = *c.first;
                    const component * const source_component = c.second;
                    component * const dest_component = get_optional_component_by_id(source_component_id);

                    if(dest_component) {
                        const size_type index_from = source.entity_index_map.at(entity);

                        dest_component->move_from(*source_component, index_from, index_to);
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
                size_type &entity_index = *entity_index_it->second;

                auto last_index_it = entity_index_map.end_values() - 1;

                if(*last_index_it == entity_index) { // Removing last element
                    --last_index_it;

                    for(const auto& c : components) {
                        component * const component = c.second;

                        component->erase(entity_index);
                    }
                } else { // Removing not last element
                    size_type& source_entity_index = *last_index_it;
                    const entity_type source_entity = get_component<entity_type>()
                        .template get<entity_type>(source_entity_index);

                    // Keep the array compact by swapping with last item
                    // and then removing it.
                    for(const auto& c : components) {
                        component * const component = c.second;

                        component->move(source_entity_index, entity_index);
                        component->erase(source_entity_index);
                    }

                    source_entity_index = entity_index;
                }

                entity_index_map.erase(entity_index_it);
            }

            /**
             * Create an empty archetype with the same components as `tmpl`.
             *
             * @param tmpl The template archetype.
             *
             * @return The new empty archetype.
             */
            static constexpr archetype make_from_template(
                const archetype& tmpl,
                allocator_type * const allocator = nullptr
            ) {
                archetype arch{tmpl.id()};

                for(const auto& pair : tmpl.components) {
                    arch.components.emplace(
                        *pair.first,
                        pair.second->clone_empty(allocator)
                    );
                }

                return arch;
            }

            template<typename ...Ts>
            constexpr void add_components() {
                const auto first = components.begin();

                // This is always viable as there is at least
                // an entity_t component
                const std::size_t size = first->second->size();

                (components.emplace(
                    component::template make_id<Ts>(),
                    component::template make<Ts>(alloc::get_allocator())
                ).template resize<Ts>(size), ...);

                archetype_id = extend_id<Ts...>();
            }

            constexpr std::size_t size() const {
                return get_component<entity_t>().size();
            }
    };
}

#endif // CCL_ARCHETYPE_HPP
