#pragma once

#include "imports.hpp"
#include "ast_parser.hpp"
#include "similarity.hpp"
#include "porting_utils.hpp"
#include <filesystem>
#include <map>
#include <set>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <regex>

namespace fs = std::filesystem;

namespace ast_distance {

/**
 * Represents a source file with its metadata.
 */
struct SourceFile {
    std::string path;              // Full path
    std::string relative_path;     // Relative to root
    std::string filename;          // Just the filename (e.g., "Block.kt")
    std::string stem;              // Filename without extension (e.g., "Block")
    std::string qualified_name;    // Disambiguated name (e.g., "widgets.Block")
    std::string extension;         // File extension

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
};

/**
 * Manages a codebase - scans files, extracts imports, builds dependency graph.
 */
class Codebase {
public:
    std::string root_path;
    std::string language;  // "rust" or "kotlin"
    std::map<std::string, SourceFile> files;  // keyed by relative path
    std::map<std::string, std::vector<std::string>> by_stem;  // stem -> list of files
    std::map<std::string, std::string> by_qualified;  // qualified_name -> relative_path

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
            }
            return false;
        };

        for (const auto& entry : fs::recursive_directory_iterator(root_path)) {
            if (!entry.is_regular_file()) continue;

            std::string path = entry.path().string();
            if (!has_valid_ext(path)) continue;

            // Skip test files and build artifacts
            if (path.find("/test") != std::string::npos ||
                path.find("/target/") != std::string::npos ||
                path.find("/build/") != std::string::npos) {
                continue;
            }

            SourceFile sf;
            sf.path = path;
            sf.relative_path = fs::relative(path, root_path).string();
            sf.filename = entry.path().filename().string();
            sf.stem = entry.path().stem().string();
            sf.extension = entry.path().extension().string();
            sf.qualified_name = SourceFile::make_qualified_name(sf.relative_path);

            files[sf.relative_path] = sf;
            by_stem[sf.stem].push_back(sf.relative_path);
            by_qualified[sf.qualified_name] = sf.relative_path;
        }

        // Handle duplicates - if multiple files have same stem, use qualified names
        for (auto& [stem, paths] : by_stem) {
            if (paths.size() > 1) {
                // Multiple files with same stem - ensure unique qualified names
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
            sf.imports = extractor.extract_from_file(sf.path);
            sf.package = extractor.extract_package_from_file(sf.path);
            sf.dependency_count = sf.imports.size();
        }
    }

    /**
     * Extract porting analysis data (transliterated_from, TODOs, lint, line counts).
     */
    void extract_porting_data() {
        for (auto& [path, sf] : files) {
            // Extract "Transliterated from:" header
            sf.transliterated_from = PortingAnalyzer::extract_transliterated_from(sf.path);

            // Get file stats
            FileStats stats = PortingAnalyzer::analyze_file(sf.path);
            sf.line_count = stats.line_count;
            sf.code_lines = stats.code_lines;
            sf.is_stub = stats.is_stub;
            sf.todos = stats.todos;

            // Run lint checks
            sf.lint_errors = PortingAnalyzer::lint_file(sf.path);
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
    };

    std::vector<Match> matches;
    std::vector<std::string> unmatched_source;
    std::vector<std::string> unmatched_target;

    CodebaseComparator(Codebase& src, Codebase& tgt)
        : source(src), target(tgt) {}

    /**
     * Compute name match score between two files.
     * Returns 0.0 for no match, 1.0 for perfect match.
     */
    static float name_match_score(const SourceFile& src, const SourceFile& tgt) {
        std::string src_norm = SourceFile::normalize_name(src.stem);
        std::string tgt_norm = SourceFile::normalize_name(tgt.stem);

        // Exact match
        if (src_norm == tgt_norm) {
            return 1.0f;
        }

        // Check if one contains the other (handles RatatuiLogo vs logo)
        if (tgt_norm.find(src_norm) != std::string::npos) {
            // tgt contains src (e.g., "ratatuilogo" contains "logo")
            float ratio = static_cast<float>(src_norm.length()) / tgt_norm.length();
            return 0.7f + 0.3f * ratio;  // 0.7-1.0 range
        }
        if (src_norm.find(tgt_norm) != std::string::npos) {
            // src contains tgt
            float ratio = static_cast<float>(tgt_norm.length()) / src_norm.length();
            return 0.7f + 0.3f * ratio;
        }

        // Package path similarity
        if (!src.package.parts.empty() && !tgt.package.parts.empty()) {
            float pkg_sim = src.package.similarity_to(tgt.package);
            if (pkg_sim > 0.5f) {
                return pkg_sim * 0.8f;  // Package match is good but not as strong
            }
        }

        // Check if last package component matches filename
        if (!src.package.parts.empty()) {
            std::string src_last = PackageDecl::normalize(src.package.last());
            if (src_last == tgt_norm || tgt_norm.find(src_last) != std::string::npos) {
                return 0.6f;
            }
        }
        if (!tgt.package.parts.empty()) {
            std::string tgt_last = PackageDecl::normalize(tgt.package.last());
            if (tgt_last == src_norm || src_norm.find(tgt_last) != std::string::npos) {
                return 0.6f;
            }
        }

        // Common suffix matching (Monthly vs calendar - look at parent dir)
        // e.g., calendar/Monthly.kt should match calendar.rs
        std::string src_parent = src.qualified_name;
        std::string tgt_parent = tgt.qualified_name;
        size_t src_dot = src_parent.rfind('.');
        size_t tgt_dot = tgt_parent.rfind('.');
        if (src_dot != std::string::npos) src_parent = src_parent.substr(0, src_dot);
        if (tgt_dot != std::string::npos) tgt_parent = tgt_parent.substr(0, tgt_dot);

        if (SourceFile::normalize_name(src_parent) == SourceFile::normalize_name(tgt_parent) &&
            !src_parent.empty()) {
            return 0.5f;  // Same parent directory
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
        for (const auto& [tgt_path, tgt_file] : target.files) {
            if (tgt_file.transliterated_from.empty()) continue;

            // Try to find the source file that matches the header
            for (const auto& [src_path, src_file] : source.files) {
                // Check if transliterated_from contains this source file's path
                if (tgt_file.transliterated_from.find(src_file.filename) != std::string::npos ||
                    src_file.path.find(tgt_file.transliterated_from) != std::string::npos ||
                    tgt_file.transliterated_from.find(src_file.stem) != std::string::npos) {

                    if (matched_sources.count(src_path)) continue;

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
                    m.matched_by_header = true;

                    matches.push_back(m);
                    matched_sources.insert(src_path);
                    matched_targets.insert(tgt_path);
                    break;
                }
            }
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
            m.matched_by_header = false;

            matches.push_back(m);
            matched_sources.insert(src_path);
            matched_targets.insert(tgt_path);
        }

        // Find unmatched
        for (const auto& [src_path, _] : source.files) {
            if (!matched_sources.count(src_path)) {
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
        return Language::KOTLIN;  // default
    }

    void compute_similarities() {
        ASTParser parser;

        for (auto& m : matches) {
            try {
                auto src_tree = parser.parse_file(
                    source.root_path + "/" + m.source_path,
                    string_to_language(source.language));
                auto tgt_tree = parser.parse_file(
                    target.root_path + "/" + m.target_path,
                    string_to_language(target.language));

                m.similarity = ASTSimilarity::combined_similarity(
                    src_tree.get(), tgt_tree.get());
            } catch (...) {
                m.similarity = -1.0f;  // Error
            }
        }
    }

    /**
     * Get matches sorted by priority for porting.
     * Priority: high dependents + low similarity = needs attention
     */
    std::vector<Match> ranked_for_porting() {
        auto result = matches;

        std::sort(result.begin(), result.end(),
            [](const Match& a, const Match& b) {
                // Score = dependents * (1 - similarity)
                // Higher score = more important to port
                float score_a = a.source_dependents * (1.0f - a.similarity);
                float score_b = b.source_dependents * (1.0f - b.similarity);
                return score_a > score_b;
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
                      << std::setw(10) << "Sim"
                      << std::setw(8) << "Deps"
                      << std::setw(10) << "Priority\n";
            std::cout << std::string(88, '-') << "\n";

            auto ranked = ranked_for_porting();
            for (const auto& m : ranked) {
                float priority = m.source_dependents * (1.0f - m.similarity);
                std::cout << std::setw(30) << std::left << m.source_qualified.substr(0, 28)
                          << std::setw(30) << m.target_qualified.substr(0, 28)
                          << std::setw(10) << std::fixed << std::setprecision(2) << m.similarity
                          << std::setw(8) << m.source_dependents
                          << std::setw(10) << std::fixed << std::setprecision(1) << priority
                          << "\n";
            }
        }

        if (!unmatched_source.empty()) {
            std::cout << "\n=== Missing from Target (need to port) ===\n";
            for (const auto& path : unmatched_source) {
                const auto& sf = source.files.at(path);
                std::cout << "  " << std::setw(30) << std::left << sf.qualified_name
                          << " (" << sf.dependent_count << " dependents)\n";
            }
        }
    }
};

} // namespace ast_distance
