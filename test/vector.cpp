#include <forward_list>
#include <functional>
#include <charcoal/test.hpp>
#include <charcoal/vector.hpp>

using namespace ccl;

constexpr uint32_t constructed_value = 0x1234;

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

        check(v[0].construction_magic == constructed_value);
        check(v[1].construction_magic == constructed_value);
        check(v[2].construction_magic == constructed_value);
        check(v[3].construction_magic == constructed_value);

        check(v.get_length() == 4);
        check(v.get_capacity() == 4);
    });

    suite.add_test("resize (grow from empty)", [] () {
        vector<spy> v;

        v.resize(4);

        check(v[0].construction_magic == constructed_value);
        check(v[1].construction_magic == constructed_value);
        check(v[2].construction_magic == constructed_value);
        check(v[3].construction_magic == constructed_value);

        check(v.get_length() == 4);
        check(v.get_capacity() == 4);
    });

    suite.add_test("resize (shrink)", [] () {
        int destruction_counter = 0;
        vector<spy> v;

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

    suite.add_test("ctor (copy)", [] () {
        int destruction_counter = 0;
        vector<spy> v;

        v.append(spy{});
        v.append(spy{});
        v.append(spy{});

        for(auto &x : v) {
            x.on_destroy = [&destruction_counter] () { destruction_counter++; };
        }

        vector v2{v};

        check(destruction_counter == 0);

        check(v.get_length() == 3);
        check(v.get_capacity() == 4);

        check(v2.get_length() == 3);
        check(v2.get_capacity() == 4);

        check(v.get_data() != v2.get_data());

        check(v2[0].construction_magic == constructed_value);
        check(v2[1].construction_magic == constructed_value);
        check(v2[2].construction_magic == constructed_value);
    });

    suite.add_test("ctor (move)", [] () {
        int destruction_counter = 0;
        vector<spy> v;

        v.append(spy{});
        v.append(spy{});
        v.append(spy{});

        for(auto &x : v) {
            x.on_destroy = [&destruction_counter] () { destruction_counter++; };
        }

        vector v2{std::move(v)};

        check(destruction_counter == 0);
        check(v.get_data() == nullptr);

        check(v2.get_length() == 3);
        check(v2.get_capacity() == 4);
        check(v2.get_data() != nullptr);

        check(v2[0].construction_magic == constructed_value);
        check(v2[1].construction_magic == constructed_value);
        check(v2[2].construction_magic == constructed_value);
    });

    suite.add_test("dtor", [] () {
        int destruction_counter = 0;

        {
            vector<spy> v;
            v.append(spy{});
            v.append(spy{});
            v.append(spy{});

            for(auto &x : v) {
                x.on_destroy = [&destruction_counter] () { destruction_counter++; };
            }
        }

        check(destruction_counter == 3);
    });

    suite.add_test("insert rvalue (invalid iterator)", [] () {
        vector<int> v;

        throws<std::out_of_range>(
            [&v] () {
                v.insert(v.begin() - 1, 0);
            }
        );

        throws<std::out_of_range>(
            [&v] () {
                v.insert(v.end() + 1, 0);
            }
        );
    });

    suite.add_test("insert rvalue (invalid iterator)", [] () {
        struct test_struct { int i; };
        vector<test_struct> v;
        test_struct x;

        throws<std::out_of_range>(
            [&] () {
                v.insert(v.begin() - 1, x);
            }
        );

        throws<std::out_of_range>(
            [&] () {
                v.insert(v.end() + 1, x);
            }
        );
    });

    suite.add_test("operator [] (invalid index)", []() {
        vector<int> v;

        throws<std::out_of_range>(
            [&] () {
                v[0];
            }
        );

        throws<std::out_of_range>(
            [&] () {
                (*const_cast<const vector<int>*>(&v))[0];
            }
        );
    });

    suite.add_test("resize (0)", [] () {
        vector<int> v{1, 2, 3};

        v.resize(0);

        check(v.get_length() == 0);
    });

    suite.add_test("ctor (initializer list)", []() {
        vector v{1, 2, 3};

        check(v[0] == 1);
        check(v[1] == 2);
        check(v[2] == 3);
        check(v.get_length() == 3);
    });

    suite.add_test("operator = (copy - uninitialized)", [] () {
        vector v{1, 2, 3};
        vector<int> v2;

        v2 = v;

        check(v2.get_length() == 3);
        check(v2.get_capacity() == 4);

        check(v2[0] == 1);
        check(v2[1] == 2);
        check(v2[2] == 3);
    });

    suite.add_test("operator = (copy - initialized)", [] () {
        vector v{1, 2, 3};
        vector<int> v2{5, 6, 7};

        v2 = v;

        check(v2.get_length() == 3);
        check(v2.get_capacity() == 4);

        check(v2[0] == 1);
        check(v2[1] == 2);
        check(v2[2] == 3);
    });

    suite.add_test("operator = (move)", [] () {
        vector v{1, 2, 3};
        vector<int> v2{5, 6, 7};

        v2 = std::move(v);

        check(v.get_data() == nullptr);

        check(v2.get_length() == 3);
        check(v2.get_capacity() == 4);
        check(v2.get_data() != nullptr);

        check(v2[0] == 1);
        check(v2[1] == 2);
        check(v2[2] == 3);
    });

    suite.add_test("ctor (iterators)", [] () {
        std::forward_list<int> my_list {1, 2, 3, 4, 5};
        vector<int> v{my_list.begin(), my_list.end()};

        check(v.get_length() == 5);
        check(v[0] == 1);
        check(v[1] == 2);
        check(v[2] == 3);
        check(v[3] == 4);
        check(v[4] == 5);
    });

    return suite.main(argc, argv);
}
