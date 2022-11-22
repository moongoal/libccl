#include <ccl/test.hpp>
#include <ccl/compressed-pair.hpp>

using namespace ccl;

int main(int argc, char ** argv) {
    suite.add_test("full", [] () {
        compressed_pair<int, int> p{1, 2};

        check(sizeof(p) == sizeof(int) * 2);
        check(p.first == 1);
        check(p.second == 2);
    });

    return suite.main(argc, argv);
}
