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
        generic_component * const g = &c;

        equals(g->cast<int, counting_test_allocator>().get()[0], 2);
    }, skip_if_typechecking_disabled);

    return suite.main(argc, argv);
}
