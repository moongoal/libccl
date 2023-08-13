#include <ccl/test/test.hpp>
#include <ccl/memory/allocator.hpp>
#include <ccl/util.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("allocate/deallocate", [] () {
        allocator a;

        int * const x = reinterpret_cast<int*>(a.allocate(sizeof(int), sizeof(int) * 2, 0));
        a.deallocate(x);

        differs(x, nullptr);
        equals(x, align_address(x, sizeof(int) * 2));
    });

    suite.add_test("typed allocate", [] () {
        allocator a;

        int * const x = a.allocate<int>(1);
        a.deallocate(x);

        differs(x, nullptr);
    });

    suite.add_test("get_features", [] () {
        allocator a;

        equals(a.get_features(), 0);
    });

    suite.add_test("get_allocation_info", [] () {
        allocator a;
        allocation_info info;

        int * const x = a.allocate<int>(1);
        info = a.get_allocation_info(x);
        a.deallocate(x);

        equals(info.size, 0);
        equals(info.alignment, 0);
    });

    suite.add_test("owns", [] () {
        allocator a;

        int * const x = reinterpret_cast<int*>(a.allocate(sizeof(int), sizeof(int) * 2, 0));
        const bool owns_x = a.owns(x);
        a.deallocate(x);

        equals(owns_x, false);
    });

    suite.add_test("get_default_allocator", [] () {
        allocator * const a = get_default_allocator();

        equals(a, nullptr);
    });

    suite.add_test("set_default_allocator", [] () {
        allocator * const old = get_default_allocator();
        allocator a;

        set_default_allocator(a);
        allocator * const new_allocator = get_default_allocator();
        set_default_allocator(*old);

        equals(&a, new_allocator);
    });

    suite.add_test("get_default_allocator<allocator>", [] () {
        allocator * const old = get_default_allocator();
        allocator a;

        set_default_allocator(a);
        allocator * const new_allocator = get_default_allocator<allocator>();
        set_default_allocator(*old);

        equals(&a, new_allocator);
    });

    return suite.main(argc, argv);
}
