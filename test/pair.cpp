#include <ccl/test/test.hpp>
#include <ccl/pair.hpp>

using namespace ccl;

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

    constexpr spy& operator=(spy&& other) {
        on_destroy = other.on_destroy;
        other.on_destroy = nullptr;

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

    suite.add_test("ctor (default)", [] () {
        pair<int, int> p CCLUNUSED;
    });

    suite.add_test("make_pair (rvalues)", [] () {
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter += 1; };

        {
            pair<int, spy> n = make_pair(1, spy{on_destroy});

            equals(n.first, 1);
            equals(n.second.construction_magic, constructed_value);
        }

        equals(destruction_counter, 1);
    });

    suite.add_test("make_pair (lvalues)", [] () {
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter += 1; };

        {
            const int first = 1;
            const spy second = spy{on_destroy};

            pair n = make_pair(first, second);

            equals(n.first, 1);
            equals(n.second.construction_magic, constructed_value);
        }

        equals(destruction_counter, 2);
    });

    suite.add_test("ctor (copy)", [] () {
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter += 1; };

        {
            pair<int, spy> n = make_pair(1, spy{on_destroy});
            pair<int, spy> m = n;

            equals(m.first, 1);
            equals(m.second.construction_magic, constructed_value);
        }

        equals(destruction_counter, 2);
    });

    suite.add_test("ctor (move)", [] () {
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter += 1; };

        {
            pair<int, spy> n = make_pair(1, spy{on_destroy});
            pair<int, spy> m = std::move(n);

            equals(m.first, 1);
            equals(m.second.construction_magic, constructed_value);
        }

        equals(destruction_counter, 1);
    });

    suite.add_test("operator ==, !=", [] () {
        pair<int, int> a { 1, 2 };
        pair<int, int> b { 2, 3 };
        pair<int, int> c { 1, 2 };

        check(a != b);
        check(a == c);
    });

    suite.add_test("operator = (copy)", [] () {
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter += 1; };

        {
            pair<int, spy> n = make_pair(1, spy{on_destroy});
            pair<int, spy> m;

            m = n;

            equals(m.first, 1);
            equals(m.second.construction_magic, constructed_value);
        }

        equals(destruction_counter, 2);
    });

    suite.add_test("operator = (move)", [] () {
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter += 1; };

        {
            pair<int, spy> n = make_pair(1, spy{on_destroy});
            pair<int, spy> m;

            m = std::move(n);

            equals(m.first, 1);
            equals(m.second.construction_magic, constructed_value);
        }

        equals(destruction_counter, 1);
    });

    return suite.main(argc, argv);
}
