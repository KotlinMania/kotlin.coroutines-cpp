#pragma once

#include <tree_sitter/api.h>
#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <sstream>
#include <regex>

// External declarations for tree-sitter language functions
extern "C" {
    const TSLanguage* tree_sitter_rust();
    const TSLanguage* tree_sitter_kotlin();
    const TSLanguage* tree_sitter_cpp();
}

namespace ast_distance {

/**
 * Represents a package/namespace declaration.
 */
struct PackageDecl {
    std::string raw;           // Original text
    std::string path;          // Normalized path (e.g., "ratatui.widgets.block")
    std::vector<std::string> parts;  // Split parts ["ratatui", "widgets", "block"]

    // Get the last component (usually the module/class name context)
    std::string last() const {
        return parts.empty() ? "" : parts.back();
    }

    // Get without the last component (parent package)
    std::string parent() const {
        if (parts.size() <= 1) return "";
        std::string result;
        for (size_t i = 0; i < parts.size() - 1; ++i) {
            if (!result.empty()) result += ".";
            result += parts[i];
        }
        return result;
    }

    // Normalize for comparison (lowercase, no underscores)
    static std::string normalize(const std::string& s) {
        std::string result;
        for (char c : s) {
            if (c == '_' || c == '-') continue;
            result += std::tolower(c);
        }
        return result;
    }

    // Check if this package path matches another (fuzzy)
    float similarity_to(const PackageDecl& other) const {
        if (parts.empty() || other.parts.empty()) return 0.0f;

        // Count matching parts from the end (most specific first)
        int matches = 0;
        int min_len = std::min(parts.size(), other.parts.size());

        for (int i = 0; i < min_len; ++i) {
            std::string a = normalize(parts[parts.size() - 1 - i]);
            std::string b = normalize(other.parts[other.parts.size() - 1 - i]);
            if (a == b) {
                matches++;
            } else if (a.find(b) != std::string::npos || b.find(a) != std::string::npos) {
                matches++;  // Substring match counts
            } else {
                break;  // Stop at first mismatch
            }
        }

        return static_cast<float>(matches) / static_cast<float>(min_len);
    }
};

/**
 * Represents an import/use statement.
 */
struct Import {
    std::string raw;           // Original import text
    std::string module_path;   // Normalized module path (e.g., "ratatui::style::Color")
    std::string item;          // Specific item if any (e.g., "Color")
    bool is_wildcard;          // true if "use foo::*" or "import foo.*"

    // For dependency resolution
    std::string to_file_path() const {
        // Convert module path to potential file path
        // e.g., "ratatui::style::Color" -> "ratatui/style" or "ratatui/style/color"
        std::string path = module_path;
        // Replace :: with /
        size_t pos;
        while ((pos = path.find("::")) != std::string::npos) {
            path.replace(pos, 2, "/");
        }
        // Replace . with /
        while ((pos = path.find(".")) != std::string::npos) {
            path.replace(pos, 1, "/");
        }
        return path;
    }
};

/**
 * Extract imports from source files using tree-sitter.
 */
class ImportExtractor {
public:
    ImportExtractor() {
        parser_ = ts_parser_new();
    }

    ~ImportExtractor() {
        if (parser_) {
            ts_parser_delete(parser_);
        }
    }

    // Non-copyable
    ImportExtractor(const ImportExtractor&) = delete;
    ImportExtractor& operator=(const ImportExtractor&) = delete;

    /**
     * Extract all imports from a Rust file.
     */
    std::vector<Import> extract_rust_imports(const std::string& source) {
        std::vector<Import> imports;

        if (!ts_parser_set_language(parser_, tree_sitter_rust())) {
            return imports;
        }

        TSTree* tree = ts_parser_parse_string(parser_, nullptr, source.c_str(), source.length());
        if (!tree) return imports;

        TSNode root = ts_tree_root_node(tree);
        extract_rust_imports_recursive(root, source, imports);

        ts_tree_delete(tree);
        return imports;
    }

    /**
     * Extract all imports from a Kotlin file.
     */
    std::vector<Import> extract_kotlin_imports(const std::string& source) {
        std::vector<Import> imports;

        if (!ts_parser_set_language(parser_, tree_sitter_kotlin())) {
            return imports;
        }

        TSTree* tree = ts_parser_parse_string(parser_, nullptr, source.c_str(), source.length());
        if (!tree) return imports;

        TSNode root = ts_tree_root_node(tree);
        extract_kotlin_imports_recursive(root, source, imports);

        ts_tree_delete(tree);
        return imports;
    }

    /**
     * Extract all imports from a C++ file.
     */
    std::vector<Import> extract_cpp_imports(const std::string& source) {
        std::vector<Import> imports;

        if (!ts_parser_set_language(parser_, tree_sitter_cpp())) {
            return imports;
        }

        TSTree* tree = ts_parser_parse_string(parser_, nullptr, source.c_str(), source.length());
        if (!tree) return imports;

        TSNode root = ts_tree_root_node(tree);
        extract_cpp_imports_recursive(root, source, imports);

        ts_tree_delete(tree);
        return imports;
    }

    /**
     * Extract imports from a file (auto-detect language by extension).
     */
    std::vector<Import> extract_from_file(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return {};
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        if (filepath.ends_with(".rs")) {
            return extract_rust_imports(source);
        } else if (filepath.ends_with(".kt") || filepath.ends_with(".kts")) {
            return extract_kotlin_imports(source);
        } else if (filepath.ends_with(".cpp") || filepath.ends_with(".hpp") ||
                   filepath.ends_with(".cc") || filepath.ends_with(".h")) {
            return extract_cpp_imports(source);
        }

        return {};
    }

    /**
     * Extract package declaration from a Kotlin file.
     */
    PackageDecl extract_kotlin_package(const std::string& source) {
        PackageDecl pkg;

        if (!ts_parser_set_language(parser_, tree_sitter_kotlin())) {
            return pkg;
        }

        TSTree* tree = ts_parser_parse_string(parser_, nullptr, source.c_str(), source.length());
        if (!tree) return pkg;

        TSNode root = ts_tree_root_node(tree);
        extract_kotlin_package_recursive(root, source, pkg);

        ts_tree_delete(tree);
        return pkg;
    }

    /**
     * Extract module path from a Rust file (uses file path + mod declarations).
     */
    PackageDecl extract_rust_module(const std::string& source, const std::string& file_path) {
        PackageDecl pkg;

        // For Rust, derive from file path: src/widgets/block.rs -> widgets.block
        std::filesystem::path p(file_path);
        std::vector<std::string> parts;

        for (const auto& part : p.parent_path()) {
            std::string s = part.string();
            if (!s.empty() && s != "." && s != "src" && s != "lib") {
                parts.push_back(s);
            }
        }

        // Add stem (filename without extension)
        std::string stem = p.stem().string();
        if (stem != "mod" && stem != "lib") {
            parts.push_back(stem);
        }

        pkg.parts = parts;

        // Build path string
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i > 0) pkg.path += ".";
            pkg.path += parts[i];
        }

        return pkg;
    }

    /**
     * Extract namespace from a C++ file (uses file path + namespace declarations).
     */
    PackageDecl extract_cpp_namespace(const std::string& source, const std::string& file_path) {
        PackageDecl pkg;

        if (!ts_parser_set_language(parser_, tree_sitter_cpp())) {
            return pkg;
        }

        TSTree* tree = ts_parser_parse_string(parser_, nullptr, source.c_str(), source.length());
        if (tree) {
            TSNode root = ts_tree_root_node(tree);
            extract_cpp_namespace_recursive(root, source, pkg);
            ts_tree_delete(tree);
        }

        // If no namespace found, derive from file path
        if (pkg.parts.empty()) {
            std::filesystem::path p(file_path);
            std::vector<std::string> parts;

            for (const auto& part : p.parent_path()) {
                std::string s = part.string();
                if (!s.empty() && s != "." && s != "src" && s != "include") {
                    parts.push_back(s);
                }
            }

            std::string stem = p.stem().string();
            if (!stem.empty()) {
                parts.push_back(stem);
            }

            pkg.parts = parts;
            for (size_t i = 0; i < parts.size(); ++i) {
                if (i > 0) pkg.path += ".";
                pkg.path += parts[i];
            }
        }

        return pkg;
    }

    /**
     * Extract package from file (auto-detect language).
     */
    PackageDecl extract_package_from_file(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return {};
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        if (filepath.ends_with(".rs")) {
            return extract_rust_module(source, filepath);
        } else if (filepath.ends_with(".kt") || filepath.ends_with(".kts")) {
            return extract_kotlin_package(source);
        } else if (filepath.ends_with(".cpp") || filepath.ends_with(".hpp") ||
                   filepath.ends_with(".cc") || filepath.ends_with(".h")) {
            return extract_cpp_namespace(source, filepath);
        }

        return {};
    }

private:
    TSParser* parser_;

    std::string get_node_text(TSNode node, const std::string& source) {
        uint32_t start = ts_node_start_byte(node);
        uint32_t end = ts_node_end_byte(node);
        if (end > start && end <= source.length()) {
            return source.substr(start, end - start);
        }
        return "";
    }

    void extract_rust_imports_recursive(TSNode node, const std::string& source,
                                         std::vector<Import>& imports) {
        const char* type = ts_node_type(node);

        // Handle use_declaration
        if (std::string(type) == "use_declaration") {
            Import imp;
            imp.raw = get_node_text(node, source);
            imp.is_wildcard = imp.raw.find("::*") != std::string::npos;

            // Extract the path - look for scoped_identifier or path
            uint32_t child_count = ts_node_child_count(node);
            for (uint32_t i = 0; i < child_count; ++i) {
                TSNode child = ts_node_child(node, i);
                const char* child_type = ts_node_type(child);
                std::string ct(child_type);

                if (ct == "scoped_identifier" || ct == "identifier" ||
                    ct == "use_wildcard" || ct == "use_list" || ct == "scoped_use_list") {
                    imp.module_path = get_node_text(child, source);
                    // Clean up the path
                    // Remove "use " prefix if present
                    if (imp.module_path.starts_with("use ")) {
                        imp.module_path = imp.module_path.substr(4);
                    }
                    // Remove trailing ;
                    if (imp.module_path.ends_with(";")) {
                        imp.module_path = imp.module_path.substr(0, imp.module_path.length() - 1);
                    }
                    break;
                }
            }

            // Extract the last component as the item name
            size_t last_sep = imp.module_path.rfind("::");
            if (last_sep != std::string::npos) {
                imp.item = imp.module_path.substr(last_sep + 2);
            } else {
                imp.item = imp.module_path;
            }

            if (!imp.module_path.empty()) {
                imports.push_back(imp);
            }
        }

        // Recurse into children
        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count; ++i) {
            extract_rust_imports_recursive(ts_node_child(node, i), source, imports);
        }
    }

    void extract_kotlin_package_recursive(TSNode node, const std::string& source,
                                           PackageDecl& pkg) {
        const char* type = ts_node_type(node);

        // Handle package_header (Kotlin)
        if (std::string(type) == "package_header") {
            pkg.raw = get_node_text(node, source);

            // Extract the identifier
            uint32_t child_count = ts_node_child_count(node);
            for (uint32_t i = 0; i < child_count; ++i) {
                TSNode child = ts_node_child(node, i);
                const char* child_type = ts_node_type(child);

                if (std::string(child_type) == "identifier") {
                    pkg.path = get_node_text(child, source);
                    break;
                }
            }

            // Clean up and split into parts
            if (pkg.path.empty()) {
                pkg.path = pkg.raw;
            }
            if (pkg.path.starts_with("package ")) {
                pkg.path = pkg.path.substr(8);
            }
            // Trim whitespace
            while (!pkg.path.empty() && (pkg.path.back() == '\n' || pkg.path.back() == '\r' ||
                    pkg.path.back() == ' ' || pkg.path.back() == ';')) {
                pkg.path.pop_back();
            }

            // Split by dots
            std::string current;
            for (char c : pkg.path) {
                if (c == '.') {
                    if (!current.empty()) {
                        pkg.parts.push_back(current);
                        current.clear();
                    }
                } else {
                    current += c;
                }
            }
            if (!current.empty()) {
                pkg.parts.push_back(current);
            }

            return;  // Found it, stop searching
        }

        // Recurse (but only into top-level, package is always at top)
        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count && pkg.path.empty(); ++i) {
            extract_kotlin_package_recursive(ts_node_child(node, i), source, pkg);
        }
    }

    void extract_kotlin_imports_recursive(TSNode node, const std::string& source,
                                           std::vector<Import>& imports) {
        const char* type = ts_node_type(node);

        // Handle import_header (Kotlin)
        if (std::string(type) == "import_header") {
            Import imp;
            imp.raw = get_node_text(node, source);
            imp.is_wildcard = imp.raw.find(".*") != std::string::npos;

            // Extract the identifier
            uint32_t child_count = ts_node_child_count(node);
            for (uint32_t i = 0; i < child_count; ++i) {
                TSNode child = ts_node_child(node, i);
                const char* child_type = ts_node_type(child);

                if (std::string(child_type) == "identifier") {
                    imp.module_path = get_node_text(child, source);
                    break;
                }
            }

            // Clean up
            // Remove "import " prefix
            if (imp.module_path.empty()) {
                imp.module_path = imp.raw;
            }

            // Simple regex-like cleanup
            if (imp.module_path.starts_with("import ")) {
                imp.module_path = imp.module_path.substr(7);
            }
            // Trim whitespace and newlines
            while (!imp.module_path.empty() &&
                   (imp.module_path.back() == '\n' || imp.module_path.back() == '\r' ||
                    imp.module_path.back() == ' ')) {
                imp.module_path.pop_back();
            }

            // Extract item
            size_t last_dot = imp.module_path.rfind('.');
            if (last_dot != std::string::npos) {
                imp.item = imp.module_path.substr(last_dot + 1);
            } else {
                imp.item = imp.module_path;
            }

            if (!imp.module_path.empty()) {
                imports.push_back(imp);
            }
        }

        // Recurse
        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count; ++i) {
            extract_kotlin_imports_recursive(ts_node_child(node, i), source, imports);
        }
    }

    void extract_cpp_imports_recursive(TSNode node, const std::string& source,
                                         std::vector<Import>& imports) {
        const char* type = ts_node_type(node);
        std::string type_s(type);

        // Handle #include directives
        if (type_s == "preproc_include") {
            Import imp;
            imp.raw = get_node_text(node, source);
            imp.is_wildcard = false;

            // Extract the path from the include
            uint32_t child_count = ts_node_child_count(node);
            for (uint32_t i = 0; i < child_count; ++i) {
                TSNode child = ts_node_child(node, i);
                const char* child_type = ts_node_type(child);
                std::string ct(child_type);

                if (ct == "string_literal" || ct == "system_lib_string") {
                    std::string path = get_node_text(child, source);
                    // Remove quotes or angle brackets
                    if (path.length() >= 2) {
                        if ((path.front() == '"' && path.back() == '"') ||
                            (path.front() == '<' && path.back() == '>')) {
                            path = path.substr(1, path.length() - 2);
                        }
                    }
                    imp.module_path = path;
                    // Convert / to :: for consistency
                    size_t pos;
                    while ((pos = imp.module_path.find("/")) != std::string::npos) {
                        imp.module_path.replace(pos, 1, "::");
                    }
                    // Remove .hpp, .h extension
                    if (imp.module_path.ends_with(".hpp")) {
                        imp.module_path = imp.module_path.substr(0, imp.module_path.length() - 4);
                    } else if (imp.module_path.ends_with(".h")) {
                        imp.module_path = imp.module_path.substr(0, imp.module_path.length() - 2);
                    }
                    break;
                }
            }

            // Extract item (last component)
            size_t last_sep = imp.module_path.rfind("::");
            if (last_sep != std::string::npos) {
                imp.item = imp.module_path.substr(last_sep + 2);
            } else {
                imp.item = imp.module_path;
            }

            if (!imp.module_path.empty()) {
                imports.push_back(imp);
            }
        }

        // Recurse into children
        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count; ++i) {
            extract_cpp_imports_recursive(ts_node_child(node, i), source, imports);
        }
    }

    void extract_cpp_namespace_recursive(TSNode node, const std::string& source,
                                          PackageDecl& pkg) {
        const char* type = ts_node_type(node);
        std::string type_s(type);

        // Handle namespace_definition
        if (type_s == "namespace_definition") {
            // Get the namespace name
            uint32_t child_count = ts_node_child_count(node);
            for (uint32_t i = 0; i < child_count; ++i) {
                TSNode child = ts_node_child(node, i);
                const char* child_type = ts_node_type(child);
                std::string ct(child_type);

                if (ct == "namespace_identifier" || ct == "identifier") {
                    std::string name = get_node_text(child, source);
                    if (!name.empty()) {
                        pkg.parts.push_back(name);
                        if (!pkg.path.empty()) pkg.path += ".";
                        pkg.path += name;
                    }
                    break;
                }
            }

            // Check for nested namespace in the body
            for (uint32_t i = 0; i < child_count; ++i) {
                TSNode child = ts_node_child(node, i);
                const char* child_type = ts_node_type(child);
                if (std::string(child_type) == "declaration_list") {
                    extract_cpp_namespace_recursive(child, source, pkg);
                    return;  // Found nested, stop
                }
            }
            return;  // Found namespace, stop further searching
        }

        // Recurse (only into top-level)
        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count && pkg.path.empty(); ++i) {
            extract_cpp_namespace_recursive(ts_node_child(node, i), source, pkg);
        }
    }
};

} // namespace ast_distance
