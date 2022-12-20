#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/ecs/registry.hpp>

using namespace ccl;
using namespace ccl::ecs;

using test_registry = registry<counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("add_entity", []() {
        test_registry registry;

        const entity_t e1 = registry.add_entity();
        const entity_t e2 = registry.add_entity();

        differs(e1, e2);
        equals(e1.generation(), 0U);
        check(e1.value() < e2.value());
    });

    return suite.main(argc, argv);
}
