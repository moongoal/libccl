/**
 * @file
 *
 * Entity definition.
 */
#ifndef CCL_ECS_ENTITY_HPP
#define CCL_ECS_ENTITY_HPP

#include <ccl/api.hpp>
#include <ccl/handle.hpp>

namespace ccl::ecs {
    struct entity_tag_t {};

    /**
     * The entity handle type.
     */
    using entity_t = versioned_handle<entity_tag_t>;

    /**
     * An entity ID is the raw, unversioned handle value.
     */
    using entity_id_t = typename entity_t::value_type;
}

#endif // CCL_ECS_ENTITY_HPP
