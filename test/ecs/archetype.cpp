#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/ecs/archetype.hpp>

using namespace ccl;
using namespace ccl::ecs;

using test_archetype = archetype<counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("id", [] () {
        test_archetype arch{123};
    });

    return suite.main(argc, argv);
}
