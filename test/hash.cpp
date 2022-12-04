#ifdef CCL_FEATURE_HASH_STL_TYPES
    #include <string>
#endif // CCL_FEATURE_HASH_STL_TYPES

#include <ccl/test.hpp>
#include <ccl/hash.hpp>

using namespace ccl;

struct test_externally_hashable {};

struct test_internally_hashable {
    hash_t hash() const noexcept {
        return 2;
    }
};

template<>
struct ccl::hash<test_externally_hashable> {
    hash_t operator()(const test_externally_hashable&) {
        return 1;
    }
};

enum test_enum {
    one,
    two,
    three
};

enum class test_scoped_enum {
    one,
    two,
    three
};

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("hash external", [] () {
        check(hash<test_externally_hashable>{}({}) == 1);
    });

    suite.add_test("hash internal", [] () {
        check(hash<test_internally_hashable>{}({}) == 2);
    });

    suite.add_test("hash bool", [] () { check(hash<bool>{}(true) == 1); });

    suite.add_test("hash uint8_t", [] () { check(hash<uint8_t>{}(2) == 2); });
    suite.add_test("hash uint16_t", [] () { check(hash<uint16_t>{}(2) == 2); });
    suite.add_test("hash uint32_t", [] () { check(hash<uint32_t>{}(2) == 2); });
    suite.add_test("hash uint64_t", [] () { check(hash<uint64_t>{}(2) == 2); });

    suite.add_test("hash int8_t", [] () { check(hash<int8_t>{}(2) == 2); });
    suite.add_test("hash int16_t", [] () { check(hash<int16_t>{}(2) == 2); });
    suite.add_test("hash int32_t", [] () { check(hash<int32_t>{}(2) == 2); });
    suite.add_test("hash int64_t", [] () { check(hash<int64_t>{}(2) == 2); });

    suite.add_test("hash char8_t", [] () { check(hash<char8_t>{}('x') == static_cast<hash_t>('x')); });
    suite.add_test("hash char16_t", [] () { check(hash<char16_t>{}('x') == static_cast<hash_t>('x')); });
    suite.add_test("hash char32_t", [] () { check(hash<char32_t>{}('x') == static_cast<hash_t>('x')); });
    suite.add_test("hash char", [] () { check(hash<char>{}('x') == static_cast<hash_t>('x')); });
    suite.add_test("hash wchar_t", [] () { check(hash<wchar_t>{}('x') == static_cast<hash_t>('x')); });

    suite.add_test("hash float", [] () {
        const float value = 5.443;

        check(hash<float>{}(value) == *reinterpret_cast<const uint32_t*>(&value));
    });

    suite.add_test("hash double", [] () {
        const double value = 5.443;

        check(hash<double>{}(value) == *reinterpret_cast<const hash_t*>(&value));
    });

    if constexpr(sizeof(double) == sizeof(long double)) {
        suite.add_test("hash long double (size same as double)", [] () {
            const long double value = 5.443;

            check(hash<long double>{}(value) == *reinterpret_cast<const hash_t*>(&value));
        });
    } else {
        suite.add_test("hash long double (size greater than double)", [] () {
            union pun { long double n; uint64_t bits[2]; } x;

            x.bits[0] = 0x123;
            x.bits[1] = 0x666;

            const hash_t expected_hash = (0x666ULL << 32) + 0x123;

            check(hash<long double>{}(x.n) == expected_hash);
        });
    }

    suite.add_test("hash std::nullptr_t", [] () {
        check(hash<std::nullptr_t>{}(nullptr) == 0);
    });

    suite.add_test("hash pointer", [] () {
        const float value = 5.443;

        check(hash<const float*>{}(&value) == reinterpret_cast<hash_t>(&value));
    });

    suite.add_test("hash enum", [] () {
        const test_enum value = one;

        check(hash<test_enum>{}(value) == static_cast<hash_t>(value));
    });

    suite.add_test("hash scoped enum", [] () {
        const test_scoped_enum value = test_scoped_enum::one;

        check(hash<test_scoped_enum>{}(value) == static_cast<hash_t>(value));
    });

    return suite.main(argc, argv);
}
