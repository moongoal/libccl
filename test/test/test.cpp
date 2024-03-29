#include <ccl/test/test.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test(
        "execute",
        []() {
            test_suite my_test_suite{nullptr}; // Test subject.
            int test_run_count = 0;
            const test_function run_increment = [&test_run_count]() { test_run_count++; };

            my_test_suite.add_test("execute1", run_increment);
            my_test_suite.add_test("execute2", run_increment);
            my_test_suite.add_test("execute3", run_increment);

            check(my_test_suite.execute() == true);
            check(test_run_count == 3);
        }
    );

    suite.add_test(
        "fail",
        []() {
            test_suite my_test_suite{nullptr}; // Test subject.
            const test_function run_fail = []() { fail(); };
            const test_function run_success = []() { };

            my_test_suite.add_test("fail", run_fail);
            my_test_suite.add_test("succeed", run_success);

            check(my_test_suite.execute() == false);
        }
    );

    suite.add_test(
        "check_fail",
        []() {
            test_suite my_test_suite{nullptr}; // Test subject.
            const test_function run_test = []() { check(false); };

            my_test_suite.add_test("fail", run_test);

            check(my_test_suite.execute() == false);
        }
    );

    suite.add_test(
        "check_success",
        []() {
            test_suite my_test_suite{nullptr}; // Test subject.
            const test_function run_test = []() { check(true); };

            my_test_suite.add_test("success", run_test);

            check(my_test_suite.execute() == true);
        }
    );

    suite.add_test(
        "main_success",
        []() {
            test_suite my_test_suite{nullptr}; // Test subject.
            const test_function run_test = []() { };

            my_test_suite.add_test("success", run_test);

            check(my_test_suite.main(0, nullptr) == 0);
        }
    );

    suite.add_test(
        "main_fail",
        []() {
            test_suite my_test_suite{nullptr}; // Test subject.
            const test_function run_test = []() { fail(); };

            my_test_suite.add_test("fail", run_test);

            check(my_test_suite.main(0, nullptr) != 0);
        }
    );

    suite.add_test("throws", []() {
        throws([] () {
            throw std::exception{};
        });
    });

    suite.add_test("throws (no throw)", []() {
        try {
            throws([] () {});
        } catch(test_failed_exception &) {
            return;
        }

        fail();
    });

    suite.add_test("throws (other exception)", []() {
        try {
            throws<int>([] () {
                throw std::exception{};
            });
        } catch(test_failed_exception &) {
            fail();
        } catch (std::exception &) {
            return;
        }

        fail();
    });

    suite.add_test("skip", []() {
        test_suite my_test_suite{nullptr}; // Test subject.
        int exec_count = 0;

        my_test_suite.add_test("skip", [&exec_count] () { exec_count += 1; }, []() { return true; });
        my_test_suite.add_test("no skip", [&exec_count] () { exec_count += 1; });
        my_test_suite.execute();

        equals(exec_count, 1);
    });

    suite.add_test("before all hooks", [] () {
        test_suite my_test_suite{nullptr}; // Test subject.
        int exec_count = 0;

        my_test_suite.exec_before_all([&exec_count] () {
            exec_count += 1;
        });

        my_test_suite.add_test("one", [&exec_count] () { equals(exec_count, 1); });
        my_test_suite.add_test("two", [&exec_count] () { equals(exec_count, 1); });
        my_test_suite.execute();

        equals(exec_count, 1);
    });

    suite.add_test("before each hooks", [] () {
        test_suite my_test_suite{nullptr}; // Test subject.
        int exec_count = 0;

        my_test_suite.exec_before_each([&exec_count] () {
            exec_count += 1;
        });

        my_test_suite.add_test("one", [&exec_count] () { equals(exec_count, 1); });
        my_test_suite.add_test("two", [&exec_count] () { equals(exec_count, 2); });
        my_test_suite.execute();

        equals(exec_count, 2);
    });

    suite.add_test("after all hooks", [] () {
        test_suite my_test_suite{nullptr}; // Test subject.
        int exec_count = 0;

        my_test_suite.exec_after_all([&exec_count] () {
            exec_count += 1;
        });

        my_test_suite.add_test("one", [&exec_count] () { equals(exec_count, 0); });
        my_test_suite.add_test("two", [&exec_count] () { equals(exec_count, 0); });
        my_test_suite.execute();

        equals(exec_count, 1);
    });

    suite.add_test("after each hooks", [] () {
        test_suite my_test_suite{nullptr}; // Test subject.
        int exec_count = 0;

        my_test_suite.exec_after_each([&exec_count] () {
            exec_count += 1;
        });

        my_test_suite.add_test("one", [&exec_count] () { equals(exec_count, 0); });
        my_test_suite.add_test("two", [&exec_count] () { equals(exec_count, 1); });
        my_test_suite.execute();

        equals(exec_count, 2);
    });

    suite.add_test("before each hooks (skip)", [] () {
        test_suite my_test_suite{nullptr}; // Test subject.
        int exec_count = 0;

        my_test_suite.exec_before_each([&exec_count] () {
            exec_count += 1;
        });

        my_test_suite.add_test("one", [&exec_count] () { equals(exec_count, 1); });
        my_test_suite.add_test("skip", [] () {}, [] () { return true; });
        my_test_suite.execute();

        equals(exec_count, 1);
    });

    suite.add_test("after each hooks (skip)", [] () {
        test_suite my_test_suite{nullptr}; // Test subject.
        int exec_count = 0;

        my_test_suite.exec_after_each([&exec_count] () {
            exec_count += 1;
        });

        my_test_suite.add_test("one", [&exec_count] () { equals(exec_count, 0); });
        my_test_suite.add_test("skip", [] () {}, [] () { return true; });
        my_test_suite.execute();

        equals(exec_count, 1);
    });

    return suite.main(argc, argv);
}
