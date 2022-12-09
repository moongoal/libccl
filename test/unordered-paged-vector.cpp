#include <forward_list>
#include <functional>
#include <ccl/features.hpp>
#include <ccl/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/unordered-paged-vector.hpp>

using namespace ccl;

template<typename T, typename Ptr = T*>
using test_vector = unordered_paged_vector<T, Ptr, counting_test_allocator>;

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

    spy() : construction_magic{constructed_value} {}
    spy(const auto& on_destroy) : construction_magic{constructed_value}, on_destroy{on_destroy} {}
    spy(const spy&) = default;

    spy(spy&& other) : construction_magic{constructed_value}, on_destroy{other.on_destroy} {
        other.on_destroy = nullptr;
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
        }
    );

    suite.add_test(
        "push_back",
        []() {
            test_vector<int> v;

            v.push_back(1);
            v.push_back(2);
            v.push_back(3);

            check(v[0] == 1);
            check(v[1] == 2);
            check(v[2] == 3);

            check(v.size() == 3);
            check(v.capacity() == test_vector<int>::page_size);
        }
    );

    suite.add_test(
        "reserve (less)", [] () {
            test_vector<int> v;

            v.push_back(1);
            v.push_back(2);
            v.push_back(3);

            v.reserve(1);

            check(v.size() == 3);
            check(v.capacity() == test_vector<int>::page_size);
        }
    );

    suite.add_test(
        "reserve (same, entire page)", [] () {
            test_vector<int> v;

            for(size_t i = 0; i < test_vector<int>::page_size; ++i) {
                v.push_back(1);
            }

            check(v.size() == test_vector<int>::page_size);
            check(v.capacity() == test_vector<int>::page_size);

            v.reserve(v.capacity());

            check(v.size() == test_vector<int>::page_size);
            check(v.capacity() == test_vector<int>::page_size);
        }
    );

    suite.add_test(
        "reserve (more)", [] () {
            test_vector<int> v;

            for(size_t i = 0; i < test_vector<int>::page_size; ++i) {
                v.push_back(1);
            }

            check(v.size() == test_vector<int>::page_size);
            check(v.capacity() == test_vector<int>::page_size);

            v.reserve(v.capacity() + 1);

            check(v.size() == test_vector<int>::page_size);
            check(v.capacity() == test_vector<int>::page_size * 2);
        }
    );

    suite.add_test(
        "clear", []() {
            test_vector<int> v;

            for(size_t i = 0; i < test_vector<int>::page_size + 1; ++i) {
                v.push_back(1);
            }

            v.clear();

            check(v.size() == 0);
            check(v.capacity() == 0);
        }
    );

    suite.add_test("resize (grow within page)", [] () {
        test_vector<spy> v;

        v.push_back(spy{});
        v.push_back(spy{});
        v.push_back(spy{});

        v.resize(4);

        check(v[0].construction_magic == constructed_value);
        check(v[1].construction_magic == constructed_value);
        check(v[2].construction_magic == constructed_value);
        check(v[3].construction_magic == constructed_value);

        check(v.size() == 4);
        check(v.capacity() == test_vector<spy>::page_size);
    });

    suite.add_test("resize (grow outside of page)", [] () {
        test_vector<spy> v;

        v.push_back(spy{});
        v.push_back(spy{});
        v.push_back(spy{});

        const size_t expected_size = test_vector<spy>::page_size * 3 + 5;

        v.resize(expected_size);

        check(v.size() == expected_size);
        check(v.capacity() == test_vector<spy>::page_size * 4);

        for(const spy& s : v) {
            check(s.construction_magic == constructed_value);
        }
    });

    suite.add_test("resize (grow from empty, multiple pages)", [] () {
        test_vector<spy> v;

        const size_t expected_size = test_vector<spy>::page_size * 3 + 5;

        v.resize(expected_size);

        check(v.size() == expected_size);
        check(v.capacity() == test_vector<spy>::page_size * 4);

        for(const spy& s : v) {
            check(s.construction_magic == constructed_value);
        }
    });

    suite.add_test("resize (grow from empty, single page)", [] () {
        test_vector<spy> v;

        const size_t expected_size = 2;

        v.resize(expected_size);

        check(v.size() == expected_size);
        check(v.capacity() == test_vector<spy>::page_size);

        for(const spy& s : v) {
            check(s.construction_magic == constructed_value);
        }
    });

    suite.add_test("resize (grow from empty, first and last page)", [] () {
        test_vector<spy> v;

        const size_t expected_size = test_vector<spy>::page_size * 2;

        v.resize(expected_size);

        check(v.size() == expected_size);
        check(v.capacity() == expected_size);

        for(const spy& s : v) {
            check(s.construction_magic == constructed_value);
        }
    });

    suite.add_test("resize (shrink, within first page)", [] () {
        int destruction_counter = 0;
        test_vector<spy> v;

        v.push_back(spy{});
        v.push_back(spy{});
        v.push_back(spy{});

        for(auto &x : v) {
            x.on_destroy = [&destruction_counter] () { destruction_counter++; };
        }

        v.resize(2);

        check(v.size() == 2);
        check(v.capacity() == test_vector<spy>::page_size);
        check(destruction_counter == 1);
    });

    suite.add_test("resize to 0 (shrink, within first page)", [] () {
        int destruction_counter = 0;
        test_vector<spy> v;

        v.push_back(spy{});
        v.push_back(spy{});
        v.push_back(spy{});

        for(auto &x : v) {
            x.on_destroy = [&destruction_counter] () { destruction_counter++; };
        }

        v.resize(0);

        check(v.size() == 0);
        check(v.capacity() == test_vector<spy>::page_size);
        check(destruction_counter == 3);
    });

    suite.add_test("resize (shrink, multiple full pages only)", [] () {
        const size_t item_count = test_vector<spy>::page_size * 5;
        int destruction_counter = 0;
        test_vector<spy> v;

        for(size_t i = 0; i < item_count; ++i) {
            v.push_back(spy{ [&destruction_counter] () { destruction_counter++; } });
        }

        v.resize(2);

        check(v.size() == 2);
        check(v.capacity() == item_count);
        check(destruction_counter == item_count - 2);
    });

    suite.add_test("resize (shrink, multiple full pages w/partial last page)", [] () {
        const size_t item_count = test_vector<spy>::page_size * 4 + 5;
        int destruction_counter = 0;
        test_vector<spy> v;

        for(size_t i = 0; i < item_count; ++i) {
            v.push_back(spy{ [&destruction_counter] () { destruction_counter++; } });
        }

        v.resize(2);

        check(v.size() == 2);
        check(v.capacity() == test_vector<spy>::page_size * 5);
        check(destruction_counter == item_count - 2);
    });

    suite.add_test("ctor (copy)", [] () {
        int destruction_counter = 0;
        test_vector<spy> v;

        v.push_back(spy{});
        v.push_back(spy{});
        v.push_back(spy{});

        for(auto &x : v) {
            x.on_destroy = [&destruction_counter] () { destruction_counter++; };
        }

        test_vector<spy> v2{v};

        check(destruction_counter == 0);

        check(v.size() == 3);
        check(v.capacity() == test_vector<spy>::page_size);

        check(v2.size() == 3);
        check(v2.capacity() == test_vector<spy>::page_size);

        check(v2[0].construction_magic == constructed_value);
        check(v2[1].construction_magic == constructed_value);
        check(v2[2].construction_magic == constructed_value);

        check(v.get_pages().data() != v2.get_pages().data());
    });

    // suite.add_test("ctor (move)", [] () {
    //     int destruction_counter = 0;
    //     vector<spy> v;

    //     v.push_back(spy{});
    //     v.push_back(spy{});
    //     v.push_back(spy{});

    //     for(auto &x : v) {
    //         x.on_destroy = [&destruction_counter] () { destruction_counter++; };
    //     }

    //     vector v2{std::move(v)};

    //     check(destruction_counter == 0);
    //     check(v.data() == nullptr);

    //     check(v2.size() == 3);
    //     check(v2.capacity() == 4);
    //     check(v2.data() != nullptr);

    //     check(v2[0].construction_magic == constructed_value);
    //     check(v2[1].construction_magic == constructed_value);
    //     check(v2[2].construction_magic == constructed_value);
    // });

    // suite.add_test("dtor", [] () {
    //     int destruction_counter = 0;

    //     {
    //         vector<spy> v;
    //         v.push_back(spy{});
    //         v.push_back(spy{});
    //         v.push_back(spy{});

    //         for(auto &x : v) {
    //             x.on_destroy = [&destruction_counter] () { destruction_counter++; };
    //         }
    //     }

    //     check(destruction_counter == 3);
    // });

    // suite.add_test("insert rvalue (invalid iterator)", [] () {
    //     vector<int> v;

    //     throws<std::out_of_range>(
    //         [&v] () {
    //             v.insert(v.begin() - 1, 0);
    //         }
    //     );

    //     throws<std::out_of_range>(
    //         [&v] () {
    //             v.insert(v.end() + 1, 0);
    //         }
    //     );
    // }, skip_if_exceptions_disabled);

    // suite.add_test("insert rvalue (invalid iterator)", [] () {
    //     struct test_struct { int i; };
    //     vector<test_struct> v;
    //     test_struct x;

    //     throws<std::out_of_range>(
    //         [&] () {
    //             v.insert(v.begin() - 1, x);
    //         }
    //     );

    //     throws<std::out_of_range>(
    //         [&] () {
    //             v.insert(v.end() + 1, x);
    //         }
    //     );
    // }, skip_if_exceptions_disabled);

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

    // suite.add_test("resize (0)", [] () {
    //     vector<int> v{1, 2, 3};

    //     v.resize(0);

    //     check(v.size() == 0);
    // });

    // suite.add_test("ctor (initializer list)", []() {
    //     vector v{1, 2, 3};

    //     check(v[0] == 1);
    //     check(v[1] == 2);
    //     check(v[2] == 3);
    //     check(v.size() == 3);
    // });

    suite.add_test("operator = (copy - uninitialized)", [] () {
        vector v{1, 2, 3};
        test_vector<int> v2;

        v2 = v;

        check(v2.size() == 3);
        check(v2.capacity() == test_vector<int>::page_size);

        check(v2[0] == 1);
        check(v2[1] == 2);
        check(v2[2] == 3);
    });

    suite.add_test("operator = (copy - initialized)", [] () {
        vector v{1, 2, 3};
        test_vector<int> v2;

        v2.push_back(5);
        v2.push_back(18);

        v2 = v;

        check(v2.size() == 3);
        check(v2.capacity() == test_vector<int>::page_size);

        check(v2[0] == 1);
        check(v2[1] == 2);
        check(v2[2] == 3);
    });

    suite.add_test("operator = (same type, copy)", [] () {
        test_vector<int> v;
        test_vector<int> v2{};

        v.push_back(1);
        v.push_back(2);
        v.push_back(3);

        v2.push_back(5);
        v2.push_back(18);

        v2 = v;

        check(v2.size() == 3);
        check(v2.capacity() == test_vector<int>::page_size);

        check(v2[0] == 1);
        check(v2[1] == 2);
        check(v2[2] == 3);
    });

    suite.add_test("operator = (move)", [] () {
        test_vector<int> v;
        test_vector<int> v2;

        v.push_back(1);
        v.push_back(2);
        v.push_back(3);

        v2.push_back(5);
        v2.push_back(6);
        v2.push_back(7);

        const auto old_page_data = v.get_pages().data();

        v2 = std::move(v);

        check(v.get_pages().data() == nullptr);

        check(v2.size() == 3);
        check(v2.capacity() == test_vector<int>::page_size);
        check(v2.get_pages().data() == old_page_data);

        check(v2[0] == 1);
        check(v2[1] == 2);
        check(v2[2] == 3);
    });

    // suite.add_test("ctor (range)", [] () {
    //     std::forward_list<int> my_list {1, 2, 3, 4, 5};
    //     vector<int> v{my_list};

    //     check(v.size() == 5);
    //     check(v[0] == 1);
    //     check(v[1] == 2);
    //     check(v[2] == 3);
    //     check(v[3] == 4);
    //     check(v[4] == 5);
    // });

    // suite.add_test("insert (ranges)", [] () {
    //     std::forward_list<int> my_list {1, 2, 3, 4, 5};
    //     vector<int> v { 123 };

    //     v.insert(
    //         v.begin(),
    //         my_list
    //     );

    //     check(v.size() == 6);
    //     check(v[0] == 1);
    //     check(v[1] == 2);
    //     check(v[2] == 3);
    //     check(v[3] == 4);
    //     check(v[4] == 5);
    //     check(v[5] == 123);
    // });

    // suite.add_test("insert (ranges - invalid)", [] () {
    //     std::forward_list<int> my_list {1, 2, 3, 4, 5};
    //     vector<int> v { 123 };

    //     throws<std::out_of_range>([&] () {
    //         v.insert(
    //             v.begin() - 1,
    //             my_list
    //         );
    //     });

    //     throws<std::out_of_range>([&] () {
    //         v.insert(
    //             v.end() + 1,
    //             my_list
    //         );
    //     });
    // }, skip_if_exceptions_disabled);

    // suite.add_test("emplace_at", [] () {
    //     vector<dummy> v { dummy{1}, dummy{2}, dummy{3} };
    //     v.emplace_at(v.begin() + 1, dummy{4});

    //     check(v[0].value == 2);
    //     check(v[1].value == 5);
    //     check(v[2].value == 3);
    //     check(v[3].value == 4);
    //     check(v.size() == 4);
    // });

    // suite.add_test("emplace_at (invalid iterator)", [] () {
    //     vector<dummy> v { dummy{1}, dummy{2}, dummy{3} };

    //     throws<std::out_of_range>([&] () {
    //         v.emplace_at(v.begin() - 1, dummy{4});
    //     });

    //     throws<std::out_of_range>([&] () {
    //         v.emplace_at(v.end() + 1, dummy{4});
    //     });
    // }, skip_if_exceptions_disabled);

    // suite.add_test("emplace", [] () {
    //     vector<dummy> v { dummy{1}, dummy{2}, dummy{3} };

    //     v.emplace(dummy{4});

    //     check(v[0].value == 2);
    //     check(v[1].value == 3);
    //     check(v[2].value == 4);
    //     check(v[3].value == 5);
    //     check(v.size() == 4);
    // });

    // suite.add_test("erase (last)", [] () {
    //     vector<int> v { 1, 2, 3 };

    //     v.erase(v.begin() + 2, v.end());

    //     check(v.size() == 2);
    //     check(v.begin() + 2 == v.end());
    //     check(v[0] == 1);
    //     check(v[1] == 2);
    // });

    // suite.add_test("erase (first)", [] () {
    //     vector<int> v { 1, 2, 3 };

    //     v.erase(v.begin(), v.end() - 2);

    //     check(v.size() == 2);
    //     check(v.begin() + 2 == v.end());
    //     check(v[0] == 2);
    //     check(v[1] == 3);
    // });

    // suite.add_test("erase (middle)", [] () {
    //     vector<int> v { 1, 2, 3 };

    //     v.erase(v.begin() + 1, v.end() - 1);

    //     check(v.size() == 2);
    //     check(v.begin() + 2 == v.end());
    //     check(v[0] == 1);
    //     check(v[1] == 3);
    // });

    // suite.add_test("erase (same)", [] () {
    //     vector<int> v { 1, 2, 3 };

    //     v.erase(v.begin(), v.begin());

    //     check(v.size() == 3);
    //     check(v.begin() + 3 == v.end());
    //     check(v[0] == 1);
    //     check(v[1] == 2);
    //     check(v[2] == 3);
    // });

    // suite.add_test("erase (all)", [] () {
    //     vector<int> v { 1, 2, 3 };

    //     v.erase(v.begin(), v.end());

    //     check(v.size() == 0);
    //     check(v.begin() == v.end());
    // });

    // suite.add_test("erase (invalid iterators)", [] () {
    //     vector<int> v { 1, 2, 3 };

    //     throws<std::out_of_range>([&v] () {
    //         v.erase(v.begin() - 1, v.end());
    //     });

    //     throws<std::out_of_range>([&v] () {
    //         v.erase(v.begin(), v.end() + 1);
    //     });
    // });

    return suite.main(argc, argv);
}
