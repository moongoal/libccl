/**
 * @file
 *
 * Test driver.
 */
#ifndef CCL_TEST_HPP
#define CCL_TEST_HPP

#include <functional>
#include <vector>
#include <string>
#include <exception>
#include <iostream>
#include <memory>
#include <ccl/api.hpp>

namespace ccl {
    using test_function = std::function<void()>;
    using skip_predicate = std::function<bool()>;

    constexpr const auto default_skip_predicate = [] () -> bool {
        return false;
    };

    class test_failed_exception : public std::exception {};

    class test {
        friend class test_suite;

        std::string name;
        test_function test_func;
        bool skip;

        void execute() const { test_func(); }

        public:
            test(
                const std::string_view name,
                const test_function test_func,
                const skip_predicate skip_if
            ) : name{name}, test_func{test_func}, skip{skip_if()}
            {}

            const std::string& get_name() const { return name; }
            constexpr bool should_skip() const { return skip; }
    };

    class test_suite {
        public:
            using test_ptr = std::shared_ptr<test>;

        private:
            std::vector<test_ptr> tests;
            std::ostream *ostream;

        public:
            explicit test_suite(std::ostream * const ostream = &std::cout) : ostream{ostream} {
                tests.reserve(32);
            }

            bool execute() const {
                bool all_success = true;
                int passed = 0, failed = 0, skipped = 0;

                for(const auto &t : tests) {
                    std::string state_tag = "[ PASS ]";

                    if(!t->should_skip()) {
                        try {
                            t->execute();
                            passed += 1;
                        } catch (test_failed_exception& exc) {
                            state_tag = "[*FAIL*]";
                            all_success = false;
                            failed += 1;
                        }
                    } else {
                        state_tag = "[ SKIP ]";
                        skipped += 1;
                    }

                    if(ostream) {
                        *ostream << state_tag << ' ' << t->get_name() << std::endl;
                    }
                }

                if(ostream) {
                    const int non_skipped = passed + failed;
                    const float pass_ratio = (non_skipped == 0) ? 1 : static_cast<float>(passed) / non_skipped;

                    ostream->precision(2);

                    *ostream << "\nSUMMARY\n\tTotal: " << tests.size() << '\n';
                    *ostream << "\tP/F/S: " << passed << '/' << failed << '/' << skipped << '\n';
                    *ostream << "\tPass ratio: " << pass_ratio << std::endl;
                }

                return all_success;
            }

            test_ptr add_test(
                const std::string_view name,
                const test_function test_func,
                const skip_predicate skip_if = default_skip_predicate
            ) {
                return tests.emplace_back(std::make_shared<test>(name, test_func, skip_if));
            }

            int main(int argc CCLUNUSED, char **argv CCLUNUSED) {
                return 1 - execute();
            }
    };

    inline void fail() { throw test_failed_exception(); }

    inline void check(const bool condition) {
        if(!condition) {
            fail();
        }
    }

    template<typename T, typename U, typename Z = std::common_type_t<T, U>>
    inline void equals(T&& value1, U&& value2) {
        if(static_cast<Z>(value1) != static_cast<Z>(value2)) {
            std::cerr << "equals(" << value1 << ", " << value2 << ") failed." << std::endl;

            fail();
        }
    }

    template<typename T, typename U, typename Z = std::common_type_t<T, U>>
    inline void differs(T&& value1, U&& value2) {
        if(static_cast<Z>(value1) == static_cast<Z>(value2)) {
            std::cerr << "equals(" << value1 << ", " << value2 << ") failed." << std::endl;

            fail();
        }
    }

    template<typename Exception = std::exception>
    inline void throws(std::function<void()> code) {
        try {
            code();
        } catch (Exception &) {
            return;
        }

        fail();
    }

    constexpr const auto skip_if_typechecking_disabled = [] () -> bool {
        #ifdef CCL_FEATURE_TYPECHECK_CASTS
            return false;
        #else // CCL_FEATURE_TYPECHECK_CASTS
            return true;
        #endif // CCL_FEATURE_TYPECHECK_CASTS
    };

    constexpr const auto skip_if_exceptions_disabled = [] () -> bool {
        #ifdef CCL_FEATURE_EXCEPTIONS
            return false;
        #else // CCL_FEATURE_EXCEPTIONS
            return true;
        #endif // CCL_FEATURE_EXCEPTIONS
    };
}

#endif // CCL_TEST_HPP
