#include <ccl/test/test.hpp>
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

    suite.add_test("hash bool", [] () { equals(hash<bool>{}(true), 1ULL); });

    suite.add_test("hash uint8_t", [] () { equals(hash<uint8_t>{}(2), 2ULL); });
    suite.add_test("hash uint16_t", [] () { equals(hash<uint16_t>{}(2), 2ULL); });
    suite.add_test("hash uint32_t", [] () { equals(hash<uint32_t>{}(2), 2ULL); });
    suite.add_test("hash uint64_t", [] () { equals(hash<uint64_t>{}(2), 2ULL); });

    suite.add_test("hash int8_t", [] () { equals(hash<int8_t>{}(2), 2ULL); });
    suite.add_test("hash int16_t", [] () { equals(hash<int16_t>{}(2), 2ULL); });
    suite.add_test("hash int32_t", [] () { equals(hash<int32_t>{}(2), 2ULL); });
    suite.add_test("hash int64_t", [] () { equals(hash<int64_t>{}(2), 2ULL); });

    suite.add_test("hash char8_t", [] () { equals(hash<char8_t>{}('x'), static_cast<hash_t>('x')); });
    suite.add_test("hash char16_t", [] () { equals(hash<char16_t>{}('x'), static_cast<hash_t>('x')); });
    suite.add_test("hash char32_t", [] () { equals(hash<char32_t>{}('x'), static_cast<hash_t>('x')); });
    suite.add_test("hash char", [] () { equals(hash<char>{}('x'), static_cast<hash_t>('x')); });
    suite.add_test("hash wchar_t", [] () { equals(hash<wchar_t>{}('x'), static_cast<hash_t>('x')); });

    suite.add_test("hash float", [] () {
        const float value = 5.443;

        equals(hash<float>{}(value), *reinterpret_cast<const uint32_t*>(&value));
    });

    suite.add_test("hash float -0", [] () {
        equals(hash<float>{}(-0), hash<float>{}(+0));
    });

    suite.add_test("hash double", [] () {
        const double value = 5.443;

        equals(hash<double>{}(value), *reinterpret_cast<const hash_t*>(&value));
    });

    suite.add_test("hash double -0", [] () {
        equals(hash<double>{}(-0), hash<double>{}(+0));
    });

    suite.add_test("hash long double (size same as double)", [] () {
        const long double value = 5.443;

        equals(hash<long double>{}(value), *reinterpret_cast<const hash_t*>(&value));
    }, [] () { return sizeof(long double) != sizeof(double); });


    suite.add_test("hash long double (size greater than double)", [] () {
        union pun { uint8_t b[128]; long double ld; } x1, x2;

        for(int i = 0; i < 128; ++i) {
            x1.b[i] = i;
        }

        for(int i = 0; i < 128; ++i) {
            x2.b[i] = i * 2;
        }

        x1.ld = x2.ld = 0.00000234;

        equals(
            hash<long double>{}(x1.ld),
            hash<long double>{}(x2.ld)
        );
        differs(
            hash<long double>{}(x1.ld),
            hash<long double>{}(-x1.ld)
        );
    }, [] () { return sizeof(long double) == sizeof(double); });

    suite.add_test("hash long double -0", [] () {
        equals(hash<long double>{}(-0), hash<long double>{}(+0));
    });

    suite.add_test("hash long double (slow)", [] () {
        const long double value = 5.443;
        const long double value2 = -5.443;

        differs(
            hash<long double>{}._compute_hash_slow(value),
            hash<long double>{}._compute_hash_slow(value2)
        );
    });

    suite.add_test("hash long double -0 (slow)", [] () {
        equals(
            hash<long double>{}._compute_hash_slow(-0),
            hash<long double>{}._compute_hash_slow(+0)
        );
    });

    suite.add_test("hash std::nullptr_t", [] () {
        equals(hash<std::nullptr_t>{}(nullptr), 0ULL);
    });

    suite.add_test("hash pointer", [] () {
        const float value = 5.443;

        equals(hash<const float*>{}(&value), reinterpret_cast<hash_t>(&value));
    });

    suite.add_test("hash enum", [] () {
        const test_enum value = one;

        equals(hash<test_enum>{}(value), static_cast<hash_t>(value));
    });

    suite.add_test("hash scoped enum", [] () {
        const test_scoped_enum value = test_scoped_enum::one;

        equals(hash<test_scoped_enum>{}(value), static_cast<hash_t>(value));
    });

    suite.add_test("fnv1a empty", [] () {
        equals(fnv1a_hash(0, nullptr), fnv1a_basis);
    });

    suite.add_test("fnv1a value", [] () {
        const uint8_t value[] = { 0x61, 0x78, 0x95, 0x75, 0xac };

        equals(fnv1a_hash(sizeof(value), value), 0x7242825e8642aa02ULL);
    });

    suite.add_test("hash long double negation", [] () {
        const long double value = 0.00000234;

        differs(hash<long double>{}(value), hash<long double>{}(-value));
    });

    suite.add_test("hash double negation", [] () {
        const double value = 0.00000234;

        differs(hash<double>{}(value), hash<double>{}(-value));
    });

    suite.add_test("hash float negation", [] () {
        const float value = 0.000234;

        differs(hash<float>{}(value), hash<float>{}(-value));
    });

    return suite.main(argc, argv);
}
