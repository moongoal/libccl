#include <ccl/test.hpp>
#include <ccl/hashtable.hpp>

using namespace ccl;

int main(int argc, char **argv) {
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

    return suite.main(argc, argv);
}
