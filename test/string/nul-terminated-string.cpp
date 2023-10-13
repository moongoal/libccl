#include <ccl/test/test.hpp>
#include <ccl/string/nul-terminated.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor", [] () {
        ansi_nul_terminated_string<> b;

        equals(b.length(), 0);
        equals(b.value()[0], '\0');
    });

    return suite.main(argc, argv);
}
