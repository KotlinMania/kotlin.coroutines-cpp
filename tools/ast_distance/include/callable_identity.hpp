#pragma once
#include "ast_parser.hpp"

namespace ast_distance {
inline std::string callable_owner_key(const FunctionInfo& function) {
    auto separator = function.qualified_name.rfind("::");
    if (separator == std::string::npos) return {};
    std::string owner = function.qualified_name.substr(0, separator);
    for (size_t position = 0; (position = owner.find('.', position)) != std::string::npos; position += 2)
        owner.replace(position, 1, "::");
    // Kotlin companion members lower to static members of the containing class.
    if (owner.ends_with("::Companion")) owner.erase(owner.size() - 11);
    return IdentifierStats::canonicalize(owner);
}
inline bool callable_owners_compatible(const FunctionInfo& source, const FunctionInfo& target) {
    auto left = callable_owner_key(source), right = callable_owner_key(target);
    if (left.empty() || right.empty()) return true; // Report ownerless extension/free-function lowering separately.
    return left == right || left.ends_with("::" + right) || right.ends_with("::" + left);
}
} // namespace ast_distance
