#include <string>
#include <memory>
#include <ccl/test/test.hpp>
#include <ccl/hash.hpp>
#include <ccl/compat.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("hash type_index", [] () {
        const auto& x = typeid(int);
        const std::type_index i = x;

        hash<std::type_index>{}(i);
    });

    suite.add_test("hash shared_ptr", [] () {
        auto p = std::make_shared<int>();

        hash<decltype(p)>{}(p);
    });

    suite.add_test("hash unique_ptr", [] () {
        auto p = std::make_unique<int>();

        hash<decltype(p)>{}(p);
    });

    suite.add_test("hash string", [] () {
        std::string s = "hey oh";
        std::string k = "hey doh";
        std::string n = "hey oh";

        using h = hash<std::string>;

        equals(h{}(s), h{}(n));
        differs(h{}(s), h{}(k));
    });

    return suite.main(argc, argv);
}
