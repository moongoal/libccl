#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/ecs/registry.hpp>
#include <ccl/ecs/registry-editor.hpp>

using namespace ccl;
using namespace ccl::ecs;

using test_registry = registry<counting_test_allocator>;
using test_registry_editor = ccl::ecs::internal::registry_editor<test_registry::allocator_type>;

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

    suite.add_test("add_entity (max entities)", [] () {
        test_registry registry;
        test_registry_editor editor{registry};

        editor.set_next_entity_id(test_registry::max_entity_id);

        throws<std::out_of_range>([&registry] () {
            const auto x CCLUNUSED = registry.add_entity();
        });
    });

    return suite.main(argc, argv);
}
