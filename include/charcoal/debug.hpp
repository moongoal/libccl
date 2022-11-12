/**
 * @file
 *
 * Debugging facilities.
 */
#ifndef CCL_DEBUG_HPP
#define CCL_DEBUG_HPP

#include <cstdlib>
#include <charcoal/api.hpp>

#ifdef CCL_FEATURE_ASSERTIONS
    #define CCL_ASSERT(cond) do { if(!(cond)) { ::abort(); } } while(false)
#else // CCL_FEATURE_ASSERTIONS
    #define CCL_ASSERT(cond) ((void)0)
#endif // CCL_FEATURE_ASSERTIONS

#ifdef CCL_FEATURE_EXCEPTIONS
    #define CCL_THROW_IF(cond, exc) do { if((cond)) { throw (exc); } } while(false)
#else // CCL_FEATURE_EXCEPTIONS
    #define CCL_THROW_IF(cond) ((void)0)
#endif // CCL_FEATURE_EXCEPTIONS

#endif // CCL_DEBUG_HPP
