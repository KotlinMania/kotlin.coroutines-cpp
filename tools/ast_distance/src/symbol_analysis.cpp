#include "symbol_analysis.hpp"
#include "porting_utils.hpp"
#include <regex>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <set>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

namespace ast_distance {

namespace {

struct CppClassDef {
    std::string name;
    std::string kind;
    std::string file;
    int line = 0;
    bool is_stub = false;
    std::string stub_reason;
};

struct StubItem {
    std::string file;
    std::string type;
    std::string name;
    int line = 0;
    std::string reason;
};

std::string read_file(const fs::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) return {};
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string remove_cpp_comments(const std::string& content) {
    std::string clean = std::regex_replace(content, std::regex(R"(//[^\n]*)"), "");
    clean = std::regex_replace(clean, std::regex(R"(/\*[\s\S]*?\*/)"), "");
    return clean;
}

std::string extract_class_body(const std::string& content, size_t start_pos) {
    if (start_pos >= content.size() || content[start_pos] != '{') return {};
    int depth = 1;
    size_t idx = start_pos + 1;
    while (idx < content.size() && depth > 0) {
        if (content[idx] == '{') depth++;
        else if (content[idx] == '}') depth--;
        idx++;
    }
    if (depth != 0) return {};
    if (idx <= start_pos + 1) return {};
    return content.substr(start_pos + 1, idx - start_pos - 2);
}

std::vector<CppClassDef> extract_cpp_class_definitions(const fs::path& path) {
    std::vector<CppClassDef> classes;
    std::string content = read_file(path);
    if (content.empty()) return classes;

    std::string clean = remove_cpp_comments(content);

    std::regex class_def_re(
        R"((?:template\s*<[^>]*>\s*)?(class|struct)\s+(\w+)(?:\s*:\s*[^{]+)?\s*\{)",
        std::regex::multiline);

    std::sregex_iterator begin(clean.begin(), clean.end(), class_def_re);
    std::sregex_iterator end;

    std::set<std::pair<std::string, int>> seen;
    for (auto it = begin; it != end; ++it) {
        const auto& match = *it;
        std::string kind = match[1].str();
        std::string name = match[2].str();
        int line = static_cast<int>(std::count(clean.begin(), clean.begin() + match.position(), '\n') + 1);
        auto key = std::make_pair(name, line);
        if (seen.count(key)) continue;
        seen.insert(key);

        bool is_stub = false;
        std::string stub_reason;
        size_t brace_pos = match.position() + match.length() - 1;
        std::string body = extract_class_body(clean, brace_pos);
        if (!body.empty()) {
            std::string trimmed = body;
            trimmed.erase(trimmed.begin(),
                          std::find_if(trimmed.begin(), trimmed.end(),
                                       [](unsigned char c) { return !std::isspace(c); }));
            trimmed.erase(std::find_if(trimmed.rbegin(), trimmed.rend(),
                                       [](unsigned char c) { return !std::isspace(c); }).base(),
                          trimmed.end());
            if (trimmed.size() < 30) {
                is_stub = true;
                stub_reason = "empty_body";
            } else {
                std::string condensed = std::regex_replace(trimmed, std::regex(R"(\s+)"), " ");
                std::regex dtor_only_re(
                    R"(^\s*(?:public:|private:|protected:)?\s*(?:virtual\s+)?~\w+\([^)]*\)\s*(?:=\s*default\s*)?;?\s*$)",
                    std::regex::ECMAScript);
                if (std::regex_match(condensed, dtor_only_re)) {
                    is_stub = true;
                    stub_reason = "only_destructor";
                }
            }
        }

        classes.push_back({name, kind, path.string(), line, is_stub, stub_reason});
    }

    return classes;
}

bool is_stub_file(const fs::path& path) {
    std::string content = read_file(path);
    if (content.empty()) return false;

    std::string clean = remove_cpp_comments(content);
    clean = std::regex_replace(clean, std::regex(R"(#\w+[^\n]*)"), "");
    clean = std::regex_replace(clean, std::regex(R"(namespace\s+[\w:]+\s*\{)"), "");
    clean = std::regex_replace(clean, std::regex(R"(#pragma[^\n]*)"), "");
    clean.erase(std::remove_if(clean.begin(), clean.end(),
                               [](unsigned char c) { return std::isspace(c); }),
                clean.end());
    return clean.size() < 100;
}

void build_kotlin_index(const std::string& kotlin_root,
                        Codebase& kotlin,
                        std::vector<const SourceFile*>& ranked,
                        std::unordered_map<std::string, int>& usage_count_by_file,
                        std::unordered_map<std::string, int>& usage_count_by_class,
                        std::unordered_map<std::string, int>& usage_count_by_stem) {
    kotlin.scan();
    kotlin.extract_imports();
    kotlin.build_dependency_graph();
    ranked = kotlin.ranked_by_dependents();

    for (const auto& [path, sf] : kotlin.files) {
        usage_count_by_file[path] = sf.dependent_count;
        std::string stem = fs::path(path).stem().string();
        if (stem.ends_with(".common")) stem = stem.substr(0, stem.size() - 7);
        if (stem.ends_with(".native")) stem = stem.substr(0, stem.size() - 7);
        usage_count_by_stem[stem] = sf.dependent_count;
    }

    std::regex class_re(R"((?:class|interface|object)\s+(\w+))");
    for (const auto* sf : ranked) {
        std::string content = read_file(kotlin.root_path + "/" + sf->relative_path);
        if (content.empty()) continue;
        for (auto it = std::sregex_iterator(content.begin(), content.end(), class_re);
             it != std::sregex_iterator(); ++it) {
            std::string name = (*it)[1].str();
            if (!usage_count_by_class.count(name)) {
                usage_count_by_class[name] = sf->dependent_count;
            }
        }
    }
}

int priority_for_symbol(const std::string& name,
                        const std::unordered_map<std::string, int>& class_deps) {
    auto it = class_deps.find(name);
    if (it == class_deps.end()) return 0;
    return -it->second;
}

int priority_for_file(const std::string& file,
                      const std::unordered_map<std::string, int>& stem_deps) {
    std::string stem = fs::path(file).stem().string();
    if (stem.ends_with(".common")) stem = stem.substr(0, stem.size() - 7);
    if (stem.ends_with(".native")) stem = stem.substr(0, stem.size() - 7);
    auto it = stem_deps.find(stem);
    if (it == stem_deps.end()) return 0;
    return -it->second;
}

}  // namespace

void cmd_symbols(const std::string& kotlin_root,
                 const std::string& cpp_root,
                 const SymbolAnalysisOptions& options) {
    Codebase kotlin(kotlin_root, "kotlin");
    std::vector<const SourceFile*> ranked;
    std::unordered_map<std::string, int> usage_count_by_file;
    std::unordered_map<std::string, int> usage_count_by_class;
    std::unordered_map<std::string, int> usage_count_by_stem;

    build_kotlin_index(kotlin_root, kotlin, ranked, usage_count_by_file,
                       usage_count_by_class, usage_count_by_stem);

    std::map<std::string, std::vector<CppClassDef>> duplicates;
    for (const auto& entry : fs::recursive_directory_iterator(cpp_root)) {
        if (!entry.is_regular_file()) continue;
        std::string path = entry.path().string();
        if (path.find("test") != std::string::npos) continue;
        if (!path.ends_with(".hpp") && !path.ends_with(".cpp") &&
            !path.ends_with(".h") && !path.ends_with(".cc")) {
            continue;
        }
        auto defs = extract_cpp_class_definitions(entry.path());
        for (const auto& cls : defs) {
            duplicates[cls.name].push_back(cls);
        }
    }

    std::vector<std::pair<std::string, std::vector<CppClassDef>>> dup_list;
    for (auto& [name, locs] : duplicates) {
        std::set<std::string> files;
        for (const auto& loc : locs) {
            files.insert(loc.file);
        }
        if (files.size() > 1) {
            dup_list.emplace_back(name, locs);
        }
    }

    std::sort(dup_list.begin(), dup_list.end(),
              [&](const auto& a, const auto& b) {
                  int pa = priority_for_symbol(a.first, usage_count_by_class);
                  int pb = priority_for_symbol(b.first, usage_count_by_class);
                  if (pa != pb) return pa < pb;
                  return a.first < b.first;
              });

    std::vector<StubItem> stubs;
    for (const auto& entry : fs::recursive_directory_iterator(cpp_root)) {
        if (!entry.is_regular_file()) continue;
        std::string path = entry.path().string();
        if (path.find("test") != std::string::npos) continue;
        if (path.ends_with(".cpp") || path.ends_with(".cc")) {
            if (is_stub_file(entry.path())) {
                stubs.push_back({fs::relative(entry.path(), cpp_root).string(),
                                 "file_stub", entry.path().stem().string(), 0, ""});
            }
        }
        if (path.ends_with(".hpp") || path.ends_with(".h")) {
            auto defs = extract_cpp_class_definitions(entry.path());
            for (const auto& cls : defs) {
                if (cls.is_stub) {
                    stubs.push_back({fs::relative(entry.path(), cpp_root).string(),
                                     "class_stub", cls.name, cls.line, cls.stub_reason});
                }
            }
        }
    }

    std::sort(stubs.begin(), stubs.end(),
              [&](const StubItem& a, const StubItem& b) {
                  int pa = priority_for_symbol(a.name, usage_count_by_class);
                  int pb = priority_for_symbol(b.name, usage_count_by_class);
                  if (pa == 0) pa = priority_for_file(a.file, usage_count_by_stem);
                  if (pb == 0) pb = priority_for_file(b.file, usage_count_by_stem);
                  if (pa != pb) return pa < pb;
                  return a.file < b.file;
              });

    std::cout << "======================================================================\n";
    std::cout << "SYMBOL DEFINITION ANALYSIS (ordered by dependency count)\n";
    std::cout << "======================================================================\n";

    if (options.duplicates || (!options.stubs && !options.misplaced)) {
        std::cout << "\n--- DUPLICATE DEFINITIONS (real definitions in multiple files) ---\n";
        std::cout << "Found " << dup_list.size() << " symbols with multiple definitions:\n\n";

        size_t shown = std::min<size_t>(30, dup_list.size());
        for (size_t i = 0; i < shown; ++i) {
            const auto& entry = dup_list[i];
            std::cout << "  class: " << entry.first << "\n";
            std::map<std::string, std::vector<CppClassDef>> by_file;
            for (const auto& loc : entry.second) {
                by_file[loc.file].push_back(loc);
            }
            for (const auto& [file, locs] : by_file) {
                bool stub = false;
                std::vector<int> lines;
                for (const auto& loc : locs) {
                    lines.push_back(loc.line);
                    stub = stub || loc.is_stub;
                }
                std::sort(lines.begin(), lines.end());
                std::ostringstream line_list;
                for (size_t j = 0; j < lines.size(); ++j) {
                    if (j) line_list << ", ";
                    line_list << lines[j];
                }
                std::cout << "    - " << fs::relative(file, cpp_root).string()
                          << ":" << line_list.str()
                          << (stub ? " [STUB]" : "") << "\n";
            }
        }
        if (dup_list.size() > shown) {
            std::cout << "\n  ... and " << (dup_list.size() - shown) << " more\n";
        }
    }

    if (options.stubs || (!options.duplicates && !options.misplaced)) {
        std::vector<StubItem> file_stubs;
        std::vector<StubItem> class_stubs;
        for (const auto& stub : stubs) {
            if (stub.type == "file_stub") file_stubs.push_back(stub);
            else class_stubs.push_back(stub);
        }

        std::cout << "\n--- STUB IMPLEMENTATIONS (ordered by dependency) ---\n";
        std::cout << "\nStub files (" << file_stubs.size() << "):\n";
        size_t shown_files = std::min<size_t>(20, file_stubs.size());
        for (size_t i = 0; i < shown_files; ++i) {
            std::cout << "    - " << file_stubs[i].file << "\n";
        }
        if (file_stubs.size() > shown_files) {
            std::cout << "    ... and " << (file_stubs.size() - shown_files) << " more\n";
        }

        std::cout << "\nStub classes (" << class_stubs.size() << "):\n";
        size_t shown_classes = std::min<size_t>(20, class_stubs.size());
        for (size_t i = 0; i < shown_classes; ++i) {
            const auto& stub = class_stubs[i];
            std::cout << "    - " << stub.name << " in " << stub.file;
            if (!stub.reason.empty()) std::cout << " (" << stub.reason << ")";
            std::cout << "\n";
        }
        if (class_stubs.size() > shown_classes) {
            std::cout << "    ... and " << (class_stubs.size() - shown_classes) << " more\n";
        }
    }

    std::cout << "\n======================================================================\n";
}

void cmd_symbol_lookup(const std::string& kotlin_root,
                       const std::string& cpp_root,
                       const SymbolAnalysisOptions& options) {
    if (options.symbol.empty()) {
        std::cerr << "Error: --symbols-symbol requires a symbol name\n";
        return;
    }

    Codebase kotlin(kotlin_root, "kotlin");
    std::vector<const SourceFile*> ranked;
    std::unordered_map<std::string, int> usage_count_by_file;
    std::unordered_map<std::string, int> usage_count_by_class;
    std::unordered_map<std::string, int> usage_count_by_stem;

    build_kotlin_index(kotlin_root, kotlin, ranked, usage_count_by_file,
                       usage_count_by_class, usage_count_by_stem);

    struct Loc { std::string file; bool is_def; int deps; bool is_forward = false; int refs = 0; };
    std::vector<Loc> kt_locations;
    std::vector<Loc> cpp_locations;

    std::regex kt_re("(?:class|interface|object|fun)\\s+" + options.symbol + "\\b");
    std::regex kt_ref_re("\\b" + options.symbol + "\\b");

    for (const auto* sf : ranked) {
        std::string path = kotlin.root_path + "/" + sf->relative_path;
        std::string content = read_file(path);
        if (content.empty()) continue;
        if (std::regex_search(content, kt_ref_re)) {
            bool is_def = std::regex_search(content, kt_re);
            kt_locations.push_back({sf->relative_path, is_def, sf->dependent_count});
        }
    }

    std::regex cpp_def_re("(?:class|struct)\\s+" + options.symbol + "(?:\\s*:[^\\{]+)?\\s*\\{");
    std::regex cpp_fwd_re("(?:class|struct)\\s+" + options.symbol + "\\s*;");
    std::regex cpp_ref_re("\\b" + options.symbol + "\\b");

    for (const auto& entry : fs::recursive_directory_iterator(cpp_root)) {
        if (!entry.is_regular_file()) continue;
        std::string path = entry.path().string();
        if (path.find("test") != std::string::npos) continue;
        if (!path.ends_with(".hpp") && !path.ends_with(".cpp") &&
            !path.ends_with(".h") && !path.ends_with(".cc")) {
            continue;
        }
        std::string content = read_file(entry.path());
        if (content.empty()) continue;
        if (std::regex_search(content, cpp_ref_re)) {
            bool is_def = std::regex_search(content, cpp_def_re);
            bool is_fwd = std::regex_search(content, cpp_fwd_re) && !is_def;
            int refs = 0;
            for (auto it = std::sregex_iterator(content.begin(), content.end(), cpp_ref_re);
                 it != std::sregex_iterator(); ++it) {
                refs++;
            }
            cpp_locations.push_back({fs::relative(entry.path(), cpp_root).string(), is_def, 0, is_fwd, refs});
        }
    }

    if (options.json) {
        std::cout << "{\n";
        std::cout << "  \"symbol\": \"" << options.symbol << "\",\n";
        std::cout << "  \"kotlin_locations\": [\n";
        for (size_t i = 0; i < kt_locations.size(); ++i) {
            const auto& loc = kt_locations[i];
            std::cout << "    {\"file\": \"" << loc.file << "\", \"is_definition\": "
                      << (loc.is_def ? "true" : "false") << ", \"deps\": " << loc.deps << "}";
            if (i + 1 < kt_locations.size()) std::cout << ",";
            std::cout << "\n";
        }
        std::cout << "  ],\n";
        std::cout << "  \"cpp_locations\": [\n";
        for (size_t i = 0; i < cpp_locations.size(); ++i) {
            const auto& loc = cpp_locations[i];
            std::cout << "    {\"file\": \"" << loc.file << "\", \"is_definition\": "
                      << (loc.is_def ? "true" : "false") << ", \"is_forward_decl\": "
                      << (loc.is_forward ? "true" : "false") << ", \"reference_count\": "
                      << loc.refs << "}";
            if (i + 1 < cpp_locations.size()) std::cout << ",";
            std::cout << "\n";
        }
        std::cout << "  ]\n";
        std::cout << "}\n";
        return;
    }

    std::cout << "\n=== Analysis of '" << options.symbol << "' ===\n";
    if (usage_count_by_class.count(options.symbol)) {
        std::cout << "Dependency rank: " << usage_count_by_class[options.symbol] << " files depend on this\n";
    }

    std::cout << "\nKotlin locations (" << kt_locations.size() << "):\n";
    for (const auto& loc : kt_locations) {
        std::cout << "  " << (loc.is_def ? "[DEF] " : "[ref] ") << loc.file;
        if (loc.deps > 0) std::cout << " (deps: " << loc.deps << ")";
        std::cout << "\n";
    }

    std::cout << "\nC++ locations (" << cpp_locations.size() << "):\n";
    for (const auto& loc : cpp_locations) {
        std::string marker = loc.is_def ? "[DEF] " : (loc.is_forward ? "[fwd] " : "[ref] ");
        std::cout << "  " << marker << loc.file << "\n";
    }
}

}  // namespace ast_distance
