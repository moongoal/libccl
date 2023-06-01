#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/pool.hpp>

using namespace ccl;

using test_pool = pool<int, handle_expiry_policy::recycle, counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("acquire", [] () {
        test_pool pool{10};

        const auto handle = pool.acquire();

        equals(pool.get(handle), 10);
    });

    suite.add_test("release", [] () {
        test_pool pool{10};

        const auto handle = pool.acquire();

        pool.set(handle, 15);
        pool.release(handle);

        equals(pool.get(handle), 10);
    });

    suite.add_test("set/get", [] () {
        test_pool pool{9};

        const auto handle = pool.acquire();
        const auto handle2 = pool.acquire();

        pool.set(handle, 5);
        pool.set(handle2, 10);

        equals(pool.get(handle), 5);
        equals(pool.get(handle2), 10);
    });

    suite.add_test("is_valid_handle", [] () {
        test_pool pool{9};

        const auto handle = pool.acquire();

        check(pool.is_valid_handle(handle));
    });

    suite.add_test("is_valid_handle (invalid)", [] () {
        test_pool pool{9};

        const auto handle = pool.acquire();
        pool.release(handle);

        check(!pool.is_valid_handle(handle));
    });

    return suite.main(argc, argv);
}
