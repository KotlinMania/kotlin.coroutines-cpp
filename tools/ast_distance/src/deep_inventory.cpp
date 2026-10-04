#include "deep_inventory.hpp"
#include "codebase.hpp"
#include <functional>
#include <fstream>
#include <sstream>

namespace ast_distance {
namespace {
TSNode field(TSNode node, const char* name) {
    if (ts_node_is_null(node)) return {};
    return ts_node_child_by_field_name(node, name, std::char_traits<char>::length(name));
}
std::string text(TSNode node, const std::string& source) {
    if (ts_node_is_null(node)) return {};
    return source.substr(ts_node_start_byte(node), ts_node_end_byte(node) - ts_node_start_byte(node));
}
TSNode named_child(TSNode node, const std::vector<std::string>& kinds) {
    for (uint32_t i = 0; i < ts_node_named_child_count(node); ++i) {
        auto child = ts_node_named_child(node, i);
        if (std::find(kinds.begin(), kinds.end(), ts_node_type(child)) != kinds.end()) return child;
    }
    return {};
}
TSNode declarator_name(TSNode node) {
    for (int depth = 0; depth < 64 && !ts_node_is_null(node); ++depth) {
        std::string kind = ts_node_type(node);
        if (kind == "identifier" || kind == "field_identifier" || kind == "type_identifier" ||
            kind == "qualified_identifier" || kind == "operator_name" || kind == "destructor_name") return node;
        auto next = field(node, "declarator");
        if (ts_node_is_null(next)) next = field(node, "name");
        node = next;
    }
    return {};
}
bool callable_declarator(TSNode node) {
    for (int depth = 0; depth < 64 && !ts_node_is_null(node); ++depth) {
        if (std::string(ts_node_type(node)) == "function_declarator") return true;
        node = field(node, "declarator");
    }
    return false;
}
std::string owner_key(std::string owner) {
    auto companion = owner.rfind("::Companion");
    if (companion != std::string::npos && companion + 11 == owner.size()) owner.erase(companion);
    return IdentifierStats::canonicalize(owner);
}
bool owners_match(const std::string& source, const std::string& target) {
    auto left = owner_key(source), right = owner_key(target);
    if (left.empty()) return right.empty();
    return left == right || (right.size() > left.size() + 2 && right.ends_with("::" + left));
}
bool compatible(const DeepSymbol& source, const DeepSymbol& target) {
    if (source.kind == target.kind) return true;
    // Kotlin property accessors are part of the declared property API.
    return source.kind == "property" && target.kind == "function";
}
bool symbol_matches(const DeepSymbol& source, const DeepSymbol& target) {
    if (!compatible(source, target) || !owners_match(source.owner, target.owner)) return false;
    auto left = IdentifierStats::canonicalize(source.name);
    auto right = IdentifierStats::canonicalize(target.name);
    if (left == right) return true;
    return source.kind == "property" && target.kind == "function" && right == "get" + left;
}
} // namespace

DeepInventory extract_deep_inventory(const std::vector<std::string>& paths, Language language) {
    DeepInventory inventory;
    for (const auto& path : paths) {
        std::ifstream input(path);
        if (!input) throw std::runtime_error("Cannot open inventory source: " + path);
        std::ostringstream stream; stream << input.rdbuf();
        std::string source = stream.str();
        auto adapted = language == Language::KOTLIN ? kotlin_grammar_input(source) : KotlinGrammarInput{source, {}};
        const TSLanguage* grammar = nullptr;
        switch (language) {
            case Language::KOTLIN: grammar = tree_sitter_kotlin(); break;
            case Language::CPP: grammar = tree_sitter_cpp(); break;
            case Language::RUST: grammar = tree_sitter_rust(); break;
            case Language::PYTHON: grammar = tree_sitter_python(); break;
            case Language::TYPESCRIPT: grammar = tree_sitter_typescript(); break;
        }
        TSParser* parser = ts_parser_new();
        if (!parser || !ts_parser_set_language(parser, grammar)) {
            if (parser) ts_parser_delete(parser);
            throw std::runtime_error("Cannot initialize inventory grammar");
        }
        TSTree* tree = ts_parser_parse_string(parser, nullptr, adapted.text.data(), adapted.text.size());
        if (!tree) { ts_parser_delete(parser); throw std::runtime_error("Cannot parse inventory: " + path); }
        if (ts_node_has_error(ts_tree_root_node(tree))) {
            inventory.diagnostics.push_back(path + ": parser errors; symbol inventory may be incomplete");
            std::function<void(TSNode)> errors = [&](TSNode node) {
                if (ts_node_is_error(node) || ts_node_is_missing(node)) {
                    inventory.diagnostics.push_back(path + ":" + std::to_string(ts_node_start_point(node).row + 1) +
                        "-" + std::to_string(ts_node_end_point(node).row + 1) + ": " +
                        (ts_node_is_missing(node) ? "MISSING " : "ERROR ") + ts_node_type(node));
                }
                for (uint32_t i = 0; i < ts_node_child_count(node); ++i) errors(ts_node_child(node, i));
            };
            errors(ts_tree_root_node(tree));
        }
        for (int line : adapted.fun_interface_lines) inventory.diagnostics.push_back(path + ":" + std::to_string(line) + ": offset-preserving fun-interface grammar adaptation");
        std::function<void(TSNode, std::string, int)> walk = [&](TSNode node, std::string owner, int functions) {
            std::string kind = ts_node_type(node);
            auto add = [&](std::string name, std::string category, bool definition = true) {
                if (name.empty()) return;
                std::string actual_owner = owner;
                auto separator = name.rfind("::");
                if (separator != std::string::npos) { actual_owner = name.substr(0, separator); name.erase(0, separator + 2); }
                inventory.symbols.push_back({name, actual_owner, category, path,
                    static_cast<int>(ts_node_start_point(node).row) + 1, definition});
            };
            bool container = kind == "class_declaration" || kind == "object_declaration" || kind == "companion_object" || kind == "class_specifier" ||
                kind == "struct_specifier" || kind == "enum_specifier" || kind == "struct_item" || kind == "enum_item" ||
                kind == "trait_item" || kind == "interface_declaration" || kind == "class_definition";
            if (container && functions == 0) {
                auto name_node = field(node, "name");
                if (ts_node_is_null(name_node)) name_node = named_child(node, {"type_identifier", "simple_identifier"});
                std::string name = text(name_node, source);
                if (kind == "companion_object" || (kind == "object_declaration" && text(node, source).starts_with("companion"))) name = "Companion";
                if (!name.empty()) {
                    if (name != "Companion") add(name, "type", !ts_node_is_null(field(node, "body")) || language != Language::CPP);
                    owner += (owner.empty() ? "" : "::") + name;
                }
            }
            bool callable = kind == "function_definition" || kind == "function_declaration" || kind == "function_item" || kind == "method_definition";
            if (callable) {
                if (functions == 0) {
                    auto name_node = language == Language::CPP ? declarator_name(field(node, "declarator")) : field(node, "name");
                    if (ts_node_is_null(name_node)) name_node = named_child(node, {"simple_identifier", "identifier"});
                    add(text(name_node, source), "function", !ts_node_is_null(field(node, "body")) ||
                        !ts_node_is_null(named_child(node, {"function_body"})));
                }
                ++functions;
            }
            if (functions == 0) {
                if (language == Language::CPP && (kind == "declaration" || kind == "field_declaration")) {
                    for (uint32_t i = 0; i < ts_node_child_count(node); ++i) {
                        auto role = ts_node_field_name_for_child(node, i);
                        if (!role || std::string(role) != "declarator") continue;
                        auto declarator = ts_node_child(node, i);
                        bool function = callable_declarator(declarator);
                        bool external = false;
                        for (uint32_t j = 0; j < ts_node_named_child_count(node); ++j) {
                            auto member = ts_node_named_child(node, j);
                            external |= std::string(ts_node_type(member)) == "storage_class_specifier" && text(member, source) == "extern";
                        }
                        add(text(declarator_name(declarator), source), function ? "function" : "property", !function && !external);
                    }
                } else if (kind == "property_declaration" || kind == "class_parameter") {
                    auto variable = named_child(node, {"variable_declaration"});
                    auto name = named_child(ts_node_is_null(variable) ? node : variable, {"simple_identifier", "identifier"});
                    if (kind != "class_parameter" || text(node, source).find("val ") != std::string::npos || text(node, source).find("var ") != std::string::npos)
                        add(text(name, source), "property");
                } else if (kind == "enum_entry" || kind == "enumerator" || kind == "enum_variant") {
                    auto name = field(node, "name");
                    if (ts_node_is_null(name)) name = named_child(node, {"simple_identifier", "identifier"});
                    add(text(name, source), "enum_variant");
                } else if (kind == "type_alias" || kind == "alias_declaration" || kind == "type_item") {
                    auto name = field(node, "name");
                    if (ts_node_is_null(name)) name = named_child(node, {"type_identifier"});
                    add(text(name, source), "type_alias");
                } else if (kind == "const_item" || kind == "static_item") {
                    add(text(field(node, "name"), source), "property");
                }
            }
            for (uint32_t i = 0; i < ts_node_named_child_count(node); ++i) walk(ts_node_named_child(node, i), owner, functions);
        };
        walk(ts_tree_root_node(tree), "", 0);
        ts_tree_delete(tree); ts_parser_delete(parser);
    }
    return inventory;
}

void print_deep_inventory(const Codebase& source, const Codebase& target,
                          const CodebaseComparator& comparison, std::ostream& output) {
    output << "\n=== Complete Source Symbol Inventory ===\n"
           << "Types, API declarations, functions, properties/constants, enum variants and aliases.\n"
           << "Name/owner presence is distinct from parameter/body/overload parity reported above. Parser warnings remain review requirements.\n";
    std::map<std::string, const CodebaseComparator::Match*> matches;
    for (const auto& match : comparison.matches) matches[match.source_path] = &match;
    int missing_count = 0, declared_count = 0, matched_count = 0;
    for (const auto& [key, file] : source.files) {
        auto original = extract_deep_inventory(file.paths, CodebaseComparator::string_to_language(source.language));
        DeepInventory port;
        auto found = matches.find(key);
        if (found != matches.end()) {
            auto target_file = target.files.find(found->second->target_path);
            if (target_file != target.files.end()) {
                auto paths = target_file->second.paths;
                paths.insert(paths.end(), found->second->additional_target_paths.begin(), found->second->additional_target_paths.end());
                port = extract_deep_inventory(paths, CodebaseComparator::string_to_language(target.language));
            }
        } else output << "MISSING_FILE " << file.relative_path << '\n';
        for (const auto& warning : original.diagnostics) output << "PARSER_REVIEW " << warning << '\n';
        for (const auto& warning : port.diagnostics) output << "PARSER_REVIEW " << warning << '\n';
        std::set<std::string> used_targets;
        for (const auto& symbol : original.symbols) {
            const DeepSymbol* candidate = nullptr;
            for (const auto& other : port.symbols) if (symbol_matches(symbol, other) && (!candidate || other.definition)) candidate = &other;
            if (candidate) used_targets.insert(candidate->file + ":" + std::to_string(candidate->line) + ":" + candidate->name);
            std::string status = candidate ? (candidate->definition ? "PRESENT" : "DECLARATION_ONLY") : "MISSING_SYMBOL";
            if (!candidate) ++missing_count;
            else if (candidate->definition) ++matched_count;
            else ++declared_count;
            output << status << ' ' << symbol.kind << ' ' << symbol.owner << (symbol.owner.empty() ? "" : "::")
                   << symbol.name << " at " << symbol.file << ':' << symbol.line;
            if (candidate) output << " -> " << candidate->file << ':' << candidate->line;
            output << '\n';
        }
        for (const auto& symbol : port.symbols) {
            if (used_targets.count(symbol.file + ":" + std::to_string(symbol.line) + ":" + symbol.name)) continue;
            bool source_namesake = std::any_of(original.symbols.begin(), original.symbols.end(),
                [&](const DeepSymbol& other) { return symbol_matches(other, symbol); });
            if (!source_namesake) output << "EXTRA_TARGET_SYMBOL " << symbol.kind << ' ' << symbol.owner
                << (symbol.owner.empty() ? "" : "::") << symbol.name << " at " << symbol.file << ':' << symbol.line << '\n';
        }
    }
    output << "Inventory totals: present=" << matched_count << ", declaration_only=" << declared_count
           << ", missing=" << missing_count << '\n';
}
} // namespace ast_distance
