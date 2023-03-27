#include <ccl/test/test.hpp>
#include <ccl/memory/local-allocator.hpp>
#include <ccl/internal/optional-allocator.hpp>

using namespace ccl;

using test_buffering_allocator = local_buffering_allocator<int, 16>;

template<typename Allocator>
struct test_struct : public internal::with_optional_allocator<Allocator> {
    using alloc = internal::with_optional_allocator<Allocator>;

    test_struct(Allocator * const allocator) : alloc{allocator} {}

    Allocator *get_allocator() {
        return alloc::get_allocator();
    }

    Allocator *get_allocator() const {
        return alloc::get_allocator();
    }

    test_struct& operator =(const test_struct &other) {
        alloc::operator=(other);

        return *this;
    }

    static constexpr bool is_allocator_stateless() noexcept {
        return alloc::is_allocator_stateless();
    }
};

static test_buffering_allocator default_test_allocator;

namespace ccl {
    template<>
    test_buffering_allocator * get_default_allocator() {
        return &default_test_allocator;
    }
}

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("stateless ctor (default)", [] () {
        test_struct<allocator> s{nullptr};

        equals(reinterpret_cast<allocator*>(&s), s.get_allocator());
    });

    suite.add_test("stateless ctor (value)", [] () {
        allocator a;
        test_struct<allocator> s{&a};

        equals(reinterpret_cast<allocator*>(&s), s.get_allocator());
    });

    suite.add_test("stateful ctor (default)", [] () {
        test_struct<test_buffering_allocator> s{nullptr};

        equals(&default_test_allocator, s.get_allocator());
    });

    suite.add_test("stateful ctor (value)", [] () {
        test_buffering_allocator a;
        test_struct s{&a};

        equals(&a, s.get_allocator());
    });

    suite.add_test("stateless operator =", [] () {
        test_struct<allocator> s1{nullptr};
        test_struct<allocator> s2{nullptr};

        s1 = s2;

        equals(reinterpret_cast<allocator*>(&s2), s2.get_allocator());
    });

    suite.add_test("stateful operator =", [] () {
        test_buffering_allocator a;
        test_struct<test_buffering_allocator> s1{nullptr};
        test_struct<test_buffering_allocator> s2{&a};

        s2 = s1;

        equals(&default_test_allocator, s2.get_allocator());
    });

    suite.add_test("stateless get_allocator", [] () {
        allocator a;
        test_struct<allocator> s{&a};

        equals(reinterpret_cast<allocator*>(&s), s.get_allocator());
    });

    suite.add_test("stateful get_allocator", [] () {
        test_buffering_allocator a;
        test_struct s{&a};

        equals(&a, s.get_allocator());
    });

    suite.add_test("stateless is_stateless_allocator", [] () {
        using t = test_struct<allocator>;

        equals(t::is_allocator_stateless(), true);
    });

    suite.add_test("stateful is_stateless_allocator", [] () {
        using t = test_struct<test_buffering_allocator>;

        equals(t::is_allocator_stateless(), false);
    });

    return suite.main(argc, argv);
}
