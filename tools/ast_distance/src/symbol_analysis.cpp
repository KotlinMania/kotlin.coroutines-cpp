#include "symbol_analysis.hpp"
#include "cpp_review.hpp"
#include "reexport_config.hpp"
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

bool should_skip_path(const std::string& path) {
    return path.find("/test") != std::string::npos ||
           path.find("/build/") != std::string::npos ||
           path.find("/CMakeFiles/") != std::string::npos ||
           path.find("/cmake-build") != std::string::npos ||
           path.find("/target/") != std::string::npos ||
           path.find("/_deps/") != std::string::npos;
}

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

std::vector<CppClassDef> extract_cpp_class_definitions(const fs::path& path) {
    std::vector<CppClassDef> classes;
    for (const auto& type : review_cpp(read_file(path)).types) {
        bool candidate = type.category != "implemented_type" && type.category != "interface";
        classes.push_back({type.name, type.kind, path.string(), type.line, candidate,
                           type.category + ": " + type.reason});
    }
    return classes;
}

void build_kotlin_index(const std::string& kotlin_root,
                        Codebase& kotlin,
                        std::vector<const SourceFile*>& ranked,
                        std::unordered_map<std::string, int>& usage_count_by_file,
                        std::unordered_map<std::string, int>& usage_count_by_class,
                        std::unordered_map<std::string, int>& usage_count_by_stem) {
    (void)kotlin_root;
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
        if (should_skip_path("/" + fs::relative(entry.path(), cpp_root).generic_string())) continue;
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
        std::sort(locs.begin(), locs.end(), [](const auto& a, const auto& b) {
            return std::tie(a.file, a.line) < std::tie(b.file, b.line);
        });
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
    std::vector<std::pair<std::string, std::string>> implementation_locations;
    for (const auto& entry : fs::recursive_directory_iterator(cpp_root)) {
        if (!entry.is_regular_file()) continue;
        std::string path = entry.path().string();
        if (should_skip_path("/" + fs::relative(entry.path(), cpp_root).generic_string())) continue;
        if (path.ends_with(".cpp") || path.ends_with(".cc")) {
            auto review = review_cpp(read_file(entry.path()));
            if (!review.file_category.empty()) {
                bool found_companion = false;
                if (!review.has_parse_errors) {
                    for (const auto& extension : {".hpp", ".h", ".hh", ".hxx"}) {
                        auto companion = entry.path(); companion.replace_extension(extension);
                        if (!fs::is_regular_file(companion)) continue;
                        auto header = review_cpp(read_file(companion));
                        if (!header.has_parse_errors && header.has_implementation) {
                            implementation_locations.emplace_back(fs::relative(entry.path(), cpp_root).string(),
                                fs::relative(companion, cpp_root).string());
                            found_companion = true;
                            break;
                        }
                    }
                }
                if (!found_companion) stubs.push_back({fs::relative(entry.path(), cpp_root).string(),
                    review.file_category, entry.path().stem().string(), 1, review.file_reason});
            }
        }
        if (path.ends_with(".hpp") || path.ends_with(".h")) {
            auto defs = extract_cpp_class_definitions(entry.path());
            for (const auto& cls : defs) {
                if (cls.is_stub) {
                    stubs.push_back({fs::relative(entry.path(), cpp_root).string(),
                                     "type_review", cls.name, cls.line, cls.stub_reason});
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
                  return std::tie(a.file, a.line, a.name, a.type) < std::tie(b.file, b.line, b.name, b.type);
              });

    std::sort(implementation_locations.begin(), implementation_locations.end());

    if (options.json) {
        auto quote = [](const std::string& value) { return "\"" + json_escape(value) + "\""; };
        std::cout << "{\n  \"schema_version\": 1,\n  \"classification\": \"review_candidates_not_verified_stubs\",\n  \"findings\": [";
        if (options.stubs || !options.duplicates) {
            for (size_t i = 0; i < stubs.size(); ++i) {
                const auto& finding = stubs[i];
                if (i) std::cout << ',';
                std::cout << "\n    {\"file\":" << quote(finding.file) << ",\"line\":" << finding.line
                    << ",\"name\":" << quote(finding.name) << ",\"category\":" << quote(finding.type)
                    << ",\"reason\":" << quote(finding.reason) << '}';
            }
        }
        std::cout << "\n  ],\n  \"duplicates\": [";
        if (options.duplicates || !options.stubs) {
            for (size_t i = 0; i < dup_list.size(); ++i) {
                if (i) std::cout << ',';
                std::cout << "\n    {\"name\":" << quote(dup_list[i].first) << ",\"locations\": [";
                for (size_t j = 0; j < dup_list[i].second.size(); ++j) {
                    if (j) std::cout << ',';
                    const auto& loc = dup_list[i].second[j];
                    std::cout << "{\"file\":" << quote(fs::relative(loc.file, cpp_root).string())
                        << ",\"line\":" << loc.line << '}';
                }
                std::cout << "]}";
            }
        }
        std::cout << "\n  ],\n  \"implementation_locations\": [";
        for (size_t i = 0; i < implementation_locations.size(); ++i) {
            if (i) std::cout << ',';
            std::cout << "\n    {\"translation_unit\":" << quote(implementation_locations[i].first)
                << ",\"companion\":" << quote(implementation_locations[i].second)
                << ",\"reason\":\"Definitions found in companion; logical-unit parity still requires comparison\"}";
        }
        std::cout << "\n  ]\n}\n";
        return;
    }

    std::cout << "======================================================================\n";
    std::cout << "SYMBOL DEFINITION ANALYSIS (ordered by dependency count)\n";
    std::cout << "======================================================================\n";

    if (options.duplicates || (!options.stubs && !options.misplaced)) {
        std::cout << "\n--- DUPLICATE DEFINITIONS (real definitions in multiple files) ---\n";
        std::cout << "Found " << dup_list.size() << " symbols with multiple definitions:\n\n";

        size_t shown = dup_list.size();
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
                          << (stub ? " [REVIEW]" : "") << "\n";
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
            if (stub.type != "type_review") file_stubs.push_back(stub);
            else class_stubs.push_back(stub);
        }

        std::cout << "\n--- IMPLEMENTATION REVIEW CANDIDATES (ordered by dependency) ---\n";
        std::cout << "\nTranslation units to review (" << file_stubs.size() << "):\n";
        size_t shown_files = file_stubs.size();
        for (size_t i = 0; i < shown_files; ++i) {
            std::cout << "    - " << file_stubs[i].file << ":" << file_stubs[i].line << " (" << file_stubs[i].type << ") " << file_stubs[i].reason << "\n";
        }
        if (file_stubs.size() > shown_files) {
            std::cout << "    ... and " << (file_stubs.size() - shown_files) << " more\n";
        }

        std::cout << "\nTypes to review (" << class_stubs.size() << "):\n";
        size_t shown_classes = class_stubs.size();
        for (size_t i = 0; i < shown_classes; ++i) {
            const auto& stub = class_stubs[i];
            std::cout << "    - " << stub.name << " in " << stub.file << ":" << stub.line;
            if (!stub.reason.empty()) std::cout << " (" << stub.reason << ")";
            std::cout << "\n";
        }
        if (class_stubs.size() > shown_classes) {
            std::cout << "    ... and " << (class_stubs.size() - shown_classes) << " more\n";
        }
    }

    std::cout << "\nDefinitions found in companion files (" << implementation_locations.size() << "):\n";
    for (const auto& [translation_unit, companion] : implementation_locations)
        std::cout << "    - " << translation_unit << " -> " << companion << " (compare logical-unit parity)\n";
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
        if (should_skip_path("/" + fs::relative(entry.path(), cpp_root).generic_string())) continue;
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
