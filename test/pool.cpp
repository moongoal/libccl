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

    suite.add_test("get", [] () {
        test_pool pool;

        const auto handle = pool.acquire();

        differs(pool.get(handle), nullptr);
    });

    suite.add_test("get (invalid)", [] () {
        test_pool pool;

        const auto handle = test_pool::handle_type::make(100, 100);

        equals(pool.get(handle), nullptr);
    });

    suite.add_test("set", [] () {
        test_pool pool;

        const auto handle = pool.acquire();

        pool.set(handle, 5);

        equals(*pool.get(handle), 5);
    });

    return suite.main(argc, argv);
}
