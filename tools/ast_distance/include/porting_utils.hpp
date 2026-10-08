#pragma once

/**
 * Porting utilities for kotlinx.coroutines C++ port analysis.
 *
 * Features:
 * - TODO scanning with tag extraction
 * - Lint checks (unused parameters)
 * - Line counting and ratio analysis
 * - "Transliterated from:" header parsing
 * - Stub detection
 */

#include <string>
#include <vector>
#include <map>
#include <set>
#include <regex>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace ast_distance {

/**
 * Represents a TODO comment found in source code.
 */
struct TodoItem {
    std::string file_path;
    int line_num = 0;
    std::string tag;        // e.g., "port", "semantics", "suspend-plugin"
    std::string message;
    std::vector<std::string> context;  // Lines around the TODO

    // Optional: Kotlin line reference extracted from message
    int kt_line_start = 0;
    int kt_line_end = 0;

    void print(bool verbose = true) const {
        printf("%s:%d: TODO(%s): %s\n",
               file_path.c_str(), line_num,
               tag.empty() ? "untagged" : tag.c_str(),
               message.c_str());

        if (verbose && !context.empty()) {
            printf("  Context:\n");
            for (const auto& line : context) {
                printf("    %s\n", line.c_str());
            }
        }
    }
};

/**
 * Represents a lint error found in source code.
 */
struct LintError {
    std::string file_path;
    int line_num = 0;
    std::string type;       // e.g., "unused_param", "missing_guard"
    std::string message;

    void print() const {
        printf("%s:%d: %s: %s\n", file_path.c_str(), line_num, type.c_str(), message.c_str());
    }
};

/**
 * File statistics for porting analysis.
 */
struct FileStats {
    std::string path;
    std::string relative_path;
    int line_count = 0;
    int code_lines = 0;      // Non-comment, non-blank lines
    int comment_lines = 0;
    int blank_lines = 0;
    bool is_stub = false;
    bool has_header_guard = false;
    std::string transliterated_from;  // Kotlin source path if found

    std::vector<TodoItem> todos;
    std::vector<LintError> lint_errors;

    float code_ratio(int kt_lines) const {
        if (kt_lines == 0) return 0.0f;
        return static_cast<float>(line_count) / static_cast<float>(kt_lines);
    }

    void print() const {
        printf("File: %s\n", path.c_str());
        printf("  Lines: %d (code: %d, comments: %d, blank: %d)\n",
               line_count, code_lines, comment_lines, blank_lines);
        if (!transliterated_from.empty()) {
            printf("  Transliterated from: %s\n", transliterated_from.c_str());
        }
        if (is_stub) printf("  WARNING: Appears to be a stub\n");
        if (!has_header_guard) printf("  WARNING: Missing header guard\n");
        printf("  TODOs: %zu, Lint errors: %zu\n", todos.size(), lint_errors.size());
    }
};

/**
 * Porting analysis utilities.
 */
class PortingAnalyzer {
public:
    // Keywords to ignore when checking for unused parameters
    static inline const std::set<std::string> IGNORED_KEYWORDS = {
        "if", "while", "for", "switch", "catch", "when", "return",
        "sizeof", "alignof", "decltype", "static_assert", "constexpr", "template",
        "void", "int", "bool", "float", "double", "char", "short", "long", "unsigned",
        "auto", "const", "static", "virtual", "override", "final", "explicit",
        "inline", "noexcept", "nullptr", "true", "false", "this", "new", "delete",
        // Kotlin keywords that look like function calls
        "check", "require", "assert"
    };

    static bool is_kotlin_file(const std::string& filepath) {
        return filepath.size() >= 3 &&
               filepath.substr(filepath.size() - 3) == ".kt";
    }

    /**
     * Convert camelCase / PascalCase to snake_case.
     * e.g. "endArg" -> "end_arg", "ForwardHeapKind" -> "forward_heap_kind"
     */
    static std::string camel_to_snake(const std::string& camel) {
        std::string snake;
        for (size_t i = 0; i < camel.size(); i++) {
            unsigned char uc = static_cast<unsigned char>(camel[i]);
            if (std::isupper(uc)) {
                if (i > 0) snake += '_';
                snake += static_cast<char>(std::tolower(uc));
            } else {
                snake += static_cast<char>(uc);
            }
        }
        return snake;
    }

    /**
     * Find the project root from a Kotlin file path by locating src/commonMain or src/commonTest.
     * Handles both absolute and relative paths. Returns empty string if not found.
     */
    static std::string find_project_root(const std::string& filepath) {
        size_t pos = filepath.find("/src/commonMain/");
        if (pos == std::string::npos) {
            pos = filepath.find("/src/commonTest/");
        }
        if (pos == std::string::npos) {
            pos = filepath.find("src/commonMain/");
            if (pos == 0) return ".";
        }
        if (pos == std::string::npos) {
            pos = filepath.find("src/commonTest/");
            if (pos == 0) return ".";
        }
        if (pos != std::string::npos) {
            return filepath.substr(0, pos);
        }
        return "";
    }

    /**
     * Load the Rust source file for a Kotlin port file.
     *
     * Uses the port-lint header to find the Rust source path, then tries to resolve it under
     * the project's tmp/ directory. Supports multiple layouts:
     * - <root>/tmp/<source_path>
     * - <root>/tmp/src/<source_path>
     * - <root>/tmp/<crate>/<source_path>
     * - <root>/tmp/<crate>/src/<source_path>
     */
    static std::string load_rust_source_for_port(const std::string& kotlin_filepath) {
        std::string source_path = extract_transliterated_from(kotlin_filepath);
        if (source_path.empty()) return "";

        std::string root = find_project_root(kotlin_filepath);
        if (root.empty()) return "";

        std::filesystem::path root_path(root);
        std::filesystem::path tmp_dir = root_path / "tmp";
        if (!std::filesystem::exists(tmp_dir) || !std::filesystem::is_directory(tmp_dir)) return "";

        std::vector<std::filesystem::path> candidates;
        candidates.push_back(tmp_dir / source_path);
        candidates.push_back(tmp_dir / "src" / source_path);

        for (const auto& entry : std::filesystem::directory_iterator(tmp_dir)) {
            if (!entry.is_directory()) continue;
            const auto dir = entry.path();
            candidates.push_back(dir / source_path);
            candidates.push_back(dir / "src" / source_path);
        }

        for (const auto& p : candidates) {
            std::error_code ec;
            if (!std::filesystem::exists(p, ec) || !std::filesystem::is_regular_file(p, ec)) continue;
            std::ifstream file(p.string());
            if (!file.is_open()) continue;
            std::stringstream buf;
            buf << file.rdbuf();
            return buf.str();
        }

        return "";
    }

    /**
     * Check if a Rust source file has a given parameter name as unused (prefixed with _).
     * Converts Kotlin camelCase to Rust snake_case.
     *
     * e.g. for Kotlin "_endArg", checks if Rust has "_end_arg" as a word boundary match.
     */
    static bool rust_has_unused_param(const std::string& rust_content,
                                      const std::string& kotlin_param) {
        if (rust_content.empty()) return false;

        std::string bare = kotlin_param;
        if (!bare.empty() && bare[0] == '_') {
            bare = bare.substr(1);
        }

        std::string snake = camel_to_snake(bare);
        std::string pattern = "_" + snake;

        try {
            std::regex rust_re("\\b" + pattern + "\\b");
            return std::regex_search(rust_content, rust_re);
        } catch (...) {
            return false;
        }
    }

    /**
     * Scan a file for TODO comments.
     */
    static std::vector<TodoItem> scan_todos(const std::string& filepath, int context_lines = 3) {
        std::vector<TodoItem> todos;
        std::ifstream file(filepath);
        if (!file.is_open()) return todos;

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }

        // Pattern: // TODO(tag): message  or  // TODO: message
        std::regex todo_re(R"(//\s*TODO(\([^)]*\))?:\s*(.+))");
        std::regex line_ref_re(R"(Line\s+(\d+)(?:-(\d+))?)", std::regex::icase);

        for (size_t i = 0; i < lines.size(); ++i) {
            std::smatch match;
            if (std::regex_search(lines[i], match, todo_re)) {
                TodoItem todo;
                todo.file_path = filepath;
                todo.line_num = static_cast<int>(i + 1);

                // Extract tag
                std::string tag_part = match[1].str();
                if (!tag_part.empty() && tag_part.size() > 2) {
                    todo.tag = tag_part.substr(1, tag_part.size() - 2);  // Remove ()
                }

                todo.message = match[2].str();

                // Extract Kotlin line reference if present
                std::smatch line_match;
                if (std::regex_search(todo.message, line_match, line_ref_re)) {
                    todo.kt_line_start = std::stoi(line_match[1].str());
                    if (line_match[2].matched) {
                        todo.kt_line_end = std::stoi(line_match[2].str());
                    } else {
                        todo.kt_line_end = todo.kt_line_start;
                    }
                }

                // Get context lines
                int start = std::max(0, static_cast<int>(i) - context_lines);
                int end = std::min(static_cast<int>(lines.size()), static_cast<int>(i) + context_lines + 1);
                for (int j = start; j < end; ++j) {
                    std::string prefix = (j == static_cast<int>(i)) ? ">>> " : "    ";
                    char buf[16];
                    snprintf(buf, sizeof(buf), "%4d: ", j + 1);
                    todo.context.push_back(prefix + buf + lines[j]);
                }

                todos.push_back(std::move(todo));
            }
        }

        return todos;
    }

    /**
     * Extract source path header from a file.
     * Recognizes formats:
     *   - "Transliterated from: path/to/file.<ext>"
     *   - "// port-lint: source path/to/file.<ext>"
     *   - "// port-lint: tests path/to/file.<ext>"
     *   - "// port-lint: source codex-rs/path/to/file.<ext>"
     * Returns the source path if found.
     *
     * `// port-lint: tests <path>` is returned as `tests:<path>` so test-only
     * translation units cannot be mistaken for the production port.
     */
    static std::string extract_transliterated_from(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) return "";

        // Match "Transliterated from: <path>" or "port-lint: source/tests <path>"
        std::regex trans_re(R"(Transliterated from:\s*(.+))", std::regex::icase);
        // NOTE: Some ports append qualifiers after the path (e.g. "(tests)").
        // For matching purposes we only want the first path token.
        std::regex portlint_re(R"(port-lint:\s*(source|tests)\s+([^\s]+))", std::regex::icase);
        std::string line;
        int line_count = 0;

        while (std::getline(file, line) && line_count++ < 50) {
            std::smatch match;
            if (std::regex_search(line, match, trans_re)) {
                std::string result = match[1].str();
                result.erase(0, result.find_first_not_of(" \t\r\n"));
                result.erase(result.find_last_not_of(" \t\r\n") + 1);
                return result;
            }
            if (std::regex_search(line, match, portlint_re)) {
                std::string kind = match[1].str();
                std::string result = match[2].str();
                result.erase(0, result.find_first_not_of(" \t\r\n"));
                result.erase(result.find_last_not_of(" \t\r\n") + 1);
                // Strip "codex-rs/" prefix if present
                if (result.substr(0, 9) == "codex-rs/") {
                    result = result.substr(9);
                }
                std::transform(kind.begin(), kind.end(), kind.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                if (kind == "tests") {
                    return "tests:" + result;
                }
                return result;
            }
        }

        return "";
    }

    /**
     * Analyze file statistics (line counts, stub detection, header guards).
     */
    static FileStats analyze_file(const std::string& filepath) {
        FileStats stats;
        stats.path = filepath;

        // Get relative path (simple: just filename for now)
        std::filesystem::path p(filepath);
        stats.relative_path = p.filename().string();
        std::string ext = p.extension().string();

        std::ifstream file(filepath);
        if (!file.is_open()) return stats;

        std::string content;
        std::string line;
        bool in_block_comment = false;

        while (std::getline(file, line)) {
            stats.line_count++;
            content += line + "\n";

            // Trim for analysis
            std::string trimmed = line;
            trimmed.erase(0, trimmed.find_first_not_of(" \t"));

            if (trimmed.empty()) {
                stats.blank_lines++;
            } else if (trimmed.find("//") == 0) {
                stats.comment_lines++;
            } else if (trimmed.find("/*") != std::string::npos) {
                stats.comment_lines++;
                if (trimmed.find("*/") == std::string::npos) {
                    in_block_comment = true;
                }
            } else if (in_block_comment) {
                stats.comment_lines++;
                if (trimmed.find("*/") != std::string::npos) {
                    in_block_comment = false;
                }
            } else {
                stats.code_lines++;
            }
        }

        // Check header guard
        if (filepath.find(".hpp") != std::string::npos ||
            filepath.find(".h") != std::string::npos) {
            stats.has_header_guard = (content.find("#pragma once") != std::string::npos ||
                                      content.find("#ifndef") != std::string::npos);
        } else {
            stats.has_header_guard = true;  // Not applicable for .cpp
        }

        // Stub detection - remove boilerplate, check remaining content.
        // Works for C++, Kotlin, Rust, and Python.
        std::string clean = content;
        // Remove line comments
        clean = std::regex_replace(clean, std::regex(R"(//[^\n]*)"), "");
        // Remove block comments
        clean = std::regex_replace(clean, std::regex(R"(/\*[\s\S]*?\*/)"), "");
        // Remove C++ includes
        clean = std::regex_replace(clean, std::regex(R"(#include[^\n]*)"), "");
        // Remove C++ namespace declarations
        clean = std::regex_replace(clean, std::regex(R"(namespace[^\{]*\{?)"), "");
        // Remove C++ pragma
        clean = std::regex_replace(clean, std::regex(R"(#pragma[^\n]*)"), "");
        // Remove Kotlin/Java package declarations
        clean = std::regex_replace(clean, std::regex(R"(package\s+[^\n]*)"), "");
        // Remove Kotlin/Java/Rust import/use statements
        clean = std::regex_replace(clean, std::regex(R"(import\s+[^\n]*)"), "");
        clean = std::regex_replace(clean, std::regex(R"(use\s+[^\n]*)"), "");
        // Remove Rust mod declarations
        clean = std::regex_replace(clean, std::regex(R"(mod\s+\w+\s*;)"), "");
        // Remove Python import statements
        clean = std::regex_replace(clean, std::regex(R"(from\s+[^\n]*)"), "");
        // Remove license/copyright blocks (common multi-line pattern)
        clean = std::regex_replace(clean, std::regex(R"(Copyright[^\n]*)"), "");
        clean = std::regex_replace(clean, std::regex(R"(Licensed under[^\n]*)"), "");
        clean = std::regex_replace(clean, std::regex(R"(Apache License[^\n]*)"), "");
        // Remove whitespace
        clean.erase(std::remove_if(clean.begin(), clean.end(),
                   [](unsigned char c) { return std::isspace(c); }), clean.end());

        // If after stripping all boilerplate less than 100 chars remain,
        // this file usually has no real implementation.
        //
        // However, some legitimately small files are "module roots" (Rust `mod foo;` + `pub use`)
        // or Kotlin marker files made up of a few empty `object` declarations. Those are real
        // transliterations and should not be blocked by guardrails.
        bool has_real_declarations = false;
        if (ext == ".kt" || ext == ".kts") {
            int object_hits = 0;
            for (size_t pos = 0; (pos = clean.find("object", pos)) != std::string::npos; pos += 6) {
                object_hits++;
            }
            if (object_hits >= 2 ||
                clean.find("fun") != std::string::npos ||
                clean.find("class") != std::string::npos ||
                clean.find("interface") != std::string::npos ||
                clean.find("enum") != std::string::npos ||
                clean.find("typealias") != std::string::npos ||
                clean.find("val") != std::string::npos ||
                clean.find("var") != std::string::npos) {
                has_real_declarations = true;
            }
        }

        stats.is_stub = (clean.length() < 100) && !has_real_declarations;

        // Extract transliterated from
        stats.transliterated_from = extract_transliterated_from(filepath);

        // Scan TODOs
        stats.todos = scan_todos(filepath);

        return stats;
    }

    /**
     * Extract parameter names from a Kotlin function parameter list.
     *
     * In Kotlin, parameters are declared as `name: Type` (name before colon),
     * unlike C++ where the type comes first. This extracts the identifier
     * immediately before each `:` separator, skipping annotations and modifiers.
     *
     * Collects ALL params including _-prefixed ones. In a port context, _-prefixed
     * params can hide incomplete translations and should be cross-referenced with
     * the Rust source (see check_unused_params()).
     */
    static std::vector<std::string> extract_kotlin_param_names(const std::string& args_str) {
        std::vector<std::string> params;

        // Split by commas (respecting angle brackets for generics)
        std::vector<std::string> segments;
        int angle_depth = 0;
        std::string current;
        for (char c : args_str) {
            if (c == '<') { angle_depth++; current += c; }
            else if (c == '>') { angle_depth--; current += c; }
            else if (c == ',' && angle_depth == 0) {
                segments.push_back(current);
                current.clear();
            } else {
                current += c;
            }
        }
        if (!current.empty()) segments.push_back(current);

        // For each segment, find `name:` pattern (Kotlin param syntax)
        std::regex kotlin_param_re(R"(\b(\w+)\s*:)");
        for (const auto& seg : segments) {
            // Strip default value
            std::string s = seg;
            // Find '=' not inside angle brackets
            int adepth = 0;
            size_t eq_pos = std::string::npos;
            for (size_t i = 0; i < s.size(); i++) {
                if (s[i] == '<') adepth++;
                else if (s[i] == '>') adepth--;
                else if (s[i] == '=' && adepth == 0) { eq_pos = i; break; }
            }
            if (eq_pos != std::string::npos) {
                s = s.substr(0, eq_pos);
            }

            // Find the last `name:` pattern (to skip modifiers like vararg)
            std::string last_name;
            auto tok_begin = std::sregex_iterator(s.begin(), s.end(), kotlin_param_re);
            auto tok_end = std::sregex_iterator();
            for (auto tok_it = tok_begin; tok_it != tok_end; ++tok_it) {
                last_name = (*tok_it)[1].str();
            }

            if (!last_name.empty() &&
                !IGNORED_KEYWORDS.count(last_name)) {
                params.push_back(last_name);
            }
        }

        return params;
    }

    /**
     * Extract parameter names from a C/C++ function parameter list.
     *
     * In C/C++, parameters are declared as `Type name` (name is last token),
     * so we extract the last identifier from each comma-separated segment.
     */
    static std::vector<std::string> extract_cpp_param_names(const std::string& args_str) {
        std::vector<std::string> params;
        std::stringstream ss(args_str);
        std::string param;

        while (std::getline(ss, param, ',')) {
            // Extract parameter name (last token before = if present)
            size_t eq_pos = param.find('=');
            if (eq_pos != std::string::npos) {
                param = param.substr(0, eq_pos);
            }

            // Tokenize and get last identifier
            std::regex token_re(R"(\b(\w+)\b)");
            std::string last_token;
            auto tok_begin = std::sregex_iterator(param.begin(), param.end(), token_re);
            auto tok_end = std::sregex_iterator();
            for (auto tok_it = tok_begin; tok_it != tok_end; ++tok_it) {
                last_token = (*tok_it)[1].str();
            }

            if (!last_token.empty() &&
                !IGNORED_KEYWORDS.count(last_token) &&
                last_token[0] != '_') {  // Allow _unused pattern
                params.push_back(last_token);
            }
        }

        return params;
    }

    static std::vector<LintError> check_unused_params(const std::string& filepath) {
        std::vector<LintError> errors;

        std::ifstream file(filepath);
        if (!file.is_open()) return errors;

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        bool kotlin = is_kotlin_file(filepath);

        // For Kotlin port files, load the Rust source for cross-referencing.
        // This prevents hiding incomplete translations by adding _ prefixes to params.
        std::string rust_content;
        if (kotlin) {
            rust_content = load_rust_source_for_port(filepath);
        }

        // For Kotlin files, require 'fun' keyword before the function name.
        // For C/C++ files, use the original heuristic pattern.
        std::regex func_re = kotlin
            ? std::regex(R"(\bfun\s+(?:<[^>]*>\s+)?(\w+)\s*\(([^)]*)\)\s*(?::\s*\w+(?:<[^>]*>)?\s*)?\{)")
            : std::regex(R"((\w+)\s*\(([^)]*)\)\s*(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?\{)");

        auto begin = std::sregex_iterator(content.begin(), content.end(), func_re);
        auto end = std::sregex_iterator();

        for (auto it = begin; it != end; ++it) {
            std::smatch match = *it;
            std::string func_name = match[1].str();
            std::string args_str = match[2].str();

            // Skip keywords that look like functions
            if (IGNORED_KEYWORDS.count(func_name)) continue;

            // Find function body (simple bracket counting)
            size_t start_pos = match.position() + match.length();
            int depth = 1;
            size_t idx = start_pos;

            while (idx < content.length() && depth > 0) {
                if (content[idx] == '{') depth++;
                else if (content[idx] == '}') depth--;
                idx++;
            }

            if (depth != 0) continue;  // Failed to parse

            std::string body = content.substr(start_pos, idx - start_pos - 1);

            // Parse parameters
            if (args_str.empty() || args_str.find("void") == 0) continue;

            std::vector<std::string> params = kotlin
                ? extract_kotlin_param_names(args_str)
                : extract_cpp_param_names(args_str);

            // Check usage in body
            for (const auto& p : params) {
                std::regex usage_re("\\b" + p + "\\b");
                if (!std::regex_search(body, usage_re)) {
                    // For _-prefixed params in Kotlin port files: only suppress if the Rust
                    // source also has the param as unused (_param). This prevents agents
                    // from hiding incomplete translations with _ prefix.
                    if (kotlin && !p.empty() && p[0] == '_') {
                        if (rust_has_unused_param(rust_content, p)) {
                            continue;
                        }
                    }

                    // Check for (void)param pattern (C/C++ only)
                    if (!kotlin) {
                        std::string void_cast1 = "(void)" + p;
                        std::string void_cast2 = "(void) " + p;
                        if (body.find(void_cast1) != std::string::npos ||
                            body.find(void_cast2) != std::string::npos) {
                            continue;
                        }
                    }

                    LintError err;
                    err.file_path = filepath;
                    err.line_num = static_cast<int>(
                        std::count(content.begin(), content.begin() + match.position(), '\n') + 1);
                    err.type = "unused_param";
                    err.message = "Unused parameter '" + p + "' in function '" + func_name + "'";
                    errors.push_back(err);
                }
            }
        }

        return errors;
    }

    /**
     * Run all lint checks on a file.
     */
    static std::vector<LintError> lint_file(const std::string& filepath) {
        std::vector<LintError> errors;

        // Unused parameters
        auto param_errors = check_unused_params(filepath);
        errors.insert(errors.end(), param_errors.begin(), param_errors.end());

        // Header guard check
        if (filepath.find(".hpp") != std::string::npos ||
            filepath.find(".h") != std::string::npos) {
            std::ifstream file(filepath);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                std::string content = buffer.str();

                if (content.find("#pragma once") == std::string::npos &&
                    content.find("#ifndef") == std::string::npos) {
                    LintError err;
                    err.file_path = filepath;
                    err.line_num = 1;
                    err.type = "missing_guard";
                    err.message = "Missing header guard (#pragma once or #ifndef)";
                    errors.push_back(err);
                }
            }
        }

        return errors;
    }

    /**
     * Scan a directory for source files and analyze them.
     * Supports C++ (.hpp, .cpp, .h), Kotlin (.kt), and Rust (.rs) files.
     */
    static std::vector<FileStats> analyze_directory(const std::string& directory) {
        std::vector<FileStats> results;

        if (std::filesystem::is_regular_file(directory)) {
            std::filesystem::path p(directory);
            std::string ext = p.extension().string();
            if (ext == ".hpp" || ext == ".cpp" || ext == ".h" ||
                ext == ".kt" || ext == ".kts" || ext == ".rs") {
                FileStats stats = analyze_file(directory);
                stats.lint_errors = lint_file(directory);
                results.push_back(std::move(stats));
            }
            return results;
        }

        for (const auto& entry : std::filesystem::recursive_directory_iterator(directory)) {
            if (!entry.is_regular_file()) continue;

            std::string path = entry.path().string();
            std::string ext = entry.path().extension().string();

            // Support C++, Kotlin, and Rust files
            if (ext != ".hpp" && ext != ".cpp" && ext != ".h" &&
                ext != ".kt" && ext != ".kts" && ext != ".rs") continue;

            // Skip vendor, build, tmp directories
            if (path.find("/vendor/") != std::string::npos ||
                path.find("/build/") != std::string::npos ||
                path.find("/tmp/") != std::string::npos ||
                path.find("/.git/") != std::string::npos) {
                continue;
            }

            FileStats stats = analyze_file(path);
            stats.lint_errors = lint_file(path);
            results.push_back(std::move(stats));
        }

        return results;
    }

    /**
     * Group TODOs by tag.
     */
    static std::map<std::string, std::vector<TodoItem>> group_todos_by_tag(
            const std::vector<TodoItem>& todos) {
        std::map<std::string, std::vector<TodoItem>> grouped;
        for (const auto& todo : todos) {
            std::string tag = todo.tag.empty() ? "untagged" : todo.tag;
            grouped[tag].push_back(todo);
        }
        return grouped;
    }

    /**
     * Print a TODO report.
     */
    static void print_todo_report(const std::vector<TodoItem>& todos, bool verbose = true) {
        if (todos.empty()) {
            printf("No TODOs found.\n");
            return;
        }

        printf("\n================================================================================\n");
        printf("TODO REPORT - Found %zu TODO(s)\n", todos.size());
        printf("================================================================================\n\n");

        // Group by tag
        auto grouped = group_todos_by_tag(todos);

        printf("Summary by tag:\n");
        for (const auto& [tag, items] : grouped) {
            printf("  %s: %zu\n", tag.c_str(), items.size());
        }
        printf("\n");

        if (!verbose) {
            for (const auto& todo : todos) {
                todo.print(false);
            }
            return;
        }

        // Detailed report
        for (const auto& todo : todos) {
            printf("--------------------------------------------------------------------------------\n");
            printf("FILE: %s\n", todo.file_path.c_str());
            printf("LINE: %d\n", todo.line_num);
            printf("TAG:  %s\n", todo.tag.empty() ? "none" : todo.tag.c_str());
            printf("MSG:  %s\n", todo.message.c_str());

            if (todo.kt_line_start > 0) {
                if (todo.kt_line_end > todo.kt_line_start) {
                    printf("KT:   Lines %d-%d\n", todo.kt_line_start, todo.kt_line_end);
                } else {
                    printf("KT:   Line %d\n", todo.kt_line_start);
                }
            }

            printf("\nContext:\n");
            for (const auto& line : todo.context) {
                printf("  %s\n", line.c_str());
            }
            printf("\n");
        }
    }

    /**
     * Print a lint report.
     */
    static void print_lint_report(const std::vector<LintError>& errors) {
        if (errors.empty()) {
            printf("No lint errors found.\n");
            return;
        }

        printf("\n================================================================================\n");
        printf("LINT REPORT - Found %zu error(s)\n", errors.size());
        printf("================================================================================\n\n");

        // Group by type
        std::map<std::string, std::vector<LintError>> grouped;
        for (const auto& err : errors) {
            grouped[err.type].push_back(err);
        }

        printf("Summary by type:\n");
        for (const auto& [type, items] : grouped) {
            printf("  %s: %zu\n", type.c_str(), items.size());
        }
        printf("\n");

        for (const auto& err : errors) {
            err.print();
        }
    }
};

} // namespace ast_distance
