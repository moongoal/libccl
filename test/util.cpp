#include <ccl/test/test.hpp>
#include <ccl/util.hpp>

using namespace ccl;

int main(int argc, char ** argv) {
    test_suite suite;

    suite.add_test("choose", [] () {
        equals(choose(1, 2, true), 1);
        equals(choose(1, 2, false), 2);
        equals(choose(1, 2UL, true), 1);
        equals(choose(static_cast<int*>(nullptr), reinterpret_cast<int*>(0x123), true), nullptr);
        equals(choose(1.0f, 2.0, false), 2.0);
    });

    suite.add_test("choose (ptr)", [] () {
        int * const ptr1 = reinterpret_cast<int*>(0x123);
        int * const ptr2 = reinterpret_cast<int*>(0x321);

        equals(choose(ptr1, ptr2, true), ptr1);
        equals(choose(ptr1, ptr2, false), ptr2);
    });

    suite.add_test("is_power_2", [] () {
        check(is_power_2(5) == false);
        check(is_power_2(1) == true);
        check(is_power_2(2) == true);
        check(is_power_2(0) == true);
        check(is_power_2(64) == true);
    });

    suite.add_test(
        "increase_capacity",
        [] () {
            check(increase_capacity(2, 3) == 4);
            check(increase_capacity(2, 2) == 2);
            check(increase_capacity(2, 7) == 8);
            check(increase_capacity(2, 1) == 2);
            check(increase_capacity(2, 2) == 2);
        }
    );

    suite.add_test(
        "increase_capacity (capacity > threshold)",
        [] () {
            check(increase_capacity(4, 2) == 4);
        }
    );

    suite.add_test(
        "increase_capacity (capacity not power of two)",
        [] () {
            throws<std::invalid_argument>([] () {
                check(increase_capacity(6, 2) == 4);
            });
        }
    );

    suite.add_test(
        "increase_paged_capacity",
        [] () {
            check(increase_paged_capacity(4096, 4097, 4096) == 8192);
            check(increase_paged_capacity(0, 100, 4096) == 4096);
            check(increase_paged_capacity(4096, 4096, 4096) == 4096);
        }
    );

    suite.add_test(
        "increase_paged_capacity (capacity > threshold)",
        [] () {
            check(increase_paged_capacity(4096, 512, 4096) == 4096);
        }
    );

    suite.add_test(
        "increase_paged_capacity (capacity not a multiple of page size)",
        [] () {
            throws<std::invalid_argument>([] () {
                increase_paged_capacity(2, 3, 4096);
            });
        }
    );

    suite.add_test(
        "increase_paged_capacity (capacity not power of two)",
        [] () {
            throws<std::invalid_argument>([] () {
                check(increase_paged_capacity(5, 2, 4096) == 4);
            });
        }
    );

    suite.add_test(
        "increase_paged_capacity (page size not power of two)",
        [] () {
            throws<std::invalid_argument>([] () {
                check(increase_paged_capacity(2, 2, 4097) == 4);
            });
        }
    );

    suite.add_test("max", [] () {
        check(max(0, 0, 0, 0, 0, 0) == 0);
        check(max(1, 2, 3) == 3);
        check(max(-500, 0, 0) == 0);
        check(max(1, 2) == 2);
        check(max(5) == 5);
    });

    suite.add_test("min", [] () {
        check(min(0, 0, 0, 0, 0, 0) == 0);
        check(min(1, 2, 3) == 1);
        check(min(-500, 0, 0) == -500);
        check(min(1, 2) == 1);
        check(min(5) == 5);
    });

    suite.add_test("bitcount", [] () {
        check(bitcount(4) == 3);
        check(bitcount(1) == 1);
        check(bitcount(0) == 0);
    });

    suite.add_test("is_type_in_pack", [] () {
        check(is_type_in_pack<int, float, int>());
        check(!is_type_in_pack<char, float, int>());
    });

    suite.add_test("size_of", [] () {
        check(size_of<int[24]>() == sizeof(int) * 24);
        check(size_of<int>() == sizeof(int));
    });

    suite.add_test("is_address_aligned", [] () {
        uint32_t n;

        equals(is_address_aligned(&n), true);
        equals(
            is_address_aligned(
                reinterpret_cast<uint32_t*>(1)
            ),
            false
        );
    });

    suite.add_test("align_size", [] () {
        equals(align_size(15, 16), 16);
    });

    suite.add_test("swap", [] () {
        int a = 5, b = 6;

        ccl::swap(a, b);

        equals(a, 6);
        equals(b, 5);
    });

    suite.add_test("swap (pointers)", [] () {
        int a = 5, b = 6, *ap = &a, *bp = &b;

        ccl::swap(ap, bp);

        equals(ap, &b);
        equals(bp, &a);
    });

    suite.add_test("clamp", [] () {
        equals(clamp(1, 2, 3), 2); // Clamped to min
        equals(clamp(1, -5, 3), 1); // Not clamped
        equals(clamp(1, -5, 0), 0); // Clamped to max
    });

    suite.add_test("and", [] () {
        equals(and_(true, false, true), false);
        equals(and_(true, true), true);
    });

    suite.add_test("or", [] () {
        equals(or_(true, false, true), true);
        equals(or_(true, true), true);
    });

    return suite.main(argc, argv);
}
