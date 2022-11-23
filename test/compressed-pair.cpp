#include <ccl/test.hpp>
#include <ccl/compressed-pair.hpp>

using namespace ccl;

struct empty {};
struct empty2 {};

int main(int argc, char ** argv) {
    suite.add_test("non empty, different types", [] () {
        compressed_pair<int, float> p{1, 2};

        check(sizeof(p) == sizeof(int) * 2);
        check(p.first() == 1);
        check(p.second() == 2);
    });

    suite.add_test("first empty, different types", [] () {
        compressed_pair<empty, float> p{{}, 2};

        check(sizeof(p) == sizeof(float));
        check(p.second() == 2);
    });

    suite.add_test("second empty, different types", [] () {
        compressed_pair<unsigned long long, empty> p{1, {}};

        check(sizeof(p) == sizeof(unsigned long long));
        check(p.first() == 1);
    });

    suite.add_test("both empty, different types", [] () {
        compressed_pair<empty, empty2> p{{}, {}};

        check(sizeof(p) <= 1);
    });

    suite.add_test("non empty, same types", [] () {
        compressed_pair<int, int> p{1, 2};

        check(sizeof(p) == sizeof(int) * 2);
        check(p.first() == 1);
        check(p.second() == 2);
    });

    suite.add_test("both empty, same types", [] () {
        compressed_pair<empty, empty> p{{}, {}};

        check(sizeof(p) <= 1);
    });

    return suite.main(argc, argv);
}
