#include "ast_parser.hpp"
#include "callable_identity.hpp"
#include "similarity.hpp"
#include "tree_lstm.hpp"
#include "codebase.hpp"
#include "deep_inventory.hpp"
#include "porting_utils.hpp"
#include "transliteration_similarity.hpp"
#include "transliteration_engine.hpp"
#include "symbol_analysis.hpp"
#include "symbol_extraction.hpp"
#include "symbol_extractor.hpp"
#include "reexport_config.hpp"
#include "ast_config.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <memory>
#include <optional>
#include <regex>
#include <set>
#include <sstream>
#include <ctime>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <cstring>
#include <fcntl.h>
#include <tree_sitter/api.h>

using namespace ast_distance;

static ReexportConfig g_reexport_config;
static AstConfig g_ast_config;
static void print_transliteration_distance(const TransliterationDistance& report, std::ostream& stream);

// Forward declarations for guardrails helpers defined later in this file.

static std::string ltrim_copy(std::string s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) i++;
    s.erase(0, i);
    return s;
}

static std::string rtrim_copy(std::string s) {
    size_t i = s.size();
    while (i > 0 && std::isspace(static_cast<unsigned char>(s[i - 1]))) i--;
    s.erase(i);
    return s;
}

static std::string trim_copy(std::string s) {
    return rtrim_copy(ltrim_copy(std::move(s)));
}

static std::vector<std::string> rust_significant_lines(const std::string& contents) {
    std::vector<std::string> out;
    bool in_block = false;

    std::istringstream iss(contents);
    std::string line;
    while (std::getline(iss, line)) {
        std::string cur = line;

        // Remove block comments (/* ... */) possibly spanning multiple lines.
        while (true) {
            if (in_block) {
                auto end = cur.find("*/");
                if (end == std::string::npos) {
                    cur.clear();
                    break;
                }
                cur = cur.substr(end + 2);
                in_block = false;
                continue;
            }

            auto start = cur.find("/*");
            if (start == std::string::npos) break;
            auto end = cur.find("*/", start + 2);
            if (end == std::string::npos) {
                cur = cur.substr(0, start);
                in_block = true;
                break;
            }
            cur = cur.substr(0, start) + cur.substr(end + 2);
        }

        // Remove line comments (// ...), including doc comments (///, //!).
        auto sl = cur.find("//");
        if (sl != std::string::npos) {
            cur = cur.substr(0, sl);
        }

        cur = trim_copy(cur);
        if (!cur.empty()) {
            out.push_back(cur);
        }
    }

    return out;
}

Language parse_language(const std::string& lang_str) {
    if (lang_str == "rust") return Language::RUST;
    if (lang_str == "kotlin") return Language::KOTLIN;
    if (lang_str == "cpp") return Language::CPP;
    if (lang_str == "python") return Language::PYTHON;
    if (lang_str == "typescript" || lang_str == "ts") return Language::TYPESCRIPT;
    throw std::runtime_error("Unknown language: " + lang_str + " (use rust, kotlin, cpp, python, or typescript)");
}

const char* language_name(Language lang) {
    switch (lang) {
        case Language::RUST: return "Rust";
        case Language::KOTLIN: return "Kotlin";
        case Language::CPP: return "C++";
        case Language::PYTHON: return "Python";
        case Language::TYPESCRIPT: return "TypeScript";
    }
    return "Unknown";
}

const char* language_config_name(Language lang) {
    switch (lang) {
        case Language::RUST: return "rust";
        case Language::KOTLIN: return "kotlin";
        case Language::CPP: return "cpp";
        case Language::PYTHON: return "python";
        case Language::TYPESCRIPT: return "typescript";
    }
    return "unknown";
}

static std::optional<std::filesystem::path> find_repo_root(std::filesystem::path path) {
    std::error_code ec;
    if (path.empty()) return std::nullopt;

    path = path.lexically_normal();
    if (!std::filesystem::is_directory(path, ec)) {
        path = path.parent_path();
    }

    while (!path.empty()) {
        if (std::filesystem::exists(path / ".git", ec)) {
            return path;
        }
        auto parent = path.parent_path();
        if (parent == path) break;
        path = parent;
    }
    return std::nullopt;
}

static std::string lowercase_ascii(std::string s);

static std::string repo_relative_display_path(const std::string& raw_path) {
    std::error_code ec;
    std::filesystem::path input(raw_path);
    std::filesystem::path cwd = std::filesystem::current_path(ec);
    std::filesystem::path absolute =
        input.is_absolute() ? input : (cwd / input);
    absolute = absolute.lexically_normal();

    if (auto repo_root = find_repo_root(absolute)) {
        auto rel = std::filesystem::relative(absolute, *repo_root, ec);
        if (!ec && !rel.empty()) {
            return rel.generic_string();
        }
    }

    if (input.is_relative()) {
        return input.lexically_normal().generic_string();
    }

    // Last resort: never reveal fully-qualified absolute paths in reports.
    // Express the path relative to CWD so output stays portable across
    // checkouts and avoids leaking $HOME / user directories.
    if (!cwd.empty()) {
        auto rel = std::filesystem::relative(absolute, cwd, ec);
        if (!ec && !rel.empty()) {
            return rel.generic_string();
        }
    }
    return absolute.filename().generic_string();
}

static std::string kotlin_namespace_segment_from_source(const std::string& segment) {
    std::string out;
    out.reserve(segment.size());
    for (char c : segment) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        }
    }
    if (!out.empty()) return out;
    return lowercase_ascii(segment);
}

static std::filesystem::path expected_target_relative_path_for_source(const SourceFile& sf,
                                                                      const std::string& target_lang) {
    std::filesystem::path rel(sf.relative_path);
    std::vector<std::string> dirs;
    std::filesystem::path parent = rel.parent_path();
    bool first_segment = true;
    for (const auto& part : parent) {
        std::string segment = part.string();
        if (segment.empty() || segment == ".") continue;
        if (first_segment && (segment == "src" || segment == "include")) {
            first_segment = false;
            continue;
        }
        first_segment = false;
        if (target_lang == "kotlin") {
            dirs.push_back(kotlin_namespace_segment_from_source(segment));
        } else if (target_lang == "typescript") {
            dirs.push_back(SourceFile::to_kebab_case(segment));
        } else {
            dirs.push_back(segment);
        }
    }

    std::string stem = rel.stem().string();
    if (target_lang == "kotlin" && rel.extension() == ".rs" && stem != "mod" && stem != "lib" &&
        !sf.paths.empty()) {
        std::error_code ec;
        std::filesystem::path mod_dir = std::filesystem::path(sf.paths.front()).parent_path() / stem;
        bool has_rs_children = false;
        if (std::filesystem::is_directory(mod_dir, ec)) {
            for (const auto& entry : std::filesystem::directory_iterator(mod_dir, ec)) {
                if (ec) break;
                if (entry.is_regular_file(ec) && entry.path().extension() == ".rs") {
                    has_rs_children = true;
                    break;
                }
            }
        }
        if (has_rs_children) {
            dirs.push_back(kotlin_namespace_segment_from_source(stem));
        }
    }

    std::string target_file = rel.filename().string();
    if (target_lang == "kotlin") {
        target_file = SourceFile::to_pascal_case(stem) + ".kt";
    } else if (target_lang == "typescript") {
        target_file = SourceFile::to_kebab_case(stem) + ".ts";
    } else if (target_lang == "cpp") {
        target_file = stem + ".cpp";
    } else if (target_lang == "python") {
        target_file = lowercase_ascii(stem) + ".py";
    }

    std::filesystem::path out;
    for (const auto& dir : dirs) {
        if (!dir.empty()) out /= dir;
    }
    out /= target_file;
    return out.lexically_normal();
}

static std::string expected_target_qualified_name_for_source(const SourceFile& sf,
                                                            const std::string& target_lang) {
    std::filesystem::path expected = expected_target_relative_path_for_source(sf, target_lang);
    expected.replace_extension("");
    std::string qualified = expected.generic_string();
    std::replace(qualified.begin(), qualified.end(), '/', '.');
    return qualified;
}

static std::string lowercase_ascii(std::string s) {
    for (char& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

static std::string title_ascii_segment(std::string s) {
    s = lowercase_ascii(std::move(s));
    if (!s.empty() && std::isalpha(static_cast<unsigned char>(s[0]))) {
        s[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[0])));
    }
    return s;
}

static std::string rust_identifier_to_kotlin_lower_camel(const std::string& rust_name) {
    std::string name = rust_name;
    if (name.rfind("r#", 0) == 0) {
        name = name.substr(2);
    }

    // Generated Rust parsers use private names like `___action42` and
    // `___pop_Variant9`. Kotlin ports must keep legal identifiers, so parity
    // means `action42` and `popVariant9`, not underscore-prefixed spellings.
    while (!name.empty() && name[0] == '_') {
        name.erase(name.begin());
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

    std::string result = lowercase_ascii(parts[0]);
    for (size_t i = 1; i < parts.size(); ++i) {
        result += title_ascii_segment(parts[i]);
    }
    return result;
}

static std::string expected_target_function_name(
        const std::string& source_name,
        Language source_lang,
        Language target_lang) {
    if (source_lang == Language::RUST && target_lang == Language::KOTLIN) {
        return rust_identifier_to_kotlin_lower_camel(source_name);
    }
    return source_name;
}

static void print_cheat_detection_failure(
        std::ostream& out,
        const std::string& subject,
        const std::vector<std::string>& reasons,
        const std::string& consequence,
        const std::string& remediation = "") {
    out << "\n*** CHEAT DETECTION FAILED ***\n";
    if (!subject.empty()) {
        out << subject << "\n";
    }
    if (!consequence.empty()) {
        out << consequence << "\n";
    }
    out << "Why:\n";
    for (const auto& reason : reasons) {
        out << "  - " << reason << "\n";
    }
    if (!remediation.empty()) {
        out << remediation << "\n";
    }
}

struct RustModReexportHint {
    std::string exported_name;
    std::string expected_target_name;
    std::string actual_rust_path;
    std::string likely_source;
};

static std::vector<std::string> split_rust_path(const std::string& path) {
    std::vector<std::string> parts;
    size_t start = 0;
    while (start <= path.size()) {
        size_t pos = path.find("::", start);
        std::string part = trim_copy(pos == std::string::npos
            ? path.substr(start)
            : path.substr(start, pos - start));
        if (!part.empty()) {
            parts.push_back(part);
        }
        if (pos == std::string::npos) break;
        start = pos + 2;
    }
    return parts;
}

static std::vector<std::string> split_reexport_items(const std::string& items) {
    std::vector<std::string> out;
    std::string current;
    int brace_depth = 0;
    for (char c : items) {
        if (c == '{') ++brace_depth;
        if (c == '}') --brace_depth;
        if (c == ',' && brace_depth == 0) {
            std::string item = trim_copy(current);
            if (!item.empty()) out.push_back(item);
            current.clear();
            continue;
        }
        current.push_back(c);
    }
    std::string item = trim_copy(current);
    if (!item.empty()) out.push_back(item);
    return out;
}

static std::pair<std::string, std::string> parse_reexport_item_name(std::string item) {
    item = trim_copy(std::move(item));
    static const std::regex alias_re(R"(^(.+?)\s+as\s+([A-Za-z_][A-Za-z0-9_]*)$)");
    std::smatch match;
    if (std::regex_match(item, match, alias_re)) {
        std::string actual = trim_copy(match[1].str());
        std::string alias = trim_copy(match[2].str());
        return {actual, alias};
    }
    return {item, ""};
}

static std::string last_rust_path_component(const std::string& path) {
    auto parts = split_rust_path(path);
    if (parts.empty()) return "";
    return parts.back();
}

static std::string module_path_for_reexport(const std::string& actual_path) {
    auto parts = split_rust_path(actual_path);
    if (parts.size() <= 1) return "";
    std::ostringstream out;
    for (size_t i = 0; i + 1 < parts.size(); ++i) {
        if (i > 0) out << "::";
        out << parts[i];
    }
    return out.str();
}

static std::string display_relative_to(const std::filesystem::path& path,
                                       const std::filesystem::path& base) {
    std::error_code ec;
    auto rel = std::filesystem::relative(path, base, ec);
    if (!ec && !rel.empty()) {
        return rel.generic_string();
    }
    return path.lexically_normal().generic_string();
}

static std::string likely_rust_reexport_source_file(
        const std::filesystem::path& mod_rs_path,
        const std::string& module_path) {
    auto modules = split_rust_path(module_path);
    if (modules.empty()) return "";

    std::error_code ec;
    std::filesystem::path mod_abs = std::filesystem::absolute(mod_rs_path, ec);
    if (ec) mod_abs = mod_rs_path;
    std::filesystem::path base = mod_abs.parent_path();
    std::filesystem::path display_base = base;
    std::vector<std::string> relative_modules;

    for (const auto& raw : modules) {
        std::string part = raw;
        if (part == "self") {
            continue;
        }
        if (part == "super") {
            base = base.parent_path();
            display_base = display_base.parent_path();
            continue;
        }
        if (part == "crate") {
            // Without a crate root map, crate-relative reexports are still useful
            // as Rust paths, but a local file guess would be misleading.
            return "";
        }
        relative_modules.push_back(part);
    }

    if (relative_modules.empty()) return "";

    std::filesystem::path file_candidate = base;
    for (size_t i = 0; i + 1 < relative_modules.size(); ++i) {
        file_candidate /= relative_modules[i];
    }
    file_candidate /= relative_modules.back() + ".rs";

    std::filesystem::path mod_candidate = base;
    for (const auto& part : relative_modules) {
        mod_candidate /= part;
    }
    mod_candidate /= "mod.rs";

    if (std::filesystem::exists(file_candidate, ec)) {
        return display_relative_to(file_candidate, display_base);
    }
    if (std::filesystem::exists(mod_candidate, ec)) {
        return display_relative_to(mod_candidate, display_base);
    }
    return display_relative_to(file_candidate, display_base);
}

static void add_rust_mod_reexport_hint(
        std::vector<RustModReexportHint>& hints,
        const std::filesystem::path& mod_rs_path,
        const std::string& actual_path,
        const std::string& exported_name,
        Language source_lang,
        Language target_lang) {
    if (actual_path.empty() || exported_name.empty() || exported_name == "self" || exported_name == "*") {
        return;
    }
    RustModReexportHint hint;
    hint.exported_name = exported_name;
    hint.expected_target_name = expected_target_function_name(exported_name, source_lang, target_lang);
    hint.actual_rust_path = actual_path;
    hint.likely_source = likely_rust_reexport_source_file(
        mod_rs_path, module_path_for_reexport(actual_path));
    hints.push_back(std::move(hint));
}

static std::vector<RustModReexportHint> rust_mod_reexport_hints(
        const std::string& file_path,
        const std::string& source_text,
        Language source_lang,
        Language target_lang) {
    std::vector<RustModReexportHint> hints;
    if (source_lang != Language::RUST) return hints;
    if (std::filesystem::path(file_path).filename() != "mod.rs") return hints;

    std::filesystem::path mod_rs_path(file_path);
    static const std::regex use_line_re(R"(^\s*pub(\([^)]*\))?\s+use\s+(.+)\s*;\s*$)");
    static const std::regex brace_re(R"(^(.+)::\{(.+)\}$)");

    for (const auto& line : rust_significant_lines(source_text)) {
        std::smatch match;
        if (!std::regex_match(line, match, use_line_re)) continue;
        std::string spec = trim_copy(match[2].str());
        if (spec.empty() || spec.find('*') != std::string::npos) continue;

        std::smatch brace_match;
        if (std::regex_match(spec, brace_match, brace_re)) {
            std::string prefix = trim_copy(brace_match[1].str());
            for (const auto& item : split_reexport_items(brace_match[2].str())) {
                auto [actual_item, alias] = parse_reexport_item_name(item);
                if (actual_item == "self" || actual_item == "super" || actual_item == "crate") continue;
                std::string exported = alias.empty() ? last_rust_path_component(actual_item) : alias;
                std::string actual_path = prefix + "::" + actual_item;
                add_rust_mod_reexport_hint(
                    hints, mod_rs_path, actual_path, exported, source_lang, target_lang);
            }
            continue;
        }

        auto [actual_path, alias] = parse_reexport_item_name(spec);
        std::string actual_name = last_rust_path_component(actual_path);
        std::string exported = alias.empty() ? actual_name : alias;
        add_rust_mod_reexport_hint(
            hints, mod_rs_path, actual_path, exported, source_lang, target_lang);
    }

    return hints;
}

// Extract the names of all type-alias declarations in a single source file
// using the existing SymbolExtractor / tree-sitter infrastructure.
//
// Rust `pub type X = ...` parses as `type_item` and SymbolExtractor labels
// it `"type"`. Kotlin `typealias X = ...` parses as `type_alias` and is
// labeled `"typealias"`. We accept either label.
static std::vector<std::string> extract_type_alias_names(
    const std::string& source_text,
    const TSLanguage* ts_lang
) {
    std::vector<std::string> names;
    if (!ts_lang || source_text.empty()) return names;

    TSParser* parser = ts_parser_new();
    if (!parser) return names;
    if (!ts_parser_set_language(parser, ts_lang)) {
        ts_parser_delete(parser);
        return names;
    }
    TSTree* tree = ts_parser_parse_string(
        parser, nullptr, source_text.c_str(), source_text.size());
    if (tree) {
        TSNode root = ts_tree_root_node(tree);
        auto symbols = ast_distance::SymbolExtractor::extract_symbols(
            root, source_text, "", "");
        for (const auto& sym : symbols) {
            if (sym.type == "type" || sym.type == "typealias") {
                if (!sym.name.empty()) names.push_back(sym.name);
            }
        }
        ts_tree_delete(tree);
    }
    ts_parser_delete(parser);
    return names;
}

static void warn_kotlin_suspicious_constructs(
    const std::filesystem::path& source_path,
    Language source_lang,
    const std::filesystem::path& target_path,
    Language target_lang
) {
    if (!(source_lang == Language::RUST && target_lang == Language::KOTLIN)) {
        return;
    }

    std::string content = CodebaseComparator::read_file_to_string(target_path.string());
    if (content.empty()) {
        return;
    }

    auto warn = [&](const std::string& what) {
        std::cerr
            << "Warning: Kotlin-only " << what << " detected in target file: "
            << target_path.string() << "\n"
            << "  Source file: " << source_path.string() << "\n"
            << "  This has no direct Rust equivalent and may hide unfinished transliteration.\n";
    };

    if (content.find("@file:Suppress") != std::string::npos ||
        content.find("@Suppress") != std::string::npos ||
        content.find("SuppressWarnings") != std::string::npos) {
        warn("suppression annotation (@Suppress/@file:Suppress/SuppressWarnings)");
    }

    // Match Kotlin typealiases against Rust `pub type` declarations.
    // A Kotlin `typealias X = Y` paired with Rust `pub type X = Y` is the
    // faithful 1:1 port of a Rust type alias and should not warn. Only warn
    // for Kotlin typealiases that have no matching name on the Rust side --
    // those are Kotlin-only inventions that hide unfinished transliteration
    // (or worse, are wrapper-class shims rebranded as typealiases).
    //
    // Matching is by canonicalized name (snake_case <-> camelCase via
    // IdentifierStats::canonicalize), per the rest of the parity logic.
    std::vector<std::string> kotlin_aliases =
        extract_type_alias_names(content, tree_sitter_kotlin());
    if (!kotlin_aliases.empty()) {
        std::string source_content =
            CodebaseComparator::read_file_to_string(source_path.string());
        std::vector<std::string> rust_aliases =
            extract_type_alias_names(source_content, tree_sitter_rust());

        std::set<std::string> rust_canon;
        for (const auto& n : rust_aliases) {
            rust_canon.insert(IdentifierStats::canonicalize(n));
        }

        for (const auto& kotlin_name : kotlin_aliases) {
            std::string canon = IdentifierStats::canonicalize(kotlin_name);
            if (rust_canon.find(canon) == rust_canon.end()) {
                warn("typealias `" + kotlin_name + "` (no matching `pub type` in Rust source)");
            }
        }
    }
}

void print_usage(const char* program) {
    std::cerr << "AST Distance - Cross-language AST comparison and porting analysis\n\n";
    std::cerr << "Usage:\n";
    std::cerr << "      Loads .ast_distance_config.json when present.\n";
    std::cerr << "      Reporting is always full-detail (file-by-file + function-by-function).\n\n";
    std::cerr << "  " << program << " <file1> <lang1> <file2> <lang2>\n";
    std::cerr << "      Compare AST similarity between two files\n\n";
    std::cerr << "  " << program << " --compare-functions <file1> <lang1> <file2> <lang2> [--with-companions]\n";
    std::cerr << "      Compare strict function-name parity and per-function cosine/line similarity\n\n";
    std::cerr << "  " << program << " --transliterate <source_file> kotlin cpp\n";
    std::cerr << "      Emit AST-driven C++ buffer on stdout; complete span/rule evidence on stderr\n\n";
    std::cerr << "  " << program << " --translit-distance <source_file> kotlin <target_file> cpp\n";
    std::cerr << "      Score emitted C++ against target C++, with fallback and ordered logic evidence\n\n";
    std::cerr << "  " << program << " --scan <directory> <rust|kotlin|cpp|python|typescript>\n";
    std::cerr << "      Scan directory and show file list with import counts\n\n";
    std::cerr << "  " << program << " --deps <directory> <rust|kotlin|cpp|python|typescript>\n";
    std::cerr << "      Build and show dependency graph\n\n";
    std::cerr << "  " << program << " --deep <src_dir> <src_lang> <tgt_dir> <tgt_lang>\n";
    std::cerr << "      Full analysis: AST + deps + TODOs + lint + line ratios\n\n";
    std::cerr << "  " << program << " --numpy-mlx <numpy_dir> <mlx_dir>\n";
    std::cerr << "      Python-focused report: compare two Python codebases and audit residual NumPy usage\n\n";
    std::cerr << "  " << program << " --emberlint <path>\n";
    std::cerr << "      Fast multi-language lint: NumPy usage + common MLX graph breakers (casts, conversions, operators)\n\n";
    std::cerr << "  " << program << " --todos <directory>\n";
    std::cerr << "      Scan for TODO comments with tags and context\n\n";
    std::cerr << "  " << program << " --lint <directory>\n";
    std::cerr << "      Run lint checks (unused params, missing guards)\n\n";
    std::cerr << "  " << program << " --stats <directory>\n";
    std::cerr << "      Show file statistics (line counts, stubs, TODOs)\n\n";
    std::cerr << "Symbol Analysis:\n";
    std::cerr << "  " << program << " --symbols <kotlin_root> <cpp_root> [--json]\n";
    std::cerr << "      Run symbol analysis (duplicates + stubs)\n\n";
    std::cerr << "  " << program << " --symbols-duplicates <kotlin_root> <cpp_root> [--json]\n";
    std::cerr << "      Show duplicate class/struct definitions\n\n";
    std::cerr << "  " << program << " --symbols-stubs <kotlin_root> <cpp_root> [--json]\n";
    std::cerr << "      Show stub files/classes\n\n";
    std::cerr << "  " << program << " --symbols-symbol <kotlin_root> <cpp_root> <symbol> [--json]\n";
    std::cerr << "      Analyze a specific symbol\n\n";
    std::cerr << "  " << program << " --symbol-parity <rust_root> <kotlin_root>\n";
    std::cerr << "      Rust->Kotlin symbol parity analysis\n\n";
        std::cerr << "  " << program << " --symbol-parity <rust_root> <kotlin_root> --kind <function|struct|enum|trait>\n";
    std::cerr << "      Filter by symbol kind\n\n";
    std::cerr << "  " << program << " --symbol-parity <rust_root> <kotlin_root> --file <path>\n";
    std::cerr << "      Filter to specific source file\n\n";
        std::cerr << "  " << program << " --import-map <kotlin_root>\n";
    std::cerr << "      Build type registry and show missing imports per file\n\n";
    std::cerr << "  " << program << " --import-map <kotlin_root> --file <path>\n";
    std::cerr << "      Show imports for a specific file\n\n";
        std::cerr << "Compiler Error Analysis:\n";
    std::cerr << "  " << program << " --compiler-fixup <kotlin_root> <error_file>\n";
    std::cerr << "      Parse compiler errors and suggest import fixes\n\n";
        }

void print_histogram(const std::vector<int>& hist) {
    std::cout << "Node Type Histogram:\n";
    for (int i = 0; i < static_cast<int>(hist.size()); ++i) {
        if (hist[i] > 0) {
            const char* name = node_type_name(static_cast<NodeType>(i));
            std::cout << "  " << std::setw(15) << std::left << name
                      << ": " << hist[i] << "\n";
        }
    }
}

struct NumpyMlxAudit {
    int numpy_imports = 0;
    int numpy_refs = 0;
    int mlx_imports = 0;
    int mlx_refs = 0;
};

static int count_occurrences(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return 0;
    int count = 0;
    size_t pos = 0;
    while (true) {
        pos = haystack.find(needle, pos);
        if (pos == std::string::npos) break;
        count++;
        pos += needle.size();
    }
    return count;
}

static std::string read_file_to_string(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

// Rust files often contain inline `#[cfg(test)]` modules. Kotlin Multiplatform ports place tests in
// `src/commonTest` (or platform-specific test source sets) rather than in `commonMain`.
//
// For similarity scoring (especially `--complete`), treat those inline test modules as out-of-scope
// for the `commonMain` port and strip them from the Rust source before parsing.
//
// This is intentionally a lightweight preprocessor (brace matching) rather than a full Rust parser.
static std::string strip_rust_cfg_test_blocks(const std::string& source) {
    const std::string marker = "#[cfg(test)]";
    std::string out;
    out.reserve(source.size());

    size_t pos = 0;
    while (true) {
        size_t idx = source.find(marker, pos);
        if (idx == std::string::npos) {
            out.append(source, pos, std::string::npos);
            break;
        }

        out.append(source, pos, idx - pos);

        // Skip the attribute and whitespace to the annotated item.
        size_t cur = idx + marker.size();
        while (cur < source.size() && std::isspace(static_cast<unsigned char>(source[cur]))) {
            cur++;
        }

        // If it's a declaration without a body, skip to the next semicolon.
        size_t semi = source.find(';', cur);
        size_t brace = source.find('{', cur);
        if (semi != std::string::npos && (brace == std::string::npos || semi < brace)) {
            pos = semi + 1;
            continue;
        }

        // Otherwise, skip a braced block with simple brace matching.
        if (brace == std::string::npos) {
            // Unexpected shape; fall back to dropping just the attribute.
            pos = cur;
            continue;
        }

        int depth = 1;
        size_t i = brace + 1;
        while (i < source.size() && depth > 0) {
            char c = source[i];
            if (c == '{') depth++;
            else if (c == '}') depth--;
            i++;
        }
        pos = i;
    }

    return out;
}

static NumpyMlxAudit audit_numpy_mlx_python_file(const std::string& path) {
    NumpyMlxAudit a;

    std::ifstream file(path);
    if (!file.is_open()) return a;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string s = buffer.str();

    // Imports
    a.numpy_imports += count_occurrences(s, "import numpy");
    a.numpy_imports += count_occurrences(s, "from numpy import");

    a.mlx_imports += count_occurrences(s, "import mlx");
    a.mlx_imports += count_occurrences(s, "from mlx");
    a.mlx_imports += count_occurrences(s, "_mlx_numpy");

    // References (heuristic)
    a.numpy_refs += count_occurrences(s, "numpy.");
    a.numpy_refs += count_occurrences(s, "np.");

    a.mlx_refs += count_occurrences(s, "mlx.");
    a.mlx_refs += count_occurrences(s, "mx.");
    a.mlx_refs += count_occurrences(s, "_mlx_numpy.");

    return a;
}

void cmd_numpy_mlx(const std::string& numpy_dir, const std::string& mlx_dir) {
    std::cout << "=== NumPy -> MLX (Python) Deep Report ===\n\n";
    std::cout << "NumPy source: " << numpy_dir << "\n";
    std::cout << "MLX target:   " << mlx_dir << "\n\n";

    Codebase source(numpy_dir, "python");
    source.scan();
    source.extract_imports();
    source.build_dependency_graph();

    Codebase target(mlx_dir, "python");
    target.scan();
    target.extract_imports();
    target.build_dependency_graph();

    CodebaseComparator comp(source, target);
    comp.find_matches();
    comp.compute_similarities();

    int files_with_numpy = 0;
    int total_numpy_imports = 0;
    int total_numpy_refs = 0;
    int total_mlx_imports = 0;
    int total_mlx_refs = 0;

    struct Row {
        std::string qualified;
        std::string rel_path;
        NumpyMlxAudit audit;
        float similarity = -1.0f;
    };
    std::vector<Row> rows;
    rows.reserve(comp.matches.size());

    for (const auto& m : comp.matches) {
        auto a = NumpyMlxAudit{};
        const auto& tgt_file = target.files.at(m.target_path);
        for (const auto& tgt_path : tgt_file.paths) {
            auto part = audit_numpy_mlx_python_file(tgt_path);
            a.numpy_imports += part.numpy_imports;
            a.numpy_refs += part.numpy_refs;
            a.mlx_imports += part.mlx_imports;
            a.mlx_refs += part.mlx_refs;
        }

        total_numpy_imports += a.numpy_imports;
        total_numpy_refs += a.numpy_refs;
        total_mlx_imports += a.mlx_imports;
        total_mlx_refs += a.mlx_refs;
        if (a.numpy_imports > 0 || a.numpy_refs > 0) files_with_numpy++;

        Row r;
        r.qualified = m.target_qualified;
        r.rel_path = tgt_file.relative_path;
        r.audit = a;
        r.similarity = m.similarity;
        rows.push_back(std::move(r));
    }

    std::cout << "Matched files: " << comp.matches.size() << "\n";
    std::cout << "Unmatched:     " << comp.unmatched_source.size() << " source, "
              << comp.unmatched_target.size() << " target\n\n";

    std::cout << "=== Residual NumPy Usage In Target ===\n";
    std::cout << "Files with NumPy patterns: " << files_with_numpy << "\n";
    std::cout << "NumPy imports (heuristic): " << total_numpy_imports << "\n";
    std::cout << "NumPy refs (heuristic):    " << total_numpy_refs << "\n\n";

    std::cout << "=== MLX Usage In Target (Informational) ===\n";
    std::cout << "MLX imports (heuristic): " << total_mlx_imports << "\n";
    std::cout << "MLX refs (heuristic):    " << total_mlx_refs << "\n\n";

    if (files_with_numpy > 0) {
        std::cout << "Top files still referencing NumPy (target):\n";
        std::cout << std::setw(40) << std::left << "File"
                  << std::setw(8) << "Sim"
                  << std::setw(8) << "Imp"
                  << std::setw(8) << "Refs"
                  << "Path\n";
        std::cout << std::string(80, '-') << "\n";

        int shown = 0;
        for (const auto& r : rows) {
            if (r.audit.numpy_imports == 0 && r.audit.numpy_refs == 0) continue;
            if (shown++ >= 50) {
                std::cout << "... and " << (files_with_numpy - 50) << " more\n";
                break;
            }
            std::cout << std::setw(40) << std::left << r.qualified.substr(0, 38)
                      << std::setw(8) << std::fixed << std::setprecision(2) << r.similarity
                      << std::setw(8) << r.audit.numpy_imports
                      << std::setw(8) << r.audit.numpy_refs
                      << r.rel_path << "\n";
        }
        std::cout << "\n";
    }

    comp.print_report();
}

struct EmberLintCounts {
    int python_numpy_imports = 0;
    int python_numpy_refs = 0;
    int python_precision_casts = 0;
    int python_tensor_conversions = 0;
    int python_operators = 0;
    int cpp_pybind_numpy_include = 0;
    int cpp_py_array_t = 0;
    int dep_numpy_lines = 0;
};

static bool should_skip_scan_path(const std::filesystem::path& p) {
    auto s = p.string();
    return s.find("/target/") != std::string::npos ||
           s.find("/build/") != std::string::npos ||
           s.find("/build_") != std::string::npos ||
           s.find("/_deps/") != std::string::npos ||
           s.find("/.git/") != std::string::npos ||
           s.find("/.venv/") != std::string::npos ||
           s.find("/__pycache__/") != std::string::npos;
}

static EmberLintCounts emberlint_scan_file(const std::filesystem::path& path) {
    EmberLintCounts c;
    std::ifstream f(path);
    if (!f.is_open()) return c;
    std::stringstream buffer;
    buffer << f.rdbuf();
    std::string s = buffer.str();

    auto ext = path.extension().string();

    if (ext == ".py") {
        // Semantic checks (tree-sitter-python):
        // - NumPy imports and alias-aware references (AST-based, ignores comments/strings)
        // - precision-reducing casts: float(...), int(...), bool(...)
        // - graph-breaking conversions: .numpy(), .item(), .tolist(), numpy.array/asarray(...)
        // - python operators on expressions (excluding subscript indices and obvious string concatenation)
        static thread_local TSParser* parser = nullptr;
        if (parser == nullptr) {
            parser = ts_parser_new();
            if (parser != nullptr) {
                (void)ts_parser_set_language(parser, tree_sitter_python());
            }
        }

        if (parser != nullptr && ts_parser_set_language(parser, tree_sitter_python())) {
            TSTree* tree = ts_parser_parse_string(parser, nullptr, s.c_str(), static_cast<uint32_t>(s.size()));
            if (tree != nullptr) {
                TSNode root = ts_tree_root_node(tree);

                auto node_text = [&](TSNode n) -> std::string {
                    uint32_t start = ts_node_start_byte(n);
                    uint32_t end = ts_node_end_byte(n);
                    if (end > start && end <= s.size()) {
                        return s.substr(start, end - start);
                    }
                    return "";
                };

                // Collect NumPy aliases and count from-imports.
                std::set<std::string> numpy_aliases;
                int from_numpy_imports = 0;

                auto is_numpy_module = [](const std::string& mod) -> bool {
                    // Avoid false positives like "numpy_to_mlx".
                    return mod == "numpy" || mod.rfind("numpy.", 0) == 0;
                };

                std::function<void(TSNode)> collect_imports = [&](TSNode n) {
                    const char* t = ts_node_type(n);
                    if (strcmp(t, "import_statement") == 0) {
                        uint32_t cc = ts_node_child_count(n);
                        for (uint32_t i = 0; i < cc; ++i) {
                            TSNode ch = ts_node_child(n, i);
                            if (!ts_node_is_named(ch)) continue;
                            const char* ct = ts_node_type(ch);
                            if (strcmp(ct, "dotted_name") == 0) {
                                std::string mod = node_text(ch);
                                if (is_numpy_module(mod)) {
                                    // `import numpy` or `import numpy.linalg` binds name `numpy`.
                                    numpy_aliases.insert("numpy");
                                }
                            } else if (strcmp(ct, "aliased_import") == 0) {
                                std::string mod;
                                std::string alias;
                                uint32_t icc = ts_node_child_count(ch);
                                for (uint32_t j = 0; j < icc; ++j) {
                                    TSNode ich = ts_node_child(ch, j);
                                    if (!ts_node_is_named(ich)) continue;
                                    const char* it = ts_node_type(ich);
                                    if (strcmp(it, "dotted_name") == 0) {
                                        mod = node_text(ich);
                                    } else if (strcmp(it, "identifier") == 0) {
                                        alias = node_text(ich);
                                    }
                                }
                                if (!mod.empty() && is_numpy_module(mod)) {
                                    // `import numpy as np` or `import numpy.linalg as la`
                                    if (!alias.empty()) numpy_aliases.insert(alias);
                                    else numpy_aliases.insert("numpy");
                                }
                            }
                        }
                    } else if (strcmp(t, "import_from_statement") == 0) {
                        // Ignore relative imports (`from . import ...`).
                        bool has_relative = false;
                        uint32_t cc = ts_node_child_count(n);
                        for (uint32_t i = 0; i < cc; ++i) {
                            TSNode ch = ts_node_child(n, i);
                            if (!ts_node_is_named(ch)) continue;
                            if (strcmp(ts_node_type(ch), "relative_import") == 0) {
                                has_relative = true;
                                break;
                            }
                        }
                        if (!has_relative) {
                            // First dotted_name child is the module.
                            TSNode module_dn{};
                            bool found = false;
                            for (uint32_t i = 0; i < cc; ++i) {
                                TSNode ch = ts_node_child(n, i);
                                if (!ts_node_is_named(ch)) continue;
                                if (strcmp(ts_node_type(ch), "dotted_name") == 0) {
                                    module_dn = ch;
                                    found = true;
                                    break;
                                }
                            }
                            if (found) {
                                std::string mod = node_text(module_dn);
                                if (is_numpy_module(mod)) {
                                    from_numpy_imports++;
                                }
                            }
                        }
                    }

                    uint32_t cc = ts_node_child_count(n);
                    for (uint32_t i = 0; i < cc; ++i) {
                        collect_imports(ts_node_child(n, i));
                    }
                };

                collect_imports(root);
                c.python_numpy_imports += static_cast<int>(numpy_aliases.size()) + from_numpy_imports;

                std::function<bool(TSNode, const char*)> subtree_has_type = [&](TSNode n, const char* want) -> bool {
                    if (strcmp(ts_node_type(n), want) == 0) return true;
                    uint32_t cc = ts_node_child_count(n);
                    for (uint32_t i = 0; i < cc; ++i) {
                        if (subtree_has_type(ts_node_child(n, i), want)) return true;
                    }
                    return false;
                };

                std::function<void(TSNode, const std::string&, bool)> walk =
                    [&](TSNode n, const std::string& current_fn, bool in_subscript_index) {
                        const char* t = ts_node_type(n);
                        std::string type_s(t);

                        std::string fn = current_fn;
                        if (type_s == "function_definition") {
                            TSNode name_node = ts_node_child_by_field_name(n, "name", 4);
                            if (!ts_node_is_null(name_node)) {
                                fn = node_text(name_node);
                            } else {
                                // Fallback: first identifier child
                                uint32_t ncc = ts_node_child_count(n);
                                for (uint32_t i = 0; i < ncc; ++i) {
                                    TSNode ch = ts_node_child(n, i);
                                    if (strcmp(ts_node_type(ch), "identifier") == 0) {
                                        fn = node_text(ch);
                                        break;
                                    }
                                }
                            }
                        }

                        bool in_stringy_fn = (!fn.empty() &&
                                              (fn.find("hash") != std::string::npos ||
                                               fn.find("Hash") != std::string::npos ||
                                               fn.find("str") != std::string::npos ||
                                               fn.find("Str") != std::string::npos));

                        if (type_s == "attribute") {
                            TSNode obj_node = ts_node_child_by_field_name(n, "object", 6);
                            if (!ts_node_is_null(obj_node) && strcmp(ts_node_type(obj_node), "identifier") == 0) {
                                std::string obj = node_text(obj_node);
                                if (numpy_aliases.count(obj)) {
                                    c.python_numpy_refs++;
                                }
                            }
                        }

                        if (type_s == "call") {
                            TSNode func_node = ts_node_child_by_field_name(n, "function", 8);
                            if (!ts_node_is_null(func_node)) {
                                const char* ft = ts_node_type(func_node);
                                if (strcmp(ft, "identifier") == 0) {
                                    std::string callee = node_text(func_node);
                                    if (callee == "float" || callee == "int" || callee == "bool") {
                                        // Avoid flagging obvious constant parsing (e.g. float("3") / float(3)).
                                        TSNode args = ts_node_child_by_field_name(n, "arguments", 9);
                                        if (ts_node_is_null(args)) {
                                            // Tree-sitter-python uses argument_list; be robust.
                                            uint32_t cc2 = ts_node_child_count(n);
                                            for (uint32_t i = 0; i < cc2; ++i) {
                                                TSNode ch = ts_node_child(n, i);
                                                if (ts_node_is_named(ch) && strcmp(ts_node_type(ch), "argument_list") == 0) {
                                                    args = ch;
                                                    break;
                                                }
                                            }
                                        }

                                        bool count_cast = false;
                                        if (!ts_node_is_null(args) && ts_node_named_child_count(args) > 0) {
                                            TSNode first = ts_node_named_child(args, 0);
                                            const char* at = ts_node_type(first);
                                            bool is_literal = (strcmp(at, "integer") == 0 ||
                                                               strcmp(at, "float") == 0 ||
                                                               strcmp(at, "string") == 0 ||
                                                               strcmp(at, "true") == 0 ||
                                                               strcmp(at, "false") == 0 ||
                                                               strcmp(at, "none") == 0);
                                            count_cast = !is_literal;
                                        }
                                        if (count_cast) {
                                            c.python_precision_casts++;
                                        }
                                    }
                                } else if (strcmp(ft, "attribute") == 0) {
                                    TSNode attr_name_node = ts_node_child_by_field_name(func_node, "attribute", 9);
                                    TSNode attr_obj_node = ts_node_child_by_field_name(func_node, "object", 6);
                                    std::string attr_name;
                                    if (!ts_node_is_null(attr_name_node)) {
                                        attr_name = node_text(attr_name_node);
                                    }

                                    if (attr_name == "numpy") {
                                        c.python_tensor_conversions++;
                                    } else if (attr_name == "item" || attr_name == "tolist") {
                                        c.python_tensor_conversions++;
                                    } else if (attr_name == "array" || attr_name == "asarray") {
                                        // numpy.array/asarray(...)
                                        if (!ts_node_is_null(attr_obj_node) &&
                                            strcmp(ts_node_type(attr_obj_node), "identifier") == 0) {
                                            std::string obj = node_text(attr_obj_node);
                                            if (numpy_aliases.count(obj)) {
                                                c.python_tensor_conversions++;
                                            }
                                        }
                                    }
                                }
                            }
                        } else if (type_s == "binary_operator") {
                            // operator token is an unnamed child.
                            std::string op;
                            uint32_t cc = ts_node_child_count(n);
                            for (uint32_t i = 0; i < cc; ++i) {
                                TSNode ch = ts_node_child(n, i);
                                if (!ts_node_is_named(ch)) {
                                    op = ts_node_type(ch);
                                    break;
                                }
                            }

                            bool is_op = (op == "+" || op == "-" || op == "*" || op == "/" ||
                                          op == "//" || op == "%" || op == "**" || op == "@");
                            if (is_op && !in_subscript_index && !in_stringy_fn) {
                                bool is_non_tensor_op = false;
                                if (op == "+") {
                                    // Heuristic: avoid string concatenation.
                                    if (ts_node_named_child_count(n) >= 2) {
                                        TSNode left = ts_node_named_child(n, 0);
                                        TSNode right = ts_node_named_child(n, ts_node_named_child_count(n) - 1);
                                        if (subtree_has_type(left, "string") || subtree_has_type(right, "string")) {
                                            is_non_tensor_op = true;
                                        }
                                    }
                                }
                                if (!is_non_tensor_op) {
                                    c.python_operators++;
                                }
                            }
                        }

                        // Recurse
                        if (type_s == "subscript") {
                            // Only treat the index/slice expression as safe for operators.
                            TSNode index = ts_node_child_by_field_name(n, "subscript", 9);
                            if (ts_node_is_null(index)) {
                                index = ts_node_child_by_field_name(n, "index", 5);
                            }
                            if (ts_node_is_null(index)) {
                                index = ts_node_child_by_field_name(n, "slice", 5);
                            }

                            uint32_t cc2 = ts_node_child_count(n);
                            for (uint32_t i = 0; i < cc2; ++i) {
                                TSNode ch = ts_node_child(n, i);
                                bool child_is_index = (!ts_node_is_null(index) && ts_node_eq(ch, index));
                                walk(ch, fn, in_subscript_index || child_is_index);
                            }
                            return;
                        }

                        uint32_t cc2 = ts_node_child_count(n);
                        for (uint32_t i = 0; i < cc2; ++i) {
                            walk(ts_node_child(n, i), fn, in_subscript_index);
                        }
                    };

                walk(root, "", false);
                ts_tree_delete(tree);
            }
        }
    } else if (ext == ".cpp" || ext == ".cc" || ext == ".h" || ext == ".hpp") {
        // Only count real includes / real typed usage, not string literals in tools.
        static const std::regex include_numpy(R"(^\s*#\s*include\s*[<"]pybind11/numpy\.h[>"])",
                                             std::regex_constants::multiline);
        static const std::regex py_array_t(R"(\bpy::array_t\s*<)");

        c.cpp_pybind_numpy_include += static_cast<int>(
            std::distance(std::sregex_iterator(s.begin(), s.end(), include_numpy), std::sregex_iterator()));
        c.cpp_py_array_t += static_cast<int>(
            std::distance(std::sregex_iterator(s.begin(), s.end(), py_array_t), std::sregex_iterator()));
    } else {
        // Dependency/config files: detect actual dependencies (not docstring conventions or comments).
        if (path.filename() == "pyproject.toml" ||
            path.filename() == "environment.yml" ||
            path.filename() == "requirements.txt" ||
            path.string().find("requirements/") != std::string::npos) {
            std::istringstream lines(s);
            std::string line;

            auto strip_comment = [](const std::string& line2) -> std::string {
                auto p = line2.find('#');
                if (p == std::string::npos) return line2;
                return line2.substr(0, p);
            };

            if (path.filename() == "pyproject.toml") {
                bool in_dep_array = false;
                while (std::getline(lines, line)) {
                    std::string no_comment = strip_comment(line);
                    if (no_comment.find_first_not_of(" \t\r\n") == std::string::npos) continue;

                    if (!in_dep_array) {
                        static const std::regex dep_start(R"(^\s*(dependencies|requires|optional-dependencies)\s*=\s*\[)");
                        if (std::regex_search(no_comment, dep_start)) {
                            in_dep_array = true;
                        } else {
                            continue;
                        }
                    }

                    if (no_comment.find("\"numpy") != std::string::npos ||
                        no_comment.find("'numpy") != std::string::npos) {
                        c.dep_numpy_lines++;
                    }

                    if (no_comment.find(']') != std::string::npos) {
                        in_dep_array = false;
                    }
                }
            } else if (path.filename() == "environment.yml") {
                static const std::regex env_dep(R"(^\s*-\s*numpy(\b|[<>=!~]))");
                while (std::getline(lines, line)) {
                    std::string no_comment = strip_comment(line);
                    if (std::regex_search(no_comment, env_dep)) {
                        c.dep_numpy_lines++;
                    }
                }
            } else {
                // requirements*.txt and requirements/...
                static const std::regex req_dep(R"(^\s*numpy(\b|[<>=!~\[]))", std::regex_constants::icase);
                while (std::getline(lines, line)) {
                    std::string no_comment = strip_comment(line);
                    if (no_comment.find_first_not_of(" \t\r\n") == std::string::npos) continue;
                    if (std::regex_search(no_comment, req_dep)) {
                        c.dep_numpy_lines++;
                    }
                }
            }
        }
    }

    return c;
}

void cmd_emberlint(const std::string& root) {
    std::cout << "=== EmberLint (fast checks) ===\n\n";
    std::cout << "Path: " << root << "\n\n";

    EmberLintCounts total;

    struct Hit {
        std::string rel;
        EmberLintCounts c;
    };
    std::vector<Hit> hits;

    std::filesystem::path rp(root);
    if (std::filesystem::is_regular_file(rp)) {
        auto c = emberlint_scan_file(rp);
        total.python_numpy_imports += c.python_numpy_imports;
        total.python_numpy_refs += c.python_numpy_refs;
        total.python_precision_casts += c.python_precision_casts;
        total.python_tensor_conversions += c.python_tensor_conversions;
        total.python_operators += c.python_operators;
        total.cpp_pybind_numpy_include += c.cpp_pybind_numpy_include;
        total.cpp_py_array_t += c.cpp_py_array_t;
        total.dep_numpy_lines += c.dep_numpy_lines;
        hits.push_back(Hit{rp.filename().string(), c});
    } else {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(rp)) {
            if (!entry.is_regular_file()) continue;
            if (should_skip_scan_path(entry.path())) continue;

            auto c = emberlint_scan_file(entry.path());
            total.python_numpy_imports += c.python_numpy_imports;
            total.python_numpy_refs += c.python_numpy_refs;
            total.python_precision_casts += c.python_precision_casts;
            total.python_tensor_conversions += c.python_tensor_conversions;
            total.python_operators += c.python_operators;
            total.cpp_pybind_numpy_include += c.cpp_pybind_numpy_include;
            total.cpp_py_array_t += c.cpp_py_array_t;
            total.dep_numpy_lines += c.dep_numpy_lines;

            bool has_hard_issue =
                (c.python_numpy_imports != 0 ||
                 c.python_numpy_refs != 0 ||
                 c.python_precision_casts != 0 ||
                 c.python_tensor_conversions != 0 ||
                 c.cpp_pybind_numpy_include != 0 ||
                 c.cpp_py_array_t != 0 ||
                 c.dep_numpy_lines != 0);
            if (!has_hard_issue) {
                continue;
            }

            hits.push_back(Hit{std::filesystem::relative(entry.path(), rp).string(), c});
        }
    }

    std::cout << "Summary:\n";
    std::cout << "  Python NumPy imports: " << total.python_numpy_imports << "\n";
    std::cout << "  Python NumPy refs:    " << total.python_numpy_refs << "\n";
    std::cout << "  Precision casts:      " << total.python_precision_casts << "\n";
    std::cout << "  Tensor conversions:   " << total.python_tensor_conversions << "\n";
    std::cout << "  Python operators:     " << total.python_operators << "\n";
    std::cout << "  C++ pybind NumPy:     " << total.cpp_pybind_numpy_include << "\n";
    std::cout << "  C++ py::array_t:      " << total.cpp_py_array_t << "\n";
    std::cout << "  Dep/config 'numpy':   " << total.dep_numpy_lines << "\n\n";

    if (!hits.empty()) {
        std::sort(hits.begin(), hits.end(), [](const Hit& a, const Hit& b) {
            auto score = [](const EmberLintCounts& c) {
                return 10 * c.python_numpy_imports +
                       3 * c.python_numpy_refs +
                       8 * c.python_precision_casts +
                       8 * c.python_tensor_conversions +
                       20 * c.cpp_pybind_numpy_include +
                       5 * c.cpp_py_array_t +
                       c.dep_numpy_lines;
            };
            return score(a.c) > score(b.c);
        });

        std::cout << "Top hits:\n";
        std::cout << std::setw(8) << "PyImp"
                  << std::setw(8) << "PyRef"
                  << std::setw(8) << "Cast"
                  << std::setw(8) << "Conv"
                  << std::setw(8) << "Ops"
                  << std::setw(8) << "CppInc"
                  << std::setw(8) << "ArrT"
                  << std::setw(8) << "Deps"
                  << "Path\n";
        std::cout << std::string(80, '-') << "\n";

        int shown = 0;
        for (const auto& h : hits) {
            if (shown++ >= 50) {
                std::cout << "... and " << (hits.size() - 50) << " more\n";
                break;
            }
            std::cout << std::setw(8) << h.c.python_numpy_imports
                      << std::setw(8) << h.c.python_numpy_refs
                      << std::setw(8) << h.c.python_precision_casts
                      << std::setw(8) << h.c.python_tensor_conversions
                      << std::setw(8) << h.c.python_operators
                      << std::setw(8) << h.c.cpp_pybind_numpy_include
                      << std::setw(8) << h.c.cpp_py_array_t
                      << std::setw(8) << h.c.dep_numpy_lines
                      << h.rel << "\n";
        }
        std::cout << "\n";
    }

    bool ok = (total.python_numpy_imports == 0 &&
               total.python_numpy_refs == 0 &&
               total.python_precision_casts == 0 &&
               total.python_tensor_conversions == 0 &&
               total.cpp_pybind_numpy_include == 0 &&
               total.cpp_py_array_t == 0 &&
               total.dep_numpy_lines == 0);
    if (ok) {
        std::cout << "OK: no issues found.\n";
    } else {
        std::cout << "FAIL: issues found.\n";
    }
}

void cmd_scan(const std::string& dir, const std::string& lang) {
    Codebase cb(dir, lang);
    cb.scan();
    cb.extract_imports();

    std::cout << "=== Scanned " << cb.files.size() << " " << lang << " files ===\n\n";

    std::cout << std::setw(40) << std::left << "Qualified Name"
              << std::setw(8) << "Imports"
              << "Path\n";
    std::cout << std::string(80, '-') << "\n";

    for (const auto& [path, sf] : cb.files) {
        std::cout << std::setw(40) << std::left << sf.qualified_name.substr(0, 38)
                  << std::setw(8) << sf.imports.size()
                  << sf.relative_path << "\n";
    }
}

void cmd_deps(const std::string& dir, const std::string& lang) {
    Codebase cb(dir, lang);
    cb.scan();
    cb.extract_imports();
    cb.build_dependency_graph();

    cb.print_summary();

    std::cout << "\n=== Files by Dependent Count ===\n\n";
    std::cout << std::setw(40) << std::left << "File"
              << std::setw(10) << "Deps"
              << std::setw(10) << "DepBy"
              << "Status\n";
    std::cout << std::string(70, '-') << "\n";

    auto ranked = cb.ranked_by_dependents();
    for (const auto* sf : ranked) {
        std::string status;
        if (sf->dependent_count >= 5) {
            status = "CORE";
        } else if (sf->dependent_count == 0) {
            status = "leaf";
        }

        std::cout << std::setw(40) << std::left << sf->qualified_name.substr(0, 38)
                  << std::setw(10) << sf->dependency_count
                  << std::setw(10) << sf->dependent_count
                  << status << "\n";
    }

    // Show top dependencies for most-depended files
    std::cout << "\n=== Core Files (most dependents) ===\n";
    auto roots = cb.root_files(3);
    for (const auto* sf : roots) {
        std::cout << "\n" << sf->qualified_name << " (" << sf->dependent_count << " dependents):\n";
        std::cout << "  Imported by:\n";
        int count = 0;
        for (const auto& dep : sf->imported_by) {
            if (count++ >= 5) {
                std::cout << "    ... and " << (sf->imported_by.size() - 5) << " more\n";
                break;
            }
            std::cout << "    - " << cb.files[dep].qualified_name << "\n";
        }
    }
}

void cmd_rank(const std::string& src_dir, const std::string& src_lang,
              const std::string& tgt_dir, const std::string& tgt_lang) {
    Codebase source(src_dir, src_lang);
    source.scan();
    source.extract_imports();
    source.build_dependency_graph();

    Codebase target(tgt_dir, tgt_lang);
    target.scan();
    target.extract_imports();
    target.build_dependency_graph();

    CodebaseComparator comp(source, target);
    comp.find_matches();
    comp.compute_similarities();

    comp.print_report();
}

static std::string markdown_code_span(const std::string& text) {
    std::string escaped;
    escaped.reserve(text.size());
    for (char c : text) {
        if (c == '`') {
            escaped.push_back('\'');
        } else if (c == '|') {
            escaped += "\\|";
        } else {
            escaped.push_back(c);
        }
    }
    return "`" + escaped + "`";
}

static std::string markdown_symbol_list(const std::vector<std::string>& names) {
    if (names.empty()) return "_none_";
    std::ostringstream out;
    for (size_t i = 0; i < names.size(); ++i) {
        if (i > 0) out << ", ";
        out << markdown_code_span(names[i]);
    }
    return out.str();
}

static std::string parity_cell(int matched, int source_total, int target_total) {
    std::ostringstream out;
    out << matched << "/" << source_total << " matched";
    if (target_total != source_total) {
        out << " (target " << target_total << ")";
    }
    return out.str();
}

static std::vector<CodebaseComparator::ProvenanceProposal>
dedupe_provenance_proposals(std::vector<CodebaseComparator::ProvenanceProposal> proposals) {
    std::vector<CodebaseComparator::ProvenanceProposal> deduped;
    std::set<std::tuple<std::string, std::string, std::string, std::string>> seen;
    for (auto& proposal : proposals) {
        auto key = std::make_tuple(
            proposal.source_path,
            proposal.target_path,
            proposal.current_header,
            proposal.proposed_header);
        if (!seen.insert(key).second) continue;
        deduped.push_back(std::move(proposal));
    }
    return deduped;
}

static std::vector<CodebaseComparator::ProvenanceProposal>
collect_provenance_proposals(const std::vector<CodebaseComparator::Match>& matches) {
    std::vector<CodebaseComparator::ProvenanceProposal> proposals;
    for (const auto& m : matches) {
        proposals.insert(
            proposals.end(),
            m.provenance_proposals.begin(),
            m.provenance_proposals.end());
    }
    return dedupe_provenance_proposals(std::move(proposals));
}

static void write_port_lint_proposed_changes_file(
        const std::vector<CodebaseComparator::ProvenanceProposal>& proposals,
        const std::string& source_display,
        const std::string& target_display) {
    std::time_t now = std::time(nullptr);
    char date_buf[100];
    std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", std::localtime(&now));

    std::ofstream report("port_lint_proposed_changes.md");
    report << "# port-lint Proposed Changes\n\n";
    report << "**Generated:** " << date_buf << "\n";
    if (!source_display.empty()) report << "**Source:** " << source_display << "\n";
    if (!target_display.empty()) report << "**Target:** " << target_display << "\n";
    report << "\n";
    report << "These are review proposals only. They are emitted when a Rust -> Kotlin "
              "pair matches only after fallback normalization, so the existing "
              "`port-lint` header is not an exact provenance match.\n\n";

    if (proposals.empty()) {
        report << "_No fallback provenance matches detected._\n";
        return;
    }

    report << "| Target file | Current header | Proposed header | Source path | Reason |\n";
    report << "|-------------|----------------|-----------------|-------------|--------|\n";
    for (const auto& proposal : proposals) {
        report << "| " << markdown_code_span(proposal.target_path)
               << " | " << markdown_code_span(proposal.current_header)
               << " | " << markdown_code_span(proposal.proposed_header)
               << " | " << markdown_code_span(proposal.source_path)
               << " | " << markdown_code_span(proposal.reason)
               << " |\n";
    }
}

static SourceFile provenance_file_from_path(const std::string& path, bool is_source) {
    SourceFile file;
    file.paths.push_back(path);
    std::filesystem::path p(path);
    file.filename = p.filename().string();
    file.stem = p.stem().string();
    file.extension = p.extension().string();
    file.relative_path = is_source
        ? CodebaseComparator::canonical_source_annotation_path(path)
        : repo_relative_display_path(path);
    file.qualified_name = SourceFile::make_qualified_name(file.relative_path);
    return file;
}

static std::vector<CodebaseComparator::ProvenanceProposal>
provenance_proposals_for_direct_file_pair(const std::string& file1,
                                          Language lang1,
                                          const std::string& file2,
                                          Language lang2) {
    std::string source_path;
    std::string target_path;
    if (lang1 == Language::RUST && lang2 == Language::KOTLIN) {
        source_path = file1;
        target_path = file2;
    } else if (lang1 == Language::KOTLIN && lang2 == Language::RUST) {
        source_path = file2;
        target_path = file1;
    } else {
        return {};
    }

    SourceFile source = provenance_file_from_path(source_path, true);
    SourceFile target = provenance_file_from_path(target_path, false);
    target.transliterated_from = PortingAnalyzer::extract_transliterated_from(target_path);
    if (target.transliterated_from.empty() ||
        CodebaseComparator::is_test_transliteration(target.transliterated_from)) {
        return {};
    }

    auto match = CodebaseComparator::exact_transliteration_header_match_result(source, target);
    if (!match.normalized_fallback) {
        return {};
    }
    return {match.proposal};
}

static void print_direct_provenance_proposals(
        const std::vector<CodebaseComparator::ProvenanceProposal>& proposals) {
    if (proposals.empty()) return;

    std::cout << "\n=== Provenance Header Fallback ===\n";
    std::cout << "This file pair matched only after fallback normalization; "
                 "do not treat the current header as an exact match.\n";
    for (const auto& proposal : proposals) {
        std::cout << "Target:   " << proposal.target_path << "\n";
        std::cout << "Current:  " << proposal.current_header << "\n";
        std::cout << "Proposed: " << proposal.proposed_header << "\n";
        std::cout << "Why:      " << proposal.reason << "\n";
    }
}

static std::string match_target_label_for_report(const CodebaseComparator::Match& m) {
    std::string label = m.target_qualified;
    if (m.is_stub) {
        label += " [STUB]";
    } else if (!m.zero_reasons.empty()) {
        label += " [ZERO]";
    }
    if (m.matched_by_normalized_provenance) {
        label += " [PROVENANCE-FALLBACK]";
    }
    return label;
}

static void write_match_detail_block(std::ostream& report,
                                     const CodebaseComparator::Match& m,
                                     int rank) {
    report << "### " << rank << ". " << m.source_qualified << "\n\n";
    report << "- **Target:** `" << match_target_label_for_report(m) << "`";
    report << "\n";
    report << "- **Similarity:** " << std::fixed << std::setprecision(2) << m.similarity << "\n";
    report << "- **Dependents:** " << m.source_dependents << "\n";
    report << "- **Priority Score:** " << std::fixed << std::setprecision(1)
           << m.priority_score() << "\n";
    report << "- **Functions:** "
           << parity_cell(m.matched_function_count, m.source_function_count, m.target_function_count)
           << "\n";
    report << "- **Missing functions:** " << markdown_symbol_list(m.missing_functions) << "\n";
    report << "- **Types:** "
           << parity_cell(m.matched_type_count, m.source_type_count, m.target_type_count)
           << "\n";
    report << "- **Missing types:** " << markdown_symbol_list(m.missing_types) << "\n";
    if (m.source_test_function_count > 0) {
        report << "- **Tests:** " << m.matched_test_function_count << "/"
               << m.source_test_function_count << " matched\n";
    }
    for (const auto& warning : m.provenance_warnings) {
        report << "- **Provenance warning:** " << warning << "\n";
    }
    for (const auto& proposal : m.provenance_proposals) {
        report << "- **Proposed provenance header:** "
               << markdown_code_span(proposal.proposed_header)
               << " (current: " << markdown_code_span(proposal.current_header) << ")\n";
    }
    if (m.todo_count > 0) report << "- **TODOs:** " << m.todo_count << "\n";
    if (m.lint_count > 0) report << "- **Lint issues:** " << m.lint_count << "\n";
    report << "\n";
}

void generate_reports(const Codebase& source, const Codebase& target,
                      const CodebaseComparator& comp,
                      const std::vector<CodebaseComparator::Match>& ranked,
                      const std::vector<const SourceFile*>& missing,
                      const std::vector<std::pair<float, const CodebaseComparator::Match*>>& doc_gaps,
                      int incomplete_count,
                      int total_src_doc_lines,
                      int total_tgt_doc_lines,
                      const std::vector<CodebaseComparator::Match>& reexport_matches,
                      const std::vector<const SourceFile*>& reexport_missing) {
    (void)incomplete_count;

    auto write_reexport_section = [&](std::ofstream& out) {
        if (reexport_matches.empty() && reexport_missing.empty()) return;
        out << "## Reexport / Wiring Modules\n\n";
        out << "These files match `reexport_modules` patterns in `"
            << g_reexport_config.config_path << "`. They are filtered out of\n"
            << "normal priority and missing-file ladders because they are wiring\n"
            << "modules, not direct logic ports. Consult them for call-site routing;\n"
            << "do not treat them as the next implementation target by default.\n\n";
        if (!reexport_matches.empty()) {
            out << "### Matched\n\n";
            out << "| Source | Target | Path |\n";
            out << "|--------|--------|------|\n";
            for (const auto& m : reexport_matches) {
                out << "| `" << m.source_qualified << "` | `" << m.target_qualified
                    << "` | `" << m.source_path << "` |\n";
            }
            out << "\n";
        }
        if (!reexport_missing.empty()) {
            out << "### Missing\n\n";
            out << "| Source | Expected target | Deps | Source path | Expected path |\n";
            out << "|--------|-----------------|------|-------------|---------------|\n";
            for (const auto* sf : reexport_missing) {
                auto expected_path = expected_target_relative_path_for_source(*sf, target.language);
                out << "| `" << sf->qualified_name << "` | `"
                    << expected_target_qualified_name_for_source(*sf, target.language)
                    << "` | " << sf->dependent_count
                    << " | `" << sf->relative_path << "` | `"
                    << expected_path.generic_string() << "` |\n";
            }
            out << "\n";
        }
    };

    struct MissingSourceSurface {
        const SourceFile* file = nullptr;
        int function_count = 0;
        int test_count = 0;
        int type_count = 0;
        std::vector<std::string> functions;
        std::vector<std::string> types;

        int symbol_count() const {
            return function_count + type_count;
        }
    };

    std::vector<MissingSourceSurface> missing_surfaces;
    missing_surfaces.reserve(missing.size());
    ASTParser report_parser;
    Language report_source_lang = parse_language(source.language);
    TSParser* missing_symbol_parser = ts_parser_new();
    bool missing_symbol_parser_ready = false;
    if (source.language == "rust") {
        missing_symbol_parser_ready = ts_parser_set_language(missing_symbol_parser, tree_sitter_rust());
    } else if (source.language == "cpp") {
        missing_symbol_parser_ready = ts_parser_set_language(missing_symbol_parser, tree_sitter_cpp());
    } else if (source.language == "typescript") {
        missing_symbol_parser_ready = ts_parser_set_language(missing_symbol_parser, tree_sitter_typescript());
    } else if (source.language == "kotlin") {
        missing_symbol_parser_ready = ts_parser_set_language(missing_symbol_parser, tree_sitter_kotlin());
    } else if (source.language == "python") {
        missing_symbol_parser_ready = ts_parser_set_language(missing_symbol_parser, tree_sitter_python());
    }

    for (const auto* sf : missing) {
        MissingSourceSurface surface;
        surface.file = sf;
        try {
            auto funcs = report_parser.extract_function_infos_from_files(sf->paths, report_source_lang);
            surface.function_count = static_cast<int>(funcs.size());
            for (const auto& f : funcs) {
                if (f.is_test) surface.test_count++;
                if (!f.name.empty()) surface.functions.push_back(f.name);
            }
        } catch (...) {
            // Missing-file surface is diagnostic; keep reporting even if one file fails to parse.
        }

        if (missing_symbol_parser_ready) {
            std::string text = CodebaseComparator::read_files_to_string(sf->paths);
            if (!text.empty()) {
                TSTree* tree = ts_parser_parse_string(
                    missing_symbol_parser, nullptr, text.c_str(), text.size());
                if (tree) {
                    TSNode root = ts_tree_root_node(tree);
                    auto symbols = SymbolExtractor::extract_symbols(
                        root, text, sf->package.path, sf->relative_path);
                    std::set<std::string> seen_types;
                    for (const auto& sym : symbols) {
                        if (sym.name.empty()) continue;
                        std::string key = IdentifierStats::canonicalize(sym.name);
                        if (key.empty()) key = sym.name;
                        if (!seen_types.insert(key).second) continue;
                        surface.types.push_back(sym.name);
                    }
                    surface.type_count = static_cast<int>(surface.types.size());
                    ts_tree_delete(tree);
                }
            }
        }
        missing_surfaces.push_back(std::move(surface));
    }
    ts_parser_delete(missing_symbol_parser);

    auto missing_by_symbol_surface = missing_surfaces;
    std::sort(missing_by_symbol_surface.begin(), missing_by_symbol_surface.end(),
        [](const MissingSourceSurface& a, const MissingSourceSurface& b) {
            if (a.symbol_count() != b.symbol_count()) return a.symbol_count() > b.symbol_count();
            if (a.file->dependent_count != b.file->dependent_count) {
                return a.file->dependent_count > b.file->dependent_count;
            }
            return a.file->qualified_name < b.file->qualified_name;
        });
    std::unordered_map<const SourceFile*, const MissingSourceSurface*> missing_surface_by_file;
    for (const auto& surface : missing_surfaces) {
        missing_surface_by_file[surface.file] = &surface;
    }
    
    std::cout << "\n=== Generating Reports ===\n\n";
    
    // Calculate statistics
    int total_source = source.files.size();
    int total_target_logical = target.files.size();
    int total_target_physical = 0;
    for (auto const& [key, val] : target.files) {
        (void)key;
        total_target_physical += static_cast<int>(val.paths.size());
    }
    int matched = comp.matches.size();
    float completion_pct = (static_cast<float>(matched) / static_cast<float>(total_source)) * 100.0f;
    
    // Count quality distribution. Stubs are always critical regardless of
    // whatever similarity score they carry, and are excluded from the average.
    int excellent = 0, good = 0, critical = 0;
    float avg_similarity = 0.0f;        // Average inline-code (function body) cosine
    float avg_doc_similarity = 0.0f;    // Average documentation cosine
    int avg_count = 0;
    for (const auto& m : comp.matches) {
        if (m.is_stub) { critical++; continue; }
        avg_similarity += m.similarity;
        avg_doc_similarity += m.doc_similarity;
        avg_count++;
        if (m.similarity >= 0.85) excellent++;
        else if (m.similarity >= 0.60) good++;
        else critical++;
    }
    if (avg_count > 0) {
        avg_similarity /= avg_count;
        avg_doc_similarity /= avg_count;
    }
    float matched_denominator = matched > 0 ? static_cast<float>(matched) : 1.0f;

    int total_source_functions = 0;
    int total_matched_functions = 0;
    int total_target_functions = 0;
    int total_source_types = 0;
    int total_matched_types = 0;
    int total_target_types = 0;
    int zeroed_files = 0;
    for (const auto& m : ranked) {
        total_source_functions += m.source_function_count;
        total_matched_functions += m.matched_function_count;
        total_target_functions += m.target_function_count;
        total_source_types += m.source_type_count;
        total_matched_types += m.matched_type_count;
        total_target_types += m.target_type_count;
        if (!m.zero_reasons.empty()) zeroed_files++;
    }
    int missing_source_functions = 0;
    int missing_source_types = 0;
    int missing_source_symbols = 0;
    int missing_source_files_with_symbols = 0;
    for (const auto& surface : missing_surfaces) {
        total_source_functions += surface.function_count;
        total_source_types += surface.type_count;
        missing_source_functions += surface.function_count;
        missing_source_types += surface.type_count;
        missing_source_symbols += surface.symbol_count();
        if (surface.symbol_count() > 0) missing_source_files_with_symbols++;
    }
    const int total_source_symbols = total_source_functions + total_source_types;
    const int total_matched_symbols = total_matched_functions + total_matched_types;
    const int total_target_symbols = total_target_functions + total_target_types;

    auto parity_count_cell = [](int matched_count, int source_count, int target_count) {
        std::ostringstream out;
        out << matched_count << "/" << source_count << " matched";
        if (target_count > 0) out << " (target " << target_count << ")";
        return out.str();
    };
    auto parity_pct_cell = [](int matched_count, int source_count) {
        if (source_count <= 0) return std::string("N/A");
        std::ostringstream out;
        out << std::fixed << std::setprecision(1)
            << (100.0f * static_cast<float>(matched_count) / static_cast<float>(source_count))
            << "%";
        return out.str();
    };
    auto avg_similarity_cell = [&]() {
        if (avg_count <= 0) return std::string("N/A (no matched function-bearing files)");
        std::ostringstream out;
        out << std::fixed << std::setprecision(2) << avg_similarity;
        return out.str();
    };
    
    // Get current date/time as string
    std::time_t now = std::time(nullptr);
    char date_buf[100];
    std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", std::localtime(&now));
    const std::string source_display_path = repo_relative_display_path(source.root_path);
    const std::string target_display_path = repo_relative_display_path(target.root_path);
    const auto provenance_proposals = collect_provenance_proposals(ranked);
    
    // 1. Generate port_status_report.md
    {
        auto status_order = ranked;
        std::sort(status_order.begin(), status_order.end(),
            [](const CodebaseComparator::Match& a, const CodebaseComparator::Match& b) {
                if (a.similarity != b.similarity) return a.similarity < b.similarity;
                if (a.zero_reasons.empty() != b.zero_reasons.empty()) {
                    return !a.zero_reasons.empty();
                }
                if (a.source_dependents != b.source_dependents) {
                    return a.source_dependents > b.source_dependents;
                }
                if (a.symbol_deficit() != b.symbol_deficit()) {
                    return a.symbol_deficit() > b.symbol_deficit();
                }
                return a.source_qualified < b.source_qualified;
            });

        std::ofstream report("port_status_report.md");
        report << "# Code Port - Progress Report\n\n";
        report << "**Generated:** " << date_buf << "\n";
        report << "**Source:** " << source_display_path << "\n";
        report << "**Target:** " << target_display_path << "\n\n";
        
        report << "## Executive Summary\n\n";
        report << "Function counts below cover bodies; abstract/interface signatures are listed separately in deep_symbol_inventory.txt.\n\n";
        report << "| Metric | Count | Percentage |\n";
        report << "|--------|-------|------------|\n";
        report << "| Function parity | "
               << parity_count_cell(total_matched_functions, total_source_functions, total_target_functions)
               << " | " << parity_pct_cell(total_matched_functions, total_source_functions) << " |\n";
        report << "| Class/type parity | "
               << parity_count_cell(total_matched_types, total_source_types, total_target_types)
               << " | " << parity_pct_cell(total_matched_types, total_source_types) << " |\n";
        report << "| Combined symbol parity | "
               << parity_count_cell(total_matched_symbols, total_source_symbols, total_target_symbols)
               << " | " << parity_pct_cell(total_matched_symbols, total_source_symbols) << " |\n";
        report << "| Average function body similarity | " << avg_similarity_cell() << " | inline-code cosine |\n";
        report << "| Average documentation similarity | " << std::fixed << std::setprecision(2)
               << avg_doc_similarity << " | doc text cosine |\n";
        report << "| Missing source functions | " << missing_source_functions << " | 0% parity until ported |\n";
        report << "| Missing source classes/types | " << missing_source_types << " | 0% parity until ported |\n";
        report << "| Missing source symbol files | " << missing_source_files_with_symbols
               << " | " << missing_source_symbols << " symbols |\n";
        report << "| Cheat/scoring failures | " << zeroed_files << " | forced to 0% |\n";
        report << "| Total source files | " << total_source << " | 100% |\n";
        report << "| Target units (paired) | " << total_target_logical << " | - |\n";
        report << "| Target files (total) | " << total_target_physical << " | - |\n";
        report << "| Porting progress | " << matched << " | "
               << std::fixed << std::setprecision(1)
               << completion_pct << "% (matched) |\n";
        report << "| Missing files | " << missing.size() << " | "
               << std::fixed << std::setprecision(1)
               << (static_cast<float>(missing.size()) / total_source * 100.0f) << "% |\n";
        if (!reexport_missing.empty()) {
            report << "| Reexport/wiring files | " << reexport_missing.size() << " | consult-only |\n";
        }
        report << "\n";
        if (ranked.empty()) {
            report << "**Report warning:** no source/target files were matched. "
                      "Check the source and target roots or missing `port-lint` provenance before trusting this run.\n\n";
        }
        
        report << "## Port Quality Analysis\n\n";
        report << "**Average Function Similarity:** " << avg_similarity_cell() << "\n\n";
        report << "Similarity in this report is the required function-by-function body/parameter score. "
                  "Class/type parity and symbol deficits are reported beside it; whole-file shape is diagnostic only.\n\n";
        report << "**Work Distribution:**\n";
        report << "- Critical (<0.60): " << critical << " files ("
               << std::fixed << std::setprecision(1) << (static_cast<float>(critical) / matched_denominator * 100.0f) << "% of matched)\n";
        report << "- Needs review (0.60-0.84): " << good << " files ("
               << std::fixed << std::setprecision(1) << (static_cast<float>(good) / matched_denominator * 100.0f) << "% of matched)\n";
        report << "- Excellent (>=0.85): " << excellent << " files ("
               << std::fixed << std::setprecision(1) << (static_cast<float>(excellent) / matched_denominator * 100.0f) << "% of matched)\n";
        report << "\n";

        report << "## Worst Function Scores First\n\n";
        report << "Every matched file is listed from lowest function body/parameter similarity upward. "
                  "Missing symbol names are not capped.\n\n";
        report << "| Rank | Source | Target | Function similarity | Functions | Missing functions | Types | Missing types | Tests | Symbol deficit | Priority |\n";
        report << "|------|--------|--------|---------------------|-----------|-------------------|-------|---------------|-------|----------------|----------|\n";
        int detail_rank = 1;
        for (const auto& m : status_order) {
            std::string tests = "-";
            if (m.source_test_function_count > 0) {
                tests = std::to_string(m.matched_test_function_count) + "/" +
                        std::to_string(m.source_test_function_count);
            }
            report << "| " << detail_rank++ << " | `" << m.source_qualified << "` | `"
                   << match_target_label_for_report(m) << "` | "
                   << std::fixed << std::setprecision(2) << m.similarity << " | "
                   << parity_cell(m.matched_function_count, m.source_function_count, m.target_function_count)
                   << " | " << markdown_symbol_list(m.missing_functions) << " | "
                   << parity_cell(m.matched_type_count, m.source_type_count, m.target_type_count)
                   << " | " << markdown_symbol_list(m.missing_types) << " | "
                   << tests << " | " << m.symbol_deficit() << " | "
                   << std::fixed << std::setprecision(1) << m.priority_score() << " |\n";
        }
        report << "\n";

        report << "## Cheat Detection / Scoring Failures\n\n";
        bool wrote_failures = false;
        for (const auto& m : status_order) {
            if (m.zero_reasons.empty()) continue;
            wrote_failures = true;
            report << "- `" << m.source_qualified << "` -> `"
                   << match_target_label_for_report(m)
                   << "`: function-by-function score forced to 0. "
                   << CodebaseComparator::join_reasons(m.zero_reasons) << "\n";
        }
        if (!wrote_failures) {
            report << "_None detected._\n";
        }
        report << "\n";
        
        report << "### Critical Ports (Similarity < 0.60, Worst First)\n\n";
        report << "These files need significant work:\n\n";
        for (const auto& m : status_order) {
            if (m.similarity < 0.60) {
                report << "- `" << m.source_qualified << "` -> `"
                       << match_target_label_for_report(m)
                       << "` (" << std::fixed << std::setprecision(2) << m.similarity;
                if (m.source_dependents > 0) report << ", " << m.source_dependents << " deps";
                report << ")\n";
            }
        }
	        report << "\n";

	        report << "## Incorrect Ports (Missing Types)\n\n";
	        report << "These files are matched (often via `// port-lint`) but appear to be missing one or more type declarations\n";
	        report << "present in the Rust source file.\n\n";
	        report << "| Source | Target | Missing types | Examples |\n";
	        report << "|--------|--------|---------------|----------|\n";
	        int shown_incorrect = 0;
	        for (const auto& m : ranked) {
	            if (m.source_type_count == 0) continue;
	            if (m.type_coverage >= 1.0f) continue;
	            shown_incorrect++;
	            report << "| `" << m.source_qualified << "` | `"
                       << match_target_label_for_report(m) << "` | "
	                   << (m.source_type_count - m.matched_type_count) << "/" << m.source_type_count << " | ";
	            if (!m.missing_types.empty()) {
	                report << markdown_symbol_list(m.missing_types);
	            } else {
	                report << "-";
	            }
	            report << " |\n";
	        }
	        if (shown_incorrect == 0) {
	            report << "| _None detected_ | | | |\n";
	        }
	        report << "\n";
	        
	        report << "## High Priority Missing Files\n\n";
	        if (missing.empty()) {
	            report << "No missing files detected.\n\n";
	        } else {
	            report << "| Rank | Source file | Expected target | Deps | Functions | Classes/types | Symbols | Source path | Expected path |\n";
	            report << "|------|-------------|-----------------|------|-----------|---------------|---------|-------------|---------------|\n";
	            int shown_missing = 0;
	            for (const auto& surface : missing_by_symbol_surface) {
	                const auto* sf = surface.file;
	                auto expected_path = expected_target_relative_path_for_source(*sf, target.language);
	                shown_missing++;
	                report << "| " << shown_missing << " | `" << sf->qualified_name << "` | "
	                       << "`" << expected_target_qualified_name_for_source(*sf, target.language)
	                       << "` | " << sf->dependent_count << " | "
	                       << surface.function_count << " | "
	                       << surface.type_count << " | "
	                       << surface.symbol_count() << " | `"
	                       << sf->relative_path << "` | `"
	                       << expected_path.generic_string() << "` |\n";
	            }
	            report << "\n";
	        }
	        
	        report << "## Documentation Gaps\n\n";
	        float doc_coverage_pct = 0.0f;
	        bool docs_missing = false;
	        if (total_src_doc_lines > 0) {
	            doc_coverage_pct = 100.0f * total_tgt_doc_lines / total_src_doc_lines;
	            docs_missing = doc_coverage_pct < 85.0f;
	        }
	        if (docs_missing) {
	            report << "Documentation correspondence is incomplete; it is reported separately from implementation scoring.\n\n";
	        }
	        report << "**Documentation line amount:** " << total_tgt_doc_lines << " / "
	               << total_src_doc_lines << " lines (";
	        if (total_src_doc_lines > 0) {
	            report << std::fixed << std::setprecision(0)
	                   << (100.0f * total_tgt_doc_lines / total_src_doc_lines) << "%)\n\n";
	        } else {
	            report << "N/A)\n\n";
	        }
	        
	        report << "Documentation gaps (>20%), complete list:\n\n";
	        if (doc_gaps.empty()) {
	            report << "No significant documentation line-count gaps found; text/reference correspondence is separate.\n\n";
	        } else {
	            for (const auto& [gap, m] : doc_gaps) {
	                report << "- `" << m->source_qualified << "` - " 
	                       << std::fixed << std::setprecision(0) << (gap * 100) << "% gap ("
	                       << m->source_doc_lines << " → " << m->target_doc_lines << " lines)\n";
	            }
	            report << "\n";
	        }
	        write_reexport_section(report);

	        std::cout << "✅ Generated: port_status_report.md\n";
	    }
    
    // 2. Generate high_priority_ports.md
    {
        std::ofstream report("high_priority_ports.md");
        report << "# High Priority Ports - Action Plan\n\n";
        
        report << "## Files by Impact\n\n";
        report << "Priority = deps * 1,000,000 + SymDeficit * 10,000"
                  " + SrcSymbols * 100 + (1 - function similarity) * 10\n\n";
        report << "Dependency fanout is ranked first so the ladder favors ports that clear"
                  " downstream compilation failures fastest.\n\n";
        report << "This list is complete and includes function/type detail for every matched file. "
                  "Function similarity is the required body/parameter comparison; file-level shape does not rescue a port. "
                  "Function counts cover bodies; abstract/interface signatures are listed separately in deep_symbol_inventory.txt.\n\n";
        report << "| Rank | Source | Target | Function similarity | Deps | Functions | Missing functions | Types | Missing types | SymDeficit | SrcSymbols | Priority |\n";
        report << "|------|--------|--------|------------|------|-----------|-------------------|-------|---------------|-----------|------------|----------|\n";

        int rank = 1;
        for (const auto& m : ranked) {
            float priority = m.priority_score();
            report << "| " << rank++ << " | `" << m.source_qualified << "` | `"
                   << match_target_label_for_report(m) << "` | "
                   << std::fixed << std::setprecision(2) << m.similarity << " | "
                   << m.source_dependents << " | "
                   << parity_cell(m.matched_function_count, m.source_function_count, m.target_function_count)
                   << " | " << markdown_symbol_list(m.missing_functions) << " | "
                   << parity_cell(m.matched_type_count, m.source_type_count, m.target_type_count)
                   << " | " << markdown_symbol_list(m.missing_types) << " | "
                   << m.symbol_deficit() << " | " << m.source_symbol_surface() << " | "
                   << std::fixed << std::setprecision(1) << priority << " |\n";
        }
        report << "\n";

        report << "## Cheat Detection / Scoring Failures\n\n";
        bool wrote_priority_failures = false;
        for (const auto& m : ranked) {
            if (m.zero_reasons.empty()) continue;
            wrote_priority_failures = true;
            report << "- `" << m.source_qualified << "` -> `"
                   << match_target_label_for_report(m)
                   << "`: function-by-function score forced to 0. "
                   << CodebaseComparator::join_reasons(m.zero_reasons) << "\n";
        }
        if (!wrote_priority_failures) {
            report << "_None detected._\n";
        }
        report << "\n";
        
        report << "## Critical Issues (Function Similarity < 0.60 with Dependencies)\n\n";
        bool has_critical = false;
        for (const auto& m : ranked) {
            if (m.similarity < 0.60 && m.source_dependents > 0) {
                if (!has_critical) {
                    report << "These files need immediate attention:\n\n";
                    has_critical = true;
                }
                report << "- **" << m.source_qualified << "** → `"
                       << match_target_label_for_report(m) << "`\n";
                report << "  - Function similarity: " << std::fixed << std::setprecision(2) << m.similarity << "\n";
                report << "  - Dependencies: " << m.source_dependents << "\n";
                report << "  - Functions: "
                       << parity_cell(m.matched_function_count, m.source_function_count, m.target_function_count)
                       << "\n";
                report << "  - Missing functions: " << markdown_symbol_list(m.missing_functions) << "\n";
                report << "  - Types: "
                       << parity_cell(m.matched_type_count, m.source_type_count, m.target_type_count)
                       << "\n";
                report << "  - Missing types: " << markdown_symbol_list(m.missing_types) << "\n";
                if (!m.zero_reasons.empty()) {
                    report << "  - Scoring failure: "
                           << CodebaseComparator::join_reasons(m.zero_reasons) << "\n";
                }
                if (m.todo_count > 0) report << "  - TODOs: " << m.todo_count << "\n";
                if (m.lint_count > 0) report << "  - Lint issues: " << m.lint_count << "\n";
                report << "\n";
            }
        }
	        if (!has_critical) {
	            report << "No critical issues with dependencies.\n\n";
	        }

	        report << "## Missing Files (by Dependents)\n\n";
	        if (missing.empty()) {
	            report << "No missing files detected.\n\n";
	        } else {
	            report << "| Rank | Source file | Expected target | Deps | Functions | Classes/types | Symbols | Source path | Expected path |\n";
	            report << "|------|-------------|-----------------|------|-----------|---------------|---------|-------------|---------------|\n";
	            int shown_missing = 0;
	            for (const auto* sf : missing) {
	                auto surface_it = missing_surface_by_file.find(sf);
	                int function_count = 0;
	                int type_count = 0;
	                int symbol_count = 0;
	                if (surface_it != missing_surface_by_file.end()) {
	                    function_count = surface_it->second->function_count;
	                    type_count = surface_it->second->type_count;
	                    symbol_count = surface_it->second->symbol_count();
	                }
	                auto expected_path = expected_target_relative_path_for_source(*sf, target.language);
	                shown_missing++;
	                report << "| " << shown_missing << " | `" << sf->qualified_name << "` | "
	                       << "`" << expected_target_qualified_name_for_source(*sf, target.language)
	                       << "` | " << sf->dependent_count << " | "
	                       << function_count << " | " << type_count << " | " << symbol_count
	                       << " | `" << sf->relative_path << "` | `"
	                       << expected_path.generic_string() << "` |\n";
	            }
	            report << "\n";
	        }
	        write_reexport_section(report);

	        std::cout << "✅ Generated: high_priority_ports.md\n";
	    }
    
    // 3. Generate NEXT_ACTIONS.md
    {
        std::ofstream report("NEXT_ACTIONS.md");
        report << "# Immediate Actions - High-Value Files\n\n";
        report << "Based on AST analysis, here are the concrete next steps.\n\n";
        
        // Parity (function/symbol/class) is the end-state measurement; the
        // average cosine lines beneath it report inline-code body similarity
        // and documentation similarity per file as established diagnostics.
        report << "## Summary\n\n";
        report << "- **Files Present:** " << matched << "/" << total_source
               << " (" << std::fixed << std::setprecision(1) << completion_pct << "%)\n";
        report << "- **Function parity:** "
               << parity_count_cell(total_matched_functions, total_source_functions, total_target_functions)
               << " — " << parity_pct_cell(total_matched_functions, total_source_functions) << "\n";
        report << "- **Class/type parity:** "
               << parity_count_cell(total_matched_types, total_source_types, total_target_types)
               << " — " << parity_pct_cell(total_matched_types, total_source_types) << "\n";
        report << "- **Combined symbol parity:** "
               << parity_count_cell(total_matched_symbols, total_source_symbols, total_target_symbols)
               << " — " << parity_pct_cell(total_matched_symbols, total_source_symbols) << "\n";
        report << "- **Average inline-code cosine:** " << std::fixed << std::setprecision(2)
               << avg_similarity << " (function body across "
               << avg_count << " matched files)\n";
        report << "- **Average documentation cosine:** " << std::fixed << std::setprecision(2)
               << avg_doc_similarity << " (doc text across "
               << avg_count << " matched files)\n";
        report << "- **Cheat-zeroed Files:** " << zeroed_files << "\n";
        report << "- **Critical Issues:** " << critical << " files with <0.60 function similarity\n";
        report << "- **Needs Review:** " << good << " files with 0.60-0.84 function similarity\n";
        report << "- **Excellent:** " << excellent << " files with >=0.85 function similarity\n\n";
        
        report << "## Priority 1: Fix Incomplete High-Dependency Files\n\n";
        int p1_count = 0;
        for (const auto& m : ranked) {
            if (m.similarity < 0.85 && m.source_dependents >= 10) {
                p1_count++;
                report << "### " << p1_count << ". " << m.source_qualified << "\n";
                report << "- **Similarity:** " << std::fixed << std::setprecision(2) 
                       << m.similarity << " (needs " << std::fixed << std::setprecision(0)
                       << ((0.85f - m.similarity) * 100.0f) << "% improvement)\n";
                report << "- **Dependencies:** " << m.source_dependents << "\n";
                report << "- **Priority Score:** " << std::fixed << std::setprecision(1)
                       << m.priority_score() << "\n";
                report << "- **Functions:** "
                       << parity_cell(m.matched_function_count, m.source_function_count, m.target_function_count)
                       << "\n";
                report << "- **Missing functions:** " << markdown_symbol_list(m.missing_functions) << "\n";
                report << "- **Types:** "
                       << parity_cell(m.matched_type_count, m.source_type_count, m.target_type_count)
                       << "\n";
                report << "- **Missing types:** " << markdown_symbol_list(m.missing_types) << "\n";
                if (m.symbol_deficit() > 0) {
                    report << "- **Symbol Deficit:** " << m.symbol_deficit()
                           << " (functions: " << m.function_deficit()
                           << ", types: " << m.type_deficit() << ")\n";
                }
                if (m.source_test_function_count > 0) {
                    int missing_tests = m.source_test_function_count
                                      - m.matched_test_function_count;
                    if (missing_tests > 0) {
                        report << "- **Missing Tests:** " << missing_tests << " of "
                               << m.source_test_function_count << " `#[test]` functions"
                               << " have no Kotlin counterpart\n";
                    }
                }
                if (m.todo_count > 0) report << "- **TODOs:** " << m.todo_count << "\n";
                report << "- **Action:** ";
                if (m.similarity < 0.60) report << "Deep review - likely missing major functionality\n";
                else if (m.similarity < 0.75) report << "Review and complete missing sections\n";
                else report << "Minor refinements needed\n";
                report << "\n";
            }
        }
        if (p1_count == 0) {
            report << "No incomplete high-dependency files detected.\n\n";
        }
        
	        report << "## Priority 2: Port Missing High-Value Files\n\n";
	        report << "Critical missing files (>10 dependencies):\n\n";
	        int p2_count = 0;
	        for (const auto* sf : missing) {
	            if (sf->dependent_count >= 10) {
	                p2_count++;
	                report << p2_count << ". **" << sf->qualified_name << "** (" 
	                       << sf->dependent_count << " deps)\n";
	                report << "   - Path: `" << sf->relative_path << "`\n";
	                report << "   - Essential for " << sf->dependent_count << " other files\n\n";
	            }
	        }
	        if (p2_count == 0) {
	            report << "No missing high-value files detected.\n\n";
	        }

        report << "## Detailed Work Items\n\n";
        report << "Every matched file is listed below with function and type symbol parity.\n\n";
        int detail_block_rank = 1;
        for (const auto& m : ranked) {
            write_match_detail_block(report, m, detail_block_rank++);
        }
        
        report << "## Success Criteria\n\n";
        report << "For each file to be considered \"complete\":\n";
        report << "- **Similarity ≥ 0.85** (Excellent threshold)\n";
        report << "- All public APIs ported\n";
        report << "- All tests ported\n";
        report << "- Documentation ported\n";
        report << "- port-lint header present\n\n";
        
        if (g_ast_config.agent_swarm) {
            report << "## Next Commands\n\n";
            report << "```bash\n";
            report << "# Initialize task queue for systematic porting\n";
            report << "cd tools/ast_distance\n";
            report << "./ast_distance --init-tasks ../../" << source_display_path
                   << " " << source.language << " ../../" << target_display_path
                   << " " << target.language << " tasks.json ../../AGENTS.md\n\n";
            report << "# Get next high-priority task\n";
            report << "./ast_distance --assign tasks.json <agent-id>\n";
            report << "```\n";
        }

        write_reexport_section(report);
        
        std::cout << "✅ Generated: NEXT_ACTIONS.md\n";
    }

    // 4. Generate port_lint_proposed_changes.md
    {
        write_port_lint_proposed_changes_file(
            provenance_proposals,
            source_display_path,
            target_display_path);
        std::cout << "✅ Generated: port_lint_proposed_changes.md\n";
    }
    
    std::cout << "\n📁 All reports generated successfully!\n";
}

void cmd_deep(const std::string& src_dir, const std::string& src_lang,
              const std::string& tgt_dir, const std::string& tgt_lang) {
    std::cout << "=== Deep Analysis: " << src_dir << " (" << src_lang << ") -> "
              << tgt_dir << " (" << tgt_lang << ") ===\n\n";

    // Scan both codebases
    std::cout << "Scanning source codebase (" << src_lang << ")...\n";
    Codebase source(src_dir, src_lang);
    source.scan();
    source.extract_imports();
    source.build_dependency_graph();
    source.print_summary();

    std::cout << "\nScanning target codebase (" << tgt_lang << ")...\n";
    Codebase target(tgt_dir, tgt_lang);
    target.scan();
    target.extract_imports();
    target.build_dependency_graph();
    target.extract_porting_data();  // Extract TODOs, lint, line counts
    target.print_summary();

    // Compare
    std::cout << "\nComparing codebases...\n";
    CodebaseComparator comp(source, target);
    comp.find_matches();

    std::cout << "Computing AST similarities...\n";
    comp.compute_similarities();

    auto ranked_all = comp.ranked_for_porting();
    std::vector<CodebaseComparator::Match> ranked;
    std::vector<CodebaseComparator::Match> ranked_reexports;
    ranked.reserve(ranked_all.size());
    for (auto& m : ranked_all) {
        if (g_reexport_config.matches(m.source_path)) {
            ranked_reexports.push_back(std::move(m));
        } else {
            ranked.push_back(std::move(m));
        }
    }

    comp.print_report();

    // Porting quality summary
    std::cout << "\n=== Porting Quality Summary ===\n\n";

    int total_todos = 0;
    int total_lint = 0;
    int stub_count = 0;
    int exact_header_matched = 0;
    int fallback_header_matched = 0;

    for (const auto& m : comp.matches) {
        total_todos += m.todo_count;
        total_lint += m.lint_count;
        if (m.is_stub) stub_count++;
        if (m.matched_by_header && m.matched_by_normalized_provenance) {
            fallback_header_matched++;
        } else if (m.matched_by_header) {
            exact_header_matched++;
        }
    }

    std::cout << "Matched by exact header:          " << exact_header_matched
              << " / " << comp.matches.size() << "\n";
    std::cout << "Matched by provenance fallback:   " << fallback_header_matched
              << " / " << comp.matches.size() << "\n";
    std::cout << "Matched by name:                  "
              << (comp.matches.size() - exact_header_matched - fallback_header_matched)
              << " / " << comp.matches.size() << "\n";
	    std::cout << "Total TODOs in target: " << total_todos << "\n";
	    std::cout << "Total lint errors:    " << total_lint << "\n";
	    std::cout << "Stub files:           " << stub_count << "\n";

		    int incomplete = 0;
		    int func_gap_files = 0;
		    int total_func_deficit = 0;
		    int total_type_deficit = 0;
		    int test_gap_files = 0;
		    int total_test_deficit = 0;
		    for (const auto& m : ranked) {
	        if (m.similarity < 0.6) incomplete++;
	        if (m.function_deficit() > 0) {
	            func_gap_files++;
	            total_func_deficit += m.function_deficit();
	        }
	        total_type_deficit += m.type_deficit();
	        int missing_tests = std::max(0,
	            m.source_test_function_count - m.matched_test_function_count);
	        if (missing_tests > 0) {
	            test_gap_files++;
	            total_test_deficit += missing_tests;
	        }
	    }

	    int total_src_doc_lines = 0;
	    int total_tgt_doc_lines = 0;
	    for (const auto& m : comp.matches) {
	        total_src_doc_lines += m.source_doc_lines;
	        total_tgt_doc_lines += m.target_doc_lines;
	    }
	    const float kDocCoverageWarnPct = 85.0f;
	    float doc_coverage_pct = 0.0f;
	    if (total_src_doc_lines > 0) {
	        doc_coverage_pct = 100.0f * static_cast<float>(total_tgt_doc_lines) /
	                           static_cast<float>(total_src_doc_lines);
	    }
	    bool docs_missing = (total_src_doc_lines > 0 && doc_coverage_pct < kDocCoverageWarnPct);

	    std::cout << "\n=== Big Picture ===\n\n";
	    std::cout << "- Missing files: " << comp.unmatched_source.size() << "\n";
	    std::cout << "- Incomplete ports (similarity < 60%): " << incomplete << "\n";
	    std::cout << "- Stub files: " << stub_count << "\n";
	    std::cout << "- Files missing functions: " << func_gap_files
	              << " (total deficit: " << total_func_deficit << " functions)\n";
	    std::cout << "- Type definitions missing: " << total_type_deficit << "\n";
	    std::cout << "- Files missing tests: " << test_gap_files
	              << " (total deficit: " << total_test_deficit
	              << " unported `#[test]` functions)\n";
	    if (total_src_doc_lines > 0) {
	        int pct = static_cast<int>(doc_coverage_pct + 0.5f);
	        std::cout << "- Documentation line amount: " << total_tgt_doc_lines << " / "
	                  << total_src_doc_lines << " lines (" << pct << "%)\n";
		    } else {
		        std::cout << "- Documentation line amount: N/A (source has no docs)\n";
		    }
		    std::cout << "\nPrimary focus: ";
		    if (!comp.unmatched_source.empty()) {
		        std::cout << "create missing files (highest deps first)\n";
		    } else if (stub_count > 0) {
		        std::cout << "replace stub files with real implementations\n";
		    } else if (func_gap_files > 0 || test_gap_files > 0) {
		        std::cout << "port missing functions/tests to reach per-file parity"
		                  << " (" << total_func_deficit << " functions, "
		                  << total_test_deficit << " tests)\n";
		    } else if (total_type_deficit > 0) {
		        std::cout << "port missing type definitions\n";
		    } else if (incomplete > 0) {
		        std::cout << "improve incomplete ports (similarity < 60%)\n";
		    } else if (docs_missing) {
		        std::cout << "port missing documentation\n";
		    } else {
		        std::cout << "no major gaps detected\n";
		    }

			    // Show files with issues
			    std::cout << "\n=== Files with Issues ===\n\n";
		    std::cout << std::setw(30) << std::left << "File"
		              << std::setw(11) << "Similarity"
		              << std::setw(11) << "LineRatio"
		              << std::setw(14) << "FunctionParity"
		              << std::setw(10) << "Tests"
		              << std::setw(6) << "TODOs"
		              << std::setw(6) << "Lint"
		              << "Status\n";
		    std::cout << std::string(100, '-') << "\n";

	    for (const auto& m : ranked) {
	        bool func_gap = (m.function_deficit() > 0);
	        bool type_gap = (m.type_deficit() > 0);
	        int missing_tests = std::max(0,
	            m.source_test_function_count - m.matched_test_function_count);
	        bool test_gap = (missing_tests > 0);
	        if (m.todo_count == 0 && m.lint_count == 0 && !m.is_stub
	            && !func_gap && !type_gap && !test_gap && m.similarity >= 0.6) {
	            continue;  // Skip files without issues
	        }

	        std::string status;
	        if (m.is_stub) status = "STUB";
	        else if (m.similarity < 0.4) status = "LOW_SIM";
	        else if (func_gap) status = "MISSING_FUNCS";
	        else if (type_gap) status = "MISSING_TYPES";
	        else if (test_gap) status = "MISSING_TESTS";
	        else if (m.lint_count > 0) status = "LINT";
	        else if (m.todo_count > 0) status = "TODO";

	        float ratio = 0.0f;
	        if (m.source_lines > 0) {
	            ratio = static_cast<float>(m.target_lines) / static_cast<float>(m.source_lines);
	        }

	        std::string funcs = "-";
	        if (m.source_function_count > 0) {
	            funcs = std::to_string(m.matched_function_count) + "/" +
	                    std::to_string(m.source_function_count);
	        }

	        std::string tests = "-";
	        if (m.source_test_function_count > 0) {
	            tests = std::to_string(m.matched_test_function_count) + "/" +
	                    std::to_string(m.source_test_function_count);
	        }

                std::string target_display = match_target_label_for_report(m);
		        std::cout << std::setw(30) << std::left << target_display.substr(0, 28)
		                  << std::setw(11) << std::fixed << std::setprecision(2) << m.similarity
		                  << std::setw(11) << std::fixed << std::setprecision(2) << ratio
		                  << std::setw(14) << funcs
		                  << std::setw(10) << tests
		                  << std::setw(6) << m.todo_count
		                  << std::setw(6) << m.lint_count
		                  << status << "\n";
	        if (!m.missing_functions.empty()) {
	            std::cout << "  missing functions: " << markdown_symbol_list(m.missing_functions) << "\n";
	        }
	        if (!m.missing_types.empty()) {
	            std::cout << "  missing types: " << markdown_symbol_list(m.missing_types) << "\n";
	        }
	    }

	    // Porting recommendations
	    std::cout << "\n=== Porting Recommendations ===\n\n";

	    std::cout << "Incomplete ports (similarity < 60%): " << incomplete << "\n";
	    std::cout << "Missing files: " << comp.unmatched_source.size() << "\n\n";

	    if (incomplete > 0) {
	        std::cout << "Incomplete ports to complete:\n";
	        for (const auto& m : ranked) {
	            if (m.similarity < 0.6) {
	                std::string funcs = "-";
	                if (m.source_function_count > 0) {
	                    funcs = std::to_string(m.matched_function_count) + "/" +
	                            std::to_string(m.source_function_count);
	                }
		                std::cout << "  " << std::setw(30) << std::left << m.source_qualified
		                          << " similarity=" << std::fixed << std::setprecision(2) << m.similarity
		                          << " function_parity=" << funcs
		                          << " dependents=" << m.source_dependents;
	                if (m.is_stub) std::cout << " [STUB]";
	                if (m.todo_count > 0) std::cout << " [" << m.todo_count << " TODOs]";
	                std::cout << "\n";
	                if (!m.missing_functions.empty()) {
	                    std::cout << "    missing functions: "
	                              << markdown_symbol_list(m.missing_functions) << "\n";
	                }
	                if (!m.missing_types.empty()) {
	                    std::cout << "    missing types: "
	                              << markdown_symbol_list(m.missing_types) << "\n";
	                }
	            }
	        }
	    }

	    // Prepare missing files vector for report generation
	    std::vector<const SourceFile*> missing;
	    std::vector<const SourceFile*> missing_reexports;
	    if (!comp.unmatched_source.empty()) {
	        std::cout << "\n=== Missing Files (by Dependents) ===\n\n";
	        // Sort unmatched by dependents
	        for (const auto& path : comp.unmatched_source) {
	            const SourceFile* sf = &source.files.at(path);
	            if (g_reexport_config.matches(sf->relative_path)) {
	                missing_reexports.push_back(sf);
	            } else {
	                missing.push_back(sf);
	            }
	        }
	        auto by_dependents = [](const SourceFile* a, const SourceFile* b) {
	            return a->dependent_count > b->dependent_count;
	        };
	        std::sort(missing.begin(), missing.end(), by_dependents);
	        std::sort(missing_reexports.begin(), missing_reexports.end(), by_dependents);

		        std::cout << std::setw(30) << std::left << "Source File"
		                  << std::setw(38) << "Expected Target"
		                  << std::setw(11) << "Dependents"
		                  << "Path\n";
		        std::cout << std::string(119, '-') << "\n";

	        for (const auto* sf : missing) {
		            std::string expected_target =
		                expected_target_qualified_name_for_source(*sf, target.language);
		            std::cout << std::setw(30) << std::left << sf->qualified_name.substr(0, 28)
		                      << std::setw(38) << expected_target.substr(0, 36)
		                      << std::setw(11) << sf->dependent_count
		                      << sf->relative_path << "\n";
		        }
	        if (!missing_reexports.empty()) {
	            std::cout << "\n=== Reexport / Wiring Modules (consult, don't transliterate) ===\n\n";
	            for (const auto* sf : missing_reexports) {
	                std::string expected_target =
	                    expected_target_qualified_name_for_source(*sf, target.language);
	                std::cout << std::setw(30) << std::left << sf->qualified_name.substr(0, 28)
	                          << std::setw(38) << expected_target.substr(0, 36)
	                          << std::setw(11) << sf->dependent_count
	                          << sf->relative_path << "\n";
	            }
	        }
		    }

	    // Documentation gaps section
	    std::cout << "\n=== Documentation Gaps ===\n\n";

	    // Collect files with doc gaps, sorted by gap severity
	    std::vector<std::pair<float, const CodebaseComparator::Match*>> doc_gaps;
    for (const auto& m : comp.matches) {
        float gap = m.doc_gap_ratio();
        if (gap > 0.2f && m.source_doc_lines > 5) {  // >20% gap and source has meaningful docs
            doc_gaps.emplace_back(gap, &m);
        }
    }

	    std::sort(doc_gaps.begin(), doc_gaps.end(),
	        [](const auto& a, const auto& b) {
	            // Sort by gap ratio * source doc lines (prioritize big gaps in well-documented files)
	            return (a.first * a.second->source_doc_lines) > (b.first * b.second->source_doc_lines);
	        });

	    if (docs_missing) {
	        std::cout << "Documentation correspondence is incomplete; it is reported separately from implementation scoring.\n";
	    }
	    if (total_src_doc_lines > 0) {
	        int pct = static_cast<int>(doc_coverage_pct + 0.5f);
	        std::cout << "Documentation line amount: " << total_tgt_doc_lines << " / "
	                  << total_src_doc_lines << " lines (" << pct << "%)\n";
	    } else {
	        std::cout << "Documentation line amount: N/A (source has no docs)\n";
	    }
	    std::cout << "Files with >20% doc gap: " << doc_gaps.size() << "\n\n";

	    if (doc_gaps.empty()) {
	        std::cout << "No significant documentation line-count gaps found; text/reference correspondence is separate.\n";
	    } else {
	        std::cout << std::setw(30) << std::left << "File"
                  << std::setw(12) << "Src Docs"
                  << std::setw(12) << "Tgt Docs"
                  << std::setw(10) << "Gap %"
                  << std::setw(10) << "DocSim"
                  << std::setw(10) << "DocAmt"
                  << std::setw(10) << "DocEq"
                  << "\n";
        std::cout << std::string(94, '-') << "\n";

        for (const auto& [gap, m] : doc_gaps) {
	            std::string gap_str = std::to_string(static_cast<int>(gap * 100)) + "%";
	            std::cout << std::setw(30) << std::left << m->source_qualified.substr(0, 28)
	                      << std::setw(12) << m->source_doc_lines
	                      << std::setw(12) << m->target_doc_lines
		                      << std::setw(10) << gap_str
		                      << std::setw(10) << std::fixed << std::setprecision(2) << m->doc_similarity
		                      << std::setw(10) << std::fixed << std::setprecision(2) << m->doc_coverage
		                      << std::setw(10) << std::fixed << std::setprecision(2) << m->doc_weighted
		                      << "\n";
	        }
	    }

    // Generate markdown reports
    generate_reports(source, target, comp, ranked, missing, doc_gaps,
                     incomplete, total_src_doc_lines, total_tgt_doc_lines,
                     ranked_reexports, missing_reexports);
    std::ostringstream inventory;
    print_deep_inventory(source, target, comp, inventory);
    std::cout << inventory.str();
    std::ofstream symbol_report("deep_symbol_inventory.txt");
    symbol_report << inventory.str();
    if (!symbol_report) throw std::runtime_error("Cannot write deep_symbol_inventory.txt");

    if (src_lang == "kotlin" && tgt_lang == "cpp") {
        std::ofstream evidence("deep_transliteration_evidence.txt");
        if (!evidence) throw std::runtime_error("Cannot write deep_transliteration_evidence.txt");
        std::cout << "\n=== AST Emitted Transliteration Distance (bounded Kotlin -> C++ rules) ===\n";
        std::cout << "Source\tTarget\tScore\tOrderedLogic\tRuleCoverage\tDocs\tFallbacks\tDocMisses\n";
        for (const auto& match : comp.matches) {
            const auto& source_file = source.files.at(match.source_path);
            const auto& target_file = target.files.at(match.target_path);
            auto target_paths = target_file.paths;
            target_paths.insert(target_paths.end(), match.additional_target_paths.begin(), match.additional_target_paths.end());
            auto distance = transliteration_distance(CodebaseComparator::read_files_to_string(source_file.paths), Language::KOTLIN,
                CodebaseComparator::read_files_to_string(target_paths), Language::CPP);
            std::cout << source_file.relative_path << '\t' << target_file.relative_path << '\t'
                << std::fixed << std::setprecision(6) << distance.score << '\t' << distance.normalized_logic << '\t'
                << distance.translation.rule_coverage << '\t' << distance.documentation_parity << '\t'
                << distance.translation.rule_misses << '\t' << distance.translation.documentation_misses << '\n';
            for (const auto& diagnostic : distance.translation.diagnostics)
                std::cout << "  " << source_file.relative_path << ':' << diagnostic << '\n';
            evidence << "=== " << source_file.relative_path << " -> " << target_file.relative_path << " ===\n";
            evidence << "Locations below refer to these concatenated physical files, in order:\nSource:";
            for (const auto& path : source_file.paths) evidence << ' ' << path;
            evidence << "\nTarget:";
            for (const auto& path : target_paths) evidence << ' ' << path;
            evidence << '\n';
            print_transliteration_distance(distance, evidence);
            std::ostringstream suspension;
            print_suspension_review(source_file.paths, target_paths, suspension);
            std::cout << suspension.str();
            evidence << suspension.str();
        }
        if (!evidence) throw std::runtime_error("Cannot write deep_transliteration_evidence.txt");
        std::cout << "Full metrics, source/emitted byte maps, ordered logic differences, missing/extra callables,\n"
                     "and emitted buffers saved to deep_transliteration_evidence.txt. Unsupported rules are visible fallbacks.\n";
    }
}

void cmd_missing(const std::string& src_dir, const std::string& src_lang,
                 const std::string& tgt_dir, const std::string& tgt_lang) {
    Codebase source(src_dir, src_lang);
    source.scan();
    source.extract_imports();
    source.build_dependency_graph();

    Codebase target(tgt_dir, tgt_lang);
    target.scan();

    CodebaseComparator comp(source, target);
    comp.find_matches();
    comp.compute_similarities();

    std::cout << "=== Missing from " << tgt_lang << " (ranked by dependents) ===\n\n";
    std::cout << std::setw(40) << std::left << "Source File"
              << std::setw(10) << "Deps"
              << "Path\n";
    std::cout << std::string(80, '-') << "\n";

    // Sort unmatched by dependents
    std::vector<const SourceFile*> missing;
    std::vector<const SourceFile*> missing_reexports;
    for (const auto& path : comp.unmatched_source) {
        const SourceFile* sf = &source.files.at(path);
        if (g_reexport_config.matches(sf->relative_path)) {
            missing_reexports.push_back(sf);
        } else {
            missing.push_back(sf);
        }
    }
    auto by_dependents = [](const SourceFile* a, const SourceFile* b) {
        return a->dependent_count > b->dependent_count;
    };
    std::sort(missing.begin(), missing.end(), by_dependents);
    std::sort(missing_reexports.begin(), missing_reexports.end(), by_dependents);

    for (const auto* sf : missing) {
        std::cout << std::setw(40) << std::left << sf->qualified_name.substr(0, 38)
                  << std::setw(10) << sf->dependent_count
                  << sf->relative_path << "\n";
    }
    if (!missing_reexports.empty()) {
        std::cout << "\n=== Reexport / Wiring Modules (consult, don't transliterate) ===\n\n";
        std::cout << std::setw(40) << std::left << "Source File"
                  << std::setw(10) << "Deps"
                  << "Path\n";
        std::cout << std::string(80, '-') << "\n";
        for (const auto* sf : missing_reexports) {
            std::cout << std::setw(40) << std::left << sf->qualified_name.substr(0, 38)
                      << std::setw(10) << sf->dependent_count
                      << sf->relative_path << "\n";
        }
    }

    // Matched-but-stub files are not missing in the structural sense but are
    // incomplete ports. Show them separately so they don't hide in the matched set.
    std::vector<const CodebaseComparator::Match*> stub_matches;
    for (const auto& m : comp.matches) {
        if (m.is_stub) stub_matches.push_back(&m);
    }
    if (!stub_matches.empty()) {
        std::sort(stub_matches.begin(), stub_matches.end(),
            [&](const CodebaseComparator::Match* a, const CodebaseComparator::Match* b) {
                return source.files.at(a->source_path).dependent_count >
                       source.files.at(b->source_path).dependent_count;
            });
        std::cout << "\n=== Matched but STUB (port exists, needs real implementation) ===\n\n";
        std::cout << std::setw(40) << std::left << "Source File"
                  << std::setw(10) << "Deps"
                  << "Target\n";
        std::cout << std::string(80, '-') << "\n";
        for (const auto* m : stub_matches) {
            const auto& sf = source.files.at(m->source_path);
            std::cout << std::setw(40) << std::left << sf.qualified_name.substr(0, 38)
                      << std::setw(10) << sf.dependent_count
                      << m->target_path << "\n";
        }
        std::cout << "\nTotal stubs: " << stub_matches.size() << "\n";
    }

    std::cout << "\nTotal missing: " << missing.size() << "\n";
}

void cmd_todos(const std::string& directory, bool verbose = true) {
    std::cout << "Scanning for TODOs in: " << directory << "\n\n";

    auto file_stats = PortingAnalyzer::analyze_directory(directory);

    // Collect all TODOs
    std::vector<TodoItem> all_todos;
    for (const auto& fs : file_stats) {
        all_todos.insert(all_todos.end(), fs.todos.begin(), fs.todos.end());
    }

    PortingAnalyzer::print_todo_report(all_todos, verbose);
}

void cmd_lint(const std::string& directory) {
    std::cout << "Running lint checks on: " << directory << "\n\n";

    auto file_stats = PortingAnalyzer::analyze_directory(directory);

    // Collect all lint errors
    std::vector<LintError> all_errors;
    for (const auto& fs : file_stats) {
        all_errors.insert(all_errors.end(), fs.lint_errors.begin(), fs.lint_errors.end());
    }

    PortingAnalyzer::print_lint_report(all_errors);

    if (!all_errors.empty()) {
        std::cout << "\nLint check failed with " << all_errors.size() << " error(s).\n";
    }
}

void cmd_stats(const std::string& directory) {
    std::cout << "=== File Statistics: " << directory << " ===\n\n";

    auto file_stats = PortingAnalyzer::analyze_directory(directory);

    // Sort by line count descending
    std::sort(file_stats.begin(), file_stats.end(),
        [](const FileStats& a, const FileStats& b) {
            return a.line_count > b.line_count;
        });

    int total_lines = 0;
    int total_code = 0;
    int total_todos = 0;
    int total_lint = 0;
    int stub_count = 0;

    std::cout << std::setw(40) << std::left << "File"
              << std::setw(8) << "Lines"
              << std::setw(8) << "Code"
              << std::setw(6) << "TODOs"
              << std::setw(6) << "Lint"
              << "Status\n";
    std::cout << std::string(80, '-') << "\n";

    for (const auto& fs : file_stats) {
        std::string status;
        if (fs.is_stub) {
            status = "STUB";
            stub_count++;
        } else if (!fs.lint_errors.empty()) {
            status = "LINT_ERR";
        } else if (!fs.todos.empty()) {
            status = "HAS_TODO";
        } else {
            status = "OK";
        }

        std::string name = fs.relative_path;
        if (name.length() > 38) {
            name = "..." + name.substr(name.length() - 35);
        }

        std::cout << std::setw(40) << std::left << name
                  << std::setw(8) << fs.line_count
                  << std::setw(8) << fs.code_lines
                  << std::setw(6) << fs.todos.size()
                  << std::setw(6) << fs.lint_errors.size()
                  << status << "\n";

        total_lines += fs.line_count;
        total_code += fs.code_lines;
        total_todos += fs.todos.size();
        total_lint += fs.lint_errors.size();
    }

    std::cout << std::string(80, '-') << "\n";
    std::cout << std::setw(40) << std::left << "TOTAL"
              << std::setw(8) << total_lines
              << std::setw(8) << total_code
              << std::setw(6) << total_todos
              << std::setw(6) << total_lint
              << "\n\n";

    std::cout << "Summary:\n";
    std::cout << "  Files:      " << file_stats.size() << "\n";
    std::cout << "  Stubs:      " << stub_count << "\n";
    std::cout << "  TODOs:      " << total_todos << "\n";
    std::cout << "  Lint errors: " << total_lint << "\n";
}

static void verify_comparison_identity(const std::string& source_path, Language source_language,
                                       const std::string& target_path, Language target_language) {
    const bool kotlin_cpp = (source_language == Language::KOTLIN && target_language == Language::CPP) ||
        (source_language == Language::CPP && target_language == Language::KOTLIN);
    if (!kotlin_cpp) return;
    // File identity follows upstream Kotlin -> C++ provenance even when the
    // caller requests the body comparison in the reverse language direction.
    const std::string& upstream_path = source_language == Language::KOTLIN ? source_path : target_path;
    const std::string& port_path = target_language == Language::CPP ? target_path : source_path;
    Codebase source(upstream_path, "kotlin");
    Codebase target(port_path, "cpp");
    source.scan(); source.extract_imports();
    target.scan(); target.extract_imports(); target.extract_porting_data();
    if (source.files.size() != 1 || target.files.size() != 1)
        throw std::runtime_error("Explicit file comparison requires one source/target compilation unit");
    const auto& left = source.files.begin()->second;
    const auto& right = target.files.begin()->second;
    if (!right.identity_conflicts.empty())
        throw std::runtime_error("Identity conflict: " + right.identity_conflicts.front());
    if (!CodebaseComparator::namespace_identity_matches(left.package, right.package))
        throw std::runtime_error("Namespace/package mismatch: " + upstream_path + " [" + left.package.path +
            "] vs " + port_path + " [" + right.package.path + "]");
    if (!right.transliterated_from.empty()) {
        CodebaseComparator comparator(source, target);
        if (comparator.identity_header_match(left, right).score == 0)
            throw std::runtime_error("Provenance mismatch: target identifies " + right.transliterated_from +
                ", not " + upstream_path);
    }
}

static void print_emission_evidence(const TransliterationOutput& output, std::ostream& stream) {
    stream << "Rule hits: " << output.rule_hits << " | misses: " << output.rule_misses
           << " | documentation misses: " << output.documentation_misses
           << " | supported code-byte coverage: " << output.rule_coverage << '\n';
    stream << "Source line\tSource bytes\tEmitted bytes\tNode\tSupported\n";
    for (const auto& span : output.spans)
        stream << span.source_line << '\t' << span.source_start << ':' << span.source_end << '\t'
               << span.target_start << ':' << span.target_end << '\t' << span.node_type << '\t'
               << (span.supported ? "yes" : "fallback") << '\n';
    for (const auto& diagnostic : output.diagnostics) stream << "Diagnostic: " << diagnostic << '\n';
}
static void print_transliteration_distance(const TransliterationDistance& report, std::ostream& stream) {
stream << std::fixed << std::setprecision(6)
    << "score_method: positional exact-token cosine including code and translated comment words after in-memory Kotlin-to-C++ emission\n"
    << "translated_text_cosine: " << report.translated_text_cosine << '\n'
    << "translated_ast_cosine: " << report.translated_ast_cosine << '\n'
    << "function_name_parity: " << report.symbol_parity << '\n'
    << "normalized_logic: " << report.normalized_logic << '\n'
    << "documentation_correspondence: " << report.documentation_parity << '\n'
    << "span_rule_coverage: " << report.translation.rule_coverage << '\n'
    << "unsupported_fraction (diagnostic only): " << report.fallback_penalty << '\n'
    << "score: " << report.score << '\n'
    << "Score evidence: " << (report.translation.rule_misses || report.translation.documentation_misses || report.translated_parse_errors || report.target_parse_errors ? "provisional (unsupported rules or parse errors)" : "supported") << '\n'
    << "Generated parse errors: " << (report.translated_parse_errors ? "yes (provisional)" : "no") << '\n'
    << "Target parse errors: " << (report.target_parse_errors ? "yes (provisional)" : "no") << '\n';
print_emission_evidence(report.translation, stream);
for (const auto& function : report.functions) {
    stream << "Function " << function.source_name << ':' << function.source_line << " -> "
        << function.target_name << ':' << function.target_line << " | normalized logic: " << function.normalized_logic << '\n';
    if (function.emitted_tokens == function.target_tokens) stream << "  Ordered logic sequences are equal\n";
    else {
        stream << "  Emitted logic:";
        for (const auto& token : function.emitted_tokens) stream << ' ' << token;
        stream << "\n  Target logic:";
        for (const auto& token : function.target_tokens) stream << ' ' << token;
        stream << '\n';
    }
}
for (const auto& function : report.missing_functions) stream << "Missing source function: " << function << '\n';
for (const auto& function : report.extra_functions) stream << "Extra target function: " << function << '\n';
stream << "=== Emitted target buffer ===\n" << report.translation.buffer;
}
static std::string read_emission_input(const std::string& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Cannot read transliteration input: " + path);
    return std::string(std::istreambuf_iterator<char>(file), {});
}
int main(int argc, char* argv[]) {
    setvbuf(stdout, nullptr, _IOLBF, 0);

    std::vector<std::string> rest;
    rest.reserve(static_cast<size_t>(argc));
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        rest.push_back(arg);
    }

    g_ast_config = load_ast_config();

    {
        std::string cfg_path = default_reexport_config_path();
        if (load_reexport_config(cfg_path, g_reexport_config)) {
            if (!g_reexport_config.patterns.empty()) {
                std::cerr << "Info: loaded " << g_reexport_config.patterns.size()
                          << " reexport-module pattern"
                          << (g_reexport_config.patterns.size() == 1 ? "" : "s")
                          << " from " << cfg_path << ".\n";
            }
        }
    }

    std::vector<char*> argv_rebased;
    argv_rebased.reserve(rest.size() + 1);
    argv_rebased.push_back(argv[0]);
    for (auto& s : rest) {
        argv_rebased.push_back(const_cast<char*>(s.c_str()));
    }
    argc = static_cast<int>(argv_rebased.size());
    argv = argv_rebased.data();

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string mode = argv[1];

    try {
        if (mode == "--scan" && argc >= 4) {
            cmd_scan(argv[2], argv[3]);

        } else if (mode == "--deps" && argc >= 4) {
            cmd_deps(argv[2], argv[3]);

        } else if (mode == "--rank") {
            if (g_ast_config.unlock_parameters && argc >= 6) {
                cmd_rank(argv[2], argv[3], argv[4], argv[5]);
            } else {
                std::cerr << "Error: --rank is disabled because it hides parts of the full report.\n"
                          << "Use --deep <src_dir> <src_lang> <tgt_dir> <tgt_lang> instead.\n";
                return 2;
            }

        } else if (mode == "--deep" && argc >= 6) {
            cmd_deep(argv[2], argv[3], argv[4], argv[5]);

        } else if (mode == "--numpy-mlx" && argc >= 4) {
            cmd_numpy_mlx(argv[2], argv[3]);

        } else if (mode == "--emberlint" && argc >= 3) {
            cmd_emberlint(argv[2]);

        } else if (mode == "--missing") {
            if (g_ast_config.unlock_parameters && argc >= 6) {
                cmd_missing(argv[2], argv[3], argv[4], argv[5]);
            } else {
                std::cerr << "Error: --missing is disabled because it hides parts of the full report.\n"
                          << "Use --deep <src_dir> <src_lang> <tgt_dir> <tgt_lang> instead.\n";
                return 2;
            }

        } else if (mode == "--todos" && argc >= 3) {
            bool verbose = true;
            if (argc >= 4 && std::string(argv[3]) == "--summary") {
                if (g_ast_config.unlock_parameters) {
                    verbose = false;
                } else {
                    std::cerr << "Error: --summary is disabled. Use full detailed output.\n";
                    return 2;
                }
            }
            cmd_todos(argv[2], verbose);

        } else if (mode == "--lint" && argc >= 3) {
            cmd_lint(argv[2]);

        } else if (mode == "--stats" && argc >= 3) {
            cmd_stats(argv[2]);

        } else if ((mode == "--symbols" || mode == "--symbols-duplicates" ||
                    mode == "--symbols-stubs") && argc >= 4) {
            SymbolAnalysisOptions options;
            options.duplicates = mode == "--symbols-duplicates";
            options.stubs = mode == "--symbols-stubs";
            for (int i = 4; i < argc; ++i) {
                if (std::string(argv[i]) == "--json") options.json = true;
                else throw std::runtime_error("Unknown symbol-report option: " + std::string(argv[i]));
            }
            cmd_symbols(argv[2], argv[3], options);

        } else if (mode == "--symbols-symbol" && argc >= 5) {
            SymbolAnalysisOptions options;
            options.symbol = argv[4];
            for (int i = 5; i < argc; ++i) {
                if (std::string(argv[i]) == "--json") options.json = true;
                else throw std::runtime_error("Unknown symbol-lookup option: " + std::string(argv[i]));
            }
            cmd_symbol_lookup(argv[2], argv[3], options);

        } else if (mode == "--symbol-parity" && argc >= 4) {
            SymbolParityOptions options;
            std::string src_root = argv[2];
            std::string tgt_root = argv[3];
            int arg_start = 4;

            // Optional: --symbol-parity <src_root> <src_lang> <tgt_root> <tgt_lang>
            if (argc >= 6 && argv[4][0] != '-') {
                options.source_lang = parse_language(argv[3]);
                tgt_root = argv[4];
                options.target_lang = parse_language(argv[5]);
                arg_start = 6;
            } else {
                // Auto-detect based on file extensions in roots or defaults
                options.source_lang = Language::RUST;
                options.target_lang = Language::KOTLIN;
            }

            for (int i = arg_start; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg == "--missing-only" || arg == "--missing") {
                    if (g_ast_config.unlock_parameters) {
                        options.missing_only = true;
                    } else {
                        std::cerr << "Error: --missing-only is disabled. Full parity output is required.\n";
                        return 2;
                    }
                } else if (arg == "--include-stubs") {
                    if (g_ast_config.unlock_parameters) {
                        options.include_stubs = true;
                    } else {
                        std::cerr << "Error: --include-stubs is disabled. Full parity output is required.\n";
                        return 2;
                    }
                } else if (arg == "--kind" && i + 1 < argc) {
                    options.filter_kind = argv[++i];
                } else if (arg == "--file" && i + 1 < argc) {
                    options.filter_file = argv[++i];
                }
            }
            cmd_symbol_parity(src_root, tgt_root, options);

        } else if (mode == "--import-map" && argc >= 3) {
            ImportMapOptions options;
            for (int i = 3; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg == "--summary") {
                    if (!g_ast_config.unlock_parameters) {
                        std::cerr << "Error: --summary is disabled. Use full detailed output.\n";
                        return 2;
                    }
                    // No summary-mode field on ImportMapOptions; flag is currently a no-op
                    // when unlocked. Kept here for forward compatibility.
                } else if (arg == "--file" && i + 1 < argc) {
                    options.filter_file = argv[++i];
                } else if (arg == "--min" && i + 1 < argc) {
                    options.min_unresolved = std::stoi(argv[++i]);
                }
            }
            cmd_import_map(argv[2], options);

        } else if (mode == "--compiler-fixup" && argc >= 4) {
            CompilerFixupOptions options;
            for (int i = 4; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg == "--file" && i + 1 < argc) {
                    options.filter_file = argv[++i];
                } else if (arg == "--min" && i + 1 < argc) {
                    options.min_errors = std::stoi(argv[++i]);
                }
            }
            cmd_compiler_fixup(argv[2], argv[3], options);

        } else if (mode == "--dump-node" || mode == "--dump") {
            std::cerr << "Error: --dump and --dump-node are disabled. Use markdown report generation only.\n";
            return 2;

        } else if (mode == "--transliterate") {
            if (argc != 5) throw std::runtime_error("Usage: --transliterate source_file source_lang target_lang");
            auto output = transliterate(read_emission_input(argv[2]), parse_language(argv[3]), parse_language(argv[4]));
            std::cout << output.buffer;
            print_emission_evidence(output, std::cerr);
        } else if (mode == "--translit-distance") {
            if (argc != 6) throw std::runtime_error("Usage: --translit-distance source_file source_lang target_file target_lang");
            auto source_text = read_emission_input(argv[2]);
            auto target_text = read_emission_input(argv[4]);
            verify_comparison_identity(argv[2], parse_language(argv[3]), argv[4], parse_language(argv[5]));
            auto report = transliteration_distance(source_text, parse_language(argv[3]), target_text, parse_language(argv[5]));
            print_transliteration_distance(report, std::cout);
        } else if (mode == "--compare-functions" && argc >= 6) {
            const bool with_companions = argc == 7 && std::string(argv[6]) == "--with-companions";
            if (argc != 6 && !with_companions)
                throw std::runtime_error("Usage: --compare-functions file1 lang1 file2 lang2 [--with-companions]");
            ASTParser parser;
            std::string file1 = argv[2];
            Language lang1 = parse_language(argv[3]);
            std::string file2 = argv[4];
            Language lang2 = parse_language(argv[5]);

            std::cout << "Extracting functions from " << file1 << " (" << language_name(lang1) << ")...\n";
            std::ifstream stream1(file1);
            std::stringstream buffer1;
            buffer1 << stream1.rdbuf();
            std::string file1_text = buffer1.str();
            if (!stream1.is_open()) throw std::runtime_error("Cannot open source: " + file1);
            auto print_extraction_diagnostics = [&](const std::string& path) {
                for (int line : parser.last_fun_interface_lines())
                    std::cerr << "Info: " << path << ':' << line
                        << " parsed fun interface with offset-preserving grammar compatibility adapter\n";
                for (int line : parser.last_cpp_statement_macro_lines())
                    std::cerr << "Info: " << path << ':' << line
                        << " parsed coroutine statement macro with offset-preserving terminator adapter; expansion remains unverified\n";
                if (parser.last_extraction_has_errors())
                    std::cerr << "Warning: " << path << " has parser errors; function inventory may be incomplete\n";
                for (const auto& diagnostic : parser.last_extraction_diagnostics())
                    std::cerr << "  " << path << ':' << diagnostic << '\n';
            };
            auto funcs1 = parser.extract_function_infos(file1_text, lang1);
            auto source_unmapped = parser.get_unmapped_node_types();
            parser.clear_unmapped();
            print_extraction_diagnostics(file1);
            auto reexport_hints = rust_mod_reexport_hints(file1, file1_text, lang1, lang2);

            std::cout << "Found " << funcs1.size() << " " << language_name(lang1) << " functions\n";

            std::cout << "\nExtracting functions from " << file2 << " (" << language_name(lang2) << ")...\n";
            std::ifstream stream2(file2);
            std::stringstream buffer2;
            buffer2 << stream2.rdbuf();
            std::string file2_text = buffer2.str();
            if (!stream2.is_open()) throw std::runtime_error("Cannot open target: " + file2);
            verify_comparison_identity(file1, lang1, file2, lang2);
            auto funcs2 = parser.extract_function_infos(file2_text, lang2);
            std::vector<std::string> target_locations(funcs2.size(), file2);
            auto target_unmapped = parser.get_unmapped_node_types();
            print_extraction_diagnostics(file2);
            if (with_companions) {
                if (lang2 != Language::CPP)
                    throw std::runtime_error("--with-companions requires a C++ target");
                Codebase target_unit(file2, "cpp");
                target_unit.scan();
                if (target_unit.files.size() != 1)
                    throw std::runtime_error("Companion comparison requires one logical C++ unit");
                for (const auto& path : target_unit.files.begin()->second.paths) {
                    if (path == file2) continue;
                    std::ifstream companion_stream(path);
                    if (!companion_stream.is_open()) throw std::runtime_error("Cannot open companion: " + path);
                    std::stringstream companion_buffer;
                    companion_buffer << companion_stream.rdbuf();
                    parser.clear_unmapped();
                    auto companion_functions = parser.extract_function_infos(companion_buffer.str(), lang2);
                    print_extraction_diagnostics(path);
                    for (const auto& [kind, count] : parser.get_unmapped_node_types()) target_unmapped[kind] += count;
                    std::cout << "Companion " << path << ": " << companion_functions.size() << " function bodies\n";
                    target_locations.insert(target_locations.end(), companion_functions.size(), path);
                    funcs2.insert(funcs2.end(), std::make_move_iterator(companion_functions.begin()),
                                  std::make_move_iterator(companion_functions.end()));
                }
            }
            auto target_label = [&](int index) {
                const auto& function = funcs2[index];
                return (with_companions ? target_locations[index] + ":" : "") + function.qualified_name +
                    ":" + std::to_string(function.start_line) + " " + function.signature;
            };

            std::cout << "Found " << funcs2.size() << " " << language_name(lang2) << " functions\n";

            auto print_unmapped = [](const char* role, const std::string& path, const auto& types) {
                if (types.empty()) return;
                std::cout << "\nUnmapped parser nodes in " << role << " " << path << ":\n";
                for (const auto& [type, count] : types) std::cout << "  " << type << ": " << count << '\n';
                std::cout << "These constructs are not covered by the normalized term mapping; inspect the source evidence.\n";
            };
            print_unmapped("source", file1, source_unmapped);
            print_unmapped("target", file2, target_unmapped);
            std::vector<std::string> target_zero_reasons;
            if (lang2 == Language::KOTLIN) {
                target_zero_reasons =
                    CodebaseComparator::kotlin_contamination_reasons_for_text(file2_text);
            }

            const bool strict_name_parity =
                (lang1 == lang2) || (lang1 == Language::RUST && lang2 == Language::KOTLIN);

            auto target_lookup_key = [&](const FunctionInfo& info) -> std::string {
                if (strict_name_parity) {
                    return info.name;
                }
                std::string key = IdentifierStats::canonicalize(info.name);
                return key.empty() ? info.name : key;
            };

            auto expected_lookup_key = [&](const FunctionInfo& info) -> std::string {
                if (strict_name_parity) {
                    return expected_target_function_name(info.name, lang1, lang2);
                }
                std::string key = IdentifierStats::canonicalize(info.name);
                return key.empty() ? info.name : key;
            };

            auto guarded_combined_similarity = [&target_zero_reasons](
                                                  const FunctionInfo& source_func,
                                                  const FunctionInfo& target_func) -> float {
                if (!target_zero_reasons.empty()) {
                    return 0.0f;
                }
                // Guardrail: treat stub markers as a failure only when they appear in the
                // target function but not in the source baseline.
                if (target_func.has_stub_markers && !source_func.has_stub_markers) {
                    return 0.0f;
                }
                return ASTSimilarity::function_parameter_body_cosine_similarity(
                    source_func.body_tree.get(),
                    target_func.body_tree.get(),
                    source_func.identifiers,
                    target_func.identifiers);
            };

            auto guarded_ast_cosine = [&target_zero_reasons](
                                         const FunctionInfo& source_func,
                                         const FunctionInfo& target_func) -> float {
                if (!target_zero_reasons.empty()) {
                    return 0.0f;
                }
                if (target_func.has_stub_markers && !source_func.has_stub_markers) {
                    return 0.0f;
                }
                return ASTSimilarity::histogram_cosine_similarity(
                    source_func.body_tree.get(),
                    target_func.body_tree.get());
            };

            auto guarded_identifier_cosine = [&target_zero_reasons](
                                                const FunctionInfo& source_func,
                                                const FunctionInfo& target_func) -> float {
                if (!target_zero_reasons.empty()) {
                    return 0.0f;
                }
                if (target_func.has_stub_markers && !source_func.has_stub_markers) {
                    return 0.0f;
                }
                return source_func.identifiers.canonical_cosine_similarity(target_func.identifiers);
            };

            struct FunctionMatchCandidate {
                float score = 0.0f;
                float ast_cosine = 0.0f;
                float id_cosine = 0.0f;
                float logic_similarity = 0.0f;
                int source_index = 0;
                int target_index = 0;
            };

            std::unordered_map<std::string, std::vector<int>> target_by_name;
            target_by_name.reserve(funcs2.size());
            for (int j = 0; j < static_cast<int>(funcs2.size()); ++j) {
                target_by_name[target_lookup_key(funcs2[j])].push_back(j);
            }

            std::vector<FunctionMatchCandidate> candidates;
            bool owner_fallback_ambiguity = false;
            for (int i = 0; i < static_cast<int>(funcs1.size()); ++i) {
                auto bucket = target_by_name.find(expected_lookup_key(funcs1[i]));
                if (bucket == target_by_name.end()) {
                    continue;
                }
                auto source_owner = callable_owner_key(funcs1[i]);
                bool matching_owner = !source_owner.empty() && std::any_of(
                    bucket->second.begin(), bucket->second.end(), [&](int j) {
                        return callable_owners_compatible(funcs1[i], funcs2[j]) && !callable_owner_key(funcs2[j]).empty();
                    });
                if (!matching_owner && bucket->second.size() > 1) owner_fallback_ambiguity = true;
                for (int j : bucket->second) {
                    if (!callable_owners_compatible(funcs1[i], funcs2[j])) continue;
                    if (matching_owner && callable_owner_key(funcs2[j]).empty()) continue;
                    if (source_owner.empty() != callable_owner_key(funcs2[j]).empty()) owner_fallback_ambiguity = true;
                    candidates.push_back({
                        guarded_combined_similarity(funcs1[i], funcs2[j]),
                        guarded_ast_cosine(funcs1[i], funcs2[j]),
                        guarded_identifier_cosine(funcs1[i], funcs2[j]),
                        normalized_logic_similarity(funcs1[i].body_tree.get(), funcs2[j].body_tree.get()),
                        i,
                        j,
                    });
                }
            }

            std::sort(candidates.begin(), candidates.end(),
                [](const FunctionMatchCandidate& a, const FunctionMatchCandidate& b) {
                    if (a.score != b.score) return a.score > b.score;
                    return std::tie(a.source_index, a.target_index) < std::tie(b.source_index, b.target_index);
                });

            std::vector<bool> source_used(funcs1.size(), false);
            std::vector<bool> target_used(funcs2.size(), false);

            struct FunctionPairReport {
                std::string source_name;
                std::string expected_name;
                std::string target_name;
                int source_lines = 0;
                int target_lines = 0;
                int line_gap = 0;
                float line_ratio = 0.0f;
                int source_nodes = 0;
                int target_nodes = 0;
                float ast_cosine = 0.0f;
                float id_cosine = 0.0f;
                float combined = 0.0f;
                float logic_similarity = 0.0f;
                bool stub_guarded = false;
            };

            std::vector<FunctionPairReport> reports;
            reports.reserve(std::min(funcs1.size(), funcs2.size()));
            float total_combined = 0.0f;
            float total_ast_cosine = 0.0f;
            float total_id_cosine = 0.0f;
            float total_line_balance = 0.0f;
            float total_logic_similarity = 0.0f;

            for (const auto& candidate : candidates) {
                if (source_used[candidate.source_index] || target_used[candidate.target_index]) {
                    continue;
                }
                const auto& source_func = funcs1[candidate.source_index];
                const auto& target_func = funcs2[candidate.target_index];
                source_used[candidate.source_index] = true;
                target_used[candidate.target_index] = true;

                FunctionPairReport report;
                report.source_name = source_func.qualified_name + ":" + std::to_string(source_func.start_line) + " " + source_func.signature;
                report.expected_name = expected_target_function_name(source_func.name, lang1, lang2);
                report.target_name = target_label(candidate.target_index);
                report.source_lines = source_func.line_count;
                report.target_lines = target_func.line_count;
                report.line_gap = target_func.line_count - source_func.line_count;
                report.line_ratio = source_func.line_count > 0
                    ? static_cast<float>(target_func.line_count) / static_cast<float>(source_func.line_count)
                    : 0.0f;
                report.source_nodes = source_func.body_tree ? source_func.body_tree->size() : 0;
                report.target_nodes = target_func.body_tree ? target_func.body_tree->size() : 0;
                report.ast_cosine = candidate.ast_cosine;
                report.id_cosine = candidate.id_cosine;
                report.combined = candidate.score;
                report.logic_similarity = candidate.logic_similarity;
                report.stub_guarded = target_func.has_stub_markers && !source_func.has_stub_markers;
                reports.push_back(report);

                total_combined += report.combined;
                total_ast_cosine += report.ast_cosine;
                total_id_cosine += report.id_cosine;
                total_logic_similarity += report.logic_similarity;
                if (source_func.line_count > 0 && target_func.line_count > 0) {
                    int min_lines = std::min(source_func.line_count, target_func.line_count);
                    int max_lines = std::max(source_func.line_count, target_func.line_count);
                    total_line_balance += static_cast<float>(min_lines) / static_cast<float>(max_lines);
                }
            }

            // Iterator-trait equivalence (Rust impl Iterator <-> Kotlin : Iterator).
            //
            // Kotlin's kotlin.collections.Iterator interface forces a `hasNext():
            // Boolean` method that has no Rust counterpart -- Rust's
            // `Iterator::next() -> Option<T>` folds the "is there a next?"
            // question into the return value. When a Kotlin class is the
            // faithful port of a Rust `impl Iterator for X`, the forced
            // `hasNext` should not count as an unmatched Kotlin-only extra.
            //
            // Laser-focused rule: suppress `hasNext` from unmatched-target
            // ONLY when:
            //   - Rust source contains at least one `impl Iterator for ...`
            //     (or `impl<...> Iterator for ...`)
            //   - Kotlin target declares a class implementing Iterator<...>
            //     or MutableIterator<...>
            //
            // Both sides must show the iterator pattern. A unilateral Kotlin
            // Iterator extension without a Rust counterpart is still a
            // Kotlin-only invention and is reported as before.
            const bool both_sides_have_iterator_pattern = [&]() -> bool {
                if (lang1 != Language::RUST || lang2 != Language::KOTLIN) {
                    return false;
                }
                static const std::regex rust_impl_iter(
                    R"(\bimpl(?:\s*<[^{>]*>)?\s+(?:[A-Za-z0-9_:]+\s*::\s*)?Iterator\b\s+for\b)");
                static const std::regex kotlin_extends_iter(
                    R"(:\s*(?:[A-Za-z_][A-Za-z0-9_.]*\s*\.)?(?:Mutable)?Iterator\s*<)");
                return std::regex_search(file1_text, rust_impl_iter)
                    && std::regex_search(file2_text, kotlin_extends_iter);
            }();

            int suppressed_hasnext = 0;
            if (both_sides_have_iterator_pattern) {
                for (int j = 0; j < static_cast<int>(funcs2.size()); ++j) {
                    if (!target_used[j] && funcs2[j].name == "hasNext") {
                        target_used[j] = true;
                        ++suppressed_hasnext;
                    }
                }
            }

            if (owner_fallback_ambiguity)
                std::cerr << "Warning: repeated names without matching owner context use similarity-ranked pairing; inspect locations/signatures for ambiguity\n";
            int unmatched_source = 0;
            int unmatched_target = 0;
            for (bool used : source_used) {
                if (!used) ++unmatched_source;
            }
            for (bool used : target_used) {
                if (!used) ++unmatched_target;
            }

            const float source_total = static_cast<float>(funcs1.size());
            const float matched_total = static_cast<float>(reports.size());
            const float coverage = [&]() -> float {
                if (source_total > 0.0f) {
                    return matched_total / source_total;
                }
                return funcs2.empty() ? 1.0f : 0.0f;
            }();
            const float overall_score = [&]() -> float {
                if (source_total > 0.0f) {
                    return total_combined / source_total;
                }
                return funcs2.empty() ? 1.0f : 0.0f;
            }();
            const float matched_avg =
                matched_total > 0.0f ? total_combined / matched_total : overall_score;
            const float ast_avg =
                matched_total > 0.0f ? total_ast_cosine / matched_total : overall_score;
            const float id_avg =
                matched_total > 0.0f ? total_id_cosine / matched_total : overall_score;
            const float line_balance_avg =
                matched_total > 0.0f ? total_line_balance / matched_total : overall_score;

            std::cout << "\n=== Function-by-Function Comparison (required score) ===\n";
            if (lang1 == Language::RUST && lang2 == Language::KOTLIN) {
                std::cout << "Rule: Rust snake_case/private generated names must match legal Kotlin lowerCamelCase names.\n";
                std::cout << "Examples: `foo_bar` -> `fooBar`, `___action42` -> `action42`, "
                          << "`___pop_Variant9` -> `popVariant9`.\n";
            } else if (lang1 == lang2) {
                std::cout << "Rule: same-language function names must match exactly.\n";
            } else {
                std::cout << "Rule: cross-language fallback uses canonicalized identifiers.\n";
            }
            std::cout << "Body metric: 40% identifier cosine + 20% normalized AST histogram + 40% ordered normalized logic; identifier-empty bodies use 40% AST + 60% logic; both are gated by ordered-logic similarity.\n";
            std::cout << "Strict matched pairs: " << reports.size() << " / " << funcs1.size()
                      << " source functions";
            std::cout << " | unmatched source: " << unmatched_source
                      << " | unmatched target: " << unmatched_target << "\n";
            std::cout << "Name coverage: " << std::fixed << std::setprecision(3) << coverage << "\n";
            std::cout << "Function body score (missing source functions count as 0): "
                      << std::fixed << std::setprecision(3) << overall_score << "\n";
            if (funcs1.empty()) {
                if (funcs2.empty()) {
                    std::cout << "No functions in either file; treating function similarity as 1.000.\n";
                } else {
                    std::cout << "No source functions; target defines functions (score forced to 0.000).\n";
                }
            } else if (funcs2.empty()) {
                std::cout << "Function comparison failed: no target function bodies were extracted.\n";
            } else if (unmatched_source > 0) {
                std::cout << "Function comparison incomplete: " << unmatched_source
                          << " source functions were not matched by strict parity.\n";
            }
            std::cout << "Matched average combined: " << std::fixed << std::setprecision(3) << matched_avg
                      << " | AST cosine avg: " << ast_avg
                      << " | identifier cosine avg: " << id_avg
                      << " | normalized-logic avg: " << (matched_total > 0 ? total_logic_similarity / matched_total : 0.0f)
                      << " | line-balance avg: " << line_balance_avg << "\n";
            if (!target_zero_reasons.empty()) {
                print_cheat_detection_failure(
                    std::cout,
                    "Target file score forced to 0.0000.",
                    target_zero_reasons,
                    "All function-by-function similarity scores are zero because the Kotlin target contains cheating or untranslated Rust residue.");
            }

            if (!reexport_hints.empty()) {
                std::cout << "\nRust mod.rs reexports detected:\n";
                if (funcs1.empty()) {
                    std::cout << "  No local Rust function bodies were extracted from this mod.rs; "
                              << "compare the actual reexported source files for body similarity.\n";
                }
                for (const auto& hint : reexport_hints) {
                    std::cout << "  - " << hint.exported_name
                              << " -> expected " << hint.expected_target_name
                              << " (actual: " << hint.actual_rust_path;
                    if (!hint.likely_source.empty()) {
                        std::cout << ", likely source: " << hint.likely_source;
                    }
                    std::cout << ")\n";
                }
            }

            if (unmatched_source > 0) {
                std::cout << "\nMissing expected target functions:\n";
                for (int i = 0; i < static_cast<int>(funcs1.size()); ++i) {
                    if (!source_used[i]) {
                        std::cout << "  - " << funcs1[i].qualified_name << ":" << funcs1[i].start_line << " " << funcs1[i].signature
                                  << " -> expected " << expected_target_function_name(funcs1[i].name, lang1, lang2)
                                  << " (" << funcs1[i].line_count << " lines)\n";
                    }
                }
            }

            if (unmatched_target > 0) {
                std::cout << "\nExtra target functions not matched by strict source-name parity:\n";
                for (int j = 0; j < static_cast<int>(funcs2.size()); ++j) {
                    if (!target_used[j]) {
                        std::cout << "  - " << (with_companions ? target_locations[j] + ":" : "") << funcs2[j].qualified_name << ":" << funcs2[j].start_line
                                  << " (" << funcs2[j].line_count << " lines)\n";
                    }
                }
            }
            if (suppressed_hasnext > 0) {
                std::cout << "\nIterator-trait equivalence: suppressed "
                          << suppressed_hasnext
                          << " forced `hasNext` method(s) (Kotlin Iterator interface "
                             "requires it; Rust's Iterator::next folds the check into "
                             "Option<T>).\n";
            }

            auto line_reports = reports;
            std::sort(line_reports.begin(), line_reports.end(),
                [](const FunctionPairReport& a, const FunctionPairReport& b) {
                    int a_gap = std::abs(a.line_gap);
                    int b_gap = std::abs(b.line_gap);
                    if (a_gap != b_gap) return a_gap > b_gap;
                    return a.source_name < b.source_name;
                });

            std::cout << "\n=== Worst Line Parity (top 25 by absolute line gap) ===\n";
            std::cout << "Source\tExpectedTarget\tActualTarget\tSrcLines\tTargetLines\tLineRatio\tLineGap\tCombined\n";
            for (size_t i = 0; i < std::min<size_t>(25, line_reports.size()); ++i) {
                const auto& report = line_reports[i];
                std::cout << report.source_name << "\t"
                          << report.expected_name << "\t"
                          << report.target_name << "\t"
                          << report.source_lines << "\t"
                          << report.target_lines << "\t"
                          << std::fixed << std::setprecision(3) << report.line_ratio << "\t"
                          << report.line_gap << "\t"
                          << report.combined << "\n";
            }

            std::sort(reports.begin(), reports.end(),
                [](const FunctionPairReport& a, const FunctionPairReport& b) {
                    if (a.combined != b.combined) return a.combined < b.combined;
                    if (std::abs(a.line_gap) != std::abs(b.line_gap)) {
                        return std::abs(a.line_gap) > std::abs(b.line_gap);
                    }
                    return a.source_name < b.source_name;
                });

            std::cout << "\n=== Per-Function Similarity (strict name parity, worst score first) ===\n";
            std::cout << "Source\tExpectedTarget\tActualTarget\tSrcLines\tTargetLines\tLineRatio\tLineGap"
                      << "\tSrcNodes\tTargetNodes\tASTCosine\tIdentifierCosine\tNormalizedLogic\tCombined\tStubGuard\n";
            for (const auto& report : reports) {
                std::cout << report.source_name << "\t"
                          << report.expected_name << "\t"
                          << report.target_name << "\t"
                          << report.source_lines << "\t"
                          << report.target_lines << "\t"
                          << std::fixed << std::setprecision(3) << report.line_ratio << "\t"
                          << report.line_gap << "\t"
                          << report.source_nodes << "\t"
                          << report.target_nodes << "\t"
                          << report.ast_cosine << "\t"
                          << report.id_cosine << "\t"
                          << report.logic_similarity << "\t"
                          << report.combined << "\t"
                          << (report.stub_guarded ? "yes" : "no") << "\n";
            }
            std::cout << "\n=== Ordered Normalized Logic Evidence (differences remain visible regardless of combined score) ===\n";
            for (const auto& candidate : candidates) {
                if (!source_used[candidate.source_index] || !target_used[candidate.target_index]) continue;
                const auto& source = funcs1[candidate.source_index];
                const auto& target = funcs2[candidate.target_index];
                // Print only the actual selected pair, not rejected overload candidates.
                auto selected = std::find_if(reports.begin(), reports.end(), [&](const auto& report) {
                    return report.source_name == source.qualified_name + ":" + std::to_string(source.start_line) + " " + source.signature &&
                           report.target_name == target_label(candidate.target_index);
                });
                if (selected == reports.end()) continue;
                auto source_tokens = normalized_logic_tokens(source.body_tree.get());
                auto target_tokens = normalized_logic_tokens(target.body_tree.get());
                if (source_tokens == target_tokens) {
                    std::cout << source.qualified_name << ':' << source.start_line << " -> " << (with_companions ? target_locations[candidate.target_index] + ":" : "") << target.qualified_name << ':' << target.start_line
                              << ": exact normalized logic sequence (" << source_tokens.size() << " tokens)\n";
                    continue;
                }
                std::cout << source.qualified_name << ':' << source.start_line << " -> " << (with_companions ? target_locations[candidate.target_index] + ":" : "") << target.qualified_name << ':' << target.start_line << '\n';
                auto print_sequence = [](const char* side, const auto& tokens) {
                    std::cout << "  " << side << ":";
                    for (size_t i = 0; i < tokens.size(); ++i) std::cout << ' ' << i + 1 << '=' << tokens[i];
                    std::cout << '\n';
                };
                print_sequence("source", source_tokens);
                print_sequence("target", target_tokens);
            }


            // Per-function parity is 1:1 — each Rust function paired with its
            // strict-name Kotlin counterpart, one line per pair. The
            // unmatched-source / unmatched-target lists above already surface
            // the gaps. An N x M Cartesian product of "every Rust function vs
            // every Kotlin function" is not a faithfulness check and is not
            // emitted: this is a gap tester, not a cross-similarity explorer.


        } else if (mode[0] != '-' && argc >= 5) {
            // Default: compare two files with explicit languages
            ASTParser parser;
            std::string file1 = argv[1];
            Language lang1 = parse_language(argv[2]);
            std::string file2 = argv[3];
            Language lang2 = parse_language(argv[4]);
            auto direct_provenance_proposals =
                provenance_proposals_for_direct_file_pair(file1, lang1, file2, lang2);
            warn_kotlin_suspicious_constructs(file1, lang1, file2, lang2);

            auto file_contains_macro_rules = [](const std::string& path) -> bool {
                std::ifstream in(path);
                if (!in) return false;
                std::string content(
                    (std::istreambuf_iterator<char>(in)),
                    std::istreambuf_iterator<char>());
                return content.find("macro_rules!") != std::string::npos;
            };

            bool macro_friendly = false;
            if (lang1 == Language::RUST) macro_friendly |= file_contains_macro_rules(file1);
            if (lang2 == Language::RUST) macro_friendly |= file_contains_macro_rules(file2);

            verify_comparison_identity(file1, lang1, file2, lang2);
            std::cout << "Parsing " << language_name(lang1) << " file: " << file1 << "\n";
            std::optional<std::string> file1_source;
            if (lang1 == Language::RUST && lang2 == Language::KOTLIN) {
                file1_source = strip_rust_cfg_test_blocks(read_file_to_string(file1));
            }
            TreePtr tree1 = file1_source ? parser.parse_string(*file1_source, lang1) : parser.parse_file(file1, lang1);

            std::cout << "Parsing " << language_name(lang2) << " file: " << file2 << "\n";
            std::optional<std::string> file2_source;
            if (lang2 == Language::RUST && lang1 == Language::KOTLIN) {
                file2_source = strip_rust_cfg_test_blocks(read_file_to_string(file2));
            }
            TreePtr tree2 = file2_source ? parser.parse_string(*file2_source, lang2) : parser.parse_file(file2, lang2);

            std::cout << "\n";
            std::string text1 = file1_source ? *file1_source : read_file_to_string(file1);
            std::string text2 = file2_source ? *file2_source : read_file_to_string(file2);
            if (lang1 == Language::KOTLIN && lang2 == Language::CPP) {
                std::cout << "\n=== Primary Literal Transliteration Score ===\n";
                print_transliteration_distance(transliteration_distance(text1, lang1, text2, lang2), std::cout);
                std::cout << "\n=== Legacy Structural/Vocabulary Diagnostics (separate from literal score) ===\n";
            }
            auto report = ASTSimilarity::compare(tree1.get(), tree2.get(), macro_friendly);
            report.print();

            auto translit_report = TransliterationSimilarity::compare(text1, lang1, text2, lang2);

            std::cout << "\n=== Transliteration-Normalized Text ===\n";
            std::cout << "Tokens:               "
                      << translit_report.source_tokens << " vs "
                      << translit_report.target_tokens << "\n";
            std::cout << "Token cosine:         " << std::fixed << std::setprecision(4)
                      << translit_report.token_cosine << "\n";
            std::cout << "Token jaccard:        " << std::fixed << std::setprecision(4)
                      << translit_report.token_jaccard << "\n";
            std::cout << "5-gram jaccard:       " << std::fixed << std::setprecision(4)
                      << translit_report.kgram_jaccard << "\n";
            std::cout << "Transliteration score:" << std::fixed << std::setprecision(4)
                      << translit_report.score << "\n";
            print_direct_provenance_proposals(direct_provenance_proposals);

            auto ids1 = file1_source ? parser.extract_identifiers(*file1_source, lang1)
                                     : parser.extract_identifiers_from_file(file1, lang1);
            auto ids2 = file2_source ? parser.extract_identifiers(*file2_source, lang2)
                                     : parser.extract_identifiers_from_file(file2, lang2);
            float content_score = 0.0f;

            std::cout << "\n=== Identifier Content Analysis ===\n";
            std::cout << "Identifiers:          "
                      << ids1.total_identifiers << " vs " << ids2.total_identifiers << "\n";
            std::cout << "Unique (raw):         "
                      << ids1.identifier_freq.size() << " vs " << ids2.identifier_freq.size() << "\n";
            std::cout << "Unique (canonical):   "
                      << ids1.canonical_freq.size() << " vs " << ids2.canonical_freq.size() << "\n";
            std::cout << "Raw cosine:           " << std::fixed << std::setprecision(4)
                      << ids1.identifier_cosine_similarity(ids2) << "\n";
            std::cout << "Canonical cosine:     " << std::fixed << std::setprecision(4)
                      << ids1.canonical_cosine_similarity(ids2) << "\n";
            std::cout << "Canonical jaccard:    " << std::fixed << std::setprecision(4)
                      << ids1.canonical_jaccard_similarity(ids2) << "\n";

            bool function_scored = false;
            CodebaseComparator::FunctionComparisonResult function_result;
            std::vector<FunctionInfo> src_funcs_for_report;
            std::vector<FunctionInfo> tgt_funcs_for_report;
            std::vector<std::string> target_zero_reasons;
            if (lang2 == Language::KOTLIN) {
                target_zero_reasons =
                    CodebaseComparator::kotlin_contamination_reasons_for_files({file2});
            }
            try {
                src_funcs_for_report = file1_source ? parser.extract_function_infos(*file1_source, lang1)
                                                    : parser.extract_function_infos_from_file(file1, lang1);
                tgt_funcs_for_report = file2_source ? parser.extract_function_infos(*file2_source, lang2)
                                                    : parser.extract_function_infos_from_file(file2, lang2);
                function_result = CodebaseComparator::compare_function_sets(
                    src_funcs_for_report, tgt_funcs_for_report);
                function_scored = (function_result.score >= 0.0f);
                if (!target_zero_reasons.empty()) {
                    content_score = 0.0f;
                } else if (function_scored) {
                    content_score = function_result.score;
                }
            } catch (...) {
                function_scored = false;
            }

            bool file1_stubs = parser.has_stub_bodies_in_files({file1}, lang1);
            bool file2_stubs = parser.has_stub_bodies_in_files({file2}, lang2);

            if (function_scored) {
                // OR with file-level result so that categorical stubs (e.g.
                // mod.rs port-lint header) are never cleared by function scoring.
                file1_stubs = file1_stubs || function_result.has_source_stub;
                file2_stubs = file2_stubs || function_result.has_target_stub;

                // Short-circuit: if the target is a stub, skip blending entirely.
                // Without this guard the lifting logic below can inflate content_score
                // before the final zero-out.
                if (file2_stubs) {
                    content_score = 0.0f;
                    goto stub_check;
                }

                // Reports are function-by-function only. Whole-file AST shape
                // and transliteration-normalized text stay diagnostic and never
                // rescue a weak function body/parameter match.
                if (!target_zero_reasons.empty()) {
                    content_score = 0.0f;
                } else {
                    content_score = function_result.score;
                }
            }

            if (function_scored) {
                std::cout << "\n=== Function-by-Function Comparison (required score) ===\n";
                std::cout << "Scored by function bodies: "
                          << function_result.source_total << " vs "
                          << function_result.target_total << " functions, matched "
                          << function_result.matched_pairs << "\n";
                if (function_result.source_total == 0) {
                    if (function_result.target_total == 0) {
                        std::cout << "No functions in either file; treating function similarity as 1.000.\n";
                    } else {
                        std::cout << "No source functions; target defines functions (score forced to 0.000).\n";
                    }
                } else if (function_result.target_total == 0) {
                    std::cout << "Function comparison failed: no target function bodies were extracted.\n";
                } else if (function_result.unmatched_source > 0) {
                    std::cout << "Function comparison incomplete: "
                              << function_result.unmatched_source
                              << " source functions were not matched by name/parity.\n";
                }
                if (function_result.has_source_stub || function_result.has_target_stub) {
                    std::cout << "Function-level TODO/stub markers found; affected matches scored as 0.\n";
                }
            }

            stub_check:
            // IMPORTANT: stubs in the *target* indicate incomplete transliteration and should gate completion.
            // Stubs in the *source* are treated as baseline and should not force similarity to zero.
            if (file2_stubs) {
                content_score = 0.0f;
                std::cout << "\n*** STUB DETECTED ***\n";
                if (file1_stubs)
                    std::cout << "  " << file1 << " has TODO/stub/placeholder in function bodies\n";
                std::cout << "  " << file2 << " has TODO/stub/placeholder in function bodies\n";
                std::cout << "  Function-by-function score forced to 0.0000\n";
            } else if (!target_zero_reasons.empty()) {
                content_score = 0.0f;
                print_cheat_detection_failure(
                    std::cout,
                    file2 + " score forced to 0.0000.",
                    target_zero_reasons,
                    "The required function-by-function score is invalidated.");
            } else {
                if (file1_stubs) {
                    std::cout << "\nNote: " << file1
                              << " contains TODO/stub/placeholder markers in function bodies (source baseline).\n";
                }
                std::cout << "\nFunction-by-function Score:  " << std::fixed << std::setprecision(4)
                          << content_score << "\n";
            }

            std::cout << "\n=== " << language_name(lang1) << " AST Histogram ===\n";
            print_histogram(report.hist1);

            std::cout << "\n=== " << language_name(lang2) << " AST Histogram ===\n";
            print_histogram(report.hist2);

            // Show unmapped node types for diagnostics (only if any exist)
            auto& unmapped = parser.get_unmapped_node_types();
            if (!unmapped.empty()) {
                int total_unmapped = 0;
                for (const auto& [_, count] : unmapped) total_unmapped += count;
                std::cout << "\n=== Unmapped Node Types (" << unmapped.size()
                          << " types, " << total_unmapped << " nodes) ===\n";
                std::vector<std::pair<std::string, int>> sorted_unmapped(unmapped.begin(), unmapped.end());
                std::sort(sorted_unmapped.begin(), sorted_unmapped.end(),
                    [](const auto& a, const auto& b) { return a.second > b.second; });
                for (const auto& [type, count] : sorted_unmapped) {
                    std::cout << "  " << std::setw(40) << std::left << type
                              << ": " << count << "\n";
                }
            }

            // Extract and compare comment statistics
            std::cout << "\n=== " << language_name(lang1) << " Comments ===\n";
            auto comments1 = parser.extract_comments_from_file(file1, lang1);
            comments1.print();

            std::cout << "\n=== " << language_name(lang2) << " Comments ===\n";
            auto comments2 = parser.extract_comments_from_file(file2, lang2);
            comments2.print();

            // Documentation comparison
            std::cout << "\n=== Documentation Comparison ===\n";
            int doc_diff = std::abs(comments1.doc_comment_count - comments2.doc_comment_count);
            int line_diff = std::abs(comments1.total_doc_lines - comments2.total_doc_lines);
            std::cout << "Doc comment count: " << comments1.doc_comment_count
                      << " vs " << comments2.doc_comment_count
                      << " (diff: " << doc_diff << ")\n";
            std::cout << "Doc lines:         " << comments1.total_doc_lines
                      << " vs " << comments2.total_doc_lines
                      << " (diff: " << line_diff << ")\n";

            // Simple doc coverage similarity
            float doc_count_sim = 1.0f;
            if (comments1.doc_comment_count > 0 || comments2.doc_comment_count > 0) {
                int max_doc = std::max(comments1.doc_comment_count, comments2.doc_comment_count);
                int min_doc = std::min(comments1.doc_comment_count, comments2.doc_comment_count);
                doc_count_sim = static_cast<float>(min_doc) / static_cast<float>(max_doc);
            }
            std::cout << "Doc count similarity: " << std::fixed << std::setprecision(2)
                      << (doc_count_sim * 100.0f) << "%\n";

            // Bag-of-words text similarity for documentation
            float doc_cosine = comments1.doc_cosine_similarity(comments2);
            float doc_jaccard = comments1.doc_jaccard_similarity(comments2);

            // Doc amount grading:
            // - coverage: target/source doc lines, capped at 1.0 (extra docs are not penalized)
            // - balance:  min/max doc lines (informational; does penalize extras)
            float doc_line_cov = comments1.doc_line_coverage_capped(comments2);
            float doc_line_bal = comments1.doc_line_balance(comments2);

            // Equal-weight doc grade: similarity + amount.
            float doc_weighted = 0.5f * doc_cosine + 0.5f * doc_line_cov;

            std::cout << "Doc text cosine:      " << std::fixed << std::setprecision(2)
                      << (doc_cosine * 100.0f) << "%\n";
            std::cout << "Doc text jaccard:     " << std::fixed << std::setprecision(2)
                      << (doc_jaccard * 100.0f) << "%\n";
            std::cout << "Doc line coverage:    " << std::fixed << std::setprecision(2)
                      << (doc_line_cov * 100.0f) << "%\n";
            std::cout << "Doc line balance:     " << std::fixed << std::setprecision(2)
                      << (doc_line_bal * 100.0f) << "%\n";
            std::cout << "Doc weighted (eq):    " << std::fixed << std::setprecision(2)
                      << (doc_weighted * 100.0f) << "%\n";
            std::cout << "Unique doc words:     " << comments1.word_freq.size()
                      << " vs " << comments2.word_freq.size() << "\n";
            if (!direct_provenance_proposals.empty()) {
                write_port_lint_proposed_changes_file(
                    direct_provenance_proposals,
                    repo_relative_display_path(file1),
                    repo_relative_display_path(file2));
                std::cout << "\nGenerated: port_lint_proposed_changes.md\n";
            }

        } else {
            print_usage(argv[0]);
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
