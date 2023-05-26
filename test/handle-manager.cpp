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

    return suite.main(argc, argv);
}
