#include <ccl/test.hpp>
#include <ccl/bitset.hpp>
#include <ccl/util.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    suite.add_test("push_back_set", [] () {
        bitset x;

        x.push_back_set();

        check(x.size() == 1);
        check(x.size_bits() == 1);
        check(x[0] == true);
    });

    suite.add_test("push_back_clear", [] () {
        bitset x;

        x.push_back_clear();

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

        throws<std::out_of_range>([&x] () {
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

        x.push_back_clear();
        x.set(0);

        check(x.size() == 1);
        check(x.size_bits() == 1);
        check(x[0] == true);
    });

    suite.add_test("clear", [] () {
        bitset x;

        x.push_back_set();
        x.clear(0);

        check(x.size() == 1);
        check(x.size_bits() == 1);
        check(x[0] == false);
    });

    suite.add_test("assign", [] () {
        bitset x;

        x.push_back_set();
        x.assign(0, false);

        check(x.size() == 1);
        check(x.size_bits() == 1);
        check(x[0] == false);
    });

    // Regression: with more than one cluster, location of
    // bit is wrong
    suite.add_test("assign (more than one cluster)", [] () {
        using cluster_type = bitset<>::cluster_type;
        static constexpr size_t cluster_size = sizeof(cluster_type) * 8 /* bits */;

        bitset x;

        for(size_t i = 0; i < 2 * cluster_size; ++i) {
            x.push_back_set();
        }

        check(x.get_clusters().size() == 2);
        check(x.get_clusters()[0] == ~static_cast<cluster_type>(0));
        check(x.get_clusters()[1] == ~static_cast<cluster_type>(0));
    });

    suite.add_test("reserve (grow)", [] () {
        bitset x;

        x.push_back_set();
        x.reserve(sizeof(bitset<>::cluster_type) * 8 /* bits */ * 2 /* two clusters of bits */);

        check(x.capacity() == 2);
        check(x.size() == 1);
        check(x.size_bits() == 1);
        check(x[0] == true);
    });

    suite.add_test("reserve (shrink)", [] () {
        bitset x;

        x.push_back_set();
        x.reserve(0);

        check(x.capacity() == 1);
        check(x.size() == 1);
        check(x.size_bits() == 1);
        check(x[0] == true);
    });

    suite.add_test("reserve (same value)", [] () {
        bitset x;

        x.push_back_set();
        x.reserve(sizeof(bitset<>::cluster_type) * 8 /* bits */);

        check(x.capacity() == 1);
        check(x.size() == 1);
        check(x.size_bits() == 1);
        check(x[0] == true);
    });

    suite.add_test("resize (grow)", [] () {
        bitset x;

        x.push_back_set();
        x.resize(2);

        x[1];
    });

    suite.add_test("resize (shrink)", [] () {
        bitset x;

        x.push_back_set();
        x.push_back_set();
        x.resize(1);

        throws<std::out_of_range>([&x] () {
            x[1];
        });
    });

    return suite.main(argc, argv);
}
