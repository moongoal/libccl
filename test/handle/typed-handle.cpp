#include <ccl/test/test.hpp>
#include <ccl/handle/typed-handle.hpp>

using namespace ccl;

using generic_typed_handle = typed_handle<void>;

class A {};
class B : public A {};
class C {};

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor (default)", [] () {
        generic_typed_handle h;

        equals(h.value(), 0);
    });

    suite.add_test("ctor (value)", [] () {
        generic_typed_handle h{4};

        equals(h.value(), 4);
    });

    suite.add_test("ctor (copy)", [] () {
        const auto raw = generic_typed_handle{3};
        generic_typed_handle h{raw};

        equals(h.value(), 3);
    });

    suite.add_test("equality assignment", [] () {
        const auto h = generic_typed_handle{3};
        generic_typed_handle b{5};

        b = h;

        equals(b.value(), 3);
    });

    suite.add_test("comparison <", [] () {
        generic_typed_handle a = generic_typed_handle{2};
        generic_typed_handle b = generic_typed_handle{3};

        check(a < b);
        check(!(b < a));
    });

    suite.add_test("comparison >", [] () {
        generic_typed_handle a{2};
        generic_typed_handle b{3};

        check(b > a);
        check(!(a > b));
    });

    suite.add_test("comparison <=", [] () {
        generic_typed_handle base{2};
        generic_typed_handle minor{1};
        generic_typed_handle equal{2};
        generic_typed_handle major{3};

        check(!(base <= minor));
        check(minor <= base);

        check(base <= equal);

        check(base <= major);
        check(!(major <= base));
    });

    suite.add_test("comparison >=", [] () {
        generic_typed_handle base{2};
        generic_typed_handle minor{1};
        generic_typed_handle equal{2};
        generic_typed_handle major{3};

        check(base >= minor);
        check(!(minor >= base));

        check(base >= equal);

        check(major >= base);
        check(!(base >= major));
    });

    suite.add_test("comparison ==", [] () {
        generic_typed_handle first{2};
        generic_typed_handle second{2};
        generic_typed_handle third{3};

        check(first == second);
        check(!(first == third));
    });

    suite.add_test("comparison !=", [] () {
        generic_typed_handle first{2};
        generic_typed_handle second{3};
        generic_typed_handle third{2};

        check(!(first != third));
        check(first != second);
    });

    suite.add_test("static_handle_cast", [] () {
        using base_handle_type = typed_handle<A>;
        using subc_handle_type = typed_handle<B>;

        const auto base_handle = base_handle_type{1};
        const auto subc_handle = subc_handle_type{1};
        const base_handle_type converted_handle = static_handle_cast<A>(subc_handle);

        equals(converted_handle, base_handle);
    });

    suite.add_test("reinterpret_handle_cast", [] () {
        using base_handle_type = typed_handle<A>;
        using subc_handle_type = typed_handle<C>;

        const auto base_handle = base_handle_type{1};
        const auto subc_handle = subc_handle_type{1};
        const base_handle_type converted_handle = reinterpret_handle_cast<A>(subc_handle);

        equals(converted_handle, base_handle);
    });

    return suite.main(argc, argv);
}
