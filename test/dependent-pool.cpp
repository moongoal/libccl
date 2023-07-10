#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/dependent-pool.hpp>

using namespace ccl;

using test_primary_pool = pool<int, handle_expiry_policy::recycle, counting_test_allocator>;
using test_dependent_pool = dependent_pool<double, test_primary_pool, counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("set/get", [] () {
        test_primary_pool primary_pool{9};
        test_dependent_pool pool{primary_pool, 1.0};

        const auto handle = primary_pool.acquire();
        const auto handle2 = primary_pool.acquire();

        pool.populate(handle);
        pool.populate(handle2);

        pool.set(handle, 5.2);
        pool.set(handle2, 10);

        equals(pool.get(handle), 5.2);
        equals(pool.get(handle2), 10);
    });

    suite.add_test("is_valid_handle", [] () {
        test_primary_pool primary_pool{9};
        test_dependent_pool pool{primary_pool, 1.0};

        const auto handle = primary_pool.acquire();

        check(pool.is_valid_handle(handle));
    });

    suite.add_test("is_valid_handle (invalid)", [] () {
        test_primary_pool primary_pool{9};
        test_dependent_pool pool{primary_pool, 1.0};

        const auto handle = primary_pool.acquire();
        primary_pool.release(handle);

        check(!pool.is_valid_handle(handle));
    });

    suite.add_test("populate", [] () {
        test_primary_pool primary_pool{9};
        test_dependent_pool pool{primary_pool, 1.0};

        const auto handle = primary_pool.acquire();
        const auto handle2 = primary_pool.acquire();

        pool.populate(handle);
        pool.populate(handle2, 10);

        equals(pool.get(handle), 1.0);
        equals(pool.get(handle2), 10);
    });

    return suite.main(argc, argv);
}
