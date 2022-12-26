#include <ccl/test/test.hpp>
#include <ccl/tagged-pointer.hpp>
#include <ccl/util.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("constants", [] () {
        using ptr_t = tagged_pointer<uint32_t>;

        align_address(reinterpret_cast<uint32_t*>(1), alignof(uint32_t));

        equals(ptr_t::alignment, 4U);
        equals(ptr_t::tag_mask, 3U);
        equals(ptr_t::address_mask, 0xFFFFFFFF'FFFFFFFCULL);
        equals(ptr_t::max_tag_value, 3U);
    });

    suite.add_test("ctor (default)", [] () {
        tagged_pointer<uint32_t> p;

        equals(p.raw(), 0U);
        equals(p.address(), null_v<uint32_t>);
        equals(p.tag(), 0U);
    });

    suite.add_test("ctor (value)", [] () {
        uint32_t n = 10;
        tagged_pointer<uint32_t> p{&n, 2};

        equals(p.raw(), reinterpret_cast<std::uintptr_t>(&n) | 2U);
        equals(p.address(), &n);
        equals(p.tag(), 2U);
    });

    suite.add_test("ctor (copy)", [] () {
        uint32_t n = 10;
        tagged_pointer<uint32_t> p{&n, 2};
        tagged_pointer<uint32_t> p2{p};

        equals(p2.raw(), reinterpret_cast<std::uintptr_t>(&n) | 2U);
        equals(p2.address(), &n);
        equals(p2.tag(), 2U);
    });

    suite.add_test("set_address", [] () {
        uint32_t n = 10;
        tagged_pointer<uint32_t> p{nullptr, 1};

        p.set_address(&n);

        equals(p.raw(), reinterpret_cast<std::uintptr_t>(&n) | 1U);
        equals(p.address(), &n);
        equals(p.tag(), 1U);
    });

    suite.add_test("set_address (bad alignment)", [] () {
        tagged_pointer<uint32_t> p;

        throws<std::invalid_argument>([&p] () {
            p.set_address(reinterpret_cast<uint32_t*>(1));
        });
    });

    suite.add_test("set_tag", [] () {
        uint32_t n = 10;
        tagged_pointer<uint32_t> p{&n, 1};

        p.set_tag(2);

        equals(p.raw(), reinterpret_cast<std::uintptr_t>(&n) | 2U);
        equals(p.address(), &n);
        equals(p.tag(), 2U);
    });

    suite.add_test("set_tag (too large value)", [] () {
        uint32_t n = 10;
        tagged_pointer<uint32_t> p{&n, 1};

        throws<std::out_of_range>([&p] () {
            p.set_tag(4);
        });
    });

    suite.add_test("operator T*", [] () {
        uint32_t n = 10;
        tagged_pointer<uint32_t> p{&n, 1};

        equals(static_cast<uint32_t*>(p), &n);
    });

    suite.add_test("operator *", [] () {
        uint32_t n = 10;
        tagged_pointer<uint32_t> p{&n, 1};

        equals(*p, n);
    });

    suite.add_test("operator ->", [] () {
        struct S { int n = 5; };
        S s;
        tagged_pointer<S> p{&s, 1};

        equals(p->n, s.n);
    });

    suite.add_test("operators ==, !=", [] () {
        tagged_pointer<uint32_t> p1{reinterpret_cast<uint32_t*>(4), 1};
        tagged_pointer<uint32_t> p2{reinterpret_cast<uint32_t*>(8), 1};
        tagged_pointer<uint32_t> p3{reinterpret_cast<uint32_t*>(4), 2};
        tagged_pointer<uint32_t> p4{reinterpret_cast<uint32_t*>(4), 1};

        check(p1 == p4);
        check(p1 != p2);
        check(p1 != p3);
    });

    suite.add_test("operators >, <", [] () {
        tagged_pointer<uint32_t> p1{reinterpret_cast<uint32_t*>(4), 1};
        tagged_pointer<uint32_t> p2{reinterpret_cast<uint32_t*>(8), 1};

        check(p2 > p1);
        check(p1 < p2);
    });

    suite.add_test("operators >=, <=", [] () {
        tagged_pointer<uint32_t> p1{reinterpret_cast<uint32_t*>(4), 1};
        tagged_pointer<uint32_t> p2{reinterpret_cast<uint32_t*>(8), 1};

        check(p2 >= p1);
        check(p1 <= p2);
    });

    return suite.main(argc, argv);
}
