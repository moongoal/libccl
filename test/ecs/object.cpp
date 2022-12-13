#include <functional>
#include <ccl/test/test.hpp>
#include <ccl/ecs/object.hpp>

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

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("get", [] () {
        check(object{2}.get() == 2);
    });

    suite.add_test("dtor", [] () {
        int destruction_count = 0;
        const auto on_destroy = [&destruction_count] () { destruction_count += 1; };
        object<spy> * const s = new object<spy>;
        generic_object * const g = s;

        s->get().on_destroy = on_destroy;

        delete g;

        equals(destruction_count, 1);
    });

    suite.add_test("cast from generic", [] () {
        object obj{2};
        generic_object * const g = &obj;

        equals(g->cast<int>().get(), 2);
    }, skip_if_typechecking_disabled);

    return suite.main(argc, argv);
}
