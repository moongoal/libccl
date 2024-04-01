#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/pool.hpp>

using namespace ccl;

using test_pool = pool<int, counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("acquire one", [] () {
        test_pool pool;

        differs(pool.acquire(), nullptr);
    });

    suite.add_test("acquire one", [] () {
        test_pool pool;

        differs(pool.acquire(), nullptr);
    });

    suite.add_test("release one", [] () {
        test_pool pool;

        int * const x = pool.acquire();

        *x = 0xabcdef;

        pool.release(x);
        equals(*pool.acquire(), 0xabcdef);
    });

    suite.add_test("acquire entire page", [] () {
        test_pool pool;

        for(size_t i = 0; i < test_pool::item_collection_type::page_size; ++i) {
            differs(pool.acquire(), nullptr);
        }

        // Extra item to trigger resize
        differs(pool.acquire(), nullptr);
    });

    return suite.main(argc, argv);
}
