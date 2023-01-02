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

    /**
     * A null entity value. This is a placeholder value
     * to initialise unused entities but is not treated specially
     * by the ECS registry.
     */
    static constexpr entity_t null_entity = entity_t{~static_cast<handle_t>(0U)};
}

#endif // CCL_ECS_ENTITY_HPP
