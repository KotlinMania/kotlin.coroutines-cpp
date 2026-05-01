#include "ast_parser.hpp"
#include "similarity.hpp"
#include "tree_lstm.hpp"
#include "codebase.hpp"
#include "porting_utils.hpp"
#include "transliteration_similarity.hpp"
#include "task_manager.hpp"
#include "symbol_analysis.hpp"
#include "symbol_extraction.hpp"
#include "symbol_extractor.hpp"
#include "reexport_config.hpp"
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

struct GuardrailsContext {
    bool active = false;
    std::string task_file;
    int agent = 0;
    bool override_mode = false;
};

static GuardrailsContext g_guardrails;
static ReexportConfig g_reexport_config;

static std::optional<std::chrono::seconds> file_age_seconds(const std::string& path) {
    try {
        if (!std::filesystem::exists(path)) return std::nullopt;
        auto ftime = std::filesystem::last_write_time(path);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now());
        auto age = std::chrono::system_clock::now() - sctp;
        return std::chrono::duration_cast<std::chrono::seconds>(age);
    } catch (...) {
        return std::nullopt;
    }
}

static void cull_stale_task_file_if_needed(const std::string& task_file, const std::string& mode, int agent) {
    static constexpr auto kStaleTtl = std::chrono::minutes(20);

    if (agent > 0) return;
    if (task_file.empty()) return;
    if (mode == "--init-tasks") return;

    auto age = file_age_seconds(task_file);
    if (!age.has_value()) return;
    if (*age <= std::chrono::duration_cast<std::chrono::seconds>(kStaleTtl)) return;

    try {
        std::filesystem::remove(task_file);
        std::filesystem::remove(task_file + ".lock");
        std::cerr << "Info: stale task file removed (" << task_file << ", age " << age->count()
                  << "s > " << std::chrono::duration_cast<std::chrono::seconds>(kStaleTtl).count() << "s).\n";
    } catch (...) {
        // Best-effort.
    }
}

// Forward declarations for guardrails helpers defined later in this file.
static void print_agent_activity_section(const TaskManager& tm, const std::string& task_file, int current_agent);

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

static bool rust_is_module_wiring_only(const std::filesystem::path& source_path) {
    std::string contents;
    try {
        contents = CodebaseComparator::read_file_to_string(source_path.string());
    } catch (...) {
        return false;
    }

    auto lines = rust_significant_lines(contents);
    if (lines.empty()) {
        // Empty after stripping comments/whitespace: treat as wiring-only.
        return true;
    }

    static const std::regex mod_re(
        R"(^\s*(pub(\(crate\))?\s+)?mod\s+[A-Za-z_][A-Za-z0-9_]*\s*;\s*$)");
    static const std::regex use_re(
        R"(^\s*(pub(\(crate\))?\s+)?use\s+.*;\s*$)");
    static const std::regex attr_re(
        R"(^\s*#\s*\[.*\]\s*$)");

    for (const auto& l : lines) {
        if (std::regex_match(l, mod_re)) continue;
        if (std::regex_match(l, use_re)) continue;
        if (std::regex_match(l, attr_re)) continue;
        return false;
    }

    return true;
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

static std::string current_project_name() {
    try {
        return std::filesystem::current_path().filename().string();
    } catch (...) {
        return "ast-distance-port";
    }
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
    std::filesystem::path absolute =
        input.is_absolute() ? input : (std::filesystem::current_path(ec) / input);
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
    return absolute.generic_string();
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

static void write_missing_config_after_comparison(const ConfigEndpoint& source,
                                                  const ConfigEndpoint& target) {
    const std::string path = default_reexport_config_path();
    if (g_reexport_config.loaded || std::filesystem::exists(path)) return;
    if (write_ast_distance_config_stub(path, current_project_name(), source, target)) {
        std::cerr << "Info: wrote " << path
                  << " from this comparison; add reexport_modules entries there as needed.\n";
    } else {
        std::cerr << "Warning: could not write " << path << ".\n";
    }
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

static bool comparison_mode_requires_direct_terminal(const std::string& mode, int argc) {
    return mode == "--compare-functions" || (mode[0] != '-' && argc >= 5);
}

static std::string parent_process_command() {
    std::string cmd = "ps -o comm= -p " + std::to_string(static_cast<long long>(getppid()));
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    std::string out;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        out += buffer;
    }
    pclose(pipe);
    out.erase(std::remove(out.begin(), out.end(), '\n'), out.end());
    out.erase(std::remove(out.begin(), out.end(), '\r'), out.end());
    return out;
}

struct ProcessCommandLine {
    long long pid = 0;
    long long ppid = 0;
    std::string command;
};

static std::vector<ProcessCommandLine> process_command_lines() {
    FILE* pipe = popen("ps -axo pid=,ppid=,command=", "r");
    if (!pipe) return {};

    std::vector<ProcessCommandLine> rows;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        std::istringstream in(line);
        ProcessCommandLine row;
        if (!(in >> row.pid >> row.ppid)) {
            continue;
        }
        std::getline(in, row.command);
        row.command = ltrim_copy(row.command);
        rows.push_back(std::move(row));
    }
    pclose(pipe);
    return rows;
}

static std::string lowercase_for_guard(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

static bool command_line_contains_filter_operator(const std::string& command) {
    static const std::regex shell_operator(
        R"((^|[[:space:]])(\|&?|\&>|[0-9]*>{1,2}&?[0-9-]?|<\(|>\(|<<<)([[:space:]]|$))");
    return std::regex_search(command, shell_operator);
}

static bool command_line_contains_filter_program(const std::string& command) {
    static const std::regex filter_program(
        R"((^|[[:space:]/])(grep|egrep|fgrep|rg|ripgrep|awk|sed|tee|head|tail|cut|sort|uniq|wc|jq|yq|perl|ruby|node|xargs|less|more|bat|cat|script|unbuffer|stdbuf|expect)([[:space:]]|$))",
        std::regex::icase);
    return std::regex_search(command, filter_program);
}

static std::vector<std::string> visible_filter_pipeline_reasons() {
    std::vector<std::string> reasons;
    auto rows = process_command_lines();
    const long long self = static_cast<long long>(getpid());
    const long long parent = static_cast<long long>(getppid());

    std::unordered_map<long long, ProcessCommandLine> by_pid;
    by_pid.reserve(rows.size());
    for (const auto& row : rows) {
        by_pid[row.pid] = row;
    }

    auto add_command_reason = [&](const std::string& role, const std::string& command) {
        std::string trimmed = command;
        if (trimmed.size() > 180) {
            trimmed = trimmed.substr(0, 177) + "...";
        }
        reasons.push_back(role + " command line contains output filtering: `" + trimmed + "`");
    };

    for (const auto& row : rows) {
        if (row.pid == self) {
            continue;
        }
        bool related = row.ppid == parent;
        long long ancestor = parent;
        int depth = 0;
        while (!related && ancestor > 1 && depth++ < 8) {
            auto it = by_pid.find(ancestor);
            if (it == by_pid.end()) break;
            if (row.pid == it->second.pid || row.ppid == it->second.pid) {
                related = true;
                break;
            }
            ancestor = it->second.ppid;
        }
        if (!related) {
            continue;
        }

        std::string lower = lowercase_for_guard(row.command);
        if (lower.find("ast_distance") != std::string::npos && row.pid != parent) {
            continue;
        }
        if (command_line_contains_filter_operator(row.command) ||
            command_line_contains_filter_program(row.command)) {
            add_command_reason(row.pid == parent ? "parent" : "sibling/ancestor", row.command);
            if (reasons.size() >= 3) {
                break;
            }
        }
    }
    return reasons;
}

static int reject_redirected_comparison_output_if_needed(const std::string& mode, int argc) {
    if (!comparison_mode_requires_direct_terminal(mode, argc)) {
        return 0;
    }

    std::vector<std::string> reasons;
    std::string parent = parent_process_command();
    std::string parent_lower = lowercase_for_guard(parent);
    if (parent_lower == "script" || parent_lower.find("/script") != std::string::npos) {
        reasons.push_back("parent process appears to be `script`; PTY wrapping is not allowed");
    }
    if (reasons.empty() && isatty(STDOUT_FILENO)) {
        return 0;
    }
    auto filter_reasons = visible_filter_pipeline_reasons();
    reasons.insert(reasons.end(), filter_reasons.begin(), filter_reasons.end());

    if (reasons.empty()) {
        return 0;
    }

    std::cerr << "\n*** REDIRECT GUARD REJECTED ***\n";
    std::cerr << "Comparison output must be read directly from the tool result.\n";
    std::cerr << "Why:\n";
    for (const auto& reason : reasons) {
        std::cerr << "  - " << reason << "\n";
    }
    std::cerr << "Run the comparison without pipes, redirects, grep, tee, or script wrapping.\n";
    return 2;
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
    std::cerr << "  " << program << " [--agent <number>] [--task-file <tasks.json>] [--override] <command>\n";
    std::cerr << "      Guardrails: when a task system is initialized, commands require --agent.\n\n";
    std::cerr << "      Loads .ast_distance_config.json when present; comparison commands create a stub when absent.\n\n";
    std::cerr << "  " << program << " <file1> <lang1> <file2> <lang2>\n";
    std::cerr << "      Compare AST similarity between two files\n\n";
    std::cerr << "  " << program << " --compare-functions <file1> <lang1> <file2> <lang2>\n";
    std::cerr << "      Compare strict function-name parity and per-function cosine/line similarity\n\n";
    std::cerr << "  " << program << " --dump <file> <rust|kotlin|cpp|python|typescript>\n";
    std::cerr << "      Dump AST structure of a file\n\n";
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
    std::cerr << "  " << program << " --symbols <kotlin_root> <cpp_root>\n";
    std::cerr << "      Run symbol analysis (duplicates + stubs)\n\n";
    std::cerr << "  " << program << " --symbols-duplicates <kotlin_root> <cpp_root>\n";
    std::cerr << "      Show duplicate class/struct definitions\n\n";
    std::cerr << "  " << program << " --symbols-stubs <kotlin_root> <cpp_root>\n";
    std::cerr << "      Show stub files/classes\n\n";
    std::cerr << "  " << program << " --symbols-symbol <kotlin_root> <cpp_root> <symbol> [--json]\n";
    std::cerr << "      Analyze a specific symbol (optionally output JSON)\n\n";
    std::cerr << "  " << program << " --symbol-parity <rust_root> <kotlin_root>\n";
    std::cerr << "      Rust->Kotlin symbol parity analysis\n\n";
    std::cerr << "  " << program << " --symbol-parity <rust_root> <kotlin_root> --json\n";
    std::cerr << "      Output symbol parity as JSON\n\n";
    std::cerr << "  " << program << " --symbol-parity <rust_root> <kotlin_root> --kind <function|struct|enum|trait>\n";
    std::cerr << "      Filter by symbol kind\n\n";
    std::cerr << "  " << program << " --symbol-parity <rust_root> <kotlin_root> --file <path>\n";
    std::cerr << "      Filter to specific source file\n\n";
    std::cerr << "  " << program << " --symbol-parity <rust_root> <kotlin_root> --missing-only\n";
    std::cerr << "      Show only missing symbols (concise mode)\n\n";
    std::cerr << "  " << program << " --import-map <kotlin_root>\n";
    std::cerr << "      Build type registry and show missing imports per file\n\n";
    std::cerr << "  " << program << " --import-map <kotlin_root> --summary\n";
    std::cerr << "      Show only per-file unresolved counts\n\n";
    std::cerr << "  " << program << " --import-map <kotlin_root> --file <path>\n";
    std::cerr << "      Show imports for a specific file\n\n";
    std::cerr << "  " << program << " --import-map <kotlin_root> --json\n";
    std::cerr << "      Output as JSON\n\n";
    std::cerr << "Compiler Error Analysis:\n";
    std::cerr << "  " << program << " --compiler-fixup <kotlin_root> <error_file>\n";
    std::cerr << "      Parse compiler errors and suggest import fixes\n\n";
    std::cerr << "  " << program << " --compiler-fixup <kotlin_root> <error_file> --json\n";
    std::cerr << "      Output as JSON\n\n";
    std::cerr << "  " << program << " --compiler-fixup <kotlin_root> <error_file> --verbose\n";
    std::cerr << "      Show alternative imports for ambiguous references\n\n";
    std::cerr << "Swarm Task Management:\n";
    std::cerr << "  " << program << " --init-tasks <src_dir> <src_lang> <tgt_dir> <tgt_lang> <task_file>\n";
    std::cerr << "      Generate task file from missing/incomplete ports\n\n";
    std::cerr << "  " << program << " --tasks <task_file>\n";
    std::cerr << "      Show task status summary\n\n";
    std::cerr << "  " << program << " --assign <task_file> [agent_number]\n";
    std::cerr << "      Assign highest-priority pending task to an agent session\n";
    std::cerr << "      If agent_number is omitted, a new one is allocated and printed\n";
    std::cerr << "      Outputs complete porting instructions and AGENTS.md guidelines\n\n";
    std::cerr << "  " << program << " --complete <task_file> <source_qualified>\n";
    std::cerr << "      Mark a task as completed\n\n";
    std::cerr << "  " << program << " --release <task_file> <source_qualified>\n";
    std::cerr << "      Release an assigned task back to pending\n\n";
    std::cerr << "  Languages: rust, kotlin, cpp, python, typescript\n\n";
    std::cerr << "Port-Lint Headers:\n";
    std::cerr << "  Add a header comment to each ported file to enable accurate source tracking.\n";
    std::cerr << "  This allows --deep analysis to match files by explicit declaration rather\n";
    std::cerr << "  than heuristic name matching, improving accuracy and enabling documentation\n";
    std::cerr << "  gap detection.\n\n";
    std::cerr << "  Format:\n";
    std::cerr << "    // port-lint: source <relative-path-to-source-file>\n";
    std::cerr << "    ## port-lint: source <relative-path-to-source-file>   (Python)\n\n";
    std::cerr << "  Example (Kotlin):\n";
    std::cerr << "    // port-lint: source core/src/config.rs\n";
    std::cerr << "    package com.example.config\n\n";
    std::cerr << "  The header must appear in the first 50 lines of the file.\n";
    std::cerr << "  When present, the tool will:\n";
    std::cerr << "    - Match files explicitly instead of by name similarity\n";
    std::cerr << "    - Compare documentation coverage between source and target\n";
    std::cerr << "    - Report 'Matched by header' vs 'Matched by name' statistics\n\n";
}

void dump_tree(Tree* node, int indent = 0) {
    std::string pad(indent * 2, ' ');
    const char* type_name = node_type_name(static_cast<NodeType>(node->node_type));

    std::cout << pad << type_name << " (" << node->label << ")";
    if (node->is_leaf()) {
        std::cout << " [leaf]";
    }
    std::cout << "\n";

    for (auto& child : node->children) {
        dump_tree(child.get(), indent + 1);
    }
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
    float avg_similarity = 0.0f;
    int avg_count = 0;
    for (const auto& m : comp.matches) {
        if (m.is_stub) { critical++; continue; }
        avg_similarity += m.similarity;
        avg_count++;
        if (m.similarity >= 0.85) excellent++;
        else if (m.similarity >= 0.60) good++;
        else critical++;
    }
    if (avg_count > 0) {
        avg_similarity /= avg_count;
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
        report << "| Average function body similarity | " << avg_similarity_cell() << " | required score |\n";
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
	            report << "There is missing documentation that is hurting overall scoring.\n\n";
	        }
	        report << "**Documentation coverage:** " << total_tgt_doc_lines << " / " 
	               << total_src_doc_lines << " lines (";
	        if (total_src_doc_lines > 0) {
	            report << std::fixed << std::setprecision(0)
	                   << (100.0f * total_tgt_doc_lines / total_src_doc_lines) << "%)\n\n";
	        } else {
	            report << "N/A)\n\n";
	        }
	        
	        report << "Documentation gaps (>20%), complete list:\n\n";
	        if (doc_gaps.empty()) {
	            report << "No significant documentation gaps found.\n\n";
	        } else {
	            int shown_docs = 0;
	            for (const auto& [gap, m] : doc_gaps) {
	                shown_docs++;
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
                  "Function similarity is the required body/parameter comparison; file-level shape does not rescue a port.\n\n";
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
        
        // File presence is the only file-level metric retained. Everything
        // else is function-level / type-level / symbol-level parity using
        // the totals already aggregated above (total_matched_functions,
        // total_source_functions, etc.) and the existing parity helpers.
        // The previous "Average Similarity" line averaged file-level scores
        // and obscured cases where a file appeared ported (matching imports,
        // doc strings) while its actual functions/classes were missing.
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
        report << "- **Cheat-zeroed Files:** " << zeroed_files << "\n";
        report << "- **Critical Issues:** " << critical << " files with <0.60 function similarity\n\n";
        
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

    // Agent-scoped dashboard (guardrails): in task mode, lock the view to the assigned file
    // and its direct imports to prevent "dashboard skipping" and out-of-scope prioritization.
    bool guard_active = (g_guardrails.active && g_guardrails.agent > 0);
    if (guard_active && !g_guardrails.override_mode) {
        TaskManager tm(g_guardrails.task_file);
        if (tm.load()) {
            print_agent_activity_section(tm, g_guardrails.task_file, g_guardrails.agent);

            std::string agent_id = std::to_string(g_guardrails.agent);
            const PortTask* focus = nullptr;
            for (const auto& t : tm.tasks) {
                if (t.status == TaskStatus::ASSIGNED && t.assigned_to == agent_id) {
                    focus = &t;
                    break;
                }
            }

            std::cout << "\n=== Agent Scope Dashboard ===\n\n";
            std::cout << "Task file: " << g_guardrails.task_file << "\n";
            std::cout << "You are agent #" << g_guardrails.agent << "\n\n";

            if (!focus) {
                std::cout << "No task assigned to agent #" << g_guardrails.agent << ".\n";
                std::cout << "Run: ast_distance --assign " << g_guardrails.task_file
                          << " " << g_guardrails.agent << "\n\n";
                std::cout << "Note: full project dashboard is locked in agent scope.\n";
                std::cout << "Use --override if you need full-project output.\n";
                return;
            }

            // Build relative_path -> logical key maps for locating the focus file.
            std::map<std::string, std::string> src_rel_to_key;
            for (const auto& [k, sf] : source.files) {
                src_rel_to_key[sf.relative_path] = k;
            }
            std::map<std::string, std::string> tgt_rel_to_key;
            for (const auto& [k, sf] : target.files) {
                tgt_rel_to_key[sf.relative_path] = k;
            }

            std::string focus_src_key;
            auto it_src = src_rel_to_key.find(focus->source_path);
            if (it_src != src_rel_to_key.end()) focus_src_key = it_src->second;

            std::string focus_tgt_key;
            auto it_tgt = tgt_rel_to_key.find(focus->target_path);
            if (it_tgt != tgt_rel_to_key.end()) focus_tgt_key = it_tgt->second;

            std::filesystem::path focus_source_path =
                std::filesystem::path(tm.source_root) / focus->source_path;
            std::filesystem::path focus_target_path =
                std::filesystem::path(tm.target_root) / focus->target_path;

            std::cout << "Assigned task: " << focus->source_qualified << "\n";
            std::cout << "Source: " << focus_source_path.string();
            if (!std::filesystem::exists(focus_source_path)) std::cout << " (missing?)";
            std::cout << "\n";
            std::cout << "Target: " << focus_target_path.string();
            if (std::filesystem::exists(focus_target_path)) std::cout << " (exists)";
            else std::cout << " (missing)";
            std::cout << "\n\n";

            // Scope: focus file + its direct imports, plus the target file + its imports (if it exists).
            std::set<std::string> scope_source_keys;
            std::set<std::string> scope_target_keys;
            if (!focus_src_key.empty()) {
                scope_source_keys.insert(focus_src_key);
                const auto& sf = source.files.at(focus_src_key);
                scope_source_keys.insert(sf.depends_on.begin(), sf.depends_on.end());
            }
            if (!focus_tgt_key.empty()) {
                scope_target_keys.insert(focus_tgt_key);
                const auto& tf = target.files.at(focus_tgt_key);
                scope_target_keys.insert(tf.depends_on.begin(), tf.depends_on.end());
            }

            std::cout << "Scope: " << scope_source_keys.size()
                      << " source files (focus + imports), " << scope_target_keys.size()
                      << " target files (focus + imports)\n";

            // Determine which scoped imports are already assigned to other agents.
            std::map<std::string, int> assigned_to_agent;
            for (const auto& t : tm.tasks) {
                if (t.status != TaskStatus::ASSIGNED) continue;
                int a = 0;
                try {
                    a = std::stoi(t.assigned_to);
                } catch (...) {
                    continue;
                }
                assigned_to_agent[t.source_qualified] = a;
            }

            std::map<std::string, int> blocked_by;
            for (const auto& key : scope_source_keys) {
                if (key == focus_src_key) continue;
                const auto& sf = source.files.at(key);
                auto ita = assigned_to_agent.find(sf.qualified_name);
                if (ita != assigned_to_agent.end() && ita->second != g_guardrails.agent) {
                    blocked_by[key] = ita->second;
                }
            }

            if (!blocked_by.empty()) {
                std::cout << "\nBlocked imports (assigned to other agents):\n";
                for (const auto& [key, a] : blocked_by) {
                    const auto& sf = source.files.at(key);
                    std::cout << "  - " << sf.qualified_name << " (agent #" << a << ")\n";
                }
            }

            // Match lookup by source logical key.
            std::map<std::string, const CodebaseComparator::Match*> match_by_source;
            for (const auto& m : comp.matches) {
                match_by_source[m.source_path] = &m;
            }
            std::set<std::string> unmatched_source(
                comp.unmatched_source.begin(), comp.unmatched_source.end());

            auto format_fn_parity = [](const CodebaseComparator::Match* m) -> std::string {
                if (!m || m->source_function_count <= 0) return "-";
                return std::to_string(m->matched_function_count) + "/" +
                       std::to_string(m->source_function_count);
            };

            auto print_scope_row = [&](const std::string& key, bool is_focus) {
                const auto& sf = source.files.at(key);
                const CodebaseComparator::Match* m = nullptr;
                auto it = match_by_source.find(key);
                if (it != match_by_source.end()) m = it->second;

                bool missing = unmatched_source.count(key) > 0;
                std::string status;
                if (is_focus) status = "FOCUS";
                if (missing) status = status.empty() ? "MISSING_FILE" : status + ",MISSING_FILE";
                if (m && m->is_stub) status = status.empty() ? "STUB(FAIL)" : status + ",STUB(FAIL)";
                if (m && m->todo_count > 0) status = status.empty() ? "TODO(FAIL)" : status + ",TODO(FAIL)";
                if (m && m->source_function_count > 0 &&
                    m->matched_function_count < m->source_function_count) {
                    status = status.empty() ? "MISSING_FUNCTIONS" : status + ",MISSING_FUNCTIONS";
                }
                auto itb = blocked_by.find(key);
                if (itb != blocked_by.end()) {
                    std::string b = "BLOCKED(agent #" + std::to_string(itb->second) + ")";
                    status = status.empty() ? b : status + "," + b;
                }

                int dependents = sf.dependent_count;
                std::string sim_str = "-";
                int todos = 0;
                int lint = 0;
                if (m && !missing) {
                    std::ostringstream sim;
                    sim << std::fixed << std::setprecision(2) << m->similarity;
                    sim_str = sim.str();
                    todos = m->todo_count;
                    lint = m->lint_count;
                }

                std::cout << std::setw(30) << std::left << sf.qualified_name.substr(0, 28)
                          << std::setw(11) << dependents
                          << std::setw(11) << sim_str
                          << std::setw(16) << format_fn_parity(m)
                          << std::setw(8) << todos
                          << std::setw(6) << lint
                          << status
                          << "\n";
            };

            std::cout << "\n=== Scope Work Items ===\n\n";
            std::cout << std::setw(30) << std::left << "File"
                      << std::setw(11) << "Dependents"
                      << std::setw(11) << "Similarity"
                      << std::setw(16) << "Function parity"
                      << std::setw(8) << "TODOs"
                      << std::setw(6) << "Lint"
                      << "Status\n";
            std::cout << std::string(90, '-') << "\n";

            if (!focus_src_key.empty()) {
                print_scope_row(focus_src_key, true);
            } else {
                std::cout << std::setw(30) << std::left << focus->source_qualified.substr(0, 28)
                          << std::setw(11) << focus->dependent_count
                          << std::setw(11) << "-"
                          << std::setw(16) << "-"
                          << std::setw(8) << 0
                          << std::setw(6) << 0
                          << "FOCUS,UNKNOWN_SOURCE_PATH\n";
            }

            std::vector<std::string> others;
            for (const auto& k : scope_source_keys) {
                if (k == focus_src_key) continue;
                others.push_back(k);
            }
            std::sort(others.begin(), others.end(),
                      [&](const std::string& a, const std::string& b) {
                          return source.files.at(a).dependent_count >
                                 source.files.at(b).dependent_count;
                      });
            for (const auto& k : others) {
                print_scope_row(k, false);
            }

            // Deterministic next action hint for the focus file.
            std::cout << "\n=== Next Action (Focus) ===\n\n";
            const CodebaseComparator::Match* focus_match = nullptr;
            if (!focus_src_key.empty()) {
                auto it = match_by_source.find(focus_src_key);
                if (it != match_by_source.end()) focus_match = it->second;
            }

            if (!focus_match || unmatched_source.count(focus_src_key) > 0) {
                std::cout << "Target file is missing. Create it and port the full implementation.\n";
            } else if (focus_match->is_stub) {
                std::cout << "Replace the stub with a real implementation (stubs are a failure mode).\n";
            } else if (focus_match->todo_count > 0) {
                std::cout << "Remove TODOs and finish the implementation (TODOs are a failure mode).\n";
            } else if (focus_match->source_function_count > 0 &&
                       focus_match->matched_function_count < focus_match->source_function_count) {
                std::cout << "Port missing functions to reach per-file parity.\n";
            } else if (focus_match->similarity < 0.85f) {
                std::cout << "Improve similarity to >= 0.85 (whole-file + identifier-content + parity).\n";
            } else {
                std::cout << "Looks complete. If you have validated behavior and tests, mark it complete.\n";
            }
            std::cout << "\nMark complete with:\n";
            std::cout << "  ast_distance --agent " << g_guardrails.agent << " --complete "
                      << g_guardrails.task_file << " " << focus->source_qualified << "\n";

            std::cout << "\nNote: full project dashboard is locked in agent scope.\n";
            std::cout << "Use --override to view the full project view.\n";
            return;
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
	        std::cout << "- Documentation coverage: " << total_tgt_doc_lines << " / "
	                  << total_src_doc_lines << " lines (" << pct << "%)\n";
		    } else {
		        std::cout << "- Documentation coverage: N/A (source has no docs)\n";
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
	        std::cout << "There is missing documentation that is hurting overall scoring.\n";
	    }
	    if (total_src_doc_lines > 0) {
	        int pct = static_cast<int>(doc_coverage_pct + 0.5f);
	        std::cout << "Documentation coverage: " << total_tgt_doc_lines << " / "
	                  << total_src_doc_lines << " lines (" << pct << "%)\n";
	    } else {
	        std::cout << "Documentation coverage: N/A (source has no docs)\n";
	    }
	    std::cout << "Files with >20% doc gap: " << doc_gaps.size() << "\n\n";

	    if (doc_gaps.empty()) {
	        std::cout << "No significant documentation gaps found.\n";
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

// ============ Agent Guardrails (Swarm Mode) ============

struct AgentState {
    long long last_used_epoch = 0;      // seconds since epoch
    long long last_score_epoch = 0;     // seconds since epoch
    float last_score = -1.0f;           // -1 = unknown
};

static long long now_epoch_seconds() {
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

static float activity_score_from_idle_minutes(long long idle_minutes) {
    if (idle_minutes <= 15) return 100.0f;
    if (idle_minutes >= 60) return 0.0f;
    // Linear decay from 100% at 15m idle to 0% at 60m idle.
    float t = static_cast<float>(idle_minutes - 15) / 45.0f;
    return 100.0f * (1.0f - t);
}

static std::string format_idle_minutes(long long idle_minutes) {
    if (idle_minutes < 0) idle_minutes = 0;
    long long h = idle_minutes / 60;
    long long m = idle_minutes % 60;
    if (h == 0) return std::to_string(m) + "m";
    std::ostringstream ss;
    ss << h << "h" << std::setw(2) << std::setfill('0') << m << "m";
    return ss.str();
}

static std::filesystem::path guardrails_agents_dir(const std::string& task_file) {
    std::filesystem::path base = std::filesystem::path(task_file).parent_path();
    if (base.empty()) base = std::filesystem::current_path();
    return base / ".cache" / "ast_distance" / "agents";
}

static std::filesystem::path agent_lock_path(const std::string& task_file, int agent) {
    auto dir = guardrails_agents_dir(task_file);
    return dir / ("agent_" + std::to_string(agent) + ".lock");
}

static std::filesystem::path agent_notice_path(const std::string& task_file, int agent) {
    auto dir = guardrails_agents_dir(task_file);
    return dir / ("agent_" + std::to_string(agent) + ".notice");
}

static std::filesystem::path agent_state_path(const std::string& task_file, int agent) {
    auto dir = guardrails_agents_dir(task_file);
    return dir / ("agent_" + std::to_string(agent) + ".state");
}

static AgentState load_agent_state(const std::filesystem::path& path) {
    AgentState st;
    std::ifstream f(path);
    if (!f.is_open()) return st;
    std::string line;
    while (std::getline(f, line)) {
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string k = line.substr(0, eq);
        std::string v = line.substr(eq + 1);
        try {
            if (k == "last_used_epoch") st.last_used_epoch = std::stoll(v);
            else if (k == "last_score_epoch") st.last_score_epoch = std::stoll(v);
            else if (k == "last_score") st.last_score = std::stof(v);
        } catch (...) {
            // Ignore malformed fields.
        }
    }
    return st;
}

static void save_agent_state(const std::filesystem::path& path, const AgentState& st) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream f(path);
    if (!f.is_open()) return;
    f << "last_used_epoch=" << st.last_used_epoch << "\n";
    f << "last_score_epoch=" << st.last_score_epoch << "\n";
    f << "last_score=" << st.last_score << "\n";
}

static void touch_agent_last_used(const std::string& task_file, int agent) {
    auto p = agent_state_path(task_file, agent);
    AgentState st = load_agent_state(p);
    st.last_used_epoch = now_epoch_seconds();
    save_agent_state(p, st);
}

static int read_pid_from_lockfile(const std::filesystem::path& path) {
    std::ifstream f(path);
    if (!f.is_open()) return -1;
    int pid = -1;
    f >> pid;
    return pid;
}

class AgentLock {
public:
    AgentLock(const std::string& task_file, int agent, bool override_wait)
        : fd_(-1), locked_(false), holder_pid_(-1) {
        lock_path_ = agent_lock_path(task_file, agent);
        std::filesystem::create_directories(lock_path_.parent_path());
        fd_ = open(lock_path_.string().c_str(), O_CREAT | O_RDWR, 0666);
        if (fd_ < 0) return;
        int flags = LOCK_EX;
        if (!override_wait) flags |= LOCK_NB;
        if (flock(fd_, flags) == 0) {
            locked_ = true;
            std::string pid_str = std::to_string(getpid()) + "\n";
            ftruncate(fd_, 0);
            lseek(fd_, 0, SEEK_SET);
            ::write(fd_, pid_str.c_str(), pid_str.size());
        } else {
            holder_pid_ = read_pid_from_lockfile(lock_path_);
        }
    }

    ~AgentLock() {
        if (fd_ >= 0) {
            if (locked_) flock(fd_, LOCK_UN);
            close(fd_);
        }
    }

    bool locked() const { return locked_; }
    int holder_pid() const { return holder_pid_; }
    const std::filesystem::path& path() const { return lock_path_; }

private:
    int fd_;
    bool locked_;
    int holder_pid_;
    std::filesystem::path lock_path_;
};

static void write_agent_notice(const std::string& task_file, int agent, const std::string& msg) {
    auto p = agent_notice_path(task_file, agent);
    std::filesystem::create_directories(p.parent_path());
    std::ofstream f(p, std::ios::app);
    if (!f.is_open()) return;
    f << msg << "\n";
}

static void print_and_clear_agent_notice(const std::string& task_file, int agent) {
    auto p = agent_notice_path(task_file, agent);
    std::ifstream f(p);
    if (!f.is_open()) return;
    std::stringstream buf;
    buf << f.rdbuf();
    std::string s = buf.str();
    if (!s.empty()) {
        std::cerr << "\n=== NOTICE (Agent #" << agent << ") ===\n\n";
        std::cerr << s << "\n";
    }
    std::error_code ec;
    std::filesystem::remove(p, ec);
}

static void print_agent_activity_section(const TaskManager& tm, const std::string& task_file, int current_agent) {
    std::cout << "\n=== Agent Activity ===\n\n";
    std::cout << "You are agent #" << current_agent << "\n\n";

    struct Row {
        int agent = 0;
        std::string task;
        std::string since;
        long long idle_min = 0;
        float score = 0.0f;
        float trend = 0.0f;
        bool has_trend = false;
    };
    std::vector<Row> rows;

    long long now = now_epoch_seconds();
    for (const auto& t : tm.tasks) {
        if (t.status != TaskStatus::ASSIGNED) continue;
        int agent = 0;
        try {
            agent = std::stoi(t.assigned_to);
        } catch (...) {
            continue; // Guardrails require numeric agent IDs.
        }
        AgentState st = load_agent_state(agent_state_path(task_file, agent));
        long long idle_s = (st.last_used_epoch > 0) ? (now - st.last_used_epoch) : (60LL * 60LL);
        long long idle_min = std::max(0LL, (idle_s + 30) / 60);
        float score = activity_score_from_idle_minutes(idle_min);

        float trend = 0.0f;
        bool has_trend = false;
        if (st.last_score >= 0.0f) {
            trend = score - st.last_score;
            has_trend = true;
        }

        // Update last reported score for trending.
        st.last_score = score;
        st.last_score_epoch = now;
        save_agent_state(agent_state_path(task_file, agent), st);

        Row r;
        r.agent = agent;
        r.task = t.source_qualified;
        r.since = t.assigned_at;
        r.idle_min = idle_min;
        r.score = score;
        r.trend = trend;
        r.has_trend = has_trend;
        rows.push_back(r);
    }

    if (rows.empty()) {
        std::cout << "No agents currently assigned.\n";
        return;
    }

    std::sort(rows.begin(), rows.end(),
              [](const Row& a, const Row& b) { return a.agent < b.agent; });

    std::cout << std::setw(8) << std::left << "Agent"
              << std::setw(34) << "Working On"
              << std::setw(10) << "Idle"
              << std::setw(10) << "Active"
              << "Trend\n";
    std::cout << std::string(76, '-') << "\n";
    for (const auto& r : rows) {
        std::ostringstream trend;
        if (r.has_trend) {
            int d = static_cast<int>(std::round(r.trend));
            if (d > 0) trend << "+" << d;
            else trend << d;
        } else {
            trend << "-";
        }

	        std::ostringstream score;
	        score << static_cast<int>(std::round(r.score)) << "%";

	        std::string agent_label = "#" + std::to_string(r.agent);
	        std::cout << std::setw(8) << std::left << agent_label
	                  << std::setw(34) << std::left << r.task.substr(0, 32)
	                  << std::setw(10) << format_idle_minutes(r.idle_min)
	                  << std::setw(10) << score.str()
	                  << trend.str()
	                  << "\n";
	    }
}

// ============ Swarm Task Management Commands ============

void cmd_init_tasks(const std::string& src_dir, const std::string& src_lang,
                    const std::string& tgt_dir, const std::string& tgt_lang,
                    const std::string& task_file, const std::string& agents_md = "") {
    std::cout << "=== Initializing Task File ===\n\n";

    // Scan both codebases
    Codebase source(src_dir, src_lang);
    source.scan();
    source.extract_imports();
    source.build_dependency_graph();

    Codebase target(tgt_dir, tgt_lang);
    target.scan();
    target.extract_porting_data();  // TODOs, lint, line counts (task inclusion needs this)

    CodebaseComparator comp(source, target);
    comp.find_matches();
    std::cout << "Computing AST similarities...\n";
    comp.compute_similarities();

    // Build task list from missing files
    TaskManager tm(task_file);
    tm.source_root = src_dir;
    tm.target_root = tgt_dir;
    tm.source_lang = src_lang;
    tm.target_lang = tgt_lang;
    tm.agents_md_path = agents_md;

    // Add missing files and unacceptable ports as tasks (sorted by dependents).
    // NOTE: TODOs/stubs are an automatic failure mode for "complete" status,
    // so they must stay in the queue even if similarity looks high.
    std::set<std::string> pending_keys;

    for (const auto& path : comp.unmatched_source) {
        pending_keys.insert(path);
    }

    for (const auto& m : comp.matches) {
        bool func_gap = (m.source_function_count > 0 &&
                         m.matched_function_count < m.source_function_count);
        if (m.similarity < 0.85f || m.todo_count > 0 || m.lint_count > 0 || m.is_stub || func_gap) {
            pending_keys.insert(m.source_path);
        }
    }

    std::vector<const SourceFile*> pending_files;
    pending_files.reserve(pending_keys.size());
    for (const auto& key : pending_keys) {
        pending_files.push_back(&source.files.at(key));
    }

    std::sort(pending_files.begin(), pending_files.end(),
              [](const SourceFile* a, const SourceFile* b) {
                  return a->dependent_count > b->dependent_count;
              });

    for (const auto* sf : pending_files) {
        PortTask task;
        task.source_path = sf->relative_path;
        task.source_qualified = sf->qualified_name;

        // mod.rs files are module wiring and should not be transliterated.
        if (std::filesystem::path(task.source_path).filename() == "mod.rs") {
            continue;
        }
        if (tm.source_lang == "rust" &&
            rust_is_module_wiring_only(std::filesystem::path(tm.source_root) / sf->relative_path)) {
            continue;
        }
        if (g_reexport_config.matches(sf->relative_path)) {
            continue;
        }

        // Generate expected target path
        std::string kt_path = sf->relative_path;
        if (tm.target_lang == "typescript") {
            std::filesystem::path rel(sf->relative_path);
            std::string filename = rel.stem().string();
            std::filesystem::path expected_rel = rel.parent_path() / (SourceFile::to_kebab_case(filename) + ".ts");
            kt_path = expected_rel.string();
        } else if (kt_path.size() > 3 && kt_path.substr(kt_path.size() - 3) == ".rs") {
            std::filesystem::path rel(sf->relative_path);
            std::string filename = rel.stem().string();

            // If there is a sibling Rust module directory with the same stem, treat this as a
            // module root and place the Kotlin file inside that directory.
            std::filesystem::path full_src = std::filesystem::path(tm.source_root) / rel;
            std::filesystem::path mod_dir = full_src.parent_path() / filename;
            bool has_rs_children = false;
            if (std::filesystem::exists(mod_dir) && std::filesystem::is_directory(mod_dir)) {
                for (const auto& entry : std::filesystem::directory_iterator(mod_dir)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".rs") {
                        has_rs_children = true;
                        break;
                    }
                }
            }

            if (has_rs_children) {
                std::filesystem::path kt_rel =
                    rel.parent_path() / filename / (SourceFile::to_pascal_case(filename) + ".kt");
                kt_path = kt_rel.string();
            } else {
                std::filesystem::path kt_rel =
                    rel.parent_path() / (SourceFile::to_pascal_case(filename) + ".kt");
                kt_path = kt_rel.string();
            }
        }
        // Remove src/ prefix if present
        if (kt_path.rfind("src/", 0) == 0) {
            kt_path = kt_path.substr(4);
        } else if (kt_path.rfind("include/", 0) == 0) {
            kt_path = kt_path.substr(8);
        }
        task.target_path = kt_path;
        task.dependent_count = sf->dependent_count;
        task.dependency_count = sf->dependency_count;

        // Build dependency list
        for (const auto& dep : sf->imports) {
            task.dependencies.push_back(dep.module_path);
        }

        tm.tasks.push_back(task);
    }

    tm.save();

    std::cout << "Generated " << tm.tasks.size() << " tasks\n";
    std::cout << "Task file: " << task_file << "\n";

    // Show top priority tasks
    std::cout << "\nTop 10 priority tasks:\n";
    int count = 0;
    for (const auto& t : tm.tasks) {
        if (count++ >= 10) break;
        std::cout << "  " << std::setw(30) << std::left << t.source_qualified
                  << " deps=" << t.dependent_count << "\n";
    }
}

void cmd_tasks(const std::string& task_file, int agent) {
    TaskManager tm(task_file);
    if (!tm.load()) {
        std::cerr << "Error: Could not load task file: " << task_file << "\n";
        return;
    }

    int pending, assigned, completed, blocked;
    tm.get_stats(pending, assigned, completed, blocked);

    print_agent_activity_section(tm, task_file, agent);

    std::cout << "=== Task Status ===\n\n";
    std::cout << "Task file: " << task_file << "\n";
    std::cout << "Source root: " << tm.source_root << "\n";
    std::cout << "Target root: " << tm.target_root << "\n\n";

    std::cout << "Status Summary:\n";
    std::cout << "  Pending:   " << pending << "\n";
    std::cout << "  Assigned:  " << assigned << "\n";
    std::cout << "  Completed: " << completed << "\n";
    std::cout << "  Blocked:   " << blocked << "\n";
    std::cout << "  Total:     " << tm.tasks.size() << "\n\n";

    if (assigned > 0) {
        std::cout << "Currently Assigned:\n";
        for (const auto& t : tm.tasks) {
            if (t.status == TaskStatus::ASSIGNED) {
                std::cout << "  " << std::setw(30) << std::left << t.source_qualified
                          << " -> " << t.assigned_to
                          << " (since " << t.assigned_at << ")\n";
            }
        }
        std::cout << "\n";
    }

    // Show pending tasks by priority
    std::cout << "Pending Tasks (by priority):\n";
    std::cout << std::setw(35) << std::left << "Source"
              << std::setw(10) << "Deps"
              << "Target Path\n";
    std::cout << std::string(70, '-') << "\n";

    int shown = 0;
    for (const auto& t : tm.tasks) {
        if (t.status == TaskStatus::PENDING) {
            if (shown++ >= 20) {
                std::cout << "... and " << (pending - 20) << " more\n";
                break;
            }
            std::cout << std::setw(35) << std::left << t.source_qualified.substr(0, 33)
                      << std::setw(10) << t.dependent_count
                      << t.target_path << "\n";
        }
    }
}

void cmd_assign(const std::string& task_file, int requested_agent, bool override_mode) {
    TaskManager tm(task_file);
    if (!tm.load()) {
        std::cerr << "Error: Could not load task file: " << task_file << "\n";
        return;
    }

    int agent = requested_agent;
    if (agent <= 0) {
        // Allocate a new agent number (monotonic) to avoid collisions/reuse confusion.
        int max_agent = 0;
        for (const auto& t : tm.tasks) {
            if (t.status != TaskStatus::ASSIGNED) continue;
            try {
                max_agent = std::max(max_agent, std::stoi(t.assigned_to));
            } catch (...) {
                // Ignore non-numeric IDs.
            }
        }
        auto dir = guardrails_agents_dir(task_file);
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
            if (ec) break;
            auto name = entry.path().filename().string();
            if (!name.starts_with("agent_")) continue;
            auto pos = name.find_first_of('.');
            auto num_str = name.substr(std::string("agent_").size(),
                                       pos == std::string::npos ? std::string::npos : pos - std::string("agent_").size());
            try {
                max_agent = std::max(max_agent, std::stoi(num_str));
            } catch (...) {
            }
        }
        agent = max_agent + 1;
    }

    if (agent <= 0) {
        std::cerr << "Error: Could not allocate agent number\n";
        return;
    }

    AgentLock agent_lock(task_file, agent, override_mode);
    if (!agent_lock.locked()) {
        int holder = agent_lock.holder_pid();
        std::ostringstream msg;
        msg << "Agent #" << agent << " appears to be in use";
        if (holder > 0) msg << " by pid " << holder;
        msg << ".\n"
            << "Re-run with --override to wait, or pick a different agent number.\n";
        std::cerr << "Error: " << msg.str();
        write_agent_notice(task_file, agent,
                           "Conflict: another PID attempted to use this agent number.\n" + msg.str());
        return;
    }

    print_and_clear_agent_notice(task_file, agent);
    touch_agent_last_used(task_file, agent);

    std::string agent_id = std::to_string(agent);

    // Check if agent already has an assigned task
    for (const auto& t : tm.tasks) {
        if (t.status == TaskStatus::ASSIGNED && t.assigned_to == agent_id) {
            std::cerr << "Agent #" << agent << " already has an assigned task: "
                      << t.source_qualified << "\n";
            std::cerr << "Complete it with: ast_distance --agent " << agent
                      << " --complete " << task_file
                      << " " << t.source_qualified << "\n";
            std::cerr << "Or release it with: ast_distance --agent " << agent
                      << " --release " << task_file
                      << " " << t.source_qualified << "\n";
            return;
        }
    }

    PortTask* task = tm.assign_next(agent_id);
    if (!task) {
        std::cout << "No pending tasks available.\n";

        int pending, assigned, completed, blocked;
        tm.get_stats(pending, assigned, completed, blocked);
        std::cout << "\nStatus: " << completed << "/" << tm.tasks.size() << " completed, "
                  << assigned << " assigned, " << pending << " pending\n";
        return;
    }

    tm.save();

    print_agent_activity_section(tm, task_file, agent);

    // Print full assignment details
    tm.print_assignment(*task, agent);
}

void cmd_complete(const std::string& task_file, const std::string& source_qualified,
                  int agent, bool override_mode) {
    TaskManager tm(task_file);
    if (!tm.load()) {
        std::cerr << "Error: Could not load task file: " << task_file << "\n";
        return;
    }

    const PortTask* task_ptr = nullptr;
    for (const auto& t : tm.tasks) {
        if (t.source_qualified == source_qualified) {
            task_ptr = &t;
            break;
        }
    }
    if (!task_ptr) {
        std::cerr << "Task not found: " << source_qualified << "\n";
        return;
    }

    if (agent > 0 && task_ptr->status == TaskStatus::ASSIGNED &&
        task_ptr->assigned_to != std::to_string(agent) && !override_mode) {
        std::cerr << "Error: Agent #" << agent << " cannot complete a task assigned to agent "
                  << task_ptr->assigned_to << "\n";
        std::cerr << "Use --override only if you are explicitly taking over the task.\n";
        return;
    }

    std::filesystem::path source_path = std::filesystem::path(tm.source_root) / task_ptr->source_path;

    // Kotlin Multiplatform convention: tests belong in src/commonTest, not src/commonMain.
    // Task files may only store a single target_root, so adjust dynamically for Rust test modules.
    std::string effective_target_root = tm.target_root;
    bool is_test_task = false;
    if (tm.target_lang == "kotlin") {
        const std::string& p = task_ptr->source_path;

        // Rust tests may live in:
        // - `tests/...`
        // - `src/tests/...`
        // - `src/**/tests.rs` (module-level tests)
        // - `src/tests.rs`
        if (p == "tests.rs") is_test_task = true;
        if (!is_test_task && p.size() >= 9 && p.compare(p.size() - 9, 9, "/tests.rs") == 0) is_test_task = true;
        if (!is_test_task && p.find("/tests/") != std::string::npos) is_test_task = true;
        if (!is_test_task && p.rfind("tests/", 0) == 0) is_test_task = true;

        if (is_test_task) {
            auto replace_all = [&](const std::string& from, const std::string& to) {
                size_t pos = 0;
                while ((pos = effective_target_root.find(from, pos)) != std::string::npos) {
                    effective_target_root.replace(pos, from.size(), to);
                    pos += to.size();
                }
            };
            replace_all("/src/commonMain/", "/src/commonTest/");
            replace_all("src/commonMain/", "src/commonTest/");
        }
    }

    std::filesystem::path target_path = std::filesystem::path(effective_target_root) / task_ptr->target_path;

    // Some task generators may record Kotlin target paths using the Rust filename stem
    // (e.g. `typing/ty.kt`) even though the Kotlin port convention is PascalCase filenames
    // (e.g. `typing/Ty.kt`). Treat this as a tool-level false negative: if the recorded
    // target path doesn't exist, try the PascalCase variant before failing.
    std::filesystem::path resolved_target_path = target_path;
    if (!std::filesystem::exists(resolved_target_path) &&
        tm.target_lang == "kotlin" &&
        !override_mode) {
        // Module root mapping: Rust `foo.rs` commonly corresponds to Kotlin package dir `foo/`
        // with a `Foo.kt` file inside (mirrors `foo/mod.rs` layout).
        auto sp = std::filesystem::path(task_ptr->source_path);
        if (sp.extension() == ".rs") {
            std::string mod = sp.stem().string();
            auto candidate = std::filesystem::path(effective_target_root) /
                             sp.parent_path() / mod / (SourceFile::to_pascal_case(mod) + ".kt");
            if (std::filesystem::exists(candidate)) {
                resolved_target_path = candidate;
            }
        }

        auto fname = resolved_target_path.filename().string();
        if (fname.size() > 3 && fname.substr(fname.size() - 3) == ".kt") {
            std::string stem = fname.substr(0, fname.size() - 3);
            std::string pascal = SourceFile::to_pascal_case(stem);
            auto candidate = resolved_target_path.parent_path() / (pascal + ".kt");
            if (std::filesystem::exists(candidate)) {
                resolved_target_path = candidate;
            }
        }
    }

    if (!std::filesystem::exists(resolved_target_path) && !override_mode) {
        std::cerr << "Error: Cannot complete task - target file does not exist: " << target_path.string() << "\n";
        std::cerr << "Create the file and add a port-lint header first.\n";
        return;
    }

    if (std::filesystem::exists(resolved_target_path)) {
        FileStats stats = PortingAnalyzer::analyze_file(resolved_target_path.string());
        if (!stats.todos.empty() && !override_mode) {
            std::cerr << "Error: Cannot complete task - target file contains TODO markers\n";
            std::cerr << "TODOs are an automatic failure mode for porting completeness.\n";
            std::cerr << "Complete the TODOs or remove the file.\n";
            return;
        }
        if (stats.is_stub && !override_mode) {
            std::cerr << "Error: Cannot complete task - target file is detected as a stub\n";
            std::cerr << "Complete the real implementation before marking complete.\n";
            return;
        }

        // Require no stub bodies and high similarity.
        Language src_lang = parse_language(tm.source_lang);
        Language tgt_lang = parse_language(tm.target_lang);
        warn_kotlin_suspicious_constructs(source_path, src_lang, resolved_target_path, tgt_lang);

        ASTParser parser;
        bool has_stubs = parser.has_stub_bodies_in_files({resolved_target_path.string()}, tgt_lang);
        if (has_stubs && !override_mode) {
            std::cerr << "Error: Cannot complete task - target file contains stub/TODO markers in function bodies\n";
            std::cerr << "The code is fake. Complete the real implementation before marking complete.\n";
            return;
        }

        if (tgt_lang == Language::KOTLIN) {
            auto contamination =
                CodebaseComparator::kotlin_contamination_reasons_for_files({resolved_target_path.string()});
            if (!contamination.empty() && !override_mode) {
                std::cerr << "Error: Cannot complete task - Kotlin target failed cheat detection\n";
                print_cheat_detection_failure(
                    std::cerr,
                    "Target file score forced to 0.0000.",
                    contamination,
                    "The task cannot be marked complete because function-by-function scoring is invalidated.",
                    "Remove Rust syntax, snake_case identifiers, score-padding suppressions, and translator-note residue before marking complete.");
                return;
            }
        }

        std::optional<std::string> source_contents;
        if (src_lang == Language::RUST && tgt_lang == Language::KOTLIN) {
            source_contents = strip_rust_cfg_test_blocks(read_file_to_string(source_path.string()));
        }

        float similarity = 0.0f;
        try {
            auto src_functions = source_contents ? parser.extract_function_infos(*source_contents, src_lang)
                                                : parser.extract_function_infos_from_file(source_path.string(), src_lang);
            auto tgt_functions = parser.extract_function_infos_from_file(resolved_target_path.string(), tgt_lang);
            auto fn_result = CodebaseComparator::compare_function_sets(src_functions, tgt_functions);
            if (src_functions.empty() || tgt_functions.empty()) {
                similarity = 0.0f;
            } else if (fn_result.score >= 0.0f) {
                similarity = fn_result.score;
            } else {
                similarity = 0.0f;
            }
        } catch (const std::exception& e) {
            std::cerr << "Warning: function-level comparison failed during --complete: "
                      << e.what() << "\n";
            similarity = 0.0f;
        } catch (...) {
            std::cerr << "Warning: function-level comparison failed during --complete (unknown error)\n";
            similarity = 0.0f;
        }
        float kMinSimilarity = is_test_task ? 0.75f : 0.85f;
        const float kEpsilon = 1e-4f; // numeric stability for float blends
        if (similarity < (kMinSimilarity - kEpsilon) && !override_mode) {
            std::cerr << "Error: Cannot complete task with low function similarity: " << similarity << "\n";
            std::cerr << "Target exists but function bodies/parameters do not match source.\n";
            std::cerr << "Complete the port (aim for >= " << kMinSimilarity << ") or use --override.\n";
            return;
        }
    }

    if (!tm.complete_task(source_qualified)) {
        std::cerr << "Task not found: " << source_qualified << "\n";
        return;
    }

    std::cout << "Marked as completed: " << source_qualified << "\n";

    // Rescan to update priorities based on new state
    std::cout << "Rescanning codebases to update priorities...\n";

    if (tm.source_root.empty() || tm.source_lang.empty()) {
        std::cerr << "Warning: Task file missing source/target info, cannot rescan.\n";
        tm.save();
        return;
    }

    // Scan both codebases
    Codebase source(tm.source_root, tm.source_lang);
    source.scan();
    source.extract_imports();
    source.build_dependency_graph();

    Codebase target(tm.target_root, tm.target_lang);
    target.scan();
    target.extract_porting_data();  // TODOs, lint, line counts (task inclusion needs this)

    CodebaseComparator comp(source, target);
    comp.find_matches();
    comp.compute_similarities();

    // Build set of currently assigned task qualified names
    std::set<std::string> assigned_tasks;
    std::map<std::string, std::string> assigned_to_map;
    std::map<std::string, std::string> assigned_at_map;
    for (const auto& t : tm.tasks) {
        if (t.status == TaskStatus::ASSIGNED) {
            assigned_tasks.insert(t.source_qualified);
            assigned_to_map[t.source_qualified] = t.assigned_to;
            assigned_at_map[t.source_qualified] = t.assigned_at;
        }
    }

    // Build set of completed tasks
    std::set<std::string> completed_tasks;
    std::map<std::string, std::string> completed_at_map;
    for (const auto& t : tm.tasks) {
        if (t.status == TaskStatus::COMPLETED) {
            completed_tasks.insert(t.source_qualified);
            completed_at_map[t.source_qualified] = t.completed_at;
        }
    }

    // Rebuild task list from missing files
    tm.tasks.clear();

    std::set<std::string> pending_keys;
    for (const auto& path : comp.unmatched_source) {
        pending_keys.insert(path);
    }
    for (const auto& m : comp.matches) {
        bool func_gap = (m.source_function_count > 0 &&
                         m.matched_function_count < m.source_function_count);
        if (m.similarity < 0.85f || m.todo_count > 0 || m.lint_count > 0 || m.is_stub || func_gap) {
            pending_keys.insert(m.source_path);
        }
    }

    std::vector<const SourceFile*> missing;
    missing.reserve(pending_keys.size());
    for (const auto& key : pending_keys) {
        missing.push_back(&source.files.at(key));
    }

    std::sort(missing.begin(), missing.end(),
        [](const SourceFile* a, const SourceFile* b) {
            return a->dependent_count > b->dependent_count;
        });

    for (const auto* sf : missing) {
        // Skip files that are marked as completed (agent said they finished,
        // but file may not have been detected yet or similarity check pending)
        if (completed_tasks.count(sf->qualified_name)) {
            continue;
        }

        PortTask task;
        task.source_path = sf->relative_path;
        task.source_qualified = sf->qualified_name;

        // mod.rs files are module wiring and should not be transliterated.
        if (std::filesystem::path(task.source_path).filename() == "mod.rs") {
            continue;
        }
        if (tm.source_lang == "rust" &&
            rust_is_module_wiring_only(std::filesystem::path(tm.source_root) / sf->relative_path)) {
            continue;
        }
        if (g_reexport_config.matches(sf->relative_path)) {
            continue;
        }

        // Generate expected target path
        std::string kt_path = sf->relative_path;
        if (tm.target_lang == "typescript") {
            std::filesystem::path rel(sf->relative_path);
            std::string filename = rel.stem().string();
            std::filesystem::path expected_rel = rel.parent_path() / (SourceFile::to_kebab_case(filename) + ".ts");
            kt_path = expected_rel.string();
        } else if (kt_path.size() > 3 && kt_path.substr(kt_path.size() - 3) == ".rs") {
            // Convert filename from snake_case to PascalCase for Kotlin
            std::filesystem::path rel(sf->relative_path);
            std::string filename = rel.stem().string();

            // If there is a sibling Rust module directory with the same stem, treat this as a
            // module root and place the Kotlin file inside that directory.
            std::filesystem::path full_src = std::filesystem::path(tm.source_root) / rel;
            std::filesystem::path mod_dir = full_src.parent_path() / filename;
            bool has_rs_children = false;
            if (std::filesystem::exists(mod_dir) && std::filesystem::is_directory(mod_dir)) {
                for (const auto& entry : std::filesystem::directory_iterator(mod_dir)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".rs") {
                        has_rs_children = true;
                        break;
                    }
                }
            }

            if (has_rs_children) {
                std::filesystem::path kt_rel =
                    rel.parent_path() / filename / (SourceFile::to_pascal_case(filename) + ".kt");
                kt_path = kt_rel.string();
            } else {
                std::filesystem::path kt_rel =
                    rel.parent_path() / (SourceFile::to_pascal_case(filename) + ".kt");
                kt_path = kt_rel.string();
            }
        }
        if (kt_path.rfind("src/", 0) == 0) {
            kt_path = kt_path.substr(4);
        } else if (kt_path.rfind("include/", 0) == 0) {
            kt_path = kt_path.substr(8);
        }
        task.target_path = kt_path;
        task.dependent_count = sf->dependent_count;
        task.dependency_count = sf->dependency_count;

        // Restore assignment status if it was assigned
        if (assigned_tasks.count(sf->qualified_name)) {
            task.status = TaskStatus::ASSIGNED;
            task.assigned_to = assigned_to_map[sf->qualified_name];
            task.assigned_at = assigned_at_map[sf->qualified_name];
        }

        for (const auto& dep : sf->imports) {
            task.dependencies.push_back(dep.module_path);
        }

        tm.tasks.push_back(task);
    }

    // Add completed tasks back (preserving their info from original task list)
    // Note: completed tasks may still be in unmatched_source if file hasn't been created yet,
    // but we skipped them above. They'll be removed from pending once file actually exists.
    for (const auto& qualified : completed_tasks) {
        // Find in source codebase for full info
        bool found = false;
        for (const auto& [path, sf] : source.files) {
            if (sf.qualified_name == qualified) {
                PortTask task;
                task.source_path = sf.relative_path;
                task.source_qualified = sf.qualified_name;
                task.dependent_count = sf.dependent_count;
                task.dependency_count = sf.dependency_count;

                // Restore expected target path so future --complete/--release checks can resolve it.
                std::string kt_path = sf.relative_path;
                if (tm.target_lang == "typescript") {
                    std::filesystem::path rel(sf.relative_path);
                    std::string filename = rel.stem().string();
                    std::filesystem::path expected_rel = rel.parent_path() / (SourceFile::to_kebab_case(filename) + ".ts");
                    kt_path = expected_rel.string();
                } else if (kt_path.size() > 3 && kt_path.substr(kt_path.size() - 3) == ".rs") {
                    std::filesystem::path rel(sf.relative_path);
                    std::string filename = rel.stem().string();

                    std::filesystem::path full_src = std::filesystem::path(tm.source_root) / rel;
                    std::filesystem::path mod_dir = full_src.parent_path() / filename;
                    bool has_rs_children = false;
                    if (std::filesystem::exists(mod_dir) && std::filesystem::is_directory(mod_dir)) {
                        for (const auto& entry : std::filesystem::directory_iterator(mod_dir)) {
                            if (entry.is_regular_file() && entry.path().extension() == ".rs") {
                                has_rs_children = true;
                                break;
                            }
                        }
                    }

                    if (has_rs_children) {
                        std::filesystem::path kt_rel =
                            rel.parent_path() / filename / (SourceFile::to_pascal_case(filename) + ".kt");
                        kt_path = kt_rel.string();
                    } else {
                        std::filesystem::path kt_rel =
                            rel.parent_path() / (SourceFile::to_pascal_case(filename) + ".kt");
                        kt_path = kt_rel.string();
                    }
                }
                if (kt_path.rfind("src/", 0) == 0) {
                    kt_path = kt_path.substr(4);
                } else if (kt_path.rfind("include/", 0) == 0) {
                    kt_path = kt_path.substr(8);
                }
                task.target_path = kt_path;

                for (const auto& dep : sf.imports) {
                    task.dependencies.push_back(dep.module_path);
                }
                task.status = TaskStatus::COMPLETED;
                task.completed_at = completed_at_map[qualified];
                tm.tasks.push_back(task);
                found = true;
                break;
            }
        }
        // If not found in source (maybe renamed/removed), still track it
        if (!found) {
            PortTask task;
            task.source_path = qualified; // Use qualified name as path fallback
            task.source_qualified = qualified;
            task.status = TaskStatus::COMPLETED;
            task.completed_at = completed_at_map[qualified];
            tm.tasks.push_back(task);
        }
    }

    tm.save();

    int pending, assigned, completed, blocked;
    tm.get_stats(pending, assigned, completed, blocked);
    std::cout << "Progress: " << completed << "/" << (pending + assigned + completed) << " completed\n";
    std::cout << "Remaining: " << pending << " pending, " << assigned << " assigned\n";

    // Show updated top priorities
    std::cout << "\nUpdated top priorities:\n";
    int shown = 0;
    for (const auto& t : tm.tasks) {
        if (t.status == TaskStatus::PENDING && shown++ < 5) {
            std::cout << "  " << std::setw(30) << std::left << t.source_qualified
                      << " deps=" << t.dependent_count << "\n";
        }
    }
}

void cmd_release(const std::string& task_file, const std::string& source_qualified,
                 int agent, bool override_mode) {
    TaskManager tm(task_file);
    if (!tm.load()) {
        std::cerr << "Error: Could not load task file: " << task_file << "\n";
        return;
    }

    // Find the task
    auto it = std::find_if(tm.tasks.begin(), tm.tasks.end(),
        [&](const PortTask& t) { return t.source_qualified == source_qualified; });
    
    if (it == tm.tasks.end() || it->status != TaskStatus::ASSIGNED) {
        std::cerr << "Task not found or not assigned: " << source_qualified << "\n";
        return;
    }

    if (agent > 0 && it->assigned_to != std::to_string(agent) && !override_mode) {
        std::cerr << "Error: Agent #" << agent << " cannot release a task assigned to agent "
                  << it->assigned_to << "\n";
        std::cerr << "Use --override only if you are explicitly taking over the task.\n";
        return;
    }
    
    // Check if target file exists - if so, require completion or deletion
    std::filesystem::path target_path = std::filesystem::path(tm.target_root) / it->target_path;
    if (std::filesystem::exists(target_path)) {
        std::filesystem::path source_path = std::filesystem::path(tm.source_root) / it->source_path;
        
        // Try to compute similarity - if we can't even parse, that's a HARD FAIL
        std::cerr << "Error: Cannot release task - target file already exists: " << target_path.string() << "\n";
        std::cerr << "Checking similarity...\n";

        FileStats stats = PortingAnalyzer::analyze_file(target_path.string());
        if (!stats.todos.empty() && !override_mode) {
            std::cerr << "Error: Cannot release task - target file contains TODO markers\n";
            std::cerr << "TODOs are an automatic failure mode for porting completeness.\n";
            std::cerr << "Complete the TODOs or delete the file to release.\n";
            return;
        }
        if (stats.is_stub && !override_mode) {
            std::cerr << "Error: Cannot release task - target file is detected as a stub\n";
            std::cerr << "Complete the real implementation or delete the file to release.\n";
            return;
        }
        
        // Parse and compare
        Language src_lang = parse_language(tm.source_lang);
        Language tgt_lang = parse_language(tm.target_lang);
        
        ASTParser parser;
        bool has_stubs = false;
        float similarity = 0.0f;

        // Require no stub bodies (e.g., Kotlin TODO(), Rust unimplemented!(), Python pass/...)
        has_stubs = has_stubs || parser.has_stub_bodies_in_files({target_path.string()}, tgt_lang);
        if (has_stubs) {
            std::cerr << "Error: Cannot release task - target file contains stub/TODO markers in function bodies\n";
            std::cerr << "The code is fake. Complete the real implementation or delete the file.\n";
            return;
        }

        if (tgt_lang == Language::KOTLIN) {
            auto contamination =
                CodebaseComparator::kotlin_contamination_reasons_for_files({target_path.string()});
            if (!contamination.empty()) {
                std::cerr << "Error: Cannot release task - Kotlin target failed cheat detection\n";
                print_cheat_detection_failure(
                    std::cerr,
                    "Target file score forced to 0.0000.",
                    contamination,
                    "The task cannot be released while the existing target invalidates function-by-function scoring.",
                    "Remove Rust syntax, snake_case identifiers, score-padding suppressions, translator-note residue, or delete the file.");
                return;
            }
        }

        auto src_functions = parser.extract_function_infos_from_file(
            source_path.string(), src_lang);
        auto tgt_functions = parser.extract_function_infos_from_file(
            target_path.string(), tgt_lang);
        auto fn_result = CodebaseComparator::compare_function_sets(src_functions, tgt_functions);
        similarity = fn_result.score >= 0.0f ? fn_result.score : 0.0f;

        // Require >= 0.50 function-by-function similarity to release.
        if (similarity < 0.50f) {
            std::cerr << "Error: Cannot release task with low function similarity: " << similarity << "\n";
            std::cerr << "Target file exists but function bodies/parameters do not match source\n";
            std::cerr << "Either complete the port or delete the target file to release.\n";
            return;
        }
        
        std::cerr << "Warning: Releasing with partial port (function similarity " << similarity << ")\n";
        std::cerr << "Consider completing it instead (use --complete).\n";
    }

    if (tm.release_task(source_qualified)) {
        tm.save();
        std::cout << "Released task: " << source_qualified << "\n";
    }
}

int main(int argc, char* argv[]) {
    setvbuf(stdout, nullptr, _IOLBF, 0);

    int agent = 0;
    bool override_mode = false;
    std::string task_file_flag;

    std::vector<std::string> rest;
    rest.reserve(static_cast<size_t>(argc));
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--agent" && i + 1 < argc) {
            agent = std::stoi(argv[++i]);
        } else if (arg == "--task-file" && i + 1 < argc) {
            task_file_flag = argv[++i];
        } else if (arg == "--override") {
            override_mode = true;
        } else {
            rest.push_back(arg);
        }
    }

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
    if (int redirect_status = reject_redirected_comparison_output_if_needed(mode, argc);
        redirect_status != 0) {
        return redirect_status;
    }

    // Guardrails: if a task system exists, require --agent and lock the session number.
    GuardrailsContext guard;
    guard.agent = agent;
    guard.override_mode = override_mode;

    if (!task_file_flag.empty()) {
        guard.task_file = task_file_flag;
    } else if ((mode == "--tasks" || mode == "--assign" || mode == "--complete" ||
                mode == "--release") &&
               argc >= 3) {
        guard.task_file = argv[2];
    } else if (mode == "--init-tasks" && argc >= 7) {
        guard.task_file = argv[6];
    } else if (std::filesystem::exists("tasks.json")) {
        guard.task_file = "tasks.json";
    }

    cull_stale_task_file_if_needed(guard.task_file, mode, agent);

    if (!guard.task_file.empty()) {
        TaskManager tm(guard.task_file);
        guard.active = tm.load();
    }

    if (guard.active && mode != "--assign" && agent <= 0) {
        std::cerr << "Error: task system detected (" << guard.task_file << ").\n";
        std::cerr << "All commands require an agent session number.\n";
        std::cerr << "Get one with: ast_distance --assign " << guard.task_file << "\n";
        std::cerr << "Then re-run with: ast_distance --agent <number> ...\n";
        return 2;
    }

    std::unique_ptr<AgentLock> agent_lock;
    if (guard.active && mode != "--assign" && agent > 0) {
        agent_lock = std::make_unique<AgentLock>(guard.task_file, agent, override_mode);
        if (!agent_lock->locked()) {
            int holder = agent_lock->holder_pid();
            std::ostringstream msg;
            msg << "Agent #" << agent << " appears to be in use";
            if (holder > 0) msg << " by pid " << holder;
            msg << ".\n"
                << "Re-run with --override to wait, or pick a different agent number.\n";
            std::cerr << "Error: " << msg.str();
            write_agent_notice(guard.task_file, agent,
                               "Conflict: another PID attempted to use this agent number.\n" +
                                   msg.str());
            return 3;
        }
        print_and_clear_agent_notice(guard.task_file, agent);
        touch_agent_last_used(guard.task_file, agent);
    }

    g_guardrails = guard;

    try {
        if (mode == "--scan" && argc >= 4) {
            cmd_scan(argv[2], argv[3]);

        } else if (mode == "--deps" && argc >= 4) {
            cmd_deps(argv[2], argv[3]);

        } else if (mode == "--rank") {
            std::cerr << "Error: --rank has been removed because it hides parts of the full report.\n"
                      << "Use --deep <src_dir> <src_lang> <tgt_dir> <tgt_lang> instead.\n";
            return 2;

        } else if (mode == "--deep" && argc >= 6) {
            cmd_deep(argv[2], argv[3], argv[4], argv[5]);
            write_missing_config_after_comparison({argv[2], argv[3]}, {argv[4], argv[5]});

        } else if (mode == "--numpy-mlx" && argc >= 4) {
            cmd_numpy_mlx(argv[2], argv[3]);

        } else if (mode == "--emberlint" && argc >= 3) {
            cmd_emberlint(argv[2]);

        } else if (mode == "--missing") {
            std::cerr << "Error: --missing has been removed because it hides parts of the full report.\n"
                      << "Use --deep <src_dir> <src_lang> <tgt_dir> <tgt_lang> instead.\n";
            return 2;

        } else if (mode == "--todos" && argc >= 3) {
            bool verbose = true;
            if (argc >= 4 && std::string(argv[3]) == "--summary") {
                verbose = false;
            }
            cmd_todos(argv[2], verbose);

        } else if (mode == "--lint" && argc >= 3) {
            cmd_lint(argv[2]);

        } else if (mode == "--stats" && argc >= 3) {
            cmd_stats(argv[2]);

        } else if (mode == "--symbols" && argc >= 4) {
            SymbolAnalysisOptions options;
            cmd_symbols(argv[2], argv[3], options);

        } else if (mode == "--symbols-duplicates" && argc >= 4) {
            SymbolAnalysisOptions options;
            options.duplicates = true;
            cmd_symbols(argv[2], argv[3], options);

        } else if (mode == "--symbols-stubs" && argc >= 4) {
            SymbolAnalysisOptions options;
            options.stubs = true;
            cmd_symbols(argv[2], argv[3], options);

        } else if (mode == "--symbols-symbol" && argc >= 5) {
            SymbolAnalysisOptions options;
            options.symbol = argv[4];
            if (argc >= 6 && std::string(argv[5]) == "--json") {
                options.json = true;
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
                if (arg == "--json") {
                    options.json = true;
                } else if (arg == "--verbose" || arg == "-v") {
                    options.verbose = true;
                } else if (arg == "--missing-only" || arg == "--missing") {
                    options.missing_only = true;
                } else if (arg == "--include-stubs") {
                    options.include_stubs = true;
                } else if (arg == "--kind" && i + 1 < argc) {
                    options.filter_kind = argv[++i];
                } else if (arg == "--file" && i + 1 < argc) {
                    options.filter_file = argv[++i];
                }
            }
            cmd_symbol_parity(src_root, tgt_root, options);
            write_missing_config_after_comparison(
                {src_root, language_config_name(options.source_lang)},
                {tgt_root, language_config_name(options.target_lang)});

        } else if (mode == "--import-map" && argc >= 3) {
            ImportMapOptions options;
            for (int i = 3; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg == "--json") {
                    options.json = true;
                } else if (arg == "--summary") {
                    options.summary_only = true;
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
                if (arg == "--json") {
                    options.json = true;
                } else if (arg == "--verbose" || arg == "-v") {
                    options.verbose = true;
                } else if (arg == "--file" && i + 1 < argc) {
                    options.filter_file = argv[++i];
                } else if (arg == "--min" && i + 1 < argc) {
                    options.min_errors = std::stoi(argv[++i]);
                }
            }
            cmd_compiler_fixup(argv[2], argv[3], options);

        // Swarm task management commands
        } else if (mode == "--init-tasks" && argc >= 7) {
            std::string agents_md = (argc >= 8) ? argv[7] : "";
            cmd_init_tasks(argv[2], argv[3], argv[4], argv[5], argv[6], agents_md);

        } else if (mode == "--tasks" && argc >= 3) {
            cmd_tasks(argv[2], agent);

        } else if (mode == "--assign" && argc >= 3) {
            int requested_agent = 0;
            if (argc >= 4) {
                requested_agent = std::stoi(argv[3]);
            } else if (agent > 0) {
                requested_agent = agent;
            }
            cmd_assign(argv[2], requested_agent, override_mode);

        } else if (mode == "--complete" && argc >= 4) {
            cmd_complete(argv[2], argv[3], agent, override_mode);

        } else if (mode == "--release" && argc >= 4) {
            cmd_release(argv[2], argv[3], agent, override_mode);

        } else if (mode == "--dump-node" && argc >= 4) {
            ASTParser parser;
            std::string filepath = argv[2];
            Language lang = parse_language(argv[3]);
            TreePtr tree = parser.parse_file(filepath, lang);
            if (tree) {
                dump_tree(tree.get(), 0);
            }
            return 0;

        } else if (mode == "--dump" && argc >= 4) {
            ASTParser parser;
            std::string filepath = argv[2];
            std::string lang_str = argv[3];
            Language lang;
            if (lang_str == "rust") lang = Language::RUST;
            else if (lang_str == "cpp") lang = Language::CPP;
            else if (lang_str == "python") lang = Language::PYTHON;
            else lang = Language::KOTLIN;

            std::cout << "Parsing " << filepath << " as " << lang_str << "...\n\n";
            TreePtr tree = parser.parse_file(filepath, lang);

            std::cout << "AST Structure:\n";
            dump_tree(tree.get());

            std::cout << "\n";
            auto hist = tree->node_type_histogram(ASTSimilarity::NUM_NODE_TYPES);
            print_histogram(hist);

            std::cout << "\nTree Statistics:\n";
            std::cout << "  Size:  " << tree->size() << " nodes\n";
            std::cout << "  Depth: " << tree->depth() << "\n";

        } else if (mode == "--compare-functions" && argc >= 6) {
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
            auto funcs1 = parser.extract_function_infos(file1_text, lang1);
            auto reexport_hints = rust_mod_reexport_hints(file1, file1_text, lang1, lang2);

            std::cout << "Found " << funcs1.size() << " " << language_name(lang1) << " functions\n";

            std::cout << "\nExtracting functions from " << file2 << " (" << language_name(lang2) << ")...\n";
            std::ifstream stream2(file2);
            std::stringstream buffer2;
            buffer2 << stream2.rdbuf();
            std::string file2_text = buffer2.str();
            auto funcs2 = parser.extract_function_infos(file2_text, lang2);

            std::cout << "Found " << funcs2.size() << " " << language_name(lang2) << " functions\n";

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
                int source_index = 0;
                int target_index = 0;
            };

            std::unordered_map<std::string, std::vector<int>> target_by_name;
            target_by_name.reserve(funcs2.size());
            for (int j = 0; j < static_cast<int>(funcs2.size()); ++j) {
                target_by_name[target_lookup_key(funcs2[j])].push_back(j);
            }

            std::vector<FunctionMatchCandidate> candidates;
            for (int i = 0; i < static_cast<int>(funcs1.size()); ++i) {
                auto bucket = target_by_name.find(expected_lookup_key(funcs1[i]));
                if (bucket == target_by_name.end()) {
                    continue;
                }
                for (int j : bucket->second) {
                    candidates.push_back({
                        guarded_combined_similarity(funcs1[i], funcs2[j]),
                        guarded_ast_cosine(funcs1[i], funcs2[j]),
                        guarded_identifier_cosine(funcs1[i], funcs2[j]),
                        i,
                        j,
                    });
                }
            }

            std::sort(candidates.begin(), candidates.end(),
                [](const FunctionMatchCandidate& a, const FunctionMatchCandidate& b) {
                    return a.score > b.score;
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
                bool stub_guarded = false;
            };

            std::vector<FunctionPairReport> reports;
            reports.reserve(std::min(funcs1.size(), funcs2.size()));
            float total_combined = 0.0f;
            float total_ast_cosine = 0.0f;
            float total_id_cosine = 0.0f;
            float total_line_balance = 0.0f;

            for (const auto& candidate : candidates) {
                if (source_used[candidate.source_index] || target_used[candidate.target_index]) {
                    continue;
                }
                const auto& source_func = funcs1[candidate.source_index];
                const auto& target_func = funcs2[candidate.target_index];
                source_used[candidate.source_index] = true;
                target_used[candidate.target_index] = true;

                FunctionPairReport report;
                report.source_name = source_func.name;
                report.expected_name = expected_target_function_name(source_func.name, lang1, lang2);
                report.target_name = target_func.name;
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
                report.stub_guarded = target_func.has_stub_markers && !source_func.has_stub_markers;
                reports.push_back(report);

                total_combined += report.combined;
                total_ast_cosine += report.ast_cosine;
                total_id_cosine += report.id_cosine;
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
            const float coverage = source_total > 0.0f ? matched_total / source_total : 0.0f;
            const float overall_score = source_total > 0.0f ? total_combined / source_total : 0.0f;
            const float matched_avg = matched_total > 0.0f ? total_combined / matched_total : 0.0f;
            const float ast_avg = matched_total > 0.0f ? total_ast_cosine / matched_total : 0.0f;
            const float id_avg = matched_total > 0.0f ? total_id_cosine / matched_total : 0.0f;
            const float line_balance_avg = matched_total > 0.0f ? total_line_balance / matched_total : 0.0f;

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
            std::cout << "Strict matched pairs: " << reports.size() << " / " << funcs1.size()
                      << " source functions";
            std::cout << " | unmatched source: " << unmatched_source
                      << " | unmatched target: " << unmatched_target << "\n";
            std::cout << "Name coverage: " << std::fixed << std::setprecision(3) << coverage << "\n";
            std::cout << "Function body score (missing source functions count as 0): "
                      << std::fixed << std::setprecision(3) << overall_score << "\n";
            if (funcs1.empty()) {
                std::cout << "Function comparison failed: no source function bodies were extracted.\n";
            } else if (funcs2.empty()) {
                std::cout << "Function comparison failed: no target function bodies were extracted.\n";
            } else if (unmatched_source > 0) {
                std::cout << "Function comparison incomplete: " << unmatched_source
                          << " source functions were not matched by strict parity.\n";
            }
            std::cout << "Matched average combined: " << std::fixed << std::setprecision(3) << matched_avg
                      << " | AST cosine avg: " << ast_avg
                      << " | identifier cosine avg: " << id_avg
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
                        std::cout << "  - " << funcs1[i].name
                                  << " -> expected " << expected_target_function_name(funcs1[i].name, lang1, lang2)
                                  << " (" << funcs1[i].line_count << " lines)\n";
                    }
                }
            }

            if (unmatched_target > 0) {
                std::cout << "\nExtra target functions not matched by strict source-name parity:\n";
                for (int j = 0; j < static_cast<int>(funcs2.size()); ++j) {
                    if (!target_used[j]) {
                        std::cout << "  - " << funcs2[j].name
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
                      << "\tSrcNodes\tTargetNodes\tASTCosine\tIdentifierCosine\tCombined\tStubGuard\n";
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
                          << report.combined << "\t"
                          << (report.stub_guarded ? "yes" : "no") << "\n";
            }

            // Per-function parity is 1:1 — each Rust function paired with its
            // strict-name Kotlin counterpart, one line per pair. The
            // unmatched-source / unmatched-target lists above already surface
            // the gaps. An N x M Cartesian product of "every Rust function vs
            // every Kotlin function" is not a faithfulness check and is not
            // emitted: this is a gap tester, not a cross-similarity explorer.

            write_missing_config_after_comparison(
                {file1, language_config_name(lang1)},
                {file2, language_config_name(lang2)});

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
            auto report = ASTSimilarity::compare(tree1.get(), tree2.get(), macro_friendly);
            report.print();

            std::string text1 = file1_source ? *file1_source : read_file_to_string(file1);
            std::string text2 = file2_source ? *file2_source : read_file_to_string(file2);
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
                    std::cout << "Function comparison failed: no source function bodies were extracted.\n";
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
            write_missing_config_after_comparison(
                {file1, language_config_name(lang1)},
                {file2, language_config_name(lang2)});

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
