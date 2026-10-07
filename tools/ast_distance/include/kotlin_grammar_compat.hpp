#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include <string_view>
#include <functional>
#include <iterator>
#include <tree_sitter/api.h>

namespace ast_distance {
// The vendored Kotlin grammar predates fun interfaces. Blank only the SAM
// modifier in parser input; byte offsets and original source text stay intact.
struct KotlinGrammarInput {
    std::string text;
    std::vector<int> fun_interface_lines;
};
// Tree-sitter permits a word to become an identifier when its keyword token
// is not expected in that parse state. Kotlin's lexical contract still forbids
// bare hard keywords (KtTokens.java:350-359); backticks and soft keywords remain
// legal. Validate the original CST spans without changing or hiding tokens.
// This diagnoses a parser classification, not the validity of the source:
// valid declarations can also reach this state through grammar limitations.
inline std::vector<std::string> kotlin_identifier_diagnostics(TSNode root, const std::string& source) {
    static constexpr std::string_view hard_keywords[] = {
        "as", "break", "class", "continue", "do", "else", "false", "for", "fun",
        "if", "in", "interface", "is", "null", "object", "package", "return",
        "super", "this", "throw", "true", "try", "typealias", "typeof", "val", "var", "when", "while"};
    std::vector<std::string> diagnostics;
    std::function<void(TSNode)> visit = [&](TSNode node) {
        if (std::string_view(ts_node_type(node)) == "simple_identifier") {
            const auto begin = ts_node_start_byte(node), end = ts_node_end_byte(node);
            const std::string_view spelling(source.data() + begin, end - begin);
            if (std::find(std::begin(hard_keywords), std::end(hard_keywords), spelling) != std::end(hard_keywords))
                diagnostics.push_back(std::to_string(ts_node_start_point(node).row + 1) +
                    ": grammar treated hard keyword as identifier: " + std::string(spelling) +
                    " bytes " + std::to_string(begin) + ":" + std::to_string(end));
        }
        for (uint32_t i = 0; i < ts_node_named_child_count(node); ++i) visit(ts_node_named_child(node, i));
    };
    visit(root);
    return diagnostics;
}
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
