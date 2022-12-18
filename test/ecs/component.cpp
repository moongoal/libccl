#include <functional>
#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/ecs/component.hpp>

using namespace ccl;
using namespace ccl::ecs;

constexpr uint32_t constructed_value = 0x1234;

struct spy {
    uint32_t construction_magic;
    std::function<void()> on_destroy;

    spy() {
        construction_magic = constructed_value;
    }

    ~spy() {
        if(on_destroy) {
            on_destroy();
        }

        construction_magic = 0;
    }
};

template<typename T>
using test_component = component<T, counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("get", [] () {
        test_component<int> c;

        c.get().push_back(2);

        check(c.get()[0] == 2);
    });

    suite.add_test("dtor", [] () {
        int destruction_count = 0;
        const auto on_destroy = [&destruction_count] () { destruction_count += 1; };

        {
            test_component<spy> c;

            c.get().emplace();
            c.get()[0].on_destroy = on_destroy;
        }

        equals(destruction_count, 1);
    });

    suite.add_test("cast from generic", [] () {
        test_component<int> c;

        c.get().emplace(2);
        component_i * const g = &c;

        equals(g->cast<int, counting_test_allocator>().get()[0], 2);
    }, skip_if_typechecking_disabled);

    suite.add_test("set", [] () {
        test_component<int> c;

        c.get().emplace(2);
        c.get().emplace(3);

        component_i * const g = &c;

        c.set(0, 5);

        equals(g->cast<int, counting_test_allocator>().get()[0], 5);
    });

    suite.add_test("push_back", [] () {
        test_component<int> c;

        c.push_back(2);
        c.push_back(3);

        equals(c.get()[0], 2);
        equals(c.get()[1], 3);
    });

    suite.add_test("push_back_from", [] () {
        test_component<int> c;
        test_component<int> d;

        c.push_back(2);
        c.push_back(3);

        d.push_back_from(1, c);
        d.push_back_from(0, c);

        equals(d.get()[0], 3);
        equals(d.get()[1], 2);
    });

    suite.add_test("emplace_empty", [] () {
        struct S { int value = 0x123; };
        test_component<S> c;

        c.emplace_empty();
        c.emplace_empty();

        equals(c.size(), 2UL);
        equals(c.get()[0].value, 0x123);
        equals(c.get()[1].value, 0x123);
    });

    suite.add_test("copy_from", [] () {
        test_component<int> c;
        test_component<int> d;

        c.push_back(5);
        c.push_back(6);

        d.push_back(0);
        d.push_back(0);

        d.copy_from(c, 0, 1);
        d.copy_from(c, 1, 0);

        equals(d.get()[0], 6);
        equals(d.get()[1], 5);
    });

    suite.add_test("move", [] () {
        test_component<int> c;

        c.push_back(5);
        c.push_back(6);

        c.move(1, 0);

        equals(c.get()[0], 6);
        equals(c.get()[1], 6);
    });

    suite.add_test("erase", [] () {
        test_component<int> c;

        c.push_back(5);
        c.push_back(6);

        c.erase(0);

        equals(c.size(), 1UL);
        equals(c.get()[0], 6);
    });

    suite.add_test("id", [] () {
        test_component<int> c;
        test_component<int> d;
        test_component<spy> e;

        equals(c.id(), d.id());
        differs(c.id(), e.id());
    });

    return suite.main(argc, argv);
}
