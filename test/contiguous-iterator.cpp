#include <utility>
#include <ccl/test/test.hpp>
#include <ccl/contiguous-iterator.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor (default)", [] () {
        contiguous_iterator<int> it;

        equals(it.ptr, nullptr);
    });

    suite.add_test("ctor (ptr)", [] () {
        int * const ptr = reinterpret_cast<int*>(0x123);
        contiguous_iterator<int> it{ptr};

        equals(it.ptr, ptr);
    });

    suite.add_test("ctor (copy)", [] () {
        int * const ptr = reinterpret_cast<int*>(0x123);
        contiguous_iterator<int> it{ptr};
        contiguous_iterator<int> it2{it};

        equals(it2.ptr, it.ptr);
    });

    suite.add_test("ctor (move)", [] () {
        int * const ptr = reinterpret_cast<int*>(0x123);
        contiguous_iterator<int> it{ptr};
        contiguous_iterator<int> it2{std::move(it)};

        equals(it2.ptr, it.ptr);
    });

    suite.add_test("operator = (copy)", [] () {
        int * const ptr = reinterpret_cast<int*>(0x123);
        contiguous_iterator<int> it{ptr};
        contiguous_iterator<int> it2;

        it2 = it;

        equals(it2.ptr, it.ptr);
    });

    suite.add_test("operator = (move)", [] () {
        int * const ptr = reinterpret_cast<int*>(0x123);
        contiguous_iterator<int> it{ptr};
        contiguous_iterator<int> it2;

        it2 = std::move(it);

        equals(it2.ptr, ptr);
    });

    suite.add_test("operator *", [] () {
        int n = 10;
        int * const p = &n;
        contiguous_iterator it{p};

        // Read
        equals(*p, n);

        // Write
        *p = 5;
        equals(n, 5);
    });

    suite.add_test("operator ->", [] () {
        struct S { int a; int b; } s, *p;

        p = &s;
        contiguous_iterator it{p};

        // Read
        s.a = 10;
        s.b = 20;
        equals(it->a, 10);
        equals(it->b, 20);

        // Write
        it->a = 5;
        it->b = 6;
        equals(it->a, 5);
        equals(it->b, 6);
    });

    suite.add_test("operator +=", [] () {
        int n[] = { 1, 2, 3, 4, 5 };
        contiguous_iterator it{n};

        it += 2;
        equals(*it, 3);
    });

    suite.add_test("operator -=", [] () {
        int n[] = { 1, 2, 3, 4, 5 };
        contiguous_iterator it{n + 3};

        it -= 2;
        equals(*it, 2);
    });

    suite.add_test("operator +", [] () {
        int n[] = { 1, 2, 3, 4, 5 };
        contiguous_iterator it{n};
        contiguous_iterator it2{n + 1};

        it2 = it + 2;
        equals(*it, 1);
        equals(*it2, 3);
    });

    suite.add_test("operator + (inverted order)", [] () {
        int n[] = { 1, 2, 3, 4, 5 };
        contiguous_iterator it{n};
        contiguous_iterator it2 = 1 + n;

        it2 = it + 2;
        equals(*it, 1);
        equals(*it2, 3);
    });

    suite.add_test("operator -", [] () {
        int n[] = { 1, 2, 3, 4, 5 };
        contiguous_iterator it{n};
        contiguous_iterator it2{n + 5};

        it = it2 - 1;
        equals(*it, 5);
        equals(it2.ptr, n + 5);
    });

    suite.add_test("operator []", [] () {
        int n[] = { 1, 2, 3, 4, 5 };
        contiguous_iterator it{n + 1};

        equals(it[0], 2);
        equals(it[1], 3);
        equals(it[-1], 1);
    });

    suite.add_test("operator ++ (prefix)", [] () {
        int n[] = { 1, 2, 3, 4, 5 };
        contiguous_iterator it{n};
        contiguous_iterator it2 = ++it;

        equals(*it, 2);
        equals(*it2, 2);
    });

    suite.add_test("operator ++ (postfix)", [] () {
        int n[] = { 1, 2, 3, 4, 5 };
        contiguous_iterator it{n};
        contiguous_iterator it2 = it++;

        equals(*it, 2);
        equals(*it2, 1);
    });

    suite.add_test("operator -- (prefix)", [] () {
        int n[] = { 1, 2, 3, 4, 5 };
        contiguous_iterator it{n + 1};
        contiguous_iterator it2 = --it;

        equals(*it, 1);
        equals(*it2, 1);
    });

    suite.add_test("operator -- (postfix)", [] () {
        int n[] = { 1, 2, 3, 4, 5 };
        contiguous_iterator it{n + 1};
        contiguous_iterator it2 = it--;

        equals(*it, 1);
        equals(*it2, 2);
    });

    suite.add_test("operator ==", [] () {
        int n[] = { 1, 2, 3, 4, 5 };
        contiguous_iterator it{n + 1};
        contiguous_iterator it2 = n + 1;

        check(it == it);
        check(it == it2);
        check(it2 == it);
    });

    suite.add_test("operator !=", [] () {
        int n[] = { 1, 2, 3, 4, 5 };
        contiguous_iterator it{n + 1};
        contiguous_iterator it2 = n;

        check(!(it != it));
        check(it != it2);
        check(it2 != it);
    });

    suite.add_test("operator >", [] () {
        int n[] = { 1, 2, 3, 4, 5 };
        contiguous_iterator it{n};
        contiguous_iterator it2 = n + 1;

        check(!(it > it));
        check(it2 > it);
    });

    suite.add_test("operator >=", [] () {
        int n[] = { 1, 2, 3, 4, 5 };
        contiguous_iterator it{n};
        contiguous_iterator it2 = n + 1;
        contiguous_iterator it3 = n;

        check(it >= it);
        check(it2 >= it);
        check(!(it >= it2));
        check(it3 >= it);
    });

    suite.add_test("operator <", [] () {
        int n[] = { 1, 2, 3, 4, 5 };
        contiguous_iterator it{n};
        contiguous_iterator it2 = n + 1;

        check(!(it < it));
        check(it < it2);
    });

    suite.add_test("operator <=", [] () {
        int n[] = { 1, 2, 3, 4, 5 };
        contiguous_iterator it{n};
        contiguous_iterator it2 = n + 1;
        contiguous_iterator it3 = n;

        check(it <= it);
        check(it <= it2);
        check(!(it2 <= it));
        check(it3 <= it);
    });

    return suite.main(argc, argv);
}
