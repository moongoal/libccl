/**
 * @file
 *
 * Internal registry editor used for testing purposes.
 */
#ifndef CCL_ECS_REGISTRY_EDITOR_HPP
#define CCL_ECS_REGISTRY_EDITOR_HPP

#include <ccl/api.hpp>
#include <ccl/concepts.hpp>
#include <ccl/ecs/registry.hpp>

namespace ccl::ecs::internal {
    template<basic_allocator Allocator>
    class registry_editor {
        public:
            using registry_type = registry<Allocator>;
            using allocator_type = Allocator;

        private:
            registry_type *registry;

        public:
            registry_editor(registry_type &registry) : registry{&registry} {}

            void set_next_entity_id(const entity_id_t value) {
                registry->next_entity_id = value;
            }
    };
}

#endif // CCL_ECS_REGISTRY_EDITOR_HPP
