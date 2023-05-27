#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/handle-manager.hpp>

using namespace ccl;

template<handle_manager_expiry_policy ExpiryPolicy>
using test_handle_manager = handle_manager<int, ExpiryPolicy, counting_test_allocator>;

using test_recycle_handle_manager = test_handle_manager<handle_manager_expiry_policy::recycle>;
using test_discard_handle_manager = test_handle_manager<handle_manager_expiry_policy::discard>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("acquire", [] () {
        test_recycle_handle_manager manager;

        const auto handle1 = manager.acquire();
        const auto handle2 = manager.acquire();

        differs(handle1, handle2);
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

    suite.add_test("release", [] () {
        test_recycle_handle_manager manager;

        for(size_t i = 0; i < test_recycle_handle_manager::vector_type::page_size; ++i) {
            manager.release(manager.acquire());
        }

        const auto handle = manager.acquire();

        equals(handle.generation(), 1);
        equals(handle.value(), 0);
    });

    return suite.main(argc, argv);
}
