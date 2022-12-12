/**
 * @file
 *
 * ECS Archetype definition.
 */
#ifndef CCL_ARCHETYPE_HPP
#define CCL_ARCHETYPE_HPP

#include <span>
#include <typeinfo>
#include <ccl/api.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/concepts.hpp>
#include <ccl/ecs/entity.hpp>
#include <ccl/hashtable.hpp>
#include <ccl/dense-map.hpp>
#include <ccl/vector.hpp>
#include <ccl/paged-vector.hpp>

namespace ccl::ecs {
    template<basic_allocator Allocator>
    class archetype {
        public:
            using entity_type = entity_t;
            using entity_id = entity_id_t;
            using allocator_type = Allocator;
            using size_type = uint32_t;
            using entity_index_collection = hashtable<entity_type, size_type, hash<entity_type>, allocator_type>;

            template<typename T>
            using typed_component = paged_vector<T, T*, allocator_type>;

            using untyped_component = void*;
            using component_collection = hashtable<size_t, untyped_component, allocator_type>;

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

        public:
            /**
             * @return The archetype ID.
             */
            constexpr hash_t get_id() const noexcept { return archetype_id; }

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
                const size_t component_hash = typeid(T).hash_code();

                return components.contains(component_hash);
            }

            /**
             * Get an existing component.
             *
             * @tparam T The component type.
             *
             * @return A const reference to the component.
             */
            template<typename T>
            constexpr const typed_component<T>& get_component() const {
                const size_t component_hash = typeid(T).hash_code();
                const auto it = components.find(component_hash);

                CCL_THROW_IF(it == components.end(), std::out_of_range{"Component not present in archetype."});

                return *reinterpret_cast<const typed_component<T>*>(*it);
            }

            /**
             * Compute an archetype ID.
             *
             * @param component_types The sequence of component types, in any order.
             *
             * @return The computed archetype ID.
             */
            static constexpr hash_t make_id(const std::span<const type_info*> component_types) {
                hash_t id = 0;

                for(const auto * const t : component_types) {
                    id ^= t->hash_code();
                }

                return id;
            }
    };
}

#endif // CCL_ARCHETYPE_HPP
