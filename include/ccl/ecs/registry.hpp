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
#include <ccl/ecs/archetype.hpp>
#include <ccl/ecs/entity.hpp>
#include <ccl/ecs/view.hpp>
#include <ccl/concepts.hpp>
#include <ccl/dense-map.hpp>

namespace ccl::ecs {
    namespace internal {
        template<basic_allocator Allocator>
        class registry_editor;
    }

    template<basic_allocator Allocator>
    class registry {
        friend class internal::registry_editor<Allocator>;

        public:
            using allocator_type = Allocator;
            using archetype = ccl::ecs::archetype<Allocator>;

            static constexpr entity_id_t max_entity_id = entity_t::underlying_type::low_part_max;

        private:
            entity_id_t next_entity_id = 0;
            dense_map<hash_t, archetype, allocator_type> archetype_map;

        public:
            /**
             * Add a new entity to this registry. The returned entity is guaranteed
             * To be new, unique and of the 0th generation. Will throw an `std::out_of_range`
             * exception if the maximum allowed number of entities is reached.
             *
             * @return The newly created entity.
             */
            CCLNODISCARD entity_t add_entity() {
                CCL_THROW_IF(
                    next_entity_id >= max_entity_id,
                    std::out_of_range{"Maximum number of entities reached."}
                );

                return entity_t::make(0, next_entity_id++);
            }

            template<typename ...Components>
            void add_components(
                const entity_t entity,
                const Components&& ...components
            ) {
                archetype * const old_arch = get_entity_archetype(entity);
                archetype * new_arch;

                hash_t new_archetype_id = old_arch
                    ? old_arch->template extend_id<Components...>()
                    : archetype::template make_id<Components...>();

                auto new_arch_it = archetype_map.find(new_archetype_id);

                if(new_arch_it == archetype_map.end()) {
                    new_arch = &archetype_map.emplace(
                        entity,
                        archetype::template make<Components...>()
                    );
                } else {
                    new_arch = new_arch_it->second();
                }

                CCL_ASSERT(new_arch);

                new_arch->add_entity(entity);

                if(old_arch) {
                    new_arch->copy_entity_components_from(entity, *old_arch);
                    old_arch->remove_entity(entity);
                }

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
    };
}
#endif // CCL_ECS_REGISTRY_HPP
