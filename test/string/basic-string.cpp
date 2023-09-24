#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/string/basic-string.hpp>

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

    suite.add_test("ctor (c-string)", [] () {
        const char * const cstr = "abc";
        test_string<> v{cstr};

        check(v.size() == 3);
    });

    suite.add_test("ctor (string literal)", [] () {
        test_string<> v{"abc"};

        check(v.size() == 3);
    });

    suite.add_test("ctor (copy)", [] () {
        test_string<> v;

        v.push_back('a');
        v.push_back('a');
        v.push_back('a');

        test_string<> v2{v};

        check(v.size() == 3);
        check(v2.size() == 3);

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
        check(v2.data() != nullptr);
    });

    suite.add_test("ctor (initializer list)", [] () {
        test_string<> v{'a', 'b', 'c', 'd', 'e', 'f', 'g'};

        check(v.size() == 7);
    });

    suite.add_test("operator ==", [] () {
        test_string<> s1{"abc"};
        test_string<> s2{"cde"};
        test_string<> s3{"abc"};

        check(s1 == s3);
        check(!(s1 == s2));
        check(s1 == s1);
    });

    suite.add_test("operator !=", [] () {
        test_string<> s1{"abc"};
        test_string<> s2{"cde"};
        test_string<> s3{"abc"};

        check(!(s1 != s3));
        check(s1 != s2);
        check(!(s1 != s1));
    });

    suite.add_test("operator >", [] () {
        test_string<> s1{"abc"};
        test_string<> s2{"cde"};
        test_string<> s3{"abc"};

        check(!(s1 > s3));
        check(s2 > s1);
        check(!(s1 > s1));
    });

    suite.add_test("operator <", [] () {
        test_string<> s1{"abc"};
        test_string<> s2{"cde"};
        test_string<> s3{"abc"};

        check(!(s1 < s3));
        check(s1 < s2);
        check(!(s1 < s1));
    });

    suite.add_test("operator >=", [] () {
        test_string<> s1{"abc"};
        test_string<> s2{"cde"};
        test_string<> s3{"abc"};

        check(s1 >= s3);
        check(s2 >= s1);
        check(!(s1 >= s2));
        check(s1 >= s1);
    });

    suite.add_test("operator <=", [] () {
        test_string<> s1{"abc"};
        test_string<> s2{"cde"};
        test_string<> s3{"abc"};

        check(s3 <= s1);
        check(s1 <= s2);
        check(s1 <= s2);
        check(s1 <= s1);
    });

    suite.add_test("operator []", [] () {
        test_string<> s1{"abc"};
        test_string<> s2{"abd"};

        differs(s1[2], s2[2]);

        s2[2] = 'c';

        equals(s1[2], s2[2]);
    });

    suite.add_test("hash", [] () {
        test_string<> s1{"abc"};
        test_string<> s2{"abd"};
        test_string<> s3{"abc"};

        const hash_t h1 = hash<test_string<>>{}(s1);
        const hash_t h2 = hash<test_string<>>{}(s2);
        const hash_t h3 = hash<test_string<>>{}(s3);

        equals(h1, h3);
        differs(h1, h2);
    });

    suite.add_test("hash (empty)", [] () {
        test_string<> s1;

        const hash_t h1 = hash<test_string<>>{}(s1);

        differs(h1, 0);
    });

    suite.add_test("iterators (non-const)", [] () {
        test_string<> s1{"abc"};
        int count = 0;

        for(auto it = s1.begin(); it < s1.end(); ++it) {
            ++count;
            *it = 'x';
        }

        equals(count, 3);
        equals(s1[0], 'x');
    });

    suite.add_test("iterators (const)", [] () {
        const test_string<> s1{"abc"};
        int count = 0;

        for(auto it = s1.begin(); it < s1.end(); ++it) {
            ++count;
        }

        equals(count, 3);
    });

    suite.add_test("const iterators", [] () {
        test_string<> s1{"abc"};
        int count = 0;

        for(auto it = s1.cbegin(); it < s1.cend(); ++it) {
            ++count;
        }

        equals(count, 3);
    });

    suite.add_test("reverse iterators", [] () {
        test_string<> s1{"abc"};

        equals(*s1.rbegin(), 'c');
        equals(s1.rend() - s1.rbegin(), 3);
    });

    suite.add_test("reverse iterators (const)", [] () {
        const test_string<> s1{"abc"};

        equals(*s1.rbegin(), 'c');
        equals(s1.rend() - s1.rbegin(), 3);
    });

    suite.add_test("const reverse iterators", [] () {
        test_string<> s1{"abc"};

        equals(*s1.crbegin(), 'c');
        equals(s1.rend() - s1.rbegin(), 3);
    });

    suite.add_test("clear", [] () {
        test_string<> s1{"abcdegdfosgosugosgfdogod"};

        s1.clear();
        equals(s1.size(), 0);
    });

    suite.add_test("insert", [] () {
        test_string<> actual{"abd"};
        test_string<> expected{"abcd"};

        actual.insert(actual.begin() + 2, 'c');
        equals(actual, expected);
    });

    suite.add_test("insert_range", [] () {
        test_string<> actual{"abe"};
        test_string<> other{"cd"};
        test_string<> expected{"abcde"};

        actual.insert_range(actual.begin() + 2, other);
        equals(actual, expected);
    });

    suite.add_test("erase", [] () {
        test_string<> actual{"abcd"};
        test_string<> expected{"abd"};

        actual.erase(actual.begin() + 2, actual.begin() + 3);
        equals(actual, expected);
    });

    return suite.main(argc, argv);
}
