#include <cstring>
#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/string/basic-string.hpp>

using namespace ccl;

template<typename CharType = char>
using test_string = basic_string<CharType, char_traits<CharType>, counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor", []() {
        test_string<> v;

        check(v.length() == 0);
        check(v.raw() == nullptr);
    });

    suite.add_test("ctor (raw, w/size)", []() {
        test_string<> v{"abcd", 4};

        check(v.length() == 4);

        for(unsigned i = 0; i < v.length(); ++i) {
            equals(v[i], 'a' + i);
        }
    });

    suite.add_test("ctor (raw, wo/size)", []() {
        test_string<> v{"abcd"};

        check(v.length() == 4);

        for(unsigned i = 0; i < v.length(); ++i) {
            equals(v[i], 'a' + i);
        }
    });

    suite.add_test("ctor (copy)", [] () {
        test_string<> v{"abcd"};
        test_string<> v2{v};

        check(v.length() == 4);
        check(v2.length() == 4);
        check(v.raw() != v2.raw());

        for(unsigned i = 0; i < v.length(); ++i) {
            equals(v[i], v2[i]);
        }
    });

    suite.add_test("ctor (move)", [] () {
        test_string<> v{"abcd"};
        test_string<> v2{std::move(v)};

        equals(v.raw(), nullptr);

        equals(v2.length(), 4);
        differs(v2.raw(), nullptr);
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

    suite.add_test("is_empty", [] () {
        test_string<> s1{"abc"}, s2;

        equals(s1.is_empty(), false);
        equals(s2.is_empty(), true);
    });

    suite.add_test("to_nul_terminated", [] () {
        test_string<> s{"abcd"};
        char out[5];

        // Set to non-zero
        for(int i = 0; i < 5; ++i) {
            out[i] = 'Z';
        }

        s.to_nul_terminated(std::span<char, 5>(out));

        for(int i = 0; i < 4; ++i) {
            equals(out[i], s[i]);
        }

        equals(out[4], '\0');
    });

    suite.add_test("to_nul_terminated (truncate)", [] () {
        test_string<> s{"abcd"};
        char out[3];

        // Set to non-zero
        for(int i = 0; i < 2; ++i) {
            out[i] = 'Z';
        }

        s.to_nul_terminated(std::span<char, 3>(out));

        equals(out[0], s[0]);
        equals(out[1], s[1]);
        equals(out[2], '\0');
    });

    suite.add_test("to_nul_terminated (long buffer)", [] () {
        test_string<> s{"abcd"};
        char out[128];

        // Set to non-zero
        for(int i = 0; i < 128; ++i) {
            out[i] = 'Z';
        }

        s.to_nul_terminated(std::span<char, 128>(out));

        for(int i = 0; i < 4; ++i) {
            equals(out[i], s[i]);
        }

        equals(out[4], '\0');
    });

    suite.add_test("starts_with", [] () {
        test_string<> s1{"abcd"}, s2{"ab"}, s3{"ac"}, s4{"abcd"}, s5{"abcde"};

        equals(s1.starts_with(s2), true);
        equals(s1.starts_with(s3), false);
        equals(s1.starts_with(s4), true);
        equals(s1.starts_with(s5), false);
    });

    suite.add_test("starts_with (c-string)", [] () {
        test_string<> s1{"abcd"};
        const char *s2{"ab"}, *s3{"ac"}, *s4{"abcd"}, *s5{"abcde"};

        equals(s1.starts_with(s2, 2), true);
        equals(s1.starts_with(s3, 2), false);
        equals(s1.starts_with(s4, 4), true);
        equals(s1.starts_with(s5, 5), false);
    });

    suite.add_test("starts_with (c-string literal)", [] () {
        test_string<> s1{"abcd"};

        equals(s1.starts_with("ab"), true);
        equals(s1.starts_with("ac"), false);
        equals(s1.starts_with("abcd"), true);
        equals(s1.starts_with("abcde"), false);
    });

    suite.add_test("ends_with", [] () {
        test_string<> s1{"abcd"}, s2{"cd"}, s3{"ac"}, s4{"abcd"}, s5{"abcde"};

        equals(s1.ends_with(s2), true);
        equals(s1.ends_with(s3), false);
        equals(s1.ends_with(s4), true);
        equals(s1.ends_with(s5), false);
    });

    suite.add_test("ends_with (c-string)", [] () {
        test_string<> s1{"abcd"};
        const char *s2{"cd"}, *s3{"ac"}, *s4{"abcd"}, *s5{"abcde"};

        equals(s1.ends_with(s2, 2), true);
        equals(s1.ends_with(s3, 2), false);
        equals(s1.ends_with(s4, 4), true);
        equals(s1.ends_with(s5, 5), false);
    });

    suite.add_test("ends_with (c-string literal)", [] () {
        test_string<> s1{"abcd"};

        equals(s1.ends_with("cd"), true);
        equals(s1.ends_with("ac"), false);
        equals(s1.ends_with("abcd"), true);
        equals(s1.ends_with("abcde"), false);
    });

    suite.add_test("compare", [] () {
        test_string<> s1{"abcd"}, s2{"cd"}, s3{"ac"}, s4{"abcd"}, s5{"abcde"};

        equals(s1.compare(s2), -1);
        equals(s1.compare(s3), -1);
        equals(s1.compare(s4), 0);
        equals(s1.compare(s5), -1);
    });

    return suite.main(argc, argv);
}
