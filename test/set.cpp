#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/set.hpp>
#include <ccl/util.hpp>

using namespace ccl;

template<typename K, typename H = hash<K>>
using test_set = set<K, H, counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("insert one", []() {
        using my_set = test_set<int>;

        my_set x;

        x.insert(5);

        check(x.capacity() == my_set::minimum_capacity);
        check(x.contains(5));
    });

    suite.add_test("insert same", []() {
        using my_set = test_set<int>;

        my_set x;
        const int lvalue = 5;

        x.insert(5);
        x.insert(lvalue); // lvalue version
        x.insert(5); // rvalue version

        check(x.capacity() == my_set::minimum_capacity);
        check(x.contains(5));
    });

    suite.add_test("insert move", []() {
        struct S {
            int dummy;
            bool operator ==(const S& other) const { return dummy == other.dummy; }

            constexpr hash_t hash() const noexcept {
                return dummy;
            }
        };

        using my_set = test_set<S>;

        my_set x;

        x.insert(S{});

        check(x.capacity() == my_set::minimum_capacity);
    });

    suite.add_test("insert grow", []() {
        using my_set = test_set<int>;

        my_set x;

        for(std::size_t i = 0; i <= my_set::minimum_capacity; ++i) {
            x.insert(i);
        }

        check(x.capacity() > my_set::minimum_capacity);

        for(std::size_t i = 0; i <= my_set::minimum_capacity; ++i) {
            check(x.contains(i));
        }
    });

    suite.add_test("insert iterators", [] () {
        using my_set = test_set<int>;

        vector<int> v { 1, 2, 3 };
        my_set x;

        x.insert(v.begin(), v.end());

        check(x.contains(1));
        check(x.contains(2));
        check(x.contains(3));
    });

    suite.add_test("insert_range", [] () {
        using my_set = test_set<int>;

        vector<int> v { 1, 2, 3 };
        my_set x;

        x.insert_range(v);

        check(x.contains(1));
        check(x.contains(2));
        check(x.contains(3));
    });

    suite.add_test("insert initializer list", [] () {
        using my_set = test_set<int>;

        my_set x;

        x.insert({ 1, 2, 3 });

        check(x.contains(1));
        check(x.contains(2));
        check(x.contains(3));
    });

    suite.add_test("erase", [] () {
        using my_set = test_set<int>;

        my_set x;

        x.insert(1);
        x.erase(1);

        check(!x.contains(1));
    });

    suite.add_test("ctor (copy)", [] () {
        using my_set = test_set<int>;

        my_set x { vector<int> { 1, 2, 3 } };
        my_set y { x };

        check(x.contains(1));
        check(x.contains(2));
        check(x.contains(3));

        check(y.contains(1));
        check(y.contains(2));
        check(y.contains(3));
    });

    suite.add_test("ctor (range)", [] () {
        using my_set = test_set<int>;

        vector<int> v { 1, 2, 3 };
        my_set x { v };

        check(x.contains(1));
        check(x.contains(2));
        check(x.contains(3));
    });

    suite.add_test("ctor (range w/allocator)", [] () {
        using my_set = test_set<int>;

        vector<int> v { 1, 2, 3 };
        my_set x { v, 0, get_default_allocator<counting_test_allocator>() };

        check(x.contains(1));
        check(x.contains(2));
        check(x.contains(3));
    });

    suite.add_test("ctor (move)", [] () {
        using my_set = test_set<int>;

        vector<int> v { 1, 2, 3 };
        my_set x { v };
        my_set y { std::move(x) };

        check(y.contains(1));
        check(y.contains(2));
        check(y.contains(3));
    });

    suite.add_test("assignment operator (copy)", [] () {
        using my_set = test_set<int>;

        my_set x { vector<int> { 1, 2, 3 } };
        my_set y;

        y = x;

        check(x.contains(1));
        check(x.contains(2));
        check(x.contains(3));

        check(y.contains(1));
        check(y.contains(2));
        check(y.contains(3));
    });

     suite.add_test("assignment operator (move)", [] () {
        using my_set = test_set<int>;

        vector<int> v { 1, 2, 3 };
        my_set x { v };
        my_set y { vector<int>{ 4, 5, 6 } };

        y = std::move(x);

        check(y.contains(1));
        check(y.contains(2));
        check(y.contains(3));
    });

    suite.add_test("begin", [] () {
        using my_set = test_set<int>;

        vector<int> v { 1 };
        my_set x { v, 0, get_default_allocator<counting_test_allocator>() };

        auto it = x.begin();

        check(*it == 1);
        check(++it == x.end());
    });

    suite.add_test("last", [] () {
        using my_set = test_set<int>;

        vector<int> v { 2, 1 };
        my_set x { v, 0, get_default_allocator<counting_test_allocator>() };

        const auto it = --x.end();

        check(*it == 1 || *it == 2);
    });

    suite.add_test("clear", [] () {
        using my_set = test_set<int>;

        vector<int> v { 1, 2, 3 };
        my_set x { v, 0, get_default_allocator<counting_test_allocator>() };

        x.clear();

        check(x.begin() == x.end());
    });

    suite.add_test("find (not present)", []() {
        using my_set = test_set<int>;

        my_set x;

        check(x.find(1) == x.end());
    });

    suite.add_test("find", []() {
        using my_set = test_set<int>;

        my_set x;

        x.insert(1);
        x.insert(2);

        check(*x.find(1) == 1);
        check(*x.find(2) == 2);
    });

    suite.add_test("contains", []() {
        using my_set = test_set<int>;

        my_set x;

        x.insert(1);
        x.insert(2);

        check(x.contains(1));
        check(x.contains(2));
        check(!x.contains(3));
    });

    suite.add_test("reserve", []() {
        using my_set = test_set<int>;

        my_set x;

        const auto old_capacity = x.capacity();

        x.reserve(test_set<int>::minimum_capacity * 2);

        check(x.capacity() > old_capacity);
        check(is_power_2(x.capacity()));
    });

    suite.add_test("reserve (smaller capacity)", []() {
        using my_set = test_set<int>;

        my_set x;

        const auto old_capacity = x.capacity();
        x.reserve(test_set<int>::minimum_capacity - 1);

        equals(x.capacity(), old_capacity);
    });

    return suite.main(argc, argv);
}
