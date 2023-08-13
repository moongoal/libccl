#include <ccl/test/test.hpp>
#include <ccl/memory/allocator.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("allocate/deallocate", [] () {
        null_allocator a;

        int * const x = reinterpret_cast<int*>(a.allocate(sizeof(int), sizeof(int) * 2, 0));

        equals(x, nullptr);
    });

    suite.add_test("get_features", [] () {
        null_allocator a;

        equals(a.get_features(), 0);
    });

    suite.add_test("get_allocation_info", [] () {
        null_allocator a;
        allocation_info info;

        int * const x = a.allocate<int>(1);
        info = a.get_allocation_info(x);

        equals(info.size, 0);
        equals(info.alignment, 0);
    });

    suite.add_test("owns", [] () {
        null_allocator a;

        int * const x = reinterpret_cast<int*>(a.allocate(sizeof(int), sizeof(int) * 2, 0));
        const bool owns_x = a.owns(x);

        equals(owns_x, false);
    });

    suite.add_test("get_default_allocator<null_allocator>", [] () {
        equals(get_default_allocator<allocator>(), nullptr);
    });

    return suite.main(argc, argv);
}
