/**
 * @file
 *
 * CCL base library.
 */
#ifndef CCL_HPP
#define CCL_HPP

#include <ccl/api.hpp>
#include <ccl/bitset.hpp>
#include <ccl/pair.hpp>
#include <ccl/compressed-pair.hpp>
#include <ccl/concepts.hpp>
#include <ccl/debug.hpp>
#include <ccl/definitions.hpp>
#include <ccl/exceptions.hpp>
#include <ccl/features.hpp>
#include <ccl/hash.hpp>
#include <ccl/hashtable.hpp>
#include <ccl/macros.hpp>
#include <ccl/maybe.hpp>
#include <ccl/set.hpp>
#include <ccl/test/test.hpp>
#include <ccl/util.hpp>
#include <ccl/vector.hpp>
#include <ccl/version.hpp>
#include <ccl/dense-map.hpp>
#include <ccl/packed-integer.hpp>
#include <ccl/tagged-pointer.hpp>
#include <ccl/paged-vector.hpp>
#include <ccl/deque.hpp>
#include <ccl/sparse-set.hpp>
#include <ccl/type-traits.hpp>
#include <ccl/either.hpp>

#include <ccl/memory/memory.hpp>
#include <ccl/pointer/pointer.hpp>

#include <ccl/i18n/language.hpp>

#ifdef CCL_FEATURE_STL_COMPAT
    #include <ccl/compat.hpp>
#endif // CCL_FEATURE_STL_COMPAT

#endif // CCL_HPP
