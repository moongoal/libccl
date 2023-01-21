#include <ccl/test/test.hpp>
#include <ccl/handle.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor (default)", [] () {
        untyped_versioned_handle h;

        equals(h.generation(), 0);
        equals(h.value(), 0);
    });

    suite.add_test("ctor (copy)", [] () {
        untyped_versioned_handle h = untyped_versioned_handle::make(5, 4);

        equals(h.generation(), 5);
        equals(h.value(), 4);
    });

    suite.add_test("ctor (value)", [] () {
        const auto raw = untyped_versioned_handle::underlying_type::make(2, 3);
        untyped_versioned_handle h{raw.get()};

        equals(h.generation(), 2);
        equals(h.value(), 3);
    });

    suite.add_test("equality assignment", [] () {
        const auto raw = untyped_versioned_handle::underlying_type::make(2, 3);
        untyped_versioned_handle h{raw.get()};
        untyped_versioned_handle b{5};

        b = h;

        equals(b.generation(), 2);
        equals(b.value(), 3);
    });

    suite.add_test("comparison <", [] () {
        untyped_versioned_handle a = untyped_versioned_handle::make(1, 2);
        untyped_versioned_handle b = untyped_versioned_handle::make(2, 3);

        check(a < b);
        check(!(b < a));
    });

    suite.add_test("comparison >", [] () {
        untyped_versioned_handle a = untyped_versioned_handle::make(1, 2);
        untyped_versioned_handle b = untyped_versioned_handle::make(2, 3);

        check(b > a);
        check(!(a > b));
    });

    suite.add_test("comparison <=", [] () {
        untyped_versioned_handle base = untyped_versioned_handle::make(1, 2);
        untyped_versioned_handle minor = untyped_versioned_handle::make(2, 1);
        untyped_versioned_handle equal = untyped_versioned_handle::make(1, 2);
        untyped_versioned_handle invalid_generation = untyped_versioned_handle::make(0, 2);
        untyped_versioned_handle major = untyped_versioned_handle::make(4, 3);

        check(!(base <= minor));
        check(minor <= base);

        check(base <= equal);

        check(!(base <= invalid_generation));
        check(!(invalid_generation <= base));

        check(base <= major);
        check(!(major <= base));
    });

    suite.add_test("comparison >=", [] () {
        untyped_versioned_handle base = untyped_versioned_handle::make(1, 2);
        untyped_versioned_handle minor = untyped_versioned_handle::make(2, 1);
        untyped_versioned_handle equal = untyped_versioned_handle::make(1, 2);
        untyped_versioned_handle invalid_generation = untyped_versioned_handle::make(0, 2);
        untyped_versioned_handle major = untyped_versioned_handle::make(4, 3);

        check(base >= minor);
        check(!(minor >= base));

        check(base >= equal);

        check(!(base >= invalid_generation));
        check(!(invalid_generation >= base));

        check(major >= base);
        check(!(base >= major));
    });

    suite.add_test("comparison ==", [] () {
        untyped_versioned_handle base = untyped_versioned_handle::make(1, 2);
        untyped_versioned_handle invalid_generation = untyped_versioned_handle::make(0, 2);
        untyped_versioned_handle same_generation = untyped_versioned_handle::make(1, 2);

        check(base == same_generation);
        check(!(base == invalid_generation));
    });

    suite.add_test("comparison !=", [] () {
        untyped_versioned_handle base = untyped_versioned_handle::make(1, 2);
        untyped_versioned_handle invalid_generation = untyped_versioned_handle::make(0, 2);
        untyped_versioned_handle same_generation = untyped_versioned_handle::make(1, 2);

        check(!(base != same_generation));
        check(base != invalid_generation);
    });

    return suite.main(argc, argv);
}
