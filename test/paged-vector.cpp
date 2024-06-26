#include <forward_list>
#include <functional>
#include <ccl/features.hpp>
#include <ccl/test/test.hpp>
#include <ccl/test/counting-test-allocator.hpp>
#include <ccl/paged-vector.hpp>

using namespace ccl;

template<typename T, typename Ptr = T*>
using test_ring = paged_vector<T, Ptr, counting_test_allocator>;

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
            test_ring<int> v;

            check(v.capacity() == 0);
            check(v.size() == 0);
        }
    );

    suite.add_test(
        "push_back",
        []() {
            test_ring<int> v;

            v.push_back(1);
            v.push_back(2);
            v.push_back(3);

            check(v[0] == 1);
            check(v[1] == 2);
            check(v[2] == 3);

            check(v.size() == 3);
            check(v.capacity() == test_ring<int>::page_size);
        }
    );

    suite.add_test(
        "reserve (less)", [] () {
            test_ring<int> v;

            v.push_back(1);
            v.push_back(2);
            v.push_back(3);

            v.reserve(1);

            check(v.size() == 3);
            check(v.capacity() == test_ring<int>::page_size);
        }
    );

    suite.add_test(
        "reserve (same, entire page)", [] () {
            test_ring<int> v;

            for(std::size_t i = 0; i < test_ring<int>::page_size; ++i) {
                v.push_back(1);
            }

            check(v.size() == test_ring<int>::page_size);
            check(v.capacity() == test_ring<int>::page_size);

            v.reserve(v.capacity());

            check(v.size() == test_ring<int>::page_size);
            check(v.capacity() == test_ring<int>::page_size);
        }
    );

    suite.add_test(
        "reserve (more)", [] () {
            test_ring<int> v;

            for(std::size_t i = 0; i < test_ring<int>::page_size; ++i) {
                v.push_back(1);
            }

            check(v.size() == test_ring<int>::page_size);
            check(v.capacity() == test_ring<int>::page_size);

            v.reserve(v.capacity() + 1);

            check(v.size() == test_ring<int>::page_size);
            check(v.capacity() == test_ring<int>::page_size * 2);
        }
    );

    suite.add_test(
        "clear", []() {
            test_ring<int> v;

            for(std::size_t i = 0; i < test_ring<int>::page_size + 1; ++i) {
                v.push_back(1);
            }

            v.clear();

            check(v.size() == 0);
            check(v.capacity() == test_ring<int>::page_size * 2);
        }
    );

    suite.add_test("clear twice", []() {
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter++; };
        test_ring<spy> v;

        for(std::size_t i = 0; i < test_ring<int>::page_size + 1; ++i) {
            v.push_back(spy { on_destroy });
        }

        v.clear();
        v.clear();

        check(v.size() == 0);
        check(v.capacity() == test_ring<int>::page_size * 2);
        equals(destruction_counter, test_ring<int>::page_size + 1);
    });

    suite.add_test("resize (grow within page)", [] () {
        test_ring<spy> v;

        v.push_back(spy{});
        v.push_back(spy{});
        v.push_back(spy{});

        v.resize(4);

        check(v[0].construction_magic == constructed_value);
        check(v[1].construction_magic == constructed_value);
        check(v[2].construction_magic == constructed_value);
        check(v[3].construction_magic == constructed_value);

        check(v.size() == 4);
        check(v.capacity() == test_ring<spy>::page_size);
    });

    suite.add_test("resize (grow outside of page)", [] () {
        test_ring<spy> v;

        v.push_back(spy{});
        v.push_back(spy{});
        v.push_back(spy{});

        const std::size_t expected_size = test_ring<spy>::page_size * 3 + 5;

        v.resize(expected_size);

        check(v.size() == expected_size);
        check(v.capacity() == test_ring<spy>::page_size * 4);

        for(const spy& s : v) {
            check(s.construction_magic == constructed_value);
        }
    });

    suite.add_test("resize (grow from empty, multiple pages)", [] () {
        test_ring<spy> v;

        const std::size_t expected_size = test_ring<spy>::page_size * 3 + 5;

        v.resize(expected_size);

        check(v.size() == expected_size);
        check(v.capacity() == test_ring<spy>::page_size * 4);

        for(const spy& s : v) {
            check(s.construction_magic == constructed_value);
        }
    });

    suite.add_test("resize (grow from empty, single page)", [] () {
        test_ring<spy> v;

        const std::size_t expected_size = 2;

        v.resize(expected_size);

        check(v.size() == expected_size);
        check(v.capacity() == test_ring<spy>::page_size);

        for(const spy& s : v) {
            check(s.construction_magic == constructed_value);
        }
    });

    suite.add_test("resize (grow from empty, first and last page)", [] () {
        test_ring<spy> v;

        const std::size_t expected_size = test_ring<spy>::page_size * 2;

        v.resize(expected_size);

        check(v.size() == expected_size);
        check(v.capacity() == expected_size);

        for(const spy& s : v) {
            check(s.construction_magic == constructed_value);
        }
    });

    suite.add_test("resize (shrink, within first page)", [] () {
        int destruction_counter = 0;
        test_ring<spy> v;

        v.push_back(spy{});
        v.push_back(spy{});
        v.push_back(spy{});

        for(auto &x : v) {
            x.on_destroy = [&destruction_counter] () { destruction_counter++; };
        }

        v.resize(2);

        check(v.size() == 2);
        check(v.capacity() == test_ring<spy>::page_size);
        check(destruction_counter == 1);
    });

    suite.add_test("resize to 0 (shrink, within first page)", [] () {
        int destruction_counter = 0;
        test_ring<spy> v;

        v.push_back(spy{});
        v.push_back(spy{});
        v.push_back(spy{});

        for(auto &x : v) {
            x.on_destroy = [&destruction_counter] () { destruction_counter++; };
        }

        v.resize(0);

        check(v.size() == 0);
        check(v.capacity() == test_ring<spy>::page_size);
        check(destruction_counter == 3);
    });

    suite.add_test("resize (shrink, multiple full pages only)", [] () {
        const std::size_t item_count = test_ring<spy>::page_size * 5;
        int destruction_counter = 0;
        test_ring<spy> v;

        for(std::size_t i = 0; i < item_count; ++i) {
            v.push_back(spy{ [&destruction_counter] () { destruction_counter++; } });
        }

        v.resize(2);

        check(v.size() == 2);
        check(v.capacity() == item_count);
        check(destruction_counter == item_count - 2);
    });

    suite.add_test("resize (shrink, multiple full pages w/partial last page)", [] () {
        const std::size_t item_count = test_ring<spy>::page_size * 4 + 5;
        int destruction_counter = 0;
        test_ring<spy> v;

        for(std::size_t i = 0; i < item_count; ++i) {
            v.push_back(spy{ [&destruction_counter] () { destruction_counter++; } });
        }

        v.resize(2);

        check(v.size() == 2);
        check(v.capacity() == test_ring<spy>::page_size * 5);
        check(destruction_counter == item_count - 2);
    });

    suite.add_test("ctor (copy)", [] () {
        int destruction_counter = 0;
        test_ring<spy> v;

        v.push_back(spy{});
        v.push_back(spy{});
        v.push_back(spy{});

        for(auto &x : v) {
            x.on_destroy = [&destruction_counter] () { destruction_counter++; };
        }

        test_ring<spy> v2{v};

        check(destruction_counter == 0);

        check(v.size() == 3);
        check(v.capacity() == test_ring<spy>::page_size);

        check(v2.size() == 3);
        check(v2.capacity() == test_ring<spy>::page_size);

        check(v2[0].construction_magic == constructed_value);
        check(v2[1].construction_magic == constructed_value);
        check(v2[2].construction_magic == constructed_value);

        check(v.pages().data() != v2.pages().data());
    });

    suite.add_test("ctor (move)", [] () {
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter++; };
        test_ring<spy> v;

        v.push_back(spy{on_destroy});
        v.push_back(spy{on_destroy});
        v.push_back(spy{on_destroy});

        test_ring<spy> v2{std::move(v)};

        check(destruction_counter == 0);
        check(v.pages().data() == nullptr);

        check(v2.size() == 3);
        check(v2.capacity() == test_ring<spy>::page_size);
        check(v2.pages().data() != nullptr);

        check(v2[0].construction_magic == constructed_value);
        check(v2[1].construction_magic == constructed_value);
        check(v2[2].construction_magic == constructed_value);
    });

    suite.add_test("ctor (move, empty)", [] () {
        test_ring<spy> v;
        test_ring<spy> v2{std::move(v)};

        check(v2.size() == 0);
        check(v2.capacity() == 0);
        check(v2.pages().data() == nullptr);
    });

    suite.add_test("dtor", [] () {
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter++; };

        {
            test_ring<spy> v;
            v.push_back(spy{on_destroy});
            v.push_back(spy{on_destroy});
            v.push_back(spy{on_destroy});
        }

        check(destruction_counter == 3);
    });

    suite.add_test(
        "insert", []() {
            test_ring<int> v;

            v.insert(v.begin(), 1);
            v.insert(v.end(), 2);
            v.insert(v.begin() + 1, 3);

            check(v[0] == 1);
            check(v[1] == 3);
            check(v[2] == 2);
        }
    );

    suite.add_test(
        "insert (multi pages)", []() {
            test_ring<int> v;

            const std::size_t initial_item_count = test_ring<int>::page_size - 2;

            for(std::size_t i = 0; i < initial_item_count; ++i) {
                v.push_back(666);
            }

            v.insert(v.begin(), 1);
            v.insert(v.end(), 2);
            v.insert(v.begin() + 1, 3);

            check(v[0] == 1);
            check(v[1] == 3);
            check(v[v.size() - 1] == 2);
            check(v.size() == initial_item_count + 3);

            for(std::size_t i = 2; i < initial_item_count + 2; ++i) {
                equals(v[i], 666);
            }
        }
    );

    suite.add_test("insert rvalue", [] () {
        int destruction_counter = 0;
        const auto on_destroy = [&destruction_counter] () { destruction_counter++; };

        {
            test_ring<spy> v;

            for(std::size_t i = 0; i < test_ring<spy>::page_size * 2; ++i) {
                v.insert(v.end(), spy{ on_destroy });
            }

            equals(destruction_counter, 0);
        }

        equals(destruction_counter, test_ring<spy>::page_size * 2);
    });

    suite.add_test("insert rvalue (invalid iterator)", [] () {
        test_ring<int> v;

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
        test_ring<int> v;

        throws<std::out_of_range>(
            [&] () {
                v[0];
            }
        );

        throws<std::out_of_range>(
            [&] () {
                (*const_cast<const test_ring<int>*>(&v))[0];
            }
        );
    }, skip_if_exceptions_disabled);

    suite.add_test("operator = (copy - uninitialized)", [] () {
        vector v{1, 2, 3};
        test_ring<int> v2;

        v2 = v;

        check(v2.size() == 3);
        check(v2.capacity() == test_ring<int>::page_size);

        check(v2[0] == 1);
        check(v2[1] == 2);
        check(v2[2] == 3);
    });

    suite.add_test("operator = (copy - initialized)", [] () {
        vector v{1, 2, 3};
        test_ring<int> v2;

        v2.push_back(5);
        v2.push_back(18);

        v2 = v;

        check(v2.size() == 3);
        check(v2.capacity() == test_ring<int>::page_size);

        check(v2[0] == 1);
        check(v2[1] == 2);
        check(v2[2] == 3);
    });

    suite.add_test("operator = (same type, copy)", [] () {
        test_ring<int> v;
        test_ring<int> v2{};

        v.push_back(1);
        v.push_back(2);
        v.push_back(3);

        v2.push_back(5);
        v2.push_back(18);

        v2 = v;

        check(v2.size() == 3);
        check(v2.capacity() == test_ring<int>::page_size);

        check(v2[0] == 1);
        check(v2[1] == 2);
        check(v2[2] == 3);
    });

    suite.add_test("operator = (move)", [] () {
        test_ring<int> v;
        test_ring<int> v2;

        v.push_back(1);
        v.push_back(2);
        v.push_back(3);

        v2.push_back(5);
        v2.push_back(6);
        v2.push_back(7);

        const auto old_page_data = v.pages().data();

        v2 = std::move(v);

        check(v.pages().data() == nullptr);

        check(v2.size() == 3);
        check(v2.capacity() == test_ring<int>::page_size);
        check(v2.pages().data() == old_page_data);

        check(v2[0] == 1);
        check(v2[1] == 2);
        check(v2[2] == 3);
    });

    suite.add_test("ctor (initializer list)", []() {
        test_ring<int> v{1, 2, 3};

        check(v[0] == 1);
        check(v[1] == 2);
        check(v[2] == 3);
        check(v.size() == 3);
    });

    suite.add_test("ctor (range)", [] () {
        std::forward_list<int> my_list {1, 2, 3, 4, 5};
        test_ring<int> v{my_list};

        check(v.size() == 5);
        check(v[0] == 1);
        check(v[1] == 2);
        check(v[2] == 3);
        check(v[3] == 4);
        check(v[4] == 5);
    });

    suite.add_test("insert (ranges)", [] () {
        std::forward_list<int> my_list {1, 2, 3, 4, 5};
        std::forward_list<int> my_list2 {6};
        test_ring<int> v;

        v.insert(
            v.begin(),
            my_list
        );

        v.insert(
            v.end(),
            my_list2
        );

        check(v.size() == 6);
        check(v[0] == 1);
        check(v[1] == 2);
        check(v[2] == 3);
        check(v[3] == 4);
        check(v[4] == 5);
        check(v[5] == 6);
    });

    suite.add_test("insert (ranges - invalid)", [] () {
        std::forward_list<int> my_list {1, 2, 3, 4, 5};
        test_ring<int> v;

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
        test_ring<dummy> v { dummy{1}, dummy{2}, dummy{3} };
        v.emplace_at(v.begin() + 1, dummy{4});

        check(v[0].value == 2);
        check(v[1].value == 5);
        check(v[2].value == 3);
        check(v[3].value == 4);
        check(v.size() == 4);
    });

    suite.add_test("emplace_at (full page)", [] () {
        test_ring<dummy> v;
        const std::size_t item_count = test_ring<dummy>::page_size;

        for(std::size_t i = 0; i < item_count; ++i) {
            v.emplace_at(v.end(), dummy{998});
        }

        v.emplace_at(v.begin() + 1, dummy{4});

        equals(v.size(), item_count + 1);
        equals(v.capacity(), test_ring<dummy>::page_size * 2);

        equals(v[v.begin()].value, 999);
        equals(v[v.begin() + 1].value, 5);

        for(std::size_t i = 0; i < item_count - 2; ++i) {
            equals(v[v.begin() + 2 + i].value, 999);
        }
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

    suite.add_test("emplace_back", [] () {
        test_ring<dummy> v;

        v.push_back(dummy{1});
        v.push_back(dummy{2});
        v.push_back(dummy{3});

        v.emplace_back(dummy{4});

        check(v[0].value == 2);
        check(v[1].value == 3);
        check(v[2].value == 4);
        check(v[3].value == 5);
        check(v.size() == 4);
    });

    suite.add_test("emplace_back page full", [] () {
        test_ring<dummy> v;

        for(std::size_t i = 0; i < test_ring<dummy>::page_size; ++i) {
            v.push_back(dummy{1});
        }

        v.emplace_back(dummy{4});

        equals(v.size(), test_ring<dummy>::page_size + 1);
        equals(v.capacity(), test_ring<dummy>::page_size * 2);
        equals((v.end() - 1)->value, 5);
    });

    suite.add_test("erase (last)", [] () {
        test_ring<int> v { 1, 2, 3 };

        v.erase(v.begin() + 2, v.end());

        check(v.size() == 2);
        check(v.begin() + 2 == v.end());
        check(v[0] == 1);
        check(v[1] == 2);
    });

    suite.add_test("erase (first)", [] () {
        test_ring<int> v { 1, 2, 3 };

        v.erase(v.begin(), v.end() - 2);

        check(v.size() == 2);
        check(v.begin() + 2 == v.end());
        check(v[0] == 2);
        check(v[1] == 3);
    });

    suite.add_test("erase (middle)", [] () {
        test_ring<int> v { 1, 2, 3 };

        v.erase(v.begin() + 1, v.end() - 1);

        check(v.size() == 2);
        check(v.begin() + 2 == v.end());
        check(v[0] == 1);
        check(v[1] == 3);
    });

    suite.add_test("erase (same)", [] () {
        test_ring<int> v { 1, 2, 3 };

        v.erase(v.begin(), v.begin());

        check(v.size() == 3);
        check(v.begin() + 3 == v.end());
        check(v[0] == 1);
        check(v[1] == 2);
        check(v[2] == 3);
    });

    suite.add_test("erase (all)", [] () {
        test_ring<int> v { 1, 2, 3 };

        v.erase(v.begin(), v.end());

        check(v.size() == 0);
        check(v.begin() == v.end());
    });

    suite.add_test("erase (invalid iterators)", [] () {
        test_ring<int> v { 1, 2, 3 };

        throws<std::out_of_range>([&v] () {
            v.erase(v.begin() - 1, v.end());
        });

        throws<std::out_of_range>([&v] () {
            v.erase(v.begin(), v.end() + 1);
        });
    });

    return suite.main(argc, argv);
}
