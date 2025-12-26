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

// External declarations for tree-sitter language functions
extern "C" {
    const TSLanguage* tree_sitter_rust();
    const TSLanguage* tree_sitter_kotlin();
    const TSLanguage* tree_sitter_cpp();
}

namespace ast_distance {

enum class Language {
    RUST,
    KOTLIN,
    CPP
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
     * Parse a source file into a normalized AST.
     */
    TreePtr parse_file(const std::string& filepath, Language lang) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filepath);
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return parse_string(buffer.str(), lang);
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
     * Extract comment statistics from a file.
     */
    CommentStats extract_comments_from_file(const std::string& filepath, Language lang) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return CommentStats{};
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return extract_comments(buffer.str(), lang);
    }

    /**
     * Parse and extract only function bodies for comparison.
     */
    std::vector<std::pair<std::string, TreePtr>> extract_functions(
            const std::string& source, Language lang) {
        std::vector<std::pair<std::string, TreePtr>> functions;

        // Set language
        const TSLanguage* ts_lang;
        switch (lang) {
            case Language::RUST: ts_lang = tree_sitter_rust(); break;
            case Language::KOTLIN: ts_lang = tree_sitter_kotlin(); break;
            case Language::CPP: ts_lang = tree_sitter_cpp(); break;
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
        extract_functions_recursive(root, source, lang, functions);

        ts_tree_delete(ts_tree);
        return functions;
    }

private:
    TSParser* parser_;

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

    TreePtr convert_node(TSNode node, const std::string& source, Language lang) {
        const char* type_str = ts_node_type(node);

        // Normalize node type
        NodeType normalized_type;
        switch (lang) {
            case Language::RUST: normalized_type = rust_node_to_type(type_str); break;
            case Language::KOTLIN: normalized_type = kotlin_node_to_type(type_str); break;
            case Language::CPP: normalized_type = cpp_node_to_type(type_str); break;
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
            // Skip unnamed nodes (syntax tokens like punctuation)
            if (ts_node_is_named(child)) {
                tree_node->add_child(convert_node(child, source, lang));
            }
        }

        return tree_node;
    }

    void extract_functions_recursive(
            TSNode node,
            const std::string& source,
            Language lang,
            std::vector<std::pair<std::string, TreePtr>>& functions) {

        const char* type_str = ts_node_type(node);
        bool is_function = false;
        std::string func_name;

        // Check if this is a function declaration
        std::string type_s(type_str);
        if (lang == Language::RUST) {
            is_function = (type_s == "function_item");
        } else if (lang == Language::KOTLIN) {
            is_function = (type_s == "function_declaration");
        } else if (lang == Language::CPP) {
            is_function = (type_s == "function_definition" || type_s == "function_declarator");
        }

        if (is_function) {
            // Extract function name
            uint32_t child_count = ts_node_child_count(node);
            for (uint32_t i = 0; i < child_count; ++i) {
                TSNode child = ts_node_child(node, i);
                const char* child_type = ts_node_type(child);

                std::string ct(child_type);
                if ((lang == Language::RUST && ct == "identifier") ||
                    (lang == Language::KOTLIN && ct == "simple_identifier") ||
                    (lang == Language::CPP && (ct == "identifier" || ct == "field_identifier"))) {
                    uint32_t start = ts_node_start_byte(child);
                    uint32_t end = ts_node_end_byte(child);
                    if (end > start && end <= source.length()) {
                        func_name = source.substr(start, end - start);
                        break;
                    }
                }
            }

            // Convert the function AST
            TreePtr func_tree = convert_node(node, source, lang);
            functions.emplace_back(func_name, func_tree);
        }

        // Recurse into children
        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count; ++i) {
            TSNode child = ts_node_child(node, i);
            extract_functions_recursive(child, source, lang, functions);
        }
    }
};

} // namespace ast_distance
