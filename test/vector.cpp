#include <forward_list>
#include <functional>
#include <ccl/features.hpp>
#include <ccl/test/test.hpp>
#include <ccl/vector.hpp>
#include <ccl/test/counting-test-allocator.hpp>

using namespace ccl;

template<typename T>
using test_vector = vector<T, counting_test_allocator>;

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
            test_vector<int> v;

            check(v.capacity() == 0);
            check(v.size() == 0);
            check(v.data() == nullptr);
        }
    );

    suite.add_test(
        "append",
        []() {
            test_vector<int> v;

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
            test_vector<int> v;

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
            test_vector<int> v;

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
            test_vector<int> v;

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
            test_vector<int> v;

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

    suite.add_test("shrink_to_fit", [] () {
        test_vector<int> v;

        v.push_back(10);
        v.push_back(10);
        v.push_back(10);
        v.push_back(10);
        v.push_back(10);
        v.push_back(10);

        v.erase(v.begin() + 2, v.end());
        v.shrink_to_fit();

        equals(v.capacity(), 2);
    });

    suite.add_test("shrink_to_fit (empty)", [] () {
        test_vector<int> v;

        v.push_back(10);
        v.push_back(10);
        v.push_back(10);
        v.push_back(10);
        v.push_back(10);
        v.push_back(10);

        v.erase(v.begin(), v.end());
        v.shrink_to_fit();

        equals(v.capacity(), 0);
        equals(v.size(), 0);
        equals(v.data(), nullptr);
    });

    suite.add_test(
        "insert", []() {
            test_vector<int> v;

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
            test_vector<int> v;

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
        test_vector<spy> v;

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
        test_vector<spy> v;

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
        test_vector<spy> v;

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
        test_vector<spy> v;

        v.append(spy{});
        v.append(spy{});
        v.append(spy{});

        for(auto &x : v) {
            x.on_destroy = [&destruction_counter] () { destruction_counter++; };
        }

        test_vector<spy> v2{v};

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
        test_vector<spy> v;

        v.append(spy{});
        v.append(spy{});
        v.append(spy{});

        for(auto &x : v) {
            x.on_destroy = [&destruction_counter] () { destruction_counter++; };
        }

        test_vector<spy> v2{std::move(v)};

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
            test_vector<spy> v;
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
        test_vector<int> v;

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

    suite.add_test("insert lvalue (invalid iterator)", [] () {
        struct test_struct { int i; };
        test_vector<test_struct> v;
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
        test_vector<int> v;

        throws<std::out_of_range>(
            [&] () {
                v[0];
            }
        );

        throws<std::out_of_range>(
            [&] () {
                (*const_cast<const test_vector<int>*>(&v))[0];
            }
        );
    }, skip_if_exceptions_disabled);

    suite.add_test("resize (0)", [] () {
        test_vector<int> v{1, 2, 3};

        v.resize(0);

        check(v.size() == 0);
    });

    suite.add_test("ctor (initializer list)", []() {
        test_vector<int> v{1, 2, 3};

        check(v[0] == 1);
        check(v[1] == 2);
        check(v[2] == 3);
        check(v.size() == 3);
    });

    suite.add_test("operator = (copy - uninitialized)", [] () {
        test_vector<int> v{1, 2, 3};
        test_vector<int> v2;

        v2 = v;

        check(v2.size() == 3);
        check(v2.capacity() == 4);

        check(v2[0] == 1);
        check(v2[1] == 2);
        check(v2[2] == 3);
    });

    suite.add_test("operator = (copy - initialized)", [] () {
        test_vector<int> v{1, 2, 3};
        test_vector<int> v2{5, 6, 7};

        v2 = v;

        check(v2.size() == 3);
        check(v2.capacity() == 4);

        check(v2[0] == 1);
        check(v2[1] == 2);
        check(v2[2] == 3);
    });

    suite.add_test("operator = (move)", [] () {
        test_vector<int> v{1, 2, 3};
        test_vector<int> v2{5, 6, 7};

        auto * const old_v_data = v.data();

        v2 = std::move(v);

        differs(v.data(), old_v_data);

        equals(v2.size(), 3U);
        equals(v2.capacity(), 4U);
        equals(v2.data(), old_v_data);

        equals(v2[0], 1);
        equals(v2[1], 2);
        equals(v2[2], 3);
    });

    suite.add_test("ctor (range)", [] () {
        std::forward_list<int> my_list {1, 2, 3, 4, 5};
        test_vector<int> v{my_list};

        check(v.size() == 5);
        check(v[0] == 1);
        check(v[1] == 2);
        check(v[2] == 3);
        check(v[3] == 4);
        check(v[4] == 5);
    });

    suite.add_test("insert_range", [] () {
        std::forward_list<int> my_list {1, 2, 3, 4, 5};
        std::forward_list<int> my_list2 {6};
        test_vector<int> v { 123 };

        v.insert_range(
            v.begin(),
            my_list
        );

        v.insert_range(
            v.end(),
            my_list2
        );

        check(v.size() == 7);
        check(v[0] == 1);
        check(v[1] == 2);
        check(v[2] == 3);
        check(v[3] == 4);
        check(v[4] == 5);
        check(v[5] == 123);
        check(v[6] == 6);
    });

    suite.add_test("insert_range (invalid)", [] () {
        std::forward_list<int> my_list {1, 2, 3, 4, 5};
        test_vector<int> v { 123 };

        throws<std::out_of_range>([&] () {
            v.insert_range(
                v.begin() - 1,
                my_list
            );
        });

        throws<std::out_of_range>([&] () {
            v.insert_range(
                v.end() + 1,
                my_list
            );
        });
    }, skip_if_exceptions_disabled);

    suite.add_test("emplace_at", [] () {
        test_vector<dummy> v { dummy{1}, dummy{2}, dummy{3} };
        v.emplace_at(v.begin() + 1, dummy{4});

        check(v[0].value == 2);
        check(v[1].value == 5);
        check(v[2].value == 3);
        check(v[3].value == 4);
        check(v.size() == 4);
    });

    suite.add_test("emplace_at (invalid iterator)", [] () {
        test_vector<dummy> v { dummy{1}, dummy{2}, dummy{3} };

        throws<std::out_of_range>([&] () {
            v.emplace_at(v.begin() - 1, dummy{4});
        });

        throws<std::out_of_range>([&] () {
            v.emplace_at(v.end() + 1, dummy{4});
        });
    }, skip_if_exceptions_disabled);

    suite.add_test("emplace", [] () {
        test_vector<dummy> v { dummy{1}, dummy{2}, dummy{3} };

        v.emplace(v.begin() + 1, dummy{4});

        check(v[0].value == 2);
        check(v[1].value == 5);
        check(v[2].value == 3);
        check(v[3].value == 4);
        check(v.size() == 4);
    });

    suite.add_test("emplace_back", [] () {
        test_vector<dummy> v { dummy{1}, dummy{2}, dummy{3} };

        v.emplace_back(dummy{4});

        check(v[0].value == 2);
        check(v[1].value == 3);
        check(v[2].value == 4);
        check(v[3].value == 5);
        check(v.size() == 4);
    });

    suite.add_test("erase (last)", [] () {
        test_vector<int> v { 1, 2, 3 };

        v.erase(v.begin() + 2, v.end());

        check(v.size() == 2);
        check(v.begin() + 2 == v.end());
        check(v[0] == 1);
        check(v[1] == 2);
    });

    suite.add_test("erase (first)", [] () {
        test_vector<int> v { 1, 2, 3 };

        v.erase(v.begin(), v.end() - 2);

        check(v.size() == 2);
        check(v.begin() + 2 == v.end());
        check(v[0] == 2);
        check(v[1] == 3);
    });

    suite.add_test("erase (middle)", [] () {
        test_vector<int> v { 1, 2, 3 };

        v.erase(v.begin() + 1, v.end() - 1);

        check(v.size() == 2);
        check(v.begin() + 2 == v.end());
        check(v[0] == 1);
        check(v[1] == 3);
    });

    suite.add_test("erase (same)", [] () {
        test_vector<int> v { 1, 2, 3 };

        v.erase(v.begin(), v.begin());

        check(v.size() == 3);
        check(v.begin() + 3 == v.end());
        check(v[0] == 1);
        check(v[1] == 2);
        check(v[2] == 3);
    });

    suite.add_test("erase (all)", [] () {
        test_vector<int> v { 1, 2, 3 };

        v.erase(v.begin(), v.end());

        check(v.size() == 0);
        check(v.begin() == v.end());
    });

    suite.add_test("erase (invalid iterators)", [] () {
        test_vector<int> v { 1, 2, 3 };

        throws<std::out_of_range>([&v] () {
            v.erase(v.begin() - 1, v.end());
        });

        throws<std::out_of_range>([&v] () {
            v.erase(v.begin(), v.end() + 1);
        });
    });

    suite.add_test("reverse iterator", [] () {
        test_vector<int> v { 1, 2, 3 };

        auto it = v.rbegin();
        const auto end = v.rend();

        equals(*it, 3);

        ++it;
        equals(*it, 2);

        ++it;
        equals(*it, 1);

        ++it;
        check(it == end);
    });

    suite.add_test("iterator -> const_iterator", [] () {
        using myvec = test_vector<int>;

        myvec v { 1, 2, 3 };

        myvec::iterator it = v.begin();
        volatile myvec::const_iterator it2 CCLUNUSED = it;
    });

    return suite.main(argc, argv);
}
