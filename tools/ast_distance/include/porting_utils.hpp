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
        "inline", "noexcept", "nullptr", "true", "false", "this", "new", "delete"
    };

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
     *   - "Transliterated from: path/to/file.kt"
     *   - "// port-lint: source path/to/file.rs"
     *   - "// port-lint: source codex-rs/path/to/file.rs"
     * Returns the source path if found.
     */
    static std::string extract_transliterated_from(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) return "";

        // Match "Transliterated from: *.kt" or "port-lint: source *.rs"
        std::regex trans_re(R"(Transliterated from:\s*(.+\.kt))", std::regex::icase);
        std::regex portlint_re(R"(port-lint:\s*source\s+(.+\.rs))", std::regex::icase);
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
                std::string result = match[1].str();
                result.erase(0, result.find_first_not_of(" \t\r\n"));
                result.erase(result.find_last_not_of(" \t\r\n") + 1);
                // Strip "codex-rs/" prefix if present
                if (result.substr(0, 9) == "codex-rs/") {
                    result = result.substr(9);
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

        // Stub detection - remove comments and includes, check remaining content
        std::string clean = content;
        // Remove line comments
        clean = std::regex_replace(clean, std::regex(R"(//[^\n]*)"), "");
        // Remove block comments
        clean = std::regex_replace(clean, std::regex(R"(/\*[\s\S]*?\*/)"), "");
        // Remove includes
        clean = std::regex_replace(clean, std::regex(R"(#include[^\n]*)"), "");
        // Remove namespace declarations
        clean = std::regex_replace(clean, std::regex(R"(namespace[^\{]*\{?)"), "");
        // Remove pragma
        clean = std::regex_replace(clean, std::regex(R"(#pragma[^\n]*)"), "");
        // Remove whitespace
        clean.erase(std::remove_if(clean.begin(), clean.end(),
                   [](unsigned char c) { return std::isspace(c); }), clean.end());

        stats.is_stub = (clean.length() < 50);

        // Extract transliterated from
        stats.transliterated_from = extract_transliterated_from(filepath);

        // Scan TODOs
        stats.todos = scan_todos(filepath);

        return stats;
    }

    /**
     * Check for unused parameters in functions.
     * Simple heuristic-based checker.
     */
    static std::vector<LintError> check_unused_params(const std::string& filepath) {
        std::vector<LintError> errors;

        std::ifstream file(filepath);
        if (!file.is_open()) return errors;

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        // Simple function pattern: name(params) { or name(params) const {
        std::regex func_re(R"((\w+)\s*\(([^)]*)\)\s*(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?\{)");

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

            // Check usage in body
            for (const auto& p : params) {
                std::regex usage_re("\\b" + p + "\\b");
                if (!std::regex_search(body, usage_re)) {
                    // Check for (void)param pattern
                    std::string void_cast1 = "(void)" + p;
                    std::string void_cast2 = "(void) " + p;
                    if (body.find(void_cast1) == std::string::npos &&
                        body.find(void_cast2) == std::string::npos) {

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
