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
    bool is_enum_member = false;
    std::string first_parameter_type;
    std::string namespace_path;
    bool namespace_known = false;
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
// Review explicit Kotlin suspension contracts against parsed target syntax.
// Evidence is lexical/structural; compiled IR and behavior require separate checks.
void print_suspension_review(const std::vector<std::string>& source_paths,
                             const std::vector<std::string>& target_paths,
                             std::ostream& output);
} // namespace ast_distance
