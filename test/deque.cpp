#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/deque.hpp>

using namespace ccl;

template<typename T>
using test_deque = deque<T, counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor", [] () {
        test_deque<int> n;

        equals(n.capacity(), 0U);
        equals(n.capacity_back(), 0U);
        equals(n.capacity_front(), 0U);
        equals(n.size(), 0U);
        equals(n.is_empty(), true);
        equals(n.data(), nullptr);
    });

    return suite.main(argc, argv);
}
