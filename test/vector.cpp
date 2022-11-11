#include <functional>
#include <charcoal/test.hpp>
#include <charcoal/vector.hpp>

using namespace ccl;

constexpr uint32_t constructed_value = 0x1234;

struct spy {
    uint32_t constructed;
    std::function<void()> on_destroy;

    spy() {
        constructed = constructed_value;
    }

    ~spy() {
        if(on_destroy) {
            on_destroy();
        }
    }
};

int main(int argc, char **argv) {
    suite.add_test(
        "ctor",
        []() {
            vector<int> v;

            check(v.get_capacity() == 0);
            check(v.get_length() == 0);
            check(v.get_data() == nullptr);
        }
    );

    suite.add_test(
        "append",
        []() {
            vector<int> v;

            v.append(1);
            v.append(2);
            v.append(3);

            check(v[0] == 1);
            check(v[1] == 2);
            check(v[2] == 3);

            check(v.get_length() == 3);
            check(v.get_capacity() == 4);
        }
    );

    suite.add_test(
        "prepend",
        []() {
            vector<int> v;

            v.prepend(1);
            v.prepend(2);
            v.prepend(3);

            check(v[0] == 3);
            check(v[1] == 2);
            check(v[2] == 1);

            check(v.get_length() == 3);
            check(v.get_capacity() == 4);
        }
    );

    suite.add_test(
        "reserve_less", [] () {
            vector<int> v;

            v.prepend(1);
            v.prepend(2);
            v.prepend(3);

            int * const old_data = v.get_data();

            v.reserve(1);

            check(v.get_length() == 3);
            check(v.get_capacity() == 4);
            check(v.get_data() == old_data);
        }
    );

    suite.add_test(
        "reserve_same", [] () {
            vector<int> v;

            v.prepend(1);
            v.prepend(2);
            v.prepend(3);

            int * const old_data = v.get_data();

            v.reserve(v.get_capacity());

            check(v.get_length() == 3);
            check(v.get_capacity() == 4);
            check(v.get_data() == old_data);
        }
    );

    suite.add_test(
        "reserve_more", [] () {
            vector<int> v;

            v.prepend(1);
            v.prepend(2);
            v.prepend(3);

            int * const old_data = v.get_data();

            v.reserve(5);

            check(v.get_length() == 3);
            check(v.get_capacity() == 8);
            check(v.get_data() != old_data);

            check(v[0] == 3);
            check(v[1] == 2);
            check(v[2] == 1);
        }
    );

    suite.add_test(
        "insert", []() {
            vector<int> v;

            v.insert(v.begin(), 1);
            v.insert(v.end(), 2);
            v.insert(v.begin() + 1, 3);

            check(v[0] == 1);
            check(v[1] == 3);
            check(v[2] == 2);
        }
    );

    suite.add_test(
        "clear", []() {
            vector<int> v;

            v.append(1);
            v.append(2);
            v.append(3);

            int * const old_data = v.get_data();

            v.clear();

            check(v.get_length() == 0);
            check(v.get_capacity() == 4);
            check(old_data == v.get_data());
        }
    );

    suite.add_test("resize (grow)", [] () {
        vector<spy> v;

        v.append(spy{});
        v.append(spy{});
        v.append(spy{});

        v.resize(4);

        check(v[0].constructed == constructed_value);
        check(v[1].constructed == constructed_value);
        check(v[2].constructed == constructed_value);
        check(v[3].constructed == constructed_value);

        check(v.get_length() == 4);
        check(v.get_capacity() == 4);
    });

    suite.add_test("resize (grow from empty)", [] () {
        vector<spy> v;

        v.resize(4);

        check(v[0].constructed == constructed_value);
        check(v[1].constructed == constructed_value);
        check(v[2].constructed == constructed_value);
        check(v[3].constructed == constructed_value);

        check(v.get_length() == 4);
        check(v.get_capacity() == 4);
    });

    suite.add_test("resize (shrink)", [] () {
        vector<spy> v;
        int destruction_counter = 0;

        v.append(spy{});
        v.append(spy{});
        v.append(spy{});

        for(auto &x : v) {
            x.on_destroy = [&destruction_counter] () { destruction_counter++; };
        }

        v.resize(2);

        check(v.get_length() == 2);
        check(v.get_capacity() == 4);
        check(destruction_counter == 1);
    });

    return suite.main(argc, argv);
}
