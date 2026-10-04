#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace ast_distance {
// The vendored Kotlin grammar predates fun interfaces. Blank only the SAM
// modifier in parser input; byte offsets and original source text stay intact.
struct KotlinGrammarInput {
    std::string text;
    std::vector<int> fun_interface_lines;
};
inline KotlinGrammarInput kotlin_grammar_input(const std::string& source) {
    KotlinGrammarInput result{source, {}};
    std::vector<std::pair<std::string, size_t>> tokens;
    for (size_t i = 0; i < source.size();) {
        if (source[i] == '\n') { ++i; continue; }
        if (std::isspace(static_cast<unsigned char>(source[i]))) { ++i; continue; }
        if (source.compare(i, 2, "//") == 0) {
            while (i < source.size() && source[i] != '\n') ++i;
            continue;
        }
        if (source.compare(i, 2, "/*") == 0) {
            int depth = 1; i += 2;
            while (i < source.size() && depth) {
                if (source.compare(i, 2, "/*") == 0) { ++depth; i += 2; }
                else if (source.compare(i, 2, "*/") == 0) { --depth; i += 2; }
                else { ++i; }
            }
            continue;
        }
        if (source[i] == '"' || source[i] == '\'' || source[i] == '`') {
            char quote = source[i];
            bool triple = quote == '"' && source.compare(i, 3, "\"\"\"") == 0;
            i += triple ? 3 : 1;
            while (i < source.size()) {
                if (triple && source.compare(i, 3, "\"\"\"") == 0) { i += 3; break; }
                if (!triple && source[i] == quote) { ++i; break; }
                if (!triple && quote != '`' && source[i] == '\\' && i + 1 < source.size()) {
                    i += 2; continue;
                }
                ++i;
            }
            tokens.clear();
            continue;
        }
        size_t start = i;
        if (std::isalpha(static_cast<unsigned char>(source[i])) || source[i] == '_') {
            while (i < source.size() && (std::isalnum(static_cast<unsigned char>(source[i])) || source[i] == '_')) ++i;
            std::string token = source.substr(start, i - start);
            if (token == "interface" && !tokens.empty() && tokens.back().first == "fun") {
                size_t modifier = tokens.back().second;
                result.text.replace(modifier, 3, "   ");
                result.fun_interface_lines.push_back(1 + static_cast<int>(std::count(source.begin(), source.begin() + modifier, '\n')));
            }
            tokens = {{token, start}};
        } else { ++i; tokens.clear(); }
    }
    return result;
}
} // namespace ast_distance
