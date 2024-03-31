#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/pointer/weak.hpp>

using namespace ccl;

using test_wptr = weak_ptr<int, counting_test_allocator>;
using test_sptr = shared_ptr<int, counting_test_allocator>;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("default ctor", [] () {
        test_wptr ptr;

        equals(ptr.get(), nullptr);
        equals(ptr.get_ctrl_block(), nullptr);
        equals(static_cast<bool>(ptr.lock()), false);
    });

    suite.add_test("ctor (nullptr)", [] () {
        test_wptr ptr{nullptr};

        equals(ptr.get(), nullptr);
        equals(ptr.get_ctrl_block(), nullptr);
        equals(static_cast<bool>(ptr.lock()), false);
    });

    suite.add_test("ctor (from shared)", [] () {
        test_sptr shared = make_shared_default<int, counting_test_allocator>(0);
        test_wptr weak = shared;

        equals(weak.get(), shared.get());
        equals(weak.get_ctrl_block(), shared.get_ctrl_block());
        equals(static_cast<bool>(weak.lock()), true);
    });

    suite.add_test("ctor (copy)", [] () {
        test_sptr shared = make_shared_default<int, counting_test_allocator>(0);
        test_wptr weak1 = shared;
        test_wptr weak2 = weak1;

        equals(weak1.get_ctrl_block(), weak2.get_ctrl_block());
        equals(weak1.get(), weak2.get());
        equals(static_cast<bool>(weak2.lock()), true);

        equals(
            shared_ptr_ctrl_block_base::extract_weak_ref_count(weak1.get_ctrl_block()->get_counters()),
            2
        );

        equals(
            shared_ptr_ctrl_block_base::extract_shared_ref_count(weak1.get_ctrl_block()->get_counters()),
            1
        );
    });

    suite.add_test("ctor (move)", [] () {
        test_sptr shared = make_shared_default<int, counting_test_allocator>(0);
        test_wptr weak1 = shared;
        test_wptr weak2 = std::move(weak1);

        equals(weak2.get(), shared.get());
        equals(weak2.get_ctrl_block(), shared.get_ctrl_block());
        equals(static_cast<bool>(weak2.lock()), true);

        equals(
            shared_ptr_ctrl_block_base::extract_weak_ref_count(weak2.get_ctrl_block()->get_counters()),
            1
        );

        equals(
            shared_ptr_ctrl_block_base::extract_shared_ref_count(weak2.get_ctrl_block()->get_counters()),
            1
        );
    });

    suite.add_test("dtor", [] () {
        int n = 0;
        auto my_deleter = [&n] (int *x, shared_ptr_ctrl_block_base&) { n += 1; delete x; };

        {
            test_sptr s{new int, my_deleter};
            test_wptr w{s};
        }

        equals(n, 1);
    });

    suite.add_test("operator =(copy)", [] () {
        test_sptr shared = make_shared_default<int, counting_test_allocator>(0);
        test_wptr weak1 = shared;
        test_wptr weak2;

        weak2 = weak1;

        equals(weak1.get_ctrl_block(), weak2.get_ctrl_block());
        equals(weak1.get(), weak2.get());
        equals(static_cast<bool>(weak2.lock()), true);

        equals(
            shared_ptr_ctrl_block_base::extract_weak_ref_count(weak1.get_ctrl_block()->get_counters()),
            2
        );

        equals(
            shared_ptr_ctrl_block_base::extract_shared_ref_count(weak1.get_ctrl_block()->get_counters()),
            1
        );
    });

    suite.add_test("operator = (move)", [] () {
        test_sptr shared = make_shared_default<int, counting_test_allocator>(0);
        test_wptr weak1 = shared;
        test_wptr weak2;

        weak2 = std::move(weak1);

        equals(weak2.get(), shared.get());
        equals(weak2.get_ctrl_block(), shared.get_ctrl_block());
        equals(static_cast<bool>(weak2.lock()), true);

        equals(
            shared_ptr_ctrl_block_base::extract_weak_ref_count(weak2.get_ctrl_block()->get_counters()),
            1
        );

        equals(
            shared_ptr_ctrl_block_base::extract_shared_ref_count(weak2.get_ctrl_block()->get_counters()),
            1
        );
    });

    suite.add_test("lock", [] () {
        test_sptr shared = make_shared_new<int, counting_test_allocator>();

        {
            test_wptr weak = shared;

            {
                test_sptr locked = weak.lock();

                *locked = 5;
            }

            equals(*shared, 5);
        }
    });

    return suite.main(argc, argv);
}
