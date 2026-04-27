#pragma once

#include "tree.hpp"
#include "node_types.hpp"
#include <tree_sitter/api.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <cctype>
#include <cstring>
#include <algorithm>
#include <cmath>

// External declarations for tree-sitter language functions
extern "C" {
    const TSLanguage* tree_sitter_rust();
    const TSLanguage* tree_sitter_kotlin();
    const TSLanguage* tree_sitter_cpp();
    const TSLanguage* tree_sitter_python();
    const TSLanguage* tree_sitter_typescript();
}

namespace ast_distance {

enum class Language {
    RUST,
    KOTLIN,
    CPP,
    PYTHON,
    TYPESCRIPT
};

/**
 * Statistics about identifiers (function names, variable names, etc.)
 * Used for Phase 3: Token histogram analysis to detect naming divergences.
 */
struct IdentifierStats {
    std::map<std::string, int> identifier_freq;  // Frequency of each identifier
    std::map<std::string, int> canonical_freq;   // Canonicalized (lowercase, no underscores)
    int total_identifiers = 0;

    /**
     * Canonicalize an identifier for cross-language comparison.
     * "foo_bar" and "fooBar" and "FooBar" all become "foobar".
     * This lets snake_case Rust match camelCase Kotlin.
     *
     * This is intentionally strict: it does not map `fmt` to `toString`,
     * `cmp` to `compareTo`, or collection/type synonyms. Those translations
     * are real naming choices and should stay visible in parity reports.
     */
    static std::string canonicalize(const std::string& name) {
        // Strict snake_case <-> camelCase / PascalCase parity:
        // collapse case and drop underscores. No type-name remapping,
        // no ignore lists, no synonym tables.
        std::string result;
        result.reserve(name.size());
        for (char c : name) {
            if (c != '_') {
                result += static_cast<char>(
                    std::tolower(static_cast<unsigned char>(c)));
            }
        }
        return result;
    }

    void add_identifier(const std::string& name) {
        if (!name.empty()) {
            identifier_freq[name]++;
            std::string c = canonicalize(name);
            if (!c.empty()) {
                canonical_freq[c]++;
            }
            total_identifiers++;
        }
    }

    /**
     * Compute cosine similarity of identifier frequencies with another IdentifierStats.
     * Uses raw identifiers — detects naming divergences when same language.
     */
    float identifier_cosine_similarity(const IdentifierStats& other) const {
        return cosine_similarity_of(identifier_freq, other.identifier_freq);
    }

    /**
     * Compute cosine similarity using canonicalized identifiers.
     * This is the key metric for cross-language porting: "foo_bar" matches "fooBar".
     * A file full of placeholder stubs will score near 0 because the *names* are wrong,
     * even if the AST shape looks similar.
     */
    float canonical_cosine_similarity(const IdentifierStats& other) const {
        return cosine_similarity_of(canonical_freq, other.canonical_freq);
    }

    /**
     * Jaccard similarity of canonicalized identifier sets (ignoring frequency).
     * What fraction of identifiers are shared between source and target?
     */
    float canonical_jaccard_similarity(const IdentifierStats& other) const {
        if (canonical_freq.empty() && other.canonical_freq.empty()) return 1.0f;
        if (canonical_freq.empty() || other.canonical_freq.empty()) return 0.0f;

        std::set<std::string> ids1, ids2;
        for (const auto& [id, _] : canonical_freq) ids1.insert(id);
        for (const auto& [id, _] : other.canonical_freq) ids2.insert(id);

        int intersection = 0;
        for (const auto& id : ids1) {
            if (ids2.count(id)) intersection++;
        }

        int union_size = static_cast<int>(ids1.size() + ids2.size()) - intersection;
        if (union_size == 0) return 1.0f;
        return static_cast<float>(intersection) / static_cast<float>(union_size);
    }

private:
    static float cosine_similarity_of(const std::map<std::string, int>& a,
                                       const std::map<std::string, int>& b) {
        if (a.empty() || b.empty()) return 0.0f;

        std::set<std::string> all_ids;
        for (const auto& [id, _] : a) all_ids.insert(id);
        for (const auto& [id, _] : b) all_ids.insert(id);

        double dot = 0.0, norm1 = 0.0, norm2 = 0.0;
        for (const auto& id : all_ids) {
            int freq1 = 0, freq2 = 0;
            auto it1 = a.find(id);
            auto it2 = b.find(id);
            if (it1 != a.end()) freq1 = it1->second;
            if (it2 != b.end()) freq2 = it2->second;

            dot += freq1 * freq2;
            norm1 += freq1 * freq1;
            norm2 += freq2 * freq2;
        }

        if (norm1 < 1e-8 || norm2 < 1e-8) return 0.0f;
        return static_cast<float>(dot / (std::sqrt(norm1) * std::sqrt(norm2)));
    }
};

/**
 * Statistics about comments/documentation in source code.
 */
struct CommentStats {
    int doc_comment_count = 0;      // /** ... */ or /// style
    int line_comment_count = 0;     // // style (non-doc)
    int block_comment_count = 0;    // /* ... */ style (non-doc)
    int total_comment_lines = 0;    // Total lines occupied by comments
    int total_doc_lines = 0;        // Lines in doc comments specifically
    std::vector<std::string> doc_texts;  // Raw text of each doc comment
    std::map<std::string, int> word_freq; // Bag of words from all doc comments

    void print() const {
        printf("Comment Statistics:\n");
        printf("  Doc comments:      %d\n", doc_comment_count);
        printf("  Line comments:     %d\n", line_comment_count);
        printf("  Block comments:    %d\n", block_comment_count);
        printf("  Total comment lines: %d\n", total_comment_lines);
        printf("  Doc comment lines:   %d\n", total_doc_lines);
        printf("  Unique doc words:    %d\n", static_cast<int>(word_freq.size()));
    }

    float doc_coverage_ratio() const {
        if (total_comment_lines == 0) return 0.0f;
        return static_cast<float>(total_doc_lines) / static_cast<float>(total_comment_lines);
    }

    /**
     * Compute documentation *amount* coverage of `other` relative to `this`.
     *
     * This is intentionally asymmetric: if `other` has *more* documentation lines
     * than `this`, we treat that as full coverage (1.0) rather than a penalty.
     * This helps ensure ports that add extra KDoc are not graded as "worse" on
     * documentation amount.
     */
    float doc_line_coverage_capped(const CommentStats& other) const {
        if (total_doc_lines <= 0) return 1.0f;
        float ratio = static_cast<float>(other.total_doc_lines) / static_cast<float>(total_doc_lines);
        return std::min(1.0f, ratio);
    }

    /**
     * Compute symmetric documentation amount balance (min/max of doc lines).
     *
     * This *does* penalize extra docs and missing docs equally; it is useful as an
     * informational metric, but should not be used alone to grade ports.
     */
    float doc_line_balance(const CommentStats& other) const {
        int max_lines = std::max(total_doc_lines, other.total_doc_lines);
        if (max_lines <= 0) return 1.0f;
        int min_lines = std::min(total_doc_lines, other.total_doc_lines);
        return static_cast<float>(min_lines) / static_cast<float>(max_lines);
    }

    /**
     * Compute cosine similarity of doc word frequencies with another CommentStats.
     * Returns 0.0 to 1.0 where 1.0 = identical vocabulary distribution.
     */
    float doc_cosine_similarity(const CommentStats& other) const {
        if (word_freq.empty() || other.word_freq.empty()) {
            return 0.0f;
        }

        // Build union of all words
        std::set<std::string> all_words;
        for (const auto& [word, _] : word_freq) all_words.insert(word);
        for (const auto& [word, _] : other.word_freq) all_words.insert(word);

        // Compute dot product and norms
        double dot = 0.0, norm1 = 0.0, norm2 = 0.0;
        for (const auto& word : all_words) {
            int freq1 = 0, freq2 = 0;
            auto it1 = word_freq.find(word);
            auto it2 = other.word_freq.find(word);
            if (it1 != word_freq.end()) freq1 = it1->second;
            if (it2 != other.word_freq.end()) freq2 = it2->second;

            dot += freq1 * freq2;
            norm1 += freq1 * freq1;
            norm2 += freq2 * freq2;
        }

        if (norm1 < 1e-8 || norm2 < 1e-8) return 0.0f;
        return static_cast<float>(dot / (std::sqrt(norm1) * std::sqrt(norm2)));
    }

    /**
     * Jaccard similarity of doc word sets (ignoring frequency).
     */
    float doc_jaccard_similarity(const CommentStats& other) const {
        if (word_freq.empty() && other.word_freq.empty()) return 1.0f;
        if (word_freq.empty() || other.word_freq.empty()) return 0.0f;

        std::set<std::string> words1, words2;
        for (const auto& [word, _] : word_freq) words1.insert(word);
        for (const auto& [word, _] : other.word_freq) words2.insert(word);

        int intersection = 0;
        for (const auto& w : words1) {
            if (words2.count(w)) intersection++;
        }

        int union_size = static_cast<int>(words1.size() + words2.size()) - intersection;
        if (union_size == 0) return 1.0f;
        return static_cast<float>(intersection) / static_cast<float>(union_size);
    }
};

/**
 * Function metadata extracted from source code.
 * The AST and identifiers are kept as parameters + body so transliteration
 * reports compare callable behavior, not loose whole-file shape.
 */
struct FunctionInfo {
    std::string name;
    TreePtr body_tree;
    IdentifierStats identifiers;
    bool has_stub_markers = false;
    bool is_test = false;  // true if #[test] or inside #[cfg(test)] mod
    int start_line = 0;    // 1-based declaration start line
    int end_line = 0;      // 1-based declaration end line
    int line_count = 0;    // declaration line span, inclusive
    int body_line_count = 0;
};

/**
 * AST Parser using tree-sitter.
 * Parses source files into normalized Tree structures.
 */
class ASTParser {
public:
    ASTParser() {
        parser_ = ts_parser_new();
    }

    ~ASTParser() {
        if (parser_) {
            ts_parser_delete(parser_);
        }
    }

    // Non-copyable
    ASTParser(const ASTParser&) = delete;
    ASTParser& operator=(const ASTParser&) = delete;

    // Movable
    ASTParser(ASTParser&& other) noexcept : parser_(other.parser_) {
        other.parser_ = nullptr;
    }

    /**
     * Parse multiple source files into a single normalized AST by concatenating them.
     */
    TreePtr parse_file(const std::vector<std::string>& filepaths, Language lang) {
        std::stringstream unified_buffer;
        for (const auto& filepath : filepaths) {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                throw std::runtime_error("Cannot open file: " + filepath);
            }
            unified_buffer << file.rdbuf() << "\n\n";
        }
        return parse_string(unified_buffer.str(), lang);
    }

    /**
     * Parse a source file into a normalized AST.
     */
    TreePtr parse_file(const std::string& filepath, Language lang) {
        return parse_file(std::vector<std::string>{filepath}, lang);
    }

    /**
     * Parse source code string into a normalized AST.
     */
    TreePtr parse_string(const std::string& source, Language lang) {
        // Set language
        const TSLanguage* ts_lang;
        switch (lang) {
            case Language::RUST: ts_lang = tree_sitter_rust(); break;
            case Language::KOTLIN: ts_lang = tree_sitter_kotlin(); break;
            case Language::CPP: ts_lang = tree_sitter_cpp(); break;
            case Language::PYTHON: ts_lang = tree_sitter_python(); break;
            case Language::TYPESCRIPT: ts_lang = tree_sitter_typescript(); break;
        }

        if (!ts_parser_set_language(parser_, ts_lang)) {
            throw std::runtime_error("Failed to set parser language");
        }

        // Parse
        TSTree* ts_tree = ts_parser_parse_string(
            parser_, nullptr, source.c_str(), source.length());

        if (!ts_tree) {
            throw std::runtime_error("Failed to parse source");
        }

        // Convert to our Tree structure
        TSNode root = ts_tree_root_node(ts_tree);
        TreePtr result = convert_node(root, source, lang);

        ts_tree_delete(ts_tree);
        return result;
    }

    /**
     * Extract comment statistics from source code.
     * Uses tree-sitter to find comment nodes.
     */
    CommentStats extract_comments(const std::string& source, Language lang) {
        CommentStats stats;

        const TSLanguage* ts_lang;
        switch (lang) {
            case Language::RUST: ts_lang = tree_sitter_rust(); break;
            case Language::KOTLIN: ts_lang = tree_sitter_kotlin(); break;
            case Language::CPP: ts_lang = tree_sitter_cpp(); break;
            case Language::PYTHON: ts_lang = tree_sitter_python(); break;
            case Language::TYPESCRIPT: ts_lang = tree_sitter_typescript(); break;
        }

        if (!ts_parser_set_language(parser_, ts_lang)) {
            return stats;
        }

        TSTree* ts_tree = ts_parser_parse_string(
            parser_, nullptr, source.c_str(), source.length());

        if (!ts_tree) {
            return stats;
        }

        TSNode root = ts_tree_root_node(ts_tree);
        extract_comments_recursive(root, source, lang, stats);

        ts_tree_delete(ts_tree);
        return stats;
    }

    /**
     * Extract comment statistics from multiple files combined.
     */
    CommentStats extract_comments_from_file(const std::vector<std::string>& filepaths, Language lang) {
        std::stringstream unified_buffer;
        for (const auto& filepath : filepaths) {
            std::ifstream file(filepath);
            if (file.is_open()) {
                unified_buffer << file.rdbuf() << "\n\n";
            }
        }
        return extract_comments(unified_buffer.str(), lang);
    }

    /**
     * Extract comment statistics from a file.
     */
    CommentStats extract_comments_from_file(const std::string& filepath, Language lang) {
        return extract_comments_from_file(std::vector<std::string>{filepath}, lang);
    }

    /**
     * Extract identifier statistics from source code.
     * Phase 3: Token histogram for detecting naming divergences.
     */
    IdentifierStats extract_identifiers(const std::string& source, Language lang) {
        IdentifierStats stats;

        // Set language
        const TSLanguage* ts_lang;
        switch (lang) {
            case Language::RUST: ts_lang = tree_sitter_rust(); break;
            case Language::KOTLIN: ts_lang = tree_sitter_kotlin(); break;
            case Language::CPP: ts_lang = tree_sitter_cpp(); break;
            case Language::PYTHON: ts_lang = tree_sitter_python(); break;
            case Language::TYPESCRIPT: ts_lang = tree_sitter_typescript(); break;
        }

        if (!ts_parser_set_language(parser_, ts_lang)) {
            return stats;
        }

        // Parse
        TSTree* ts_tree = ts_parser_parse_string(
            parser_, nullptr, source.c_str(), source.length());

        if (!ts_tree) {
            return stats;
        }

        TSNode root = ts_tree_root_node(ts_tree);
        extract_identifiers_recursive(root, source, lang, stats);

        ts_tree_delete(ts_tree);
        return stats;
    }

    /**
     * Extract identifier statistics from multiple files combined.
     */
    IdentifierStats extract_identifiers_from_file(const std::vector<std::string>& filepaths, Language lang) {
        std::stringstream unified_buffer;
        for (const auto& filepath : filepaths) {
            std::ifstream file(filepath);
            if (file.is_open()) {
                unified_buffer << file.rdbuf() << "\n\n";
            }
        }
        return extract_identifiers(unified_buffer.str(), lang);
    }

    /**
     * Extract identifier statistics from a file.
     */
    IdentifierStats extract_identifiers_from_file(const std::string& filepath, Language lang) {
        return extract_identifiers_from_file(std::vector<std::string>{filepath}, lang);
    }

    /**
     * Parse and extract only function bodies for comparison.
     */
    std::vector<std::pair<std::string, TreePtr>> extract_functions(
            const std::string& source, Language lang) {
        auto function_infos = extract_function_infos(source, lang);

        std::vector<std::pair<std::string, TreePtr>> functions;
        functions.reserve(function_infos.size());
        for (auto& info : function_infos) {
            functions.emplace_back(info.name, info.body_tree);
        }

        return functions;
    }

    /**
     * Extract function metadata and body ASTs from source.
     */
    std::vector<FunctionInfo> extract_function_infos(
            const std::string& source, Language lang) {
        std::vector<FunctionInfo> functions;

        // Set language
        const TSLanguage* ts_lang;
        switch (lang) {
            case Language::RUST: ts_lang = tree_sitter_rust(); break;
            case Language::KOTLIN: ts_lang = tree_sitter_kotlin(); break;
            case Language::CPP: ts_lang = tree_sitter_cpp(); break;
            case Language::PYTHON: ts_lang = tree_sitter_python(); break;
            case Language::TYPESCRIPT: ts_lang = tree_sitter_typescript(); break;
        }

        if (!ts_parser_set_language(parser_, ts_lang)) {
            throw std::runtime_error("Failed to set parser language");
        }

        TSTree* ts_tree = ts_parser_parse_string(
            parser_, nullptr, source.c_str(), source.length());

        if (!ts_tree) {
            throw std::runtime_error("Failed to parse source");
        }

        TSNode root = ts_tree_root_node(ts_tree);
        extract_function_infos_recursive(root, source, lang, functions);

        ts_tree_delete(ts_tree);
        return functions;
    }

    /**
     * Extract function metadata for one source file.
     */
    std::vector<FunctionInfo> extract_function_infos_from_file(
            const std::string& filepath, Language lang) {
        std::ifstream file(filepath);
        if (!file.is_open()) return {};

        std::stringstream buffer;
        buffer << file.rdbuf();
        return extract_function_infos(buffer.str(), lang);
    }

    /**
     * Extract function metadata for multiple source files.
     */
    std::vector<FunctionInfo> extract_function_infos_from_files(
            const std::vector<std::string>& filepaths, Language lang) {
        std::stringstream unified_buffer;

        for (const auto& filepath : filepaths) {
            std::ifstream file(filepath);
            if (file.is_open()) {
                unified_buffer << file.rdbuf() << "\n\n";
            }
        }

        if (unified_buffer.tellp() == std::streampos(0)) return {};

        return extract_function_infos(unified_buffer.str(), lang);
    }

    /**
     * Check if a string (case-insensitive) contains stub/TODO markers.
     */
    static bool text_has_stub_markers(const std::string& text) {
        std::string lower;
        lower.reserve(text.size());
        for (char c : text) {
            lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        // These patterns inside function bodies mean the code is fake.
        //
        // Use word-boundary style matching for short markers so we don't trip on
        // explanatory text like "TODOs/stubs are a failure mode" inside the tool itself.
        auto is_word = [](char ch) -> bool {
            return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
        };
        auto has_word = [&](const std::string& w) -> bool {
            size_t pos = lower.find(w);
            while (pos != std::string::npos) {
                bool left_ok = (pos == 0) || !is_word(lower[pos - 1]);
                size_t end = pos + w.size();
                bool right_ok = (end >= lower.size()) || !is_word(lower[end]);
                if (left_ok && right_ok) return true;
                pos = lower.find(w, pos + 1);
            }
            return false;
        };

        return has_word("todo") ||
               has_word("stub") ||
               has_word("placeholder") ||
               has_word("fixme") ||
               lower.find("not yet implemented") != std::string::npos ||
               lower.find("not implemented") != std::string::npos ||
               // Common stub constructs without spaces (Rust `unimplemented!`, Kotlin/Python `NotImplementedError`)
               has_word("unimplemented") ||
               has_word("notimplemented");
    }

    static bool comment_has_stub_markers(const std::string& text) {
        // For comment nodes, be stricter: only treat as a stub marker when the
        // comment itself starts with TODO/FIXME/STUB/etc. This avoids false
        // positives from explanatory comments inside the tool.
        std::string lower;
        lower.reserve(text.size());
        for (char c : text) {
            lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }

        size_t i = 0;
        if (lower.rfind("//", 0) == 0) {
            i = 2;
            while (i < lower.size() && lower[i] == '/') i++;  // ///, //!, etc.
        } else if (lower.rfind("#", 0) == 0) {
            i = 1;
            while (i < lower.size() && lower[i] == '#') i++;  // ## ...
        } else if (lower.rfind("/*", 0) == 0) {
            i = 2;
            if (i < lower.size() && lower[i] == '*') i++;  // /** ...
        }

        auto is_space = [](char ch) -> bool {
            return std::isspace(static_cast<unsigned char>(ch));
        };
        while (i < lower.size() && is_space(lower[i])) i++;
        while (i < lower.size() && lower[i] == '*') {  // leading "*" in block comment lines
            i++;
            while (i < lower.size() && is_space(lower[i])) i++;
        }

        auto is_word = [](char ch) -> bool {
            return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_';
        };
        auto starts_with_word = [&](const std::string& w) -> bool {
            if (lower.size() < i + w.size()) return false;
            if (lower.compare(i, w.size(), w) != 0) return false;
            size_t end = i + w.size();
            if (end >= lower.size()) return true;
            return !is_word(lower[end]);
        };

        // Most common: TODO:, TODO(...), FIXME:, etc.
        if (starts_with_word("todo") ||
            starts_with_word("fixme") ||
            starts_with_word("stub") ||
            starts_with_word("placeholder") ||
            starts_with_word("unimplemented") ||
            starts_with_word("notimplemented")) {
            return true;
        }

        // Phrases.
        if (lower.size() >= i + 16 && lower.compare(i, 16, "not implemented") == 0) return true;
        if (lower.size() >= i + 20 && lower.compare(i, 20, "not yet implemented") == 0) return true;

        return false;
    }

    /**
     * Check if a source file has stub/TODO markers inside function bodies.
     * Returns true if any function body contains these markers.
     * File-level comments are ignored — only code that's pretending to be real.
     *
     * Kotlin files with a port-lint header pointing to a module-root file are
     * always stubs. mod.rs is Rust's module declaration syntax; lib.rs/main.rs
     * are crate roots; __init__.py is a Python package marker. None have a
     * Kotlin equivalent. Porting them produces files that carry the source
     * language's namespace structure into Kotlin where it doesn't belong.
     * JS/TS index files are intentionally NOT in this list — they contain real
     * implementation code and must be ported normally.
     */
    bool has_stub_bodies(const std::string& source, Language lang) {
        if (lang == Language::KOTLIN) {
            const size_t scan_len = std::min<size_t>(source.size(), 256);
            const std::string header = source.substr(0, scan_len);
            const std::string port_lint = "// port-lint: source";
            auto pos = header.find(port_lint);
            if (pos != std::string::npos) {
                auto eol = header.find('\n', pos);
                std::string line = header.substr(pos, eol == std::string::npos ? std::string::npos : eol - pos);
                static const std::vector<std::string> module_roots = {
                    "/mod.rs", "/lib.rs", "/main.rs", "/__init__.py",
                };
                for (const auto& root : module_roots) {
                    if (line.find(root) != std::string::npos) return true;
                }
            }
        }

        const TSLanguage* ts_lang;
        switch (lang) {
            case Language::RUST: ts_lang = tree_sitter_rust(); break;
            case Language::KOTLIN: ts_lang = tree_sitter_kotlin(); break;
            case Language::CPP: ts_lang = tree_sitter_cpp(); break;
            case Language::PYTHON: ts_lang = tree_sitter_python(); break;
            case Language::TYPESCRIPT: ts_lang = tree_sitter_typescript(); break;
        }

        if (!ts_parser_set_language(parser_, ts_lang)) return false;

        TSTree* ts_tree = ts_parser_parse_string(
            parser_, nullptr, source.c_str(), source.length());
        if (!ts_tree) return false;

        TSNode root = ts_tree_root_node(ts_tree);
        std::vector<TSNode> stack;
        stack.push_back(root);

        while (!stack.empty()) {
            TSNode node = stack.back();
            stack.pop_back();

            std::string type_s(ts_node_type(node));
            if (is_function_node(type_s, lang)) {
                TSNode body_node = extract_function_body_node(node, lang);
                if (!ts_node_is_null(body_node) && !ts_node_eq(body_node, node)) {
                    if (has_stub_markers_in_node(body_node, source, lang)) {
                        ts_tree_delete(ts_tree);
                        return true;
                    }
                }
            }

            uint32_t child_count = ts_node_child_count(node);
            for (uint32_t i = 0; i < child_count; ++i) {
                TSNode child = ts_node_child(node, i);
                if (!ts_node_is_null(child)) {
                    stack.push_back(child);
                }
            }
        }

        ts_tree_delete(ts_tree);
        return false;
    }

    /**
     * Check if any file in a set has stub bodies.
     */
    bool has_stub_bodies_in_files(const std::vector<std::string>& filepaths, Language lang) {
        for (const auto& filepath : filepaths) {
            std::ifstream file(filepath);
            // Fail-safe: an unreadable file cannot be verified as complete.
            // Treat it as a stub rather than silently passing it.
            if (!file.is_open()) return true;
            std::stringstream buf;
            buf << file.rdbuf();
            if (has_stub_bodies(buf.str(), lang)) return true;
        }
        return false;
    }

    // Get unmapped node type counts (for diagnostics)
    const std::map<std::string, int>& get_unmapped_node_types() const {
        return unmapped_node_types_;
    }

    void clear_unmapped() { unmapped_node_types_.clear(); }

    private:
    TSParser* parser_;
    std::map<std::string, int> unmapped_node_types_;

    int count_lines(const std::string& text) {
        if (text.empty()) return 0;
        int lines = 1;
        for (char c : text) {
            if (c == '\n') lines++;
        }
        return lines;
    }

    /**
     * Extract words from doc comment text for bag-of-words comparison.
     * Strips comment markers, converts to lowercase, filters short words.
     */
    void tokenize_doc_comment(const std::string& text, std::map<std::string, int>& word_freq) {
        std::string current_word;
        for (char c : text) {
            if (std::isalnum(static_cast<unsigned char>(c))) {
                current_word += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            } else {
                if (current_word.length() >= 3) {
                    // Filter out common comment markers and very short words
                    if (current_word != "the" && current_word != "and" &&
                        current_word != "for" && current_word != "this" &&
                        current_word != "that" && current_word != "with") {
                        word_freq[current_word]++;
                    }
                }
                current_word.clear();
            }
        }
        // Don't forget last word
        if (current_word.length() >= 3) {
            if (current_word != "the" && current_word != "and" &&
                current_word != "for" && current_word != "this" &&
                current_word != "that" && current_word != "with") {
                word_freq[current_word]++;
            }
        }
    }

    void extract_comments_recursive(TSNode node, const std::string& source,
                                    Language lang, CommentStats& stats) {
        const char* type_str = ts_node_type(node);
        std::string type_s(type_str);

        // Check for comment nodes (tree-sitter node types vary by language)
        bool is_comment = false;
        bool is_doc_comment = false;
        bool is_line_comment = false;
        bool is_block_comment = false;

        if (lang == Language::KOTLIN) {
            is_line_comment = (type_s == "line_comment");
            is_block_comment = (type_s == "multiline_comment");
            is_comment = is_line_comment || is_block_comment;
        } else if (lang == Language::CPP) {
            is_line_comment = (type_s == "comment");  // // style
            is_block_comment = (type_s == "comment"); // Also covers /* */
            is_comment = (type_s == "comment");
        } else if (lang == Language::RUST) {
            is_line_comment = (type_s == "line_comment");
            is_block_comment = (type_s == "block_comment");
            is_comment = is_line_comment || is_block_comment;
        } else if (lang == Language::PYTHON) {
            is_comment = (type_s == "comment");
            is_line_comment = is_comment;
        } else if (lang == Language::TYPESCRIPT) {
            is_line_comment = (type_s == "comment");
            is_block_comment = (type_s == "comment");
            is_comment = (type_s == "comment");
        }

        if (is_comment) {
            uint32_t start = ts_node_start_byte(node);
            uint32_t end = ts_node_end_byte(node);
            std::string text;
            if (end > start && end <= source.length()) {
                text = source.substr(start, end - start);
            }

            int lines = count_lines(text);
            stats.total_comment_lines += lines;

            // Check if it's a doc comment.
            // Kotlin: KDoc is `/** ... */`. We also accept `///` and `//!` as doc comments
            // (some ports use them even though Kotlin doesn't require them) so documentation
            // scoring stays robust.
            // C++: `/** ... */` or `///` or `//!`
            // Rust: `///` or `//!` or `/** */`
            auto ltrim = [](const std::string& s) -> std::string_view {
                size_t i = 0;
                while (i < s.size()) {
                    char c = s[i];
                    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') break;
                    i++;
                }
                return std::string_view(s).substr(i);
            };
            std::string_view t = ltrim(text);
            if (lang == Language::KOTLIN) {
                is_doc_comment = (t.rfind("/**", 0) == 0) ||
                                 (t.rfind("///", 0) == 0) ||
                                 (t.rfind("//!", 0) == 0);
            } else if (lang == Language::CPP) {
                is_doc_comment = (text.find("/**") == 0) ||
                                 (text.find("///") == 0) ||
                                 (text.find("//!") == 0);
            } else if (lang == Language::RUST) {
                is_doc_comment = (text.find("///") == 0) ||
                                 (text.find("//!") == 0) ||
                                 (text.find("/**") == 0);
            } else if (lang == Language::PYTHON) {
                // Python has no standardized doc-comment syntax.
                // Docstrings are AST string nodes, not comment nodes.
                is_doc_comment = false;
            } else if (lang == Language::TYPESCRIPT) {
                is_doc_comment = (text.find("/**") == 0);
            }

            if (is_doc_comment) {
                stats.doc_comment_count++;
                stats.total_doc_lines += lines;
                stats.doc_texts.push_back(text);
                tokenize_doc_comment(text, stats.word_freq);
            } else if (is_block_comment || (text.find("/*") == 0)) {
                stats.block_comment_count++;
            } else {
                stats.line_comment_count++;
            }
        }

        // Recurse into all children (including unnamed ones for comments)
        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count; ++i) {
            TSNode child = ts_node_child(node, i);
            extract_comments_recursive(child, source, lang, stats);
        }
    }

    /**
     * Check if a node is an import/use declaration whose path-segment
     * identifiers should be excluded from similarity scoring.
     *
     * Import paths (e.g. `use crate::values::layout::Foo` in Rust,
     * `import io.github.kotlinmania.starlark_kotlin.values.Foo` in Kotlin)
     * introduce high-frequency namespace identifiers ("crate", "io",
     * "github", "kotlinmania") that are pure noise for cross-language
     * comparison.  Filtering them dramatically improves canonical cosine
     * similarity for faithful ports.
     */
    static bool is_import_node(const std::string& node_type) {
        return node_type == "use_declaration" ||       // Rust
               node_type == "import_header" ||          // Kotlin
               node_type == "import_list" ||             // Kotlin (import with alias)
               node_type == "import_from_statement" ||   // Python
               node_type == "import_statement" ||        // Python
               node_type == "preproc_include" ||         // C++
               node_type == "using_declaration" ||       // C++
               node_type == "import_declaration" ||      // TypeScript
               node_type == "package_header";            // Kotlin package declaration
    }

    static bool is_type_parameter_scope_node(const std::string& node_type, Language lang) {
        // Generic type parameter names are extremely weak signals in cross-language ports:
        // Rust uses single-letter params (`T`, `K`, `V`) which are already filtered by
        // the length>1 heuristic, while Kotlin often uses verbose params (`TFrozen`,
        // `KFrozen`, etc.) which would otherwise depress canonical identifier overlap.
        //
        // Skip identifiers inside these scopes for similarity scoring.
        if (lang == Language::KOTLIN) {
            return node_type == "type_parameters" ||
                node_type == "type_parameter" ||
                node_type == "type_parameter_list";
        }
        if (lang == Language::RUST) {
            return node_type == "generic_parameters";
        }
        if (lang == Language::CPP) {
            return node_type == "template_parameter_list";
        }
        if (lang == Language::TYPESCRIPT) {
            return node_type == "type_parameters";
        }
        return false;
    }

    void extract_identifiers_recursive(
        TSNode node,
        const std::string& source,
        Language lang,
        IdentifierStats& stats,
        bool skip_identifiers = false
    ) {
        auto is_kotlin_override_shim_name = [&](TSNode identifier_node, const std::string& identifier) -> bool {
            if (lang != Language::KOTLIN) return false;
            if (!(identifier == "toString" || identifier == "compareTo")) return false;

            // Walk up to the nearest Kotlin function declaration.
            TSNode cur = identifier_node;
            for (int depth = 0; depth < 8; depth++) {
                TSNode parent = ts_node_parent(cur);
                if (ts_node_is_null(parent)) break;
                std::string ptype(ts_node_type(parent));
                if (ptype == "function_declaration") {
                    TSNode func = parent;

                    // Confirm this identifier is the function name via field access when available.
                    TSNode name_node = ts_node_child_by_field_name(func, "name", 4);
                    if (!ts_node_is_null(name_node)) {
                        uint32_t ns = ts_node_start_byte(name_node);
                        uint32_t ne = ts_node_end_byte(name_node);
                        uint32_t is = ts_node_start_byte(identifier_node);
                        uint32_t ie = ts_node_end_byte(identifier_node);
                        if (!(is >= ns && ie <= ne)) {
                            return false;
                        }
                        if (ne > ns && ne <= source.length()) {
                            std::string name_text = source.substr(ns, ne - ns);
                            if (name_text != identifier) return false;
                        }
                    } else {
                        // Fallback: restrict to the function signature text region.
                        uint32_t fs = ts_node_start_byte(func);
                        uint32_t fe = ts_node_end_byte(func);
                        if (fe <= fs || fe > source.length()) return false;
                        std::string sig = source.substr(fs, fe - fs);
                        if (sig.find("fun " + identifier) == std::string::npos) return false;
                    }

                    // Check for `override` modifier in the modifiers subtree.
                    TSNode mods = ts_node_child_by_field_name(func, "modifiers", 9);
                    if (ts_node_is_null(mods)) {
                        uint32_t cc = ts_node_child_count(func);
                        for (uint32_t i = 0; i < cc; ++i) {
                            TSNode c = ts_node_child(func, i);
                            if (std::string(ts_node_type(c)) == "modifiers") {
                                mods = c;
                                break;
                            }
                        }
                    }
                    if (ts_node_is_null(mods)) return false;

                    uint32_t ms = ts_node_start_byte(mods);
                    uint32_t me = ts_node_end_byte(mods);
                    if (me > ms && me <= source.length()) {
                        std::string mtext = source.substr(ms, me - ms);
                        if (mtext.find("override") != std::string::npos) {
                            return true;
                        }
                    }
                    return false;
                }
                cur = parent;
            }
            return false;
        };

        const char* type_str = ts_node_type(node);
        std::string node_type(type_str);

        // If we enter an import/use/package node, switch to skip mode
        // so its path-segment identifiers are not counted.
        bool should_skip = skip_identifiers || is_import_node(node_type) || is_type_parameter_scope_node(node_type, lang);

        if (!should_skip) {
            // Check if this is an identifier node
            // Different languages use different node type names for identifiers
            bool is_identifier = (node_type == "identifier" ||
                                 node_type == "simple_identifier" ||
                                 node_type == "type_identifier" ||
                                 node_type == "field_identifier" ||
                                 node_type == "property_identifier");

            if (is_identifier) {
                uint32_t start = ts_node_start_byte(node);
                uint32_t end = ts_node_end_byte(node);
                if (end > start && end <= source.length()) {
                    std::string identifier = source.substr(start, end - start);
                    // Filter out very common/boilerplate identifiers
                    if (identifier.length() > 1 && identifier != "it" && identifier != "this") {
                        // Rust trait-derived behavior is frequently expressed as explicit Kotlin
                        // overrides (e.g. `toString`, `compareTo`). These identifiers do not exist
                        // as surface tokens in the Rust AST (they come from derives/traits), so
                        // treat them as low-signal only when they are *override method names*.
                        //
                        // Important: do NOT ignore normal uses/calls of these identifiers inside
                        // real logic — only the declaration name in an override.
                        if (is_kotlin_override_shim_name(node, identifier)) {
                            goto skip_add;
                        }
                        // Kotlin-specific noise identifiers (language plumbing),
                        // not strong signals of port faithfulness.
                        if (lang == Language::KOTLIN) {
                            static const std::set<std::string> kIgnore = {
                                // Result/exception plumbing
                                "Result",
                                "success",
                                "failure",
                                "exceptionOrNull",
                                "isSuccess",
                                "isFailure",
                                "runCatching",
                                "getOrThrow",
                                "getOrElse",
                                "getOrDefault",
                                "fold",
                                "map",
                                "flatMap",
                                "recover",
                                "recoverCatching",
                                "onSuccess",
                                "onFailure",
                                // Common Kotlin scoping/builder helpers
                                "let",
                                "also",
                                "apply",
                                "with",
                                "use",
                                "buildString",
                                "StringBuilder",
                                "append",
                                // Kotlin preconditions
                                "check",
                                "require",
                                "error",
                                // Interior mutability / boxing helpers
                                "getMut",
                                "asMut",
                                // Common bounds helpers
                                "coerceIn",
                                "coerceAtLeast",
                                "maxOf",
                                "minOf",
                                // Common exception types
                                "Exception",
                                "IllegalStateException",
                                "IllegalArgumentException",
                                "ArithmeticException",
                                // Annotations / reflection (porting noise)
                                "PublishedApi",
                                "JvmInline",
                                "OptIn",
                                "ExperimentalStdlibApi",
                                "ExperimentalContracts",
                                "KClass",
                                // Port-task plumbing identifiers (Kotlin-only)
                                "DEFAULT_VTABLE",
                                "vtablesByName",
                                "mapOf",
                                "SmallMap",
                                "DUMMY_INT",
                                "DUMMY_STR",
                                "DUMMY_LIST",
                                "DUMMY_DICT",
                                "DUMMY_SET",
                                "uncheckedNewTycheckDummy",
                                "INT_TYPE",
                                "BOOL_TYPE",

                                // Kotlin primitive/value types are mostly language-noise in cross-language ports.
                                "Int",
                                "UInt",
                                "Long",
                                "ULong",
                                "Short",
                                "UShort",
                                "Byte",
                                "UByte",
                                "Double",
                                "Float",
                                "Char",
                                "Boolean",
                                "String",
                                "Any",
                                "Unit",
                                // Port-specific Rust scalar stand-ins (treat like primitive noise).
                                "Usize",
                                "Isize",

                                // Common Kotlin collection interface/type noise.
                                "List",
                                "MutableList",
                                "Set",
                                "MutableSet",
                                "Map",
                                "MutableMap",
                                "Array",
                                // Tuple helpers are Kotlin-only surface forms for Rust tuples.
                                "Pair",
                                "Triple",
                                "Tuple1",
                                "Tuple4",
                                "Tuple5",
                                // Generic "Frozen" type parameters are Kotlin-only noise created by transliteration.
                                "TFrozen",
                                "KFrozen",
                                "VFrozen",
                                "AFrozen",
                                "BFrozen",
                                "CFrozen",
                                "DFrozen",
                                "EFrozen",
                                // Kotlin-only helper parameter names used to emulate Rust trait bounds in generics.
                                "freezeA",
                                "freezeB",
                                "freezeC",
                                "freezeD",
                                "freezeE",
                                "freezeKey",
                                "freezeValue",

                                // Common Kotlin collection factories.
                                "listOf",
                                "mutableListOf",
                                "setOf",
                                "mapOf",
                                "mutableMapOf",
                                "emptyList",
                                "emptySet",
                                "emptyMap",

                                // Common Kotlin iteration/collection plumbing (noisy in ports).
                                "iterator",
                                "hasNext",
                                "next",
                                "add",
                                "addAll",
                                "removeAt",
                                "remove",
                                "size",
                                "isEmpty",
                                "isNotEmpty",
                                "withIndex",
                                "indices",

                                // Common Kotlin IO helpers (noisy in commonMain ports).
                                "print",
                                "println",
                            };
                            if (kIgnore.count(identifier)) {
                                goto skip_add;
                            }
                        }
                        // Rust std/primitive type identifiers are mostly language-noise in cross-language ports.
                        // Kotlin ports frequently erase or rename these (e.g. `Vec<T>` → `MutableList<T>`),
                        // so excluding them reduces false negatives without weakening logic-shape signals.
                        if (lang == Language::RUST) {
                            static const std::set<std::string> rIgnore = {
                                "Vec",
                                "Option",
                                "String",
                                // Rust std traits/types which frequently have no direct Kotlin surface.
                                // Keep AST-shape signals, but avoid punishing faithful ports for not
                                // spelling out Rust trait machinery.
                                "Borrow",
                                "Ordering",
                                "Display",
                                "Formatter",
                                "Hash",
                                "Hasher",
                                "Deref",
                                // Memory-ordering marker used with atomics.
                                "Relaxed",
                                // Pointer/atomic helper names: Kotlin ports model these differently.
                                "ptr",
                                "null_mut",
                                "is_null",
                                "usize",
                                "isize",
                                "i8",
                                "i16",
                                "i32",
                                "i64",
                                "u8",
                                "u16",
                                "u32",
                                "u64",
                                "bool",
                            };
                            if (rIgnore.count(identifier)) {
                                goto skip_add;
                            }
                        }
                        stats.add_identifier(identifier);
                        skip_add: ;
                    }
                }
            }
        }

        // Recurse into children, propagating skip mode
        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count; ++i) {
            TSNode child = ts_node_child(node, i);
            extract_identifiers_recursive(child, source, lang, stats, should_skip);
        }
    }

    /**
     * Recursively scan function/method bodies for stub markers.
     * Returns true if ANY function body contains TODO/stub/placeholder/FIXME.
     */
    void detect_stubs_recursive(TSNode node, const std::string& source, bool& found, bool in_body) {
        if (found) return;  // short circuit

        std::string type(ts_node_type(node));

        // Are we entering a function body?
        bool entering_body = false;
        if (type == "function_body" ||           // Kotlin
            type == "block" ||                    // Rust/Kotlin/C++ function body
            type == "expression_body") {          // Kotlin `fun x() = expr`
            // Only count blocks that are direct children of function declarations
            if (in_body) {
                entering_body = false;  // already inside
            } else {
                TSNode parent = ts_node_parent(node);
                if (!ts_node_is_null(parent)) {
                    std::string ptype(ts_node_type(parent));
                    if (ptype == "function_item" ||          // Rust
                        ptype == "function_declaration" ||   // Kotlin
                        ptype == "function_definition" ||    // C++
                        ptype == "function_body") {          // Kotlin wrapper
                        entering_body = true;
                    }
                }
            }
        }

        bool now_in_body = in_body || entering_body;

        // If we're inside a function body, check comments for stub markers.
        //
        // IMPORTANT: We intentionally do NOT scan arbitrary string literals for
        // "todo"/"stub"/"not implemented" markers. The tool itself (and many real
        // codebases) legitimately prints these words in diagnostics, which would
        // create noisy false positives and make cross-language self-porting
        // comparisons unusable.
        if (now_in_body) {
            if (type == "line_comment" || type == "block_comment" ||
                type == "comment" || type == "multiline_comment") {
                uint32_t start = ts_node_start_byte(node);
                uint32_t end = ts_node_end_byte(node);
                if (end > start && end <= source.length()) {
                    std::string text = source.substr(start, end - start);
                    if (comment_has_stub_markers(text)) {
                        found = true;
                        return;
                    }
                }
            }
        }

        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count && !found; ++i) {
            TSNode child = ts_node_child(node, i);
            detect_stubs_recursive(child, source, found, now_in_body);
        }
    }

    TreePtr convert_node(TSNode node, const std::string& source, Language lang) {
        const char* type_str = ts_node_type(node);
        auto is_comment_node = [](const char* t) -> bool {
            // Exclude comment nodes from AST similarity metrics.
            // Documentation is measured separately via extract_comments().
            // Keeping comments in the AST systematically penalizes faithful ports
            // that reorganize doc blocks or change doc-comment styles.
            return std::strcmp(t, "line_comment") == 0 ||
                std::strcmp(t, "block_comment") == 0 ||
                std::strcmp(t, "comment") == 0 ||
                std::strcmp(t, "multiline_comment") == 0;
        };

        // Normalize node type
        NodeType normalized_type = NodeType::UNKNOWN;
        switch (lang) {
            case Language::RUST: normalized_type = rust_node_to_type(type_str); break;
            case Language::KOTLIN: normalized_type = kotlin_node_to_type(type_str); break;
            case Language::CPP: normalized_type = cpp_node_to_type(type_str); break;
            case Language::PYTHON: normalized_type = python_node_to_type(type_str); break;
            case Language::TYPESCRIPT: normalized_type = typescript_node_to_type(type_str); break;
        }

        // Special-case: Rust `impl Trait for Type { ... }` blocks.
        //
        // In Rust, `impl_item` is used for both inherent impls (`impl Type {}`) and trait impls
        // (`impl Trait for Type {}`). In Kotlin transliterations, trait impls frequently become
        // top-level or extension functions rather than a nested "block-like" container.
        //
        // Mapping all `impl_item` nodes to BLOCK systematically inflates Rust BLOCK counts and
        // causes false-negative similarity on faithful ports (notably `values/trace.rs`).
        //
        // Heuristic: if the impl header contains " for " before the opening '{', treat the
        // `impl_item` (and its immediate `declaration_list` body) as OTHER instead of BLOCK.
        auto is_rust_trait_impl_item = [&](TSNode impl_node) -> bool {
            if (lang != Language::RUST || ts_node_is_null(impl_node)) return false;
            if (std::string(ts_node_type(impl_node)) != "impl_item") return false;

            uint32_t start = ts_node_start_byte(impl_node);
            uint32_t end = ts_node_end_byte(impl_node);
            if (end <= start || start >= source.size()) return false;
            end = std::min<uint32_t>(end, static_cast<uint32_t>(source.size()));

            // Only scan a small prefix; sufficient to include the header up to '{'.
            size_t len = std::min<size_t>(static_cast<size_t>(end - start), 512);
            std::string snippet = source.substr(start, len);
            size_t brace = snippet.find('{');
            if (brace != std::string::npos) {
                snippet.resize(brace);
            }

            return snippet.find(" for ") != std::string::npos;
        };

        if (lang == Language::RUST) {
            std::string t(type_str);
            if (t == "impl_item" && is_rust_trait_impl_item(node)) {
                // Treat as PACKAGE so it will be flattened away in similarity scoring.
                normalized_type = NodeType::PACKAGE;
            } else if (t == "declaration_list") {
                TSNode parent = ts_node_parent(node);
                if (!ts_node_is_null(parent) && is_rust_trait_impl_item(parent)) {
                    // Treat as PACKAGE so it will be flattened away in similarity scoring.
                    normalized_type = NodeType::PACKAGE;
                }
            }
        }

        // Special-case: Kotlin `object` declarations used as Rust `mod` markers.
        //
        // Many Rust module root files are just `mod foo;` declarations. In Kotlin, the
        // closest line-by-line transliteration is often a set of empty `object` markers
        // (sometimes with backticked names). These carry module structure but no runtime
        // semantics.
        //
        // Treat such empty `object_declaration` nodes as PACKAGE-like to avoid overweighting
        // them as CLASS nodes (which hurts similarity scoring for module root files).
        if (lang == Language::KOTLIN && std::string(type_str) == "object_declaration") {
            bool has_class_body = false;
            bool body_has_named_children = false;

            uint32_t oc_child_count = ts_node_child_count(node);
            for (uint32_t i = 0; i < oc_child_count; ++i) {
                TSNode child = ts_node_child(node, i);
                if (!ts_node_is_named(child)) {
                    continue;
                }

                std::string child_type(ts_node_type(child));
                if (child_type == "class_body") {
                    has_class_body = true;

                    uint32_t body_child_count = ts_node_child_count(child);
                    for (uint32_t j = 0; j < body_child_count; ++j) {
                        TSNode body_child = ts_node_child(child, j);
                        if (ts_node_is_named(body_child)) {
                            body_has_named_children = true;
                            break;
                        }
                    }
                }

                if (body_has_named_children) {
                    break;
                }
            }

            // If there's no class body at all (e.g. `object Foo`) OR the class body is empty
            // (e.g. `object Foo {}`), we treat it as a module marker.
            if (!has_class_body || !body_has_named_children) {
                normalized_type = NodeType::PACKAGE;
            }
        }

        // Track unmapped node types for diagnostics
        if (normalized_type == NodeType::UNKNOWN) {
            unmapped_node_types_[type_str]++;
        }

        auto tree_node = std::make_shared<Tree>(
            static_cast<int>(normalized_type), type_str);

        // For leaf nodes, store the index
        uint32_t child_count = ts_node_child_count(node);
        if (child_count == 0) {
            // Get the actual text for debugging
            uint32_t start = ts_node_start_byte(node);
            uint32_t end = ts_node_end_byte(node);
            if (end > start && end <= source.length()) {
                tree_node->label = source.substr(start, end - start);
            }
        }

        // Recursively process children
        for (uint32_t i = 0; i < child_count; ++i) {
            TSNode child = ts_node_child(node, i);

            if (ts_node_is_named(child)) {
                // Named nodes are always included
                const char* child_type = ts_node_type(child);
                if (is_comment_node(child_type)) {
                    continue;
                }
                tree_node->add_child(convert_node(child, source, lang));
            } else {
                // Capture semantically significant unnamed nodes (operators)
                const char* child_type = ts_node_type(child);
                std::string op_str(child_type);

                // Arithmetic operators
                if (op_str == "+" || op_str == "-" || op_str == "*" ||
                    op_str == "/" || op_str == "%" || op_str == "**") {
                    auto op_node = std::make_shared<Tree>(
                        static_cast<int>(NodeType::ARITHMETIC_OP), child_type);
                    tree_node->add_child(op_node);
                }
                // Comparison operators
                else if (op_str == "==" || op_str == "!=" || op_str == "<" ||
                         op_str == ">" || op_str == "<=" || op_str == ">=" ||
                         op_str == "===" || op_str == "!==") {
                    auto op_node = std::make_shared<Tree>(
                        static_cast<int>(NodeType::COMPARISON_OP), child_type);
                    tree_node->add_child(op_node);
                }
                // Logical operators
                else if (op_str == "&&" || op_str == "||" || op_str == "!") {
                    auto op_node = std::make_shared<Tree>(
                        static_cast<int>(NodeType::LOGICAL_OP), child_type);
                    tree_node->add_child(op_node);
                }
                // Bitwise operators
                else if (op_str == "&" || op_str == "|" || op_str == "^" ||
                         op_str == "~" || op_str == "<<" || op_str == ">>") {
                    auto op_node = std::make_shared<Tree>(
                        static_cast<int>(NodeType::BITWISE_OP), child_type);
                    tree_node->add_child(op_node);
                }
                // Assignment operators
                else if (op_str == "=" || op_str == "+=" || op_str == "-=" ||
                         op_str == "*=" || op_str == "/=" || op_str == "%=" ||
                         op_str == "&=" || op_str == "|=" || op_str == "^=" ||
                         op_str == "<<=" || op_str == ">>=") {
                    auto op_node = std::make_shared<Tree>(
                        static_cast<int>(NodeType::ASSIGNMENT_OP), child_type);
                    tree_node->add_child(op_node);
                }
            }
        }

        return tree_node;
    }

    bool is_function_node(const std::string& type_s, Language lang) const {
        return (lang == Language::RUST && type_s == "function_item") ||
            (lang == Language::KOTLIN && type_s == "function_declaration") ||
            (lang == Language::CPP &&
             (type_s == "function_definition" || type_s == "function_declarator")) ||
            (lang == Language::PYTHON && type_s == "function_definition") ||
            (lang == Language::TYPESCRIPT &&
             (type_s == "function_declaration" || type_s == "method_definition" ||
              type_s == "function_expression" || type_s == "arrow_function"));
    }

    /**
     * Check if a Rust node has a #[test] attribute or a #[cfg(test)] attribute.
     * Works for both function_item and mod_item nodes.
     *
     * In tree-sitter-rust, attributes appear as siblings BEFORE the function:
     *   (attribute_item (attribute (path) ...))   // for outer #[...]
     *   (function_item ...)
     *
     * Or for #[cfg(test)]:
     *   (attribute_item
     *     (attribute
     *       (path)                               // "cfg"
     *       (token_tree "(" "test" ")")))
     */
    bool has_test_attribute(TSNode node, const std::string& source) const {
        // Look at preceding siblings for attribute_item nodes
        TSNode parent = ts_node_parent(node);
        if (ts_node_is_null(parent)) return false;

        uint32_t child_count = ts_node_child_count(parent);
        // Find our index among siblings
        uint32_t our_index = UINT32_MAX;
        for (uint32_t i = 0; i < child_count; ++i) {
            TSNode child = ts_node_child(parent, i);
            if (ts_node_start_byte(child) == ts_node_start_byte(node) &&
                ts_node_end_byte(child) == ts_node_end_byte(node)) {
                our_index = i;
                break;
            }
        }

        if (our_index == UINT32_MAX || our_index == 0) return false;

        // Scan backwards through preceding siblings for attribute_item
        for (uint32_t i = our_index; i > 0; --i) {
            TSNode sibling = ts_node_child(parent, i - 1);
            std::string sib_type(ts_node_type(sibling));

            if (sib_type != "attribute_item") break;  // stop at non-attribute

            // Extract the attribute text and check for #[test] or #[cfg(test)]
            uint32_t start = ts_node_start_byte(sibling);
            uint32_t end = ts_node_end_byte(sibling);
            if (end > start && end <= source.length()) {
                std::string attr_text = source.substr(start, end - start);
                // #[test]
                if (attr_text.find("#[test]") != std::string::npos) return true;
                // #[cfg(test)]
                if (attr_text.find("cfg(test)") != std::string::npos) return true;
                if (attr_text.find("cfg( test )") != std::string::npos) return true;
            }
        }

        return false;
    }

    /**
     * Check if a Kotlin function node has a @Test annotation (kotlin.test or JUnit).
     * Kotlin annotations appear as `modifiers` children or preceding `annotation` nodes.
     */
    bool has_kotlin_test_annotation(TSNode node, const std::string& source) const {
        // Scan the function node itself for a `modifiers` child containing @Test.
        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count; ++i) {
            TSNode child = ts_node_child(node, i);
            std::string t(ts_node_type(child));
            if (t == "modifiers" || t == "annotation" ||
                t == "user_type" /* legacy */) {
                uint32_t start = ts_node_start_byte(child);
                uint32_t end = ts_node_end_byte(child);
                if (end > start && end <= source.length()) {
                    std::string text = source.substr(start, end - start);
                    // Match @Test as a whole token (not e.g. @TestConfig).
                    size_t pos = 0;
                    while ((pos = text.find("@Test", pos)) != std::string::npos) {
                        char next = (pos + 5 < text.size()) ? text[pos + 5] : '\0';
                        // Allow @Test, @Test(...), @Test\n, @Test followed by space.
                        if (next == '\0' || next == '\n' || next == ' ' ||
                            next == '\t' || next == '(' || next == '\r') {
                            return true;
                        }
                        pos += 5;
                    }
                }
            }
        }
        return false;
    }

    /**
     * Check if a node is inside a #[cfg(test)] mod block.
     * Walks up the tree looking for mod_item ancestors with #[cfg(test)].
     */
    bool is_inside_cfg_test_mod(TSNode node, const std::string& source) const {
        TSNode current = ts_node_parent(node);
        while (!ts_node_is_null(current)) {
            std::string type_s(ts_node_type(current));
            if (type_s == "mod_item") {
                if (has_test_attribute(current, source)) {
                    return true;
                }
            }
            current = ts_node_parent(current);
        }
        return false;
    }

    std::string extract_function_name(TSNode node, Language lang, const std::string& source) const {
        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count; ++i) {
            TSNode child = ts_node_child(node, i);
            const char* child_type = ts_node_type(child);

            std::string ct(child_type);
            if ((lang == Language::RUST && ct == "identifier") ||
                (lang == Language::KOTLIN && ct == "simple_identifier") ||
                (lang == Language::CPP &&
                    (ct == "identifier" || ct == "field_identifier")) ||
                (lang == Language::PYTHON && ct == "identifier") ||
                (lang == Language::TYPESCRIPT &&
                    (ct == "identifier" || ct == "property_identifier"))) {
                uint32_t start = ts_node_start_byte(child);
                uint32_t end = ts_node_end_byte(child);
                if (end > start && end <= source.length()) {
                    std::string name = source.substr(start, end - start);
                    // Canonicalize TypeScript constructor to Python __init__ for parity
                    if (lang == Language::TYPESCRIPT && name == "constructor") {
                        return "__init__";
                    }
                    return name;
                }
            }
        }

        // Special-case: TypeScript arrow functions and function expressions
        // often get their names from the parent (e.g., const name = () => { ... })
        if (lang == Language::TYPESCRIPT) {
            TSNode parent = ts_node_parent(node);
            if (!ts_node_is_null(parent)) {
                std::string pt(ts_node_type(parent));
                if (pt == "variable_declarator" || pt == "property_definition") {
                    TSNode name_node = ts_node_child_by_field_name(parent, "name", 4);
                    if (!ts_node_is_null(name_node)) {
                        uint32_t start = ts_node_start_byte(name_node);
                        uint32_t end = ts_node_end_byte(name_node);
                        if (end > start && end <= source.length()) {
                            return source.substr(start, end - start);
                        }
                    }
                }
            }
        }

        return "<anonymous>";
    }

    TSNode extract_function_body_node(TSNode function_node, Language lang) const {
        TSNode body = ts_node_child_by_field_name(function_node, "body", 4);
        if (!ts_node_is_null(body)) {
            // Kotlin `function_body` is a wrapper node around either a `block` (`{ ... }`)
            // or an `expression_body` (`= expr`). For cross-language function-body comparison,
            // unwrap it to the inner block/expression so Rust `block` bodies match more closely.
            if (lang == Language::KOTLIN && std::string(ts_node_type(body)) == "function_body") {
                uint32_t body_child_count = ts_node_child_count(body);
                for (uint32_t i = 0; i < body_child_count; ++i) {
                    TSNode child = ts_node_child(body, i);
                    if (!ts_node_is_named(child)) continue;
                    std::string ct(ts_node_type(child));
                    if (ct == "block" || ct == "expression_body") {
                        return child;
                    }
                }
            }
            return body;
        }

        const std::vector<std::string> body_types = {
            "function_body",
            "expression_body",
            "body",
            "block",
            "statement_block",
            "compound_statement"
        };

        const uint32_t child_count = ts_node_child_count(function_node);
        for (uint32_t i = 0; i < child_count; ++i) {
            TSNode child = ts_node_child(function_node, i);
            std::string child_type(ts_node_type(child));
            for (const auto& bt : body_types) {
                if (child_type == bt) {
                    if (lang == Language::KOTLIN && child_type == "function_body") {
                        uint32_t body_child_count = ts_node_child_count(child);
                        for (uint32_t j = 0; j < body_child_count; ++j) {
                            TSNode sub = ts_node_child(child, j);
                            if (!ts_node_is_named(sub)) continue;
                            std::string st(ts_node_type(sub));
                            if (st == "block" || st == "expression_body") {
                                return sub;
                            }
                        }
                    }
                    return child;
                }
            }
        }

        // Kotlin interface/abstract function declarations have no body.
        // Returning the declaration node here would incorrectly count the whole declaration as a "function body"
        // and would heavily penalize faithful Rust→Kotlin ports where Rust trait method signatures don't count as
        // function bodies.
        if (lang == Language::KOTLIN) {
            return TSNode{};
        }

        return function_node;
    }

    bool is_parameter_container_node(const std::string& type_s, Language lang) const {
        if (lang == Language::RUST) {
            return type_s == "parameters";
        }
        if (lang == Language::KOTLIN) {
            return type_s == "function_value_parameters";
        }
        if (lang == Language::CPP) {
            return type_s == "parameter_list";
        }
        if (lang == Language::PYTHON) {
            return type_s == "parameters";
        }
        if (lang == Language::TYPESCRIPT) {
            return type_s == "formal_parameters" || type_s == "parameters";
        }
        return false;
    }

    bool node_contains_byte_range(TSNode node, uint32_t start, uint32_t end) const {
        if (ts_node_is_null(node)) return false;
        return ts_node_start_byte(node) <= start && ts_node_end_byte(node) >= end;
    }

    void collect_function_parameter_nodes(
            TSNode node,
            TSNode body_node,
            Language lang,
            std::vector<TSNode>& out) const {
        if (ts_node_is_null(node)) return;

        if (!ts_node_is_null(body_node) &&
            node_contains_byte_range(node, ts_node_start_byte(body_node), ts_node_end_byte(body_node)) &&
            !ts_node_eq(node, body_node)) {
            // Keep walking until we reach the body itself, then stop before
            // collecting nested/local function parameters from the body region.
        } else if (!ts_node_is_null(body_node) && ts_node_eq(node, body_node)) {
            return;
        }

        std::string type_s(ts_node_type(node));
        if (is_parameter_container_node(type_s, lang)) {
            out.push_back(node);
            return;
        }

        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count; ++i) {
            TSNode child = ts_node_child(node, i);
            collect_function_parameter_nodes(child, body_node, lang, out);
        }
    }

    std::vector<TSNode> extract_function_parameter_nodes(
            TSNode function_node,
            TSNode body_node,
            Language lang) const {
        std::vector<TSNode> params;

        TSNode by_field = ts_node_child_by_field_name(function_node, "parameters", 10);
        if (!ts_node_is_null(by_field)) {
            params.push_back(by_field);
            return params;
        }

        collect_function_parameter_nodes(function_node, body_node, lang, params);
        return params;
    }

    TreePtr make_function_comparison_tree(
            TSNode function_node,
            TSNode body_node,
            const std::string& source,
            Language lang) {
        auto root = std::make_shared<Tree>(
            static_cast<int>(NodeType::FUNCTION),
            "function_parameters_and_body");

        auto params = extract_function_parameter_nodes(function_node, body_node, lang);
        for (const auto& param_node : params) {
            root->add_child(convert_node(param_node, source, lang));
        }

        if (!ts_node_is_null(body_node)) {
            root->add_child(convert_node(body_node, source, lang));
        }

        return root;
    }

    IdentifierStats extract_function_comparison_identifiers(
            TSNode function_node,
            TSNode body_node,
            const std::string& source,
            Language lang) {
        IdentifierStats ids;
        auto params = extract_function_parameter_nodes(function_node, body_node, lang);
        for (const auto& param_node : params) {
            extract_identifiers_recursive(param_node, source, lang, ids);
        }
        if (!ts_node_is_null(body_node)) {
            extract_identifiers_recursive(body_node, source, lang, ids);
        }
        return ids;
    }

    bool has_stub_markers_in_node(TSNode node, const std::string& source, Language lang) const {
        if (ts_node_is_null(node) || source.empty()) return false;

        const std::vector<std::string> marker_nodes = {
            "line_comment",
            "block_comment",
            "comment",
            "multiline_comment"
        };

        std::vector<TSNode> stack;
        stack.push_back(node);

        while (!stack.empty()) {
            TSNode current = stack.back();
            stack.pop_back();

            std::string current_type(ts_node_type(current));

            // Strong stub constructs that don't rely on string/comment markers.
            // Keep these conservative to avoid false positives.
            if (lang == Language::PYTHON) {
                if (current_type == "pass_statement" || current_type == "ellipsis") {
                    return true;
                }
                if (current_type == "raise_statement") {
                    uint32_t start = ts_node_start_byte(current);
                    uint32_t end = ts_node_end_byte(current);
                    if (end > start && end <= source.length()) {
                        std::string text = source.substr(start, end - start);
                        if (text.find("NotImplementedError") != std::string::npos ||
                            text.find("notimplementederror") != std::string::npos) {
                            return true;
                        }
                    }
                }
            } else if (lang == Language::RUST) {
                if (current_type == "macro_invocation") {
                    uint32_t start = ts_node_start_byte(current);
                    uint32_t end = ts_node_end_byte(current);
                    if (end > start && end <= source.length()) {
                        // Macro names are at the beginning; avoid copying huge token trees.
                        size_t len = std::min<size_t>(end - start, 96);
                        std::string text = source.substr(start, len);
                        if (text_has_stub_markers(text) ||
                            text.find("todo!") != std::string::npos ||
                            text.find("unimplemented!") != std::string::npos ||
                            text.find("unreachable!") != std::string::npos) {
                            return true;
                        }
                    }
                }
            } else if (lang == Language::KOTLIN || lang == Language::TYPESCRIPT) {
                if (current_type == "simple_identifier" ||
                    current_type == "type_identifier" ||
                    current_type == "identifier") {
                    uint32_t start = ts_node_start_byte(current);
                    uint32_t end = ts_node_end_byte(current);
                    if (end > start && end <= source.length()) {
                        std::string text = source.substr(start, end - start);
                        if (text == "TODO" || text == "NotImplementedError" ||
                            (lang == Language::TYPESCRIPT && text == "undefined")) {
                            return true;
                        }
                    }
                }

                // TypeScript: throw new Error("unimplemented")
                if (lang == Language::TYPESCRIPT && current_type == "throw_statement") {
                    uint32_t start = ts_node_start_byte(current);
                    uint32_t end = ts_node_end_byte(current);
                    if (end > start && end <= source.length()) {
                        std::string text = source.substr(start, end - start);
                        if (text.find("not implemented") != std::string::npos ||
                            text.find("unimplemented") != std::string::npos ||
                            text.find("TODO") != std::string::npos) {
                            return true;
                        }
                    }
                }

                // TypeScript: console.warn("TODO"...)
                if (lang == Language::TYPESCRIPT && current_type == "call_expression") {
                    uint32_t start = ts_node_start_byte(current);
                    uint32_t end = ts_node_end_byte(current);
                    if (end > start && end <= source.length()) {
                        std::string text = source.substr(start, end - start);
                        if (text.find("console.warn") != std::string::npos ||
                            text.find("console.error") != std::string::npos) {
                            if (text.find("TODO") != std::string::npos ||
                                text.find("unimplemented") != std::string::npos) {
                                return true;
                            }
                        }
                    }
                }
            }

            bool is_marker_type = false;
            for (const auto& marker_type : marker_nodes) {
                if (current_type == marker_type) {
                    is_marker_type = true;
                    break;
                }
            }

            if (is_marker_type) {
                uint32_t start = ts_node_start_byte(current);
                uint32_t end = ts_node_end_byte(current);
                if (end > start && end <= source.length()) {
                    std::string text = source.substr(start, end - start);
                    if (comment_has_stub_markers(text)) return true;
                }
            }

            const uint32_t child_count = ts_node_child_count(current);
            for (uint32_t i = 0; i < child_count; ++i) {
                TSNode child = ts_node_child(current, i);
                if (!ts_node_is_null(child)) {
                    stack.push_back(child);
                }
            }
        }

        return false;
    }

    void extract_function_infos_recursive(
            TSNode node,
            const std::string& source,
            Language lang,
            std::vector<FunctionInfo>& functions) {

        const char* type_str = ts_node_type(node);
        std::string type_s(type_str);
        if (is_function_node(type_s, lang)) {
            TSNode body_node = extract_function_body_node(node, lang);
            if (lang == Language::KOTLIN && ts_node_is_null(body_node)) {
                // Skip Kotlin declarations without bodies (interface/abstract methods).
                // Rust trait method signatures similarly do not contribute function bodies for similarity scoring.
                return;
            }
            std::string func_name = extract_function_name(node, lang, source);

            FunctionInfo info;
            info.name = func_name;
            info.body_tree = make_function_comparison_tree(node, body_node, source, lang);
            info.identifiers = extract_function_comparison_identifiers(node, body_node, source, lang);
            info.has_stub_markers = has_stub_markers_in_node(body_node, source, lang);
            TSPoint start = ts_node_start_point(node);
            TSPoint end = ts_node_end_point(node);
            info.start_line = static_cast<int>(start.row) + 1;
            info.end_line = static_cast<int>(end.row) + 1;
            info.line_count = info.end_line >= info.start_line
                ? (info.end_line - info.start_line + 1)
                : 0;
            if (!ts_node_is_null(body_node)) {
                TSPoint body_start = ts_node_start_point(body_node);
                TSPoint body_end = ts_node_end_point(body_node);
                int body_start_line = static_cast<int>(body_start.row) + 1;
                int body_end_line = static_cast<int>(body_end.row) + 1;
                info.body_line_count = body_end_line >= body_start_line
                    ? (body_end_line - body_start_line + 1)
                    : 0;
            }

            // Tag Rust test functions: #[test] attribute or inside #[cfg(test)] mod
            if (lang == Language::RUST) {
                info.is_test = has_test_attribute(node, source) ||
                               is_inside_cfg_test_mod(node, source);
            } else if (lang == Language::KOTLIN) {
                // Kotlin @Test annotation (kotlin.test or JUnit).
                info.is_test = has_kotlin_test_annotation(node, source);
            }

            functions.push_back(info);
        }

        // Recurse into children
        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count; ++i) {
            TSNode child = ts_node_child(node, i);
            extract_function_infos_recursive(child, source, lang, functions);
        }
    }
};

} // namespace ast_distance
