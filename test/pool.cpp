#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/pool.hpp>

using namespace ccl;

using test_pool = pool<int, handle_expiry_policy::recycle>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("acquire/release", [] () {
        test_pool pool;

        pool.release(pool.acquire());
    });

    suite.add_test("set/get", [] () {
        test_pool pool;

        const auto handle = pool.acquire();
        const auto handle2 = pool.acquire();

        pool.set(handle, 5);
        pool.set(handle2, 10);

        equals(pool.get(handle), 5);
        equals(pool.get(handle2), 10);
    });

    return suite.main(argc, argv);
}
