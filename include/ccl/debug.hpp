/**
 * @file
 *
 * Debugging facilities.
 */
#ifndef CCL_DEBUG_HPP
#define CCL_DEBUG_HPP

#include <cstdlib>
#include <ccl/api.hpp>

#ifdef CCL_FEATURE_ASSERTIONS
    #define CCL_ASSERT(cond) do { if(!(cond)) CCLUNLIKELY { ::abort(); } } while(false)
#else // CCL_FEATURE_ASSERTIONS
    #define CCL_ASSERT(cond) ((void)0)
#endif // CCL_FEATURE_ASSERTIONS

#ifdef CCL_FEATURE_EXCEPTIONS
    #define CCL_THROW_IF(cond, exc) do { if((cond)) CCLUNLIKELY { throw (exc); } } while(false)
    #define CCL_THROW(exc) do { throw (exc); } while(false)
#else // CCL_FEATURE_EXCEPTIONS
    #define CCL_THROW_IF(cond, exc) ((void)0)
    #define CCL_THROW(exc) ((void)0)
#endif // CCL_FEATURE_EXCEPTIONS

#endif // CCL_DEBUG_HPP
