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

    suite.add_test("at (not present)", []() {
        using my_hashtable = hashtable<int, float>;

        my_hashtable x;

        throws<std::out_of_range>([&x]() {
            decltype(auto) value CCLUNUSED = x.at(1);
        });
    });

    suite.add_test("at", []() {
        using my_hashtable = hashtable<int, float>;

        my_hashtable x;

        for(size_t i = 0; i < my_hashtable::minimum_capacity; ++i) {
            x.insert(i, i + 1);
        }

        for(size_t i = 0; i < my_hashtable::minimum_capacity; ++i) {
            check(x.at(i) == i + 1);
        }
    });

    suite.add_test("operator [] not present", []() {
        using my_hashtable = hashtable<int, float>;

        my_hashtable x;

        decltype(auto) value CCLUNUSED = x[1]; // Default-constructed
    });

    suite.add_test("erase key", [] () {
        using my_hashtable = hashtable<int, float>;

        my_hashtable x;

        x.insert(1, 1);
        x.erase(1);

        throws<std::out_of_range>([&x]() {
            decltype(auto) v CCLUNUSED = x.at(1);
        });
    });

    suite.add_test("erase iterator", [] () {
        using my_hashtable = hashtable<int, float>;

        my_hashtable x;

        x.insert(1, 1);
        x.insert(2, 3);
        x.insert(3, 4);

        const auto it = x.find(2);

        x.erase(it);

        check(x.contains(1));
        check(!x.contains(2));
        check(x.contains(3));
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

    suite.add_test("find (not present)", []() {
        using my_hashtable = hashtable<int, float>;

        my_hashtable x;

        check(x.find(1) == x.end());
    });

    suite.add_test("find", []() {
        using my_hashtable = hashtable<int, float>;

        my_hashtable x;

        x.emplace(1, 1);
        x.emplace(2, 3);

        check(x.find(1)->second() == 1);
        check(x.find(2)->second() == 3);
    });

    suite.add_test("contains", []() {
        using my_hashtable = hashtable<int, float>;

        my_hashtable x;

        x.emplace(1, 1);
        x.emplace(2, 3);

        check(x.contains(1));
        check(x.contains(2));
        check(!x.contains(3));
    });

    return suite.main(argc, argv);
}
