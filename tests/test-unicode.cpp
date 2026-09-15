#include "../src/unicode.h"

#include <cstdio>
#include <string>
#include <vector>

static bool test_k2_horizon_regex() {
    const std::vector<std::string> regex_exprs = {
        "(?:'[sS]|'[tT]|'[rR][eE]|'[vV][eE]|'[mM]|'[lL][lL]|'[dD])|[^\\r\\n\\p{L}\\p{N}]?(?:\\p{L}|\\p{M}|\\u200C|\\u200D)+|\\p{N}{1,3}| ?[^\\s\\p{L}\\p{N}]+[\\r\\n]*|\\s*[\\r\\n]+|\\s+(?!\\S)|\\s+",
    };

    // Empty input must not fall through to std::regex because MSVC rejects the K2 pattern's Unicode escapes.
    if (!unicode_regex_split("", regex_exprs, false).empty()) {
        fprintf(stderr, "empty K2 Horizon input produced pieces\n");
        return false;
    }

    const auto expect = [&] (const std::string & input, const std::vector<std::string> & expected, const char * label) -> bool {
        const auto actual = unicode_regex_split(input, regex_exprs, false);
        if (actual != expected) {
            fprintf(stderr, "unexpected K2 Horizon split for %s:", label);
            for (const auto & piece : actual) {
                fprintf(stderr, " [%s]", piece.c_str());
            }
            fprintf(stderr, "\n");
            return false;
        }
        return true;
    };

    if (!expect("abc1234", { "abc", "123", "4" }, "ASCII")) {
        return false;
    }

    // Combining marks and joiners remain part of the surrounding word.
    if (!expect("a\xCC\x81" "b", { "a\xCC\x81" "b" }, "combining mark") ||
        !expect("a\xE2\x80\x8C" "b", { "a\xE2\x80\x8C" "b" }, "zero-width non-joiner") ||
        !expect("a\xE2\x80\x8D" "b", { "a\xE2\x80\x8D" "b" }, "zero-width joiner")) {
        return false;
    }

    // Non-ASCII letters and numbers must use Unicode character categories.
    if (!expect("caf\xC3\xA9", { "caf\xC3\xA9" }, "non-ASCII letter") ||
        !expect("\xD9\xA1\xD9\xA2\xD9\xA3\xD9\xA4", {
            "\xD9\xA1\xD9\xA2\xD9\xA3", "\xD9\xA4"
        }, "non-ASCII numbers")) {
        return false;
    }

    const std::string nbsp = "\xC2\xA0";
    if (!expect(std::string("a") + nbsp + "!", { "a", nbsp, "!" }, "non-ASCII whitespace")) {
        return false;
    }

    return true;
}

int main() {
    if (!test_k2_horizon_regex()) {
        return 1;
    }

    const std::vector<std::string> regex_exprs = {
        "[~][A-Za-z]+| ?[\\p{S}]+|\\s+",
    };
    const std::vector<std::string> expected = { " ~", "foo" };
    const auto actual = unicode_regex_split(" ~foo", regex_exprs, false);

    if (actual != expected) {
        fprintf(stderr, "unexpected split:");
        for (const auto & piece : actual) {
            fprintf(stderr, " [%s]", piece.c_str());
        }
        fprintf(stderr, "\n");
        return 1;
    }

    return 0;
}
