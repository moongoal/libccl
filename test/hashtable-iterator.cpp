#include <ccl/test/test.hpp>
#include <ccl/hashtable.hpp>

using namespace ccl;

using test_hashtable = hashtable<int, int>;
using test_iterator = hashtable_iterator<test_hashtable>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor (default)", [] () {
        test_iterator x;

        equals(x.hashtable, nullptr);
        equals(x.index, 0);
    });

    suite.add_test("ctor (copy, empty)", [] () {
        test_hashtable t;

        test_iterator it{t.begin()};
        test_iterator it2{it};

        equals(it.hashtable, &t);
        equals(it.hashtable, it2.hashtable);
        equals(it.index, it2.index);
    });

    suite.add_test("ctor (copy, non-empty)", [] () {
        test_hashtable t;

        t[2] = 3;

        test_iterator it{t.begin()};
        test_iterator it2{it};

        equals(it.hashtable, &t);
        equals(it.hashtable, it2.hashtable);
        equals(it.index, it2.index);
    });

    suite.add_test("operator = (copy, empty)", [] () {
        test_hashtable t;

        test_iterator it{t.begin()};
        const auto index = it.index;
        test_iterator it2;

        it2 = it;

        equals(it2.hashtable, &t);
        equals(index, it2.index);
    });

    suite.add_test("operator = (copy, non-empty)", [] () {
        test_hashtable t;

        t[2] = 3;

        test_iterator it{t.begin()};
        const auto index = it.index;
        test_iterator it2;

        it2 = it;

        equals(it2.hashtable, &t);
        equals(index, it2.index);
    });

    suite.add_test("operator * (non-const)", [] () {
        test_hashtable t;

        t[2] = 3;

        test_iterator it = t.begin();

        equals(*(*it).first, 2);
        equals(*(*it).second, 3);
    });

    suite.add_test("operator * (const)", [] () {
        test_hashtable t;

        t[2] = 3;

        const test_iterator it = t.begin();

        equals(*(*it).first, 2);
        equals(*(*it).second, 3);
    });

    suite.add_test("operator -> (non-const)", [] () {
        test_hashtable t;

        t[2] = 3;

        test_iterator it = t.begin();

        equals(*it->first, 2);
        equals(*it->second, 3);
    });

    suite.add_test("operator -> (const)", [] () {
        test_hashtable t;

        t[2] = 3;

        const test_iterator it = t.begin();

        equals(*it->first, 2);
        equals(*it->second, 3);
    });

    suite.add_test("operator ++ (prefix)", [] () {
        test_hashtable t;

        t[2] = 3;

        test_iterator it = t.begin();
        const test_iterator it2 = ++it;

        differs(it, t.begin());
        equals(it2.hashtable, it.hashtable);
        equals(it2, t.end());
        equals(it.index, it2.index);
    });

    suite.add_test("operator ++ (postfix)", [] () {
        test_hashtable t;

        t[2] = 3;

        test_iterator it = t.begin();
        const test_iterator it2 = it++;

        differs(it, t.begin());
        equals(it2, t.begin());
        equals(it2.hashtable, it.hashtable);
        equals(it, t.end());
        differs(it.index, it2.index);
    });

    suite.add_test("operator -- (prefix)", [] () {
        test_hashtable t;

        t[2] = 3;

        test_iterator it = t.end();
        const test_iterator it2 = --it;

        equals(it, t.begin());
        equals(it2, t.begin());
        equals(it2.hashtable, it.hashtable);
        differs(it, t.end());
        equals(it.index, it2.index);
    });

    suite.add_test("operator -- (postfix)", [] () {
        test_hashtable t;

        t[2] = 3;

        test_iterator it = t.end();
        const test_iterator it2 = it--;

        equals(it, t.begin());
        equals(it2, t.end());
        equals(it2.hashtable, it.hashtable);
        differs(it, t.end());
        differs(it.index, it2.index);
    });

    suite.add_test("operator -- (starting at beginning)", [] () {
        test_hashtable t;

        t[2] = 3;

        test_iterator it = t.begin();
        it--;
        it--;

        equals(it.index, 0);
    });

    suite.add_test("operator == (same table, different index)", [] () {
        test_hashtable t;

        t[2] = 3;

        equals(t.begin(), t.begin());
    });

    suite.add_test("operator == (different table, same index)", [] () {
        test_hashtable t, t2;

        t[2] = 3;
        t2[2] = 3;

        check(!(t.begin() == t2.begin()));
    });

    suite.add_test("operator != (same table, different index)", [] () {
        test_hashtable t;

        t[2] = 3;

        check(t.begin() != t.end());
    });

    suite.add_test("operator != (different table, same index)", [] () {
        test_hashtable t, t2;

        t[2] = 3;
        t2[2] = 3;

        check(t.begin() != t2.begin());
    });

    suite.add_test("operator >", [] () {
        test_hashtable t;

        t[2] = 3;

        check(t.end() > t.begin());
        check(!(t.end() > t.end()));
    });

    suite.add_test("operator > (different tables)", [] () {
        test_hashtable t, t2;

        throws<std::runtime_error>([&] () {
            check(t.end() > t2.begin());
        });
    });

    suite.add_test("operator <", [] () {
        test_hashtable t;

        t[2] = 3;

        check(t.begin() < t.end());
        check(!(t.end() < t.begin()));
    });

    suite.add_test("operator < (different tables)", [] () {
        test_hashtable t, t2;

        throws<std::runtime_error>([&] () {
            check(t.end() < t2.begin());
        });
    });

    suite.add_test("operator >=", [] () {
        test_hashtable t;

        t[2] = 3;

        check(t.end() >= t.begin());
        check(t.end() >= t.end());
        check(!(t.begin() >= t.end()));
    });

    suite.add_test("operator >= (different table)", [] () {
        test_hashtable t, t2;

        throws<std::runtime_error>([&] () {
            check(t.end() >= t2.begin());
        });
    });

    suite.add_test("operator <=", [] () {
        test_hashtable t;

        t[2] = 3;

        check(t.begin() <= t.end());
        check(t.end() <= t.end());
        check(!(t.end() <= t.begin()));
    });

    suite.add_test("operator <= (different table)", [] () {
        test_hashtable t, t2;

        throws<std::runtime_error>([&] () {
            check(t.end() <= t2.begin());
        });
    });

    return suite.main(argc, argv);
}
