#include <charcoal/test.hpp>
#include <charcoal/util.hpp>

using namespace ccl;

int main(int argc, char ** argv) {
    suite.add_test(
        "increase_capacity",
        [] () {
            check(increase_capacity(2, 3) == 4);
            check(increase_capacity(2, 2) == 2);
            check(increase_capacity(2, 7) == 8);
            check(increase_capacity(2, 1) == 2);
        }
    );

    return suite.main(argc, argv);
}
