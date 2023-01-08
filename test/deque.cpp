#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/deque.hpp>

using namespace ccl;

template<typename T>
using test_deque = deque<T, counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor", [] () {
        test_deque<int> q;

        equals(q.capacity(), 0U);
        equals(q.capacity_back(), 0U);
        equals(q.capacity_front(), 0U);
        equals(q.size(), 0U);
        equals(q.is_empty(), true);
        equals(q.data(), nullptr);
    });

    suite.add_test("reserve (empty)", [] () {
        test_deque<int> q;

        q.reserve(16);

        check(q.capacity() >= 16U);
        equals(q.capacity_back(), 8U);
        equals(q.capacity_front(), 8U);
        equals(q.size(), 0U);
        equals(q.is_empty(), true);
        differs(q.data(), nullptr);
    });

    suite.add_test("push_back (empty)", [] () {
        test_deque<int> q;

        q.push_back(5);

        differs(q.capacity(), 0U);
        equals(q.size(), 1U);
        equals(q.is_empty(), false);
        differs(q.data(), nullptr);
    });

    return suite.main(argc, argv);
}
