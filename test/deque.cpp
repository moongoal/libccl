#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/deque.hpp>

using namespace ccl;

template<typename T>
using test_deque = deque<T, counting_test_allocator>;

constexpr uint32_t constructed_value = 0x1234;

struct spy {
    uint32_t construction_magic;
    std::function<void()> on_destroy;

    spy() : construction_magic{constructed_value} {}
    spy(const auto& on_destroy) : construction_magic{constructed_value}, on_destroy{on_destroy} {}
    spy(const spy&) = default;

    spy(spy&& other) : construction_magic{constructed_value}, on_destroy{other.on_destroy} {
        other.on_destroy = nullptr;
    }

    constexpr spy& operator=(const spy& other) {
        on_destroy = other.on_destroy;

        return *this;
    }

    ~spy() {
        if(!construction_magic) {
            std::abort();
        }

        if(on_destroy) {
            on_destroy();
        }

        construction_magic = 0;
    }
};

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("ctor", [] () {
        test_deque<int> q;

        equals(q.capacity(), 0U);
        equals(q.capacity_back(), 0U);
        equals(q.capacity_front(), 0U);
        equals(q.size(), 0U);
        equals(q.is_empty(), true);
        equals(q.data(), nullptr);
    });

    suite.add_test("dtor", [] () {
        int destruction_counter = 0;

        {
            test_deque<spy> q;
            const auto on_destroy = [&destruction_counter] () { destruction_counter += 1; };

            q.emplace_back(on_destroy);
            q.emplace_back(on_destroy);
        }

        equals(destruction_counter, 2);
    });

    suite.add_test("reserve (empty)", [] () {
        test_deque<int> q;

        q.reserve(16);

        check(q.capacity() >= 16U);
        equals(q.capacity_back(), 8U);
        equals(q.capacity_front(), 8U);
        equals(q.size(), 0U);
        equals(q.is_empty(), true);
        differs(q.data(), nullptr);
    });

    suite.add_test("push_back (empty)", [] () {
        test_deque<int> q;

        q.push_back(5);

        differs(q.capacity(), 0U);
        equals(q.size(), 1U);
        equals(q.is_empty(), false);
        differs(q.data(), nullptr);
    });

    suite.add_test("push_back", [] () {
        test_deque<int> q;

        q.push_back(5);
        q.push_back(10);

        differs(q.capacity(), 0U);
        equals(q.size(), 2U);
        equals(q.is_empty(), false);
        differs(q.data(), nullptr);
    });

    suite.add_test("emplace_back (empty)", [] () {
        test_deque<spy> q;
        int destruction_counter = 0;

        q.emplace_back([&destruction_counter] () { destruction_counter += 1; });

        differs(q.capacity(), 0U);
        equals(q.size(), 1U);
        equals(q.is_empty(), false);
        differs(q.data(), nullptr);
        equals(q.cfront()->construction_magic, constructed_value);
    });

    suite.add_test("emplace_back", [] () {
        test_deque<spy> q;
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter += 1; };

        q.emplace_back();
        q.emplace_back(on_destroy);

        differs(q.capacity(), 0U);
        equals(q.size(), 2U);
        equals(q.is_empty(), false);
        differs(q.data(), nullptr);
        equals(q.cfront()->construction_magic, constructed_value);
        equals((q.cfront() + 1)->construction_magic, constructed_value);
    });

    suite.add_test("push_front", [] () {
        test_deque<int> q;

        q.push_front(5);
        q.push_front(10);

        differs(q.capacity(), 0U);
        equals(q.size(), 2U);
        equals(q.is_empty(), false);
        differs(q.data(), nullptr);
    });

    suite.add_test("emplace_front (empty)", [] () {
        test_deque<spy> q;
        int destruction_counter = 0;

        q.emplace_front([&destruction_counter] () { destruction_counter += 1; });

        differs(q.capacity(), 0U);
        equals(q.size(), 1U);
        equals(q.is_empty(), false);
        differs(q.data(), nullptr);
        equals(q.cfront()->construction_magic, constructed_value);
    });

    suite.add_test("emplace_front", [] () {
        test_deque<spy> q;
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter += 1; };

        q.emplace_front();
        q.emplace_front(on_destroy);

        differs(q.capacity(), 0U);
        equals(q.size(), 2U);
        equals(q.is_empty(), false);
        differs(q.data(), nullptr);
        equals(q.cfront()->construction_magic, constructed_value);
        equals((q.cfront() + 1)->construction_magic, constructed_value);
    });

    return suite.main(argc, argv);
}
