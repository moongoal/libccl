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
        }
    );

    return suite.main(argc, argv);
}
