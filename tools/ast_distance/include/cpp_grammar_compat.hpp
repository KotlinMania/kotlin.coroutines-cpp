#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace ast_distance {
struct CppGrammarInput {
    std::string text;
    std::vector<int> statement_macro_lines;
};

// Parse the unexpanded coroutine DSL as calls. Its begin/end macros supply their
// own statement terminators; tree-sitter cannot see the expansion. Add only a
// parser terminator in available whitespace, preserving bytes, lines and all
// original call/argument text. This does not validate the macro expansion.
inline CppGrammarInput cpp_grammar_input(const std::string& source) {
    CppGrammarInput result{source, {}};
    struct Token { std::string text; size_t start, end; };
    std::vector<Token> tokens;
    for (size_t i = 0; i < source.size();) {
        if (std::isspace(static_cast<unsigned char>(source[i]))) { ++i; continue; }
        if (source.compare(i, 2, "//") == 0) {
            while (i < source.size() && source[i] != '\n') ++i;
            continue;
        }
        if (source.compare(i, 2, "/*") == 0) {
            auto end = source.find("*/", i + 2);
            i = end == std::string::npos ? source.size() : end + 2;
            continue;
        }
        if (source[i] == '#') {
            // Do not change macro definitions or other preprocessor directives.
            do {
                auto end = source.find('\n', i);
                if (end == std::string::npos) { i = source.size(); break; }
                bool continued = end > 0 && source[end - 1] == '\\';
                i = end + 1;
                if (!continued) break;
            } while (i < source.size());
            continue;
        }
        if (source[i] == '"' || source[i] == '\'') {
            char quote = source[i];
            // A digit separator is not the start of a character literal.
            if (quote == '\'' && i && std::isalnum(static_cast<unsigned char>(source[i - 1])) &&
                i + 1 < source.size() && std::isalnum(static_cast<unsigned char>(source[i + 1]))) {
                ++i; continue;
            }
            size_t start = i++;
            if (quote == '"' && start && source[start - 1] == 'R') {
                auto opening = source.find('(', i);
                if (opening == std::string::npos || opening - i > 16) break;
                std::string closing = ")" + source.substr(i, opening - i) + "\"";
                auto end = source.find(closing, opening + 1);
                i = end == std::string::npos ? source.size() : end + closing.size();
            } else {
                while (i < source.size()) {
                    if (source[i] == '\\' && i + 1 < source.size()) { i += 2; continue; }
                    if (source[i++] == quote) break;
                }
            }
            tokens.push_back({"<literal>", start, i});
            continue;
        }
        size_t start = i++;
        if (std::isalpha(static_cast<unsigned char>(source[start])) || source[start] == '_')
            while (i < source.size() && (std::isalnum(static_cast<unsigned char>(source[i])) || source[i] == '_')) ++i;
        tokens.push_back({source.substr(start, i - start), start, i});
    }
    for (size_t i = 0; i + 1 < tokens.size(); ++i) {
        const auto& token = tokens[i];
        if ((token.text != "coroutine_begin" && token.text != "coroutine_end") || tokens[i + 1].text != "(") continue;
        // Bounded to standalone statements, not declarations/member expressions.
        auto newline = token.start == 0 ? std::string::npos : source.rfind('\n', token.start - 1);
        size_t line_start = newline == std::string::npos ? 0 : newline + 1;
        bool line_statement = std::all_of(source.begin() + line_start, source.begin() + token.start,
            [](unsigned char c) { return std::isspace(c); });
        if (!line_statement && (i == 0 || (tokens[i - 1].text != "{" && tokens[i - 1].text != ";"))) continue;
        size_t close = i + 1;
        int depth = 0;
        for (; close < tokens.size(); ++close) {
            if (tokens[close].text == "(") ++depth;
            if (tokens[close].text == ")" && --depth == 0) break;
        }
        if (close == tokens.size() || close + 1 == tokens.size()) continue;
        const auto& next = tokens[close + 1].text;
        if (next == ";" || next == "{" || next == ")" || next == "]" || next == "," ||
            next == "." || next == "(" || next == ":" || next == "?" || next == "=" ||
            next == "+" || next == "-" || next == "*" || next == "/" || next == "%" ||
            next == "&" || next == "|" || next == "^" || next == "<" || next == ">") continue;
        for (size_t position = tokens[close].end; position < source.size() &&
            std::isspace(static_cast<unsigned char>(source[position])); ++position) {
            if (source[position] == '\n' || source[position] == '\r') continue;
            result.text[position] = ';';
            result.statement_macro_lines.push_back(1 + static_cast<int>(std::count(source.begin(), source.begin() + token.start, '\n')));
            break;
        }
        i = close;
    }
    return result;
}
} // namespace ast_distance
