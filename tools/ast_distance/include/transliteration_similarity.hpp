#pragma once

#include "ast_parser.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ast_distance {

class TransliterationSimilarity {
public:
    struct Report {
        float score = 0.0f;
        float token_cosine = 0.0f;
        float token_jaccard = 0.0f;
        float kgram_jaccard = 0.0f;
        int source_tokens = 0;
        int target_tokens = 0;
    };

    static Report compare(const std::string& source,
                          Language source_lang,
                          const std::string& target,
                          Language target_lang) {
        auto source_tokens = normalize_tokens(source, source_lang);
        auto target_tokens = normalize_tokens(target, target_lang);

        Report report;
        report.source_tokens = static_cast<int>(source_tokens.size());
        report.target_tokens = static_cast<int>(target_tokens.size());
        report.token_cosine = token_cosine(source_tokens, target_tokens);
        report.token_jaccard = token_jaccard(source_tokens, target_tokens);
        report.kgram_jaccard = kgram_jaccard(source_tokens, target_tokens);
        report.score = 0.55f * report.token_cosine +
                       0.25f * report.token_jaccard +
                       0.20f * report.kgram_jaccard;
        return report;
    }

private:
    static bool is_ident_start(char c) {
        unsigned char u = static_cast<unsigned char>(c);
        return std::isalpha(u) || c == '_';
    }

    static bool is_ident_continue(char c) {
        unsigned char u = static_cast<unsigned char>(c);
        return std::isalnum(u) || c == '_';
    }

    static bool is_digit(char c) {
        return std::isdigit(static_cast<unsigned char>(c));
    }

    static std::string normalize_identifier(const std::string& token) {
        std::string out = IdentifierStats::canonicalize(token);
        if (out == "i8") return "byte";
        if (out == "u8") return "ubyte";
        if (out == "i16") return "short";
        if (out == "u16") return "ushort";
        if (out == "shortarray") return "vecshort";
        if (out == "arrayvec") return "vec";
        return out;
    }

    static void skip_line_comment(const std::string& text, size_t& i) {
        while (i < text.size() && text[i] != '\n') {
            ++i;
        }
    }

    static void skip_block_comment(const std::string& text, size_t& i) {
        i += 2;
        int depth = 1;
        while (i + 1 < text.size() && depth > 0) {
            if (text[i] == '/' && text[i + 1] == '*') {
                depth++;
                i += 2;
            } else if (text[i] == '*' && text[i + 1] == '/') {
                depth--;
                i += 2;
            } else {
                ++i;
            }
        }
    }

    static bool skip_rust_raw_string(const std::string& text, size_t& i) {
        if (i >= text.size() || text[i] != 'r') return false;
        size_t j = i + 1;
        while (j < text.size() && text[j] == '#') {
            ++j;
        }
        if (j >= text.size() || text[j] != '"') return false;

        size_t hashes = j - (i + 1);
        j++;
        while (j < text.size()) {
            if (text[j] == '"') {
                bool closes = true;
                for (size_t h = 0; h < hashes; ++h) {
                    if (j + 1 + h >= text.size() || text[j + 1 + h] != '#') {
                        closes = false;
                        break;
                    }
                }
                if (closes) {
                    i = j + 1 + hashes;
                    return true;
                }
            }
            ++j;
        }

        i = text.size();
        return true;
    }

    static void skip_quoted(const std::string& text, size_t& i, char quote) {
        ++i;
        while (i < text.size()) {
            if (text[i] == '\\') {
                i += 2;
                continue;
            }
            if (text[i] == quote) {
                ++i;
                return;
            }
            ++i;
        }
    }

    static void push_token(std::vector<std::string>& tokens, std::string token) {
        if (!token.empty()) {
            tokens.push_back(std::move(token));
        }
    }

    static std::vector<std::string> normalize_tokens(const std::string& text, Language lang) {
        std::vector<std::string> tokens;
        tokens.reserve(text.size() / 4);

        for (size_t i = 0; i < text.size();) {
            char c = text[i];
            if (std::isspace(static_cast<unsigned char>(c))) {
                ++i;
                continue;
            }

            if (i + 1 < text.size() && text[i] == '/' && text[i + 1] == '/') {
                skip_line_comment(text, i);
                continue;
            }
            if (i + 1 < text.size() && text[i] == '/' && text[i + 1] == '*') {
                skip_block_comment(text, i);
                continue;
            }

            if (lang == Language::RUST && skip_rust_raw_string(text, i)) {
                push_token(tokens, "str");
                continue;
            }

            if (lang == Language::KOTLIN && c == '`') {
                size_t start = ++i;
                while (i < text.size() && text[i] != '`') {
                    ++i;
                }
                push_token(tokens, normalize_identifier(text.substr(start, i - start)));
                if (i < text.size()) ++i;
                continue;
            }

            if (c == '"' || c == '\'') {
                if (lang == Language::RUST && c == '\'' && i + 1 < text.size() &&
                    is_ident_start(text[i + 1])) {
                    size_t start = i + 1;
                    size_t j = start;
                    while (j < text.size() && is_ident_continue(text[j])) {
                        ++j;
                    }
                    if (j >= text.size() || text[j] != '\'') {
                        i = j;
                        continue;
                    }
                }
                skip_quoted(text, i, c);
                push_token(tokens, "str");
                continue;
            }

            if (is_digit(c)) {
                size_t start = i;
                while (i < text.size() &&
                       (std::isalnum(static_cast<unsigned char>(text[i])) ||
                        text[i] == '_' || text[i] == '.')) {
                    ++i;
                }
                push_token(tokens, "num:" + text.substr(start, i - start));
                continue;
            }

            if (is_ident_start(c)) {
                size_t start = i;
                while (i < text.size() && is_ident_continue(text[i])) {
                    ++i;
                }
                push_token(tokens, normalize_identifier(text.substr(start, i - start)));
                continue;
            }

            if (i + 1 < text.size()) {
                std::string two = text.substr(i, 2);
                if (two == "::" || two == "?.") {
                    push_token(tokens, ".");
                    i += 2;
                    continue;
                }
                if (two == "->" || two == "=>") {
                    push_token(tokens, "arrow");
                    i += 2;
                    continue;
                }
                if (two == "==" || two == "!=" || two == "<=" || two == ">=" ||
                    two == "&&" || two == "||" || two == "+=" || two == "-=" ||
                    two == "*=" || two == "/=" || two == "%=") {
                    push_token(tokens, two);
                    i += 2;
                    continue;
                }
            }

            if (c == ';' || c == '@' || c == '#') {
                ++i;
                continue;
            }

            tokens.emplace_back(1, c);
            ++i;
        }

        return tokens;
    }

    static std::unordered_map<std::string, int> frequencies(const std::vector<std::string>& tokens) {
        std::unordered_map<std::string, int> out;
        out.reserve(tokens.size());
        for (const auto& token : tokens) {
            out[token]++;
        }
        return out;
    }

    static float token_cosine(const std::vector<std::string>& a,
                              const std::vector<std::string>& b) {
        if (a.empty() && b.empty()) return 1.0f;
        if (a.empty() || b.empty()) return 0.0f;

        auto fa = frequencies(a);
        auto fb = frequencies(b);
        double dot = 0.0;
        double na = 0.0;
        double nb = 0.0;
        for (const auto& [token, count] : fa) {
            na += static_cast<double>(count) * count;
            auto it = fb.find(token);
            if (it != fb.end()) {
                dot += static_cast<double>(count) * it->second;
            }
        }
        for (const auto& [_, count] : fb) {
            nb += static_cast<double>(count) * count;
        }
        if (na <= 0.0 || nb <= 0.0) return 0.0f;
        return static_cast<float>(dot / (std::sqrt(na) * std::sqrt(nb)));
    }

    static float token_jaccard(const std::vector<std::string>& a,
                               const std::vector<std::string>& b) {
        if (a.empty() && b.empty()) return 1.0f;
        if (a.empty() || b.empty()) return 0.0f;

        std::unordered_set<std::string> sa(a.begin(), a.end());
        std::unordered_set<std::string> sb(b.begin(), b.end());
        int intersection = 0;
        for (const auto& token : sa) {
            if (sb.count(token)) {
                intersection++;
            }
        }
        int union_size = static_cast<int>(sa.size() + sb.size()) - intersection;
        if (union_size == 0) return 1.0f;
        return static_cast<float>(intersection) / static_cast<float>(union_size);
    }

    static uint64_t hash_token(const std::string& token) {
        uint64_t h = 1469598103934665603ull;
        for (unsigned char c : token) {
            h ^= c;
            h *= 1099511628211ull;
        }
        return h;
    }

    static std::unordered_set<uint64_t> kgrams(const std::vector<std::string>& tokens) {
        static constexpr size_t K = 5;
        std::unordered_set<uint64_t> out;
        if (tokens.empty()) return out;

        size_t limit = tokens.size() >= K ? tokens.size() - K + 1 : 1;
        out.reserve(limit);
        for (size_t i = 0; i < limit; ++i) {
            uint64_t h = 1469598103934665603ull;
            size_t end = std::min(tokens.size(), i + K);
            for (size_t j = i; j < end; ++j) {
                h ^= hash_token(tokens[j]) + 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2);
                h *= 1099511628211ull;
            }
            out.insert(h);
        }
        return out;
    }

    static float kgram_jaccard(const std::vector<std::string>& a,
                               const std::vector<std::string>& b) {
        if (a.empty() && b.empty()) return 1.0f;
        if (a.empty() || b.empty()) return 0.0f;

        auto ga = kgrams(a);
        auto gb = kgrams(b);
        int intersection = 0;
        for (uint64_t h : ga) {
            if (gb.count(h)) {
                intersection++;
            }
        }
        int union_size = static_cast<int>(ga.size() + gb.size()) - intersection;
        if (union_size == 0) return 1.0f;
        return static_cast<float>(intersection) / static_cast<float>(union_size);
    }
};

} // namespace ast_distance
