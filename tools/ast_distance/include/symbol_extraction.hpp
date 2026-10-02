#pragma once

#include <tree_sitter/api.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <iomanip>

extern "C" {
    const TSLanguage* tree_sitter_rust();
    const TSLanguage* tree_sitter_kotlin();
}

namespace ast_distance {

enum class SymbolKind {
    FUNCTION,
    STRUCT,
    ENUM,
    TRAIT,
    IMPL_METHOD,
    CONST,
    TYPE_ALIAS,
    ENUM_VARIANT,
    FIELD,
};

inline const char* symbol_kind_name(SymbolKind kind) {
    switch (kind) {
        case SymbolKind::FUNCTION: return "FUNCTION";
        case SymbolKind::STRUCT: return "STRUCT";
        case SymbolKind::ENUM: return "ENUM";
        case SymbolKind::TRAIT: return "TRAIT";
        case SymbolKind::IMPL_METHOD: return "IMPL_METHOD";
        case SymbolKind::CONST: return "CONST";
        case SymbolKind::TYPE_ALIAS: return "TYPE_ALIAS";
        case SymbolKind::ENUM_VARIANT: return "ENUM_VARIANT";
        case SymbolKind::FIELD: return "FIELD";
    }
    return "UNKNOWN";
}

inline const char* symbol_kind_label(SymbolKind kind) {
    switch (kind) {
        case SymbolKind::FUNCTION: return "FUNCTIONS";
        case SymbolKind::STRUCT: return "STRUCTS";
        case SymbolKind::ENUM: return "ENUMS";
        case SymbolKind::TRAIT: return "TRAITS";
        case SymbolKind::IMPL_METHOD: return "IMPL METHODS";
        case SymbolKind::CONST: return "CONSTANTS";
        case SymbolKind::TYPE_ALIAS: return "TYPE ALIASES";
        case SymbolKind::ENUM_VARIANT: return "ENUM VARIANTS";
        case SymbolKind::FIELD: return "FIELDS";
    }
    return "UNKNOWN";
}

enum class Visibility {
    PUBLIC,
    PRIVATE,
    CRATE,  // pub(crate) in Rust, internal in Kotlin
};

inline const char* visibility_name(Visibility vis) {
    switch (vis) {
        case Visibility::PUBLIC: return "pub";
        case Visibility::PRIVATE: return "private";
        case Visibility::CRATE: return "pub(crate)";
    }
    return "unknown";
}

struct Symbol {
    std::string name;
    std::string qualified_name;  // "Type::method" for impl methods
    SymbolKind kind;
    Visibility visibility = Visibility::PUBLIC;
    std::string file;            // Relative path
    int line = 0;
    std::string parent;          // For impl methods: the type being impl'd
    std::vector<std::string> members;  // For structs: fields, for enums: variants
    bool is_test = false;        // Rust #[test] function or test-only type
    bool is_stub = false;        // Kotlin: placeholder/stub type (empty body, TODO markers)
    bool is_extension = false;   // Kotlin: extension function
    std::string receiver_type;   // Kotlin: receiver type for extension functions
};

struct SymbolTable {
    std::vector<Symbol> symbols;
    std::map<SymbolKind, std::vector<const Symbol*>> by_kind;
    std::map<std::string, const Symbol*> by_qualified_name;

    void add(Symbol s) {
        symbols.push_back(std::move(s));
        const Symbol* ptr = &symbols.back();
        by_kind[ptr->kind].push_back(ptr);
        if (!ptr->qualified_name.empty()) {
            by_qualified_name[ptr->qualified_name] = ptr;
        }
        // Also index by plain name for matching
        if (ptr->qualified_name != ptr->name) {
            by_qualified_name[ptr->name] = ptr;
        }
    }

    const Symbol* find(const std::string& name) const {
        auto it = by_qualified_name.find(name);
        return (it != by_qualified_name.end()) ? it->second : nullptr;
    }

    size_t size() const { return symbols.size(); }
};

struct SymbolMatch {
    const Symbol* rust_symbol = nullptr;
    const Symbol* kotlin_symbol = nullptr;
    float confidence = 0.0f;
    std::string match_reason;  // "exact", "camelCase", "qualified"
};

struct SymbolParityReport {
    std::vector<SymbolMatch> matches;
    std::vector<const Symbol*> missing_in_kotlin;
    std::vector<const Symbol*> extra_in_kotlin;
    std::map<SymbolKind, std::pair<int, int>> coverage;       // matched/total per kind (production)
    std::map<SymbolKind, std::pair<int, int>> test_coverage;  // matched/total per kind (tests)
    int stub_count = 0;                                        // Kotlin stubs detected

    void print(bool verbose = true, bool missing_only = false) const;
    void print_json() const;
};

struct SymbolParityOptions {
    bool json = false;
    bool verbose = false;
    bool missing_only = false;     // Show only missing symbols
    bool include_stubs = false;    // Include Kotlin stubs in "extra" count
    std::string filter_kind;
    std::string filter_file;
};

// Name conversion utilities
std::string snake_to_camel(const std::string& snake);
std::string rust_qualified_to_kotlin(const std::string& qualified);

// Extract symbols from a codebase root
SymbolTable extract_rust_symbols(const std::string& root);
SymbolTable extract_kotlin_symbols(const std::string& root);

// Build parity report
SymbolParityReport build_parity_report(const SymbolTable& rust, const SymbolTable& kotlin);

// CLI command
void cmd_symbol_parity(const std::string& rust_root,
                       const std::string& kotlin_root,
                       const SymbolParityOptions& options);

// ============================================================================
// Import Map: Kotlin type registry and import resolution
// ============================================================================

struct KotlinTypeEntry {
    std::string name;              // Simple name (e.g., "Value")
    std::string package;           // Package (e.g., "io.github.kotlinmania.starlark_kotlin.values.layout")
    std::string fqn;               // Fully qualified (package + name)
    std::string file;              // Relative file path
    int line = 0;
    SymbolKind kind = SymbolKind::STRUCT;
};

struct KotlinFileInfo {
    std::string file;              // Relative path
    std::string package;           // Package declaration
    std::set<std::string> imports; // Existing import statements (simple names)
    std::set<std::string> import_fqns; // Full import FQNs
    std::set<std::string> defined_types;  // Types defined in this file
    std::set<std::string> referenced_types; // Type names referenced (used)
    std::set<std::string> unresolved;  // referenced - (imported + defined + same-package)
};

struct ImportMapReport {
    // Registry: simple name → all entries with that name
    std::map<std::string, std::vector<KotlinTypeEntry>> type_registry;

    // Per-file analysis
    std::vector<KotlinFileInfo> files;

    // For each file, suggested imports
    struct ImportSuggestion {
        std::string file;
        std::string type_name;
        std::string suggested_import;  // FQN
        bool ambiguous = false;        // Multiple definitions exist
        std::vector<std::string> alternatives; // If ambiguous
    };
    std::vector<ImportSuggestion> suggestions;
};

struct ImportMapOptions {
    bool json = false;
    bool summary_only = false;     // Just show per-file counts
    std::string filter_file;       // Show only this file
    int min_unresolved = 1;        // Skip files with fewer unresolved refs
};

// Build the import map
ImportMapReport build_import_map(const std::string& kotlin_root);

// CLI command
void cmd_import_map(const std::string& kotlin_root,
                    const ImportMapOptions& options);

// ============================================================================
// Compiler Fixup: Parse compiler errors and suggest fixes
// ============================================================================

struct CompilerError {
    std::string file;          // Relative path within kotlin_root
    int line = 0;
    int col = 0;
    std::string reference;     // The unresolved symbol name
    std::string full_message;  // Full error text
};

struct FixSuggestion {
    std::string file;          // File needing the fix
    std::string reference;     // Unresolved symbol
    std::string suggested_import; // FQN to import (empty if not found)
    bool ambiguous = false;
    std::vector<std::string> alternatives;
    int occurrences = 0;       // How many errors in this file for this ref
};

struct CompilerFixupReport {
    int total_errors = 0;
    int unresolved_errors = 0;
    int fixable_errors = 0;
    int ambiguous_errors = 0;
    int unfixable_errors = 0;
    // Per-file: reference → suggestion
    std::map<std::string, std::vector<FixSuggestion>> by_file;
    // All unique unresolved references with counts
    std::map<std::string, int> unresolved_counts;
    // References not found in registry
    std::set<std::string> not_in_registry;

    void print(bool verbose = false) const;
    void print_json() const;
};

struct CompilerFixupOptions {
    bool json = false;
    bool verbose = false;
    std::string filter_file;   // Show only this file
    int min_errors = 1;        // Skip files with fewer errors
    bool apply = false;        // Auto-apply non-ambiguous fixes (future)
};

// Parse compiler errors from a file
std::vector<CompilerError> parse_compiler_errors(
    const std::string& error_file, const std::string& kotlin_root);

// Build fixup report by cross-referencing errors with type registry
CompilerFixupReport build_compiler_fixup(
    const std::vector<CompilerError>& errors,
    const ImportMapReport& import_map);

// CLI command
void cmd_compiler_fixup(const std::string& kotlin_root,
                        const std::string& error_file,
                        const CompilerFixupOptions& options);

} // namespace ast_distance
