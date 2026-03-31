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
#include <algorithm>
#include <cmath>

// External declarations for tree-sitter language functions
extern "C" {
    const TSLanguage* tree_sitter_rust();
    const TSLanguage* tree_sitter_kotlin();
    const TSLanguage* tree_sitter_cpp();
    const TSLanguage* tree_sitter_python();
}

namespace ast_distance {

enum class Language {
    RUST,
    KOTLIN,
    CPP,
    PYTHON
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
     * Also normalizes cross-language equivalents:
     *   self/this → "this", Option/nullable → "option",
     *   Vec/List/MutableList → "list", etc.
     */
    static std::string canonicalize(const std::string& name) {
        // First: lowercase + strip underscores
        std::string result;
        result.reserve(name.size());
        for (char c : name) {
            if (c != '_') {
                result += static_cast<char>(
                    std::tolower(static_cast<unsigned char>(c)));
            }
        }

        // Cross-language equivalents (applied after lowering)
        static const std::vector<std::pair<std::string, std::string>> equivalents = {
            // Keywords
            {"self", "this"},
            {"crate", ""},          // Rust path component, no Kotlin equivalent
            {"super", "super"},
            // Collections
            {"vec", "list"},
            {"mutablelist", "list"},
            {"arraylist", "list"},
            {"hashmap", "map"},
            {"mutablemap", "map"},
            {"hashset", "set"},
            {"mutableset", "set"},
            {"btreemap", "map"},
            {"btreeset", "set"},
            // Types
            {"option", "nullable"},
            {"some", "notnull"},
            {"none", "null"},
            {"box", "boxed"},
            {"arc", "arc"},
            {"string", "string"},
            {"str", "string"},
            {"i32", "int"},
            {"i64", "long"},
            {"u32", "uint"},
            {"u64", "ulong"},
            {"usize", "uint"},
            {"isize", "int"},
            {"f32", "float"},
            {"f64", "double"},
            {"bool", "boolean"},
            // Error handling
            {"result", "result"},
            {"err", "error"},
            {"ok", "success"},
            // Rust trait methods -> Kotlin equivalents
            {"fmt", "tostring"},          // Display::fmt -> toString
            {"eq", "equals"},             // PartialEq::eq -> equals
            {"partialeq", "equals"},
            {"cmp", "compareto"},         // Ord::cmp -> compareTo
            {"partialcmp", "compareto"},  // PartialOrd::partial_cmp -> compareTo
            {"hash", "hashcode"},         // Hash::hash -> hashCode
            {"clone", "copy"},            // Clone::clone -> copy (data class)
            {"default", "invoke"},        // Default::default -> companion invoke
            {"fromstr", "parse"},         // FromStr -> parse
            {"intoiter", "iterator"},     // IntoIterator::into_iter -> iterator
            {"intoiterator", "iterator"},
            {"next", "next"},             // Iterator::next (same name)
            {"serialize", "serialize"},   // serde (same name)
            {"deserialize", "deserialize"},
            {"deref", "get"},             // Deref::deref -> get/value
            {"drop", "close"},            // Drop::drop -> close/Closeable
            {"freeze", "freeze"},         // project-specific (same name)
            {"trace", "trace"},           // project-specific (same name)
            // Common prefixes
            {"fn", "fun"},
            {"impl", "class"},
            {"pub", "public"},
            {"mut", "var"},
            {"let", "val"},
        };

        for (const auto& [from, to] : equivalents) {
            if (result == from) {
                return to;
            }
        }

        return result;
    }

    void add_identifier(const std::string& name) {
        if (!name.empty()) {
            identifier_freq[name]++;
            canonical_freq[canonicalize(name)]++;
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
 * The AST is kept as the function body (not the whole declaration)
 * so that stub checks and identifier matching are aligned with behavior.
 */
struct FunctionInfo {
    std::string name;
    TreePtr body_tree;
    IdentifierStats identifiers;
    bool has_stub_markers = false;
    bool is_test = false;  // true if #[test] or inside #[cfg(test)] mod
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
        extract_identifiers_recursive(root, source, stats);

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
     */
    bool has_stub_bodies(const std::string& source, Language lang) {
        const TSLanguage* ts_lang;
        switch (lang) {
            case Language::RUST: ts_lang = tree_sitter_rust(); break;
            case Language::KOTLIN: ts_lang = tree_sitter_kotlin(); break;
            case Language::CPP: ts_lang = tree_sitter_cpp(); break;
            case Language::PYTHON: ts_lang = tree_sitter_python(); break;
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
            if (!file.is_open()) continue;
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

            // Check if it's a doc comment
            // Kotlin: /** ... */ or lines starting with *
            // C++: /** ... */ or ///
            // Rust: /// or //! or /** */
            if (lang == Language::KOTLIN) {
                is_doc_comment = (text.find("/**") == 0) ||
                                 (text.find("/*") == 0 && text.find("*") != std::string::npos);
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
               node_type == "package_header";            // Kotlin package declaration
    }

    void extract_identifiers_recursive(TSNode node, const std::string& source,
                                        IdentifierStats& stats, bool skip_identifiers = false) {
        const char* type_str = ts_node_type(node);
        std::string node_type(type_str);

        // If we enter an import/use/package node, switch to skip mode
        // so its path-segment identifiers are not counted.
        bool should_skip = skip_identifiers || is_import_node(node_type);

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
                        stats.add_identifier(identifier);
                    }
                }
            }
        }

        // Recurse into children, propagating skip mode
        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count; ++i) {
            TSNode child = ts_node_child(node, i);
            extract_identifiers_recursive(child, source, stats, should_skip);
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

        // Normalize node type
        NodeType normalized_type = NodeType::UNKNOWN;
        switch (lang) {
            case Language::RUST: normalized_type = rust_node_to_type(type_str); break;
            case Language::KOTLIN: normalized_type = kotlin_node_to_type(type_str); break;
            case Language::CPP: normalized_type = cpp_node_to_type(type_str); break;
            case Language::PYTHON: normalized_type = python_node_to_type(type_str); break;
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
            (lang == Language::PYTHON && type_s == "function_definition");
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
                (lang == Language::PYTHON && ct == "identifier")) {
                uint32_t start = ts_node_start_byte(child);
                uint32_t end = ts_node_end_byte(child);
                if (end > start && end <= source.length()) {
                    return source.substr(start, end - start);
                }
            }
        }

        return "<anonymous>";
    }

    TSNode extract_function_body_node(TSNode function_node, Language lang) const {
        (void)lang;

        TSNode body = ts_node_child_by_field_name(function_node, "body", 4);
        if (!ts_node_is_null(body)) {
            return body;
        }

        const std::vector<std::string> body_types = {
            "function_body",
            "expression_body",
            "body",
            "block",
            "compound_statement"
        };

        const uint32_t child_count = ts_node_child_count(function_node);
        for (uint32_t i = 0; i < child_count; ++i) {
            TSNode child = ts_node_child(function_node, i);
            std::string child_type(ts_node_type(child));
            for (const auto& bt : body_types) {
                if (child_type == bt) {
                    return child;
                }
            }
        }

        return function_node;
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
            } else if (lang == Language::KOTLIN) {
                if (current_type == "simple_identifier" ||
                    current_type == "type_identifier" ||
                    current_type == "identifier") {
                    uint32_t start = ts_node_start_byte(current);
                    uint32_t end = ts_node_end_byte(current);
                    if (end > start && end <= source.length()) {
                        std::string text = source.substr(start, end - start);
                        if (text == "TODO" || text == "NotImplementedError") {
                            return true;
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
            std::string func_name = extract_function_name(node, lang, source);

            IdentifierStats ids;
            extract_identifiers_recursive(body_node, source, ids);

            FunctionInfo info;
            info.name = func_name;
            info.body_tree = convert_node(body_node, source, lang);
            info.identifiers = ids;
            info.has_stub_markers = has_stub_markers_in_node(body_node, source, lang);

            // Tag Rust test functions: #[test] attribute or inside #[cfg(test)] mod
            if (lang == Language::RUST) {
                info.is_test = has_test_attribute(node, source) ||
                               is_inside_cfg_test_mod(node, source);
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
