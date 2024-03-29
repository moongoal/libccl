/**
 * @file
 *
 * Enabled features.
 */
#ifndef CCL_FEATURES_HPP
#define CCL_FEATURES_HPP

#ifndef CCL_OVERRIDE_FEATURE_ASSERTIONS
    #define CCL_FEATURE_ASSERTIONS
#endif // CCL_OVERRIDE_FEATURE_ASSERTIONS

#ifndef CCL_OVERRIDE_FEATURE_EXCEPTIONS
    #define CCL_FEATURE_EXCEPTIONS
#endif // CCL_OVERRIDE_FEATURE_EXCEPTIONS

#ifndef CCL_OVERRIDE_FEATURE_TYPECHECK_CASTS
    #define CCL_FEATURE_TYPECHECK_CASTS
#endif // CCL_OVERRIDE_FEATURE_TYPECHECK_CASTS

#ifndef CCL_OVERRIDE_FEATURE_ECS_CHECK_ARCHETYPE_COMPONENTS
    #define CCL_FEATURE_ECS_CHECK_ARCHETYPE_COMPONENTS
#endif // CCL_OVERRIDE_FEATURE_ECS_CHECK_ARCHETYPE_COMPONENTS

#ifndef CCL_OVERRIDE_FEATURE_STL_COMPAT
    #define CCL_FEATURE_STL_COMPAT
#endif // CCL_OVERRIDE_FEATURE_STL_COMPAT

#ifndef CCL_OVERRIDE_FEATURE_DEFAULT_ALLOCATION_FLAGS
    #define CCL_FEATURE_DEFAULT_ALLOCATION_FLAGS
#endif // CCL_OVERRIDE_FEATURE_DEFAULT_ALLOCATION_FLAGS

#endif // CCL_FEATURES_HPP
