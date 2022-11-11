#include <charcoal/test.hpp>
#include <charcoal/vector.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    suite.add_test(
        "ctor",
        []() {
            vector<int> v;

            check(v.get_capacity() == 0);
            check(v.get_length() == 0);
            check(v.get_data() == nullptr);
        }
    );

    suite.add_test(
        "append",
        []() {
            vector<int> v;

            v.append(1);
            v.append(2);
            v.append(3);

            check(v[0] == 1);
            check(v[1] == 2);
            check(v[2] == 3);

            check(v.get_length() == 3);
            check(v.get_capacity() == 4);
        }
    );

    return suite.main(argc, argv);
}
