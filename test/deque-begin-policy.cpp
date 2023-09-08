#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/deque.hpp>

using namespace ccl;

template<typename T>
using test_deque = deque<T, deque_reset_policy::begin, 0, counting_test_allocator>;

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

    suite.add_test("ctor (copy, non-trivial)", [] () {
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter += 1; };

        test_deque<spy> q2;

        q2.emplace_back(on_destroy);
        q2.emplace_back(on_destroy);

        test_deque<spy> q{q2};

        differs(q.capacity(), 0);
        equals(q.size(), 2U);
        differs(q.is_empty(), true);
        differs(q.data(), nullptr);
        differs(q.data(), q2.data());

        equals(q.begin()->construction_magic, constructed_value);
        equals((q.begin() + 1)->construction_magic, constructed_value);

        q2.destroy();
        q.destroy();

        equals(destruction_counter, 4);
    });

    suite.add_test("ctor (copy, trivial)", [] () {
        test_deque<int> q2;

        q2.emplace_back(1);
        q2.emplace_back(2);

        test_deque<int> q{q2};

        differs(q.capacity(), 0);
        equals(q.size(), 2U);
        differs(q.is_empty(), true);
        differs(q.data(), nullptr);
        differs(q.data(), q2.data());

        equals(*q.begin(), 1);
        equals(*(q.begin() + 1), 2);

        q2.destroy();
        q.destroy();
    });

    suite.add_test("ctor (move)", [] () {
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter += 1; };

        test_deque<spy> q2;

        q2.emplace_back(on_destroy);
        q2.emplace_back(on_destroy);

        auto * const old_q2_data = q2.data();

        test_deque<spy> q{std::move(q2)};

        differs(q.capacity(), 0);
        equals(q.size(), 2U);
        differs(q.is_empty(), true);
        equals(q.data(), old_q2_data);

        q2.destroy();
        q.destroy();

        equals(destruction_counter, 2);
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
        check(q.capacity_back() >= 16U);
        equals(q.capacity_front(), 0U);
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
        equals(q.cfront().construction_magic, constructed_value);
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
        equals(q.cfront().construction_magic, constructed_value);
        equals((q.cbegin() + 1)->construction_magic, constructed_value);
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
        equals(q.cfront().construction_magic, constructed_value);
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
        equals(q.cfront().construction_magic, constructed_value);
        equals((q.cbegin() + 1)->construction_magic, constructed_value);
    });

    suite.add_test("clear", [] () {
        int destruction_counter = 0;
        test_deque<spy> q;
        const auto on_destroy = [&destruction_counter] () { destruction_counter += 1; };

        q.emplace_back(on_destroy);
        q.emplace_back(on_destroy);

        q.clear();

        equals(destruction_counter, 2);
        equals(q.size(), 0);
        differs(q.capacity(), 0);
    });

    suite.add_test("clear (empty)", [] () {
        test_deque<int> q;

        q.clear();

        equals(q.size(), 0);
        equals(q.capacity(), 0);
    });

    suite.add_test("pop_front", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        q.pop_front();
        q.pop_front();
        q.pop_front();

        throws<std::out_of_range>([&q] () {
            q.pop_front();
        });
    });

    suite.add_test("pop_back", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        q.pop_back();
        q.pop_back();
        q.pop_back();

        throws<std::out_of_range>([&q] () {
            q.pop_back();
        });
    });

    suite.add_test("front", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        equals(q.front(), 1);
    });

    suite.add_test("front (empty)", [] () {
        test_deque<int> q;

        throws<std::out_of_range>([&q] () {
            const volatile auto x CCLUNUSED = q.front();
        });
    });

    suite.add_test("back", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        equals(q.back(), 3);
    });

    suite.add_test("back (empty)", [] () {
        test_deque<int> q;

        throws<std::out_of_range>([&q] () {
            const volatile auto x CCLUNUSED = q.back();
        });
    });

    suite.add_test("cfront", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        equals(q.cfront(), 1);
    });

    suite.add_test("cfront (empty)", [] () {
        test_deque<int> q;

        throws<std::out_of_range>([&q] () {
            const volatile auto x CCLUNUSED = q.cfront();
        });
    });

    suite.add_test("cback", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        equals(q.cback(), 3);
    });

    suite.add_test("cback (empty)", [] () {
        test_deque<int> q;

        throws<std::out_of_range>([&q] () {
            const volatile auto x CCLUNUSED = q.cback();
        });
    });

    suite.add_test("reserve", [] () {
        test_deque<int> q;

        q.reserve(100);

        check(q.capacity() >= 100);
        check(q.capacity_back() > 0);
        check(q.capacity_front() == 0);
    });

    suite.add_test("begin", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        equals(*q.begin(), 1);
    });

    suite.add_test("cbegin", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        equals(*q.cbegin(), 1);
    });

    suite.add_test("begin (const)", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        const test_deque<int>& k = q;

        equals(*k.begin(), 1);
    });

    suite.add_test("end", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        equals(*(q.end() - 1), 3);
    });

    suite.add_test("cend", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        equals(*(q.cend() - 1), 3);
    });

    suite.add_test("end (const)", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        const test_deque<int>& k = q;

        equals(*(k.end() - 1), 3);
    });

    suite.add_test("rbegin", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        equals(*q.rbegin(), 3);
    });

    suite.add_test("crbegin", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        equals(*q.crbegin(), 3);
    });

    suite.add_test("rbegin (const)", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        const test_deque<int>& k = q;

        equals(*k.rbegin(), 3);
    });

    suite.add_test("rend", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        equals(q.rend().base(), q.begin());
    });

    suite.add_test("crend", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        equals(q.crend().base(), q.cbegin());
    });

    suite.add_test("rend (const)", [] () {
        test_deque<int> q;

        q.emplace_back(1);
        q.emplace_back(2);
        q.emplace_back(3);

        const test_deque<int>& k = q;

        equals(k.rend().base(), q.cbegin());
    });

    suite.add_test("operator = (copy, empty)", [] () {
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter += 1; };

        test_deque<spy> q;

        {
            test_deque<spy> q2;

            q2.emplace_back(on_destroy);
            q2.emplace_back(on_destroy);

            q = q2;
        }

        differs(q.capacity(), 0);
        equals(q.size(), 2U);
        differs(q.is_empty(), true);
        differs(q.data(), nullptr);

        q.destroy();

        equals(destruction_counter, 4);
    });

    suite.add_test("operator = (copy)", [] () {
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter += 1; };

        test_deque<spy> q;

        q.emplace_back(on_destroy);

        {
            test_deque<spy> q2;

            q2.emplace_back(on_destroy);
            q2.emplace_back(on_destroy);

            q = q2;
        }

        differs(q.capacity(), 0);
        equals(q.size(), 2U);
        differs(q.is_empty(), true);
        differs(q.data(), nullptr);

        q.destroy();

        equals(destruction_counter, 5);
    });

    suite.add_test("operator = (move, empty)", [] () {
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter += 1; };

        test_deque<spy> q;

        {
            test_deque<spy> q2;

            q2.emplace_back(on_destroy);
            q2.emplace_back(on_destroy);

            q = std::move(q2);
        }

        differs(q.capacity(), 0);
        equals(q.size(), 2U);
        differs(q.is_empty(), true);
        differs(q.data(), nullptr);

        q.destroy();

        equals(destruction_counter, 2);
    });

    suite.add_test("operator = (move)", [] () {
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter += 1; };

        test_deque<spy> q;

        q.emplace_back(on_destroy);

        {
            test_deque<spy> q2;

            q2.emplace_back(on_destroy);
            q2.emplace_back(on_destroy);

            q = std::move(q2);
        }

        differs(q.capacity(), 0);
        equals(q.size(), 2U);
        differs(q.is_empty(), true);
        differs(q.data(), nullptr);

        q.destroy();

        equals(destruction_counter, 3);
    });

    return suite.main(argc, argv);
}
