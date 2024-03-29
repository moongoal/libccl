#include <ccl/test/test.hpp>
#include <ccl/memory/composite-allocator.hpp>
#include <ccl/memory/local-allocator.hpp>

using namespace ccl;

struct spy_allocator {
    bool allocated = false;
    bool deallocated = false;
    bool does_own = false;

    CCLNODISCARD void* allocate(const std::size_t n_bytes CCLUNUSED, const std::size_t alignment CCLUNUSED, const allocation_flags flags CCLUNUSED = 0) { allocated = true; return nullptr; }
    allocation_info get_allocation_info(const void* const ptr CCLUNUSED) const { return {}; }
    void deallocate(void * const ptr CCLUNUSED) { deallocated = true; }
    CCLNODISCARD bool owns(const void * const ptr CCLUNUSED) const { return does_own; }
    allocator_feature_flags get_features() const noexcept { return CCL_ALLOCATOR_FEATURE_OWNERSHIP_QUERY_BIT; }
};

using test_local_allocator = local_allocator<16, local_allocator_policy::return_nullptr>;
using test_composite_allocator = composite_allocator<test_local_allocator, test_local_allocator>;
using spied_composite_allocator = composite_allocator<spy_allocator, spy_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("allocate first", [] () {
        test_local_allocator allocator1, allocator2;
        test_composite_allocator composite{allocator1, allocator2};

        const void * const ptr = composite.allocate<uint8_t>(16);

        check(allocator1.owns(ptr));
    });

    suite.add_test("allocate second", [] () {
        test_local_allocator allocator1, allocator2;
        test_composite_allocator composite{allocator1, allocator2};

        const void * const ptr1 = composite.allocate<uint8_t>(15);
        const void * const ptr2 = composite.allocate<uint8_t>(2);

        check(allocator1.owns(ptr1));
        check(allocator2.owns(ptr2));
    });

    suite.add_test("allocate third", [] () {
        test_local_allocator allocator1, allocator2;
        test_composite_allocator composite{allocator1, allocator2};

        const void * const ptr1 = composite.allocate(1, 1);
        const void * const ptr2 = composite.allocate(16, 1);

        check(allocator1.owns(ptr1));
        check(allocator2.owns(ptr2));
    });

    suite.add_test("deallocate first", [] () {
        spy_allocator allocator1, allocator2;
        spied_composite_allocator composite{allocator1, allocator2};

        allocator1.does_own = true;

        composite.deallocate(nullptr);
        check(allocator1.deallocated);
    });

    suite.add_test("deallocate second", [] () {
        spy_allocator allocator1, allocator2;
        spied_composite_allocator composite{allocator1, allocator2};

        composite.deallocate(nullptr);
        check(allocator2.deallocated);
    });

    suite.add_test("owns", [] () {
        test_local_allocator allocator1, allocator2;
        test_composite_allocator composite{allocator1, allocator2};

        const void * const ptr1 = composite.allocate<uint8_t>(15);
        const void * const ptr2 = composite.allocate<uint8_t>(2);

        equals(composite.owns(ptr1), false);
        equals(composite.owns(ptr2), false);
    });

    suite.add_test("get_features", [] () {
        test_local_allocator allocator1, allocator2;
        test_composite_allocator composite{allocator1, allocator2};

        equals(composite.get_features() & CCL_ALLOCATOR_FEATURE_OWNERSHIP_QUERY_BIT, false);
    });

    suite.add_test("get_allocation_info", [] () {
        test_local_allocator allocator1, allocator2;
        test_composite_allocator composite{allocator1, allocator2};

        const void * const ptr1 = composite.allocate<uint8_t>(15);
        const auto info = composite.get_allocation_info(ptr1);

        equals(info.alignment, 0);
        equals(info.size, 0);
    });

    return suite.main(argc, argv);
}
