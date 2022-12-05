#include <utility>
#include <ccl/test.hpp>
#include <ccl/type-traits.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("pointer_traits<T*>", [] () {
        using traits = pointer_traits<int*>;

        check(std::is_same_v<int, typename traits::element_type>);
        check(std::is_same_v<int*, typename traits::pointer>);
        check(std::is_same_v<const int*, typename traits::const_pointer>);
        check(std::is_same_v<int&, typename traits::reference>);
        check(std::is_same_v<const int&, typename traits::const_reference>);
        check(std::is_same_v<ptrdiff_t, typename traits::difference_type>);
    });

    return suite.main(argc, argv);
}
