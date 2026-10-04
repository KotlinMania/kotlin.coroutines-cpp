#include "symbol_extractor.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace ast_distance {

// Get text from a node
std::string SymbolExtractor::get_node_text(TSNode node, const std::string& source) {
    uint32_t start = ts_node_start_byte(node);
    uint32_t end = ts_node_end_byte(node);
    if (start >= source.size() || end > source.size() || start >= end) {
        return "";
    }
    return source.substr(start, end - start);
}

// Get node type as string
std::string SymbolExtractor::get_node_type(TSNode node) {
    const char* type = ts_node_type(node);
    return type ? std::string(type) : "";
}

// Check if node is a symbol definition and return type
std::string SymbolExtractor::is_symbol_definition(const std::string& node_type) {
    // Rust
    if (node_type == "struct_item") return "struct";
    if (node_type == "enum_item") return "enum";
    if (node_type == "trait_item") return "trait";
    if (node_type == "type_item") return "type";

    // Kotlin
    if (node_type == "class_declaration") return "class";
    if (node_type == "object_declaration") return "object";
    if (node_type == "interface_declaration") return "interface";
    if (node_type == "type_alias") return "typealias";

    // C++
    if (node_type == "class_specifier") return "class";
    if (node_type == "struct_specifier") return "struct";
    if (node_type == "enum_specifier") return "enum";
    if (node_type == "alias_declaration" || node_type == "type_definition") return "typealias";

    return "";
}

// Extract symbol name from definition node
std::string SymbolExtractor::extract_symbol_name(TSNode node, const std::string& source) {
    uint32_t child_count = ts_node_child_count(node);

    for (uint32_t i = 0; i < child_count; ++i) {
        TSNode child = ts_node_child(node, i);
        std::string child_type = get_node_type(child);

        // Look for identifier nodes
        if (child_type == "type_identifier" ||
            child_type == "identifier" ||
            child_type == "simple_identifier") {
            std::string name = get_node_text(child, source);
            if (!name.empty()) {
                return name;
            }
        }
    }

    return "";
}

// Check if symbol is public
bool SymbolExtractor::is_public_symbol(TSNode node, const std::string& source) {
    uint32_t child_count = ts_node_child_count(node);

    for (uint32_t i = 0; i < child_count; ++i) {
        TSNode child = ts_node_child(node, i);
        std::string child_type = get_node_type(child);

        // Rust: check for "pub" visibility modifier
        if (child_type == "visibility_modifier") {
            std::string text = get_node_text(child, source);
            if (text.find("pub") != std::string::npos) {
                return true;
            }
        }

        // Kotlin: public by default, check for private/internal
        if (child_type == "modifiers") {
            std::string text = get_node_text(child, source);
            if (text.find("private") != std::string::npos ||
                text.find("internal") != std::string::npos) {
                return false;
            }
        }
    }

    // Kotlin symbols are public by default
    // Rust symbols are private by default
    // For now, assume public if we can't determine
    return true;
}

// Recursive symbol extraction
void SymbolExtractor::extract_symbols_recursive(
    TSNode node,
    const std::string& source,
    const std::string& package,
    const std::string& file_path,
    std::vector<SymbolDefinition>& symbols
) {
    std::string node_type = get_node_type(node);
    std::string symbol_type = is_symbol_definition(node_type);

    if (!symbol_type.empty()) {
        SymbolDefinition sym;
        sym.type = symbol_type;
        sym.name = extract_symbol_name(node, source);
        sym.package = package;
        sym.file_path = file_path;
        sym.is_public = is_public_symbol(node, source);

        TSPoint start_point = ts_node_start_point(node);
        sym.line_number = start_point.row + 1;  // 1-indexed

        if (!sym.name.empty()) {
            symbols.push_back(sym);
        }
    }

    // Recurse into children
    uint32_t child_count = ts_node_child_count(node);
    for (uint32_t i = 0; i < child_count; ++i) {
        TSNode child = ts_node_child(node, i);
        extract_symbols_recursive(child, source, package, file_path, symbols);
    }
}

// Main extraction function
std::vector<SymbolDefinition> SymbolExtractor::extract_symbols(
    TSNode root,
    const std::string& source,
    const std::string& package,
    const std::string& file_path
) {
    std::vector<SymbolDefinition> symbols;
    extract_symbols_recursive(root, source, package, file_path, symbols);
    return symbols;
}

// Find duplicate symbols
std::vector<SymbolComparator::DuplicateSymbol> SymbolComparator::find_duplicates(
    const std::map<std::string, std::vector<SymbolDefinition>>& symbols_by_file
) {
    // Build map of symbol name -> list of files
    std::map<std::string, std::vector<std::string>> symbol_locations;

    for (const auto& [file, symbols] : symbols_by_file) {
        for (const auto& sym : symbols) {
            symbol_locations[sym.name].push_back(file);
        }
    }

    // Find symbols in multiple files
    std::vector<DuplicateSymbol> duplicates;
    for (const auto& [name, locations] : symbol_locations) {
        if (locations.size() > 1) {
            DuplicateSymbol dup;
            dup.name = name;
            dup.locations = locations;
            duplicates.push_back(dup);
        }
    }

    return duplicates;
}

// Find namespace mismatches
std::vector<SymbolComparator::NamespaceMismatch> SymbolComparator::find_namespace_mismatches(
    const std::map<std::string, std::vector<SymbolDefinition>>& source_symbols,
    const std::map<std::string, std::vector<SymbolDefinition>>& target_symbols
) {
    std::vector<NamespaceMismatch> mismatches;

    // Build lookup maps
    std::map<std::string, SymbolDefinition> source_by_name;
    std::map<std::string, SymbolDefinition> target_by_name;

    for (const auto& [file, symbols] : source_symbols) {
        for (const auto& sym : symbols) {
            source_by_name[sym.name] = sym;
        }
    }

    for (const auto& [file, symbols] : target_symbols) {
        for (const auto& sym : symbols) {
            target_by_name[sym.name] = sym;
        }
    }

    // Compare packages for matching symbol names
    for (const auto& [name, source_sym] : source_by_name) {
        auto it = target_by_name.find(name);
        if (it != target_by_name.end()) {
            const auto& target_sym = it->second;

            // Normalize package names for comparison
            // Rust: foo::bar::baz -> foo.bar.baz
            // Kotlin: foo.bar.baz (already dots)
            std::string src_pkg = source_sym.package;
            std::replace(src_pkg.begin(), src_pkg.end(), ':', '.');

            if (src_pkg != target_sym.package) {
                NamespaceMismatch mismatch;
                mismatch.symbol_name = name;
                mismatch.source_package = source_sym.package;
                mismatch.target_package = target_sym.package;
                mismatch.source_file = source_sym.file_path;
                mismatch.target_file = target_sym.file_path;
                mismatches.push_back(mismatch);
            }
        }
    }

    return mismatches;
}

// Find missing symbols
std::vector<SymbolComparator::MissingSymbol> SymbolComparator::find_missing_symbols(
    const std::map<std::string, std::vector<SymbolDefinition>>& source_symbols,
    const std::map<std::string, std::vector<SymbolDefinition>>& target_symbols
) {
    std::vector<MissingSymbol> missing;

    // Build set of target symbol names
    std::set<std::string> target_names;
    for (const auto& [file, symbols] : target_symbols) {
        for (const auto& sym : symbols) {
            target_names.insert(sym.name);
        }
    }

    // Find source symbols not in target
    for (const auto& [file, symbols] : source_symbols) {
        for (const auto& sym : symbols) {
            if (target_names.find(sym.name) == target_names.end()) {
                MissingSymbol miss;
                miss.name = sym.name;
                miss.source_file = sym.file_path;
                miss.source_package = sym.package;
                missing.push_back(miss);
            }
        }
    }

    return missing;
}

// Print report
void SymbolComparator::print_symbol_report(
    const std::vector<DuplicateSymbol>& duplicates,
    const std::vector<NamespaceMismatch>& mismatches,
    const std::vector<MissingSymbol>& missing
) {
    std::cout << "\n=== Symbol Location Analysis ===\n\n";

    // Duplicates
    if (!duplicates.empty()) {
        std::cout << "Duplicate Definitions (" << duplicates.size() << "):\n";
        for (const auto& dup : duplicates) {
            std::cout << "  " << dup.name << " (defined in " << dup.locations.size() << " files):\n";
            for (const auto& loc : dup.locations) {
                std::cout << "    - " << loc << "\n";
            }
        }
        std::cout << "\n";
    }

    // Namespace mismatches
    if (!mismatches.empty()) {
        std::cout << "Namespace Mismatches (" << mismatches.size() << "):\n";
        for (const auto& mismatch : mismatches) {
            std::cout << "  " << mismatch.symbol_name << ":\n";
            std::cout << "    Source: " << mismatch.source_package << " (" << mismatch.source_file << ")\n";
            std::cout << "    Target: " << mismatch.target_package << " (" << mismatch.target_file << ")\n";
        }
        std::cout << "\n";
    }

    // Missing symbols
    if (!missing.empty()) {
        std::cout << "Missing Symbols (" << missing.size() << "):\n";
        for (const auto& miss : missing) {
            std::cout << "  " << miss.name << " (" << miss.source_package << ") - " << miss.source_file << "\n";
        }
        std::cout << "\n";
    }

    if (duplicates.empty() && mismatches.empty() && missing.empty()) {
        std::cout << "No symbol location issues detected.\n\n";
    }
}

} // namespace ast_distance
