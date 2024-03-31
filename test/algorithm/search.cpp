#include <ccl/test/test.hpp>
#include <ccl/vector.hpp>
#include <ccl/algorithm/search.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("linear_search", [] () {
        vector<int> v{1, -1, 100, 10'000};

        for(const int x : v) {
            const auto x_it = search_linear(v.begin(), v.end(), x);

            differs(x_it, v.end());
            equals(*x_it, x);
        }

        // Missing item
        equals(
            search_linear(v.begin(), v.end(), 0),
            v.end()
        );
    });

    return suite.main(argc, argv);
}
