#include <ccl/test.hpp>
#include <ccl/hashtable.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("insert one", []() {
        using my_hashtable = hashtable<int, float>;

        my_hashtable x;

        x.insert(5, 1);

        check(x.capacity() == my_hashtable::minimum_capacity);
    });

    suite.add_test("insert grow", []() {
        using my_hashtable = hashtable<int, float>;

        my_hashtable x;

        for(size_t i = 0; i <= my_hashtable::minimum_capacity; ++i) {
            x.insert(i, 1);
        }

        check(x.capacity() > my_hashtable::minimum_capacity);
    });

    suite.add_test("operator []", []() {
        using my_hashtable = hashtable<int, float>;

        my_hashtable x;

        for(size_t i = 0; i < my_hashtable::minimum_capacity; ++i) {
            x.insert(i, i + 1);
        }

        for(size_t i = 0; i < my_hashtable::minimum_capacity; ++i) {
            check(x[i] == i + 1);
        }
    });

    suite.add_test("operator [] not present", []() {
        using my_hashtable = hashtable<int, float>;

        my_hashtable x;

        throws<std::invalid_argument>([&x]() {
            x[1];
        });
    });

    suite.add_test("erase", [] () {
        using my_hashtable = hashtable<int, float>;

        my_hashtable x;

        x.insert(1, 1);
        x.erase(1);

        throws<std::invalid_argument>([&x]() {
            x[1];
        });
    });

    suite.add_test("emplace", [] () {
        using my_hashtable = hashtable<int, float>;

        my_hashtable x;

        x.emplace(7, 28);

        check(x[7] == 28);
    });

    suite.add_test("ctor (range)", [] () {
        using my_hashtable = hashtable<int, float>;

        vector<compressed_pair<int, float>> v {
            make_pair(1, 2.f),
            make_pair(2, 3.f),
            make_pair(3, 3.f)
        };

        my_hashtable x { v };

        check(x[1] == 2);
        check(x[2] == 3);
        check(x[3] == 3);
    });

    suite.add_test("ctor (range w/allocator)", [] () {
        using my_hashtable = hashtable<int, float>;

        vector<compressed_pair<int, float>> v {
            make_pair(1, 2.f),
            make_pair(2, 3.f),
            make_pair(3, 3.f)
        };

        my_hashtable x { v, get_default_allocator() };

        check(x[1] == 2);
        check(x[2] == 3);
        check(x[3] == 3);
    });

    suite.add_test("begin", [] () {
        using my_hashtable = hashtable<int, float>;

        vector<compressed_pair<int, float>> v {
            make_pair(1, 2.f)
        };

        my_hashtable x { v, get_default_allocator() };

        const auto it = x.begin();

        check(it->first() == 1);
        check(it->second() == 2);
        check(++it == x.end());
    });

    suite.add_test("last", [] () {
        using my_hashtable = hashtable<int, float>;

        vector<compressed_pair<int, float>> v {
            make_pair(1, 2.f)
        };

        my_hashtable x { v, get_default_allocator() };

        const auto it = --x.end();

        check(it->first() == 1);
        check(it->second() == 2);
        check(it == x.begin());
    });

    suite.add_test("clear", [] () {
        using my_hashtable = hashtable<int, float>;

        vector<compressed_pair<int, float>> v {
            make_pair(1, 2.f),
            make_pair(2, 3.f),
            make_pair(3, 3.f)
        };

        my_hashtable x { v, get_default_allocator() };

        x.clear();

        check(x.begin() == x.end());
    });

    return suite.main(argc, argv);
}
