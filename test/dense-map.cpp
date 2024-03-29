#include <ccl/test/test.hpp>
#include <ccl/dense-map.hpp>
#include <ccl/test/counting-test-allocator.hpp>

using namespace ccl;

template<typename K, typename V>
using test_map = dense_map<K, V, hash<K>, counting_test_allocator>;

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

    suite.add_test("erase (key)", [] () {
        test_map<int, S> map;

        map.insert(1, { 1, 2.0 });
        map.insert(2, { 2, 3.0 });
        map.insert(3, { 3, 3.0 });
        map.insert(3, { 3, 3.0 });

        map.erase(2);

        check(map.size() == 2);
    });

    suite.add_test("erase (iterator)", [] () {
        test_map<int, S> map;

        map.insert(1, { 1, 2.0 });
        map.insert(2, { 2, 3.0 });
        map.insert(3, { 3, 3.0 });
        map.insert(3, { 3, 3.0 });

        auto it = map.find(2);

        map.erase(it);

        check(map.size() == 2);
    });

    suite.add_test("access after erase", [] () {
        test_map<int, S> map;

        map.insert(1, { 1, 2.0 });
        map.insert(2, { 2, 3.0 });
        map.insert(3, { 3, 3.0 });

        map.erase(2);

        equals(map.at(3), S{ 3, 3.0 });
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
            equals(*kv.first, *kv.second);
        }

        check(n == 3);
    });

    suite.add_test("find", [] () {
        test_map<S, S> map;

        map.insert({ 1, 2.0 }, { 1, 2.0 });
        map.insert({ 2, 3.0 }, { 2, 3.0 });
        map.insert({ 3, 3.0 }, { 3, 4.0 });

        const auto it = map.find({ 1, 2.0 });

        check(*it->first == S{ 1, 2.0 });
        check(*it->second == S{ 1, 2.0 });
    });

    suite.add_test("find (not found)", [] () {
        test_map<S, S> map;

        map.insert({ 1, 2.0 }, { 1, 2.0 });
        map.insert({ 2, 3.0 }, { 2, 3.0 });
        map.insert({ 3, 3.0 }, { 3, 4.0 });

        const auto it = map.find({ 1, 9.0 });

        check(it == map.end());
    });

    suite.add_test("emplace", [] () {
        test_map<S, int> map;

        map.emplace({ 1, 2.0 }, 5);
        map.emplace({ 2, 3.0 }, 6);
        map.emplace({ 3, 3.0 }, 7);

        equals(*map.find({ 1, 2.0 })->second, 5);
        equals(*map.find({ 2, 3.0 })->second, 6);
        equals(*map.find({ 3, 3.0 })->second, 7);

        equals(map.size(), 3ULL);
    });

    suite.add_test("at", [] () {
        test_map<S, int> map;

        map.emplace({ 1, 2.0 }, 5);
        map.emplace({ 2, 3.0 }, 6);
        map.emplace({ 3, 3.0 }, 7);

        equals(map.at({ 1, 2.0 }), 5);
        equals(map.at({ 2, 3.0 }), 6);
        equals(map.at({ 3, 3.0 }), 7);
    });

    suite.add_test("at (not present)", [] () {
        test_map<S, int> map;

        map.emplace({ 1, 2.0 }, 5);
        map.emplace({ 2, 3.0 }, 6);
        map.emplace({ 3, 3.0 }, 7);

        throws<std::out_of_range>(
            [&map] () {
                const auto& x CCLUNUSED = map.at({ 1, 9.0 });
            }
        );
    }, skip_if_exceptions_disabled);

    suite.add_test("move-assignment operator", [] () {
        test_map<S, int> map1;
        test_map<S, int> map2;

        map1.emplace({ 1, 2.0 }, 5);
        map1.emplace({ 2, 3.0 }, 6);
        map1.emplace({ 3, 3.0 }, 7);

        map2.emplace({ 1, 2.0 }, 8);
        map2.emplace({ 2, 3.0 }, 9);
        map2.emplace({ 3, 3.0 }, 10);
        map2.emplace({ 3, 4.0 }, 10);

        map1 = std::move(map2);

        equals(map1.at({ 1, 2.0 }), 8);
        equals(map1.at({ 2, 3.0 }), 9);
        equals(map1.at({ 3, 3.0 }), 10);
        equals(map1.at({ 3, 4.0 }), 10);
        equals(map1.size(), 4ULL);
    });

    suite.add_test("copy-assignment operator", [] () {
        test_map<S, int> map1;
        test_map<S, int> map2;

        map1.emplace({ 1, 2.0 }, 5);
        map1.emplace({ 2, 3.0 }, 6);
        map1.emplace({ 3, 3.0 }, 7);

        map2.emplace({ 1, 2.0 }, 8);
        map2.emplace({ 2, 3.0 }, 9);
        map2.emplace({ 3, 3.0 }, 10);
        map2.emplace({ 3, 4.0 }, 10);

        map1 = map2;

        equals(map1.at({ 1, 2.0 }), 8);
        equals(map1.at({ 2, 3.0 }), 9);
        equals(map1.at({ 3, 3.0 }), 10);
        equals(map1.at({ 3, 4.0 }), 10);
        equals(map1.size(), 4ULL);

        equals(map2.at({ 1, 2.0 }), 8);
        equals(map2.at({ 2, 3.0 }), 9);
        equals(map2.at({ 3, 3.0 }), 10);
        equals(map2.at({ 3, 4.0 }), 10);
        equals(map2.size(), 4ULL);
    });

    suite.add_test("clear", [] () {
        test_map<S, int> map;

        map.emplace({ 1, 2.0 }, 5);
        map.emplace({ 2, 3.0 }, 6);
        map.emplace({ 3, 3.0 }, 7);

        map.clear();
        equals(map.size(), 0U);
    });

    return suite.main(argc, argv);
}
