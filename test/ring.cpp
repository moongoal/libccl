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
        "enqueue/dequeue",
        []() {
            test_ring<int> v{default_capacity};

            v.enqueue(1);
            v.enqueue(2);
            v.enqueue(3);

            equals(v.size(), 3);

            equals(*v, 1);
            v.dequeue();

            equals(*v, 2);
            v.dequeue();

            equals(*v, 3);

            equals(v.size(), 1);
            equals(v.capacity(), default_capacity);
        }
    );

    suite.add_test(
        "dequeue (empty)",
        []() {
            test_ring<int> v{default_capacity};

            throws<std::out_of_range>([&v] () {
                v.dequeue();
            });
        },
        skip_if_exceptions_disabled
    );

    suite.add_test(
        "enqueue (full)",
        []() {
            test_ring<int> v{default_capacity};

            throws<std::out_of_range>([&v] () {
                v.dequeue();
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
                v.enqueue(2);
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
                v.enqueue(2);
            }

            equals(v.is_full(), true);
        }
    );

    suite.add_test(
        "clear", []() {
            test_ring<int> v{default_capacity};

            v.enqueue(1);
            v.enqueue(2);
            v.enqueue(3);

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

        v.enqueue(spy1);
        v.enqueue(spy2);
        v.enqueue(spy3);

        test_ring<spy> v2{v};

        equals(destruction_counter, 0);

        equals(v.size(), 3);
        equals(v.capacity(), default_capacity);

        equals(v2.size(), 3);
        equals(v2.capacity(), default_capacity);

        differs(v.data(), v2.data());

        equals((*v2).construction_magic, constructed_value);
        v2.dequeue();

        equals((*v2).construction_magic, constructed_value);
        v2.dequeue();

        equals((*v2).construction_magic, constructed_value);
        v2.dequeue();
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

        v.enqueue(spy1);
        v.enqueue(spy2);
        v.enqueue(spy3);

        test_ring<spy> v2{std::move(v)};

        equals(destruction_counter, 0);

        equals(v.size(), 3);
        equals(v.capacity(), default_capacity);

        equals(v2.size(), 3);
        equals(v2.capacity(), default_capacity);

        differs(v.data(), v2.data());

        equals((*v2).construction_magic, constructed_value);
        v2.dequeue();

        equals((*v2).construction_magic, constructed_value);
        v2.dequeue();

        equals((*v2).construction_magic, constructed_value);
        v2.dequeue();
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

            v.enqueue(spy1);
            v.enqueue(spy2);
            v.enqueue(spy3);
        }

        equals(destruction_counter, 6);
    });

    suite.add_test("operator = (copy - uninitialized)", [] () {
        test_ring<int> v{default_capacity};
        test_ring<int> v2{default_capacity};

        v.enqueue(1);
        v.enqueue(2);
        v.enqueue(3);

        v2 = v;

        equals(v2.size(), 3);
        equals(v2.capacity(), default_capacity);

        equals(*v2, 1);
        v2.dequeue();

        equals(*v2, 2);
        v2.dequeue();

        equals(*v2, 3);
    });

    suite.add_test("operator = (copy - initialized)", [] () {
        test_ring<int> v{default_capacity};
        test_ring<int> v2{default_capacity};

        v.enqueue(1);
        v.enqueue(2);
        v.enqueue(3);

        v2.enqueue(4);
        v2.enqueue(5);
        v2.enqueue(6);

        v2 = v;

        equals(v2.size(), 3);
        equals(v2.capacity(), default_capacity);

        equals(*v2, 1);
        v2.dequeue();

        equals(*v2, 2);
        v2.dequeue();

        equals(*v2, 3);
    });

    suite.add_test("operator = (copy - initialized, to larger capacity)", [] () {
        // This must not be `test_ring` because the allocator is not
        // stateless and won't thest the right operator= deta path.
        ring<int> v{default_capacity};
        ring<int> v2{default_capacity + 1};

        v.enqueue(1);
        v.enqueue(2);
        v.enqueue(3);

        v2.enqueue(4);
        v2.enqueue(5);

        v2 = v;

        equals(v2.size(), 3);
        equals(v2.capacity(), default_capacity + 1);

        equals(*v2, 1);
        v2.dequeue();

        equals(*v2, 2);
        v2.dequeue();

        equals(*v2, 3);
    });

    suite.add_test("operator = (move)", [] () {
        test_ring<int> v{default_capacity};
        test_ring<int> v2{default_capacity};

        v.enqueue(1);
        v.enqueue(2);
        v.enqueue(3);

        v2.enqueue(4);
        v2.enqueue(5);
        v2.enqueue(6);

        auto * const old_v_data = v.data();

        v2 = std::move(v);

        differs(v.data(), old_v_data);

        equals(v2.size(), 3U);
        equals(v2.capacity(), default_capacity);
        equals(v2.data(), old_v_data);

        equals(*v2, 1);
        v2.dequeue();

        equals(*v2, 2);
        v2.dequeue();

        equals(*v2, 3);
    });

    suite.add_test("ctor (range)", [] () {
        std::forward_list<int> my_list {1, 2, 3, 4, 5};
        test_ring<int> v{my_list};

        equals(v.size(), 5);
        equals(*v, 1);
        v.dequeue();

        equals(*v, 2);
        v.dequeue();

        equals(*v, 3);
        v.dequeue();

        equals(*v, 4);
        v.dequeue();

        equals(*v, 5);
    });

    suite.add_test("emplace_back", [] () {
        test_ring<dummy> v {default_capacity};

        v.emplace_enqueue(dummy{1});
        v.emplace_enqueue(dummy{2});
        v.emplace_enqueue(dummy{3});
        v.emplace_enqueue(dummy{4});

        equals(v.size(), 4);

        equals((*v).value, 2);
        v.dequeue();

        equals((*v).value, 3);
        v.dequeue();

        equals((*v).value, 4);
        v.dequeue();

        equals((*v).value, 5);
        equals(v.size(), 1);
    });

    return suite.main(argc, argv);
}
