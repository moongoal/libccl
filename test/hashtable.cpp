#include <ccl/test.hpp>
#include <ccl/hashtable.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    suite.add_test("insert", []() {
        using my_hashtable = hashtable<int, float>;

        my_hashtable x;

        x.insert(5, 1);

        check(x.capacity() == my_hashtable::minimum_capacity);
    });

    return suite.main(argc, argv);
}
