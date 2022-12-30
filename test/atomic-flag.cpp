#include <ccl/test/test.hpp>
#include <ccl/atomic.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor", [] () {
        atomic_flag f;

        equals(f.test(), false);
    });

    suite.add_test("test_and_set", [] () {
        atomic_flag f;

        equals(f.test_and_set(), false);
        equals(f.test_and_set(), true);
    });

    suite.add_test("test", [] () {
        atomic_flag f;

        equals(f.test(), false);
        f.test_and_set();
        equals(f.test(), true);
    });

    suite.add_test("clear", [] () {
        atomic_flag f;

        f.test_and_set();
        f.clear();
        equals(f.test(), false);
    });

    return suite.main(argc, argv);
}
