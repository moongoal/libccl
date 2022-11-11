#include <charcoal/test.hpp>
#include <charcoal/util.hpp>

using namespace ccl;

int main(int argc, char ** argv) {
    suite.add_test("is_power_2", [] () {
        check(is_power_2(5) == false);
        check(is_power_2(1) == true);
        check(is_power_2(2) == true);
        check(is_power_2(0) == true);
        check(is_power_2(64) == true);
    });

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
