#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/handle-manager.hpp>

using namespace ccl;

template<handle_expiry_policy ExpiryPolicy>
using test_handle_manager = handle_manager<int, ExpiryPolicy, counting_test_allocator>;

using test_recycle_handle_manager = test_handle_manager<handle_expiry_policy::recycle>;
using test_discard_handle_manager = test_handle_manager<handle_expiry_policy::discard>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("acquire", [] () {
        test_recycle_handle_manager manager;

        const auto handle1 = manager.acquire();
        const auto handle2 = manager.acquire();

        differs(handle1, handle2);
    });

    suite.add_test("release", [] () {
        test_recycle_handle_manager manager;

        for(size_t i = 0; i < test_recycle_handle_manager::vector_type::page_size; ++i) {
            manager.release(manager.acquire());
        }

        const auto handle = manager.acquire();

        equals(handle.generation(), 1);
        equals(handle.value(), 0);
    });

    suite.add_test("acquire/release cycle", [] () {
        test_recycle_handle_manager manager;

        const auto handle1 = manager.acquire();
        const auto handle2 = manager.acquire();
        manager.release(handle1);
        const auto handle3 = manager.acquire();

        differs(handle1, handle3);
        differs(handle2, handle3);
    });

    suite.add_test("reset", []() {
        test_recycle_handle_manager manager;

        const auto handle1 = manager.acquire();
        manager.reset();
        const auto handle2 = manager.acquire();

        equals(handle1, handle2);
    });

    suite.add_test("is_valid_handle", []() {
        test_recycle_handle_manager manager;

        const auto handle1 = manager.acquire();
        const auto handle2 = manager.acquire();

        manager.release(handle1);

        check(!manager.is_valid_handle(handle1));
        check(manager.is_valid_handle(handle2));
    });

    suite.add_test("handle expiry (recycle)", []() {
        test_recycle_handle_manager manager;

        /*
         * +1 because the highest generation value is
         * a valid generation in recycle mode.
         */
        const size_t total_iterations =
            test_recycle_handle_manager::vector_type::page_size
            * (test_recycle_handle_manager::handle_type::max_generation + 1);

        for(size_t i = 0; i < total_iterations; ++i) {
            manager.release(manager.acquire());
        }

        const auto handle = manager.acquire();

        equals(handle.raw(), 0);
    });

    suite.add_test("handle expiry (discard)", []() {
        test_discard_handle_manager manager;

        const size_t total_iterations =
            test_recycle_handle_manager::vector_type::page_size
            * test_recycle_handle_manager::handle_type::max_generation;

        for(size_t i = 0; i < total_iterations; ++i) {
            manager.release(manager.acquire());
        }

        const auto handle = manager.acquire();

        equals(handle.generation(), 0);
        equals(handle.value(), test_recycle_handle_manager::vector_type::page_size);
    });

    suite.add_test("reset_expired", [] () {
        test_discard_handle_manager manager;

        const size_t total_iterations =
            test_recycle_handle_manager::vector_type::page_size
            * test_recycle_handle_manager::handle_type::max_generation;

        for(size_t i = 0; i < total_iterations; ++i) {
            manager.release(manager.acquire());
        }

        manager.reset_expired();

        const auto handle = manager.acquire();

        equals(handle.raw(), 0);
    });

    return suite.main(argc, argv);
}
