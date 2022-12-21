#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/ecs/component2.hpp>

using namespace ccl;
using namespace ccl::ecs;

using test_component = component2<counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("make", [] () {
        volatile test_component c = test_component::make<int>();
    });

    suite.add_test("size", [] () {
        test_component c = test_component::make<int>();

        equals(c.size(), 0ULL);

        c.emplace_empty();
        equals(c.size(), 1ULL);
    });

    suite.add_test("set", [] () {
        test_component c = test_component::make<int>();

        c.emplace_empty();
        c.emplace_empty();

        c.set<int>(0, 1);

        const int n = 2;
        c.set(1, &n);

        equals(c.get<int>(0), 1);
        equals(c.get<int>(1), n);
    });

    suite.add_test("push_back", [] () {
        test_component c = test_component::make<int>();

        c.push_back(1);

        const int n = 2;
        c.push_back(&n);

        equals(c.get<int>(0), 1);
        equals(c.get<int>(1), n);
    });

    suite.add_test("push_back_from", [] () {
        test_component c = test_component::make<int>();
        test_component d = test_component::make<int>();

        c.push_back(1);
        c.push_back(2);

        d.push_back_from(1, c);

        equals(d.get<int>(0), 2);
        equals(d.size(), 1ULL);
    });

    suite.add_test("emplace_empty", [] () {
        test_component c = test_component::make<int>();

        equals(c.size(), 0ULL);

        c.emplace_empty();
        c.emplace_empty<int>();

        equals(c.size(), 2ULL);
    });

    suite.add_test("erase", [] () {
        test_component c = test_component::make<int>();

        c.push_back(1);
        c.push_back(2);

        c.erase(0);

        equals(c.get<int>(0), 2);
        equals(c.size(), 1ULL);
    });

    suite.add_test("move", [] () {
        test_component c = test_component::make<int>();

        c.push_back(1);
        c.push_back(2);
        c.push_back(3);

        c.move(0, 2);

        equals(c.get<int>(0), 1);
        equals(c.get<int>(1), 2);
        equals(c.get<int>(2), 1);
        equals(c.size(), 3ULL);
    });

    suite.add_test("clone_empty", [] () {
        test_component c = test_component::make<int>();

        c.push_back(1);
        c.push_back(2);
        c.push_back(3);

        test_component d = c.clone_empty();

        d.push_back(5);

        equals(c.get<int>(0), 1);
        equals(c.get<int>(1), 2);
        equals(c.get<int>(2), 3);
        equals(c.size(), 3ULL);

        equals(d.get<int>(0), 5);
        equals(d.size(), 1ULL);
    });

    suite.add_test("move_from", [] () {
        test_component c = test_component::make<int>();

        c.push_back(1);
        c.push_back(2);
        c.push_back(3);

        test_component d = test_component::make<int>();

        d.push_back(5);
        d.move_from(c, 1, 0);

        equals(d.get<int>(0), 2);
        equals(d.size(), 1ULL);
    });

    suite.add_test("get", [] () {
        test_component c = test_component::make<int>();

        c.push_back(1);
        c.push_back(2);
        c.push_back(3);

        equals(c.get<int>(0), 1);
        equals(c.get<int>(1), 2);
        equals(c.get<int>(2), 3);
    });

    return suite.main(argc, argv);
}
