/**
 * @file
 *
 * Search algorithms.
 */
#ifndef CCL_ALGORITHM_SEARCH_HPP
#define CCL_ALGORITHM_SEARCH_HPP

#include <iterator>
#include <ccl/api.hpp>

namespace ccl {
    /**
     * Perform linear search on a sequence.
     *
     * @param start The starting iterator.
     * @param end The end iterator (one past the last item).
     * @param value The value to search.
     *
     * @return The iterator pointing at the found value or `end` if
     *  the value was not found.
     */
    template<std::forward_iterator It>
    auto search_linear(const It start, const It end, const auto& value) noexcept {
        for(It it = start; it < end; ++it) {
            if(*it == value) {
                return it;
            }
        }

        return end;
    }
}

#endif // CCL_ALGORITHM_SEARCH_HPP
