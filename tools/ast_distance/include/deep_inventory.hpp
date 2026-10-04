#pragma once

#include "ast_parser.hpp"
#include <iosfwd>
#include <string>
#include <vector>

namespace ast_distance {
class Codebase;
class CodebaseComparator;

struct DeepSymbol {
    std::string name;
    std::string owner;
    std::string kind;
    std::string file;
    int line = 0;
    bool definition = true;
};

struct DeepInventory {
    std::vector<DeepSymbol> symbols;
    std::vector<std::string> diagnostics;
};

DeepInventory extract_deep_inventory(const std::vector<std::string>& paths,
                                     Language language);
void print_deep_inventory(const Codebase& source, const Codebase& target,
                          const CodebaseComparator& comparison,
                          std::ostream& output);
} // namespace ast_distance
