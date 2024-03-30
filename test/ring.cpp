#include <forward_list>
#include <functional>
#include <ccl/features.hpp>
#include <ccl/test/test.hpp>
#include <ccl/ring.hpp>
#include <ccl/test/counting-test-allocator.hpp>

using namespace ccl;

template<typename T>
using test_ring = ring<T, counting_test_allocator>;

constexpr uint32_t constructed_value = 0x1234;
constexpr uint32_t default_capacity = 16;

struct spy {
    uint32_t construction_magic;
    std::function<void()> on_destroy;

    spy() {
        construction_magic = constructed_value;
    }

    ~spy() {
        if(on_destroy) {
            on_destroy();
        }

        construction_magic = 0;
    }
};

struct dummy {
    int value;

    dummy() : value{999} {}
    dummy(int value) : value{value + 1} {}
};

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test(
        "ctor",
        []() {
            test_ring<int> v{default_capacity};

            equals(v.capacity(), default_capacity);
            equals(v.size(), 0);
            differs(v.data(), nullptr);
        }
    );

    suite.add_test(
        "enqueue_back/dequeue_front",
        []() {
            test_ring<int> v{default_capacity};

            v.enqueue_back(1);
            v.enqueue_back(2);
            v.enqueue_back(3);

            equals(v.size(), 3);

            equals(v.get_front(), 1);
            v.dequeue_front();

            equals(v.get_front(), 2);
            v.dequeue_front();

            equals(v.get_front(), 3);

            equals(v.size(), 1);
            equals(v.capacity(), default_capacity);
        }
    );

    suite.add_test(
        "enqueue_front/dequeue_back",
        []() {
            test_ring<int> v{default_capacity};

            v.enqueue_front(1);
            v.enqueue_front(2);
            v.enqueue_front(3);

            equals(v.size(), 3);

            equals(v.get_back(), 1);
            v.dequeue_back();

            equals(v.get_back(), 2);
            v.dequeue_back();

            equals(v.get_back(), 3);

            equals(v.size(), 1);
            equals(v.capacity(), default_capacity);
        }
    );

    suite.add_test(
        "dequeue_front (empty)",
        []() {
            test_ring<int> v{default_capacity};

            throws<std::out_of_range>([&v] () {
                v.dequeue_front();
            });
        },
        skip_if_exceptions_disabled
    );

    suite.add_test(
        "dequeue_back (empty)",
        []() {
            test_ring<int> v{default_capacity};

            throws<std::out_of_range>([&v] () {
                v.dequeue_back();
            });
        },
        skip_if_exceptions_disabled
    );

    suite.add_test(
        "enqueue_back (full)",
        []() {
            test_ring<int> v{default_capacity};

            for(size_t i = 0; i < v.capacity(); ++i) {
                v.enqueue_back(2);
            }

            throws<std::out_of_range>([&v] () {
                v.enqueue_back(2);
            });
        },
        skip_if_exceptions_disabled
    );

    suite.add_test(
        "enqueue_front (full)",
        []() {
            test_ring<int> v{default_capacity};

            for(size_t i = 0; i < v.capacity(); ++i) {
                v.enqueue_front(2);
            }

            throws<std::out_of_range>([&v] () {
                v.enqueue_front(2);
            });
        },
        skip_if_exceptions_disabled
    );

    suite.add_test(
        "is_empty",
        []() {
            test_ring<int> v{default_capacity};

            equals(v.is_empty(), true);

            for(size_t i = 0; i < v.capacity(); ++i) {
                v.enqueue_back(2);
                equals(v.is_empty(), false);
            }
        }
    );

    suite.add_test(
        "is_full",
        []() {
            test_ring<int> v{default_capacity};

            for(size_t i = 0; i < v.capacity(); ++i) {
                equals(v.is_full(), false);
                v.enqueue_back(2);
            }

            equals(v.is_full(), true);
        }
    );

    suite.add_test(
        "clear", []() {
            test_ring<int> v{default_capacity};

            v.enqueue_back(1);
            v.enqueue_back(2);
            v.enqueue_back(3);

            int * const old_data = v.data();

            v.clear();

            equals(v.size(), 0);
            equals(v.capacity(), default_capacity);
            equals(old_data, v.data());
        }
    );

    suite.add_test("ctor (copy)", [] () {
        int destruction_counter = 0;
        test_ring<spy> v{default_capacity};

        spy spy1;
        spy spy2;
        spy spy3;

        auto dtor = [&destruction_counter] () { destruction_counter++; };

        spy1.on_destroy = dtor;
        spy2.on_destroy = dtor;
        spy3.on_destroy = dtor;

        v.enqueue_back(spy1);
        v.enqueue_back(spy2);
        v.enqueue_back(spy3);

        test_ring<spy> v2{v};

        equals(destruction_counter, 0);

        equals(v.size(), 3);
        equals(v.capacity(), default_capacity);

        equals(v2.size(), 3);
        equals(v2.capacity(), default_capacity);

        differs(v.data(), v2.data());

        equals((v2.get_front()).construction_magic, constructed_value);
        v2.dequeue_front();

        equals((v2.get_front()).construction_magic, constructed_value);
        v2.dequeue_front();

        equals((v2.get_front()).construction_magic, constructed_value);
        v2.dequeue_front();
    });

    suite.add_test("ctor (move)", [] () {
        int destruction_counter = 0;
        test_ring<spy> v{default_capacity};

        spy spy1;
        spy spy2;
        spy spy3;

        auto dtor = [&destruction_counter] () { destruction_counter++; };

        spy1.on_destroy = dtor;
        spy2.on_destroy = dtor;
        spy3.on_destroy = dtor;

        v.enqueue_back(spy1);
        v.enqueue_back(spy2);
        v.enqueue_back(spy3);

        test_ring<spy> v2{std::move(v)};

        equals(destruction_counter, 0);

        equals(v.size(), 3);
        equals(v.capacity(), default_capacity);

        equals(v2.size(), 3);
        equals(v2.capacity(), default_capacity);

        differs(v.data(), v2.data());

        equals((v2.get_front()).construction_magic, constructed_value);
        v2.dequeue_front();

        equals((v2.get_front()).construction_magic, constructed_value);
        v2.dequeue_front();

        equals((v2.get_front()).construction_magic, constructed_value);
        v2.dequeue_front();
    });

    suite.add_test("dtor", [] () {
        int destruction_counter = 0;

        {
            test_ring<spy> v{default_capacity};

            spy spy1;
            spy spy2;
            spy spy3;

            auto dtor = [&destruction_counter] () { destruction_counter++; };

            spy1.on_destroy = dtor;
            spy2.on_destroy = dtor;
            spy3.on_destroy = dtor;

            v.enqueue_back(spy1);
            v.enqueue_back(spy2);
            v.enqueue_back(spy3);
        }

        equals(destruction_counter, 6);
    });

    suite.add_test("operator = (copy - uninitialized)", [] () {
        test_ring<int> v{default_capacity};
        test_ring<int> v2{default_capacity};

        v.enqueue_back(1);
        v.enqueue_back(2);
        v.enqueue_back(3);

        v2 = v;

        equals(v2.size(), 3);
        equals(v2.capacity(), default_capacity);

        equals(v2.get_front(), 1);
        v2.dequeue_front();

        equals(v2.get_front(), 2);
        v2.dequeue_front();

        equals(v2.get_front(), 3);
    });

    suite.add_test("operator = (copy - initialized)", [] () {
        test_ring<int> v{default_capacity};
        test_ring<int> v2{default_capacity};

        v.enqueue_back(1);
        v.enqueue_back(2);
        v.enqueue_back(3);

        v2.enqueue_back(4);
        v2.enqueue_back(5);
        v2.enqueue_back(6);

        v2 = v;

        equals(v2.size(), 3);
        equals(v2.capacity(), default_capacity);

        equals(v2.get_front(), 1);
        v2.dequeue_front();

        equals(v2.get_front(), 2);
        v2.dequeue_front();

        equals(v2.get_front(), 3);
    });

    suite.add_test("operator = (copy - initialized, to larger capacity)", [] () {
        // This must not be `test_ring` because the allocator is not
        // stateless and won't thest the right operator= deta path.
        ring<int> v{default_capacity};
        ring<int> v2{default_capacity + 1};

        v.enqueue_back(1);
        v.enqueue_back(2);
        v.enqueue_back(3);

        v2.enqueue_back(4);
        v2.enqueue_back(5);

        v2 = v;

        equals(v2.size(), 3);
        equals(v2.capacity(), default_capacity + 1);

        equals(v2.get_front(), 1);
        v2.dequeue_front();

        equals(v2.get_front(), 2);
        v2.dequeue_front();

        equals(v2.get_front(), 3);
    });

    suite.add_test("operator = (move)", [] () {
        test_ring<int> v{default_capacity};
        test_ring<int> v2{default_capacity};

        v.enqueue_back(1);
        v.enqueue_back(2);
        v.enqueue_back(3);

        v2.enqueue_back(4);
        v2.enqueue_back(5);
        v2.enqueue_back(6);

        auto * const old_v_data = v.data();

        v2 = std::move(v);

        differs(v.data(), old_v_data);

        equals(v2.size(), 3U);
        equals(v2.capacity(), default_capacity);
        equals(v2.data(), old_v_data);

        equals(v2.get_front(), 1);
        v2.dequeue_front();

        equals(v2.get_front(), 2);
        v2.dequeue_front();

        equals(v2.get_front(), 3);
    });

    suite.add_test("ctor (range)", [] () {
        std::forward_list<int> my_list {1, 2, 3, 4, 5};
        test_ring<int> v{my_list};

        equals(v.size(), 5);
        equals(v.get_front(), 1);
        v.dequeue_front();

        equals(v.get_front(), 2);
        v.dequeue_front();

        equals(v.get_front(), 3);
        v.dequeue_front();

        equals(v.get_front(), 4);
        v.dequeue_front();

        equals(v.get_front(), 5);
    });

    suite.add_test("emplace_back", [] () {
        test_ring<dummy> v {default_capacity};

        v.emplace_back(dummy{1});
        v.emplace_back(dummy{2});
        v.emplace_back(dummy{3});
        v.emplace_back(dummy{4});

        equals(v.size(), 4);

        equals((v.get_front()).value, 2);
        v.dequeue_front();

        equals((v.get_front()).value, 3);
        v.dequeue_front();

        equals((v.get_front()).value, 4);
        v.dequeue_front();

        equals((v.get_front()).value, 5);
        equals(v.size(), 1);
    });

    suite.add_test("emplace_front", [] () {
        test_ring<dummy> v {default_capacity};

        v.emplace_front(dummy{1});
        v.emplace_front(dummy{2});
        v.emplace_front(dummy{3});
        v.emplace_front(dummy{4});

        equals(v.size(), 4);

        equals((v.get_back()).value, 2);
        v.dequeue_back();

        equals((v.get_back()).value, 3);
        v.dequeue_back();

        equals((v.get_back()).value, 4);
        v.dequeue_back();

        equals((v.get_back()).value, 5);
        equals(v.size(), 1);
    });

    return suite.main(argc, argv);
}
