#include "symbol_extraction.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <set>
#include <cctype>

namespace fs = std::filesystem;

namespace ast_distance {

// ============================================================================
// Name conversion utilities
// ============================================================================

std::string snake_to_camel(const std::string& snake) {
    std::string name = snake;
    if (name.rfind("r#", 0) == 0) {
        name = name.substr(2);
    }
    while (!name.empty() && name[0] == '_') {
        name.erase(name.begin());
    }
    if (name.empty()) {
        return "";
    }

    std::vector<std::string> parts;
    std::string current;
    for (char c : name) {
        if (c == '_') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
            continue;
        }
        if (std::isalnum(static_cast<unsigned char>(c))) {
            current.push_back(c);
        }
    }
    if (!current.empty()) {
        parts.push_back(current);
    }
    if (parts.empty()) {
        return "";
    }

    auto lowercase_part = [](const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return out;
    };
    auto title_part = [](const std::string& s) {
        if (s.empty()) return s;
        std::string out;
        out.reserve(s.size());
        out += static_cast<char>(std::toupper(static_cast<unsigned char>(s[0])));
        for (size_t i = 1; i < s.size(); ++i) {
            out += static_cast<char>(std::tolower(static_cast<unsigned char>(s[i])));
        }
        return out;
    };

    std::string result = lowercase_part(parts[0]);
    for (size_t i = 1; i < parts.size(); ++i) {
        result += title_part(parts[i]);
    }
    return result;
}

std::string rust_qualified_to_kotlin(const std::string& qualified) {
    // "Foo::bar" -> "Foo.bar"
    std::string result = qualified;
    size_t pos = 0;
    while ((pos = result.find("::", pos)) != std::string::npos) {
        result.replace(pos, 2, ".");
        pos += 1;
    }
    return result;
}

namespace {

// ============================================================================
// File utilities
// ============================================================================

bool should_skip_path(const std::string& path) {
    return path.find("/CMakeFiles/") != std::string::npos ||
           path.find("/cmake-build") != std::string::npos ||
           path.find("/_deps/") != std::string::npos ||
           path.find("/.gradle/") != std::string::npos ||
           path.find("/node_modules/") != std::string::npos ||
           path.find("/build/reports/") != std::string::npos ||
           path.find("/build/classes/") != std::string::npos ||
           path.find("/build/generated/") != std::string::npos ||
           path.find("/build/kotlin/") != std::string::npos ||
           path.find("/build/tmp/") != std::string::npos ||
           path.find("/target/debug/") != std::string::npos ||
           path.find("/target/release/") != std::string::npos;
}

std::string read_file_content(const fs::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) return {};
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string get_node_text(TSNode node, const std::string& source) {
    uint32_t start = ts_node_start_byte(node);
    uint32_t end = ts_node_end_byte(node);
    if (end > start && end <= source.length()) {
        return source.substr(start, end - start);
    }
    return {};
}

int get_node_line(TSNode node) {
    return static_cast<int>(ts_node_start_point(node).row) + 1;
}

// Find a named child of a given type
TSNode find_child_by_type(TSNode node, const char* type) {
    uint32_t count = ts_node_child_count(node);
    for (uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_child(node, i);
        if (std::string(ts_node_type(child)) == type) {
            return child;
        }
    }
    return ts_node_child(node, 0);  // Return first child as fallback (check with ts_node_is_null)
}

// Check if node has a child of type
bool has_child_of_type(TSNode node, const char* type) {
    uint32_t count = ts_node_child_count(node);
    for (uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_child(node, i);
        if (std::string(ts_node_type(child)) == type) {
            return true;
        }
    }
    return false;
}

// ============================================================================
// Rust symbol extraction
// ============================================================================

Visibility rust_extract_visibility(TSNode node) {
    uint32_t count = ts_node_child_count(node);
    for (uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_child(node, i);
        std::string type(ts_node_type(child));
        if (type == "visibility_modifier") {
            std::string text = "";
            uint32_t start = ts_node_start_byte(child);
            uint32_t end = ts_node_end_byte(child);
            // We need the source, but we don't have it here.
            // Use child structure: pub(crate) has a child node
            if (ts_node_child_count(child) > 1) {
                // pub(crate) or pub(super) etc
                return Visibility::CRATE;
            }
            return Visibility::PUBLIC;
        }
    }
    return Visibility::PRIVATE;
}

std::string rust_extract_name(TSNode node, const std::string& source, const char* name_type) {
    uint32_t count = ts_node_named_child_count(node);
    for (uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_named_child(node, i);
        std::string type(ts_node_type(child));
        if (type == name_type) {
            return get_node_text(child, source);
        }
    }
    return {};
}

// Check if a node has a #[test] attribute as its preceding sibling
bool rust_has_test_attribute(TSNode node, const std::string& source) {
    TSNode prev = ts_node_prev_named_sibling(node);
    while (!ts_node_is_null(prev)) {
        std::string type(ts_node_type(prev));
        if (type == "attribute_item") {
            std::string text = get_node_text(prev, source);
            if (text.find("test") != std::string::npos &&
                text.find("cfg") == std::string::npos) {
                return true;  // #[test] (not #[cfg(test)])
            }
        } else {
            break;  // Stop at non-attribute siblings
        }
        prev = ts_node_prev_named_sibling(prev);
    }
    return false;
}

// Check if a mod_item has #[cfg(test)] attribute
bool rust_is_test_module(TSNode node, const std::string& source) {
    TSNode prev = ts_node_prev_named_sibling(node);
    while (!ts_node_is_null(prev)) {
        std::string type(ts_node_type(prev));
        if (type == "attribute_item") {
            std::string text = get_node_text(prev, source);
            if (text.find("cfg") != std::string::npos && text.find("test") != std::string::npos) {
                return true;  // #[cfg(test)]
            }
        } else {
            break;
        }
        prev = ts_node_prev_named_sibling(prev);
    }
    return false;
}

void rust_extract_symbols_recursive(
    TSNode node,
    const std::string& source,
    const std::string& file,
    const std::string& parent_type,
    SymbolTable& table,
    bool in_test_module = false)
{
    std::string type(ts_node_type(node));

    if (type == "function_item") {
        std::string name = rust_extract_name(node, source, "identifier");
        if (!name.empty()) {
            Visibility vis = rust_extract_visibility(node);
            bool is_test = in_test_module ||
                           rust_has_test_attribute(node, source) ||
                           (vis == Visibility::PRIVATE && name.starts_with("test_"));
            Symbol sym;
            sym.name = name;
            sym.kind = parent_type.empty() ? SymbolKind::FUNCTION : SymbolKind::IMPL_METHOD;
            sym.visibility = vis;
            sym.file = file;
            sym.line = get_node_line(node);
            sym.parent = parent_type;
            sym.is_test = is_test;
            if (!parent_type.empty()) {
                sym.qualified_name = parent_type + "::" + name;
            } else {
                sym.qualified_name = name;
            }
            table.add(std::move(sym));
        }
    }
    else if (type == "struct_item") {
        std::string name = rust_extract_name(node, source, "type_identifier");
        if (!name.empty()) {
            Visibility vis = rust_extract_visibility(node);
            bool is_test = in_test_module ||
                           (vis == Visibility::PRIVATE && name.starts_with("Test"));
            Symbol sym;
            sym.name = name;
            sym.qualified_name = name;
            sym.kind = SymbolKind::STRUCT;
            sym.visibility = vis;
            sym.is_test = is_test;
            sym.file = file;
            sym.line = get_node_line(node);

            // Extract field names
            uint32_t count = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(node, i);
                std::string child_type(ts_node_type(child));
                if (child_type == "field_declaration_list") {
                    uint32_t fc = ts_node_named_child_count(child);
                    for (uint32_t j = 0; j < fc; ++j) {
                        TSNode field = ts_node_named_child(child, j);
                        if (std::string(ts_node_type(field)) == "field_declaration") {
                            std::string fname = rust_extract_name(field, source, "field_identifier");
                            if (!fname.empty()) {
                                sym.members.push_back(fname);
                            }
                        }
                    }
                }
            }
            table.add(std::move(sym));
        }
    }
    else if (type == "enum_item") {
        std::string name = rust_extract_name(node, source, "type_identifier");
        if (!name.empty()) {
            Visibility vis = rust_extract_visibility(node);
            Symbol sym;
            sym.name = name;
            sym.qualified_name = name;
            sym.kind = SymbolKind::ENUM;
            sym.visibility = vis;
            sym.file = file;
            sym.line = get_node_line(node);

            // Extract variant names
            uint32_t count = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(node, i);
                std::string child_type(ts_node_type(child));
                if (child_type == "enum_variant_list") {
                    uint32_t vc = ts_node_named_child_count(child);
                    for (uint32_t j = 0; j < vc; ++j) {
                        TSNode variant = ts_node_named_child(child, j);
                        if (std::string(ts_node_type(variant)) == "enum_variant") {
                            std::string vname = rust_extract_name(variant, source, "identifier");
                            if (!vname.empty()) {
                                sym.members.push_back(vname);
                            }
                        }
                    }
                }
            }
            table.add(std::move(sym));
        }
    }
    else if (type == "trait_item") {
        std::string name = rust_extract_name(node, source, "type_identifier");
        if (!name.empty()) {
            Visibility vis = rust_extract_visibility(node);
            Symbol sym;
            sym.name = name;
            sym.qualified_name = name;
            sym.kind = SymbolKind::TRAIT;
            sym.visibility = vis;
            sym.file = file;
            sym.line = get_node_line(node);

            // Extract method signatures from trait body
            uint32_t count = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(node, i);
                std::string child_type(ts_node_type(child));
                if (child_type == "declaration_list") {
                    uint32_t mc = ts_node_named_child_count(child);
                    for (uint32_t j = 0; j < mc; ++j) {
                        TSNode method = ts_node_named_child(child, j);
                        std::string mtype(ts_node_type(method));
                        if (mtype == "function_signature_item" || mtype == "function_item") {
                            std::string mname = rust_extract_name(method, source, "identifier");
                            if (!mname.empty()) {
                                sym.members.push_back(mname);
                            }
                        }
                    }
                }
            }
            table.add(std::move(sym));
        }
    }
    else if (type == "impl_item") {
        // Extract the type being impl'd
        // For `impl Type { ... }`: parent is Type
        // For `impl Trait for Type { ... }`: parent is Type (the implementing type)
        //
        // In tree-sitter-rust, `impl Trait for Type` has unnamed "for" child
        // between the trait and type nodes. We need to find the type AFTER "for".
        uint32_t count = ts_node_named_child_count(node);
        uint32_t all_count = ts_node_child_count(node);

        // Strategy: scan ALL children (named + unnamed) for "for" keyword
        // If found, the type after "for" is the implementing type
        // If not found, the first type is the implementing type
        bool has_for = false;
        for (uint32_t i = 0; i < all_count; ++i) {
            TSNode child = ts_node_child(node, i);
            if (!ts_node_is_named(child)) {
                std::string text = get_node_text(child, source);
                if (text == "for") { has_for = true; break; }
            }
        }

        auto extract_type_name = [&](TSNode n) -> std::string {
            std::string ct(ts_node_type(n));
            if (ct == "type_identifier") {
                return get_node_text(n, source);
            }
            if (ct == "generic_type") {
                TSNode type_id = find_child_by_type(n, "type_identifier");
                if (!ts_node_is_null(type_id) &&
                    std::string(ts_node_type(type_id)) == "type_identifier") {
                    return get_node_text(type_id, source);
                }
            }
            if (ct == "scoped_type_identifier") {
                uint32_t sc = ts_node_named_child_count(n);
                for (uint32_t j = sc; j > 0; --j) {
                    TSNode sub = ts_node_named_child(n, j - 1);
                    if (std::string(ts_node_type(sub)) == "type_identifier") {
                        return get_node_text(sub, source);
                    }
                }
            }
            return {};
        };

        std::string impl_type;
        if (has_for) {
            // impl Trait for Type: find the type identifier AFTER the "for" keyword
            bool past_for = false;
            for (uint32_t i = 0; i < all_count; ++i) {
                TSNode child = ts_node_child(node, i);
                if (!ts_node_is_named(child)) {
                    std::string text = get_node_text(child, source);
                    if (text == "for") past_for = true;
                    continue;
                }
                if (past_for) {
                    impl_type = extract_type_name(child);
                    if (!impl_type.empty()) break;
                }
            }
        } else {
            // impl Type: first type identifier is the implementing type
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(node, i);
                impl_type = extract_type_name(child);
                if (!impl_type.empty()) break;
            }
        }

        if (!impl_type.empty()) {
            // Recurse into impl body to find methods
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(node, i);
                std::string child_type(ts_node_type(child));
                if (child_type == "declaration_list") {
                    uint32_t mc = ts_node_named_child_count(child);
                    for (uint32_t j = 0; j < mc; ++j) {
                        TSNode method = ts_node_named_child(child, j);
                        rust_extract_symbols_recursive(method, source, file, impl_type, table, in_test_module);
                    }
                    return;  // Don't recurse further
                }
            }
        }
    }
    else if (type == "const_item") {
        std::string name = rust_extract_name(node, source, "identifier");
        if (!name.empty()) {
            Visibility vis = rust_extract_visibility(node);
            Symbol sym;
            sym.name = name;
            sym.qualified_name = parent_type.empty() ? name : (parent_type + "::" + name);
            sym.kind = SymbolKind::CONST;
            sym.visibility = vis;
            sym.file = file;
            sym.line = get_node_line(node);
            sym.parent = parent_type;
            table.add(std::move(sym));
        }
    }
    else if (type == "type_item") {
        std::string name = rust_extract_name(node, source, "type_identifier");
        if (!name.empty()) {
            Visibility vis = rust_extract_visibility(node);
            Symbol sym;
            sym.name = name;
            sym.qualified_name = name;
            sym.kind = SymbolKind::TYPE_ALIAS;
            sym.visibility = vis;
            sym.file = file;
            sym.line = get_node_line(node);
            table.add(std::move(sym));
        }
    }

    // Recurse into children (unless we already handled impl body above)
    if (type != "impl_item") {
        // Check if this is a #[cfg(test)] mod — propagate test flag
        bool child_in_test = in_test_module;
        if (type == "mod_item") {
            child_in_test = child_in_test || rust_is_test_module(node, source);
        }

        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            rust_extract_symbols_recursive(child, source, file, parent_type, table, child_in_test);
        }
    }
}

// ============================================================================
// Kotlin stub detection
// ============================================================================

bool kotlin_is_stub_body(TSNode body_node, const std::string& source) {
    std::string body = get_node_text(body_node, source);
    if (body.empty()) return true;

    std::string code;
    code.reserve(body.size());
    size_t i = 0;
    while (i < body.size()) {
        if (i + 1 < body.size() && body[i] == '/' && body[i + 1] == '/') {
            i += 2;
            while (i < body.size() && body[i] != '\n') ++i;
        } else if (i + 1 < body.size() && body[i] == '/' && body[i + 1] == '*') {
            i += 2;
            while (i + 1 < body.size() && !(body[i] == '*' && body[i + 1] == '/')) ++i;
            if (i + 1 < body.size()) i += 2;
        } else {
            code += body[i++];
        }
    }

    // Check executable tokens only; comments can faithfully preserve upstream notes.
    static const std::vector<std::string> stub_markers = {
        "TODO()",
        "TODO(\"",
        "= TODO()",
        "throw NotImplementedError",
        "throw kotlin.NotImplementedError",
        "error(\"not implemented",
        "error(\"unimplemented",
        "error(\"stub",
        "error(\"placeholder",
    };
    for (const auto& marker : stub_markers) {
        if (code.find(marker) != std::string::npos) return true;
    }

    // Empty class body: just braces with optional whitespace
    if (code.length() < 10) {
        std::string trimmed;
        for (char c : code) {
            if (!std::isspace(static_cast<unsigned char>(c))) trimmed += c;
        }
        if (trimmed == "{}" || trimmed == "{};" || trimmed.empty()) return true;
    }

    return false;
}

bool kotlin_class_is_stub(TSNode node, const std::string& source) {
    // A class is a stub if:
    // 1. It has `private constructor()` and no real methods, OR
    // 2. Its body only contains placeholder comments/TODOs, OR
    // 3. It's an `internal class` with a private constructor and empty body
    uint32_t count = ts_node_named_child_count(node);
    for (uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_named_child(node, i);
        std::string type(ts_node_type(child));
        if (type == "class_body") {
            return kotlin_is_stub_body(child, source);
        }
    }
    // No body at all (just a declaration) → could be a stub
    return false;
}

// ============================================================================
// Kotlin symbol extraction
// ============================================================================

Visibility kotlin_extract_visibility(TSNode node, const std::string& source) {
    uint32_t count = ts_node_named_child_count(node);
    for (uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_named_child(node, i);
        std::string type(ts_node_type(child));
        if (type == "modifiers") {
            uint32_t mc = ts_node_named_child_count(child);
            for (uint32_t j = 0; j < mc; ++j) {
                TSNode mod = ts_node_named_child(child, j);
                std::string mtype(ts_node_type(mod));
                if (mtype == "visibility_modifier") {
                    std::string text = get_node_text(mod, source);
                    if (text == "private") return Visibility::PRIVATE;
                    if (text == "internal") return Visibility::CRATE;
                    if (text == "public") return Visibility::PUBLIC;
                    if (text == "protected") return Visibility::PRIVATE;
                }
            }
        }
    }
    // Kotlin default is public
    return Visibility::PUBLIC;
}

std::string kotlin_extract_name(TSNode node, const std::string& source) {
    auto strip_backticks = [](std::string s) {
        if (s.size() >= 2 && s.front() == '`' && s.back() == '`') {
            return s.substr(1, s.size() - 2);
        }
        return s;
    };

    uint32_t count = ts_node_named_child_count(node);
    for (uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_named_child(node, i);
        std::string type(ts_node_type(child));
        if (type == "simple_identifier" || type == "type_identifier") {
            return strip_backticks(get_node_text(child, source));
        }
        if (type == "variable_declaration") {
            uint32_t vcount = ts_node_named_child_count(child);
            for (uint32_t j = 0; j < vcount; ++j) {
                TSNode v = ts_node_named_child(child, j);
                std::string vtype(ts_node_type(v));
                if (vtype == "simple_identifier" || vtype == "type_identifier") {
                    return strip_backticks(get_node_text(v, source));
                }
            }
        }
    }
    return {};
}

bool kotlin_has_modifier(TSNode node, const std::string& source, const std::string& modifier) {
    uint32_t count = ts_node_named_child_count(node);
    for (uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_named_child(node, i);
        std::string type(ts_node_type(child));
        if (type == "modifiers") {
            uint32_t mc = ts_node_named_child_count(child);
            for (uint32_t j = 0; j < mc; ++j) {
                TSNode mod = ts_node_named_child(child, j);
                std::string text = get_node_text(mod, source);
                if (text == modifier || text.find(modifier) != std::string::npos) {
                    return true;
                }
            }
        }
    }
    return false;
}

void kotlin_extract_symbols_recursive(
    TSNode node,
    const std::string& source,
    const std::string& file,
    const std::string& parent_type,
    SymbolTable& table)
{
    std::string type(ts_node_type(node));

    if (type == "function_declaration") {
        std::string name = kotlin_extract_name(node, source);
        if (!name.empty()) {
            Visibility vis = kotlin_extract_visibility(node, source);
            Symbol sym;
            sym.name = name;
            sym.visibility = vis;
            sym.file = file;
            sym.line = get_node_line(node);
            sym.parent = parent_type;

            // Detect extension functions: fun Type.methodName(...)
            // In tree-sitter-kotlin, the receiver type appears BEFORE the function name
            // (simple_identifier). A user_type AFTER simple_identifier is the return type.
            // We must only treat user_type as a receiver if it precedes simple_identifier.
            std::string receiver;
            bool found_name = false;
            uint32_t fcount = ts_node_named_child_count(node);
            for (uint32_t fi = 0; fi < fcount; ++fi) {
                TSNode fchild = ts_node_named_child(node, fi);
                std::string ftype(ts_node_type(fchild));
                if (ftype == "simple_identifier") {
                    found_name = true;
                    break; // Stop: anything after this is not a receiver
                }
                // user_type before simple_identifier = extension receiver
                if (ftype == "user_type") {
                    receiver = get_node_text(fchild, source);
                    // Clean up generics: "NumRef.Companion" stays, "List<Value>" → "List"
                    size_t lt = receiver.find('<');
                    if (lt != std::string::npos) receiver = receiver.substr(0, lt);
                    break;
                }
            }

            if (!receiver.empty()) {
                sym.is_extension = true;
                sym.receiver_type = receiver;
                sym.kind = SymbolKind::IMPL_METHOD;
                sym.parent = receiver;
                sym.qualified_name = receiver + "." + name;
            } else if (!parent_type.empty()) {
                sym.kind = SymbolKind::IMPL_METHOD;
                sym.qualified_name = parent_type + "." + name;
            } else {
                sym.kind = SymbolKind::FUNCTION;
                sym.qualified_name = name;
            }
            table.add(std::move(sym));
        }
    }
    else if (type == "class_declaration") {
        std::string name = kotlin_extract_name(node, source);
        if (!name.empty()) {
            Visibility vis = kotlin_extract_visibility(node, source);

            // Determine what kind of Rust symbol this maps to
            bool is_sealed = kotlin_has_modifier(node, source, "sealed");
            bool is_enum = kotlin_has_modifier(node, source, "enum");
            bool is_data = kotlin_has_modifier(node, source, "data");

            // Detect stubs
            bool is_stub = kotlin_class_is_stub(node, source);

            Symbol sym;
            sym.name = name;
            sym.qualified_name = name;
            sym.visibility = vis;
            sym.file = file;
            sym.line = get_node_line(node);
            sym.is_stub = is_stub;

            if (is_enum) {
                sym.kind = SymbolKind::ENUM;
            } else if (is_sealed) {
                // Sealed classes often map to Rust enums
                sym.kind = SymbolKind::ENUM;
            } else {
                sym.kind = SymbolKind::STRUCT;
            }

            // Extract members from class body
            uint32_t count = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(node, i);
                std::string child_type(ts_node_type(child));
                if (child_type == "class_body" || child_type == "enum_class_body") {
                    uint32_t bc = ts_node_named_child_count(child);
                    for (uint32_t j = 0; j < bc; ++j) {
                        TSNode member = ts_node_named_child(child, j);
                        std::string mtype(ts_node_type(member));
                        if (mtype == "property_declaration") {
                            std::string mname = kotlin_extract_name(member, source);
                            if (!mname.empty()) sym.members.push_back(mname);
                        }
                        // Recurse for methods
                        kotlin_extract_symbols_recursive(member, source, file, name, table);
                    }
                    // Don't recurse further since we handled the body
                    table.add(std::move(sym));
                    return;
                }
            }
            table.add(std::move(sym));
        }
    }
    else if (type == "interface_declaration") {
        std::string name = kotlin_extract_name(node, source);
        if (!name.empty()) {
            Visibility vis = kotlin_extract_visibility(node, source);
            Symbol sym;
            sym.name = name;
            sym.qualified_name = name;
            sym.kind = SymbolKind::TRAIT;
            sym.visibility = vis;
            sym.file = file;
            sym.line = get_node_line(node);

            // Extract method signatures
            uint32_t count = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(node, i);
                std::string child_type(ts_node_type(child));
                if (child_type == "class_body") {
                    uint32_t bc = ts_node_named_child_count(child);
                    for (uint32_t j = 0; j < bc; ++j) {
                        TSNode member = ts_node_named_child(child, j);
                        std::string mtype(ts_node_type(member));
                        if (mtype == "function_declaration") {
                            std::string mname = kotlin_extract_name(member, source);
                            if (!mname.empty()) sym.members.push_back(mname);
                        }
                        kotlin_extract_symbols_recursive(member, source, file, name, table);
                    }
                    table.add(std::move(sym));
                    return;
                }
            }
            table.add(std::move(sym));
        }
    }
    else if (type == "object_declaration") {
        std::string name = kotlin_extract_name(node, source);
        if (!name.empty()) {
            Visibility vis = kotlin_extract_visibility(node, source);

            // Check if companion object
            bool is_companion = kotlin_has_modifier(node, source, "companion");

            if (is_companion) {
                // Companion object methods map to impl methods on the parent
                // The parent_type should already be set from the enclosing class
                uint32_t count = ts_node_named_child_count(node);
                for (uint32_t i = 0; i < count; ++i) {
                    TSNode child = ts_node_named_child(node, i);
                    std::string child_type(ts_node_type(child));
                    if (child_type == "class_body") {
                        uint32_t bc = ts_node_named_child_count(child);
                        for (uint32_t j = 0; j < bc; ++j) {
                            TSNode member = ts_node_named_child(child, j);
                            kotlin_extract_symbols_recursive(member, source, file, parent_type, table);
                        }
                        return;
                    }
                }
            } else {
                // Named object → treat as STRUCT
                Symbol sym;
                sym.name = name;
                sym.qualified_name = name;
                sym.kind = SymbolKind::STRUCT;
                sym.visibility = vis;
                sym.file = file;
                sym.line = get_node_line(node);

                uint32_t count = ts_node_named_child_count(node);
                for (uint32_t i = 0; i < count; ++i) {
                    TSNode child = ts_node_named_child(node, i);
                    std::string child_type(ts_node_type(child));
                    if (child_type == "class_body") {
                        uint32_t bc = ts_node_named_child_count(child);
                        for (uint32_t j = 0; j < bc; ++j) {
                            TSNode member = ts_node_named_child(child, j);
                            kotlin_extract_symbols_recursive(member, source, file, name, table);
                        }
                        table.add(std::move(sym));
                        return;
                    }
                }
                table.add(std::move(sym));
            }
        }
    }
    else if (type == "property_declaration") {
        std::string name = kotlin_extract_name(node, source);
        if (!name.empty()) {
            Visibility vis = kotlin_extract_visibility(node, source);
            Symbol sym;
            sym.name = name;
            sym.qualified_name = parent_type.empty() ? name : parent_type + "." + name;
            sym.kind = SymbolKind::CONST;
            sym.visibility = vis;
            sym.file = file;
            sym.line = get_node_line(node);
            sym.parent = parent_type;
            table.add(std::move(sym));
        }
    }
    else if (type == "type_alias") {
        std::string name = kotlin_extract_name(node, source);
        if (!name.empty()) {
            Visibility vis = kotlin_extract_visibility(node, source);
            Symbol sym;
            sym.name = name;
            sym.qualified_name = name;
            sym.kind = SymbolKind::TYPE_ALIAS;
            sym.visibility = vis;
            sym.file = file;
            sym.line = get_node_line(node);
            table.add(std::move(sym));
        }
    }

    // Recurse into children for types we didn't fully handle above
    if (type != "class_declaration" && type != "interface_declaration" &&
        type != "object_declaration") {
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            kotlin_extract_symbols_recursive(child, source, file, parent_type, table);
        }
    }
}

// ============================================================================
// Symbol matching
// ============================================================================

// Try to match a Rust symbol name to a Kotlin symbol
struct MatchResult {
    std::string kotlin_name;
    float confidence;
    std::string reason;
};

std::vector<MatchResult> generate_match_candidates(const Symbol& rust_sym) {
    std::vector<MatchResult> candidates;

    // 1. Exact match (types are PascalCase in both languages)
    candidates.push_back({rust_sym.name, 1.0f, "exact"});

    // 1b. Generated Rust symbols often use leading underscores for namespacing.
    if (!rust_sym.name.empty() && rust_sym.name[0] == '_') {
        std::string stripped = rust_sym.name;
        while (!stripped.empty() && stripped[0] == '_') stripped.erase(stripped.begin());
        if (!stripped.empty() && stripped != rust_sym.name) {
            candidates.push_back({stripped, 0.97f, "strip_leading_underscore"});
        }
    }

    // 2. snake_case → camelCase for functions
    if (rust_sym.kind == SymbolKind::FUNCTION || rust_sym.kind == SymbolKind::IMPL_METHOD) {
        std::string camel = snake_to_camel(rust_sym.name);
        if (camel != rust_sym.name) {
            candidates.push_back({camel, 0.95f, "camelCase"});
        }
    }

    // 3. Qualified name transformations
    if (!rust_sym.parent.empty()) {
        // Rust: Type::method → Kotlin: Type.methodName
        std::string kotlin_qual = rust_sym.parent + "." + rust_sym.name;
        candidates.push_back({kotlin_qual, 0.9f, "qualified"});

        std::string kotlin_qual_camel = rust_sym.parent + "." + snake_to_camel(rust_sym.name);
        if (kotlin_qual_camel != kotlin_qual) {
            candidates.push_back({kotlin_qual_camel, 0.85f, "qualified+camelCase"});
        }

        // Rust: Type::method → Kotlin extension: fun Type.methodName()
        // Also try Companion pattern: Type.Companion.methodName
        std::string companion_qual = rust_sym.parent + ".Companion." + rust_sym.name;
        candidates.push_back({companion_qual, 0.8f, "companion"});
        std::string companion_camel = rust_sym.parent + ".Companion." + snake_to_camel(rust_sym.name);
        if (companion_camel != companion_qual) {
            candidates.push_back({companion_camel, 0.75f, "companion+camelCase"});
        }
    }

    // 4. Common Rust → Kotlin name patterns
    if (rust_sym.name == "new" && !rust_sym.parent.empty()) {
        // Rust Type::new() → Kotlin constructor or factory
        candidates.push_back({rust_sym.parent + ".invoke", 0.7f, "new→invoke"});
        candidates.push_back({rust_sym.parent, 0.65f, "new→constructor"});
    }

    // 5. Rust r#keyword → Kotlin name without r# prefix
    if (rust_sym.name.starts_with("r#")) {
        std::string stripped = rust_sym.name.substr(2);
        candidates.push_back({stripped, 0.95f, "raw_ident"});
        if (!rust_sym.parent.empty()) {
            candidates.push_back({rust_sym.parent + "." + stripped, 0.9f, "qualified+raw_ident"});
        }
    }

    // 6. Rust impl Display (fmt) → Kotlin toString
    if (rust_sym.name == "fmt" && rust_sym.kind == SymbolKind::IMPL_METHOD) {
        if (!rust_sym.parent.empty()) {
            candidates.push_back({rust_sym.parent + ".toString", 0.8f, "fmt→toString"});
        }
    }

    // 6b. Rust impl Drop (drop) → Kotlin close
    if (rust_sym.name == "drop" && rust_sym.kind == SymbolKind::IMPL_METHOD) {
        if (!rust_sym.parent.empty()) {
            candidates.push_back({rust_sym.parent + ".close", 0.8f, "drop→close"});
        }
    }

    // 7. Rust trait method → Kotlin operator/convention mappings
    if (rust_sym.kind == SymbolKind::IMPL_METHOD && !rust_sym.parent.empty()) {
        static const std::map<std::string, std::string> trait_mappings = {
            // Rust std::ops → Kotlin operators
            {"add", "plus"}, {"sub", "minus"}, {"mul", "times"},
            {"div", "div"}, {"rem", "rem"}, {"neg", "unaryMinus"},
            // Rust comparison traits → Kotlin
            {"eq", "equals"}, {"ne", "notEquals"},
            {"partial_cmp", "compareTo"}, {"cmp", "compareTo"},
            // Rust Iterator trait → Kotlin
            {"next", "next"}, {"into_iter", "iterator"},
            // Rust conversion traits → Kotlin
            {"from", "from"}, {"try_from", "tryFrom"},
            {"into", "into"},
            // Rust indexing → Kotlin
            {"index", "get"}, {"index_mut", "set"},
            // Rust Deref → Kotlin
            {"deref", "getValue"}, {"deref_mut", "setValue"},
            // Rust Hash → Kotlin
            {"hash", "hashCode"},
        };
        auto it = trait_mappings.find(rust_sym.name);
        if (it != trait_mappings.end()) {
            // Try with parent type
            candidates.push_back({rust_sym.parent + "." + it->second, 0.85f,
                                  rust_sym.name + "→" + it->second});
            // Also try just the operator name (tree-sitter may mis-parent)
            candidates.push_back({it->second, 0.6f,
                                  rust_sym.name + "→" + it->second + " (unqualified)"});
        }
    }

    // 8. For impl methods, also search unqualified (tree-sitter may assign
    //    different parent names due to inner class nesting)
    if (rust_sym.kind == SymbolKind::IMPL_METHOD) {
        // Try just the method name
        candidates.push_back({rust_sym.name, 0.5f, "method_name_only"});
        std::string camel = snake_to_camel(rust_sym.name);
        if (camel != rust_sym.name) {
            candidates.push_back({camel, 0.45f, "method_camelCase_only"});
        }
    }

    return candidates;
}

}  // anonymous namespace

// ============================================================================
// Public API
// ============================================================================

// ============================================================================
// Reexport graph extraction
//
// Walks the Rust source tree, opens every `mod.rs` and `lib.rs`, and parses
// `pub use` declarations into a flat graph keyed by exported name.
//
// Why: when a symbol like `Bar` shows up as missing in the parity report,
// the porter wants to know *where Bar actually lives* — the canonical
// defining file — so they can put the Kotlin counterpart in the right
// place. A `pub use foo::bar::Bar` chain points the way. Per AGENTS.md,
// the Kotlin port wires callers to the canonical location, not the
// reexport, so showing the chain saves the porter from chasing it
// manually.
//
// This is intentionally a regex parser (not tree-sitter): `pub use` is a
// shallow declaration, almost always one statement per line, and we want
// to keep this layer dependency-free for downstream callers that already
// hold a SymbolTable.
// ============================================================================

namespace {

std::string trim_use(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

// Last `::`-separated component of a Rust path.
std::string last_component(const std::string& path) {
    auto pos = path.rfind("::");
    if (pos == std::string::npos) return path;
    return path.substr(pos + 2);
}

// "foo::bar::Baz as Q" → ("foo::bar::Baz", "Q"); without "as", alias is empty.
std::pair<std::string, std::string> split_alias(const std::string& spec) {
    static const std::regex alias_re(R"(^(.+?)\s+as\s+([A-Za-z_][A-Za-z0-9_]*)\s*$)");
    std::smatch m;
    if (std::regex_match(spec, m, alias_re)) {
        return {trim_use(m[1].str()), trim_use(m[2].str())};
    }
    return {trim_use(spec), {}};
}

// "Foo, Bar as Q, Baz" → {"Foo", "Bar as Q", "Baz"}, respecting nested braces.
std::vector<std::string> split_top_level_commas(const std::string& items) {
    std::vector<std::string> out;
    std::string current;
    int depth = 0;
    for (char c : items) {
        if (c == '{') ++depth;
        else if (c == '}') --depth;
        if (c == ',' && depth == 0) {
            std::string t = trim_use(current);
            if (!t.empty()) out.push_back(t);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    std::string t = trim_use(current);
    if (!t.empty()) out.push_back(t);
    return out;
}

void collect_use_specs(const std::string& mod_file,
                       int line_no,
                       const std::string& spec,
                       ReexportGraph& graph) {
    if (spec.empty() || spec.find('*') != std::string::npos) return;

    static const std::regex brace_re(R"(^(.+?)::\{(.+)\}\s*$)");
    std::smatch m;
    if (std::regex_match(spec, m, brace_re)) {
        std::string prefix = trim_use(m[1].str());
        for (const auto& item : split_top_level_commas(m[2].str())) {
            auto [path, alias] = split_alias(item);
            if (path == "self") {
                std::string exported = alias.empty() ? last_component(prefix) : alias;
                ReexportEdge edge{mod_file, line_no, prefix, exported};
                graph.chains_by_name[exported].push_back({edge});
                continue;
            }
            std::string full = prefix + "::" + path;
            std::string exported = alias.empty() ? last_component(path) : alias;
            ReexportEdge edge{mod_file, line_no, full, exported};
            graph.chains_by_name[exported].push_back({edge});
        }
        return;
    }

    auto [path, alias] = split_alias(spec);
    std::string exported = alias.empty() ? last_component(path) : alias;
    if (exported.empty() || exported == "self") return;
    ReexportEdge edge{mod_file, line_no, path, exported};
    graph.chains_by_name[exported].push_back({edge});
}

}  // namespace

ReexportGraph extract_rust_reexports(const std::string& root) {
    ReexportGraph graph;

    // `pub` or `pub(crate)`/`pub(in foo)` followed by `use … ;`. We don't
    // try to handle multi-line `use` blocks; the stdlib btree, lalrpop, and
    // codex sources we target keep them on one line.
    static const std::regex use_re(
        R"(^\s*pub(?:\([^)]*\))?\s+use\s+(.+?)\s*;\s*$)");

    // `pub use` declarations live in `mod.rs` and `lib.rs` most often, but
    // any .rs file can reexport from a sibling submodule (the stdlib's
    // btree puts `pub use entry::{Entry, OccupiedEntry, VacantEntry}` at
    // the top of `map.rs` and `set.rs`, for example). Walk every Rust
    // file; the parser ignores anything that isn't a single-line
    // `pub use … ;`.
    for (const auto& entry : fs::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".rs") continue;
        std::string path = entry.path().string();
        if (should_skip_path(path)) continue;

        std::string content = read_file_content(entry.path());
        if (content.empty()) continue;

        std::string rel_path = fs::relative(entry.path(), root).string();

        std::istringstream stream(content);
        std::string line;
        int line_no = 0;
        while (std::getline(stream, line)) {
            ++line_no;
            std::smatch m;
            if (!std::regex_match(line, m, use_re)) continue;
            collect_use_specs(rel_path, line_no, trim_use(m[1].str()), graph);
        }
    }

    return graph;
}

SymbolTable extract_rust_symbols(const std::string& root) {
    SymbolTable table;
    TSParser* parser = ts_parser_new();
    const TSLanguage* lang = tree_sitter_rust();
    ts_parser_set_language(parser, lang);

    for (const auto& entry : fs::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file()) continue;
        std::string path = entry.path().string();
        if (should_skip_path(path)) continue;
        if (!path.ends_with(".rs")) continue;

        std::string content = read_file_content(entry.path());
        if (content.empty()) continue;

        std::string rel_path = fs::relative(entry.path(), root).string();

        TSTree* tree = ts_parser_parse_string(parser, nullptr, content.c_str(), content.length());
        if (!tree) continue;

        TSNode root_node = ts_tree_root_node(tree);
        rust_extract_symbols_recursive(root_node, content, rel_path, "", table);

        ts_tree_delete(tree);
    }

    ts_parser_delete(parser);
    return table;
}

SymbolTable extract_kotlin_symbols(const std::string& root) {
    SymbolTable table;
    TSParser* parser = ts_parser_new();
    const TSLanguage* lang = tree_sitter_kotlin();
    ts_parser_set_language(parser, lang);

    for (const auto& entry : fs::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file()) continue;
        std::string path = entry.path().string();
        if (should_skip_path(path)) continue;
        if (!path.ends_with(".kt")) continue;

        std::string content = read_file_content(entry.path());
        if (content.empty()) continue;

        std::string rel_path = fs::relative(entry.path(), root).string();

        TSTree* tree = ts_parser_parse_string(parser, nullptr, content.c_str(), content.length());
        if (!tree) continue;

        TSNode root_node = ts_tree_root_node(tree);
        kotlin_extract_symbols_recursive(root_node, content, rel_path, "", table);

        ts_tree_delete(tree);
    }

    ts_parser_delete(parser);
    return table;
}

SymbolParityReport build_parity_report(const SymbolTable& rust,
                                       const SymbolTable& kotlin,
                                       const ReexportGraph* reexports) {
    SymbolParityReport report;

    // Count stubs
    int stub_count = 0;
    for (const auto& sym : kotlin.symbols) {
        if (sym.is_stub) stub_count++;
    }
    report.stub_count = stub_count;

    // Build a lookup set of all Kotlin symbol names (multi-index)
    // For name collisions, prefer non-stub symbols
    std::map<std::string, const Symbol*> kt_by_name;
    std::map<std::string, const Symbol*> kt_by_qualified;
    for (const auto& sym : kotlin.symbols) {
        auto it = kt_by_name.find(sym.name);
        if (it == kt_by_name.end() || (it->second->is_stub && !sym.is_stub)) {
            kt_by_name[sym.name] = &sym;
        }
        auto it2 = kt_by_qualified.find(sym.qualified_name);
        if (it2 == kt_by_qualified.end() || (it2->second->is_stub && !sym.is_stub)) {
            kt_by_qualified[sym.qualified_name] = &sym;
        }
    }

    std::set<const Symbol*> matched_kotlin;

    for (const auto& rust_sym : rust.symbols) {
        auto candidates = generate_match_candidates(rust_sym);

        const Symbol* best_match = nullptr;
        float best_confidence = 0.0f;
        std::string best_reason;

        for (const auto& candidate : candidates) {
            // Try qualified name lookup first
            auto it = kt_by_qualified.find(candidate.kotlin_name);
            if (it != kt_by_qualified.end()) {
                if (candidate.confidence > best_confidence) {
                    best_match = it->second;
                    best_confidence = candidate.confidence;
                    best_reason = candidate.reason;
                }
            }
            // Then try plain name
            auto it2 = kt_by_name.find(candidate.kotlin_name);
            if (it2 != kt_by_name.end()) {
                if (candidate.confidence > best_confidence) {
                    best_match = it2->second;
                    best_confidence = candidate.confidence;
                    best_reason = candidate.reason;
                }
            }
        }

        SymbolMatch match;
        match.rust_symbol = &rust_sym;
        match.kotlin_symbol = best_match;
        match.confidence = best_confidence;
        match.match_reason = best_reason;
        // Annotate with `pub use` chains so missing-symbol output points
        // the porter at the canonical defining file. Only attach the
        // chain when the chain's mod.rs is *not* the same file the
        // symbol is defined in — otherwise we'd echo `Bar (in foo.rs)
        // exported via foo.rs` for every locally-public item.
        if (reexports) {
            auto it = reexports->chains_by_name.find(rust_sym.name);
            if (it != reexports->chains_by_name.end()) {
                for (const auto& chain : it->second) {
                    if (chain.empty()) continue;
                    if (chain.front().mod_file == rust_sym.file) continue;
                    match.reexport_chains.push_back(chain);
                }
            }
        }
        report.matches.push_back(match);

        if (best_match) {
            matched_kotlin.insert(best_match);
            if (rust_sym.is_test) {
                report.test_coverage[rust_sym.kind].first++;
            } else {
                report.coverage[rust_sym.kind].first++;
            }
        } else {
            report.missing_in_kotlin.push_back(&rust_sym);
        }
        if (rust_sym.is_test) {
            report.test_coverage[rust_sym.kind].second++;
        } else {
            report.coverage[rust_sym.kind].second++;
        }
    }

    // Find extra Kotlin symbols (not matched to any Rust symbol)
    for (const auto& kt_sym : kotlin.symbols) {
        if (!matched_kotlin.count(&kt_sym)) {
            report.extra_in_kotlin.push_back(&kt_sym);
        }
    }

    return report;
}

void SymbolParityReport::print(bool verbose, bool missing_only) const {
    std::cout << "==========================================================\n";
    std::cout << "SYMBOL PARITY: Rust \xe2\x86\x92 Kotlin\n";
    std::cout << "==========================================================\n";

    // Primary definitions: the things that matter for porting parity
    std::vector<SymbolKind> primary_kinds = {
        SymbolKind::FUNCTION, SymbolKind::STRUCT, SymbolKind::ENUM,
        SymbolKind::TRAIT, SymbolKind::IMPL_METHOD
    };

    // Secondary: type aliases are mostly generic specializations (Rust's
    // `type Foo<'v> = FooGen<Value<'v>>` pattern) or mod.rs re-exports.
    // Constants are often associated consts inside trait impls.
    // Still useful to track, but not the headline metric.
    std::vector<SymbolKind> secondary_kinds = {
        SymbolKind::CONST, SymbolKind::TYPE_ALIAS
    };

    // Helper: print one kind's detail lines
    auto print_kind_detail = [&](SymbolKind kind, bool is_test) {
        if (missing_only) {
            // Only print missing symbols, no matches
            for (const auto& m : matches) {
                if (m.rust_symbol->kind != kind) continue;
                if (m.rust_symbol->is_test != is_test) continue;
                if (!m.kotlin_symbol) {
                    std::cout << "  " << m.rust_symbol->name
                              << " (" << visibility_name(m.rust_symbol->visibility)
                              << " in " << m.rust_symbol->file
                              << ":" << m.rust_symbol->line << ")";
                    for (const auto& chain : m.reexport_chains) {
                        for (const auto& edge : chain) {
                            std::cout << "\n      via " << edge.mod_file
                                      << ":" << edge.mod_line
                                      << "  pub use " << edge.original_path;
                            if (edge.exported_name != last_component(edge.original_path)) {
                                std::cout << " as " << edge.exported_name;
                            }
                        }
                    }
                    std::cout << "\n";
                }
            }
        } else if (verbose) {
            for (const auto& m : matches) {
                if (m.rust_symbol->kind != kind) continue;
                if (m.rust_symbol->is_test != is_test) continue;
                if (m.kotlin_symbol) {
                    std::cout << "  [MATCHED] " << m.rust_symbol->name
                              << " \xe2\x86\x92 " << m.kotlin_symbol->name
                              << " (" << m.match_reason << ")\n";
                } else {
                    std::cout << "  [MISSING] " << m.rust_symbol->name
                              << " (" << visibility_name(m.rust_symbol->visibility)
                              << " in " << m.rust_symbol->file
                              << ":" << m.rust_symbol->line << ")\n";
                }
            }
        } else {
            for (const auto& m : matches) {
                if (m.rust_symbol->kind != kind) continue;
                if (m.rust_symbol->is_test != is_test) continue;
                if (!m.kotlin_symbol) {
                    std::cout << "  [MISSING] " << m.rust_symbol->name
                              << " (" << visibility_name(m.rust_symbol->visibility)
                              << " in " << m.rust_symbol->file
                              << ":" << m.rust_symbol->line << ")\n";
                }
            }
        }
    };

    // Helper: print a group of kinds, return (matched, total)
    auto print_kind_group = [&](const std::vector<SymbolKind>& kinds,
                                const std::map<SymbolKind, std::pair<int,int>>& cov,
                                bool is_test) -> std::pair<int,int> {
        int group_matched = 0;
        int group_total = 0;
        for (SymbolKind kind : kinds) {
            auto it = cov.find(kind);
            if (it == cov.end() || it->second.second == 0) continue;

            int matched = it->second.first;
            int total = it->second.second;
            float pct = (total > 0) ? (100.0f * matched / total) : 0.0f;
            group_matched += matched;
            group_total += total;

            std::cout << "\n--- " << symbol_kind_label(kind) << " ("
                      << matched << "/" << total << " matched, "
                      << std::fixed << std::setprecision(1) << pct << "%) ---\n";

            print_kind_detail(kind, is_test);
        }
        return {group_matched, group_total};
    };

    // ---- PRODUCTION CODE ----
    std::cout << "\n========== PRODUCTION CODE ==========\n";

    auto [prod_pri_m, prod_pri_t] = print_kind_group(primary_kinds, coverage, false);

    // Primary subtotal
    float prod_pri_pct = (prod_pri_t > 0) ? (100.0f * prod_pri_m / prod_pri_t) : 0.0f;
    std::cout << "\n  Definitions: " << prod_pri_m << "/" << prod_pri_t
              << " (" << std::fixed << std::setprecision(1) << prod_pri_pct << "%)\n";

    // Secondary
    auto [prod_sec_m, prod_sec_t] = print_kind_group(secondary_kinds, coverage, false);
    if (prod_sec_t > 0) {
        float prod_sec_pct = (100.0f * prod_sec_m / prod_sec_t);
        std::cout << "\n  Supplementary: " << prod_sec_m << "/" << prod_sec_t
                  << " (" << std::fixed << std::setprecision(1) << prod_sec_pct << "%)\n";
    }

    // ---- TEST CODE ----
    std::cout << "\n========== TEST CODE ==========\n";

    auto [test_pri_m, test_pri_t] = print_kind_group(primary_kinds, test_coverage, true);

    float test_pri_pct = (test_pri_t > 0) ? (100.0f * test_pri_m / test_pri_t) : 0.0f;
    std::cout << "\n  Definitions: " << test_pri_m << "/" << test_pri_t
              << " (" << std::fixed << std::setprecision(1) << test_pri_pct << "%)\n";

    auto [test_sec_m, test_sec_t] = print_kind_group(secondary_kinds, test_coverage, true);
    if (test_sec_t > 0) {
        float test_sec_pct = (100.0f * test_sec_m / test_sec_t);
        std::cout << "\n  Supplementary: " << test_sec_m << "/" << test_sec_t
                  << " (" << std::fixed << std::setprecision(1) << test_sec_pct << "%)\n";
    }

    // ---- SUMMARY ----
    // Combined symbol coverage includes both primary definitions and
    // supplementary constants/type aliases, in addition to the subtotals below.
    int prod_matched = prod_pri_m + prod_sec_m;
    int prod_total = prod_pri_t + prod_sec_t;
    int test_matched = test_pri_m + test_sec_m;
    int test_total = test_pri_t + test_sec_t;

    int missing_prod = 0, missing_test = 0;
    for (const auto* s : missing_in_kotlin) {
        if (s->is_test) missing_test++;
        else missing_prod++;
    }

    // Count real extras vs stub extras
    int extra_real = 0, extra_stubs = 0;
    for (const auto* s : extra_in_kotlin) {
        if (s->is_stub) extra_stubs++;
        else extra_real++;
    }

    std::cout << "\n==========================================================\n";
    std::cout << "SUMMARY\n";
    std::cout << "  Production symbols (all): " << prod_matched << "/" << prod_total
              << " (" << std::fixed << std::setprecision(1)
              << (prod_total > 0 ? 100.0f * prod_matched / prod_total : 0.0f)
              << "%) - " << missing_prod << " missing\n";
    std::cout << "  Test symbols (all):       " << test_matched << "/" << test_total
              << " (" << std::fixed << std::setprecision(1)
              << (test_total > 0 ? 100.0f * test_matched / test_total : 0.0f)
              << "%) - " << missing_test << " missing\n";
    std::cout << "  Production definitions: " << prod_pri_m << "/" << prod_pri_t
              << " (" << std::fixed << std::setprecision(1) << prod_pri_pct << "%)\n";
    if (prod_sec_t > 0) {
        float prod_sec_pct2 = (100.0f * prod_sec_m / prod_sec_t);
        std::cout << "  Supplementary symbols: " << prod_sec_m << "/" << prod_sec_t
                  << " (" << std::fixed << std::setprecision(1) << prod_sec_pct2
                  << "%) [constants, type aliases]\n";
    }
    std::cout << "  Test definitions:      " << test_pri_m << "/" << test_pri_t
              << " (" << std::fixed << std::setprecision(1) << test_pri_pct
              << "%) - " << missing_test << " missing\n";
    std::cout << "  Extra (Kotlin-only):   " << extra_real
              << " real + " << extra_stubs << " stubs\n";
    if (stub_count > 0) {
        std::cout << "  Kotlin stubs detected: " << stub_count << "\n";
    }
    std::cout << "==========================================================\n";
}

void SymbolParityReport::print_json() const {
    std::cout << "{\n";
    std::cout << "  \"matches\": [\n";
    for (size_t i = 0; i < matches.size(); ++i) {
        const auto& m = matches[i];
        std::cout << "    {\"rust\": \"" << m.rust_symbol->qualified_name
                  << "\", \"kind\": \"" << symbol_kind_name(m.rust_symbol->kind)
                  << "\", \"kotlin\": "
                  << (m.kotlin_symbol ? ("\"" + m.kotlin_symbol->qualified_name + "\"") : "null")
                  << ", \"confidence\": " << std::fixed << std::setprecision(2) << m.confidence
                  << ", \"reason\": \"" << m.match_reason << "\""
                  << ", \"is_test\": " << (m.rust_symbol->is_test ? "true" : "false")
                  << ", \"file\": \"" << m.rust_symbol->file
                  << "\", \"line\": " << m.rust_symbol->line
                  << "}";
        if (i + 1 < matches.size()) std::cout << ",";
        std::cout << "\n";
    }
    std::cout << "  ],\n";

    // Build a sidecar lookup so we can attach reexport chains to the
    // missing entries without changing the public missing_in_kotlin shape.
    std::map<const Symbol*, const SymbolMatch*> match_by_rust;
    for (const auto& m : matches) {
        if (!m.kotlin_symbol) match_by_rust[m.rust_symbol] = &m;
    }

    std::cout << "  \"missing\": [\n";
    for (size_t i = 0; i < missing_in_kotlin.size(); ++i) {
        const auto* s = missing_in_kotlin[i];
        std::cout << "    {\"name\": \"" << s->qualified_name
                  << "\", \"kind\": \"" << symbol_kind_name(s->kind)
                  << "\", \"visibility\": \"" << visibility_name(s->visibility)
                  << "\", \"is_test\": " << (s->is_test ? "true" : "false")
                  << ", \"file\": \"" << s->file
                  << "\", \"line\": " << s->line;
        auto it = match_by_rust.find(s);
        if (it != match_by_rust.end() && !it->second->reexport_chains.empty()) {
            std::cout << ", \"reexport_chains\": [";
            bool first_chain = true;
            for (const auto& chain : it->second->reexport_chains) {
                if (!first_chain) std::cout << ",";
                first_chain = false;
                std::cout << "[";
                for (size_t e = 0; e < chain.size(); ++e) {
                    const auto& edge = chain[e];
                    if (e) std::cout << ",";
                    std::cout << "{\"mod_file\":\"" << edge.mod_file
                              << "\",\"mod_line\":" << edge.mod_line
                              << ",\"original_path\":\"" << edge.original_path
                              << "\",\"exported_name\":\"" << edge.exported_name
                              << "\"}";
                }
                std::cout << "]";
            }
            std::cout << "]";
        }
        std::cout << "}";
        if (i + 1 < missing_in_kotlin.size()) std::cout << ",";
        std::cout << "\n";
    }
    std::cout << "  ],\n";

    std::cout << "  \"extra\": [\n";
    for (size_t i = 0; i < extra_in_kotlin.size(); ++i) {
        const auto* s = extra_in_kotlin[i];
        std::cout << "    {\"name\": \"" << s->qualified_name
                  << "\", \"kind\": \"" << symbol_kind_name(s->kind)
                  << "\", \"is_stub\": " << (s->is_stub ? "true" : "false")
                  << ", \"file\": \"" << s->file
                  << "\", \"line\": " << s->line << "}";
        if (i + 1 < extra_in_kotlin.size()) std::cout << ",";
        std::cout << "\n";
    }
    std::cout << "  ],\n";
    std::cout << "  \"stub_count\": " << stub_count << ",\n";

    std::cout << "  \"coverage\": {\n";
    bool first = true;
    for (const auto& [kind, counts] : coverage) {
        if (!first) std::cout << ",\n";
        first = false;
        float pct = (counts.second > 0) ? (100.0f * counts.first / counts.second) : 0.0f;
        std::cout << "    \"" << symbol_kind_name(kind) << "\": {"
                  << "\"matched\": " << counts.first
                  << ", \"total\": " << counts.second
                  << ", \"percent\": " << std::fixed << std::setprecision(1) << pct
                  << "}";
    }
    std::cout << "\n  },\n";

    std::cout << "  \"test_coverage\": {\n";
    first = true;
    for (const auto& [kind, counts] : test_coverage) {
        if (!first) std::cout << ",\n";
        first = false;
        float pct = (counts.second > 0) ? (100.0f * counts.first / counts.second) : 0.0f;
        std::cout << "    \"" << symbol_kind_name(kind) << "\": {"
                  << "\"matched\": " << counts.first
                  << ", \"total\": " << counts.second
                  << ", \"percent\": " << std::fixed << std::setprecision(1) << pct
                  << "}";
    }
    std::cout << "\n  }\n";
    std::cout << "}\n";
}

void cmd_symbol_parity(const std::string& rust_root,
                       const std::string& kotlin_root,
                       const SymbolParityOptions& options) {
    std::cerr << "Extracting Rust symbols from " << rust_root << "...\n";
    SymbolTable rust = extract_rust_symbols(rust_root);
    std::cerr << "Found " << rust.size() << " Rust symbols\n";

    std::cerr << "Extracting Kotlin symbols from " << kotlin_root << "...\n";
    SymbolTable kotlin = extract_kotlin_symbols(kotlin_root);
    std::cerr << "Found " << kotlin.size() << " Kotlin symbols\n";

    // Apply filters
    SymbolTable* rust_ptr = &rust;
    SymbolTable* kotlin_ptr = &kotlin;
    SymbolTable filtered_rust;
    SymbolTable filtered_kotlin;

    bool has_filter = !options.filter_kind.empty() || !options.filter_file.empty();
    if (has_filter) {
        // Filter Rust symbols
        for (const auto& sym : rust.symbols) {
            bool keep = true;
            if (!options.filter_kind.empty()) {
                std::string kind_lower = options.filter_kind;
                std::transform(kind_lower.begin(), kind_lower.end(), kind_lower.begin(), ::tolower);
                std::string sym_kind(symbol_kind_name(sym.kind));
                std::transform(sym_kind.begin(), sym_kind.end(), sym_kind.begin(), ::tolower);
                if (sym_kind != kind_lower) keep = false;
            }
            if (!options.filter_file.empty()) {
                if (sym.file.find(options.filter_file) == std::string::npos) keep = false;
            }
            if (keep) filtered_rust.add(sym);
        }
        std::cerr << "Filtered to " << filtered_rust.size() << " Rust symbols\n";
        rust_ptr = &filtered_rust;

        // Also filter Kotlin symbols for "Extra" counting when file filter is active
        if (!options.filter_file.empty()) {
            // Normalize for cross-language matching:
            // 1. Strip extension (.rs, .kt)
            // 2. Lowercase everything
            // 3. Remove underscores (so snake_case matches PascalCase after lowering)
            // e.g. "typing/starlark_value.rs" -> "typing/starlarkvalue"
            //      "typing/StarlarkValue.kt"  -> "typing/starlarkvalue"
            auto normalize_path = [](const std::string& path) -> std::string {
                std::string stem = path;
                auto dot_pos = stem.rfind('.');
                if (dot_pos != std::string::npos) {
                    stem = stem.substr(0, dot_pos);
                }
                std::transform(stem.begin(), stem.end(), stem.begin(), ::tolower);
                stem.erase(std::remove(stem.begin(), stem.end(), '_'), stem.end());
                return stem;
            };

            std::string filter_norm = normalize_path(options.filter_file);

            for (const auto& sym : kotlin.symbols) {
                std::string kt_norm = normalize_path(sym.file);
                if (kt_norm.find(filter_norm) != std::string::npos) {
                    filtered_kotlin.add(sym);
                }
            }
            kotlin_ptr = &filtered_kotlin;
            std::cerr << "Filtered Kotlin to " << filtered_kotlin.size() << " symbols\n";
        }
    }

    std::cerr << "Indexing Rust pub-use reexports...\n";
    ReexportGraph reexports = extract_rust_reexports(rust_root);
    std::cerr << "Found " << reexports.chains_by_name.size()
              << " distinct reexported names\n";

    SymbolParityReport report =
        build_parity_report(*rust_ptr, *kotlin_ptr, &reexports);
    if (options.json) {
        report.print_json();
    } else {
        report.print(options.verbose, options.missing_only);
    }
}

// ============================================================================
// Import Map: Kotlin type registry and import resolution
// ============================================================================

namespace {

// Extract package declaration from a Kotlin source file using tree-sitter
std::string kotlin_extract_package(TSNode root, const std::string& source) {
    uint32_t count = ts_node_named_child_count(root);
    for (uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_named_child(root, i);
        std::string type(ts_node_type(child));
        if (type == "package_header") {
            // The package identifier is a child of package_header
            uint32_t pc = ts_node_named_child_count(child);
            for (uint32_t j = 0; j < pc; ++j) {
                TSNode pkg_child = ts_node_named_child(child, j);
                std::string ptype(ts_node_type(pkg_child));
                if (ptype == "identifier") {
                    return get_node_text(pkg_child, source);
                }
            }
            // Fallback: get the text after "package " keyword
            std::string text = get_node_text(child, source);
            if (text.starts_with("package ")) {
                std::string pkg = text.substr(8);
                // Trim trailing whitespace/newlines
                while (!pkg.empty() && (pkg.back() == '\n' || pkg.back() == '\r' || pkg.back() == ' ')) {
                    pkg.pop_back();
                }
                return pkg;
            }
        }
    }
    return {};
}

// Extract import statements from a Kotlin file
std::set<std::string> kotlin_extract_imports_fqn(TSNode root, const std::string& source) {
    std::set<std::string> imports;
    uint32_t count = ts_node_named_child_count(root);
    for (uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_named_child(root, i);
        std::string type(ts_node_type(child));
        if (type == "import_list") {
            uint32_t ic = ts_node_named_child_count(child);
            for (uint32_t j = 0; j < ic; ++j) {
                TSNode imp = ts_node_named_child(child, j);
                std::string itype(ts_node_type(imp));
                if (itype == "import_header") {
                    // Get the identifier child
                    uint32_t hc = ts_node_named_child_count(imp);
                    for (uint32_t k = 0; k < hc; ++k) {
                        TSNode id = ts_node_named_child(imp, k);
                        std::string idtype(ts_node_type(id));
                        if (idtype == "identifier") {
                            std::string fqn = get_node_text(id, source);
                            imports.insert(fqn);
                            break;
                        }
                    }
                }
            }
        }
        // Some tree-sitter versions put import_header directly as child of source_file
        if (type == "import_header") {
            uint32_t hc = ts_node_named_child_count(child);
            for (uint32_t k = 0; k < hc; ++k) {
                TSNode id = ts_node_named_child(child, k);
                std::string idtype(ts_node_type(id));
                if (idtype == "identifier") {
                    std::string fqn = get_node_text(id, source);
                    imports.insert(fqn);
                    break;
                }
            }
        }
    }
    return imports;
}

// Extract simple names from import FQNs
std::set<std::string> imports_to_simple_names(const std::set<std::string>& fqns) {
    std::set<std::string> names;
    for (const auto& fqn : fqns) {
        auto dot = fqn.rfind('.');
        if (dot != std::string::npos) {
            names.insert(fqn.substr(dot + 1));
        } else {
            names.insert(fqn);
        }
    }
    return names;
}

// Extract type definitions from a file (class, interface, object, typealias, enum)
// Returns simple names of types defined in the file
void kotlin_extract_type_defs_recursive(
    TSNode node, const std::string& source, std::set<std::string>& types)
{
    std::string type(ts_node_type(node));

    if (type == "class_declaration" || type == "interface_declaration" ||
        type == "object_declaration" || type == "type_alias")
    {
        std::string name = kotlin_extract_name(node, source);
        if (!name.empty()) {
            types.insert(name);
        }
    }

    // Recurse into children
    uint32_t count = ts_node_named_child_count(node);
    for (uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_named_child(node, i);
        kotlin_extract_type_defs_recursive(child, source, types);
    }
}

// Kotlin built-in types, JVM types, and standard library types we should never suggest imports for
bool is_kotlin_builtin(const std::string& name) {
    // Single-letter or very short uppercase names are type parameters (T, V, K, R, etc.)
    if (name.length() <= 2 && std::isupper(static_cast<unsigned char>(name[0]))) {
        bool all_upper_or_digit = true;
        for (char c : name) {
            if (!std::isupper(static_cast<unsigned char>(c)) &&
                !std::isdigit(static_cast<unsigned char>(c)) && c != '_') {
                all_upper_or_digit = false;
                break;
            }
        }
        if (all_upper_or_digit) return true;
    }

    // Common generic type parameter patterns: T1, T2, V_, F_, etc.
    if (name.length() <= 3 && std::isupper(static_cast<unsigned char>(name[0]))) {
        bool is_param_pattern = true;
        for (size_t i = 1; i < name.length(); ++i) {
            char c = name[i];
            if (c != '_' && !std::isdigit(static_cast<unsigned char>(c))) {
                is_param_pattern = false;
                break;
            }
        }
        if (is_param_pattern) return true;
    }

    static const std::set<std::string> builtins = {
        // Primitives and basic types
        "Any", "Unit", "Nothing", "Boolean", "Byte", "Short", "Int", "Long",
        "Float", "Double", "Char", "String", "UByte", "UShort", "UInt", "ULong",
        // Collections
        "List", "MutableList", "Set", "MutableSet", "Map", "MutableMap",
        "ArrayList", "HashMap", "HashSet", "LinkedHashMap", "LinkedHashSet",
        "Collection", "MutableCollection", "Iterable", "MutableIterable",
        "Iterator", "MutableIterator", "Sequence",
        // Arrays
        "Array", "IntArray", "LongArray", "ByteArray", "ShortArray",
        "FloatArray", "DoubleArray", "CharArray", "BooleanArray",
        "UIntArray", "ULongArray", "UByteArray", "UShortArray",
        // Functions
        "Function", "Function0", "Function1", "Function2", "Function3",
        // Common stdlib
        "Pair", "Triple", "Result", "Comparable", "Comparator",
        "Exception", "RuntimeException", "IllegalStateException",
        "IllegalArgumentException", "UnsupportedOperationException",
        "IndexOutOfBoundsException", "NullPointerException",
        "NoSuchElementException", "ConcurrentModificationException",
        "NumberFormatException", "ArithmeticException",
        "StringBuilder", "Regex", "MatchResult",
        "Throwable", "Error", "AssertionError",
        "Enum", "Annotation", "Deprecated",
        "Lazy", "Closeable",
        // Numbers
        "Number",
        // Kotlin specials
        "Companion", "KClass",
        // JVM / platform types (common in KMP expect/actual)
        "JvmInline", "JvmStatic", "JvmField", "JvmOverloads", "JvmName",
        "AutoCloseable", "Volatile", "Thread", "Suppress",
        "InterruptedException", "PublishedApi", "Appendable",
        "AbstractList", "AbstractSet", "AbstractMap", "AbstractCollection",
        "Serializable", "Cloneable",
        // Kotlin annotations
        "OptIn", "ExperimentalStdlibApi", "ExperimentalUnsignedTypes",
        // Common Rust-ish patterns used as Kotlin names
        "Self", "Ordering", "Cell", "Data", "Class", "Val",
        // Common generic type parameter names (longer than 2 chars)
        "From", "To", "TLeft", "TRight", "TResult", "TKey", "TValue",
        "TInput", "TOutput",
    };
    return builtins.count(name) > 0;
}

// Extract type references from a Kotlin file — types that are *used* (not defined).
// We look for:
//   - user_type nodes (type references like `Value`, `Evaluator`, etc.)
//   - Supertype declarations
//   - Constructor calls (identifiers before `(` in call expressions)
void kotlin_extract_type_refs_recursive(
    TSNode node, const std::string& source, std::set<std::string>& refs)
{
    std::string type(ts_node_type(node));

    if (type == "user_type") {
        // Get the first simple_identifier child — that's the type name
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            std::string ctype(ts_node_type(child));
            if (ctype == "type_identifier" || ctype == "simple_identifier") {
                std::string name = get_node_text(child, source);
                if (!name.empty() && std::isupper(static_cast<unsigned char>(name[0]))) {
                    refs.insert(name);
                }
                break;  // Only first component (e.g., `Map` from `Map<K, V>`)
            }
        }
    }

    // Recurse
    uint32_t count = ts_node_named_child_count(node);
    for (uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_named_child(node, i);
        kotlin_extract_type_refs_recursive(child, source, refs);
    }
}

// Collect all type definitions in the entire codebase for the registry
struct KotlinFileParseResult {
    std::string file;
    std::string package;
    std::set<std::string> import_fqns;
    std::set<std::string> defined_types;
    std::set<std::string> referenced_types;
};

KotlinFileParseResult parse_kotlin_file_for_imports(
    TSParser* parser, const fs::path& filepath, const std::string& rel_path)
{
    KotlinFileParseResult result;
    result.file = rel_path;

    std::string content = read_file_content(filepath);
    if (content.empty()) return result;

    TSTree* tree = ts_parser_parse_string(parser, nullptr, content.c_str(), content.length());
    if (!tree) return result;

    TSNode root = ts_tree_root_node(tree);

    result.package = kotlin_extract_package(root, content);
    result.import_fqns = kotlin_extract_imports_fqn(root, content);
    kotlin_extract_type_defs_recursive(root, content, result.defined_types);
    kotlin_extract_type_refs_recursive(root, content, result.referenced_types);

    ts_tree_delete(tree);
    return result;
}

}  // anonymous namespace

ImportMapReport build_import_map(const std::string& kotlin_root) {
    ImportMapReport report;

    TSParser* parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_kotlin());

    // Phase 1: Parse all files, build type registry
    std::vector<KotlinFileParseResult> all_files;

    for (const auto& entry : fs::recursive_directory_iterator(kotlin_root)) {
        if (!entry.is_regular_file()) continue;
        std::string path = entry.path().string();
        if (should_skip_path(path)) continue;
        if (!path.ends_with(".kt")) continue;

        std::string rel_path = fs::relative(entry.path(), kotlin_root).string();
        auto parsed = parse_kotlin_file_for_imports(parser, entry.path(), rel_path);
        if (parsed.package.empty() && parsed.defined_types.empty()) continue;

        // Register all defined types
        for (const auto& type_name : parsed.defined_types) {
            KotlinTypeEntry entry;
            entry.name = type_name;
            entry.package = parsed.package;
            entry.fqn = parsed.package.empty() ? type_name : (parsed.package + "." + type_name);
            entry.file = rel_path;
            report.type_registry[type_name].push_back(entry);
        }

        all_files.push_back(std::move(parsed));
    }

    ts_parser_delete(parser);

    // Phase 2: Build a package→types index (for same-package resolution)
    std::map<std::string, std::set<std::string>> types_by_package;
    for (const auto& [name, entries] : report.type_registry) {
        for (const auto& e : entries) {
            types_by_package[e.package].insert(e.name);
        }
    }

    // Phase 3: For each file, compute unresolved types and generate suggestions
    for (const auto& parsed : all_files) {
        KotlinFileInfo info;
        info.file = parsed.file;
        info.package = parsed.package;
        info.import_fqns = parsed.import_fqns;
        info.imports = imports_to_simple_names(parsed.import_fqns);
        info.defined_types = parsed.defined_types;
        info.referenced_types = parsed.referenced_types;

        // Same-package types are auto-available
        std::set<std::string> available = info.imports;
        available.insert(info.defined_types.begin(), info.defined_types.end());
        if (!info.package.empty()) {
            auto it = types_by_package.find(info.package);
            if (it != types_by_package.end()) {
                available.insert(it->second.begin(), it->second.end());
            }
        }

        // Compute unresolved
        for (const auto& ref : info.referenced_types) {
            if (available.count(ref)) continue;
            if (is_kotlin_builtin(ref)) continue;
            info.unresolved.insert(ref);
        }

        // Generate suggestions for unresolved types
        for (const auto& unresolved : info.unresolved) {
            auto it = report.type_registry.find(unresolved);
            if (it == report.type_registry.end()) continue;  // Not found anywhere

            ImportMapReport::ImportSuggestion suggestion;
            suggestion.file = parsed.file;
            suggestion.type_name = unresolved;

            if (it->second.size() == 1) {
                suggestion.suggested_import = it->second[0].fqn;
                suggestion.ambiguous = false;
            } else {
                // Multiple definitions — mark as ambiguous
                suggestion.ambiguous = true;
                suggestion.suggested_import = it->second[0].fqn;  // first as default
                for (const auto& e : it->second) {
                    suggestion.alternatives.push_back(e.fqn + " (" + e.file + ")");
                }
            }
            report.suggestions.push_back(std::move(suggestion));
        }

        report.files.push_back(std::move(info));
    }

    // Sort files by unresolved count (descending)
    std::sort(report.files.begin(), report.files.end(),
        [](const KotlinFileInfo& a, const KotlinFileInfo& b) {
            return a.unresolved.size() > b.unresolved.size();
        });

    return report;
}

void cmd_import_map(const std::string& kotlin_root,
                    const ImportMapOptions& options) {
    std::cerr << "Building import map for " << kotlin_root << "...\n";
    auto report = build_import_map(kotlin_root);
    std::cerr << "Registry: " << report.type_registry.size() << " unique type names\n";
    std::cerr << "Files analyzed: " << report.files.size() << "\n";

    if (options.json) {
        // JSON output
        std::cout << "{\n";
        std::cout << "  \"registry_size\": " << report.type_registry.size() << ",\n";
        std::cout << "  \"files_analyzed\": " << report.files.size() << ",\n";
        std::cout << "  \"total_suggestions\": " << report.suggestions.size() << ",\n";

        // Per-file unresolved
        std::cout << "  \"files\": [\n";
        bool first_file = true;
        for (const auto& fi : report.files) {
            if (!options.filter_file.empty() &&
                fi.file.find(options.filter_file) == std::string::npos) continue;
            if ((int)fi.unresolved.size() < options.min_unresolved) continue;

            if (!first_file) std::cout << ",\n";
            first_file = false;

            std::cout << "    {\"file\": \"" << fi.file
                      << "\", \"package\": \"" << fi.package
                      << "\", \"unresolved_count\": " << fi.unresolved.size()
                      << ", \"unresolved\": [";
            bool first_u = true;
            for (const auto& u : fi.unresolved) {
                if (!first_u) std::cout << ", ";
                first_u = false;
                std::cout << "\"" << u << "\"";
            }
            std::cout << "]}";
        }
        std::cout << "\n  ],\n";

        // Suggestions
        std::cout << "  \"suggestions\": [\n";
        bool first_sug = true;
        for (const auto& s : report.suggestions) {
            if (!options.filter_file.empty() &&
                s.file.find(options.filter_file) == std::string::npos) continue;

            if (!first_sug) std::cout << ",\n";
            first_sug = false;

            std::cout << "    {\"file\": \"" << s.file
                      << "\", \"type\": \"" << s.type_name
                      << "\", \"import\": \"" << s.suggested_import
                      << "\", \"ambiguous\": " << (s.ambiguous ? "true" : "false")
                      << "}";
        }
        std::cout << "\n  ]\n";
        std::cout << "}\n";
        return;
    }

    // Text output
    std::cout << "==========================================================\n";
    std::cout << "IMPORT MAP: Kotlin Type Registry & Resolution\n";
    std::cout << "==========================================================\n\n";
    std::cout << "Type registry:    " << report.type_registry.size() << " unique type names\n";
    std::cout << "Files analyzed:   " << report.files.size() << "\n";
    std::cout << "Import suggestions: " << report.suggestions.size() << "\n\n";

    // Count files with unresolved
    int files_with_unresolved = 0;
    int total_unresolved = 0;
    for (const auto& fi : report.files) {
        if (!fi.unresolved.empty()) {
            files_with_unresolved++;
            total_unresolved += fi.unresolved.size();
        }
    }
    std::cout << "Files with unresolved types: " << files_with_unresolved
              << " / " << report.files.size() << "\n";
    std::cout << "Total unresolved references: " << total_unresolved << "\n\n";

    if (options.summary_only) {
        // Just show per-file counts
        std::cout << std::setw(60) << std::left << "File"
                  << std::setw(10) << "Unresolved"
                  << "\n";
        std::cout << std::string(70, '-') << "\n";

        for (const auto& fi : report.files) {
            if ((int)fi.unresolved.size() < options.min_unresolved) continue;
            if (!options.filter_file.empty() &&
                fi.file.find(options.filter_file) == std::string::npos) continue;

            std::string display = fi.file;
            if (display.length() > 58) {
                display = "..." + display.substr(display.length() - 55);
            }
            std::cout << std::setw(60) << std::left << display
                      << std::setw(10) << fi.unresolved.size()
                      << "\n";
        }
        return;
    }

    // Detailed per-file output
    for (const auto& fi : report.files) {
        if ((int)fi.unresolved.size() < options.min_unresolved) continue;
        if (!options.filter_file.empty() &&
            fi.file.find(options.filter_file) == std::string::npos) continue;

        std::cout << "\n--- " << fi.file << " (" << fi.unresolved.size() << " unresolved) ---\n";
        std::cout << "  package: " << fi.package << "\n";

        for (const auto& u : fi.unresolved) {
            // Find suggestion
            bool found_suggestion = false;
            for (const auto& s : report.suggestions) {
                if (s.file == fi.file && s.type_name == u) {
                    if (s.ambiguous) {
                        std::cout << "  [AMBIGUOUS] " << u << "\n";
                        for (const auto& alt : s.alternatives) {
                            std::cout << "    - " << alt << "\n";
                        }
                    } else {
                        std::cout << "  import " << s.suggested_import << "\n";
                    }
                    found_suggestion = true;
                    break;
                }
            }
            if (!found_suggestion) {
                std::cout << "  [NOT FOUND] " << u << "\n";
            }
        }
    }

    // Show ambiguous types summary
    int ambiguous_count = 0;
    std::map<std::string, std::vector<std::string>> ambiguous_types;
    for (const auto& [name, entries] : report.type_registry) {
        if (entries.size() > 1) {
            ambiguous_count++;
            for (const auto& e : entries) {
                ambiguous_types[name].push_back(e.fqn + " (" + e.file + ")");
            }
        }
    }

    if (ambiguous_count > 0) {
        std::cout << "\n==========================================================\n";
        std::cout << "AMBIGUOUS TYPES (" << ambiguous_count << " names with multiple definitions)\n";
        std::cout << "==========================================================\n\n";

        for (const auto& [name, locs] : ambiguous_types) {
            std::cout << "  " << name << ":\n";
            for (const auto& loc : locs) {
                std::cout << "    - " << loc << "\n";
            }
        }
    }

    // Show types referenced but not found anywhere in the codebase
    std::set<std::string> truly_missing;
    for (const auto& fi : report.files) {
        for (const auto& u : fi.unresolved) {
            if (report.type_registry.find(u) == report.type_registry.end()) {
                truly_missing.insert(u);
            }
        }
    }

    if (!truly_missing.empty()) {
        std::cout << "\n==========================================================\n";
        std::cout << "UNDEFINED TYPES (" << truly_missing.size()
                  << " referenced but not defined anywhere)\n";
        std::cout << "==========================================================\n\n";

        // Count how many files reference each undefined type
        std::map<std::string, int> undefined_freq;
        for (const auto& fi : report.files) {
            for (const auto& u : fi.unresolved) {
                if (truly_missing.count(u)) {
                    undefined_freq[u]++;
                }
            }
        }

        // Sort by frequency
        std::vector<std::pair<int, std::string>> sorted_undefined;
        for (const auto& [name, count] : undefined_freq) {
            sorted_undefined.push_back({count, name});
        }
        std::sort(sorted_undefined.rbegin(), sorted_undefined.rend());

        for (const auto& [count, name] : sorted_undefined) {
            std::cout << "  " << std::setw(30) << std::left << name
                      << " referenced in " << count << " file(s)\n";
        }
    }
}

// ============================================================================
// Compiler Fixup: Parse compiler errors and suggest fixes
// ============================================================================

std::vector<CompilerError> parse_compiler_errors(
    const std::string& error_file, const std::string& kotlin_root)
{
    std::vector<CompilerError> errors;

    std::ifstream file(error_file);
    if (!file.is_open()) {
        std::cerr << "Cannot open error file: " << error_file << "\n";
        return errors;
    }

    // Compute the absolute kotlin_root path for stripping
    fs::path abs_root = fs::absolute(kotlin_root);
    std::string root_prefix = abs_root.string();
    if (!root_prefix.ends_with("/")) root_prefix += "/";

    // Also handle file:// URL prefix from Kotlin compiler
    // Format: e: file:///absolute/path/file.kt:line:col Unresolved reference: 'name'
    // Or:     e: file:///absolute/path/file.kt:line:col message text
    std::string line_str;
    while (std::getline(file, line_str)) {
        if (!line_str.starts_with("e: ")) continue;

        // Extract unresolved reference errors
        auto ref_pos = line_str.find("Unresolved reference");
        if (ref_pos == std::string::npos) continue;

        CompilerError err;
        err.full_message = line_str;

        // Extract the symbol name from: Unresolved reference: 'name'
        // or: Unresolved reference 'name'
        auto quote1 = line_str.find('\'', ref_pos);
        if (quote1 == std::string::npos) continue;
        auto quote2 = line_str.find('\'', quote1 + 1);
        if (quote2 == std::string::npos) continue;
        err.reference = line_str.substr(quote1 + 1, quote2 - quote1 - 1);
        if (err.reference.empty()) continue;

        // Extract file path and line number
        // Strip "e: " prefix and optional "file://" prefix
        std::string location = line_str.substr(3);
        if (location.starts_with("file://")) {
            location = location.substr(7);
        }

        // Parse path:line:col
        // Find first colon that's followed by a digit (line number)
        size_t colon_pos = 0;
        while (true) {
            colon_pos = location.find(':', colon_pos);
            if (colon_pos == std::string::npos) break;
            if (colon_pos + 1 < location.size() &&
                std::isdigit(static_cast<unsigned char>(location[colon_pos + 1]))) {
                break;
            }
            colon_pos++;
        }

        if (colon_pos != std::string::npos) {
            std::string filepath = location.substr(0, colon_pos);

            // Make relative to kotlin_root
            if (filepath.starts_with(root_prefix)) {
                err.file = filepath.substr(root_prefix.size());
            } else {
                // Try relative matching
                auto pos = filepath.find(kotlin_root);
                if (pos != std::string::npos) {
                    err.file = filepath.substr(pos + kotlin_root.size());
                    if (!err.file.empty() && err.file[0] == '/') {
                        err.file = err.file.substr(1);
                    }
                } else {
                    err.file = filepath;
                }
            }

            // Parse line and column
            std::string rest = location.substr(colon_pos + 1);
            auto col_colon = rest.find(':');
            if (col_colon != std::string::npos) {
                try {
                    err.line = std::stoi(rest.substr(0, col_colon));
                    err.col = std::stoi(rest.substr(col_colon + 1));
                } catch (...) {}
            }
        }

        errors.push_back(std::move(err));
    }

    return errors;
}

CompilerFixupReport build_compiler_fixup(
    const std::vector<CompilerError>& errors,
    const ImportMapReport& import_map)
{
    CompilerFixupReport report;
    report.total_errors = 0;  // We only count unresolved ref errors
    report.unresolved_errors = static_cast<int>(errors.size());

    // Build per-file, per-reference counts
    // file → reference → count
    std::map<std::string, std::map<std::string, int>> file_ref_counts;
    for (const auto& err : errors) {
        file_ref_counts[err.file][err.reference]++;
        report.unresolved_counts[err.reference]++;
    }

    // Build a package→types index for same-package resolution
    std::map<std::string, std::set<std::string>> types_by_package;
    for (const auto& [name, entries] : import_map.type_registry) {
        for (const auto& e : entries) {
            types_by_package[e.package].insert(e.name);
        }
    }

    // Build file→package map and file→imports map from the import map data
    std::map<std::string, std::string> file_packages;
    std::map<std::string, std::set<std::string>> file_existing_imports;
    for (const auto& fi : import_map.files) {
        file_packages[fi.file] = fi.package;
        file_existing_imports[fi.file] = fi.import_fqns;
    }

    // For each file, look up each unresolved reference in the registry
    for (const auto& [file, ref_counts] : file_ref_counts) {
        std::string pkg = file_packages[file];

        for (const auto& [ref, count] : ref_counts) {
            FixSuggestion suggestion;
            suggestion.file = file;
            suggestion.reference = ref;
            suggestion.occurrences = count;

            auto it = import_map.type_registry.find(ref);
            if (it == import_map.type_registry.end()) {
                // Not found in registry — could be a function, property, etc.
                report.not_in_registry.insert(ref);
                report.unfixable_errors += count;
                suggestion.suggested_import = "";
                report.by_file[file].push_back(std::move(suggestion));
                continue;
            }

            const auto& entries = it->second;

            // Filter out entries that are:
            // 1. In the same package (would already be available)
            // 2. Private (can't be imported)
            std::vector<const KotlinTypeEntry*> candidates;
            for (const auto& e : entries) {
                if (e.package == pkg) continue;  // Same package, skip
                candidates.push_back(&e);
            }

            if (candidates.empty()) {
                // All definitions are in the same package — shouldn't be unresolved
                // Might be a shadowing issue or the definition is broken
                report.unfixable_errors += count;
                suggestion.suggested_import = "";
                report.by_file[file].push_back(std::move(suggestion));
                continue;
            }

            if (candidates.size() == 1) {
                suggestion.suggested_import = candidates[0]->fqn;
                suggestion.ambiguous = false;
                report.fixable_errors += count;
            } else {
                // Multiple candidates — try to pick the best one
                // Prefer entries from values/ package (canonical re-exports)
                const KotlinTypeEntry* best = nullptr;

                // Priority 1: values.Values.kt (typealias re-exports)
                for (auto* c : candidates) {
                    if (c->file == "values/Values.kt") { best = c; break; }
                }
                // Priority 2: values/ package
                if (!best) {
                    for (auto* c : candidates) {
                        if (c->package.find(".values.") != std::string::npos ||
                            c->package.find(".values") == c->package.size() - 7) {
                            best = c; break;
                        }
                    }
                }
                // Priority 3: Not from __derive_refs, fill_types_for_lint, or Errors.kt
                if (!best) {
                    for (auto* c : candidates) {
                        if (c->file.find("__derive_refs") != std::string::npos) continue;
                        if (c->file.find("fill_types_for_lint") != std::string::npos) continue;
                        if (c->file.find("Errors.kt") != std::string::npos) continue;
                        best = c; break;
                    }
                }
                // Fallback: first candidate
                if (!best) best = candidates[0];

                suggestion.suggested_import = best->fqn;
                suggestion.ambiguous = (candidates.size() > 1);
                for (auto* c : candidates) {
                    suggestion.alternatives.push_back(c->fqn + " (" + c->file + ")");
                }
                if (suggestion.ambiguous) {
                    report.ambiguous_errors += count;
                } else {
                    report.fixable_errors += count;
                }
            }

            report.by_file[file].push_back(std::move(suggestion));
        }
    }

    return report;
}

void CompilerFixupReport::print(bool verbose) const {
    std::cout << "==========================================================\n";
    std::cout << "COMPILER FIXUP REPORT\n";
    std::cout << "==========================================================\n\n";

    std::cout << "Unresolved reference errors: " << unresolved_errors << "\n";
    std::cout << "  Fixable (unique import):   " << fixable_errors << "\n";
    std::cout << "  Ambiguous (multiple defs): " << ambiguous_errors << "\n";
    std::cout << "  Not in registry:           " << unfixable_errors << "\n";
    std::cout << "  Files affected:            " << by_file.size() << "\n\n";

    // Sort files by total error count
    std::vector<std::pair<int, std::string>> file_order;
    for (const auto& [file, suggestions] : by_file) {
        int total = 0;
        for (const auto& s : suggestions) total += s.occurrences;
        file_order.push_back({total, file});
    }
    std::sort(file_order.rbegin(), file_order.rend());

    for (const auto& [total, file] : file_order) {
        const auto& suggestions = by_file.at(file);

        // Count fixable vs ambiguous vs unfixable
        int n_fix = 0, n_amb = 0, n_unf = 0;
        for (const auto& s : suggestions) {
            if (s.suggested_import.empty()) n_unf += s.occurrences;
            else if (s.ambiguous) n_amb += s.occurrences;
            else n_fix += s.occurrences;
        }

        std::cout << "--- " << file << " (" << total << " errors: "
                  << n_fix << " fixable";
        if (n_amb > 0) std::cout << ", " << n_amb << " ambiguous";
        if (n_unf > 0) std::cout << ", " << n_unf << " unfixable";
        std::cout << ") ---\n";

        // Sort suggestions: fixable first, then ambiguous, then unfixable
        auto sorted = suggestions;
        std::sort(sorted.begin(), sorted.end(),
            [](const FixSuggestion& a, const FixSuggestion& b) {
                // Fixable first, then ambiguous, then unfixable
                int a_rank = a.suggested_import.empty() ? 2 : (a.ambiguous ? 1 : 0);
                int b_rank = b.suggested_import.empty() ? 2 : (b.ambiguous ? 1 : 0);
                if (a_rank != b_rank) return a_rank < b_rank;
                return a.occurrences > b.occurrences;
            });

        for (const auto& s : sorted) {
            if (s.suggested_import.empty()) {
                std::cout << "  [NOT FOUND] " << s.reference
                          << " (" << s.occurrences << "x)\n";
            } else if (s.ambiguous) {
                std::cout << "  [AMBIGUOUS]  " << s.reference
                          << " (" << s.occurrences << "x) → "
                          << s.suggested_import << " (best guess)\n";
                if (verbose) {
                    for (const auto& alt : s.alternatives) {
                        std::cout << "               - " << alt << "\n";
                    }
                }
            } else {
                std::cout << "  [FIX] import " << s.suggested_import
                          << "  // " << s.reference
                          << " (" << s.occurrences << "x)\n";
            }
        }
        std::cout << "\n";
    }

    // Summary: top unresolved references not in registry
    if (!not_in_registry.empty()) {
        std::cout << "==========================================================\n";
        std::cout << "TOP UNRESOLVED (not in type registry — functions/properties)\n";
        std::cout << "==========================================================\n\n";

        std::vector<std::pair<int, std::string>> sorted_nr;
        for (const auto& name : not_in_registry) {
            auto it = unresolved_counts.find(name);
            if (it != unresolved_counts.end()) {
                sorted_nr.push_back({it->second, name});
            }
        }
        std::sort(sorted_nr.rbegin(), sorted_nr.rend());

        for (size_t i = 0; i < sorted_nr.size() && i < 40; ++i) {
            std::cout << "  " << std::setw(6) << sorted_nr[i].first
                      << "x  " << sorted_nr[i].second << "\n";
        }
    }
}

void CompilerFixupReport::print_json() const {
    std::cout << "{\n";
    std::cout << "  \"unresolved_errors\": " << unresolved_errors << ",\n";
    std::cout << "  \"fixable\": " << fixable_errors << ",\n";
    std::cout << "  \"ambiguous\": " << ambiguous_errors << ",\n";
    std::cout << "  \"unfixable\": " << unfixable_errors << ",\n";
    std::cout << "  \"files_affected\": " << by_file.size() << ",\n";
    std::cout << "  \"files\": [\n";

    bool first_file = true;
    for (const auto& [file, suggestions] : by_file) {
        if (!first_file) std::cout << ",\n";
        first_file = false;

        std::cout << "    {\"file\": \"" << file << "\", \"fixes\": [\n";
        bool first_fix = true;
        for (const auto& s : suggestions) {
            if (!first_fix) std::cout << ",\n";
            first_fix = false;
            std::cout << "      {\"reference\": \"" << s.reference
                      << "\", \"import\": \"" << s.suggested_import
                      << "\", \"ambiguous\": " << (s.ambiguous ? "true" : "false")
                      << ", \"occurrences\": " << s.occurrences;
            if (!s.alternatives.empty()) {
                std::cout << ", \"alternatives\": [";
                bool first_alt = true;
                for (const auto& alt : s.alternatives) {
                    if (!first_alt) std::cout << ", ";
                    first_alt = false;
                    // Escape quotes in alt
                    std::string escaped;
                    for (char c : alt) {
                        if (c == '"') escaped += "\\\"";
                        else escaped += c;
                    }
                    std::cout << "\"" << escaped << "\"";
                }
                std::cout << "]";
            }
            std::cout << "}";
        }
        std::cout << "\n    ]}";
    }
    std::cout << "\n  ],\n";

    // Not in registry
    std::cout << "  \"not_in_registry\": [\n";
    std::vector<std::pair<int, std::string>> sorted_nr;
    for (const auto& name : not_in_registry) {
        auto it = unresolved_counts.find(name);
        if (it != unresolved_counts.end()) {
            sorted_nr.push_back({it->second, name});
        }
    }
    std::sort(sorted_nr.rbegin(), sorted_nr.rend());
    bool first_nr = true;
    for (const auto& [count, name] : sorted_nr) {
        if (!first_nr) std::cout << ",\n";
        first_nr = false;
        std::cout << "    {\"name\": \"" << name << "\", \"count\": " << count << "}";
    }
    std::cout << "\n  ]\n";
    std::cout << "}\n";
}

void cmd_compiler_fixup(const std::string& kotlin_root,
                        const std::string& error_file,
                        const CompilerFixupOptions& options)
{
    std::cerr << "Parsing compiler errors from " << error_file << "...\n";
    auto errors = parse_compiler_errors(error_file, kotlin_root);
    std::cerr << "Found " << errors.size() << " unresolved reference errors\n";

    std::cerr << "Building type registry from " << kotlin_root << "...\n";
    auto import_map = build_import_map(kotlin_root);
    std::cerr << "Registry: " << import_map.type_registry.size() << " type names, "
              << import_map.files.size() << " files\n";

    auto report = build_compiler_fixup(errors, import_map);

    if (options.json) {
        report.print_json();
    } else {
        report.print(options.verbose);
    }
}

} // namespace ast_distance
