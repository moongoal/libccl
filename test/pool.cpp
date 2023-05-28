#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/pool.hpp>

using namespace ccl;

using test_pool = pool<int, handle_expiry_policy::recycle>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("", [] () {

    });

    return suite.main(argc, argv);
}
