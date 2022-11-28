#include <ccl/test.hpp>
#include <ccl/tables/table.hpp>
#include <ccl/tables/view.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    suite.add_test("ctor", [] () {
        table<allocator, int, float> my_table;

        auto view CCLUNUSED = my_table.view();
    });

    return suite.main(argc, argv);
}
