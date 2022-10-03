#include <charcoal/test.hpp>
#include <charcoal/vector.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    suite.add_test(
        "ctor",
        []() {
            vector<int> v;
        }
    );

    return suite.main(argc, argv);
}
