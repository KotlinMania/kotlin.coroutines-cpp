#pragma once

#include "imports.hpp"
#include "ast_parser.hpp"
#include "similarity.hpp"
#include "porting_utils.hpp"
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
#include <unordered_set>

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
            const std::string marker = "/src/commonMain/kotlin/";
            const auto pos = root_path.find(marker);
            if (pos != std::string::npos) {
                const std::string repo_root = root_path.substr(0, pos);
                const std::string suffix = root_path.substr(pos + marker.size());
                const fs::path test_root = fs::path(repo_root) / "src" / "commonTest" / "kotlin" / suffix;
                if (fs::exists(test_root) && fs::is_directory(test_root)) {
                    roots_to_scan.push_back(test_root);
                    // Use repo root for relative paths so commonMain/commonTest remain distinct.
                    rel_base = repo_root;
                }
            }
        }

        for (const auto& scan_root : roots_to_scan) {
            if (!fs::exists(scan_root) || !fs::is_directory(scan_root)) continue;
            for (const auto& entry : fs::recursive_directory_iterator(scan_root)) {
                if (!entry.is_regular_file()) continue;

                std::string path = entry.path().string();
                if (!has_valid_ext(path)) continue;

                // Skip build artifacts (but NOT test files - they need parity too)
                if (path.find("/target/") != std::string::npos ||
                    path.find("/build/") != std::string::npos ||
                    path.find("/build_") != std::string::npos ||
                    path.find("/_deps/") != std::string::npos) {
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

        // Function parity (name-based) within a file. Used to prevent "signature-only" stubs.
        int source_function_count = 0;
        int target_function_count = 0;
        int matched_function_count = 0;
        float function_coverage = 1.0f;  // matched / source

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

        /**
         * Porting priority score.
         *
         * Design rationale:
         *   - The old formula was `dependents * (1 - similarity)`, which zeroes
         *     out any file with no dependents (every test file, every leaf
         *     module). That hid hundreds of real gaps.
         *   - The new formula makes *symbol deficit* (missing functions +
         *     missing types) the primary driver — a file with 10 missing
         *     functions ranks above a file that merely has low AST similarity.
         *   - Import depth (number of files depending on this one) is kept as
         *     a multiplicative boost, but log-scaled so that high-fanout files
         *     don't drown out standalone files with real deficits.
         *   - Similarity gap is a tiebreaker for files with no explicit
         *     symbol-level gap.
         */
        float priority_score() const {
            float deficit = static_cast<float>(symbol_deficit());
            float import_depth = std::log1p(static_cast<float>(source_dependents));
            float sim_gap = std::max(0.0f, 1.0f - similarity);

            // 10 points per missing symbol; each missing symbol is amplified
            // by how many downstream files will benefit once it's restored.
            float deficit_score = deficit * (10.0f + import_depth * 2.0f);

            // For files with no explicit symbol-level gap, fall back to the
            // old AST-similarity × dependents signal.
            float shape_score = import_depth * sim_gap * 5.0f;

            return deficit_score + shape_score;
        }
    };

    std::vector<Match> matches;
    std::vector<std::string> unmatched_source;
    std::vector<std::string> unmatched_target;

    CodebaseComparator(Codebase& src, Codebase& tgt)
        : source(src), target(tgt) {}

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

    /**
     * Find matching files between codebases.
     * Priority: 1) "Transliterated from:" headers, 2) Name matching
     */
    void find_matches() {
        std::set<std::string> matched_sources;
        std::set<std::string> matched_targets;

        // First pass: Match by "Transliterated from:" header
        // Target files reference source files, so look in target for headers
        // Store candidates with scores for best matching
        std::vector<std::tuple<float, std::string, std::string>> header_candidates;

        for (const auto& [tgt_path, tgt_file] : target.files) {
            if (tgt_file.transliterated_from.empty()) continue;

            // Try to find the source file that matches the header
            for (const auto& [src_path, src_file] : source.files) {
                float match_score = 0.0f;

                // Check if transliterated_from contains the FULL relative path (most precise)
                if (tgt_file.transliterated_from.find(src_file.relative_path) != std::string::npos) {
                    match_score = 1.0f;  // Full path match
                }
                // Check if transliterated_from ends with the EXACT filename (not substring)
                // Use ends_with to avoid Flow.kt matching StateFlow.kt
                else if (tgt_file.transliterated_from.ends_with("/" + src_file.filename) ||
                         tgt_file.transliterated_from == src_file.filename) {
                    // Also verify directory context matches
                    std::string tgt_dir = tgt_file.qualified_name;
                    std::string src_dir = src_file.qualified_name;
                    size_t tgt_dot = tgt_dir.rfind('.');
                    size_t src_dot = src_dir.rfind('.');
                    if (tgt_dot != std::string::npos) tgt_dir = tgt_dir.substr(0, tgt_dot);
                    if (src_dot != std::string::npos) src_dir = src_dir.substr(0, src_dot);

                    if (SourceFile::normalize_name(tgt_dir) == SourceFile::normalize_name(src_dir)) {
                        match_score = 0.9f;  // Filename + directory context match
                    } else {
                        match_score = 0.5f;  // Just filename match, different directory
                    }
                }
                // Loose check - transliterated_from ends with stem (stricter than find)
                else if (tgt_file.transliterated_from.ends_with("/" + src_file.stem + ".kt") ||
                         tgt_file.transliterated_from.ends_with("/" + src_file.stem + ".rs") ||
                         tgt_file.transliterated_from.ends_with("/" + src_file.stem + ".py") ||
                         tgt_file.transliterated_from.ends_with("/" + src_file.stem + ".cpp") ||
                         tgt_file.transliterated_from.ends_with("/" + src_file.stem + ".cc") ||
                         tgt_file.transliterated_from.ends_with("/" + src_file.stem + ".hpp") ||
                         tgt_file.transliterated_from.ends_with("/" + src_file.stem + ".h")) {
                    match_score = 0.3f;  // Stem found but not as confident
                }

                if (match_score > 0.0f) {
                    header_candidates.emplace_back(match_score, src_path, tgt_path);
                }
            }
        }

        // Sort by score descending, with header preference for ties
        std::sort(header_candidates.begin(), header_candidates.end(),
            [this](const auto& a, const auto& b) {
                float score_a = std::get<0>(a);
                float score_b = std::get<0>(b);
                if (std::abs(score_a - score_b) > 0.001f) {
                    return score_a > score_b;  // Higher score first
                }
                // Same score - prefer header files
                const auto& tgt_a = target.files.at(std::get<2>(a));
                const auto& tgt_b = target.files.at(std::get<2>(b));
                bool a_header = is_header_file(tgt_a);
                bool b_header = is_header_file(tgt_b);
                if (a_header != b_header) return a_header;  // Headers first
                // Same type - prefer shorter path (less nesting)
                return std::get<2>(a).size() < std::get<2>(b).size();
            });

        for (const auto& [score, src_path, tgt_path] : header_candidates) {
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

            matches.push_back(m);
            matched_sources.insert(src_path);
            matched_targets.insert(tgt_path);
        }

        // Second pass: Name-based matching for remaining files
        std::vector<std::tuple<float, std::string, std::string>> candidates;

        for (const auto& [src_path, src_file] : source.files) {
            if (matched_sources.count(src_path)) continue;

            for (const auto& [tgt_path, tgt_file] : target.files) {
                if (matched_targets.count(tgt_path)) continue;

                float score = name_match_score(src_file, tgt_file);
                if (score > 0.4f) {
                    candidates.emplace_back(score, src_path, tgt_path);
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
        candidates.reserve(src_prod.size() * tgt_all.size());

        for (int i = 0; i < static_cast<int>(src_prod.size()); ++i) {
            const auto* source_func = src_prod[i];
            for (int j = 0; j < static_cast<int>(tgt_all.size()); ++j) {
                const auto* target_func = tgt_all[j];

                float sim = 0.0f;
                // Guardrail: a Kotlin function body containing stub markers (TODO/FIXME/STUB/etc.)
                // should not score similarity against a real Rust implementation.
                //
                // However, Rust source may contain TODO markers legitimately; do not penalize when
                // the Kotlin target is *more complete* (source has markers, target does not).
                if (!(target_func->has_stub_markers && !source_func->has_stub_markers)) {
                    sim = ASTSimilarity::combined_similarity_with_content(
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
            // Rust `Drop` impl methods appear as a `drop` function in function extraction.
            // Kotlin ports have no direct equivalent, so do not require it for parity.
            if (key == "drop") {
                continue;
            }
            // Rust often has duplicate trait method names (e.g. multiple `fmt` impls).
            // In Kotlin ports these typically collapse into a single canonical method
            // (e.g. `toString`), so only require each canonical name once.
            if (src_seen.count(key)) {
                continue;
            }
            src_seen.insert(key);
            cov.source_total++;
            if (f.is_test) cov.source_test_count++;

            auto range = tgt_by_name.equal_range(key);
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
                }
            }
        }

        if (cov.source_total > 0) {
            cov.ratio = static_cast<float>(cov.matched) /
                        static_cast<float>(cov.source_total);
        }
        return cov;
    }

    static bool rust_kotlin_ignorable_function_name_for_coverage(const std::string& canonical_name) {
        // Rust trait impl methods frequently appear as small, repeated function names
        // (e.g. `fmt`, `eq`) that don't exist as explicit functions in Kotlin ports
        // (they map to `toString`/`equals`/`hashCode`).
        //
        // Requiring these names for parity creates systematic false negatives on
        // faithful Rust→Kotlin transliterations.
        //
        // Keep this list small and explicit to preserve the strength of function-set
        // parity as a guardrail against logic rewrites.
        static const std::unordered_set<std::string> k = {
            "fmt",
            "eq",
            "hash",
            "partialcmp",
            "cmp",
            "equivalent",
            "default",
            "serialize",
            // Rust assigns via traits (`AddAssign`, `SubAssign`, `MulAssign`) which don't appear as
            // explicit function names in Kotlin (Kotlin desugars `+=` to `a = a + b` when no
            // `plusAssign` exists, which is the correct pattern for immutable value types).
            "addassign",
            "subassign",
            "mulassign",
            // Local helper in Rust (`fn split_at_safe` inside `display_for_type_error`).
            // Kotlin ports may keep this helper local or inline it, and Kotlin function
            // extraction is less reliable for nested locals, so do not require it.
            "splitatsafe",
        };
        return k.find(canonical_name) != k.end();
    }

    static std::vector<FunctionInfo> rust_kotlin_augment_target_functions_for_coverage(
            const std::vector<FunctionInfo>& target_functions) {
        // Function-name coverage is a guardrail: it ensures ports preserve the set of meaningful
        // behaviors within a file. However, some Rust behaviors are expressed through trait
        // methods whose names do not exist literally in Kotlin ports:
        //   - `Mul::mul` maps to `operator fun times(...)`
        //   - `Ord::cmp` / `PartialOrd::partial_cmp` map to `compareTo`
        //   - `Add::add` / `Sub::sub` may map to `plus` / `minus` operator functions
        //
        // To avoid false negatives on faithful transliterations, we augment the Kotlin function
        // set with a small, explicit set of canonical equivalents for coverage matching only.
        std::vector<FunctionInfo> out = target_functions;

        std::unordered_set<std::string> present;
        present.reserve(target_functions.size());
        bool has_to_string_raw = false;
        bool has_compare_to_raw = false;
        for (const auto& f : target_functions) {
            if (f.name.empty() || f.name == "<anonymous>") continue;
            if (f.name == "toString") has_to_string_raw = true;
            if (f.name == "compareTo") has_compare_to_raw = true;
            present.insert(IdentifierStats::canonicalize(f.name));
        }

        auto add_if_missing = [&](const std::string& name) {
            std::string key = IdentifierStats::canonicalize(name);
            if (present.find(key) != present.end()) return;
            FunctionInfo fi;
            fi.name = name;
            out.push_back(std::move(fi));
            present.insert(std::move(key));
        };

        if (present.find("times") != present.end()) {
            add_if_missing("mul");
        }
        if (present.find("compareto") != present.end() || has_compare_to_raw) {
            add_if_missing("cmp");
            add_if_missing("partial_cmp");
        }
        if (present.find("plus") != present.end()) {
            add_if_missing("add");
        }
        if (present.find("minus") != present.end()) {
            add_if_missing("sub");
        }
        if (present.find("hashcode") != present.end()) {
            add_if_missing("hash");
        }
        if (present.find("tostring") != present.end() || has_to_string_raw) {
            add_if_missing("fmt");
        }
        if (present.find("to") != present.end()) {
            add_if_missing("bitor");
        }

        return out;
    }

    static FunctionNameCoverage function_name_coverage_with_lang(
            const std::vector<FunctionInfo>& source_functions,
            const std::vector<FunctionInfo>& target_functions,
            Language src_lang,
            Language tgt_lang) {
        if (src_lang == Language::RUST && tgt_lang == Language::KOTLIN) {
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
                if (rust_kotlin_ignorable_function_name_for_coverage(key)) {
                    continue;
                }
                filtered.push_back(f);
            }
            auto tgt_augmented = rust_kotlin_augment_target_functions_for_coverage(target_functions);
            return function_name_coverage(filtered, tgt_augmented);
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

        std::string src_text = src_file.paths.empty() ? "" : read_file_to_string(src_file.paths.front());
        std::string tgt_text = tgt_file.paths.empty() ? "" : read_file_to_string(tgt_file.paths.front());
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
        } else {
            ts_parser_set_language(symbol_src, tree_sitter_cpp());
        }
        ts_parser_set_language(symbol_tgt, tree_sitter_kotlin());

        for (auto& m : matches) {
            try {
                const auto& src_file = source.files.at(m.source_path);
                const auto& tgt_file = target.files.at(m.target_path);
                auto src_lang = string_to_language(source.language);
                auto tgt_lang = string_to_language(target.language);

                bool has_stubs = parser.has_stub_bodies_in_files(
                    tgt_file.paths, tgt_lang);

                if (has_stubs) {
                    m.similarity = 0.0f;
                    m.is_stub = true;
                } else {
                    // Whole-document similarity is the default metric.
                    auto src_tree = parser.parse_file(src_file.paths, src_lang);
                    auto tgt_tree = parser.parse_file(tgt_file.paths, tgt_lang);

                    // Normalize ASTs: Flatten namespaces/packages to reduce structural noise
                    // Node type 82 is PACKAGE (includes C++ namespaces)
                    if (src_tree) src_tree->flatten_node_type(82);
                    if (tgt_tree) tgt_tree->flatten_node_type(82);

                    auto src_ids = parser.extract_identifiers_from_file(
                        src_file.paths, src_lang);
                    auto tgt_ids = parser.extract_identifiers_from_file(
                        tgt_file.paths, tgt_lang);

                    float file_sim = ASTSimilarity::combined_similarity_with_content(
                        src_tree.get(), tgt_tree.get(), src_ids, tgt_ids);

                    // Parity penalty: if target is missing functions (by name),
                    // reduce the score even if the file-level shape looks similar.
                    auto source_functions = parser.extract_function_infos_from_files(
                        src_file.paths, src_lang);
                    auto target_functions = parser.extract_function_infos_from_files(
                        tgt_file.paths, tgt_lang);
                    auto fn_cov = function_name_coverage_with_lang(
                        source_functions, target_functions, src_lang, tgt_lang);

                    m.source_function_count = fn_cov.source_total;
                    m.target_function_count = fn_cov.target_total;
                    m.matched_function_count = fn_cov.matched;
                    m.function_coverage = fn_cov.ratio;
                    m.source_test_function_count = fn_cov.source_test_count;
                    m.matched_test_function_count = fn_cov.matched_test_count;

                    auto ty_cov = type_name_coverage(
                        symbol_src, symbol_tgt, src_lang, tgt_lang, src_file, tgt_file);
                    m.source_type_count = ty_cov.source_total;
                    m.target_type_count = ty_cov.target_total;
                    m.matched_type_count = ty_cov.matched;
                    m.type_coverage = ty_cov.ratio;
                    m.missing_types = std::move(ty_cov.missing);

                    // Port completeness gates primarily on semantic code similarity:
                    //   similarity = content_aware_ast_similarity * function_name_coverage
                    // Type coverage is still reported separately (m.type_coverage) but is not
                    // folded into the similarity score to avoid false negatives for faithful
                    // transliterations that must introduce Kotlin-only type names.
                    m.similarity = file_sim * fn_cov.ratio;
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
     * Priority = (missing functions + missing types) × (10 + log1p(dependents) × 2)
     *          + log1p(dependents) × (1 - similarity) × 5
     *
     * Symbol deficits are the primary driver. Import depth is a
     * multiplicative boost for deficits, and a secondary signal on its own
     * when deficits are zero. See Match::priority_score() for the full
     * rationale — this replaces the old `dependents × (1 - similarity)`
     * formula, which gave priority 0 to any file with no dependents (every
     * test file, every leaf module) and hid hundreds of real gaps.
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
        std::cout << "\n";

        std::cout << "Matched:   " << matches.size() << " files\n";
        std::cout << "Unmatched: " << unmatched_source.size() << " source, "
                  << unmatched_target.size() << " target\n\n";

        if (!matches.empty()) {
		            std::cout << "=== Matched Files (by porting priority) ===\n\n";
		            std::cout << std::setw(30) << std::left << "Source"
		                      << std::setw(30) << "Target"
		                      << std::setw(10) << "Similarity"
		                      << std::setw(11) << "Dependents"
		                      << std::setw(14) << "FunctionParity"
		                      << std::setw(12) << "TypeParity"
		                      << std::setw(10) << "Priority\n";
		            std::cout << std::string(122, '-') << "\n";

	            auto ranked = ranked_for_porting();
	            for (const auto& m : ranked) {
	                std::string funcs = "-";
	                if (m.source_function_count > 0) {
	                    funcs = std::to_string(m.matched_function_count) + "/" +
	                            std::to_string(m.source_function_count);
	                }
	                std::string types = "-";
	                if (m.source_type_count > 0) {
	                    types = std::to_string(m.matched_type_count) + "/" +
	                            std::to_string(m.source_type_count);
	                }
	                float priority = m.priority_score();
		                std::string stub_flag = m.is_stub ? " [STUB]" : "";
	                std::cout << std::setw(30) << std::left << m.source_qualified.substr(0, 28)
		                          << std::setw(30) << (m.target_qualified.substr(0, 28) + stub_flag)
		                          << std::setw(10) << std::fixed << std::setprecision(2) << m.similarity
		                          << std::setw(11) << m.source_dependents
		                          << std::setw(14) << funcs
		                          << std::setw(12) << types
		                          << std::setw(10) << std::fixed << std::setprecision(1) << priority
		                          << "\n";
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
	            int shown = 0;
	            for (const auto* sf : missing) {
	                if (shown++ >= 20) {
	                    std::cout << "... and " << (missing.size() - 20) << " more missing files\n";
	                    break;
	                }
	                std::cout << std::setw(30) << std::left << sf->qualified_name.substr(0, 28)
	                          << std::setw(8) << sf->dependent_count
	                          << sf->relative_path << "\n";
	            }
	        }
	    }
	};

} // namespace ast_distance
