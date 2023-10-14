#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/string/nul-terminated.hpp>

using namespace ccl;

using test_nul_terminated_string = ansi_nul_terminated_string<counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor", [] () {
        test_nul_terminated_string b;

        equals(b.length(), 0);
        equals(b.value()[0], '\0');
    });

    suite.add_test("ctor (string)", [] () {
        ansi_string<> s{"hey"};
        test_nul_terminated_string b{s};

        equals(b.length(), 3);
        equals(b.value()[0], 'h');
        equals(b.value()[1], 'e');
        equals(b.value()[2], 'y');
        equals(b.value()[3], '\0');
    });

    suite.add_test("ctor (string, long)", [] () {
        ansi_string<> s{"heyfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"};
        test_nul_terminated_string b{s};

        equals(b.value()[0], 'h');
        equals(b.value()[1], 'e');
        equals(b.value()[2], 'y');
        equals(b.value()[3], 'f');
    });

    suite.add_test("operator = (copy)", [] () {
        ansi_string<> s{"hey"};
        test_nul_terminated_string b1{s}, b2;

        b2 = b1;

        equals(b1.value()[0], b2.value()[0]);
        differs(b1.value(), b2.value());
        equals(b1.length(), b2.length());
    });

    return suite.main(argc, argv);
}
