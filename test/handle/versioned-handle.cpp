#include <ccl/test/test.hpp>
#include <ccl/handle/versioned-handle.hpp>

using namespace ccl;

using generic_versioned_handle = versioned_handle<void>;

class A {};
class B : public A {};
class C {};

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor (default)", [] () {
        generic_versioned_handle h;

        equals(h.generation(), 0);
        equals(h.value(), generic_versioned_handle::invalid_handle_value);
    });

    suite.add_test("ctor (copy)", [] () {
        generic_versioned_handle h = generic_versioned_handle::make(5, 4);

        equals(h.generation(), 5);
        equals(h.value(), 4);
    });

    suite.add_test("ctor (value)", [] () {
        const auto raw = generic_versioned_handle::underlying_type::make(2, 3);
        generic_versioned_handle h{raw.get()};

        equals(h.generation(), 2);
        equals(h.value(), 3);
    });

    suite.add_test("equality assignment", [] () {
        const auto raw = generic_versioned_handle::underlying_type::make(2, 3);
        generic_versioned_handle h{raw.get()};
        generic_versioned_handle b{5};

        b = h;

        equals(b.generation(), 2);
        equals(b.value(), 3);
    });

    suite.add_test("comparison <", [] () {
        generic_versioned_handle a = generic_versioned_handle::make(1, 2);
        generic_versioned_handle b = generic_versioned_handle::make(2, 3);

        check(a < b);
        check(!(b < a));
    });

    suite.add_test("comparison >", [] () {
        generic_versioned_handle a = generic_versioned_handle::make(1, 2);
        generic_versioned_handle b = generic_versioned_handle::make(2, 3);

        check(b > a);
        check(!(a > b));
    });

    suite.add_test("comparison <=", [] () {
        generic_versioned_handle base = generic_versioned_handle::make(1, 2);
        generic_versioned_handle minor = generic_versioned_handle::make(2, 1);
        generic_versioned_handle equal = generic_versioned_handle::make(1, 2);
        generic_versioned_handle invalid_generation = generic_versioned_handle::make(0, 2);
        generic_versioned_handle major = generic_versioned_handle::make(4, 3);

        check(!(base <= minor));
        check(minor <= base);

        check(base <= equal);

        check(!(base <= invalid_generation));
        check(!(invalid_generation <= base));

        check(base <= major);
        check(!(major <= base));
    });

    suite.add_test("comparison >=", [] () {
        generic_versioned_handle base = generic_versioned_handle::make(1, 2);
        generic_versioned_handle minor = generic_versioned_handle::make(2, 1);
        generic_versioned_handle equal = generic_versioned_handle::make(1, 2);
        generic_versioned_handle invalid_generation = generic_versioned_handle::make(0, 2);
        generic_versioned_handle major = generic_versioned_handle::make(4, 3);

        check(base >= minor);
        check(!(minor >= base));

        check(base >= equal);

        check(!(base >= invalid_generation));
        check(!(invalid_generation >= base));

        check(major >= base);
        check(!(base >= major));
    });

    suite.add_test("comparison ==", [] () {
        generic_versioned_handle base = generic_versioned_handle::make(1, 2);
        generic_versioned_handle invalid_generation = generic_versioned_handle::make(0, 2);
        generic_versioned_handle same_generation = generic_versioned_handle::make(1, 2);

        check(base == same_generation);
        check(!(base == invalid_generation));
    });

    suite.add_test("comparison !=", [] () {
        generic_versioned_handle base = generic_versioned_handle::make(1, 2);
        generic_versioned_handle invalid_generation = generic_versioned_handle::make(0, 2);
        generic_versioned_handle same_generation = generic_versioned_handle::make(1, 2);

        check(!(base != same_generation));
        check(base != invalid_generation);
    });

    suite.add_test("static_handle_cast", [] () {
        using base_handle_type = versioned_handle<A>;
        using subc_handle_type = versioned_handle<B>;

        const auto base_handle = base_handle_type::make(1, 2);
        const auto subc_handle = subc_handle_type::make(1, 2);
        const base_handle_type converted_handle = static_handle_cast<A>(subc_handle);

        equals(converted_handle, base_handle);
    });

    suite.add_test("reinterpret_handle_cast", [] () {
        using base_handle_type = versioned_handle<A>;
        using subc_handle_type = versioned_handle<C>;

        const auto base_handle = base_handle_type::make(1, 2);
        const auto subc_handle = subc_handle_type::make(1, 2);
        const base_handle_type converted_handle = reinterpret_handle_cast<A>(subc_handle);

        equals(converted_handle, base_handle);
    });

    suite.add_test("is_null", [] () {
        generic_versioned_handle null_handle;
        generic_versioned_handle non_null_handle{0};

        check(null_handle.is_null());
        check(!non_null_handle.is_null());
    });

    return suite.main(argc, argv);
}
