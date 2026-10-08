#pragma once
#include "ast_parser.hpp"
#include <charconv>
#include <iomanip>
#include <limits>
#include <sstream>

namespace ast_distance {
// Concrete syntax wrappers do not participate. Logic tokens remain ordered,
// including exact operator identity, literals and all variable/call spellings.
inline std::string normalized_number(std::string value) {
    value.erase(std::remove_if(value.begin(), value.end(), [](char c) { return c == '_' || c == '\''; }), value.end());
    // Integer width/signedness suffixes are target syntax, not the literal value.
    if (value.find_first_of(".pP") == std::string::npos &&
        ((value.starts_with("0x") || value.starts_with("0X")) || value.find_first_of("eE") == std::string::npos)) {
        while (!value.empty() && (value.back() == 'u' || value.back() == 'U' || value.back() == 'l' || value.back() == 'L')) value.pop_back();
        int base = 10; size_t offset = 0;
        if (value.starts_with("0x") || value.starts_with("0X")) { base = 16; offset = 2; }
        else if (value.starts_with("0b") || value.starts_with("0B")) { base = 2; offset = 2; }
        else if (value.size() > 1 && value.front() == '0') { base = 8; offset = 1; }
        uint64_t number = 0;
        auto parsed = std::from_chars(value.data() + offset, value.data() + value.size(), number, base);
        if (parsed.ec == std::errc{} && parsed.ptr == value.data() + value.size()) return std::to_string(number);
    }
    return value;
}
inline std::string normalized_type_token(std::string value) {
    static const std::map<std::string, std::string> primitive = {
        {"Int", "signed32"}, {"int", "signed32"}, {"i32", "signed32"}, {"int32_t", "signed32"},
        {"unsigned", "unsigned32"}, {"UInt", "unsigned32"}, {"unsigned int", "unsigned32"}, {"u32", "unsigned32"}, {"uint32_t", "unsigned32"},
        {"Long", "signed64"}, {"long long", "signed64"}, {"i64", "signed64"}, {"int64_t", "signed64"},
        {"ULong", "unsigned64"}, {"unsigned long long", "unsigned64"}, {"u64", "unsigned64"}, {"uint64_t", "unsigned64"},
        {"Short", "signed16"}, {"short", "signed16"}, {"i16", "signed16"},
        {"Byte", "signed8"}, {"signed char", "signed8"}, {"i8", "signed8"},
        {"Boolean", "bool"}, {"bool", "bool"}, {"Double", "float64"}, {"double", "float64"},
        {"Float", "float32"}, {"float", "float32"}, {"Unit", "void"}, {"void", "void"}
    };
    auto found = primitive.find(value);
    return found == primitive.end() ? IdentifierStats::canonicalize(value) : found->second;
}
inline std::vector<std::string> normalized_logic_tokens(Tree* tree) {
    std::vector<std::string> tokens;
    std::function<void(Tree*)> visit = [&](Tree* node) {
        auto kind = static_cast<NodeType>(node->node_type);
        if (kind == NodeType::TYPE_REF && node->label.starts_with("primitive:")) {
            tokens.push_back("type:" + normalized_type_token(node->label.substr(10)));
            return;
        }
        bool literal = kind == NodeType::NUMBER || kind == NodeType::STRING ||
            kind == NodeType::CHAR || kind == NodeType::BOOLEAN || kind == NodeType::NULL_LIT;
        if (literal) {
            auto label = kind == NodeType::NUMBER ? normalized_number(node->label) : node->label;
            if (kind == NodeType::NULL_LIT) label = "null";
            tokens.push_back("literal:" + std::to_string(node->node_type) + ":" + label);
            // Literal fragments are grammar wrappers; interpolated identifiers are
            // still represented by the full literal and identifier metric.
            return;
        }
        if (kind == NodeType::VARIABLE && node->children.empty() && node->label != "this")
            tokens.push_back("identifier:" + IdentifierStats::canonicalize(node->label));
        if (kind == NodeType::TYPE_REF && node->children.empty())
            tokens.push_back("type:" + normalized_type_token(node->label));
        if (kind == NodeType::ARITHMETIC_OP || kind == NodeType::COMPARISON_OP ||
            kind == NodeType::LOGICAL_OP || kind == NodeType::BITWISE_OP || kind == NodeType::ASSIGNMENT_OP)
            tokens.push_back("operator:" + node->label);
        if (kind == NodeType::IF || kind == NodeType::WHILE || kind == NodeType::FOR ||
            kind == NodeType::SWITCH || kind == NodeType::RETURN || kind == NodeType::THROW ||
            kind == NodeType::BREAK || kind == NodeType::CONTINUE || kind == NodeType::TRY ||
            kind == NodeType::CALL || kind == NodeType::PARAM || kind == NodeType::LAMBDA)
            tokens.push_back("node:" + std::to_string(node->node_type));
        size_t parameter_start = tokens.size();
        for (const auto& child : node->children) visit(child.get());
        if (kind == NodeType::PARAM || kind == NodeType::VAR_DECL) {
            // C++ spells type before name; Kotlin spells name before type.
            // Sort declaration name/type runs only, preserving default-expression
            // operators/calls/literals and the order of distinct parameters.
            size_t begin = parameter_start;
            while (begin < tokens.size()) {
                auto declaration_token = [](const std::string& token) {
                    return token.starts_with("identifier:") || token.starts_with("type:");
                };
                if (!declaration_token(tokens[begin])) { ++begin; continue; }
                size_t end = begin + 1;
                while (end < tokens.size() && declaration_token(tokens[end])) ++end;
                std::sort(tokens.begin() + begin, tokens.begin() + end);
                begin = end;
            }
        }
    };
    if (tree) visit(tree);
    return tokens;
}
inline float normalized_logic_similarity(Tree* first, Tree* second) {
    auto left = normalized_logic_tokens(first), right = normalized_logic_tokens(second);
    if (left.empty() && right.empty()) return 1.0f;
    if (left.empty() || right.empty()) return 0.0f;
    // Preserve all evidence. For very large functions use exact ordered n-gram
    // overlap rather than dropping nodes to bound edit-distance memory/work.
    if (left.size() > 2000 || right.size() > 2000) {
        auto grams = [](const std::vector<std::string>& tokens) {
            std::map<std::string, int> counts;
            for (size_t i = 0; i < tokens.size(); ++i)
                ++counts[tokens[i] + (i + 1 < tokens.size() ? "\n" + tokens[i + 1] : "")];
            return counts;
        };
        auto a = grams(left), b = grams(right);
        size_t common = 0;
        for (const auto& [token, count] : a) { auto it = b.find(token); if (it != b.end()) common += std::min(count, it->second); }
        return 2.0f * common / (left.size() + right.size());
    }
    std::vector<size_t> previous(right.size() + 1), current(right.size() + 1);
    for (size_t j = 0; j <= right.size(); ++j) previous[j] = j;
    for (size_t i = 1; i <= left.size(); ++i) {
        current[0] = i;
        for (size_t j = 1; j <= right.size(); ++j)
            current[j] = std::min({previous[j] + 1, current[j - 1] + 1,
                previous[j - 1] + (left[i - 1] == right[j - 1] ? 0 : 1)});
        previous.swap(current);
    }
    return 1.0f - static_cast<float>(previous.back()) / std::max(left.size(), right.size());
}
} // namespace ast_distance
