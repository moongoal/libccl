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

    suite.add_test("ctor (raw)", [] () {
        test_builder b{"abcd"};

        equals(b.to_string(), test_string{"abcd"});
    });

    suite.add_test("ctor (copy)", [] () {
        test_builder b{"abcd"}, b2{b};

        equals(b.to_string(), b2.to_string());
    });

    suite.add_test("ctor (move)", [] () {
        test_builder b{"abcd"}, b2{std::move(b)};

        equals(test_string{"abcd"}, b2.to_string());
    });

    suite.add_test("operator << (bool)", [] () {
        test_builder b{"ab"};

        b << true;
        b << false;

        equals(b.to_string(), test_string{"ab10"});
    });

    suite.add_test("operator << (short)", [] () {
        test_builder b{"ab"};

        b << static_cast<short>(123);

        equals(b.to_string(), test_string{"ab123"});
    });

    suite.add_test("operator << (string)", [] () {
        test_builder b{"ab"};

        b << test_string{"cd"};

        equals(b.to_string(), test_string{"abcd"});
    });

    suite.add_test("operator << (long)", [] () {
        test_builder b{"ab"};

        b << 123l;

        equals(b.to_string(), test_string{"ab123"});
    });

    suite.add_test("operator << (unsigned long)", [] () {
        test_builder b{"ab"};

        b << 123ul;

        equals(b.to_string(), test_string{"ab123"});
    });

    suite.add_test("operator << (long long)", [] () {
        test_builder b{"ab"};

        b << 123L;

        equals(b.to_string(), test_string{"ab123"});
    });

    suite.add_test("operator << (unsigned long long)", [] () {
        test_builder b{"ab"};

        b << 123UL;

        equals(b.to_string(), test_string{"ab123"});
    });

    suite.add_test("operator << (double)", [] () {
        test_builder b{"ab"};

        b << 123.5;

        equals(b.to_string(), test_string{"ab123.500000"});
    });

    suite.add_test("operator << (long double)", [] () {
        test_builder b{"ab"};

        b << static_cast<long double>(123.5);

        equals(b.to_string(), test_string{"ab123.500000"});
    });

    suite.add_test("operator << (int)", [] () {
        test_builder b{"ab"};

        b << -123;

        equals(b.to_string(), test_string{"ab-123"});
    });

    suite.add_test("operator << (unsigned int)", [] () {
        test_builder b{"ab"};

        b << 123u;

        equals(b.to_string(), test_string{"ab123"});
    });

    suite.add_test("operator << (string literal)", [] () {
        test_builder b{"ab"};

        b << "cd";

        equals(b.to_string(), test_string{"abcd"});
    });

    suite.add_test("append(c-string)", [] () {
        test_builder b{"ab"};

        b.append("cd", 2);
        equals(b.to_string(), test_string{"abcd"});
    });

    return suite.main(argc, argv);
}
