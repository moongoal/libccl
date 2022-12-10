#include <ccl/test.hpp>
#include <ccl/dense-map.hpp>
#include <ccl/test/counting-test-allocator.hpp>

using namespace ccl;

template<typename K, typename V>
using test_map = dense_map<K, V, counting_test_allocator>;

struct S {
    int a;
    float b;

    constexpr hash_t hash() const noexcept {
        return a ^ static_cast<hash_t>(b);
    }
};

std::ostream & operator << (std::ostream &out, const S &s) {
    out << "(a=" << s.a << ", b=" << s.b << ")";

    return out;
}

constexpr bool operator ==(const S& first, const S& second) {
    return first.a == second.a && first.b == second.b;
}

constexpr bool operator !=(const S& first, const S& second) {
    return first.a != second.a || first.b != second.b;
}

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("insert ref", [] () {
        test_map<int, S> map;

        map.insert(1, { 1, 2.0 });
        map.insert(2, { 2, 3.0 });
        map.insert(3, { 3, 3.0 });
        map.insert(3, { 3, 3.0 });

        check(map.size() == 3);
    });

    suite.add_test("remove", [] () {
        test_map<int, S> map;

        map.insert(1, { 1, 2.0 });
        map.insert(2, { 2, 3.0 });
        map.insert(3, { 3, 3.0 });
        map.insert(3, { 3, 3.0 });

        map.remove(2);

        check(map.size() == 2);
    });

    suite.add_test("contains", [] () {
        test_map<int, S> map;

        map.insert(1, { 1, 1 });
        map.insert(2, { 2, 3.0 });

        check(map.contains(1));
        check(map.contains(2));
        check(!map.contains(3));
    });

    suite.add_test("begin_values/end_values", [] () {
        test_map<S, S> map;

        map.insert({ 1, 2.0 }, { 1, 2.0 });
        map.insert({ 2, 3.0 }, { 2, 3.0 });
        map.insert({ 3, 3.0 }, { 3, 3.0 });
        map.insert({ 3, 3.0 }, { 3, 3.0 });

        const auto begin = map.begin_values();
        const auto end = map.end_values();
        const ptrdiff_t size = end - begin;

        check(size == 3);
    });

    suite.add_test("begin/end", [] () {
        test_map<S, S> map;
        int n = 0;

        map.insert({ 1, 2.0 }, { 1, 2.0 });
        map.insert({ 2, 3.0 }, { 2, 3.0 });
        map.insert({ 3, 3.0 }, { 3, 4.0 });
        map.insert({ 3, 3.0 }, { 3, 3.0 });

        for(const auto &kv : map) {
            n++;
            equals(*kv.first(), *kv.second());
        }

        check(n == 3);
    });

    return suite.main(argc, argv);
}
