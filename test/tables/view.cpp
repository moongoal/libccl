#include <ccl/test.hpp>
#include <ccl/tables/table.hpp>
#include <ccl/tables/view.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    suite.add_test("full view", [] () {
        table<allocator, int, float> my_table;

        auto view CCLUNUSED = my_table.view();
    });

    suite.add_test("partial view", [] () {
        table<allocator, int, float> my_table;

        auto view CCLUNUSED = my_table.view<int>();
    });

    suite.add_test("get", [] () {
        table<allocator, int, float> my_table;

        auto view = my_table.view<int>();
        auto v = view.get<int>();

        check(std::is_same_v<decltype(v)::value_type, int>);
    });

    suite.add_test("each", [] () {
        table<allocator, int, float> my_table;

        my_table.reserve(3);

        for(size_t i = 0; i < 3; ++i) {
            const auto add_int = [i] (auto& x) { x.emplace(i); };
            const auto add_float = [i] (auto& x) { x.emplace(static_cast<float>(i) + 0.5); };

            my_table.apply(add_int, add_float);
        }

        auto view = my_table.view();
        size_t n = 0;

        view.each([&n] (const int& i, const float& f) {
            check(n == static_cast<size_t>(i));
            check(f == static_cast<float>(i) + 0.5);

            n += 1;
        });

        check(n == 3);
    });

    return suite.main(argc, argv);
}
