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

    suite.add_test("add_components", []() {
        test_registry registry;

        const entity_t e = registry.add_entity();

        registry.add_components<int, float>(e, 5, 2.0f);

        const typename test_registry::archetype * const arch = registry.get_entity_archetype(e);

        equals(arch->get_entity_component<int>(e), 5);
        equals(arch->get_entity_component<float>(e), 2.0f);
    });

    // suite.add_test("get_entity_archetype", []() {
    //     test_registry registry;

    //     const entity_t e = registry.add_entity();

    //     registry.add_components<int, float>(e, 5, 2.0f);

    //     const typename test_registry::archetype * const old_arch = registry.get_entity_archetype(e);

    //     registry.add_components<double>(e, 3.0);

    //     const typename test_registry::archetype * const arch = registry.get_entity_archetype(e);

    //     equals(arch->get_entity_component<int>(e), 5);
    //     equals(arch->get_entity_component<float>(e), 2.0f);
    //     equals(arch->get_entity_component<double>(e), 3.0);
    //     differs(old_arch, arch);
    // });

    return suite.main(argc, argv);
}
