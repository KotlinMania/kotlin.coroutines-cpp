#pragma once
#include "ast_parser.hpp"

namespace ast_distance {
inline std::string callable_type_key(std::string name) {
    // Generic argument syntax qualifies instances, not the owning type symbol.
    // This is matching only; generic arguments remain in the scored AST.
    std::string plain;
    int depth = 0;
    for (unsigned char c : name) {
        if (c == '<') { ++depth; continue; }
        if (c == '>') { if (depth) --depth; continue; }
        if (!depth && !std::isspace(c)) plain += static_cast<char>(c);
    }
    for (size_t position = 0; (position = plain.find('.', position)) != std::string::npos; position += 2) plain.replace(position, 1, "::");
    return IdentifierStats::canonicalize(plain);
}
inline bool callable_type_names_compatible(const std::string& left, const std::string& right) {
    auto a = callable_type_key(left), b = callable_type_key(right);
    return !a.empty() && !b.empty() && (a == b || a.ends_with("::" + b) || b.ends_with("::" + a));
}
inline std::string callable_owner_key(const FunctionInfo& function) {
    auto separator = function.qualified_name.rfind("::");
    if (separator == std::string::npos) return {};
    std::string owner = function.qualified_name.substr(0, separator);
    for (size_t position = 0; (position = owner.find('.', position)) != std::string::npos; position += 2)
        owner.replace(position, 1, "::");
    // Kotlin companion members lower to static members of the containing class.
    if (owner.ends_with("::Companion")) owner.erase(owner.size() - 11);
    return callable_type_key(owner);
}
inline bool extension_argument_counts_compatible(const FunctionInfo& extension, const FunctionInfo& lowered) {
    if (extension.explicit_parameter_count < 0 || lowered.explicit_parameter_count < 0) return true;
    // The free-function receiver and a trailing erased-ABI continuation are
    // carriers, not replacements for Kotlin's explicit user arguments.
    int user_arguments = lowered.explicit_parameter_count - 1;
    // Only an actually suspend declaration gains an implicit ABI argument.
    // A user-supplied Continuation in a non-suspend API is an ordinary argument.
    if (extension.is_suspend_function && lowered.trailing_continuation_parameter) --user_arguments;
    return user_arguments == extension.explicit_parameter_count;
}
inline bool callable_owners_compatible(const FunctionInfo& source, const FunctionInfo& target) {
    // Local methods/functions keep the enclosing callable as identity evidence.
    // Reused anonymous/named collector classes in separate operators must not
    // match merely because their methods share a spelling or similar bodies.
    if (!source.enclosing_functions.empty() && !target.enclosing_functions.empty()) {
        if (source.enclosing_functions.size() != target.enclosing_functions.size()) return false;
        for (size_t i = 0; i < source.enclosing_functions.size(); ++i)
            if (IdentifierStats::canonicalize(source.enclosing_functions[i]) !=
                IdentifierStats::canonicalize(target.enclosing_functions[i])) return false;
    }
    if (!source.extension_receiver.empty() && target.is_namespace_function)
        return callable_type_names_compatible(source.extension_receiver, target.first_parameter_type) &&
            extension_argument_counts_compatible(source, target);
    if (!target.extension_receiver.empty() && source.is_namespace_function)
        return callable_type_names_compatible(target.extension_receiver, source.first_parameter_type) &&
            extension_argument_counts_compatible(target, source);
    // A C++ enum cannot contain member functions. Its Kotlin instance methods
    // lower to namespace functions with the enum as the explicit first argument.
    // Require CST enum identity, the actual receiver type and argument inventory;
    // an ordinary class or a same-spelled function on another enum is not enough.
    auto enum_projection = [](const FunctionInfo& member, const FunctionInfo& lowered) {
        return member.is_enum_member && member.enclosing_functions.empty() &&
            lowered.is_namespace_function && !lowered.has_class_owner &&
            callable_type_names_compatible(callable_owner_key(member), lowered.first_parameter_type) &&
            extension_argument_counts_compatible(member, lowered);
    };
    if (source.is_enum_member && target.is_namespace_function) return enum_projection(source, target);
    if (target.is_enum_member && source.is_namespace_function) return enum_projection(target, source);
    // A receiver extension cannot be hidden inside an unrelated class method.
    // Companion extensions are the explicit static-member lowering exception.
    if (!source.extension_receiver.empty() && target.has_class_owner && !source.extension_receiver.ends_with(".Companion")) return false;
    if (!target.extension_receiver.empty() && source.has_class_owner && !target.extension_receiver.ends_with(".Companion")) return false;
    auto left = callable_owner_key(source), right = callable_owner_key(target);
    if (left.empty() || right.empty()) return true; // Report ownerless extension/free-function lowering separately.
    return left == right || left.ends_with("::" + right) || right.ends_with("::" + left);
}
} // namespace ast_distance
