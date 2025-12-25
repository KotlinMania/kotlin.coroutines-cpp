#pragma once

#include <string>
#include <unordered_map>

namespace ast_distance {

/**
 * Normalized AST node types that map across Rust and Kotlin.
 * Based on the ASTERIA paper's categorization (Table I).
 */
enum class NodeType : int {
    // Statement nodes (control flow)
    BLOCK = 0,
    IF = 1,
    FOR = 2,
    WHILE = 3,
    SWITCH = 4,       // match in Rust, when in Kotlin
    RETURN = 5,
    GOTO = 6,         // break/continue with label
    CONTINUE = 7,
    BREAK = 8,
    TRY = 9,
    THROW = 10,

    // Expression nodes - assignments
    ASSIGN = 11,
    ASSIGN_ADD = 12,
    ASSIGN_SUB = 13,
    ASSIGN_MUL = 14,
    ASSIGN_DIV = 15,
    ASSIGN_MOD = 16,
    ASSIGN_AND = 17,
    ASSIGN_OR = 18,
    ASSIGN_XOR = 19,

    // Expression nodes - comparisons
    EQ = 20,
    NE = 21,
    GT = 22,
    LT = 23,
    GE = 24,
    LE = 25,

    // Expression nodes - arithmetic
    ADD = 26,
    SUB = 27,
    MUL = 28,
    DIV = 29,
    MOD = 30,
    NEG = 31,

    // Expression nodes - bitwise
    BIT_AND = 32,
    BIT_OR = 33,
    BIT_XOR = 34,
    BIT_NOT = 35,
    SHL = 36,
    SHR = 37,

    // Expression nodes - logical
    AND = 38,
    OR = 39,
    NOT = 40,

    // Expression nodes - other
    INDEX = 41,
    FIELD_ACCESS = 42,
    CALL = 43,
    METHOD_CALL = 44,
    LAMBDA = 45,
    TERNARY = 46,
    CAST = 47,
    RANGE = 48,

    // Literals and identifiers
    VARIABLE = 50,
    NUMBER = 51,
    STRING = 52,
    BOOLEAN = 53,
    NULL_LIT = 54,
    CHAR = 55,

    // Declarations
    FUNCTION = 60,
    CLASS = 61,
    STRUCT = 62,
    ENUM = 63,
    INTERFACE = 64,   // trait in Rust
    VAR_DECL = 65,
    PARAM = 66,
    TYPE_PARAM = 67,

    // Type annotations
    TYPE_REF = 70,
    ARRAY_TYPE = 71,
    NULLABLE_TYPE = 72,
    FUNC_TYPE = 73,
    GENERIC_TYPE = 74,

    // Other
    COMMENT = 80,
    IMPORT = 81,
    PACKAGE = 82,
    ANNOTATION = 83,
    MODIFIER = 84,

    // Unknown/unhandled
    UNKNOWN = 99,

    NUM_TYPES = 100
};

/**
 * Rust AST node type mappings (from tree-sitter-rust)
 */
inline NodeType rust_node_to_type(const std::string& node_type) {
    static const std::unordered_map<std::string, NodeType> mapping = {
        // Statements
        {"block", NodeType::BLOCK},
        {"if_expression", NodeType::IF},
        {"if_let_expression", NodeType::IF},
        {"for_expression", NodeType::FOR},
        {"while_expression", NodeType::WHILE},
        {"while_let_expression", NodeType::WHILE},
        {"loop_expression", NodeType::WHILE},
        {"match_expression", NodeType::SWITCH},
        {"return_expression", NodeType::RETURN},
        {"continue_expression", NodeType::CONTINUE},
        {"break_expression", NodeType::BREAK},
        {"try_expression", NodeType::TRY},

        // Assignments
        {"assignment_expression", NodeType::ASSIGN},
        {"compound_assignment_expr", NodeType::ASSIGN},

        // Comparisons
        {"binary_expression", NodeType::UNKNOWN},  // Need to check operator

        // Literals
        {"identifier", NodeType::VARIABLE},
        {"integer_literal", NodeType::NUMBER},
        {"float_literal", NodeType::NUMBER},
        {"string_literal", NodeType::STRING},
        {"boolean_literal", NodeType::BOOLEAN},
        {"char_literal", NodeType::CHAR},

        // Function/Method calls
        {"call_expression", NodeType::CALL},
        {"method_call_expression", NodeType::METHOD_CALL},

        // Declarations
        {"function_item", NodeType::FUNCTION},
        {"struct_item", NodeType::STRUCT},
        {"enum_item", NodeType::ENUM},
        {"trait_item", NodeType::INTERFACE},
        {"impl_item", NodeType::CLASS},
        {"let_declaration", NodeType::VAR_DECL},
        {"parameter", NodeType::PARAM},
        {"type_parameter", NodeType::TYPE_PARAM},

        // Field/Index access
        {"field_expression", NodeType::FIELD_ACCESS},
        {"index_expression", NodeType::INDEX},

        // Closure
        {"closure_expression", NodeType::LAMBDA},

        // Types
        {"type_identifier", NodeType::TYPE_REF},
        {"array_type", NodeType::ARRAY_TYPE},
        {"generic_type", NodeType::GENERIC_TYPE},

        // Other
        {"use_declaration", NodeType::IMPORT},
        {"attribute_item", NodeType::ANNOTATION},
        {"line_comment", NodeType::COMMENT},
        {"block_comment", NodeType::COMMENT},
    };

    auto it = mapping.find(node_type);
    return it != mapping.end() ? it->second : NodeType::UNKNOWN;
}

/**
 * Kotlin AST node type mappings (from tree-sitter-kotlin)
 */
inline NodeType kotlin_node_to_type(const std::string& node_type) {
    static const std::unordered_map<std::string, NodeType> mapping = {
        // Statements
        {"statements", NodeType::BLOCK},
        {"control_structure_body", NodeType::BLOCK},
        {"if_expression", NodeType::IF},
        {"for_statement", NodeType::FOR},
        {"while_statement", NodeType::WHILE},
        {"do_while_statement", NodeType::WHILE},
        {"when_expression", NodeType::SWITCH},
        {"jump_expression", NodeType::RETURN},  // return, throw, break, continue
        {"try_expression", NodeType::TRY},

        // Assignments
        {"assignment", NodeType::ASSIGN},
        {"directly_assignable_expression", NodeType::ASSIGN},

        // Comparisons - handled in binary expressions

        // Literals
        {"simple_identifier", NodeType::VARIABLE},
        {"integer_literal", NodeType::NUMBER},
        {"long_literal", NodeType::NUMBER},
        {"real_literal", NodeType::NUMBER},
        {"string_literal", NodeType::STRING},
        {"boolean_literal", NodeType::BOOLEAN},
        {"character_literal", NodeType::CHAR},
        {"null_literal", NodeType::NULL_LIT},

        // Function/Method calls
        {"call_expression", NodeType::CALL},
        {"navigation_expression", NodeType::FIELD_ACCESS},
        {"indexing_expression", NodeType::INDEX},

        // Declarations
        {"function_declaration", NodeType::FUNCTION},
        {"class_declaration", NodeType::CLASS},
        {"object_declaration", NodeType::CLASS},
        {"enum_class_body", NodeType::ENUM},
        {"interface_declaration", NodeType::INTERFACE},
        {"property_declaration", NodeType::VAR_DECL},
        {"variable_declaration", NodeType::VAR_DECL},
        {"parameter", NodeType::PARAM},
        {"type_parameter", NodeType::TYPE_PARAM},

        // Lambda
        {"lambda_literal", NodeType::LAMBDA},
        {"anonymous_function", NodeType::LAMBDA},

        // Types
        {"user_type", NodeType::TYPE_REF},
        {"nullable_type", NodeType::NULLABLE_TYPE},
        {"function_type", NodeType::FUNC_TYPE},

        // Other
        {"import_header", NodeType::IMPORT},
        {"package_header", NodeType::PACKAGE},
        {"annotation", NodeType::ANNOTATION},
        {"modifier", NodeType::MODIFIER},
        {"multiline_comment", NodeType::COMMENT},
        {"line_comment", NodeType::COMMENT},
    };

    auto it = mapping.find(node_type);
    return it != mapping.end() ? it->second : NodeType::UNKNOWN;
}

/**
 * C++ AST node type mappings (from tree-sitter-cpp)
 */
inline NodeType cpp_node_to_type(const std::string& node_type) {
    static const std::unordered_map<std::string, NodeType> mapping = {
        // Statements
        {"compound_statement", NodeType::BLOCK},
        {"if_statement", NodeType::IF},
        {"for_statement", NodeType::FOR},
        {"for_range_loop", NodeType::FOR},
        {"while_statement", NodeType::WHILE},
        {"do_statement", NodeType::WHILE},
        {"switch_statement", NodeType::SWITCH},
        {"return_statement", NodeType::RETURN},
        {"continue_statement", NodeType::CONTINUE},
        {"break_statement", NodeType::BREAK},
        {"try_statement", NodeType::TRY},
        {"throw_statement", NodeType::THROW},
        {"goto_statement", NodeType::GOTO},

        // Assignments
        {"assignment_expression", NodeType::ASSIGN},
        {"compound_assignment_expr", NodeType::ASSIGN},

        // Comparisons and binary ops
        {"binary_expression", NodeType::UNKNOWN},
        {"conditional_expression", NodeType::TERNARY},
        {"unary_expression", NodeType::UNKNOWN},

        // Literals
        {"identifier", NodeType::VARIABLE},
        {"field_identifier", NodeType::VARIABLE},
        {"namespace_identifier", NodeType::VARIABLE},
        {"type_identifier", NodeType::TYPE_REF},
        {"number_literal", NodeType::NUMBER},
        {"string_literal", NodeType::STRING},
        {"raw_string_literal", NodeType::STRING},
        {"char_literal", NodeType::CHAR},
        {"true", NodeType::BOOLEAN},
        {"false", NodeType::BOOLEAN},
        {"nullptr", NodeType::NULL_LIT},

        // Function/Method calls
        {"call_expression", NodeType::CALL},
        {"field_expression", NodeType::FIELD_ACCESS},
        {"subscript_expression", NodeType::INDEX},

        // Declarations
        {"function_definition", NodeType::FUNCTION},
        {"function_declarator", NodeType::FUNCTION},
        {"class_specifier", NodeType::CLASS},
        {"struct_specifier", NodeType::STRUCT},
        {"enum_specifier", NodeType::ENUM},
        {"declaration", NodeType::VAR_DECL},
        {"init_declarator", NodeType::VAR_DECL},
        {"parameter_declaration", NodeType::PARAM},
        {"template_parameter_list", NodeType::TYPE_PARAM},

        // Templates (generics)
        {"template_declaration", NodeType::GENERIC_TYPE},
        {"template_type", NodeType::GENERIC_TYPE},

        // Lambda
        {"lambda_expression", NodeType::LAMBDA},

        // Types
        {"primitive_type", NodeType::TYPE_REF},
        {"qualified_identifier", NodeType::TYPE_REF},
        {"pointer_declarator", NodeType::TYPE_REF},
        {"reference_declarator", NodeType::TYPE_REF},
        {"array_declarator", NodeType::ARRAY_TYPE},

        // Namespaces and includes
        {"preproc_include", NodeType::IMPORT},
        {"using_declaration", NodeType::IMPORT},
        {"namespace_definition", NodeType::PACKAGE},

        // Other
        {"comment", NodeType::COMMENT},
        {"attribute", NodeType::ANNOTATION},
        {"storage_class_specifier", NodeType::MODIFIER},
        {"type_qualifier", NodeType::MODIFIER},
    };

    auto it = mapping.find(node_type);
    return it != mapping.end() ? it->second : NodeType::UNKNOWN;
}

/**
 * Get human-readable name for a node type
 */
inline const char* node_type_name(NodeType type) {
    switch (type) {
        case NodeType::BLOCK: return "BLOCK";
        case NodeType::IF: return "IF";
        case NodeType::FOR: return "FOR";
        case NodeType::WHILE: return "WHILE";
        case NodeType::SWITCH: return "SWITCH";
        case NodeType::RETURN: return "RETURN";
        case NodeType::CONTINUE: return "CONTINUE";
        case NodeType::BREAK: return "BREAK";
        case NodeType::TRY: return "TRY";
        case NodeType::THROW: return "THROW";
        case NodeType::ASSIGN: return "ASSIGN";
        case NodeType::EQ: return "EQ";
        case NodeType::NE: return "NE";
        case NodeType::GT: return "GT";
        case NodeType::LT: return "LT";
        case NodeType::ADD: return "ADD";
        case NodeType::SUB: return "SUB";
        case NodeType::MUL: return "MUL";
        case NodeType::DIV: return "DIV";
        case NodeType::CALL: return "CALL";
        case NodeType::METHOD_CALL: return "METHOD_CALL";
        case NodeType::VARIABLE: return "VARIABLE";
        case NodeType::NUMBER: return "NUMBER";
        case NodeType::STRING: return "STRING";
        case NodeType::FUNCTION: return "FUNCTION";
        case NodeType::CLASS: return "CLASS";
        case NodeType::STRUCT: return "STRUCT";
        case NodeType::VAR_DECL: return "VAR_DECL";
        case NodeType::LAMBDA: return "LAMBDA";
        case NodeType::FIELD_ACCESS: return "FIELD_ACCESS";
        case NodeType::INDEX: return "INDEX";
        case NodeType::UNKNOWN: return "UNKNOWN";
        default: return "OTHER";
    }
}

} // namespace ast_distance
