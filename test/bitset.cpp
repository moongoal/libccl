#include <ccl/test.hpp>
#include <ccl/bitset.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    suite.add_test("push_back_true", [] () {
        bitset x;

        x.push_back_true();

        check(x.size() == 1);
        check(x.size_bits() == 1);
        check(x[0] == true);
    });

    suite.add_test("push_back_false", [] () {
        bitset x;

        x.push_back_false();

        check(x.size() == 1);
        check(x.size_bits() == 1);
        check(x[0] == false);
    });

    suite.add_test("push_back", [] () {
        bitset x;

        x.push_back(true);
        x.push_back(false);

        check(x.size() == 1);
        check(x.size_bits() == 2);
        check(x[0] == true);
        check(x[1] == false);
    });

    suite.add_test("operator[] (out of bounds)", [] () {
        bitset x;

        throws<std::out_of_range>([x] () {
            x[0];
        });
    });

    suite.add_test("clear (all)", [] () {
        bitset x;

        x.push_back(true);
        x.push_back(false);
        x.clear();

        check(x.size() == 0);
        check(x.capacity() == 1);
        check(x.size_bits() == 0);
    });

    suite.add_test("set", [] () {
        bitset x;

        x.push_back_false();
        x.set(0);

        check(x.size() == 1);
        check(x.size_bits() == 1);
        check(x[0] == true);
    });

    suite.add_test("clear", [] () {
        bitset x;

        x.push_back_true();
        x.clear(0);

        check(x.size() == 1);
        check(x.size_bits() == 1);
        check(x[0] == false);
    });

    suite.add_test("assign", [] () {
        bitset x;

        x.push_back_true();
        x.assign(0, false);

        check(x.size() == 1);
        check(x.size_bits() == 1);
        check(x[0] == false);
    });

    return suite.main(argc, argv);
}
