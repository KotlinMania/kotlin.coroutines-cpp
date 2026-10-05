#include "transliteration_engine.hpp"
#include "callable_identity.hpp"
#include "similarity.hpp"
#include "reexport_config.hpp"
#include <functional>
#include <optional>
#include <set>

namespace ast_distance {
namespace {
std::string snake(std::string text) {
    std::string result;
    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char c = text[i];
        if (std::isupper(c) && i && (std::islower(static_cast<unsigned char>(text[i - 1])) ||
            (i + 1 < text.size() && std::islower(static_cast<unsigned char>(text[i + 1]))))) result += '_';
        result += static_cast<char>(std::tolower(c));
    }
    return result;
}
std::optional<std::string> cpp_primitive_type(std::string name) {
    if (name.starts_with("kotlin.")) name.erase(0, 7);
    static const std::map<std::string, std::string> primitive = {{"Int","int"},{"UInt","unsigned int"},{"Long","long long"},{"ULong","unsigned long long"},{"Short","short"},{"Byte","signed char"},{"Boolean","bool"},{"Double","double"},{"Float","float"},{"Unit","void"},{"Char","char16_t"},{"String","std::string"}};
    auto found = primitive.find(name);
    if (found == primitive.end()) return {};
    return found->second;
}
struct RenderedComment {
    std::string text;
    bool unsupported_example = false;
};
std::string documentation_name(const std::string& reference, const std::set<std::string>& function_names) {
    std::string result;
    std::istringstream parts(reference);
    for (std::string part; std::getline(parts, part, '.');) {
        if (part == "Companion" && !result.empty()) continue; // Companion members lower to class static members.
        if (!result.empty()) result += "::";
        bool all_caps = std::none_of(part.begin(), part.end(), [](unsigned char c) { return std::islower(c); });
        if (!all_caps && ((!part.empty() && std::islower(static_cast<unsigned char>(part.front()))) || function_names.contains(part))) part = snake(part);
        result += part;
    }
    return result;
}
RenderedComment render_documentation(const std::string& original, const std::set<std::string>& function_names) {
    RenderedComment output;
    auto identifier = [](const std::string& name) {
        if (name.empty() || !(std::isalpha(static_cast<unsigned char>(name.front())) || name.front() == '_')) return false;
        return std::all_of(name.begin(), name.end(), [](unsigned char c) { return std::isalnum(c) || c == '_' || c == '.'; });
    };
    auto kotlin_example = [](const std::string& example) {
        static const std::set<std::string> keywords = {"fun", "val", "var", "when", "suspend", "package", "import", "typealias", "object"};
        std::string word;
        for (unsigned char c : example) {
            if (std::isalnum(c) || c == '_') word += c;
            else { if (keywords.contains(word)) return true; word.clear(); }
        }
        return keywords.contains(word);
    };
    for (size_t i = 0; i < original.size();) {
        if (original.compare(i, 3, "```") == 0) {
            auto end = original.find("```", i + 3);
            size_t finish = end == std::string::npos ? original.size() : end + 3;
            auto example = original.substr(i, finish - i);
            output.unsupported_example |= kotlin_example(example);
            output.text += example; i = finish; continue;
        }
        if (original[i] == '`') {
            auto end = original.find('`', i + 1);
            if (end != std::string::npos) {
                auto example = original.substr(i, end + 1 - i);
                output.unsupported_example |= kotlin_example(example);
                output.text += example; i = end + 1; continue;
            }
        }
        bool param = original.compare(i, 6, "@param") == 0;
        bool see = original.compare(i, 4, "@see") == 0;
        if (param || see) {
            size_t end = i + (param ? 6 : 4);
            if (end < original.size() && std::isspace(static_cast<unsigned char>(original[end]))) {
                output.text += original.substr(i, end - i);
                while (end < original.size() && std::isspace(static_cast<unsigned char>(original[end]))) output.text += original[end++];
                size_t finish = end;
                while (finish < original.size() && (std::isalnum(static_cast<unsigned char>(original[finish])) || original[finish] == '_' || original[finish] == '.')) ++finish;
                auto name = original.substr(end, finish - end);
                if (identifier(name)) {
                    if (param) output.text += snake(name); // Parameter tags name a value, never a type.
                    else if (auto primitive = cpp_primitive_type(name))
                        output.text += primitive->find(' ') == std::string::npos ? "\\c " + *primitive : "`" + *primitive + "`";
                    else output.text += documentation_name(name, function_names);
                } else output.text += name;
                i = finish; continue;
            }
        }
        if (original[i] == '[') {
            auto end = original.find(']', i + 1);
            if (end != std::string::npos && (end + 1 >= original.size() || original[end + 1] != '(')) {
                auto first = original.substr(i + 1, end - i - 1);
                size_t finish = end + 1;
                std::string target = first, label;
                if (finish < original.size() && original[finish] == '[') {
                    auto second_end = original.find(']', finish + 1);
                    if (second_end != std::string::npos) {
                        target = original.substr(finish + 1, second_end - finish - 1);
                        label = first; finish = second_end + 1;
                    }
                }
                if (identifier(target)) {
                    if (auto primitive = cpp_primitive_type(target)) {
                        output.text += primitive->find(' ') == std::string::npos ? "\\c " + *primitive : "`" + *primitive + "`";
                        if (!label.empty()) output.text += " (" + label + ")";
                    } else {
                        output.text += "\\ref " + documentation_name(target, function_names);
                        if (!label.empty()) output.text += " \"" + json_escape(label) + "\"";
                    }
                    i = finish; continue;
                }
            }
        }
        output.text += original[i++];
    }
    return output;
}
class KotlinCppEmitter {
public:
    explicit KotlinCppEmitter(const std::string& source) : source_(source) {
        ASTParser parser;
        for (const auto& function : parser.extract_function_infos(source, Language::KOTLIN)) function_names_.insert(function.name);
    }
    TransliterationOutput output;
    std::string emit(TSNode node) {
        std::string kind = ts_node_type(node), result;
        auto children = named(node);
        bool supported = true;
        if (kind == "source_file") {
            for (auto child : children) {
                size_t misses_before = output.rule_misses;
                size_t start = result.size();
                std::string fragment = emit(child);
                result += fragment + '\n';
                output.spans.push_back({ts_node_type(child), static_cast<int>(ts_node_start_point(child).row) + 1,
                    ts_node_start_byte(child), ts_node_end_byte(child), start, start + fragment.size(),
                    output.rule_misses == misses_before});
            }
            if (std::any_of(children.begin(), children.end(), [](TSNode child) { return std::string(ts_node_type(child)) == "package_header"; })) result += "}\n";
        } else if (kind == "line_comment" || kind == "multiline_comment") {
            // Only parsed comment spans are rewritten. Narrative and executable
            // examples stay intact; identifier-bearing KDoc syntax lowers to Doxygen.
            auto comment = render_documentation(text(node), function_names_);
            result = std::move(comment.text);
            if (comment.unsupported_example) {
                ++output.documentation_misses;
                output.diagnostics.push_back(std::to_string(ts_node_start_point(node).row + 1) +
                    ": documentation example retains unsupported Kotlin syntax");
            }
        } else if (kind == "package_header") {
            auto identifier = child_of(node, "identifier");
            if (ts_node_is_null(identifier)) supported = false;
            else {
                std::string package_name = text(identifier);
                for (size_t i = 0; i < package_name.size(); ++i) {
                    if (package_name[i] == '.') { package_name.replace(i, 1, "::"); ++i; }
                }
                result = "namespace " + package_name + " {";
            }
        } else if (kind == "import_list" || kind == "import_header" || kind == "modifiers") {
            // Declaration metadata is accounted for separately from executable logic.
            result = "";
        } else if (kind == "function_declaration") {
            auto name = child_of(node, "simple_identifier"), params = child_of(node, "function_value_parameters"), body = child_of(node, "function_body");
            auto modifiers = child_of(node, "modifiers");
            bool signature_supported = true, seen_params = false, explicit_return = false;
            for (auto child : children) {
                std::string child_kind = ts_node_type(child);
                if (ts_node_eq(child, params)) { seen_params = true; continue; }
                if (child_kind == "user_type") {
                    if (!seen_params) signature_supported = false; // Extension receiver needs a dedicated lowering rule.
                    else explicit_return = true;
                } else if (child_kind != "modifiers" && child_kind != "simple_identifier" && child_kind != "function_body") signature_supported = false;
            }
            if ((!ts_node_is_null(modifiers) && text(modifiers) != "public") || !signature_supported || ts_node_is_null(name) || ts_node_is_null(params)) {
                supported = false;
            } else {
                std::string return_type = "void";
                if (!explicit_return && !ts_node_is_null(body) && text(body).starts_with("=")) supported = false;
                bool after_params = false;
                for (auto child : children) {
                    if (ts_node_eq(child, params)) { after_params = true; continue; }
                    if (after_params && std::string(ts_node_type(child)) == "user_type") return_type = emit(child);
                }
                result = return_type + " " + snake(text(name)) + "(" + emit(params) + ")";
                result += ts_node_is_null(body) ? ";" : emit(body);
            }
        } else if (kind == "function_value_parameters") {
            for (size_t i = 0; i < children.size(); ++i) {
                auto child = children[i];
                if (std::string(ts_node_type(child)) != "parameter") { supported = false; break; }
                if (!result.empty()) result += ", ";
                result += emit(child);
                // Kotlin's grammar places defaults beside the parameter node.
                if (i + 1 < children.size() && std::string(ts_node_type(children[i + 1])) != "parameter") result += " = " + emit(children[++i]);
            }
        } else if (kind == "parameter") {
            auto name = child_of(node, "simple_identifier"), type = child_of(node, "user_type");
            bool parameter_supported = true;
            for (auto child : children) {
                std::string child_kind = ts_node_type(child);
                if (child_kind != "simple_identifier" && child_kind != "user_type") parameter_supported = false;
            }
            if (!parameter_supported || ts_node_is_null(name) || ts_node_is_null(type)) supported = false;
            else result = emit(type) + " " + snake(text(name));
        } else if (kind == "function_body") {
            if (!children.empty() && ts_node_type(ts_node_child(node, 0)) == std::string("=")) result = " { return " + emit(children[0]) + "; }";
            else result = children.size() == 1 && std::string(ts_node_type(children[0])) == "statements"
                ? " " + emit(children[0]) : " { " + join_statements(children) + " }";
        } else if (kind == "statements" || kind == "control_structure_body") {
            result = children.size() == 1 && std::string(ts_node_type(children[0])) == "statements"
                ? emit(children[0]) : "{ " + join_statements(children) + " }";
        } else if (kind == "jump_expression") {
            std::string token = ts_node_type(ts_node_child(node, 0));
            if (token != "return" && token != "throw" && token != "break" && token != "continue") supported = false;
            else { result = token; for (auto child : children) result += " " + emit(child); }
        } else if (kind == "if_expression") {
            if (children.size() < 2 || children.size() > 3) supported = false;
            else {
                auto parent = ts_node_parent(node);
                std::string parent_kind = ts_node_type(parent);
                if (parent_kind != "statements" && parent_kind != "control_structure_body") supported = false;
                else {
                    result = "if (" + emit(children[0]) + ") " + emit(children[1]);
                    if (children.size() == 3) result += " else " + emit(children[2]);
                }
            }
        } else if (kind == "while_statement") {
            if (children.size() != 2) supported = false;
            else result = "while (" + emit(children[0]) + ") " + emit(children[1]);
        } else if (kind == "property_declaration") {
            auto variable = child_of(node, "variable_declaration");
            auto name = child_of(variable, "simple_identifier"), type = child_of(variable, "user_type");
            TSNode value{};
            for (auto child : children) if (!ts_node_eq(child, variable) && std::string(ts_node_type(child)) != "binding_pattern_kind" && std::string(ts_node_type(child)) != "modifiers") value = child;
            bool variable_supported = true;
            for (auto child : named(variable)) {
                std::string child_kind = ts_node_type(child);
                if (child_kind != "simple_identifier" && child_kind != "user_type") variable_supported = false;
            }
            auto modifiers = child_of(node, "modifiers");
            if (ts_node_is_null(name) || !variable_supported || !ts_node_is_null(modifiers)) supported = false;
            else {
                std::string declared = ts_node_is_null(type) ? "auto" : emit(type);
                if (ts_node_is_null(type) && !ts_node_is_null(value)) {
                    std::string value_kind = ts_node_type(value), raw = text(value);
                    if (value_kind == "integer_literal" || value_kind == "long_literal" || value_kind == "unsigned_literal" || value_kind == "hex_literal" || value_kind == "bin_literal") {
                        auto digits = normalized_number(raw); uint64_t number = 0;
                        auto parsed = std::from_chars(digits.data(), digits.data() + digits.size(), number);
                        if (parsed.ec == std::errc{} && parsed.ptr == digits.data() + digits.size()) {
                            bool unsigned_value = raw.find('u') != std::string::npos || raw.find('U') != std::string::npos;
                            bool large = raw.ends_with("L") || raw.ends_with("l") || number > (unsigned_value ? std::numeric_limits<uint32_t>::max() : static_cast<uint64_t>(std::numeric_limits<int32_t>::max()));
                            declared = unsigned_value ? (large ? "unsigned long long" : "unsigned int") : (large ? "long long" : "int");
                        } else supported = false;
                    } else if (value_kind == "real_literal") declared = raw.ends_with("f") || raw.ends_with("F") ? "float" : "double";
                    else if (value_kind == "boolean_literal") declared = "bool";
                    else if (value_kind == "string_literal") declared = "std::string";
                }
                auto binding = child_of(node, "binding_pattern_kind");
                if (!ts_node_is_null(binding) && text(binding) == "val") declared = "const " + declared;
                result = declared + " " + snake(text(name));
                if (!ts_node_is_null(value)) result += " = " + emit(value);
            }
        } else if (kind == "user_type" || kind == "type_arguments" || kind == "type_projection") {
            if (kind == "user_type" && cpp_primitive_type(text(node))) result = *cpp_primitive_type(text(node));
            else for (uint32_t i = 0; i < ts_node_child_count(node); ++i) {
                auto child = ts_node_child(node, i);
                result += ts_node_is_named(child) ? emit(child) : (text(child) == "." ? "::" : text(child));
            }
        } else if (kind == "type_identifier") {
            auto primitive = cpp_primitive_type(text(node)); result = primitive ? *primitive : text(node);
        } else if (kind == "simple_identifier") {
            if (text(node).starts_with("`")) supported = false;
            else result = snake(text(node));
        }
        else if (kind == "integer_literal" || kind == "long_literal" || kind == "real_literal" || kind == "hex_literal" || kind == "bin_literal" || kind == "boolean_literal") {
            result = text(node); result.erase(std::remove(result.begin(), result.end(), '_'), result.end());
            if (result.ends_with("L") || result.ends_with("l")) result += 'L';
        } else if (kind == "null_literal") result = "nullptr";
        else if (kind == "string_literal" || kind == "character_literal") {
            if (text(node).starts_with("\"\"\"") || text(node).find('$') != std::string::npos) supported = false;
            else result = text(node);
        } else if (kind == "comparison_expression" || kind == "equality_expression" || kind == "additive_expression" || kind == "multiplicative_expression" || kind == "conjunction_expression" || kind == "disjunction_expression" || kind == "parenthesized_expression" || kind == "prefix_expression" || kind == "postfix_expression" || kind == "assignment" || kind == "directly_assignable_expression") {
            for (uint32_t i = 0; i < ts_node_child_count(node); ++i) {
                auto child = ts_node_child(node, i); auto token = text(child);
                if (!ts_node_is_named(child) && (token == "===" || token == "!==" || token == "?." || token == "!!")) { supported = false; break; }
                result += ts_node_is_named(child) ? emit(child) : " " + token + " ";
            }
        } else if (kind == "call_expression" || kind == "call_suffix" || kind == "value_arguments" || kind == "value_argument" || kind == "navigation_expression" || kind == "navigation_suffix") {
            for (uint32_t i = 0; i < ts_node_child_count(node); ++i) {
                auto child = ts_node_child(node, i);
                if (!ts_node_is_named(child) && (text(child) == "?." || (kind == "value_argument" && text(child) == "="))) { supported = false; break; }
                result += ts_node_is_named(child) ? emit(child) : text(child);
            }
        } else supported = false;
        if (!supported) {
            result = "__ast_distance_unmapped__(\"" + json_escape(kind) + "\", \"" + json_escape(text(node)) + "\")";
            ++output.rule_misses;
            output.diagnostics.push_back(std::to_string(ts_node_start_point(node).row + 1) + ": unsupported " + kind);
        } else ++output.rule_hits;

        return result;
    }
private:
    const std::string& source_;
    std::set<std::string> function_names_;
    std::string text(TSNode node) const {
        if (ts_node_is_null(node)) return {};
        return source_.substr(ts_node_start_byte(node), ts_node_end_byte(node) - ts_node_start_byte(node));
    }
    std::vector<TSNode> named(TSNode node) const {
        std::vector<TSNode> result;
        if (!ts_node_is_null(node)) for (uint32_t i = 0; i < ts_node_named_child_count(node); ++i) result.push_back(ts_node_named_child(node, i));
        return result;
    }
    TSNode child_of(TSNode node, const std::string& kind) const {
        for (auto child : named(node)) if (ts_node_type(child) == kind) return child;
        return {};
    }
    std::string join_statements(const std::vector<TSNode>& nodes) {
        std::string result;
        for (auto node : nodes) {
            std::string fragment = emit(node);
            if (fragment.empty()) continue;
            result += fragment;
            std::string kind = ts_node_type(node);
            if (kind == "line_comment") { result += '\n'; continue; }
            if (kind == "multiline_comment") { result += ' '; continue; }
            if (!fragment.ends_with("}")) result += ';';
            result += ' ';
        }
        return result;
    }
};
float positional_cosine(const std::vector<std::string>& a, const std::vector<std::string>& b) {
    // One-hot coordinates are (token position, exact spelling). Each occupied
    // coordinate is 1, so the norms are sqrt(sequence length), not word counts.
    if (a.empty() || b.empty()) return a.empty() && b.empty() ? 1.0f : 0.0f;
    size_t matches = 0;
    for (size_t i = 0; i < std::min(a.size(), b.size()); ++i)
        if (a[i] == b[i]) ++matches;
    return static_cast<float>(matches / std::sqrt(static_cast<double>(a.size()) * b.size()));
}
float token_cosine(const std::string& a, const std::string& b) {
    auto tokens = [](const std::string& source, bool generated) {
        std::vector<std::string> result;
        auto parser = ts_parser_new(); ts_parser_set_language(parser, tree_sitter_cpp());
        auto tree = ts_parser_parse_string(parser, nullptr, source.data(), source.size());
        std::function<void(TSNode)> visit = [&](TSNode node) {
            const std::string kind = ts_node_type(node);
            if (kind == "comment") return;
            const auto text = source.substr(ts_node_start_byte(node), ts_node_end_byte(node) - ts_node_start_byte(node));
            // Fallback statements are evidence of absent replacement rules,
            // never a literal implementation to reward for matching itself.
            if (generated && (kind == "expression_statement" || kind == "call_expression") &&
                text.starts_with("__ast_distance_unmapped__(")) return;
            if (!ts_node_child_count(node) || kind == "string_literal" ||
                kind == "raw_string_literal" || kind == "char_literal") {
                if (!text.empty()) result.push_back(text);
            } else for (uint32_t i = 0; i < ts_node_child_count(node); ++i) visit(ts_node_child(node, i));
        };
        visit(ts_tree_root_node(tree)); ts_tree_delete(tree); ts_parser_delete(parser); return result;
    };
    auto left = tokens(a, true), right = tokens(b, false);
    // No executable evidence is not a successful comparison.
    return left.empty() || right.empty() ? 0.0f : positional_cosine(left, right);
}
float documentation_similarity(const std::string& source, Language source_language,
    const std::string& target, Language target_language) {
    ASTParser source_parser;
    std::set<std::string> function_names;
    for (const auto& function : source_parser.extract_function_infos(source, source_language)) function_names.insert(function.name);
    auto words = [&](const std::string& input, Language language) {
        std::vector<std::string> result;
        auto parser = ts_parser_new();
        ts_parser_set_language(parser, language == Language::KOTLIN ? tree_sitter_kotlin() : tree_sitter_cpp());
        auto adapted = language == Language::KOTLIN ? kotlin_grammar_input(input).text : input;
        auto tree = ts_parser_parse_string(parser, nullptr, adapted.data(), adapted.size());
        std::function<void(TSNode)> visit = [&](TSNode node) {
            auto kind = std::string(ts_node_type(node));
            if (kind == "comment" || kind == "line_comment" || kind == "multiline_comment") {
                auto raw = input.substr(ts_node_start_byte(node), ts_node_end_byte(node) - ts_node_start_byte(node));
                // Provenance/tool annotations are required metadata, not extra
                // upstream narrative. Exclude only anchored metadata lines.
                std::istringstream lines(raw);
                std::string narrative;
                for (std::string line; std::getline(lines, line);) {
                    std::string content = line;
                    auto first = content.find_first_not_of(" \t");
                    if (first != std::string::npos) content.erase(0, first);
                    if (content.starts_with("//")) content.erase(0, 2);
                    else if (content.starts_with("/**")) content.erase(0, 3);
                    else if (content.starts_with("/*")) content.erase(0, 2);
                    else if (content.starts_with("*")) content.erase(0, 1);
                    first = content.find_first_not_of(" \t");
                    if (first != std::string::npos) content.erase(0, first);
                    if (content.starts_with("Transliterated from:") || content.starts_with("port-lint:")) continue;
                    narrative += line + '\n';
                }
                raw = render_documentation(narrative, function_names).text;
                std::string word;
                for (size_t i = 0; i < raw.size(); ++i) {
                    if (raw.compare(i, 5, "\\ref ") == 0 || raw.compare(i, 3, "\\c ") == 0) {
                        if (!word.empty()) { result.push_back(word); word.clear(); }
                        i += raw.compare(i, 5, "\\ref ") == 0 ? 4 : 2;
                        continue;
                    }
                    auto c = raw[i];
                    if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') word += c;
                    else if (!word.empty()) { result.push_back(word); word.clear(); }
                }
                if (!word.empty()) result.push_back(word);
            } else for (uint32_t i = 0; i < ts_node_named_child_count(node); ++i) visit(ts_node_named_child(node, i));
        };
        visit(ts_tree_root_node(tree)); ts_tree_delete(tree); ts_parser_delete(parser); return result;
    };
    auto a = words(source, source_language), b = words(target, target_language);
    return positional_cosine(a, b);
}
} // namespace
TransliterationOutput transliterate(const std::string& source, Language source_language, Language target_language) {
    if (source_language != Language::KOTLIN || target_language != Language::CPP)
        throw std::runtime_error("AST emission currently supports Kotlin -> C++; unsupported language pair");
    auto compatible = kotlin_grammar_input(source);
    auto parser = ts_parser_new(); ts_parser_set_language(parser, tree_sitter_kotlin());
    auto tree = ts_parser_parse_string(parser, nullptr, compatible.text.data(), compatible.text.size());
    if (!tree) { ts_parser_delete(parser); throw std::runtime_error("Cannot parse transliteration source"); }
    KotlinCppEmitter emitter(source);
    emitter.output.buffer = emitter.emit(ts_tree_root_node(tree));
    if (ts_node_has_error(ts_tree_root_node(tree))) {
        emitter.output.diagnostics.push_back("Source grammar errors; emitted source is provisional");
        std::function<void(TSNode)> diagnose = [&](TSNode node) {
            if (ts_node_is_error(node) || ts_node_is_missing(node))
                emitter.output.diagnostics.push_back(std::to_string(ts_node_start_point(node).row + 1) + ": " +
                    (ts_node_is_missing(node) ? "MISSING " : "ERROR ") + ts_node_type(node) + " bytes " +
                    std::to_string(ts_node_start_byte(node)) + ":" + std::to_string(ts_node_end_byte(node)));
            for (uint32_t i = 0; i < ts_node_child_count(node); ++i) diagnose(ts_node_child(node, i));
        };
        diagnose(ts_tree_root_node(tree));
    }
    size_t accounted_bytes = 0, supported_bytes = 0;
    for (const auto& span : emitter.output.spans) {
        if (span.node_type == "line_comment" || span.node_type == "multiline_comment" || span.node_type == "package_header" || span.node_type == "import_list" || span.node_type == "import_header") continue;
        accounted_bytes += span.source_end - span.source_start;
        if (span.supported) supported_bytes += span.source_end - span.source_start;
    }
    emitter.output.rule_coverage = accounted_bytes ? static_cast<float>(supported_bytes) / accounted_bytes : 0;
    if (emitter.output.spans.empty() && !source.empty())
        emitter.output.spans.push_back({ts_node_type(ts_tree_root_node(tree)), 1, 0, source.size(), 0, emitter.output.buffer.size(), false});
    ts_tree_delete(tree); ts_parser_delete(parser); return std::move(emitter.output);
}
TransliterationDistance transliteration_distance(const std::string& source, Language source_language,
    const std::string& target, Language target_language) {
    TransliterationDistance result;
    result.translation = transliterate(source, source_language, target_language);
    ASTParser parser;
    auto original = parser.extract_function_infos(source, source_language);
    auto emitted = parser.extract_function_infos(result.translation.buffer, target_language);
    result.translated_parse_errors = parser.last_extraction_has_errors();
    auto actual = parser.extract_function_infos(target, target_language);
    result.target_parse_errors = parser.last_extraction_has_errors();
    struct Candidate { size_t source, target; const FunctionInfo* emitted; float score; };
    std::vector<Candidate> candidates;
    for (size_t i = 0; i < original.size(); ++i) {
        const FunctionInfo* transformed = nullptr;
        for (const auto& span : result.translation.spans) {
            if (span.node_type != "function_declaration" || span.source_line != original[i].start_line) continue;
            int first_line = 1 + std::count(result.translation.buffer.begin(), result.translation.buffer.begin() + span.target_start, '\n');
            int last_line = 1 + std::count(result.translation.buffer.begin(), result.translation.buffer.begin() + span.target_end, '\n');
            for (const auto& function : emitted)
                if (function.start_line >= first_line && function.start_line <= last_line &&
                    IdentifierStats::canonicalize(function.name) == IdentifierStats::canonicalize(original[i].name)) { transformed = &function; break; }
        }
        for (size_t j = 0; j < actual.size(); ++j) {
            if (!callable_owners_compatible(original[i], actual[j]) || IdentifierStats::canonicalize(original[i].name) != IdentifierStats::canonicalize(actual[j].name)) continue;
            float value = transformed ? ASTSimilarity::function_parameter_body_cosine_similarity(transformed->body_tree.get(), actual[j].body_tree.get(), transformed->identifiers, actual[j].identifiers) : 0;
            candidates.push_back({i, j, transformed, value});
        }
    }
    std::sort(candidates.begin(), candidates.end(), [](const auto& left, const auto& right) {
        if (left.score != right.score) return left.score > right.score;
        return std::tie(left.source, left.target) < std::tie(right.source, right.target);
    });
    std::vector<bool> used(actual.size(), false), source_used(original.size(), false);
    int matched = 0; float logic = 0;
    for (const auto& candidate : candidates) {
        if (used[candidate.target] || source_used[candidate.source]) continue;
        used[candidate.target] = source_used[candidate.source] = true; ++matched;
        float pair_logic = candidate.emitted ? normalized_logic_similarity(candidate.emitted->body_tree.get(), actual[candidate.target].body_tree.get()) : 0;
        logic += pair_logic;
        result.functions.push_back({original[candidate.source].qualified_name, actual[candidate.target].qualified_name,
            original[candidate.source].start_line, actual[candidate.target].start_line, pair_logic,
            candidate.emitted ? normalized_logic_tokens(candidate.emitted->body_tree.get()) : std::vector<std::string>{},
            normalized_logic_tokens(actual[candidate.target].body_tree.get())});
    }
    for (size_t i = 0; i < original.size(); ++i) if (!source_used[i]) result.missing_functions.push_back(original[i].qualified_name + ":" + std::to_string(original[i].start_line));
    for (size_t i = 0; i < actual.size(); ++i) if (!used[i]) result.extra_functions.push_back(actual[i].qualified_name + ":" + std::to_string(actual[i].start_line));
    result.symbol_parity = original.empty() ? 0 : static_cast<float>(matched) / original.size();
    result.normalized_logic = original.empty() ? 0 : logic / original.size();
    auto generated_tree = parser.parse_string(result.translation.buffer, target_language), target_tree = parser.parse_string(target, target_language);
    result.translated_ast_cosine = ASTSimilarity::histogram_cosine_similarity(generated_tree.get(), target_tree.get());
    result.translated_text_cosine = token_cosine(result.translation.buffer, target);
    result.documentation_parity = documentation_similarity(source, source_language, target, target_language);
    result.fallback_penalty = 1 - result.translation.rule_coverage;
    // The literal text cosine is the score. AST, logic, symbol and coverage
    // evidence are independent diagnostics, not weighted score substitutes.
    result.score = result.translated_text_cosine;
    return result;
}
} // namespace ast_distance
