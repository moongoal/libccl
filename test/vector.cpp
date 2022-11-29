#include <forward_list>
#include <functional>
#include <ccl/features.hpp>
#include <ccl/test.hpp>
#include <ccl/vector.hpp>

using namespace ccl;

constexpr const auto skip_if_exceptions_disabled = []() {
    #ifdef CCL_FEATURE_EXCEPTIONS
        return false;
    #else // CCL_FEATURE_EXCEPTIONS
        return true;
    #endif // CCL_FEATURE_EXCEPTIONS
};

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
            vector<int> v;

            check(v.capacity() == 0);
            check(v.size() == 0);
            check(v.data() == nullptr);
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

            check(v.size() == 3);
            check(v.capacity() == 4);
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

            check(v.size() == 3);
            check(v.capacity() == 4);
        }
    );

    suite.add_test(
        "reserve (less)", [] () {
            vector<int> v;

            v.prepend(1);
            v.prepend(2);
            v.prepend(3);

            int * const old_data = v.data();

            v.reserve(1);

            check(v.size() == 3);
            check(v.capacity() == 4);
            check(v.data() == old_data);
        }
    );

    suite.add_test(
        "reserve (same)", [] () {
            vector<int> v;

            v.prepend(1);
            v.prepend(2);
            v.prepend(3);

            int * const old_data = v.data();

            v.reserve(v.capacity());

            check(v.size() == 3);
            check(v.capacity() == 4);
            check(v.data() == old_data);
        }
    );

    suite.add_test(
        "reserve (more)", [] () {
            vector<int> v;

            v.prepend(1);
            v.prepend(2);
            v.prepend(3);

            int * const old_data = v.data();

            v.reserve(5);

            check(v.size() == 3);
            check(v.capacity() == 8);
            check(v.data() != old_data);

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

            int * const old_data = v.data();

            v.clear();

            check(v.size() == 0);
            check(v.capacity() == 4);
            check(old_data == v.data());
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

        check(v.size() == 4);
        check(v.capacity() == 4);
    });

    suite.add_test("resize (grow from empty)", [] () {
        vector<spy> v;

        v.resize(4);

        check(v[0].construction_magic == constructed_value);
        check(v[1].construction_magic == constructed_value);
        check(v[2].construction_magic == constructed_value);
        check(v[3].construction_magic == constructed_value);

        check(v.size() == 4);
        check(v.capacity() == 4);
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

        check(v.size() == 2);
        check(v.capacity() == 4);
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

        check(v.size() == 3);
        check(v.capacity() == 4);

        check(v2.size() == 3);
        check(v2.capacity() == 4);

        check(v.data() != v2.data());

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
        check(v.data() == nullptr);

        check(v2.size() == 3);
        check(v2.capacity() == 4);
        check(v2.data() != nullptr);

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
    }, skip_if_exceptions_disabled);

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
    }, skip_if_exceptions_disabled);

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
    }, skip_if_exceptions_disabled);

    suite.add_test("resize (0)", [] () {
        vector<int> v{1, 2, 3};

        v.resize(0);

        check(v.size() == 0);
    });

    suite.add_test("ctor (initializer list)", []() {
        vector v{1, 2, 3};

        check(v[0] == 1);
        check(v[1] == 2);
        check(v[2] == 3);
        check(v.size() == 3);
    });

    suite.add_test("operator = (copy - uninitialized)", [] () {
        vector v{1, 2, 3};
        vector<int> v2;

        v2 = v;

        check(v2.size() == 3);
        check(v2.capacity() == 4);

        check(v2[0] == 1);
        check(v2[1] == 2);
        check(v2[2] == 3);
    });

    suite.add_test("operator = (copy - initialized)", [] () {
        vector v{1, 2, 3};
        vector<int> v2{5, 6, 7};

        v2 = v;

        check(v2.size() == 3);
        check(v2.capacity() == 4);

        check(v2[0] == 1);
        check(v2[1] == 2);
        check(v2[2] == 3);
    });

    suite.add_test("operator = (move)", [] () {
        vector v{1, 2, 3};
        vector<int> v2{5, 6, 7};

        v2 = std::move(v);

        check(v.data() == nullptr);

        check(v2.size() == 3);
        check(v2.capacity() == 4);
        check(v2.data() != nullptr);

        check(v2[0] == 1);
        check(v2[1] == 2);
        check(v2[2] == 3);
    });

    suite.add_test("ctor (range)", [] () {
        std::forward_list<int> my_list {1, 2, 3, 4, 5};
        vector<int> v{my_list};

        check(v.size() == 5);
        check(v[0] == 1);
        check(v[1] == 2);
        check(v[2] == 3);
        check(v[3] == 4);
        check(v[4] == 5);
    });

    suite.add_test("insert (ranges)", [] () {
        std::forward_list<int> my_list {1, 2, 3, 4, 5};
        vector<int> v { 123 };

        v.insert(
            v.begin(),
            my_list
        );

        check(v.size() == 6);
        check(v[0] == 1);
        check(v[1] == 2);
        check(v[2] == 3);
        check(v[3] == 4);
        check(v[4] == 5);
        check(v[5] == 123);
    });

    suite.add_test("insert (ranges - invalid)", [] () {
        std::forward_list<int> my_list {1, 2, 3, 4, 5};
        vector<int> v { 123 };

        throws<std::out_of_range>([&] () {
            v.insert(
                v.begin() - 1,
                my_list
            );
        });

        throws<std::out_of_range>([&] () {
            v.insert(
                v.end() + 1,
                my_list
            );
        });
    }, skip_if_exceptions_disabled);

    suite.add_test("emplace_at", [] () {
        vector<dummy> v { dummy{1}, dummy{2}, dummy{3} };
        v.emplace_at(v.begin() + 1, dummy{4});

        check(v[0].value == 2);
        check(v[1].value == 5);
        check(v[2].value == 3);
        check(v[3].value == 4);
        check(v.size() == 4);
    });

    suite.add_test("emplace_at (invalid iterator)", [] () {
        vector<dummy> v { dummy{1}, dummy{2}, dummy{3} };

        throws<std::out_of_range>([&] () {
            v.emplace_at(v.begin() - 1, dummy{4});
        });

        throws<std::out_of_range>([&] () {
            v.emplace_at(v.end() + 1, dummy{4});
        });
    }, skip_if_exceptions_disabled);

    suite.add_test("emplace", [] () {
        vector<dummy> v { dummy{1}, dummy{2}, dummy{3} };

        v.emplace(dummy{4});

        check(v[0].value == 2);
        check(v[1].value == 3);
        check(v[2].value == 4);
        check(v[3].value == 5);
        check(v.size() == 4);
    });

    return suite.main(argc, argv);
}
