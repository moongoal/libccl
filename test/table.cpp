#include <ccl/test.hpp>
#include <ccl/table.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    suite.add_test("ctor", [] () {
        table<allocator, int, float> my_table;

        check(my_table.get<float>().size() == 0);
    });

    suite.add_test("emplace", [] () {
        table<allocator, int, float> my_table;

        my_table.emplace_row([] (auto& x) { x.emplace(5); }, [] (auto& x) { x.emplace(1); });

        check(my_table.get<int>().size() == 1);
        check(my_table.get<float>().size() == 1);

        check(my_table.get<int>()[0] == 5);
        check(my_table.get<float>()[0] == 1);
    });

    suite.add_test("reserve", [] () {
        table<allocator, int, float> my_table;

        my_table.reserve(16);

        check(my_table.get<int>().capacity() == 16);
        check(my_table.get<float>().capacity() == 16);
    });

    return suite.main(argc, argv);
}
