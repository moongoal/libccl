/**
 * @file
 *
 * Entity definition.
 */
#ifndef CCL_ECS_ENTITY_HPP
#define CCL_ECS_ENTITY_HPP

#include <ccl/api.hpp>
#include <ccl/handle.hpp>

namespace ccl {
    struct generic_entity_tag_t {};

    /**
     * The entity handle type.
     *
     * @tparam EntityType The entity type. Defaults to a generic entity type.
     */
    template<typename EntityType = generic_entity_tag_t>
    using entity_t = versioned_handle<EntityType>;

    /**
     * An entity ID is the raw, unversioned handle value.
     *
     * @tparam EntityType The entity type.
     */
    template<typename EntityType>
    using entity_id_t = typename entity_t<EntityType>::value_type;
}

#endif // CCL_ECS_ENTITY_HPP
