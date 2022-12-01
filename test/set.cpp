#include <ccl/test.hpp>
#include <ccl/set.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("insert one", []() {
        using my_set = set<int>;

        my_set x;

        x.insert(5);

        check(x.capacity() == my_set::minimum_capacity);
        check(x.contains(5));
    });

    suite.add_test("insert grow", []() {
        using my_set = set<int>;

        my_set x;

        for(size_t i = 0; i <= my_set::minimum_capacity; ++i) {
            x.insert(i);
        }

        check(x.capacity() > my_set::minimum_capacity);

        for(size_t i = 0; i <= my_set::minimum_capacity; ++i) {
            check(x.contains(i));
        }
    });

    suite.add_test("erase", [] () {
        using my_set = set<int>;

        my_set x;

        x.insert(1);
        x.erase(1);

        check(!x.contains(1));
    });

    suite.add_test("ctor (range)", [] () {
        using my_set = set<int>;

        vector<int> v { 1, 2, 3 };
        my_set x { v };

        check(x.contains(1));
        check(x.contains(2));
        check(x.contains(3));
    });

    suite.add_test("ctor (range w/allocator)", [] () {
        using my_set = set<int>;

        vector<int> v { 1, 2, 3 };
        my_set x { v, get_default_allocator() };

        check(x.contains(1));
        check(x.contains(2));
        check(x.contains(3));
    });

    suite.add_test("begin", [] () {
        using my_set = set<int>;

        vector<int> v { 1 };
        my_set x { v, get_default_allocator() };

        const auto it = x.begin();

        check(*it == 1);
        check(++it == x.end());
    });

    suite.add_test("last", [] () {
        using my_set = set<int>;

        vector<int> v { 2, 1 };
        my_set x { v, get_default_allocator() };

        const auto it = --x.end();

        check(*it == 1 || *it == 2);
    });

    suite.add_test("clear", [] () {
        using my_set = set<int>;

        vector<int> v { 1 , 2, 3 };
        my_set x { v, get_default_allocator() };

        x.clear();

        check(x.begin() == x.end());
    });

    suite.add_test("find (not present)", []() {
        using my_set = set<int>;

        my_set x;

        check(x.find(1) == x.end());
    });

    suite.add_test("find", []() {
        using my_set = set<int>;

        my_set x;

        x.insert(1);
        x.insert(2);

        check(*x.find(1) == 1);
        check(*x.find(2) == 2);
    });

    suite.add_test("contains", []() {
        using my_set = set<int>;

        my_set x;

        x.insert(1);
        x.insert(2);

        check(x.contains(1));
        check(x.contains(2));
        check(!x.contains(3));
    });

    return suite.main(argc, argv);
}
