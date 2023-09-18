#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/string/basic_string.hpp>

using namespace ccl;

template<typename CharType = char>
using test_string = basic_string<CharType, char_traits<CharType>, counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test(
        "ctor",
        []() {
            test_string<> v;

            check(v.capacity() == 0);
            check(v.size() == 0);
            check(v.data() == nullptr);
        }
    );

    suite.add_test("ctor (copy)", [] () {
        test_string<> v;

        v.push_back('a');
        v.push_back('a');
        v.push_back('a');

        test_string<> v2{v};

        check(v.size() == 3);
        check(v.capacity() == 4);

        check(v2.size() == 3);
        check(v2.capacity() == 4);

        check(v.data() != v2.data());
    });

    suite.add_test("ctor (move)", [] () {
        test_string<> v;

        v.push_back('a');
        v.push_back('a');
        v.push_back('a');

        test_string<> v2{std::move(v)};

        check(v.data() == nullptr);

        check(v2.size() == 3);
        check(v2.capacity() == 4);
        check(v2.data() != nullptr);
    });

    return suite.main(argc, argv);
}
