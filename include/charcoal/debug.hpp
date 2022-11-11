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
    #define CCL_ASSERT(cond) do { ::abort(); } while(!(cond))
    #define THROW_IF(cond, exc) do { throw (exc); } while((cond))
#else
    #define CCL_ASSERT(cond) ((void)0)
    #define THROW_IF(cond) ((void)0)
#endif // CCL_FEATURE_ASSERTIONS

#endif // CCL_DEBUG_HPP
