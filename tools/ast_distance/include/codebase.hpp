#pragma once

#include "imports.hpp"
#include "ast_parser.hpp"
#include "similarity.hpp"
#include "porting_utils.hpp"
#include "transliteration_similarity.hpp"
#include "symbol_extractor.hpp"
#include <filesystem>
#include <map>
#include <set>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <regex>
#include <unordered_map>

namespace fs = std::filesystem;

namespace ast_distance {

/**
 * Represents a source file with its metadata.
 */
struct SourceFile {
    std::vector<std::string> paths; // All physical paths (e.g., .hpp and .cpp)
    std::string relative_path;     // Relative to root
    std::string filename;          // Representative filename (e.g., header if paired)
    std::string stem;              // Filename stem (logical unit name)
    std::string qualified_name;    // Disambiguated name (e.g., "widgets.Block")
    std::string extension;         // Representative extension

    PackageDecl package;           // Package/module declaration from source
    std::vector<Import> imports;   // Imports in this file
    std::set<std::string> imported_by;  // Files that import this one (dependents)
    std::set<std::string> depends_on;   // Files this imports (dependencies)

    int dependent_count = 0;       // Number of files that depend on this
    int dependency_count = 0;      // Number of files this depends on

    // For comparison
    float similarity_score = 0.0f;
    std::string matched_file;      // Matched file in other codebase

    // Porting analysis
    std::string transliterated_from;  // "Transliterated from:" header value
    int line_count = 0;
    int code_lines = 0;
    bool is_stub = false;
    std::vector<TodoItem> todos;
    std::vector<LintError> lint_errors;

    // Get the "identity" for matching - last part of package + filename
    std::string identity() const {
        if (!package.parts.empty()) {
            return package.path;
        }
        return qualified_name;
    }

    // Compute qualified name from path
    static std::string make_qualified_name(const std::string& rel_path) {
        std::string result;
        fs::path p(rel_path);

        // Get parent directories (skip the file itself)
        std::vector<std::string> parts;
        for (const auto& part : p.parent_path()) {
            std::string s = part.string();
            if (!s.empty() && s != "." && s != "src") {
                parts.push_back(s);
            }
        }

        // Add the stem
        std::string stem = p.stem().string();

        // Build qualified name: last directory + stem
        if (!parts.empty()) {
            result = parts.back() + "." + stem;
        } else {
            result = stem;
        }

        return result;
    }

    // Normalize name for matching (snake_case <-> PascalCase)
    static std::string normalize_name(const std::string& name) {
        std::string result;
        bool prev_lower = false;

        for (size_t i = 0; i < name.size(); ++i) {
            char c = name[i];

            if (c == '_') {
                // Skip underscores, next char should be upper
                continue;
            }

            if (std::isupper(c) && prev_lower && !result.empty()) {
                // CamelCase boundary - insert separator conceptually
                result += std::tolower(c);
            } else {
                result += std::tolower(c);
            }

            prev_lower = std::islower(c);
        }

        return result;
    }

    static std::string to_kebab_case(const std::string& name) {
        std::string result;
        for (size_t i = 0; i < name.length(); ++i) {
            char c = name[i];
            if (c == '_') {
                result += '-';
            } else if (std::isupper(c) && i > 0 && name[i-1] != '-' && name[i-1] != '_') {
                result += '-';
                result += std::tolower(c);
            } else {
                result += std::tolower(c);
            }
        }
        return result;
    }

    /**
     * Convert snake_case to PascalCase for Kotlin filename generation.
     * Example: "value" -> "Value", "my_file_name" -> "MyFileName"
     */
    static std::string to_pascal_case(const std::string& name) {
        // Special-case common Rust stems that are written without underscores but should
        // map to Kotlin's acronym-style PascalCase.
        // Example: "refcell" -> "RefCell" (not "Refcell")
        auto normalize = [](const std::string& s) {
            std::string out;
            out.reserve(s.size());
            for (char c : s) {
                if (c == '_') continue;
                out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            }
            return out;
        };
        static const std::map<std::string, std::string> kSpecial = {
            {"refcell", "RefCell"},
        };
        auto it = kSpecial.find(normalize(name));
        if (it != kSpecial.end()) {
            return it->second;
        }

        std::string result;
        bool capitalize_next = true;

        for (size_t i = 0; i < name.size(); ++i) {
            char c = name[i];

            if (c == '_') {
                capitalize_next = true;
                continue;
            }

            if (capitalize_next) {
                result += std::toupper(c);
                capitalize_next = false;
            } else {
                result += c;
            }
        }

        return result;
    }
};

/**
 * Manages a codebase - scans files, extracts imports, builds dependency graph.
 */
class Codebase {
public:
    std::string root_path;
    std::string language;  // "rust" or "kotlin"
    std::map<std::string, SourceFile> files;  // keyed by logical key
    std::map<std::string, std::vector<std::string>> by_stem;  // stem -> list of files
    std::map<std::string, std::string> by_qualified;  // qualified_name -> logical key

    Codebase(const std::string& root, const std::string& lang)
        : root_path(root), language(lang) {}

    /**
     * Scan directory and build file list.
     */
    void scan() {
        auto has_valid_ext = [this](const std::string& path) {
            if (language == "rust") {
                return path.ends_with(".rs");
            } else if (language == "kotlin") {
                return path.ends_with(".kt") || path.ends_with(".kts");
            } else if (language == "cpp") {
                return path.ends_with(".cpp") || path.ends_with(".hpp") ||
                       path.ends_with(".cc") || path.ends_with(".h");
            } else if (language == "python") {
                return path.ends_with(".py");
            } else if (language == "typescript") {
                return path.ends_with(".ts") || path.ends_with(".tsx");
            }
            return false;
        };

        if (fs::is_regular_file(root_path)) {
            // Handle single file input
            if (has_valid_ext(root_path)) {
                SourceFile sf;
                sf.paths.push_back(root_path);
                sf.relative_path = fs::path(root_path).filename().string(); // Use filename as relative path
                sf.filename = fs::path(root_path).filename().string();
                sf.stem = fs::path(root_path).stem().string();
                sf.extension = fs::path(root_path).extension().string();
                sf.qualified_name = SourceFile::make_qualified_name(sf.relative_path);

                files[sf.relative_path] = sf;
                by_stem[sf.stem].push_back(sf.relative_path);
                by_qualified[sf.qualified_name] = sf.relative_path;
            }
            return;
        }

        // Kotlin Multiplatform convention:
        // If root is under `src/commonMain/kotlin/...`, also scan
        // the sibling `src/commonTest/kotlin/...` tree so test ports
        // (annotated with `// port-lint: tests ...`) are discovered.
        std::vector<fs::path> roots_to_scan;
        fs::path rel_base = root_path;
        roots_to_scan.push_back(root_path);
        if (language == "kotlin") {
            // Accept both absolute paths containing `/src/commonMain/kotlin/`
            // and relative roots beginning with `src/commonMain/kotlin/`.
            const std::string leading = "src/commonMain/kotlin";
            size_t pos = std::string::npos;
            size_t skip = 0;
            if (root_path == leading || root_path.rfind(leading + "/", 0) == 0) {
                pos = 0;
            } else {
                const std::string marker = "/" + leading;
                const auto found = root_path.find(marker);
                if (found != std::string::npos &&
                    (found + marker.size() == root_path.size() ||
                     root_path[found + marker.size()] == '/')) {
                    pos = found;
                    skip = 1;
                }
            }
            if (pos != std::string::npos) {
                const std::string repo_root = root_path.substr(0, pos);
                size_t suffix_start = pos + skip + leading.size();
                if (suffix_start < root_path.size() && root_path[suffix_start] == '/') {
                    ++suffix_start;
                }
                const std::string suffix = root_path.substr(suffix_start);
                const fs::path test_root = repo_root.empty()
                    ? fs::path("src") / "commonTest" / "kotlin" / suffix
                    : fs::path(repo_root) / "src" / "commonTest" / "kotlin" / suffix;
                if (fs::exists(test_root) && fs::is_directory(test_root)) {
                    roots_to_scan.push_back(test_root);
                    // Use repo root for relative paths so commonMain/commonTest remain distinct.
                    rel_base = repo_root.empty() ? fs::path(".") : fs::path(repo_root);
                }
            }
        }

        for (const auto& scan_root : roots_to_scan) {
            if (!fs::exists(scan_root) || !fs::is_directory(scan_root)) continue;
            for (const auto& entry : fs::recursive_directory_iterator(scan_root)) {
                if (!entry.is_regular_file()) continue;

                std::string path = entry.path().string();
                if (!has_valid_ext(path)) continue;

                // Skip build artifacts and dependencies
                if (path.find("/target/") != std::string::npos ||
                    path.find("/build/") != std::string::npos ||
                    path.find("/build_") != std::string::npos ||
                    path.find("/_deps/") != std::string::npos ||
                    path.find("/node_modules/") != std::string::npos ||
                    path.find("/vendor/") != std::string::npos) {
                    continue;
                }

                std::string rel_path = fs::relative(path, rel_base).string();
                std::string stem = entry.path().stem().string();
                std::string filename = entry.path().filename().string();
                std::string extension = entry.path().extension().string();

                // Logical grouping: keep paired translation units together
                // (e.g., .hpp + .cpp, or platform suffix variants).
                std::string directory = fs::path(rel_path).parent_path().string();
                std::string normalized_stem = stem;
                static const std::vector<std::string> suffixes = {
                    ".common", ".concurrent", ".native", ".common_native", ".darwin", ".apple"
                };
                for (const auto& suffix : suffixes) {
                    if (normalized_stem.size() > suffix.size() &&
                        normalized_stem.compare(normalized_stem.size() - suffix.size(), suffix.size(), suffix) == 0) {
                        normalized_stem = normalized_stem.substr(0, normalized_stem.size() - suffix.size());
                        break;
                    }
                }
                std::string logical_key = directory.empty() ? normalized_stem : directory + "/" + normalized_stem;

                if (files.count(logical_key)) {
                    files[logical_key].paths.push_back(path);
                    // Prefer header as representative entry when paired.
                    if (extension == ".hpp" || extension == ".h") {
                        files[logical_key].filename = filename;
                        files[logical_key].extension = extension;
                        files[logical_key].relative_path = rel_path;
                    }
                } else {
                    SourceFile sf;
                    sf.paths.push_back(path);
                    sf.relative_path = rel_path;
                    sf.filename = filename;
                    sf.stem = stem;
                    sf.extension = extension;
                    sf.qualified_name = SourceFile::make_qualified_name(rel_path);

                    files[logical_key] = sf;
                    by_stem[sf.stem].push_back(logical_key);
                    by_qualified[sf.qualified_name] = logical_key;
                }
            }
        }

        // Handle duplicates - if multiple files have same stem, use qualified names
        // Sort to process header files first so they keep the short qualified name
        for (auto& [stem, paths] : by_stem) {
            if (paths.size() > 1) {
                // Multiple files with same stem - ensure unique qualified names
                // Sort paths so headers (.hpp, .h) come before implementation (.cpp, .cc)
                std::sort(paths.begin(), paths.end(), [this](const std::string& a, const std::string& b) {
                    auto& sf_a = files[a];
                    auto& sf_b = files[b];
                    bool a_header = sf_a.extension == ".hpp" || sf_a.extension == ".h" ||
                                    sf_a.extension == ".hxx" || sf_a.extension == ".hh";
                    bool b_header = sf_b.extension == ".hpp" || sf_b.extension == ".h" ||
                                    sf_b.extension == ".hxx" || sf_b.extension == ".hh";
                    // Headers come first
                    if (a_header != b_header) return a_header > b_header;
                    // Then by path length (shorter = less nesting = more "canonical")
                    return a.size() < b.size();
                });

                std::set<std::string> seen_qualified;
                for (auto& path : paths) {
                    auto& sf = files[path];
                    if (seen_qualified.count(sf.qualified_name)) {
                        // Need more context - use full parent path
                        fs::path p(sf.relative_path);
                        std::string full_qualified;
                        for (const auto& part : p.parent_path()) {
                            std::string s = part.string();
                            if (!s.empty() && s != "." && s != "src") {
                                if (!full_qualified.empty()) full_qualified += ".";
                                full_qualified += s;
                            }
                        }
                        full_qualified += "." + sf.stem;
                        sf.qualified_name = full_qualified;
                    }
                    seen_qualified.insert(sf.qualified_name);
                    by_qualified[sf.qualified_name] = path;
                }
            }
        }
    }

    /**
     * Extract imports and packages from all files.
     */
    void extract_imports() {
        ImportExtractor extractor;

        for (auto& [path, sf] : files) {
            for (const auto& p : sf.paths) {
                auto file_imports = extractor.extract_from_file(p);
                sf.imports.insert(sf.imports.end(), file_imports.begin(), file_imports.end());

                if (language == "python") {
                    // Derive Python package from representative relative path.
                    if (sf.package.parts.empty()) {
                        sf.package = derive_python_module(sf.relative_path);
                    }
                } else if (sf.package.parts.empty()) {
                    sf.package = extractor.extract_package_from_file(p);
                }
            }
            sf.dependency_count = sf.imports.size();
        }
    }

    /**
     * Extract porting analysis data (transliterated_from, TODOs, lint, line counts).
     */
    void extract_porting_data() {
        for (auto& [path, sf] : files) {
            sf.transliterated_from.clear();
            sf.line_count = 0;
            sf.code_lines = 0;
            sf.is_stub = false;
            sf.todos.clear();
            sf.lint_errors.clear();

            bool all_parts_stub = !sf.paths.empty();
            for (const auto& p : sf.paths) {
                if (sf.transliterated_from.empty()) {
                    sf.transliterated_from = PortingAnalyzer::extract_transliterated_from(p);
                }

                FileStats stats = PortingAnalyzer::analyze_file(p);
                sf.line_count += stats.line_count;
                sf.code_lines += stats.code_lines;
                sf.todos.insert(sf.todos.end(), stats.todos.begin(), stats.todos.end());

                auto lints = PortingAnalyzer::lint_file(p);
                sf.lint_errors.insert(sf.lint_errors.end(), lints.begin(), lints.end());

                all_parts_stub = all_parts_stub && stats.is_stub;
            }

            // If any part has meaningful code, treat logical unit as non-stub.
            // Threshold: 100 code lines — files under this with no real content
            // after boilerplate stripping are stubs.
            sf.is_stub = all_parts_stub && sf.code_lines <= 100;
        }
    }

    /**
     * Build map of transliterated_from paths to files for matching.
     */
    std::map<std::string, std::string> transliteration_map() const {
        std::map<std::string, std::string> result;
        for (const auto& [path, sf] : files) {
            if (!sf.transliterated_from.empty()) {
                result[sf.transliterated_from] = path;
            }
        }
        return result;
    }

    /**
     * Build dependency graph - resolve imports to actual files.
     */
    void build_dependency_graph() {
        for (auto& [path, sf] : files) {
            for (const auto& imp : sf.imports) {
                // Try to resolve import to a file in this codebase
                std::string resolved = resolve_import(imp);
                if (!resolved.empty() && resolved != path) {
                    sf.depends_on.insert(resolved);
                    files[resolved].imported_by.insert(path);
                }
            }
        }

        // Update counts
        for (auto& [path, sf] : files) {
            sf.dependent_count = sf.imported_by.size();
        }
    }

    /**
     * Get files sorted by dependent count (most depended-on first).
     */
    std::vector<const SourceFile*> ranked_by_dependents() const {
        std::vector<const SourceFile*> result;
        for (const auto& [_, sf] : files) {
            result.push_back(&sf);
        }

        std::sort(result.begin(), result.end(),
            [](const SourceFile* a, const SourceFile* b) {
                return a->dependent_count > b->dependent_count;
            });

        return result;
    }

    /**
     * Get leaf files (no dependents - safe to port first).
     */
    std::vector<const SourceFile*> leaf_files() const {
        std::vector<const SourceFile*> result;
        for (const auto& [_, sf] : files) {
            if (sf.dependent_count == 0) {
                result.push_back(&sf);
            }
        }
        return result;
    }

    /**
     * Get root files (many dependents - core infrastructure).
     */
    std::vector<const SourceFile*> root_files(int min_dependents = 3) const {
        std::vector<const SourceFile*> result;
        for (const auto& [_, sf] : files) {
            if (sf.dependent_count >= min_dependents) {
                result.push_back(&sf);
            }
        }

        std::sort(result.begin(), result.end(),
            [](const SourceFile* a, const SourceFile* b) {
                return a->dependent_count > b->dependent_count;
            });

        return result;
    }

    void print_summary() const {
        std::cout << "Codebase: " << root_path << " (" << language << ")\n";
        std::cout << "  Files: " << files.size() << "\n";

        int total_imports = 0;
        int max_dependents = 0;
        std::string most_depended;

        for (const auto& [_, sf] : files) {
            total_imports += sf.imports.size();
            if (sf.dependent_count > max_dependents) {
                max_dependents = sf.dependent_count;
                most_depended = sf.qualified_name;
            }
        }

        std::cout << "  Total imports: " << total_imports << "\n";
        if (!most_depended.empty()) {
            std::cout << "  Most depended: " << most_depended
                      << " (" << max_dependents << " dependents)\n";
        }
    }

private:
    static PackageDecl derive_python_module(const std::string& rel_path) {
        PackageDecl pkg;
        fs::path p(rel_path);

        std::vector<std::string> parts;
        for (const auto& part : p.parent_path()) {
            std::string s = part.string();
            if (!s.empty() && s != "." && s != "src" && s != "lib") {
                parts.push_back(s);
            }
        }

        std::string stem = p.stem().string();
        if (!stem.empty() && stem != "__init__") {
            parts.push_back(stem);
        }

        pkg.parts = parts;
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i > 0) pkg.path += ".";
            pkg.path += parts[i];
        }

        return pkg;
    }

    std::string resolve_import(const Import& imp) {
        // Try to match import path to a file
        std::string module = imp.module_path;

        // For Rust: crate::foo::bar -> look for foo/bar.rs or foo/bar/mod.rs
        // For Kotlin: com.foo.Bar -> look for Bar.kt in appropriate package

        // Simplistic approach: look for matching stem
        std::string item = imp.item;
        if (item == "*") {
            // Wildcard - try to find the module itself
            size_t last_sep = module.rfind(language == "rust" ? "::" : ".");
            if (last_sep != std::string::npos) {
                item = module.substr(last_sep + (language == "rust" ? 2 : 1));
            }
        }

        // Normalize for matching
        std::string normalized = SourceFile::normalize_name(item);

        // Search in by_stem
        for (const auto& [stem, paths] : by_stem) {
            if (SourceFile::normalize_name(stem) == normalized) {
                // Found a match
                return paths[0];  // Return first match
            }
        }

        return "";
    }
};

/**
 * Compare two codebases and find matches.
 */
class CodebaseComparator {
public:
    Codebase& source;  // e.g., Rust
    Codebase& target;  // e.g., Kotlin

    struct FunctionComparisonResult {
        float score = -1.0f;
        int matched_pairs = 0;
        int source_total = 0;
        int target_total = 0;
        int unmatched_source = 0;
        int unmatched_target = 0;
        bool has_source_stub = false;
        bool has_target_stub = false;
        bool has_stub_mismatch = false;
        int stub_mismatch_count = 0;
    };

    struct ProvenanceProposal {
        std::string source_path;
        std::string target_path;
        std::string current_header;
        std::string proposed_header;
        std::string reason;
    };

    struct Match {
        std::string source_path;
        std::string target_path;
        std::string source_qualified;
        std::string target_qualified;
        float similarity;
        int source_dependents;
        int target_dependents;
        int source_lines = 0;
        int target_lines = 0;
        int todo_count = 0;
        int lint_count = 0;
        bool is_stub = false;
        bool matched_by_header = false;  // True if matched via "Transliterated from:"
        bool matched_by_normalized_provenance = false;
        std::vector<std::string> provenance_warnings;
        std::vector<ProvenanceProposal> provenance_proposals;
        std::vector<std::string> zero_reasons;  // Hard-fail reasons that force similarity to 0

        // Additional target files that explicitly point to this same source
        // file via port-lint headers. Common use: Kotlin commonTest files with
        // `// port-lint: tests ...` should count toward function/test/type
        // parity without becoming the primary production file match.
        std::vector<std::string> additional_target_paths;

        // Function parity (name-based) within a file. Used to prevent "signature-only" stubs.
        int source_function_count = 0;
        int target_function_count = 0;
        int matched_function_count = 0;
        float function_coverage = 1.0f;  // matched / source
        std::vector<std::string> missing_functions;  // source-defined functions missing from target file

        // Test-function parity (subset of the function counts above). Tracked
        // separately so reports can call out missing tests explicitly.
        int source_test_function_count = 0;
        int matched_test_function_count = 0;

        // Type/class parity (name-based) within a file. This prevents a file from
        // "matching" by AST shape while missing key type declarations.
        int source_type_count = 0;
        int target_type_count = 0;
        int matched_type_count = 0;
        float type_coverage = 1.0f;  // matched / source
        std::vector<std::string> missing_types;  // source-defined types missing from target file

        // Documentation statistics
        int source_doc_lines = 0;
        int target_doc_lines = 0;
        int source_doc_comments = 0;
        int target_doc_comments = 0;
        float doc_similarity = 0.0f;     // Cosine similarity of doc word frequencies
        float doc_coverage = 1.0f;       // target/source doc line coverage, capped at 1.0
        float doc_weighted = 0.0f;       // 0.5 * doc_similarity + 0.5 * doc_coverage

        // Compute doc gap ratio: 0 = no gap, 1 = completely missing
        float doc_gap_ratio() const {
            if (source_doc_lines == 0) return 0.0f;  // Source has no docs, no gap
            if (target_doc_lines == 0) return 1.0f;  // Target missing all docs
            float ratio = 1.0f - (static_cast<float>(target_doc_lines) / source_doc_lines);
            return std::max(0.0f, ratio);  // Clamp to 0 if target has more
        }

        // Absolute deficit counts (missing functions + missing types).
        // Tests are included in function_deficit() because they are real code.
        int function_deficit() const {
            return std::max(0, source_function_count - matched_function_count);
        }
        int type_deficit() const {
            return std::max(0, source_type_count - matched_type_count);
        }
        int symbol_deficit() const {
            return function_deficit() + type_deficit();
        }
        int source_symbol_surface() const {
            return std::max(0, source_function_count) + std::max(0, source_type_count);
        }

        /**
         * Porting priority score.
         *
         * Design rationale:
         *   - Primary: source dependents/import fanout. A hot file should be
         *     worked before a leaf module even when the leaf has more missing
         *     declarations, because the hot file clears more downstream
         *     compilation failures.
         *   - Secondary: missing functions and types/classes in that file.
         *     Deficits describe how much concrete porting work remains, but
         *     they no longer outrank fanout on their own.
         *   - Tertiary: total source symbol surface, then function similarity
         *     gap as the final tiebreaker.
         */
        float priority_score() const {
            float dependents = static_cast<float>(source_dependents);
            float deficit = static_cast<float>(symbol_deficit());
            float symbol_surface = static_cast<float>(source_symbol_surface());
            float sim_gap = std::max(0.0f, 1.0f - similarity);

            return dependents * 1000000.0f
                 + deficit * 10000.0f
                 + symbol_surface * 100.0f
                 + sim_gap * 10.0f;
        }
    };

    std::vector<Match> matches;
    std::vector<std::string> unmatched_source;
    std::vector<std::string> unmatched_target;

    CodebaseComparator(Codebase& src, Codebase& tgt)
        : source(src), target(tgt) {}

    static std::string normalize_source_annotation_path(std::string path) {
        std::replace(path.begin(), path.end(), '\\', '/');
        while (path.rfind("./", 0) == 0) {
            path = path.substr(2);
        }
        return path;
    }

    static std::string normalized_source_annotation_component(const std::string& component,
                                                              bool is_filename) {
        if (!is_filename) {
            return SourceFile::normalize_name(component);
        }

        fs::path p(component);
        std::string stem = SourceFile::normalize_name(p.stem().string());
        std::string ext = p.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return stem + ext;
    }

    static std::string normalized_source_annotation_match_key(std::string path) {
        path = normalize_source_annotation_path(std::move(path));
        std::vector<std::string> parts;
        fs::path p(path);
        for (const auto& part : p) {
            std::string segment = part.string();
            if (segment.empty() || segment == ".") {
                continue;
            }
            parts.push_back(segment);
        }

        if (parts.empty()) {
            return "";
        }

        std::ostringstream out;
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i > 0) out << "/";
            out << normalized_source_annotation_component(parts[i], i + 1 == parts.size());
        }
        return out.str();
    }

    static std::set<std::string> source_annotation_match_keys(std::string path) {
        path = normalize_source_annotation_path(std::move(path));
        std::set<std::string> variants;

        auto add_variant = [&](const std::string& variant) {
            std::string key = normalized_source_annotation_match_key(variant);
            if (!key.empty()) {
                variants.insert(std::move(key));
            }
        };

        add_variant(path);

        if (path.rfind("src/", 0) == 0) {
            add_variant(path.substr(4));
        }

        size_t crate_src = path.find("/src/");
        if (crate_src != std::string::npos) {
            add_variant(path.substr(crate_src + 5));
        }

        return variants;
    }

    static std::string canonical_source_annotation_path(std::string path) {
        path = normalize_source_annotation_path(std::move(path));

        if (path.rfind("src/", 0) == 0) {
            path = path.substr(4);
        }

        size_t crate_src = path.find("/src/");
        if (crate_src != std::string::npos) {
            path = path.substr(crate_src + 5);
        }

        return normalize_source_annotation_path(std::move(path));
    }

    static std::string port_lint_source_header_line(const std::string& source_path) {
        return "// port-lint: source " + canonical_source_annotation_path(source_path);
    }

    static std::string port_lint_header_line(const std::string& kind,
                                             const std::string& source_path) {
        return "// port-lint: " + kind + " " + canonical_source_annotation_path(source_path);
    }

    static std::string join_reasons(const std::vector<std::string>& reasons) {
        std::ostringstream out;
        for (size_t i = 0; i < reasons.size(); ++i) {
            if (i > 0) out << "; ";
            out << reasons[i];
        }
        return out.str();
    }

    static std::string strip_kotlin_comments_and_strings(const std::string& source) {
        std::string out(source.size(), ' ');
        enum class State { Normal, LineComment, BlockComment, String, RawString, CharLiteral };
        State state = State::Normal;
        int block_depth = 0;

        for (size_t i = 0; i < source.size(); ++i) {
            char c = source[i];
            char next = (i + 1 < source.size()) ? source[i + 1] : '\0';
            char next2 = (i + 2 < source.size()) ? source[i + 2] : '\0';

            if (c == '\n') {
                out[i] = '\n';
            }

            switch (state) {
                case State::Normal:
                    if (c == '/' && next == '/') {
                        state = State::LineComment;
                        ++i;
                        if (i < source.size() && source[i] == '\n') out[i] = '\n';
                    } else if (c == '/' && next == '*') {
                        state = State::BlockComment;
                        block_depth = 1;
                        ++i;
                    } else if (c == '"' && next == '"' && next2 == '"') {
                        state = State::RawString;
                        i += 2;
                    } else if (c == '"') {
                        state = State::String;
                    } else if (c == '\'') {
                        state = State::CharLiteral;
                    } else {
                        out[i] = c;
                    }
                    break;

                case State::LineComment:
                    if (c == '\n') {
                        state = State::Normal;
                        out[i] = '\n';
                    }
                    break;

                case State::BlockComment:
                    if (c == '/' && next == '*') {
                        block_depth++;
                        ++i;
                    } else if (c == '*' && next == '/') {
                        block_depth--;
                        ++i;
                        if (block_depth <= 0) state = State::Normal;
                    }
                    break;

                case State::String:
                    if (c == '\\') {
                        ++i;
                    } else if (c == '"') {
                        state = State::Normal;
                    }
                    break;

                case State::RawString:
                    if (c == '"' && next == '"' && next2 == '"') {
                        state = State::Normal;
                        i += 2;
                    }
                    break;

                case State::CharLiteral:
                    if (c == '\\') {
                        ++i;
                    } else if (c == '\'') {
                        state = State::Normal;
                    }
                    break;
            }
        }

        return out;
    }

    static std::string extract_kotlin_comments(const std::string& source) {
        std::string out;
        enum class State { Normal, LineComment, BlockComment, String, RawString, CharLiteral };
        State state = State::Normal;
        int block_depth = 0;

        for (size_t i = 0; i < source.size(); ++i) {
            char c = source[i];
            char next = (i + 1 < source.size()) ? source[i + 1] : '\0';
            char next2 = (i + 2 < source.size()) ? source[i + 2] : '\0';

            switch (state) {
                case State::Normal:
                    if (c == '/' && next == '/') {
                        state = State::LineComment;
                        ++i;
                    } else if (c == '/' && next == '*') {
                        state = State::BlockComment;
                        block_depth = 1;
                        ++i;
                    } else if (c == '"' && next == '"' && next2 == '"') {
                        state = State::RawString;
                        i += 2;
                    } else if (c == '"') {
                        state = State::String;
                    } else if (c == '\'') {
                        state = State::CharLiteral;
                    }
                    break;

                case State::LineComment:
                    if (c == '\n') {
                        state = State::Normal;
                        out += '\n';
                    } else {
                        out += c;
                    }
                    break;

                case State::BlockComment:
                    if (c == '/' && next == '*') {
                        block_depth++;
                        out += ' ';
                        ++i;
                    } else if (c == '*' && next == '/') {
                        block_depth--;
                        out += ' ';
                        ++i;
                        if (block_depth <= 0) {
                            state = State::Normal;
                            out += '\n';
                        }
                    } else {
                        out += c;
                    }
                    break;

                case State::String:
                    if (c == '\\') {
                        ++i;
                    } else if (c == '"') {
                        state = State::Normal;
                    }
                    break;

                case State::RawString:
                    if (c == '"' && next == '"' && next2 == '"') {
                        state = State::Normal;
                        i += 2;
                    }
                    break;

                case State::CharLiteral:
                    if (c == '\\') {
                        ++i;
                    } else if (c == '\'') {
                        state = State::Normal;
                    }
                    break;
            }
        }

        return out;
    }

    static bool looks_like_lower_snake_identifier(const std::string& token) {
        if (token.empty() || token == "_") return false;
        if (token.find('_') == std::string::npos) return false;
        if (token.front() == '_' || token.back() == '_') return false;

        bool has_lower = false;
        bool has_letter = false;
        for (char c : token) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (std::isalpha(uc)) {
                has_letter = true;
                if (std::islower(uc)) has_lower = true;
            }
        }
        return has_letter && has_lower;
    }

    static std::vector<std::string> kotlin_contamination_reasons_for_text(const std::string& source) {
        std::vector<std::string> reasons;
        std::string code = strip_kotlin_comments_and_strings(source);
        std::string comments = extract_kotlin_comments(source);
        // Drop the canonical port-lint provenance header from the comment scan.
        //
        //     // port-lint: source <relative-path-to-rust-file>
        //     // port-lint: tests  <relative-path-to-rust-file>
        //
        // The header is REQUIRED for port tracking and may legitimately contain
        // snake_case segments from upstream Rust filenames (`parse_tree.rs`,
        // `dedup_sorted_iter.rs`, etc.). Without this filter the header itself
        // trips the snake_case cheat detector below.
        //
        // The regex is strict: the WHOLE line (after `//` stripping by
        // extract_kotlin_comments) must be `<ws>port-lint:<ws>(source|tests)<ws><path><ws>`.
        // Anything trailing the path -- including `fn cheat(){}` riding the
        // same physical line -- fails the match and the line is scanned
        // normally. This closes the obvious bypass of letting attackers
        // smuggle Rust syntax onto a port-lint line.
        {
            static const std::regex port_lint_header_re(
                R"(^\s*port-lint:\s*(?:source|tests)\s+\S+\s*$)");
            std::istringstream in(comments);
            std::ostringstream out;
            std::string line;
            bool first = true;
            while (std::getline(in, line)) {
                if (std::regex_match(line, port_lint_header_re)) {
                    continue;
                }
                if (!first) out << "\n";
                first = false;
                out << line;
            }
            comments = out.str();
        }

        auto add_reason = [&](const std::string& reason) {
            if (std::find(reasons.begin(), reasons.end(), reason) == reasons.end()) {
                reasons.push_back(reason);
            }
        };

        auto scan_text = [&](const std::string& text, const std::string& where) {
            static const std::regex snake_re(R"(\b[A-Za-z][A-Za-z0-9]*_[A-Za-z0-9_]*\b)");
            for (std::sregex_iterator it(text.begin(), text.end(), snake_re), end; it != end; ++it) {
                std::string token = it->str();
                if (looks_like_lower_snake_identifier(token)) {
                    add_reason("snake_case identifier `" + token + "` in Kotlin " + where);
                    break;
                }
            }

            struct RustPattern {
                std::regex pattern;
                std::string reason;
            };
            static const std::vector<RustPattern> rust_patterns = {
                {std::regex(R"(\bfn\s+[A-Za-z_][A-Za-z0-9_]*\s*\()"), "Rust `fn` declaration"},
                {std::regex(R"(\blet\s+(mut\s+)?[A-Za-z_][A-Za-z0-9_]*)"), "Rust `let` binding"},
                {std::regex(R"(\bpub(\([^)]*\))?\s+(fn|struct|enum|trait|mod|use)\b)"), "Rust `pub` item"},
                {std::regex(R"(\bimpl(\s*<[^>{;]*>)?\s+[A-Za-z_][A-Za-z0-9_:<>,\s]*(\s+for\s+[A-Za-z_][A-Za-z0-9_:<>,\s]*)?\s*\{)"), "Rust `impl` block"},
                {std::regex(R"(\bmacro_rules\s*!)"), "Rust `macro_rules!`"},
                {std::regex(R"(#\s*\[[^\]]+\])"), "Rust attribute syntax"},
                {std::regex(R"(\b(assert_eq|assert_ne|debug_assert|format|println|vec|todo|unimplemented)!\s*[\(\{\[])"), "Rust macro invocation"},
                {std::regex(R"(\bmatch\s+[^{;]+\{)"), "Rust `match` expression"},
                {std::regex(R"(\buse\s+[A-Za-z_][A-Za-z0-9_:]*(::|\{))"), "Rust `use` path"},
            };

            for (const auto& rp : rust_patterns) {
                if (std::regex_search(text, rp.pattern)) {
                    add_reason(rp.reason + " in Kotlin " + where);
                    if (reasons.size() >= 4) break;
                }
            }
        };

        scan_text(code, "code");
        scan_text(comments, "comments");

        static const std::regex suppress_padding_re(
            R"(@(?:file:)?Suppress\s*\([^)]*(UNUSED_VARIABLE|unused|FunctionName)[^)]*\))",
            std::regex_constants::icase);
        if (std::regex_search(source, suppress_padding_re)) {
            add_reason("score-padding suppression annotation `@Suppress` in Kotlin code");
        }

        if (source.find("UNCHECKED_CAST") != std::string::npos &&
            (source.find(" as Self") != std::string::npos ||
             source.find(" as ParametersSpec") != std::string::npos)) {
            add_reason("unchecked cast suppression hiding transliteration work in Kotlin code");
        }

        struct CommentCheatPattern {
            std::regex pattern;
            std::string reason;
        };
        static const std::vector<CommentCheatPattern> comment_cheat_patterns = {
            {std::regex(R"((^|\n)\s*(//+|\*)?\s*Kotlin\s*:)", std::regex_constants::icase),
             "translator-note comment (`Kotlin:`) in Kotlin comments"},
            {std::regex(R"(\(\s*from\s+impl\b[^)]*\))", std::regex_constants::icase),
             "Rust impl provenance note in Kotlin comments"},
            // Rust lifetime references in Kotlin comments:
            //   - bare words `lifetime`/`lifetimes` (Rust concept must be
            //     translated to Kotlin idiom -- "scope", "reference", etc.)
            //   - apostrophe-prefixed lifetime annotations (`'a`, `'static`,
            //     `'_`, etc.) -- pure Rust syntax with no Kotlin meaning
            //
            // To distinguish real Rust lifetimes from English contractions
            // (wasn't, it's, VacantEntry's) and from KDoc inline-code spans
            // ending in possessive (`Foo`'s), require the apostrophe to be
            // anchored at start-of-string OR preceded by a Rust-typeish
            // context character (whitespace, `&`, `:`, `<`, `,`, `(`, `;`).
            // Letters, digits, underscore, and backtick before the
            // apostrophe all indicate prose / KDoc, not Rust syntax.
            //
            // Lookbehind is avoided because std::regex's ECMAScript flavor
            // rejects it in some builds; the leading alternation captures
            // the preceding character explicitly.
            {std::regex(R"(\b(lifetime|lifetimes)\b|(^|[\s&:<,;(])'[A-Za-z_][A-Za-z0-9_]*\b)",
             std::regex_constants::icase),
             "Rust lifetime explanation in Kotlin comments"},
            {std::regex(R"(\b(dyn|usize|Box|transmute|unsafe)\b)"),
             "Rust-only type/unsafe terminology in Kotlin comments"},
            {std::regex(R"(Send\s*\+\s*Sync)"),
             "Rust auto-trait terminology in Kotlin comments"},
        };
        for (const auto& cp : comment_cheat_patterns) {
            if (std::regex_search(comments, cp.pattern)) {
                add_reason(cp.reason);
            }
        }

        return reasons;
    }

    static std::vector<std::string> kotlin_contamination_reasons_for_files(
            const std::vector<std::string>& paths) {
        std::vector<std::string> reasons;
        for (const auto& path : paths) {
            std::ifstream in(path);
            if (!in.is_open()) continue;
            std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            auto file_reasons = kotlin_contamination_reasons_for_text(text);
            for (const auto& reason : file_reasons) {
                std::string qualified = fs::path(path).filename().string() + ": " + reason;
                if (std::find(reasons.begin(), reasons.end(), qualified) == reasons.end()) {
                    reasons.push_back(qualified);
                }
                if (reasons.size() >= 6) return reasons;
            }
        }
        return reasons;
    }

    /**
     * Check if a file is a header file based on extension.
     */
    static bool is_header_file(const SourceFile& file) {
        const std::string& ext = file.extension;
        return ext == ".hpp" || ext == ".h" || ext == ".hxx" || ext == ".hh";
    }

    /**
     * Compute name match score between two files.
     * Returns 0.0 for no match, up to 1.02 for perfect match with header preference.
     * Header files get a small boost (0.02) to prefer matching .hpp over .cpp
     * when both have the same qualified name.
     */
    static float name_match_score(const SourceFile& src, const SourceFile& tgt) {
        std::string src_norm = SourceFile::normalize_name(src.stem);
        std::string tgt_norm = SourceFile::normalize_name(tgt.stem);
        std::string src_qual_norm = SourceFile::normalize_name(src.qualified_name);
        std::string tgt_qual_norm = SourceFile::normalize_name(tgt.qualified_name);

        // Small boost for header files - prefer matching .hpp over .cpp
        // when both exist with the same qualified name
        float header_boost = is_header_file(tgt) ? 0.02f : 0.0f;

        // HIGHEST PRIORITY: Exact qualified name match (e.g., "flow.Channels" == "flow.Channels")
        // This ensures flow.Channels matches flow.Channels, not channels.Channels
        if (src_qual_norm == tgt_qual_norm) {
            return 1.0f + header_boost;
        }

        // Extract parent directory from qualified name
        std::string src_parent, tgt_parent;
        size_t src_dot = src.qualified_name.rfind('.');
        size_t tgt_dot = tgt.qualified_name.rfind('.');
        if (src_dot != std::string::npos) src_parent = src.qualified_name.substr(0, src_dot);
        if (tgt_dot != std::string::npos) tgt_parent = tgt.qualified_name.substr(0, tgt_dot);

        // HIGH PRIORITY: Same stem AND same parent directory
        // e.g., flow.Channels should match flow.Channels, not channels.Channels
        if (src_norm == tgt_norm && !src_parent.empty() && !tgt_parent.empty()) {
            std::string src_parent_norm = SourceFile::normalize_name(src_parent);
            std::string tgt_parent_norm = SourceFile::normalize_name(tgt_parent);
            if (src_parent_norm == tgt_parent_norm) {
                return 0.95f + header_boost;  // Same directory + same name = very strong match
            }
        }

        // MEDIUM-HIGH: Exact stem match (but different directory)
        if (src_norm == tgt_norm) {
            return 0.7f + header_boost;  // Lower than before - ambiguous without directory match
        }

        // Check if one contains the other (handles RatatuiLogo vs logo)
        if (tgt_norm.find(src_norm) != std::string::npos) {
            float ratio = static_cast<float>(src_norm.length()) / tgt_norm.length();
            return 0.5f + 0.2f * ratio + header_boost;  // 0.5-0.7 range
        }
        if (src_norm.find(tgt_norm) != std::string::npos) {
            float ratio = static_cast<float>(tgt_norm.length()) / src_norm.length();
            return 0.5f + 0.2f * ratio + header_boost;
        }

        // Package path similarity
        if (!src.package.parts.empty() && !tgt.package.parts.empty()) {
            float pkg_sim = src.package.similarity_to(tgt.package);
            if (pkg_sim > 0.5f) {
                return pkg_sim * 0.6f + header_boost;  // Package match alone is weaker
            }
        }

        // Check if last package component matches filename
        if (!src.package.parts.empty()) {
            std::string src_last = PackageDecl::normalize(src.package.last());
            if (src_last == tgt_norm || tgt_norm.find(src_last) != std::string::npos) {
                return 0.5f + header_boost;
            }
        }
        if (!tgt.package.parts.empty()) {
            std::string tgt_last = PackageDecl::normalize(tgt.package.last());
            if (tgt_last == src_norm || src_norm.find(tgt_last) != std::string::npos) {
                return 0.5f + header_boost;
            }
        }

        // Same parent directory but different filename
        if (SourceFile::normalize_name(src_parent) == SourceFile::normalize_name(tgt_parent) &&
            !src_parent.empty()) {
            return 0.4f + header_boost;  // Same parent directory
        }

        return 0.0f;
    }

    static bool is_test_transliteration(const std::string& header_path) {
        return header_path.rfind("tests:", 0) == 0;
    }

    static std::string source_path_from_transliteration(const std::string& header_path) {
        if (is_test_transliteration(header_path)) {
            return header_path.substr(std::string("tests:").size());
        }
        return header_path;
    }

    struct HeaderMatchResult {
        float score = 0.0f;
        bool normalized_fallback = false;
        std::string warning;
        ProvenanceProposal proposal;
    };

    static ProvenanceProposal provenance_proposal_for_header(
            const SourceFile& src_file,
            const SourceFile& tgt_file,
            const std::string& reason) {
        ProvenanceProposal proposal;
        const bool is_tests = is_test_transliteration(tgt_file.transliterated_from);
        const std::string kind = is_tests ? "tests" : "source";
        proposal.source_path = canonical_source_annotation_path(src_file.relative_path);
        proposal.target_path = tgt_file.relative_path.empty()
            ? tgt_file.filename
            : tgt_file.relative_path;
        proposal.current_header =
            "// port-lint: " + kind + " " +
            normalize_source_annotation_path(
                source_path_from_transliteration(tgt_file.transliterated_from));
        proposal.proposed_header = port_lint_header_line(kind, src_file.relative_path);
        proposal.reason = reason;
        return proposal;
    }

    static HeaderMatchResult exact_transliteration_header_match_result(
            const SourceFile& src_file,
            const SourceFile& tgt_file) {
        HeaderMatchResult result;
        if (tgt_file.transliterated_from.empty()) return result;

        const std::string header_path = normalize_source_annotation_path(
            source_path_from_transliteration(tgt_file.transliterated_from));
        const std::string source_rel = normalize_source_annotation_path(src_file.relative_path);
        if (header_path == source_rel) {
            result.score = 1.0f;
            return result;
        }

        const auto from_keys = source_annotation_match_keys(header_path);
        const auto source_keys = source_annotation_match_keys(source_rel);

        for (const auto& from : from_keys) {
            if (source_keys.count(from)) {
                result.score = 0.99f;
                result.normalized_fallback = true;
                result.warning =
                    "port-lint provenance header matched only after fallback normalization: `" +
                    tgt_file.transliterated_from + "` vs expected `" +
                    canonical_source_annotation_path(src_file.relative_path) + "`";
                result.proposal = provenance_proposal_for_header(src_file, tgt_file, result.warning);
                return result;
            }
        }

        // Basename fallback: when the header carries a path prefix that
        // isn't present in the configured source root (e.g. btree-kotlin's
        // `library/alloc/src/collections/btree/borrow.rs` against a source
        // root `tmp/rust-stdlib-collections-btree/` containing flat
        // `borrow.rs`), the prefix-strip variants above can't reach the
        // source path. Fall through to a lower-scored basename match so
        // those files still pair. Score 0.85 keeps this strictly below
        // the prefix-strip fallback (0.99) and the exact match (1.0), so
        // disambiguation between e.g. `lr1/mod.rs` vs `grammar/mod.rs`
        // still picks the exact match when both are candidates.
        const std::string header_basename =
            normalized_source_annotation_component(
                fs::path(header_path).filename().string(), /*is_filename=*/true);
        const std::string source_basename =
            normalized_source_annotation_component(
                fs::path(source_rel).filename().string(), /*is_filename=*/true);
        if (!header_basename.empty() && header_basename == source_basename) {
            result.score = 0.85f;
            result.normalized_fallback = true;
            result.warning =
                "port-lint provenance header matched only by basename: `" +
                tgt_file.transliterated_from + "` vs expected `" +
                canonical_source_annotation_path(src_file.relative_path) + "`";
            result.proposal = provenance_proposal_for_header(src_file, tgt_file, result.warning);
            return result;
        }

        return result;
    }

    static float exact_transliteration_header_match_score(
            const SourceFile& src_file,
            const SourceFile& tgt_file) {
        return exact_transliteration_header_match_result(src_file, tgt_file).score;
    }

    static float transliteration_header_match_score(
            const SourceFile& src_file,
            const SourceFile& tgt_file) {
        if (tgt_file.transliterated_from.empty()) return 0.0f;
        const std::string from = source_path_from_transliteration(tgt_file.transliterated_from);

        // Check if transliterated_from contains the FULL relative path (most precise)
        if (from.find(src_file.relative_path) != std::string::npos) {
            return 1.0f;
        }
        // Check if transliterated_from ends with the EXACT filename (not substring)
        // Use ends_with to avoid Flow.kt matching StateFlow.kt
        if (from.ends_with("/" + src_file.filename) || from == src_file.filename) {
            // Also verify directory context matches
            std::string tgt_dir = tgt_file.qualified_name;
            std::string src_dir = src_file.qualified_name;
            size_t tgt_dot = tgt_dir.rfind('.');
            size_t src_dot = src_dir.rfind('.');
            if (tgt_dot != std::string::npos) tgt_dir = tgt_dir.substr(0, tgt_dot);
            if (src_dot != std::string::npos) src_dir = src_dir.substr(0, src_dot);

            if (SourceFile::normalize_name(tgt_dir) == SourceFile::normalize_name(src_dir)) {
                return 0.9f;
            }
            return 0.5f;
        }
        // Loose check - transliterated_from ends with stem (stricter than find)
        if (from.ends_with("/" + src_file.stem + ".kt") ||
            from.ends_with("/" + src_file.stem + ".rs") ||
            from.ends_with("/" + src_file.stem + ".py") ||
            from.ends_with("/" + src_file.stem + ".cpp") ||
            from.ends_with("/" + src_file.stem + ".cc") ||
            from.ends_with("/" + src_file.stem + ".hpp") ||
            from.ends_with("/" + src_file.stem + ".h")) {
            return 0.3f;
        }
        return 0.0f;
    }

    /**
     * Find matching files between codebases.
     * Priority: 1) "Transliterated from:" headers, 2) Name matching
     */
    void find_matches() {
        target.extract_porting_data();

        std::set<std::string> matched_sources;
        std::set<std::string> matched_targets;
        const bool strict_provenance_matching =
            (source.language == "rust" && target.language == "kotlin");

        // First pass: Match by "Transliterated from:" header
        // Target files reference source files, so look in target for headers
        // Store candidates with scores for best matching
        struct HeaderCandidate {
            float score = 0.0f;
            std::string src_path;
            std::string tgt_path;
            bool normalized_fallback = false;
            std::string warning;
            ProvenanceProposal proposal;
        };
        std::vector<HeaderCandidate> header_candidates;

        for (const auto& [tgt_path, tgt_file] : target.files) {
            if (tgt_file.transliterated_from.empty()) continue;
            if (is_test_transliteration(tgt_file.transliterated_from)) continue;

            // Try to find the source file that matches the header
            for (const auto& [src_path, src_file] : source.files) {
                HeaderMatchResult match;
                if (strict_provenance_matching) {
                    match = exact_transliteration_header_match_result(src_file, tgt_file);
                } else {
                    match.score = transliteration_header_match_score(src_file, tgt_file);
                }

                if (match.score > 0.0f) {
                    header_candidates.push_back({
                        match.score,
                        src_path,
                        tgt_path,
                        match.normalized_fallback,
                        match.warning,
                        match.proposal,
                    });
                }
            }
        }

        // Sort by score descending, with header preference for ties
        std::sort(header_candidates.begin(), header_candidates.end(),
            [this](const auto& a, const auto& b) {
                float score_a = a.score;
                float score_b = b.score;
                if (std::abs(score_a - score_b) > 0.001f) {
                    return score_a > score_b;  // Higher score first
                }
                // Same score - prefer header files
                const auto& tgt_a = target.files.at(a.tgt_path);
                const auto& tgt_b = target.files.at(b.tgt_path);
                bool a_header = is_header_file(tgt_a);
                bool b_header = is_header_file(tgt_b);
                if (a_header != b_header) return a_header;  // Headers first
                // Same type - prefer shorter path (less nesting)
                return a.tgt_path.size() < b.tgt_path.size();
            });

        for (const auto& candidate : header_candidates) {
            const auto& src_path = candidate.src_path;
            const auto& tgt_path = candidate.tgt_path;
            if (matched_sources.count(src_path) || matched_targets.count(tgt_path)) {
                continue;  // Already matched
            }

            const auto& src_file = source.files.at(src_path);
            const auto& tgt_file = target.files.at(tgt_path);

            Match m;
            m.source_path = src_path;
            m.target_path = tgt_path;
            m.source_qualified = src_file.qualified_name;
            m.target_qualified = tgt_file.qualified_name;
            m.similarity = 0.0f;
            m.source_dependents = src_file.dependent_count;
            m.target_dependents = tgt_file.dependent_count;
            m.source_lines = src_file.line_count;
            m.target_lines = tgt_file.line_count;
            m.todo_count = tgt_file.todos.size();
            m.lint_count = tgt_file.lint_errors.size();
            m.is_stub = tgt_file.is_stub;
            // File size ratio stub detection: if target has < 30% of
            // source code lines, it's effectively a stub regardless of
            // what the content-based check thinks.
            if (!m.is_stub && src_file.code_lines > 20 && tgt_file.code_lines > 0) {
                float ratio = static_cast<float>(tgt_file.code_lines) /
                              static_cast<float>(src_file.code_lines);
                if (ratio < 0.30f) {
                    m.is_stub = true;
                }
            }
            m.matched_by_header = true;
            if (candidate.normalized_fallback) {
                m.matched_by_normalized_provenance = true;
                m.lint_count += 1;
                if (!candidate.warning.empty()) {
                    m.provenance_warnings.push_back(candidate.warning);
                }
                m.provenance_proposals.push_back(candidate.proposal);
            }

            matches.push_back(m);
            matched_sources.insert(src_path);
            matched_targets.insert(tgt_path);
        }

        // Pool additional target files that explicitly point to an already
        // matched source. This keeps `commonTest` and split translation units
        // from being misused as primary matches while still counting their
        // functions/types toward parity for the source file.
        {
            std::map<std::string, Match*> match_by_src;
            for (auto& m : matches) {
                match_by_src[m.source_path] = &m;
            }

            for (const auto& [tgt_path, tgt_file] : target.files) {
                if (matched_targets.count(tgt_path)) continue;
                if (tgt_file.transliterated_from.empty()) continue;

                Match* best = nullptr;
                float best_score = 0.0f;
                HeaderMatchResult best_match;
                for (const auto& [src_path, src_file] : source.files) {
                    auto mit = match_by_src.find(src_path);
                    if (mit == match_by_src.end()) continue;

                    HeaderMatchResult match;
                    if (strict_provenance_matching) {
                        match = exact_transliteration_header_match_result(src_file, tgt_file);
                    } else {
                        match.score = transliteration_header_match_score(src_file, tgt_file);
                    }
                    if (match.score > best_score) {
                        best_score = match.score;
                        best_match = match;
                        best = mit->second;
                    }
                }
                if (best != nullptr) {
                    for (const auto& p : tgt_file.paths) {
                        best->additional_target_paths.push_back(p);
                    }
                    if (best_match.normalized_fallback) {
                        best->matched_by_normalized_provenance = true;
                        best->lint_count += 1;
                        if (!best_match.warning.empty()) {
                            best->provenance_warnings.push_back(best_match.warning);
                        }
                        best->provenance_proposals.push_back(best_match.proposal);
                    }
                    matched_targets.insert(tgt_path);
                }
            }
        }

        // Second pass: Name-based matching for remaining files.
        // Rust -> Kotlin reports require exact provenance so a plausible
        // filename cannot get a free ride.
        std::vector<std::tuple<float, std::string, std::string>> candidates;

        if (!strict_provenance_matching) {
            for (const auto& [src_path, src_file] : source.files) {
                if (matched_sources.count(src_path)) continue;

                for (const auto& [tgt_path, tgt_file] : target.files) {
                    if (matched_targets.count(tgt_path)) continue;
                    if (is_test_transliteration(tgt_file.transliterated_from)) continue;

                    float score = name_match_score(src_file, tgt_file);
                    if (score > 0.4f) {
                        candidates.emplace_back(score, src_path, tgt_path);
                    }
                }
            }
        }

        // Sort by score descending
        std::sort(candidates.begin(), candidates.end(),
            [](const auto& a, const auto& b) {
                return std::get<0>(a) > std::get<0>(b);
            });

        // Greedy matching: take best matches first
        for (const auto& [score, src_path, tgt_path] : candidates) {
            if (matched_sources.count(src_path) || matched_targets.count(tgt_path)) {
                continue;  // Already matched
            }

            const auto& src_file = source.files.at(src_path);
            const auto& tgt_file = target.files.at(tgt_path);

            Match m;
            m.source_path = src_path;
            m.target_path = tgt_path;
            m.source_qualified = src_file.qualified_name;
            m.target_qualified = tgt_file.qualified_name;
            m.similarity = 0.0f;  // AST similarity computed later
            m.source_dependents = src_file.dependent_count;
            m.target_dependents = tgt_file.dependent_count;
            m.source_lines = src_file.line_count;
            m.target_lines = tgt_file.line_count;
            m.todo_count = tgt_file.todos.size();
            m.lint_count = tgt_file.lint_errors.size();
            m.is_stub = tgt_file.is_stub;
            // File size ratio stub detection (same as header-matched path)
            if (!m.is_stub && src_file.code_lines > 20 && tgt_file.code_lines > 0) {
                float ratio = static_cast<float>(tgt_file.code_lines) /
                              static_cast<float>(src_file.code_lines);
                if (ratio < 0.30f) {
                    m.is_stub = true;
                }
            }
            m.matched_by_header = false;

            matches.push_back(m);
            matched_sources.insert(src_path);
            matched_targets.insert(tgt_path);
        }

        // Find unmatched. Skip module-root files that are purely declarative
        // namespace/export wiring with no Kotlin equivalent:
        //   Rust:   mod.rs (module declaration), lib.rs / main.rs (crate roots)
        //   Python: __init__.py (package marker)
        // JS/TS index files are intentionally NOT excluded: index.ts files
        // contain real implementation code and must be ported.
        auto is_module_root_path = [](const std::string& p) -> bool {
            auto ends_with = [&](const std::string& suffix) {
                return p.size() >= suffix.size() &&
                       p.compare(p.size() - suffix.size(), suffix.size(), suffix) == 0;
            };
            return ends_with("/mod.rs")      || ends_with("\\mod.rs")  ||
                   ends_with("/lib.rs")      || ends_with("\\lib.rs")  ||
                   ends_with("/main.rs")     || ends_with("\\main.rs") ||
                   ends_with("/__init__.py") || ends_with("\\__init__.py");
        };
        for (const auto& [src_path, _] : source.files) {
            if (!matched_sources.count(src_path) && !is_module_root_path(src_path)) {
                unmatched_source.push_back(src_path);
            }
        }
        for (const auto& [tgt_path, _] : target.files) {
            if (!matched_targets.count(tgt_path)) {
                unmatched_target.push_back(tgt_path);
            }
        }
    }

    /**
     * Compute AST similarity for all matches.
     */
    static Language string_to_language(const std::string& lang) {
        if (lang == "rust") return Language::RUST;
        if (lang == "kotlin") return Language::KOTLIN;
        if (lang == "cpp") return Language::CPP;
        if (lang == "python") return Language::PYTHON;
        if (lang == "typescript") return Language::TYPESCRIPT;
        return Language::KOTLIN;  // default
    }

    static FunctionComparisonResult compare_function_sets(
            const std::vector<FunctionInfo>& source_functions,
            const std::vector<FunctionInfo>& target_functions) {
        // Count ALL functions, including #[test]. Previously, test functions
        // were silently excluded from the source set on the assumption that
        // they map to "separate Kotlin test files" — but that is only true if
        // the project actually ports those tests. Suppressing them hides
        // real gaps in test coverage, so we now count them and let the
        // ranking surface the deficit.
        std::vector<const FunctionInfo*> src_prod, tgt_all;
        for (const auto& f : source_functions) {
            src_prod.push_back(&f);
        }
        for (const auto& f : target_functions) {
            tgt_all.push_back(&f);
        }

        FunctionComparisonResult result;
        result.source_total = static_cast<int>(src_prod.size());
        result.target_total = static_cast<int>(tgt_all.size());

        // Track stub/TODO markers per function name.
        //
        // Guardrail intent: prevent Kotlin ports from "faking" bodies with TODO/FIXME/STUB markers.
        //
        // Rust source may legitimately contain TODO/FIXME comments; that should *not* penalize a Kotlin
        // port that is more complete than the Rust source. Therefore, we only treat it as a mismatch
        // when the Kotlin target introduces stub markers that are not present in the corresponding Rust
        // function.
        std::multiset<std::string> src_stub_names;
        std::multiset<std::string> tgt_stub_names;
        for (const auto* func : src_prod) {
            if (func->has_stub_markers) {
                result.has_source_stub = true;
                src_stub_names.insert(IdentifierStats::canonicalize(func->name));
            }
        }
        for (const auto* func : tgt_all) {
            if (func->has_stub_markers) {
                result.has_target_stub = true;
                tgt_stub_names.insert(IdentifierStats::canonicalize(func->name));
            }
        }
        // Count Kotlin-only stub markers (target stubs without matching source stubs).
        std::vector<std::string> kotlin_only;
        std::set_difference(
            tgt_stub_names.begin(), tgt_stub_names.end(),
            src_stub_names.begin(), src_stub_names.end(),
            std::back_inserter(kotlin_only));
        if (!kotlin_only.empty()) {
            result.has_stub_mismatch = true;
            result.stub_mismatch_count = static_cast<int>(kotlin_only.size());
        }

        if (src_prod.empty() || tgt_all.empty()) {
            return result;
        }

        struct FunctionMatchCandidate {
            float score;
            int source_index;
            int target_index;
        };

        std::vector<FunctionMatchCandidate> candidates;
        candidates.reserve(src_prod.size());

        std::unordered_map<std::string, std::vector<int>> target_by_name;
        target_by_name.reserve(tgt_all.size());
        for (int j = 0; j < static_cast<int>(tgt_all.size()); ++j) {
            std::string key = IdentifierStats::canonicalize(tgt_all[j]->name);
            if (key.empty()) {
                key = tgt_all[j]->name;
            }
            target_by_name[key].push_back(j);
        }

        for (int i = 0; i < static_cast<int>(src_prod.size()); ++i) {
            const auto* source_func = src_prod[i];
            std::string key = IdentifierStats::canonicalize(source_func->name);
            if (key.empty()) {
                key = source_func->name;
            }

            auto bucket = target_by_name.find(key);
            if (bucket == target_by_name.end()) {
                continue;
            }

            for (int j : bucket->second) {
                const auto* target_func = tgt_all[j];

                float sim = 0.0f;
                // Guardrail: a Kotlin function body containing stub markers (TODO/FIXME/STUB/etc.)
                // should not score similarity against a real Rust implementation.
                //
                // However, Rust source may contain TODO markers legitimately; do not penalize when
                // the Kotlin target is *more complete* (source has markers, target does not).
                if (!(target_func->has_stub_markers && !source_func->has_stub_markers)) {
                    sim = ASTSimilarity::function_parameter_body_cosine_similarity(
                        source_func->body_tree.get(),
                        target_func->body_tree.get(),
                        source_func->identifiers,
                        target_func->identifiers);
                }

                candidates.push_back({sim, i, j});
            }
        }

        std::sort(candidates.begin(), candidates.end(),
            [](const FunctionMatchCandidate& a, const FunctionMatchCandidate& b) {
                return a.score > b.score;
            });

        std::vector<bool> source_used(src_prod.size(), false);
        std::vector<bool> target_used(tgt_all.size(), false);

        float total_score = 0.0f;
        for (const auto& candidate : candidates) {
            if (source_used[candidate.source_index] || target_used[candidate.target_index]) {
                continue;
            }
            source_used[candidate.source_index] = true;
            target_used[candidate.target_index] = true;
            total_score += candidate.score;
            result.matched_pairs += 1;
        }

        // Score is measured as "how well does the target cover the source".
        //
        // Kotlin ports often contain extra helper methods (Result plumbing, derived trait shims,
        // builders, etc.) which should not penalize the score as long as every source function
        // has a faithful target counterpart.
        int denominator = result.source_total;
        result.unmatched_source = result.source_total - result.matched_pairs;
        result.unmatched_target = result.target_total - result.matched_pairs;
        if (denominator > 0) {
            result.score = total_score / static_cast<float>(denominator);
        } else {
            result.score = 0.0f;
        }

        return result;
    }

    struct FunctionNameCoverage {
        int source_total = 0;
        int target_total = 0;
        int matched = 0;
        int source_test_count = 0;       // how many source functions were #[test]
        int matched_test_count = 0;      // how many of those matched in target
        float ratio = 1.0f;
        std::vector<std::string> missing;
    };

    static FunctionNameCoverage function_name_coverage(
            const std::vector<FunctionInfo>& source_functions,
            const std::vector<FunctionInfo>& target_functions) {
        // How many source functions exist in target, by canonicalized name.
        // This is a parity signal: ports should preserve the function set within a file.
        // Test functions (#[test], #[cfg(test)] mod) ARE counted — a missing
        // test is a real gap that the ranking must surface. (The previous
        // behaviour of silently skipping them hid hundreds of unported tests.)
        FunctionNameCoverage cov;
        // Track target functions by canonical name AND whether they are test-annotated.
        // For a Rust #[test] function to be considered matched, the Kotlin counterpart
        // must also be @Test-annotated — an unannotated `internal fun` with the right
        // name is NOT a match because it will never be executed by the test runner.
        std::multimap<std::string, bool> tgt_by_name;  // name -> is_test
        for (const auto& f : target_functions) {
            if (f.name.empty() || f.name == "<anonymous>") continue;
            tgt_by_name.emplace(IdentifierStats::canonicalize(f.name), f.is_test);
            cov.target_total++;
        }

        std::set<std::string> src_seen;
        for (const auto& f : source_functions) {
            if (f.name.empty() || f.name == "<anonymous>") continue;
            std::string key = IdentifierStats::canonicalize(f.name);
            // Function extraction is name-only here, so repeated trait method names
            // in one source file are counted once per canonical name.
            if (src_seen.count(key)) {
                continue;
            }
            src_seen.insert(key);
            cov.source_total++;
            if (f.is_test) cov.source_test_count++;

            auto range = tgt_by_name.equal_range(key);
            bool matched = false;
            if (range.first != range.second) {
                // Prefer a target whose test-annotation state matches the source.
                // For a #[test] source, only a @Test-annotated Kotlin function
                // is a true match — an unannotated namesake doesn't run.
                auto best = tgt_by_name.end();
                for (auto it = range.first; it != range.second; ++it) {
                    if (it->second == f.is_test) { best = it; break; }
                }
                if (best == tgt_by_name.end() && !f.is_test) {
                    // Non-test source can match any namesake.
                    best = range.first;
                }
                if (best != tgt_by_name.end()) {
                    cov.matched++;
                    if (f.is_test) cov.matched_test_count++;
                    tgt_by_name.erase(best);
                    matched = true;
                }
            }
            if (!matched) {
                cov.missing.push_back(f.name);
            }
        }

        if (cov.source_total > 0) {
            cov.ratio = static_cast<float>(cov.matched) /
                        static_cast<float>(cov.source_total);
        }
        return cov;
    }

    static FunctionNameCoverage function_name_coverage_with_lang(
            const std::vector<FunctionInfo>& source_functions,
            const std::vector<FunctionInfo>& target_functions,
            Language src_lang,
            Language tgt_lang) {
        if (src_lang == Language::RUST && tgt_lang == Language::KOTLIN) {
            // Strict parity: every non-anonymous Rust function name must have a
            // declared Kotlin counterpart after snake_case -> camelCase
            // canonicalization. No ignore lists or synthetic aliases.
            std::vector<FunctionInfo> filtered;
            filtered.reserve(source_functions.size());
            for (const auto& f : source_functions) {
                if (f.name.empty() || f.name == "<anonymous>") {
                    filtered.push_back(f);
                    continue;
                }
                std::string key = IdentifierStats::canonicalize(f.name);
                if (key.empty()) {
                    continue;
                }
                filtered.push_back(f);
            }
            return function_name_coverage(filtered, target_functions);
        }
        return function_name_coverage(source_functions, target_functions);
    }

    static std::string read_file_to_string(const std::string& path) {
        std::ifstream in(path);
        if (!in) return {};
        std::stringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }

    static std::string read_files_to_string(const std::vector<std::string>& paths) {
        std::stringstream ss;
        for (const auto& path : paths) {
            ss << read_file_to_string(path);
            ss << '\n';
        }
        return ss.str();
    }

    struct TypeNameCoverage {
        int source_total = 0;
        int target_total = 0;
        int matched = 0;
        float ratio = 1.0f;
        std::vector<std::string> missing;
    };

    static TypeNameCoverage type_name_coverage(
            TSParser* src_parser,
            TSParser* tgt_parser,
            Language src_lang,
            Language tgt_lang,
            const SourceFile& src_file,
            const SourceFile& tgt_file) {
        TypeNameCoverage cov;

        // Only compute type parity for Rust/C++ -> Kotlin comparisons.
        // For other language pairs, keep coverage neutral.
        if (!((src_lang == Language::RUST || src_lang == Language::CPP) &&
              tgt_lang == Language::KOTLIN)) {
            return cov;
        }

        std::string src_text = read_files_to_string(src_file.paths);
        std::string tgt_text = read_files_to_string(tgt_file.paths);
        if (src_text.empty() || tgt_text.empty()) {
            return cov;
        }

        TSTree* src_tree = ts_parser_parse_string(src_parser, nullptr, src_text.c_str(), src_text.size());
        TSTree* tgt_tree = ts_parser_parse_string(tgt_parser, nullptr, tgt_text.c_str(), tgt_text.size());
        if (!src_tree || !tgt_tree) {
            if (src_tree) ts_tree_delete(src_tree);
            if (tgt_tree) ts_tree_delete(tgt_tree);
            return cov;
        }

        TSNode src_root = ts_tree_root_node(src_tree);
        TSNode tgt_root = ts_tree_root_node(tgt_tree);

        std::vector<SymbolDefinition> src_syms =
            SymbolExtractor::extract_symbols(src_root, src_text, src_file.package.path, src_file.relative_path);
        std::vector<SymbolDefinition> tgt_syms =
            SymbolExtractor::extract_symbols(tgt_root, tgt_text, tgt_file.package.path, tgt_file.relative_path);

        std::set<std::string> tgt_names;
        for (const auto& s : tgt_syms) {
            if (s.name.empty()) continue;
            tgt_names.insert(IdentifierStats::canonicalize(s.name));
        }

        std::set<std::string> src_seen;
        for (const auto& s : src_syms) {
            if (s.name.empty()) continue;
            std::string key = IdentifierStats::canonicalize(s.name);
            if (!src_seen.insert(key).second) continue;  // Deduplicate by canonical name
            cov.source_total++;
            if (tgt_names.count(key)) {
                cov.matched++;
            } else {
                cov.missing.push_back(s.name);
            }
        }
        cov.target_total = static_cast<int>(tgt_names.size());

        if (cov.source_total > 0) {
            cov.ratio = static_cast<float>(cov.matched) /
                        static_cast<float>(cov.source_total);
        }

        ts_tree_delete(src_tree);
        ts_tree_delete(tgt_tree);
        return cov;
    }

    void compute_similarities() {
        ASTParser parser;
        TSParser* symbol_src = ts_parser_new();
        TSParser* symbol_tgt = ts_parser_new();
        if (source.language == "rust") {
            ts_parser_set_language(symbol_src, tree_sitter_rust());
        } else if (source.language == "typescript") {
            ts_parser_set_language(symbol_src, tree_sitter_typescript());
        } else {
            ts_parser_set_language(symbol_src, tree_sitter_cpp());
        }
        if (target.language == "typescript") {
            ts_parser_set_language(symbol_tgt, tree_sitter_typescript());
        } else {
            ts_parser_set_language(symbol_tgt, tree_sitter_kotlin());
        }

        for (auto& m : matches) {
            try {
                m.zero_reasons.clear();
                const auto& src_file = source.files.at(m.source_path);
                const auto& tgt_file = target.files.at(m.target_path);
                auto src_lang = string_to_language(source.language);
                auto tgt_lang = string_to_language(target.language);

                std::vector<std::string> all_target_paths = tgt_file.paths;
                all_target_paths.insert(all_target_paths.end(),
                                        m.additional_target_paths.begin(),
                                        m.additional_target_paths.end());

                bool has_stubs = parser.has_stub_bodies_in_files(all_target_paths, tgt_lang);
                if (has_stubs) {
                    m.zero_reasons.push_back("target contains TODO/stub/placeholder markers in function bodies");
                    m.is_stub = true;
                }

                if (tgt_lang == Language::KOTLIN) {
                    auto contamination = kotlin_contamination_reasons_for_files(all_target_paths);
                    m.zero_reasons.insert(
                        m.zero_reasons.end(),
                        contamination.begin(),
                        contamination.end());
                }

                auto source_functions = parser.extract_function_infos_from_files(
                    src_file.paths, src_lang);
                auto target_functions = parser.extract_function_infos_from_files(
                    all_target_paths, tgt_lang);
                auto fn_cov = function_name_coverage_with_lang(
                    source_functions, target_functions, src_lang, tgt_lang);

                m.source_function_count = fn_cov.source_total;
                m.target_function_count = fn_cov.target_total;
                m.matched_function_count = fn_cov.matched;
                m.function_coverage = fn_cov.ratio;
                m.missing_functions = std::move(fn_cov.missing);
                m.source_test_function_count = fn_cov.source_test_count;
                m.matched_test_function_count = fn_cov.matched_test_count;

                SourceFile combined_tgt_file = tgt_file;
                combined_tgt_file.paths = all_target_paths;
                auto ty_cov = type_name_coverage(
                    symbol_src, symbol_tgt, src_lang, tgt_lang, src_file, combined_tgt_file);
                m.source_type_count = ty_cov.source_total;
                m.target_type_count = ty_cov.target_total;
                m.matched_type_count = ty_cov.matched;
                m.type_coverage = ty_cov.ratio;
                m.missing_types = std::move(ty_cov.missing);

                auto fn_result = compare_function_sets(source_functions, target_functions);
                if (source_functions.empty()) {
                    m.zero_reasons.push_back("no source functions found; report scoring is function-by-function only");
                } else if (target_functions.empty()) {
                    m.zero_reasons.push_back("no target functions found; report scoring is function-by-function only");
                }

                if (!m.zero_reasons.empty()) {
                    m.similarity = 0.0f;
                } else if (fn_result.score >= 0.0f) {
                    m.similarity = fn_result.score;
                } else {
                    m.similarity = 0.0f;
                }

                // Extract documentation statistics
                auto src_docs = parser.extract_comments_from_file(src_file.paths, src_lang);
                auto tgt_docs = parser.extract_comments_from_file(tgt_file.paths, tgt_lang);

                m.source_doc_lines = src_docs.total_doc_lines;
                m.target_doc_lines = tgt_docs.total_doc_lines;
                m.source_doc_comments = src_docs.doc_comment_count;
                m.target_doc_comments = tgt_docs.doc_comment_count;
                m.doc_similarity = src_docs.doc_cosine_similarity(tgt_docs);
                m.doc_coverage = src_docs.doc_line_coverage_capped(tgt_docs);
                m.doc_weighted = 0.5f * m.doc_similarity + 0.5f * m.doc_coverage;
            } catch (...) {
                m.similarity = -1.0f;  // Error
            }
        }

        ts_parser_delete(symbol_src);
        ts_parser_delete(symbol_tgt);
    }

    /**
     * Get matches sorted by priority for porting.
     *
     * Priority = dependents * 1,000,000
     *          + (missing functions + missing types/classes) * 10,000
     *          + (source functions + source types/classes) * 100
     *          + (1 - function similarity) * 10
     *
     * File fanout is the primary driver so the priority ladder favors work
     * that clears downstream compilation failures fastest. Missing symbols are
     * still visible, but a large leaf deficit should not outrank a hot file.
     */
    std::vector<Match> ranked_for_porting() {
        auto result = matches;

        std::sort(result.begin(), result.end(),
            [](const Match& a, const Match& b) {
                return a.priority_score() > b.priority_score();
            });

        return result;
    }

    void print_report() {
        std::cout << "\n=== Codebase Comparison Report ===\n\n";

        std::cout << "Source: " << source.root_path << " (" << source.files.size() << " files)\n";
        std::cout << "Target: " << target.root_path << " (" << target.files.size() << " files)\n";
        std::cout << "Scoring invariant: FnSim is required function body/parameter similarity. "
                  << "Class/type and symbol parity are reported beside it; whole-file shape is diagnostic only.\n";
        std::cout << "\n";

        std::cout << "Matched:   " << matches.size() << " files\n";
        std::cout << "Unmatched: " << unmatched_source.size() << " source, "
                  << unmatched_target.size() << " target\n\n";

        if (!matches.empty()) {
            std::cout << "=== Matched Files (by porting priority) ===\n\n";
            std::cout << std::setw(30) << std::left << "Source"
                      << std::setw(30) << "Target"
                      << std::setw(10) << "FnSim"
                      << std::setw(11) << "Dependents"
                      << std::setw(14) << "FunctionParity"
                      << std::setw(12) << "TypeParity"
                      << std::setw(10) << "Priority\n";
            std::cout << std::string(122, '-') << "\n";

            auto ranked = ranked_for_porting();
            for (const auto& m : ranked) {
                std::string funcs = "0/0";
                if (m.source_function_count > 0 || m.target_function_count > 0) {
                    funcs = std::to_string(m.matched_function_count) + "/" +
                            std::to_string(m.source_function_count);
                }
                std::string types = "0/0";
                if (m.source_type_count > 0 || m.target_type_count > 0) {
                    types = std::to_string(m.matched_type_count) + "/" +
                            std::to_string(m.source_type_count);
                }
                float priority = m.priority_score();
                std::string match_flags = m.is_stub ? " [STUB]" : "";
                if (!m.zero_reasons.empty() && !m.is_stub) match_flags = " [ZERO]";
                if (m.matched_by_normalized_provenance) {
                    match_flags += " [PROVENANCE-FALLBACK]";
                }
                std::cout << std::setw(30) << std::left << m.source_qualified.substr(0, 28)
                          << std::setw(30) << (m.target_qualified.substr(0, 28) + match_flags)
                          << std::setw(10) << std::fixed << std::setprecision(2) << m.similarity
                          << std::setw(11) << m.source_dependents
                          << std::setw(14) << funcs
                          << std::setw(12) << types
                          << std::setw(10) << std::fixed << std::setprecision(1) << priority
                          << "\n";
            }

            auto join_names = [](const std::vector<std::string>& names) {
                if (names.empty()) return std::string("none");
                std::ostringstream out;
                for (size_t i = 0; i < names.size(); ++i) {
                    if (i > 0) out << ", ";
                    out << names[i];
                }
                return out.str();
            };

            std::cout << "\n=== Function and Symbol Details ===\n\n";
            for (const auto& m : ranked) {
                std::cout << m.source_qualified << " -> " << m.target_qualified;
                if (m.is_stub) std::cout << " [STUB]";
                if (!m.zero_reasons.empty() && !m.is_stub) std::cout << " [ZERO]";
                if (m.matched_by_normalized_provenance) std::cout << " [PROVENANCE-FALLBACK]";
                std::cout << "\n";
                std::cout << "  similarity: " << std::fixed << std::setprecision(2) << m.similarity
                          << ", priority: " << std::fixed << std::setprecision(1) << m.priority_score()
                          << ", dependents: " << m.source_dependents << "\n";
                for (const auto& warning : m.provenance_warnings) {
                    std::cout << "  provenance warning: " << warning << "\n";
                }
                std::cout << "  functions: " << m.matched_function_count << "/"
                          << m.source_function_count << " matched"
                          << " (target total: " << m.target_function_count
                          << ", required body score: " << std::fixed << std::setprecision(2)
                          << m.similarity << ")\n";
                std::cout << "  missing functions: " << join_names(m.missing_functions) << "\n";
                std::cout << "  types: " << m.matched_type_count << "/"
                          << m.source_type_count << " matched"
                          << " (target total: " << m.target_type_count << ")\n";
                std::cout << "  missing types: " << join_names(m.missing_types) << "\n";
                if (!m.zero_reasons.empty()) {
                    std::cout << "  *** CHEAT DETECTION / SCORING FAILURE ***\n";
                    std::cout << "  function-by-function score forced to 0: "
                              << join_reasons(m.zero_reasons) << "\n";
                }
                if (m.source_test_function_count > 0) {
                    std::cout << "  tests: " << m.matched_test_function_count << "/"
                              << m.source_test_function_count << " matched\n";
                }
                std::cout << "\n";
            }

            bool any_zeroed = false;
            for (const auto& m : ranked) {
                if (!m.zero_reasons.empty()) {
                    any_zeroed = true;
                    break;
                }
            }
            if (any_zeroed) {
                std::cout << "\n=== Scores Forced To 0 ===\n\n";
                for (const auto& m : ranked) {
                    if (!m.zero_reasons.empty()) {
                        std::cout << "  - " << m.source_qualified << " -> "
                                  << m.target_qualified << ": "
                                  << join_reasons(m.zero_reasons) << "\n";
                    }
                }
            }

            bool any_provenance_warning = false;
            for (const auto& m : ranked) {
                if (!m.provenance_warnings.empty()) {
                    any_provenance_warning = true;
                    break;
                }
            }
            if (any_provenance_warning) {
                std::cout << "\n=== Provenance Header Fallbacks ===\n\n";
                std::cout << "These files were paired only after normalization; fix the port-lint source header.\n";
                for (const auto& m : ranked) {
                    for (size_t i = 0; i < m.provenance_warnings.size(); ++i) {
                        const auto& warning = m.provenance_warnings[i];
                        std::cout << "  - " << m.source_qualified << " -> "
                                  << m.target_qualified << ": " << warning << "\n";
                        if (i < m.provenance_proposals.size()) {
                            std::cout << "    proposed: "
                                      << m.provenance_proposals[i].proposed_header << "\n";
                        }
                    }
                }
            }
        }

        if (!unmatched_source.empty()) {
            std::cout << "\n=== Missing from Target (need to port) ===\n\n";
            std::vector<const SourceFile*> missing;
            missing.reserve(unmatched_source.size());
            for (const auto& path : unmatched_source) {
                missing.push_back(&source.files.at(path));
            }
            std::sort(missing.begin(), missing.end(),
                      [](const SourceFile* a, const SourceFile* b) {
                          return a->dependent_count > b->dependent_count;
                      });

            std::cout << std::setw(30) << std::left << "File"
                      << std::setw(8) << "Deps"
                      << "Path\n";
            std::cout << std::string(78, '-') << "\n";
            for (const auto* sf : missing) {
                std::cout << std::setw(30) << std::left << sf->qualified_name.substr(0, 28)
                          << std::setw(8) << sf->dependent_count
                          << sf->relative_path << "\n";
            }
        }
    }
};

} // namespace ast_distance
