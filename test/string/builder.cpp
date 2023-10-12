#include <ccl/test/test.hpp>
#include <ccl/string/builder.hpp>
#include <ccl/string/ansi-string.hpp>
#include <ccl/test/counting-test-allocator.hpp>

using namespace ccl;

using test_string = ansi_string<counting_test_allocator>;
using test_builder = basic_string_builder<test_string::value_type, test_string::char_traits, counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor", [] () {
        test_builder b;

        const auto s = b.to_string();

        equals(s.raw(), nullptr);
        equals(s.length(), 0);
    });

    suite.add_test("ctor (string)", [] () {
        test_string s{"abcd"};
        test_builder b{s};

        equals(b.to_string(), s);
    });

    return suite.main(argc, argv);
}
