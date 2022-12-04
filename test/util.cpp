#include <ccl/test.hpp>
#include <ccl/util.hpp>

using namespace ccl;

int main(int argc, char ** argv) {
    test_suite suite;

    suite.add_test("choose", [] () {
        check(choose(1, 2, true) == 1);
        check(choose(1, 2, false) == 2);
    });

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

    suite.add_test("max", [] () {
        check(max(0, 0, 0, 0, 0, 0) == 0);
        check(max(1, 2, 3) == 3);
        check(max(-500, 0, 0) == 0);
        check(max(1, 2) == 2);
        check(max(5) == 5);
    });

    suite.add_test("min", [] () {
        check(min(0, 0, 0, 0, 0, 0) == 0);
        check(min(1, 2, 3) == 1);
        check(min(-500, 0, 0) == -500);
        check(min(1, 2) == 1);
        check(min(5) == 5);
    });

    suite.add_test("bitcount", [] () {
        check(bitcount(4) == 3);
        check(bitcount(1) == 1);
        check(bitcount(0) == 0);
    });

    suite.add_test("is_type_in_pack", [] () {
        check(is_type_in_pack<int, float, int>());
        check(!is_type_in_pack<char, float, int>());
    });

    suite.add_test("size_of", [] () {
        check(size_of<int[24]>() == sizeof(int) * 24);
        check(size_of<int>() == sizeof(int));
    });

    return suite.main(argc, argv);
}
