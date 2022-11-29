#include <ccl/test.hpp>
#include <ccl/tables/table.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    suite.add_test("ctor", [] () {
        table<allocator, int, float> my_table;

        check(my_table.get<float>().size() == 0);
    });

    suite.add_test("apply", [] () {
        table<allocator, int, float> my_table;

        my_table.apply([] (auto& x) { x.emplace(5); }, [] (auto& x) { x.emplace(1); });

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

    suite.add_test("apply_one", [] () {
        table<allocator, int, float> my_table;

        my_table.apply([] (auto& x) { x.emplace(5); }, [] (auto& x) { x.emplace(1); });
        my_table.apply_one<float>([] (auto& x) { x[0] *= 15; });

        check(my_table.get<int>().size() == 1);
        check(my_table.get<float>().size() == 1);

        check(my_table.get<int>()[0] == 5);
        check(my_table.get<float>()[0] == 15);
    });

    suite.add_test("apply (some columns)", [] () {
        struct S { int value; };

        table<allocator, int, float, S> my_table;

        my_table.apply([] (auto& x) { x.emplace(5); }, [] (auto& x) { x.emplace(1); }, [] (auto& x) { x.emplace().value = 16; });
        my_table.apply<S, float>([] (auto& x) { x[0].value /= 2; }, [] (auto& x) { x[0] = -1; });

        check(my_table.get<int>().size() == 1);
        check(my_table.get<float>().size() == 1);
        check(my_table.get<S>().size() == 1);

        check(my_table.get<int>()[0] == 5);
        check(my_table.get<float>()[0] == -1);
        check(my_table.get<S>()[0].value == 8);
    });

    suite.add_test("emplace", [] () {
        struct S { int value; };

        table<allocator, int, float, S> my_table;

        my_table.emplace();

        check(my_table.get<int>().size() == 1);
        check(my_table.get<float>().size() == 1);
        check(my_table.get<S>().size() == 1);
    });

    suite.add_test("view (full)", [] () {
        table<allocator, int, float> my_table;

        auto view CCLUNUSED = my_table.view();
    });

    suite.add_test("view (partial)", [] () {
        table<allocator, int, float> my_table;

        auto view CCLUNUSED = my_table.view<int>();
    });

    suite.add_test("each (full)", [] () {
        table<allocator, int, float> my_table;

        my_table.reserve(3);

        for(size_t i = 0; i < 3; ++i) {
            const auto add_int = [i] (auto& x) { x.emplace(i); };
            const auto add_float = [i] (auto& x) { x.emplace(static_cast<float>(i) + 0.5); };

            my_table.apply(add_int, add_float);
        }

        size_t n = 0;

        my_table.each([&n] (const int& i, const float& f) {
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

        check(my_table.size() == 2);
    });

    suite.add_test("get (w/index)", [] () {
        table<allocator, int, float> my_table;

        my_table.apply([] (auto& x) { x.emplace(5); }, [] (auto& x) { x.emplace(1); });

        my_table.get<float>(0) = 2;

        check(my_table.get<int>(0) == 5);
        check(my_table.get<float>(0) == 2);
    });

    return suite.main(argc, argv);
}
