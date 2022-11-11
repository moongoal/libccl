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
#include <charcoal/api.hpp>

namespace ccl {
    using test_function = std::function<void()>;

    class test_failed_exception : public std::exception {};

    class test {
        friend class test_suite;

        std::string name;
        test_function test_func;

        void execute() const { test_func(); }

        public:
            test(
                const std::string_view name,
                const test_function test_func
            ) : name{name}, test_func{test_func}
            {}

            const std::string& get_name() const { return name; }
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

                for(const auto &t : tests) {
                    std::string state_tag = "[ PASS ]";

                    try {
                        t->execute();
                    } catch (test_failed_exception exc) {
                        state_tag = "[*FAIL*]";
                        all_success = false;
                    }

                    if(ostream) {
                        *ostream << state_tag << ' ' << t->get_name() << std::endl;
                    }
                }

                return all_success;
            }

            test_ptr add_test(
                const std::string_view name,
                const test_function test_func
            ) {
                return tests.emplace_back(std::make_shared<test>(name, test_func));
            }

            int main(int argc, char **argv) {
                return 1 - execute();
            }
    };

    inline void fail() { throw test_failed_exception(); }

    inline void check(const bool condition) {
        if(!condition) {
            fail();
        }
    }

    static test_suite suite;
}

#endif // CCL_TEST_HPP
