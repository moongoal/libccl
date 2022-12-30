#include <ccl/test/test.hpp>
#include <ccl/atomic.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor (default)", [] () {
        atomic<int> n CCLUNUSED;
    });

    suite.add_test("ctor (value)", [] () {
        atomic<int> n{5};

        equals(n.load(), 5);
    });

    suite.add_test("is_lock_free", [] () {
        atomic<int> n;

        n.is_lock_free();
    });

    suite.add_test("load", [] () {
        atomic<int> n{5};

        equals(n.load(), 5);
    });

    suite.add_test("store", [] () {
        atomic<int> n;

        n.store(5);
        equals(n.load(), 5);
    });

    suite.add_test("exchange", [] () {
        atomic<int> n{10};

        equals(n.exchange(5), 10);
        equals(n.load(), 5);
    });

    suite.add_test("compare_exchange_weak (fail)", [] () {
        atomic<int> n{5};
        int expected = 6;

        equals(n.compare_exchange_weak(expected, 7), false);
    });

    suite.add_test("compare_exchange_weak (success)", [] () {
        atomic<int> n{5};
        int expected = 5;

        while(!n.compare_exchange_weak(expected, 7));

        equals(n.load(), 7);
    });

    suite.add_test("compare_exchange_strong (fail)", [] () {
        atomic<int> n{5};
        int expected = 6;

        equals(n.compare_exchange_strong(expected, 7), false);
    });

    suite.add_test("compare_exchange_strong (success)", [] () {
        atomic<int> n{5};
        int expected = 5;

        equals(n.compare_exchange_strong(expected, 7), true);
    });

    suite.add_test("add_fetch", [] () {
        atomic<int> n{5};

        equals(n.add_fetch(6), 11);
    });

    suite.add_test("add_sub", [] () {
        atomic<int> n{5};

        equals(n.sub_fetch(4), 1);
    });

    suite.add_test("and_fetch", [] () {
        atomic<int> n{3};

        equals(n.and_fetch(2), 2);
    });

    suite.add_test("or_fetch", [] () {
        atomic<int> n{1};

        equals(n.or_fetch(2), 3);
    });

    suite.add_test("xor_fetch", [] () {
        atomic<int> n{3};

        equals(n.xor_fetch(2), 1);
    });

    suite.add_test("nand_fetch", [] () {
        atomic<uint32_t> n{0xffff0000};

        equals(n.nand_fetch(0xffff0000), 0x0000ffffU);
    });

    suite.add_test("fetch_add", [] () {
        atomic<int> n{5};

        equals(n.fetch_add(6), 5);
        equals(n.load(), 11);
    });

    suite.add_test("fetch_sub", [] () {
        atomic<int> n{5};

        equals(n.fetch_sub(4), 5);
        equals(n.load(), 1);
    });

    suite.add_test("fetch_and", [] () {
        atomic<int> n{3};

        equals(n.fetch_and(2), 3);
        equals(n.load(), 2);
    });

    suite.add_test("fetch_or", [] () {
        atomic<int> n{3};

        equals(n.fetch_or(2), 3);
        equals(n.load(), 3);
    });

    suite.add_test("fetch_xor", [] () {
        atomic<int> n{3};

        equals(n.fetch_xor(2), 3);
        equals(n.load(), 1);
    });

    suite.add_test("fetch_nand", [] () {
        atomic<uint32_t> n{0xffff0000};

        equals(n.fetch_nand(0xffff0000), 0xffff0000U);
        equals(n.load(), 0x0000ffffu);
    });

    suite.add_test("atomic_thread_fence", [] () {
        atomic_thread_fence();
    });

    suite.add_test("atomic_signal_fence", [] () {
        atomic_signal_fence();
    });

    return suite.main(argc, argv);
}
