#include <functional>
#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/pointer/shared.hpp>

using namespace ccl;

template<typename T>
using test_shared_ptr = shared_ptr<T, counting_test_allocator>;

using spy_lifetime_callback = std::function<void()>;

struct spy {
    spy_lifetime_callback on_destroy;

    spy(spy_lifetime_callback on_create, spy_lifetime_callback on_destroy) : on_destroy{on_destroy} {
        if(on_create) {
            on_create();
        }
    }

    ~spy() { if(on_destroy) { on_destroy(); } }
};

struct fail_stub {
    fail_stub() {
        fail();
    }

    ~fail_stub() { fail(); }
};

struct A {};
struct B : public A {};

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor (default)", [] () {
        test_shared_ptr<fail_stub> ptr;

        equals(ptr.get(), nullptr);
    });

    suite.add_test("ctor (nullptr)", [] () {
        test_shared_ptr<fail_stub> ptr{nullptr};

        equals(ptr.get(), nullptr);
    });

    suite.add_test("ctor (pointer)", [] () {
        int count = 0;

        {
            auto raw_ptr = new spy{
                [&count] () { count += 1; },
                [&count] () { count -= 1; }
            };

            test_shared_ptr<spy> ptr{raw_ptr, test_shared_ptr<spy>::new_tag};

            equals(ptr.get(), raw_ptr);
            equals(count, 1);
        }

        equals(count, 0);
    });

    suite.add_test("ctor (copy)", [] () {
        int count = 0;

        {
            auto raw_ptr = new spy{
                [&count] () { count += 1; },
                [&count] () { count -= 1; }
            };

            test_shared_ptr<spy> ptr{raw_ptr, test_shared_ptr<spy>::new_tag};

            {
                test_shared_ptr<spy> ptr2{ptr};

                equals(ptr.get(), raw_ptr);
                equals(ptr2.get(), raw_ptr);
                equals(count, 1);
            }

            equals(count, 1);
        }


        equals(count, 0);
    });

    suite.add_test("ctor (copy - subclass)", [] () {
        test_shared_ptr<A> ptr{new A{}, test_shared_ptr<A>::new_tag};
        test_shared_ptr<B> ptr2{ptr};

        equals(ptr2.use_count(), 2);
    });

    suite.add_test("ctor (move - subclass)", [] () {
        test_shared_ptr<A> ptr{new A{}, test_shared_ptr<A>::new_tag};
        test_shared_ptr<B> ptr2{std::move(ptr)};

        equals(ptr2.use_count(), 1);
    });

    suite.add_test("ctor (move)", [] () {
        int count = 0;

        {
            auto raw_ptr = new spy{
                [&count] () { count += 1; },
                [&count] () { count -= 1; }
            };

            test_shared_ptr<spy> ptr{raw_ptr, test_shared_ptr<spy>::new_tag};

            {
                test_shared_ptr<spy> ptr2{std::move(ptr)};

                equals(ptr2.get(), raw_ptr);
                equals(count, 1);
            }

            // Deleted via ptr2
            equals(count, 0);
        }

        // Should not delete again
        equals(count, 0);
    });

    suite.add_test("dtor (custom deleter)", [] () {
        int count = 0;

        {
            auto raw_ptr = new spy{
                [&count] () { count += 1; },
                [&count] () { count -= 1; }
            };

            test_shared_ptr<spy> ptr{raw_ptr, [&count] (spy * ptr, shared_ptr_ctrl_block_base&) { count = 100; delete ptr; }};

            equals(ptr.get(), raw_ptr);
            equals(count, 1);
        }

        // Set via custom deleter, decremented via dtor
        equals(count, 99);
    });

    suite.add_test("operator ==", [] () {
        test_shared_ptr<int> ptr1{new int, test_shared_ptr<int>::new_tag};
        test_shared_ptr<int> ptr2{ptr1};

        check(ptr1 == ptr2);
    });

    suite.add_test("operator !=", [] () {
        test_shared_ptr<int> ptr1{new int, test_shared_ptr<int>::new_tag};
        test_shared_ptr<int> ptr2{new int, test_shared_ptr<int>::new_tag};

        check(ptr1.get() != ptr2.get());
    });

    suite.add_test("operator >", [] () {
        test_shared_ptr<int> ptr1{new int, test_shared_ptr<int>::new_tag};
        test_shared_ptr<int> ptr2{new int, test_shared_ptr<int>::new_tag};

        if(ptr1.get() > ptr2.get()) {
            check(ptr1 > ptr2);
        } else {
            check(ptr2 > ptr1);
        }
    });

    suite.add_test("operator <", [] () {
        test_shared_ptr<int> ptr1{new int, test_shared_ptr<int>::new_tag};
        test_shared_ptr<int> ptr2{new int, test_shared_ptr<int>::new_tag};

        if(ptr1.get() > ptr2.get()) {
            check(ptr2 < ptr1);
        } else {
            check(ptr1 < ptr2);
        }
    });

    suite.add_test("operator >=", [] () {
        test_shared_ptr<int> ptr1{new int, test_shared_ptr<int>::new_tag};
        test_shared_ptr<int> ptr2{new int, test_shared_ptr<int>::new_tag};
        test_shared_ptr<int> ptr1_copy{ptr1};

        if(ptr1.get() > ptr2.get()) {
            check(ptr1 >= ptr2);
        } else {
            check(ptr2 >= ptr1);
        }

        check(ptr1 >= ptr1_copy);
    });

    suite.add_test("operator <=", [] () {
        test_shared_ptr<int> ptr1{new int, test_shared_ptr<int>::new_tag};
        test_shared_ptr<int> ptr2{new int, test_shared_ptr<int>::new_tag};
        test_shared_ptr<int> ptr1_copy{ptr1};

        if(ptr1.get() > ptr2.get()) {
            check(ptr2 <= ptr1);
        } else {
            check(ptr1 <= ptr2);
        }

        check(ptr1 <= ptr1_copy);
    });

    suite.add_test("operator *", [] () {
        test_shared_ptr<int> ptr1{new int{5}, test_shared_ptr<int>::new_tag};

        equals(*ptr1, 5);

        *ptr1 = 6;
        equals(*ptr1, 6);
    });

    suite.add_test("operator ->", [] () {
        struct S { int x = 5; };

        test_shared_ptr<S> ptr1{new S{}, test_shared_ptr<S>::new_tag};

        equals(ptr1->x, 5);

        ptr1->x = 6;
        equals(ptr1->x, 6);
    });

    suite.add_test("operator []", [] () {
        test_shared_ptr<int[]> ptr{new int[3], test_shared_ptr<int[]>::new_tag};

        ptr[0] = 1;
        ptr[1] = 2;
        ptr[2] = 3;

        equals(ptr[0], 1);
        equals(ptr[1], 2);
        equals(ptr[2], 3);
    });

    suite.add_test("use_count()", [] () {
        test_shared_ptr<int[]> ptr{new int[3], test_shared_ptr<int[]>::new_tag};

        equals(ptr.use_count(), 1);

        test_shared_ptr<int[]> ptr2{ptr};

        equals(ptr.use_count(), 2);
    });

    suite.add_test("use_count() (empty)", [] () {
        test_shared_ptr<int[]> ptr;

        equals(ptr.use_count(), 0);
    });

    suite.add_test("reset()", [] () {
        test_shared_ptr<int> ptr{new int, test_shared_ptr<int>::new_tag};

        ptr.reset();

        equals(ptr.use_count(), 0);
        equals(ptr.get(), nullptr);
    });

    suite.add_test("operator = (copy)", [] () {
        int count = 0;

        {
            auto raw_ptr = new spy{
                [&count] () { count += 1; },
                [&count] () { count -= 1; }
            };

            auto raw_ptr2 = new spy{
                [&count] () { count += 2; },
                [&count] () { count -= 2; }
            };

            test_shared_ptr<spy> ptr{raw_ptr, test_shared_ptr<spy>::new_tag};
            test_shared_ptr<spy> ptr2{raw_ptr2, test_shared_ptr<spy>::new_tag};

            ptr = ptr2;

            equals(count, 2);
            equals(ptr.use_count(), 2);
        }

        equals(count, 0);
    });

    suite.add_test("operator = (move)", [] () {
        int count = 0;

        {
            auto raw_ptr = new spy{
                [&count] () { count += 1; },
                [&count] () { count -= 1; }
            };

            auto raw_ptr2 = new spy{
                [&count] () { count += 2; },
                [&count] () { count -= 2; }
            };

            test_shared_ptr<spy> ptr{raw_ptr, test_shared_ptr<spy>::new_tag};
            test_shared_ptr<spy> ptr2{raw_ptr2, test_shared_ptr<spy>::new_tag};

            ptr = std::move(ptr2);

            equals(ptr.use_count(), 1);
        }

        equals(count, 0);
    });

    return suite.main(argc, argv);
}
