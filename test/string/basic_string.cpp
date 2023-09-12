#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/string/basic_string.hpp>

using namespace ccl;

template<typename CharType = char>
using test_string = basic_string<CharType, char_traits<CharType>, counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("", [] () {
        test_string<> s;
    });

    return suite.main(argc, argv);
}
