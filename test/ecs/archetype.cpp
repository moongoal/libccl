#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/ecs/archetype.hpp>

using namespace ccl;
using namespace ccl::ecs;

using test_archetype = archetype<counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("id", [] () {
        test_archetype arch1 = test_archetype::make<int, double>();
        test_archetype arch2 = test_archetype::make<int>();
        test_archetype arch3 = test_archetype::make<int, float>();
        test_archetype arch4 = test_archetype::make<int, double>();

        equals(arch1.id(), arch4.id());
        differs(arch1.id(), arch2.id());
        differs(arch1.id(), arch3.id());
    });

    suite.add_test("has_component", [] () {
        test_archetype arch = test_archetype::make<int, double>();

        check(arch.has_component<int>());
        check(arch.has_component<double>());
        check(arch.has_component<entity_t>());
        check(!arch.has_component<float>());
    });

    suite.add_test("get_generic_component", [] () {
        test_archetype arch = test_archetype::make<int, double>();
        const auto entity_id = component<entity_t, counting_test_allocator>::make_id();
        const auto int_id = component<int, counting_test_allocator>::make_id();
        const auto double_id = component<double, counting_test_allocator>::make_id();

        check(arch.get_generic_component(entity_id));
        check(arch.get_generic_component(int_id));
        check(arch.get_generic_component(double_id));
    });

    suite.add_test("get_generic_component (not present)", [] () {
        test_archetype arch = test_archetype::make<int, double>();
        const auto float_id = component<float, counting_test_allocator>::make_id();

        throws<std::out_of_range>([&arch, float_id] () {
            arch.get_generic_component(float_id);
        });
    });

    suite.add_test("get_component", [] () {
        test_archetype arch = test_archetype::make<int, double>();

        arch.get_component<entity_t>();
        arch.get_component<int>();
        arch.get_component<double>();
    });

    suite.add_test("get_component (not present)", [] () {
        test_archetype arch = test_archetype::make<int, double>();

        throws<std::out_of_range>([&arch] () {
            arch.get_component<float>();
        });
    });

    suite.add_test("get_optional_component", [] () {
        test_archetype arch = test_archetype::make<int, double>();

        check(arch.get_optional_component<int>());
        check(arch.get_optional_component<double>());
        check(arch.get_optional_component<entity_t>());
        check(!arch.get_optional_component<float>());
    });

    suite.add_test("extend_id", [] () {
        test_archetype arch = test_archetype::make<int, double>();
        const hash_t expected = test_archetype::make_id<int, double, float>();

        equals(arch.extend_id<float>(), expected);
    });

    suite.add_test("add_entity", [] () {
        test_archetype arch = test_archetype::make<int, double>();

        equals(arch.add_entity(entity_t{1}), 0U);
        equals(arch.add_entity(entity_t{2}), 1U);
    });

    suite.add_test("set/get_entity_component", [] () {
        test_archetype arch = test_archetype::make<int, double>();
        const entity_t e{1};
        const entity_t e2{2};

        arch.add_entity(e);
        arch.add_entity(e2);

        arch.set_entity_component<int>(e, 5);
        arch.set_entity_component<double>(e, 10);

        arch.set_entity_component<int>(e2, 6);
        arch.set_entity_component<double>(e2, 11);

        equals(arch.get_entity_component<int>(e), 5);
        equals(arch.get_entity_component<double>(e), 10.0);

        equals(arch.get_entity_component<int>(e2), 6);
        equals(arch.get_entity_component<double>(e2), 11.0);
    });

    suite.add_test("copy_entity_components_from (widen)", [] () {
        test_archetype source = test_archetype::make<int, double>();
        test_archetype dest = test_archetype::make<int, double, float, char>();

        const entity_t e{1};
        const entity_t e2{2};

        source.add_entity(e);
        source.add_entity(e2);

        source.set_entity_component<int>(e, 5);
        source.set_entity_component<double>(e, 10);

        source.set_entity_component<int>(e2, 6);
        source.set_entity_component<double>(e2, 11);

        dest.add_entity(e);
        dest.add_entity(e2);

        dest.copy_entity_components_from(e, source);
        dest.copy_entity_components_from(e2, source);

        equals(dest.get_entity_component<int>(e), 5);
        equals(dest.get_entity_component<double>(e), 10.0);

        equals(dest.get_entity_component<int>(e2), 6);
        equals(dest.get_entity_component<double>(e2), 11.0);
    });

    suite.add_test("copy_entity_components_from (shrink)", [] () {
        test_archetype source = test_archetype::make<int, double, float, char>();
        test_archetype dest = test_archetype::make<int>();

        const entity_t e{1};
        const entity_t e2{2};

        source.add_entity(e);
        source.add_entity(e2);

        source.set_entity_component<int>(e, 5);
        source.set_entity_component<double>(e, 10);

        source.set_entity_component<int>(e2, 6);
        source.set_entity_component<double>(e2, 11);

        dest.add_entity(e);
        dest.add_entity(e2);

        dest.copy_entity_components_from(e, source);
        dest.copy_entity_components_from(e2, source);

        equals(dest.get_entity_component<int>(e), 5);
        equals(dest.get_entity_component<int>(e2), 6);

        throws<std::out_of_range>([&dest, e2] () {
            dest.get_entity_component<double>(e2);
        });
    });

    suite.add_test("remove_entity", [] () {
        test_archetype arch = test_archetype::make<int, double>();
        const entity_t e{1};
        const entity_t e2{2};
        const entity_t e3{3};

        arch.add_entity(e);
        arch.add_entity(e2);
        arch.add_entity(e3);

        arch.remove_entity(e2);
        arch.remove_entity(e);
        arch.remove_entity(e3);

        equals(arch.get_component<entity_t>().size(), 0ULL);
    });

    suite.add_test("move assignment operator (empty)", [] () {
        test_archetype arch = test_archetype::make<int, double>();
        test_archetype arch2 = test_archetype::make<int, double>();

        const entity_t e{1};
        const entity_t e2{2};
        const entity_t e3{3};

        arch.add_entity(e);
        arch.add_entity(e2);
        arch.add_entity(e3);

        arch2 = std::move(arch);

        check(arch2.has_entity(e));
        check(arch2.has_entity(e2));
        check(arch2.has_entity(e3));
    });

    suite.add_test("move assignment operator (initialized)", [] () {
        test_archetype arch = test_archetype::make<int, double>();
        test_archetype arch2 = test_archetype::make<int, double>();

        const entity_t e{1};
        const entity_t e2{2};
        const entity_t e3{3};

        arch.add_entity(e);
        arch.add_entity(e2);
        arch2.add_entity(e3);

        arch2 = std::move(arch);

        check(arch2.has_entity(e));
        check(arch2.has_entity(e2));
        check(!arch2.has_entity(e3));
    });

    suite.add_test("move ctor", [] () {
        test_archetype arch = test_archetype::make<int, double>();

        const entity_t e{1};
        const entity_t e2{2};
        const entity_t e3{3};

        arch.add_entity(e);
        arch.add_entity(e2);
        arch.add_entity(e3);

        test_archetype arch2{std::move(arch)};

        check(arch2.has_entity(e));
        check(arch2.has_entity(e2));
        check(arch2.has_entity(e3));
    });

    return suite.main(argc, argv);
}
