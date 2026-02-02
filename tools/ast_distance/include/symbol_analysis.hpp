#pragma once

#include "codebase.hpp"
#include <string>
#include <vector>
#include <map>
#include <filesystem>

namespace ast_distance {

struct SymbolAnalysisOptions {
    bool duplicates = false;
    bool stubs = false;
    bool misplaced = false;
    bool json = false;
    std::string symbol;
};

// Run symbol analysis (duplicates/stubs/misplaced) for given roots.
void cmd_symbols(const std::string& kotlin_root,
                 const std::string& cpp_root,
                 const SymbolAnalysisOptions& options);

// Analyze a single symbol and print details (supports JSON output).
void cmd_symbol_lookup(const std::string& kotlin_root,
                       const std::string& cpp_root,
                       const SymbolAnalysisOptions& options);

}  // namespace ast_distance
