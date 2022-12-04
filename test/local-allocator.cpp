#include <ccl/test.hpp>
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
    });

    suite.add_test("owns not", [] () {
        local_allocator<16> allocator;

        check(!allocator.owns(nullptr));
    });

    return suite.main(argc, argv);
}
