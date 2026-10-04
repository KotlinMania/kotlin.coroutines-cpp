#pragma once
#include "ast_parser.hpp"
#include <functional>

namespace ast_distance {
struct CppReviewType {
    std::string name;
    std::string kind;
    int line = 0;
    std::string category;
    std::string reason;
};
struct CppReview {
    std::vector<CppReviewType> types;
    bool has_implementation = false;
    bool has_declarations = false;
    bool has_parse_errors = false;
    std::string file_category;
    std::string file_reason;
};
inline CppReview review_cpp(const std::string& source) {
    CppReview result;
    TSParser* parser = ts_parser_new();
    if (!parser || !ts_parser_set_language(parser, tree_sitter_cpp())) {
        if (parser) ts_parser_delete(parser);
        throw std::runtime_error("Cannot initialize C++ grammar");
    }
    TSTree* tree = ts_parser_parse_string(parser, nullptr, source.data(), source.size());
    if (!tree) { ts_parser_delete(parser); throw std::runtime_error("Cannot parse C++ review input"); }
    auto text = [&](TSNode node) {
        return source.substr(ts_node_start_byte(node), ts_node_end_byte(node) - ts_node_start_byte(node));
    };
    result.has_parse_errors = ts_node_has_error(ts_tree_root_node(tree));
    std::function<void(TSNode)> walk = [&](TSNode node) {
        std::string kind = ts_node_type(node);
        if (kind == "function_definition") {
            TSNode body = ts_node_child_by_field_name(node, "body", 4);
            result.has_implementation |= !ts_node_is_null(body);
            result.has_declarations |= ts_node_is_null(body);
        }
        if (kind == "template_instantiation" || kind == "enum_specifier" ||
            kind == "class_specifier" || kind == "struct_specifier") result.has_implementation = true;
        if (kind == "alias_declaration" || kind == "type_definition") result.has_declarations = true;
        if (kind == "declaration") {
            bool callable = false, external = false;
            std::function<void(TSNode)> inspect = [&](TSNode n) {
                callable |= std::string(ts_node_type(n)) == "function_declarator";
                external |= std::string(ts_node_type(n)) == "storage_class_specifier" && text(n) == "extern";
                for (uint32_t j = 0; j < ts_node_named_child_count(n); ++j) inspect(ts_node_named_child(n, j));
            };
            inspect(node);
            result.has_declarations |= callable || external;
            result.has_implementation |= !callable && !external;
        }
        if (kind == "class_specifier" || kind == "struct_specifier") {
            TSNode name = ts_node_child_by_field_name(node, "name", 4);
            TSNode body = ts_node_child_by_field_name(node, "body", 4);
            if (!ts_node_is_null(name) && !ts_node_is_null(body)) {
                CppReviewType type{text(name), kind == "class_specifier" ? "class" : "struct",
                    static_cast<int>(ts_node_start_point(node).row) + 1, "implemented_type", ""};
                bool substantive = false, destructor_only = false, pure_virtual = false, inherited = false;
                for (uint32_t i = 0; i < ts_node_named_child_count(node); ++i)
                    inherited |= std::string(ts_node_type(ts_node_named_child(node, i))) == "base_class_clause";
                for (uint32_t i = 0; i < ts_node_named_child_count(body); ++i) {
                    TSNode member = ts_node_named_child(body, i);
                    std::string member_kind = ts_node_type(member);
                    if (member_kind == "comment" || member_kind == "access_specifier") continue;
                    std::string member_text = text(member);
                    bool destructor = member_text.find("~" + type.name + "(") != std::string::npos;
                    bool is_pure = false;
                    std::function<void(TSNode)> inspect = [&](TSNode n) {
                        is_pure |= std::string(ts_node_type(n)) == "pure_virtual_clause";
                        for (uint32_t j = 0; j < ts_node_named_child_count(n); ++j) inspect(ts_node_named_child(n, j));
                    };
                    inspect(member);
                    pure_virtual |= is_pure;
                    destructor_only |= destructor;
                    substantive |= !destructor;
                }
                if (pure_virtual) {
                    type.category = "interface";
                    type.reason = "Contains pure virtual declarations; implementation belongs to concrete types";
                } else if (!substantive && inherited) {
                    type.category = "inherited_marker_review";
                    type.reason = "No own behavior beyond a destructor; inherited API or marker role requires upstream review";
                } else if (!substantive) {
                    type.category = destructor_only ? "destructor_only_review" : "empty_type_review";
                    type.reason = "No members implementing behavior; may be an intentional base or marker, verify against upstream";
                }
                result.types.push_back(std::move(type));
            }
        }
        for (uint32_t i = 0; i < ts_node_named_child_count(node); ++i) walk(ts_node_named_child(node, i));
    };
    walk(ts_tree_root_node(tree));
    if (result.has_parse_errors) {
        result.file_category = "parse_error_review";
        result.file_reason = "C++ grammar reports errors; inventory is incomplete";
    } else if (!result.has_implementation) {
        result.file_category = result.has_declarations ? "declarations_only_review" : "implementation_not_in_translation_unit";
        result.file_reason = "No definitions or explicit instantiations detected; inspect companion headers/platform files before calling this missing";
    }
    ts_tree_delete(tree);
    ts_parser_delete(parser);
    return result;
}
} // namespace ast_distance
