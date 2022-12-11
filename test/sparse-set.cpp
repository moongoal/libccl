#include <ccl/test.hpp>
#include <ccl/sparse-set.hpp>
#include <ccl/test/counting-test-allocator.hpp>

using namespace ccl;

template<typename K, typename H = hash<K>>
using test_set = sparse_set<K, H, counting_test_allocator>;

struct S {
    int a;
    float b;

    constexpr hash_t hash() const noexcept {
        return a ^ static_cast<hash_t>(b);
    }
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
        test_set<S> set;

        set.insert({ 1, 2.0 });
        set.insert({ 2, 3.0 });
        set.insert({ 3, 3.0 });
        set.insert({ 3, 3.0 });

        check(set.size() == 3);
    });

    suite.add_test("remove", [] () {
        test_set<S> set;

        const S s2{ 2, 3.0 };

        set.insert({ 1, 2.0 });
        set.insert(s2);
        set.insert({ 3, 3.0 });
        set.insert({ 3, 3.0 });

        set.remove(s2);

        check(set.size() == 2);
    });

    suite.add_test("contains", [] () {
        test_set<S> set;

        const S s1{ 1, 1 };
        const S s2{ 2, 3.0 };

        set.insert(s1);
        set.insert(s2);

        check(set.contains(s1));
        check(set.contains(s2));
        check(!set.contains({ 1, 0 }));
    });

    suite.add_test("begin/end", [] () {
        test_set<S> set;

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
