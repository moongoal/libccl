/**
 * @file
 *
 * ECS registry definition.
 */
#ifndef CCL_ECS_REGISTRY_HPP
#define CCL_ECS_REGISTRY_HPP

#include <ccl/api.hpp>
#include <ccl/debug.hpp>
#include <ccl/ecs/archetype.hpp>
#include <ccl/ecs/entity.hpp>
#include <ccl/ecs/view.hpp>
#include <ccl/concepts.hpp>
#include <ccl/dense-map.hpp>

namespace ccl::ecs {
    template<basic_allocator Allocator>
    class registry {
        public:
            using allocator_type = Allocator;
            using archetype = ccl::ecs::archetype<Allocator>;

        private:
            entity_id_t next_entity_id = 0;
            dense_map<hash_t, archetype> archetype_map;

        public:
            entity_t add_entity() {
                return entity_t::make(0, next_entity_id++);
            }

            template<typename ...Components>
            void add_components(
                const entity_t entity,
                const Components&& ...components
            ) {
                archetype * const old_arch = get_entity_archetype(entity);

                CCL_THROW_IF(!old_arch, std::out_of_range{"Invalid entity."});

                const hash_t new_archetype_id = old_arch->extend_id(typeid(components).hash_code()...);
                auto new_arch_it = archetype_map.find(new_archetype_id);
                archetype * new_arch;

                if(new_arch_it == archetype_map.end()) {
                    new_arch = &archetype_map.emplace(
                        entity,
                        archetype::template make_id<Components...>(components...)
                    );
                } else {
                    new_arch = &*new_arch_it;
                }

                CCL_ASSERT(new_arch);

                // TODO
                new_arch_it->migrate_entity<Components...>(entity, old_arch, std::forward<Components>(components)...);
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
