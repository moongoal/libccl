/**
 * @file
 *
 * Definitions.
 */
#ifndef CCL_DEFINITIONS_HPP
#define CCL_DEFINITIONS_HPP

#ifndef CCL_HANDLE_VALUE_WIDTH
    #define CCL_HANDLE_VALUE_WIDTH 24
#endif // CCL_HANDLE_VALUE_WIDTH

#ifndef CCL_ECS_VIEW_MAX_ARCHETYPE_COUNT
    #define CCL_ECS_VIEW_MAX_ARCHETYPE_COUNT 32
#endif // CCL_ECS_VIEW_MAX_ARCHETYPE_COUNT

#ifndef CCL_HASHTABLE_MINIMUM_CAPACITY
    #define CCL_HASHTABLE_MINIMUM_CAPACITY 256
#endif // CCL_HASHTABLE_MINIMUM_CAPACITY

#ifndef CCL_HASHTABLE_CHUNK_SIZE
    #define CCL_HASHTABLE_CHUNK_SIZE 16
#endif // CCL_HASHTABLE_CHUNK_SIZE

#ifndef CCL_SET_MINIMUM_CAPACITY
    #define CCL_SET_MINIMUM_CAPACITY 256
#endif // CCL_SET_MINIMUM_CAPACITY

#ifndef CCL_SET_KEY_CHUNK_SIZE
    #define CCL_SET_KEY_CHUNK_SIZE 16
#endif // CCL_SET_KEY_CHUNK_SIZE

#ifndef CCL_ALLOCATOR_DEFAULT_ALIGNMENT
    #define CCL_ALLOCATOR_DEFAULT_ALIGNMENT __STDCPP_DEFAULT_NEW_ALIGNMENT__
#endif // CCL_ALLOCATOR_DEFAULT_ALIGNMENT

#ifndef CCL_PAGE_SIZE
    #define CCL_PAGE_SIZE 4096
#endif // CCL_PAGE_SIZE

#endif // CCL_DEFINITIONS_HPP
