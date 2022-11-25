#include <ccl/test.hpp>
#include <ccl/util.hpp>
#include <ccl/maybe.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    suite.add_test("exists", [] () {
        maybe<int> x;

        check(sizeof(x) == sizeof(int));
    });

    suite.add_test("doesn't exist", [] () {
        maybe<empty> x;

        check(sizeof(x) <= 1);
    });

    suite.add_test("operator *", [] () {
        maybe<int> x = 5;

        check(*x == 5);
    });

    suite.add_test("get", [] () {
        maybe<int> x = 5;

        check(x.get() == 5);
    });

    return suite.main(argc, argv);
}
