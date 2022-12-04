#include <ccl/test.hpp>
#include <ccl/sparse-set.hpp>

using namespace ccl;

struct S {
    int a;
    float b;
};

constexpr bool operator ==(const S& first, const S& second) {
    return first.a == second.a && first.b == second.b;
}

constexpr bool operator !=(const S& first, const S& second) {
    return first.a == second.a && first.b == second.b;
}

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("insert ref", [] () {
        sparse_set<S> set;

        set.insert({ 1, 2.0 });
        set.insert({ 2, 3.0 });
        set.insert({ 3, 3.0 });
        set.insert({ 3, 3.0 });

        check(set.size() == 3);
    });

    suite.add_test("remove", [] () {
        sparse_set<S> set;

        const S s2{ 2, 3.0 };

        set.insert({ 1, 2.0 });
        set.insert(s2);
        set.insert({ 3, 3.0 });
        set.insert({ 3, 3.0 });

        set.remove(s2);

        check(set.size() == 2);
    });

    suite.add_test("contains", [] () {
        sparse_set<S> set;

        const S s1{ 1, 1 };
        const S s2{ 2, 3.0 };

        set.insert(s1);
        set.insert(s2);

        check(set.contains(s1));
        check(set.contains(s2));
        check(!set.contains({ 1, 0 }));
    });

    suite.add_test("begin/end", [] () {
        sparse_set<S> set;

        set.insert({ 1, 2.0 });
        set.insert({ 2, 3.0 });
        set.insert({ 3, 3.0 });
        set.insert({ 3, 3.0 });

        const auto begin = set.begin();
        const auto end = set.end();
        const ptrdiff_t size = end - begin;

        check(size == 3);
    });

    return suite.main(argc, argv);
}
