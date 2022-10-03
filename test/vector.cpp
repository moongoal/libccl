#include <charcoal/test.hpp>
#include <charcoal/vector.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    suite.add_test(
        "ctor",
        []() {
            vector<int> v;

            assert(v.get_capacity() == 0);
            assert(v.get_length() == 0);
            assert(v.get_data() == nullptr);
        }
    );

    return suite.main(argc, argv);
}
