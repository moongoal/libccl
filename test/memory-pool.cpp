#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/memory-pool.hpp>

using namespace ccl;

template<typename T>
using test_memory_pool = memory_pool<T, counting_test_allocator>;

struct S {
    uint8_t x[3]; // Three bytes shall be aligned to four by the memory pool
};

int main(int argc, char **argv) {
    test_suite suite;

    static_assert(
        test_memory_pool<S>::object_page_count * sizeof(S) <= test_memory_pool<S>::memory_vector_type::page_size,
        "Number of items storable in page does not match page size data."
    );

    suite.add_test("acquire", [] () {
        test_memory_pool<int> pool;

        int * const ptr1 = pool.acquire();
        int * const ptr2 = pool.acquire();
        int * const ptr3 = pool.acquire();

        *ptr3 = 3;
        *ptr2 = 2;
        *ptr1 = 1;

        equals(*ptr3, 3);
        equals(*ptr2, 2);
        equals(*ptr1, 1);
    });

    suite.add_test("release", [] () {
        test_memory_pool<int> pool;

        int * ptr1 = pool.acquire();
        int * ptr2 = pool.acquire();
        int * ptr3 = pool.acquire();

        *ptr3 = 3;
        *ptr1 = 1;

        pool.release(ptr2);
        ptr2 = pool.acquire();

        *ptr2 = 2;

        equals(*ptr3, 3);
        equals(*ptr2, 2);
        equals(*ptr1, 1);
    });

    suite.add_test("acquire (entire page)", [] () {
        test_memory_pool<int> pool;

        for(unsigned int i = 0; i < test_memory_pool<int>::object_page_count; ++i) {
            int * _ CCLUNUSED = pool.acquire();
        }

        // These will now be acquired from another page
        int * ptr1 = pool.acquire();
        int * ptr2 = pool.acquire();
        int * ptr3 = pool.acquire();

        *ptr3 = 3;
        *ptr1 = 1;

        pool.release(ptr2);
        ptr2 = pool.acquire();

        *ptr2 = 2;

        equals(*ptr3, 3);
        equals(*ptr2, 2);
        equals(*ptr1, 1);
    });

    return suite.main(argc, argv);
}
