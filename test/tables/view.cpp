#include <ccl/test/test.hpp>
#include <ccl/tables/table.hpp>
#include <ccl/tables/view.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

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

    suite.add_test("size", [] () {
        table<allocator, int, float> my_table;

        my_table.reserve(3);

        for(size_t i = 0; i < 2; ++i) {
            const auto add_int = [i] (auto& x) { x.emplace(i); };
            const auto add_float = [i] (auto& x) { x.emplace(static_cast<float>(i) + 0.5); };

            my_table.apply(add_int, add_float);
        }

        auto view = my_table.view();

        check(view.size() == 2);
    });

    suite.add_test("get (w/index)", [] () {
        table<allocator, int, float> my_table;

        my_table.apply([] (auto& x) { x.emplace(5); }, [] (auto& x) { x.emplace(1); });

        auto my_view = my_table.view();

        check(my_view.get<int>(0) == 5);
        check(my_view.get<float>(0) == 1);
    });

    return suite.main(argc, argv);
}
