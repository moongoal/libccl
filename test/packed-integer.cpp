#include <ccl/test/test.hpp>
#include <ccl/packed-integer.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor (default)", [] () {
        using my_int = packed_integer<uint64_t>;

        my_int n;

        equals(n.get(), 0ULL);
        equals(n.high(), 0ULL);
        equals(n.low(), 0ULL);
        equals(my_int::low_part_size, 32);
        equals(my_int::high_part_shift_bits, 32);
        equals(my_int::low_part_mask, 0xffffffff);
    });

    suite.add_test("ctor (default, uneven)", [] () {
        using my_int = packed_integer<uint64_t, 48>;

        equals(my_int::low_part_size, 48);
        equals(my_int::high_part_shift_bits, 48);
        equals(my_int::low_part_mask, 0xffffffffffff);
        equals(my_int::low_part_max, (1ULL << 48) - 1);
        equals(my_int::high_part_max, (1ULL << 16) - 1);
    });

    suite.add_test("ctor (value)", [] () {
        using my_int = packed_integer<uint64_t>;

        my_int n = 0xaaaaaaaabbbbbbbb;

        equals(n.get(), 0xaaaaaaaabbbbbbbb);
        equals(n.high(), 0xaaaaaaaa);
        equals(n.low(), 0xbbbbbbbb);
    });

    suite.add_test("ctor (copy)", [] () {
        using my_int = packed_integer<uint64_t>;

        my_int n = 0xaaaaaaaabbbbbbbb;
        my_int n2 = n;

        equals(n2.get(), 0xaaaaaaaabbbbbbbb);
        equals(n2.high(), 0xaaaaaaaa);
        equals(n2.low(), 0xbbbbbbbb);
    });

    suite.add_test("type cast", [] () {
        using my_int = packed_integer<uint64_t>;

        my_int n = 0xaaaaaaaabbbbbbbb;

        equals(n.get(), 0xaaaaaaaabbbbbbbb);
    });

    suite.add_test("assignment (copy)", [] () {
        using my_int = packed_integer<uint64_t>;

        my_int n = 0xaaaaaaaabbbbbbbb;
        my_int n2 = 4;

        n = n2;

        equals(n.get(), 4);
    });

    suite.add_test("assignment (value)", [] () {
        using my_int = packed_integer<uint64_t>;

        my_int n = 0xaaaaaaaabbbbbbbb;

        n = 4;

        equals(n.get(), 4);
    });

    suite.add_test("high, low & get", [] () {
        using my_int = packed_integer<uint64_t, 48>;

        my_int n = 0xaaaaccccbbbbbbbb;

        equals(n.high(), 0xaaaa);
        equals(n.low(), 0xccccbbbbbbbb);
        equals(n.get(), 0xaaaaccccbbbbbbbb);
    });

    suite.add_test("comparison (equality, reference)", [] () {
        using my_int = packed_integer<uint64_t, 48>;

        my_int n = 0xaaaaccccbbbbbbbb;
        my_int n2 = 0xaaaaccccbbbbbbbb;
        my_int n3 = 0xaaaaccccbbbbbbbc;

        check(n == n2);
        check(n2 != n3);
    });

    suite.add_test("comparison (equality/inequality, obj vs value)", [] () {
        using my_int = packed_integer<uint64_t, 48>;

        my_int n = 0xaaaaccccbbbbbbbb;

        check(n == 0xaaaaccccbbbbbbbb);
        check(n != 0xaaaaccccbbbbbbbd);
    });

    suite.add_test("comparison (equality/inequality, value vs obj)", [] () {
        using my_int = packed_integer<uint64_t, 48>;

        my_int n = 0xaaaaccccbbbbbbbb;

        check(0xaaaaccccbbbbbbbb == n);
        check(0xaaaaccccbbbbbbbd != n);
    });

    suite.add_test("make", [] () {
        using my_int = packed_integer<uint64_t, 48>;

        equals(0xaaaaccccbbbbbbbb, my_int::make(0xaaaa, 0xccccbbbbbbbb).get());
    });

    suite.add_test("make (high value too large)", [] () {
        using my_int = packed_integer<uint64_t, 48>;

        throws<std::out_of_range>([] () {
            const auto x CCLUNUSED = my_int::make(0xccccbbbbbbbb, 0xccccbbbbbbbb);
        });
    });

    suite.add_test("make (low value too large)", [] () {
        using my_int = packed_integer<uint64_t, 48>;

        throws<std::out_of_range>([] () {
            const auto x CCLUNUSED = my_int::make(0x1, 0xccccbbbbbbbbcc);
        });
    });

    return suite.main(argc, argv);
}
