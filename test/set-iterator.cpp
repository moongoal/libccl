#include <ccl/test/test.hpp>
#include <ccl/set.hpp>

using namespace ccl;

using test_set = set<int>;
using test_set_iterator = set_iterator<test_set>;

struct TestStruct { int x; };

bool operator ==(const TestStruct &a, const TestStruct &b) noexcept {
    return a.x == b.x;
}

namespace ccl {
    template<>
    struct hash<TestStruct> {
        hash_t operator ()(const TestStruct &x) const noexcept {
            return hash<int>()(x.x);
        }
    };
}

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor (default)", [] () {
        test_set_iterator it;

        equals(it.set, nullptr);
        equals(it.index, 0);
    });

    suite.add_test("ctor (set, index)", [] () {
        test_set s;
        test_set_iterator it = s.begin();

        equals(it.set, &s);
        equals(it.index, s.capacity());
    });

    suite.add_test("ctor (copy)", [] () {
        test_set s;

        s.insert(10);
        test_set_iterator it = s.begin();

        const auto initial_index = it.index;
        test_set_iterator it2{it};

        equals(it.set, &s);
        equals(it.index, initial_index);

        equals(it.set, it2.set);
        equals(it2.index, it.index);
    });

    suite.add_test("operator = (copy)", [] () {
        test_set s;

        s.insert(10);
        test_set_iterator it = s.begin();

        const auto initial_index = it.index;
        test_set_iterator it2;

        it2 = it;

        equals(it.set, &s);
        equals(it.index, initial_index);

        equals(it.set, it2.set);
        equals(it2.index, it.index);
    });

    suite.add_test("operator *", [] () {
        test_set s;

        s.insert(10);
        s.insert(5);
        s.insert(15);
        test_set_iterator it1 = s.find(5);
        test_set_iterator it2 = s.find(10);
        test_set_iterator it3 = s.find(15);

        equals(*it1, 5);
        equals(*it2, 10);
        equals(*it3, 15);
    });

    suite.add_test("operator ->", [] () {
        set<TestStruct> s;

        s.insert({10});
        s.insert({5});
        s.insert({15});

        auto it1 = s.find({5});
        auto it2 = s.find({10});
        auto it3 = s.find({15});

        equals(it1->x, 5);
        equals(it2->x, 10);
        equals(it3->x, 15);
    });

    suite.add_test("operator -- (prefix)", [] () {
        test_set s;

        s.insert(10);
        s.insert(5);
        s.insert(15);

        auto it = s.find(10);
        auto it2 = --it;

        equals(it, it2);
        equals(*it, 5);
    });

    suite.add_test("operator -- (prefix, begin)", [] () {
        test_set s;

        s.insert(10);
        s.insert(5);
        s.insert(15);

        auto it = s.begin();
        auto it2 = --it;

        equals(it, it2);
        equals(it.index, 0);
    });

    suite.add_test("operator -- (postfix)", [] () {
        test_set s;

        s.insert(10);
        s.insert(5);
        s.insert(15);

        auto it = s.find(10);
        auto it2 = it--;

        equals(*it, 5);
        equals(*it2, 10);
    });

    suite.add_test("operator -- (postfix, begin)", [] () {
        test_set s;

        s.insert(10);
        s.insert(5);
        s.insert(15);

        auto it = s.begin();
        auto it2 = it--;

        equals(*it2, 5);
        equals(it.index, 0);
    });

    suite.add_test("operator ++ (prefix)", [] () {
        test_set s;

        s.insert(10);
        s.insert(5);
        s.insert(15);

        auto it = s.find(10);
        auto it2 = ++it;

        equals(it, it2);
        equals(*it, 15);
    });

    suite.add_test("operator ++ (prefix, end)", [] () {
        test_set s;

        s.insert(10);
        s.insert(5);
        s.insert(15);

        auto it = s.find(15);
        auto it2 = ++it;

        equals(it, it2);
        equals(it.index, s.capacity());
    });

    suite.add_test("operator ++ (postfix)", [] () {
        test_set s;

        s.insert(10);
        s.insert(5);
        s.insert(15);

        auto it = s.find(10);
        auto it2 = it++;

        equals(*it, 15);
        equals(*it2, 10);
    });

    suite.add_test("operator ++ (postfix, end)", [] () {
        test_set s;

        s.insert(10);
        s.insert(5);
        s.insert(15);

        auto it = s.find(15);
        auto it2 = it++;

        equals(*it2, 15);
        equals(it.index, s.capacity());
    });

    suite.add_test("operator == / !=", [] () {
        test_set s;

        s.insert(10);
        s.insert(5);
        s.insert(15);

        auto it1 = s.find({5});
        auto it2 = s.find({10});
        auto it3 = s.find({15});
        auto it4 = s.find({5});

        equals(it1, it1);
        differs(it1, it2);
        differs(it1, it3);
        equals(it1, it4);
    });

    suite.add_test("operator >", [] () {
        test_set s;

        s.insert(10);
        s.insert(5);
        s.insert(15);

        auto it1 = s.find({5});
        auto it2 = s.find({10});
        auto it3 = s.find({15});

        check(!(it1 > it1));
        check(it2 > it1);
        check(s.end() > it3);
    });

    suite.add_test("operator >=", [] () {
        test_set s;

        s.insert(10);
        s.insert(5);
        s.insert(15);

        auto it1 = s.find({5});
        auto it2 = s.find({10});
        auto it3 = s.find({15});

        check(it1 >= it1);
        check(it2 >= it1);
        check(s.end() >= it3);
    });

    suite.add_test("operator <", [] () {
        test_set s;

        s.insert(10);
        s.insert(5);
        s.insert(15);

        auto it1 = s.find({5});
        auto it2 = s.find({10});
        auto it3 = s.find({15});

        check(!(it1 < it1));
        check(it1 < it2);
        check(it3 < s.end());
    });

    suite.add_test("operator <=", [] () {
        test_set s;

        s.insert(10);
        s.insert(5);
        s.insert(15);

        auto it1 = s.find({5});
        auto it2 = s.find({10});
        auto it3 = s.find({15});

        check(it1 <= it1);
        check(it1 <= it2);
        check(it3 <= s.end());
    });

    return suite.main(argc, argv);
}
