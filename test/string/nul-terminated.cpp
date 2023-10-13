#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/string/nul-terminated.hpp>

using namespace ccl;

using test_nul_terminated_string = ansi_dynamic_nul_terminated_string<counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("static ctor", [] () {
        ansi_nul_terminated_string<> b;

        equals(b.length(), 0);
        equals(b.value()[0], '\0');
    });

    suite.add_test("static ctor (string)", [] () {
        ansi_string<> s{"hey"};
        ansi_nul_terminated_string<> b{s};

        equals(b.length(), 3);
        equals(b.value()[0], 'h');
        equals(b.value()[1], 'e');
        equals(b.value()[2], 'y');
        equals(b.value()[3], '\0');
    });

    suite.add_test("dynamic ctor", [] () {
        test_nul_terminated_string b;

        equals(b.length(), 0);
        equals(b.value()[0], '\0');
    });

    suite.add_test("dynamic ctor (string)", [] () {
        ansi_string<> s{"hey"};
        test_nul_terminated_string b{s};

        equals(b.length(), 3);
        equals(b.value()[0], 'h');
        equals(b.value()[1], 'e');
        equals(b.value()[2], 'y');
        equals(b.value()[3], '\0');
    });

    return suite.main(argc, argv);
}
