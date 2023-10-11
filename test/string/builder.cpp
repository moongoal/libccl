#include <ccl/test/test.hpp>
#include <ccl/string/builder.hpp>
#include <ccl/string/ansi-string.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("", [] () {

    });

    return suite.main(argc, argv);
}
