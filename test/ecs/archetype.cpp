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

    suite.add_test("get_component", [] () {
        test_archetype arch = test_archetype::make<int, double>();

        check(arch.get_component<entity_t>());
        check(arch.get_component<int>());
        check(arch.get_component<double>());
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



    return suite.main(argc, argv);
}
