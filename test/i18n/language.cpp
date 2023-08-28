#include <cstring>
#include <ccl/test/test.hpp>
#include <ccl/i18n/language.hpp>

using namespace ccl;

int main(int argc, char **argv) {
    test_suite suite;

    suite.add_test("iso639_1_language_codes mapping", [] () {
        equals(::strcmp(i18n::iso639_1_language_codes[i18n::LANGUAGE_NONE], "<none>"), 0);
        equals(::strcmp(i18n::iso639_1_language_codes[i18n::LANGUAGE_UKRAINIAN], "uk"), 0);
        equals(::strcmp(i18n::iso639_1_language_codes[i18n::LANGUAGE_ITALIAN], "it"), 0);
        equals(::strcmp(i18n::iso639_1_language_codes[i18n::LANGUAGE_ENGLISH], "en"), 0);
        equals(::strcmp(i18n::iso639_1_language_codes[i18n::LANGUAGE_ZULU], "zu"), 0);
    });

    return suite.main(argc, argv);
}
