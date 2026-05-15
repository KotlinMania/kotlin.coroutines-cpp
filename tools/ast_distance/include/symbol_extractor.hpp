#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <tree_sitter/api.h>

namespace ast_distance {

/**
 * Represents a class/struct/enum/interface definition found in source code.
 */
struct SymbolDefinition {
    std::string name;           // Symbol name (e.g., "KeyEventRecord")
    std::string type;           // Type: "class", "struct", "interface", "enum", "data class", etc.
    std::string file_path;      // File where defined
    std::string package;        // Package/namespace
    int line_number = 0;        // Line number where defined
    bool is_public = false;     // Whether exported/public

    // Full qualified name: package.name
    std::string qualified_name() const {
        if (package.empty()) return name;
        return package + "." + name;
    }
};

/**
 * Extracts symbol definitions (classes, structs, interfaces, enums) from source code.
 */
class SymbolExtractor {
public:
    /**
     * Extract all symbol definitions from a file's AST.
     *
     * @param root The tree-sitter parse tree root
     * @param source The source code text
     * @param package The package/namespace of this file
     * @param file_path The file path for reference
     * @return Vector of symbol definitions found
     */
    static std::vector<SymbolDefinition> extract_symbols(
        TSNode root,
        const std::string& source,
        const std::string& package,
        const std::string& file_path
    );

private:
    /**
     * Recursively extract symbols from AST node.
     */
    static void extract_symbols_recursive(
        TSNode node,
        const std::string& source,
        const std::string& package,
        const std::string& file_path,
        std::vector<SymbolDefinition>& symbols
    );

    /**
     * Extract text for a node from source.
     */
    static std::string get_node_text(TSNode node, const std::string& source);

    /**
     * Get node type as string.
     */
    static std::string get_node_type(TSNode node);

    /**
     * Check if a node represents a symbol definition we care about.
     * Returns the symbol type ("class", "struct", etc.) or empty string if not a symbol.
     */
    static std::string is_symbol_definition(const std::string& node_type);

    /**
     * Extract the name from a symbol definition node.
     */
    static std::string extract_symbol_name(TSNode node, const std::string& source);

    /**
     * Check if a symbol is public/exported based on modifiers or context.
     */
    static bool is_public_symbol(TSNode node, const std::string& source);
};

/**
 * Compare symbol definitions across two codebases to detect:
 * - Duplicate definitions (same symbol in multiple files)
 * - Namespace mismatches (symbol in wrong package)
 * - Missing symbols (defined in source but not target)
 */
class SymbolComparator {
public:
    struct DuplicateSymbol {
        std::string name;
        std::vector<std::string> locations;  // File paths where duplicated
    };

    struct NamespaceMismatch {
        std::string symbol_name;
        std::string source_package;   // Package in source (Rust)
        std::string target_package;   // Package in target (Kotlin)
        std::string source_file;
        std::string target_file;
    };

    struct MissingSymbol {
        std::string name;
        std::string source_file;
        std::string source_package;
    };

    /**
     * Find symbols defined in multiple files within a codebase.
     */
    static std::vector<DuplicateSymbol> find_duplicates(
        const std::map<std::string, std::vector<SymbolDefinition>>& symbols_by_file
    );

    /**
     * Compare symbol locations between source and target codebases.
     * Detects when a symbol is in a different package/namespace than expected.
     */
    static std::vector<NamespaceMismatch> find_namespace_mismatches(
        const std::map<std::string, std::vector<SymbolDefinition>>& source_symbols,
        const std::map<std::string, std::vector<SymbolDefinition>>& target_symbols
    );

    /**
     * Find symbols in source that are missing from target.
     */
    static std::vector<MissingSymbol> find_missing_symbols(
        const std::map<std::string, std::vector<SymbolDefinition>>& source_symbols,
        const std::map<std::string, std::vector<SymbolDefinition>>& target_symbols
    );

    /**
     * Print symbol analysis report.
     */
    static void print_symbol_report(
        const std::vector<DuplicateSymbol>& duplicates,
        const std::vector<NamespaceMismatch>& mismatches,
        const std::vector<MissingSymbol>& missing
    );
};

} // namespace ast_distance
