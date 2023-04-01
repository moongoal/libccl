#include <ccl/test/test.hpp>
#include <ccl/memory/local-allocator.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("allocate", [] () {
        local_allocator<16> allocator;

        uint8_t * const ptr = allocator.allocate<uint8_t>(8);

        check(ptr);
    });

    suite.add_test("allocate all", [] () {
        local_allocator<16> allocator;

        uint8_t * const ptr = allocator.allocate<uint8_t>(16);

        check(ptr);
    });

    suite.add_test("allocate exceed (throw)", [] () {
        local_allocator<16> allocator;

        uint8_t * const ptr CCLUNUSED = allocator.allocate<uint8_t>(16);

        throws<std::bad_alloc>([&allocator] () {
            auto * const ptr_fail CCLUNUSED = allocator.allocate(1);
        });
    });

    suite.add_test("allocate exceed (nullptr)", [] () {
        local_allocator<16, local_allocator_policy::return_nullptr> allocator;

        uint8_t * const ptr CCLUNUSED = allocator.allocate<uint8_t>(16);
        auto * const ptr_fail = allocator.allocate(1);

        check(!ptr_fail);
    });

    suite.add_test("deallocate", [] () {
        local_allocator<16> allocator;

        uint8_t * const ptr CCLUNUSED = allocator.allocate<uint8_t>(16);

        allocator.deallocate(ptr);

        throws<std::bad_alloc>([&allocator] () {
            auto * const ptr_fail CCLUNUSED = allocator.allocate(1);
        });
    });

    suite.add_test("clear", [] () {
        local_allocator<16> allocator;

        uint8_t * ptr CCLUNUSED = allocator.allocate<uint8_t>(16);

        allocator.clear();

        ptr = allocator.allocate<uint8_t>(16);
    });

    suite.add_test("owns", [] () {
        local_allocator<16> allocator;

        uint8_t * const ptr = allocator.allocate<uint8_t>(16);

        check(allocator.owns(ptr));
        check(allocator.get_features() & CCL_ALLOCATOR_FEATURE_OWNERSHIP_QUERY_BIT);
    });

    suite.add_test("owns not", [] () {
        local_allocator<16> allocator;

        check(!allocator.owns(nullptr));
    });

    suite.add_test("get_allocation_info", [] () {
        local_allocator<16> allocator;
        const auto * ptr = allocator.allocate<int>(1);
        const auto info = allocator.get_allocation_info(ptr);

        equals(info.alignment, 0);
        equals(info.size, 0);
        check(~allocator.get_features() & CCL_ALLOCATOR_FEATURE_ALLOCATION_INFO_BIT);
    });

    suite.add_test("get_used_memory_size", [] () {
        local_allocator<16> allocator;

        equals(allocator.get_used_memory_size(), 0);

        const auto * ptr CCLUNUSED = allocator.allocate<int>(1);

        equals(allocator.get_used_memory_size(), 4);
    });

    return suite.main(argc, argv);
}
