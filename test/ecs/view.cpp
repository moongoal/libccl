#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/ecs/registry.hpp>
#include <ccl/ecs/view.hpp>

using namespace ccl;
using namespace ccl::ecs;

using test_registry = registry<counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("create", [] () {
        test_registry registry;

        const auto view CCLUNUSED = registry.view<int, double>();
    });

    suite.add_test("iterate", [] () {
        test_registry registry;

        const auto e1 = registry.add_entity();
        const auto e2 = registry.add_entity();
        const auto e3 = registry.add_entity();

        registry.add_components<int, double, float>(e1, 5, 10.0, 15.0f);
        registry.add_components<int, float>(e2, 6, 16.0f);
        registry.add_components<>(e3);

        const auto view = registry.view<entity_t, float>();
        int visited = 0;
        float float_total_value = 0;

        view.iterate([&visited, &float_total_value, e1, e2] (const entity_t& entity, const float& f) {
            visited += 1;
            float_total_value += f;

            if(entity != e1 && entity != e2) {
                fail();
            }
        });

        equals(visited, 2);
        equals(float_total_value, 31.0f);

        const auto view2 = registry.view<entity_t>();
        visited = 0;

        view2.iterate([&visited] (const entity_t x CCLUNUSED) {
            visited += 1;
        });

        equals(visited, 3);
    });

    return suite.main(argc, argv);
}
