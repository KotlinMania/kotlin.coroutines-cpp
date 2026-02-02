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
    std::vector<std::string> paths; // All physical paths (e.g., .hpp and .cpp)
    std::string relative_path;     // Representative relative path
    std::string filename;          // Representative filename
    std::string stem;              // Stem shared by all paths
    std::string qualified_name;    // Logical name for matching
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

        if (fs::is_regular_file(root_path)) {
            // Handle single file input
            if (has_valid_ext(root_path)) {
                SourceFile sf;
                sf.paths.push_back(root_path);
                sf.relative_path = fs::path(root_path).filename().string();
                sf.filename = fs::path(root_path).filename().string();
                sf.stem = fs::path(root_path).stem().string();
                sf.extension = fs::path(root_path).extension().string();
                sf.qualified_name = SourceFile::make_qualified_name(sf.relative_path);

                files[sf.stem] = sf;
                by_stem[sf.stem].push_back(sf.stem);
                by_qualified[sf.qualified_name] = sf.stem;
            }
            return;
        }

        for (const auto& entry : fs::recursive_directory_iterator(root_path)) {
            if (!entry.is_regular_file()) continue;

            std::string path = entry.path().string();
            if (!has_valid_ext(path)) continue;

            // Skip build artifacts
            if (path.find("/target/") != std::string::npos ||
                path.find("/build/") != std::string::npos) {
                continue;
            }

            std::string rel_path = fs::relative(path, root_path).string();
            std::string stem = entry.path().stem().string();
            std::string filename = entry.path().filename().string();
            std::string extension = entry.path().extension().string();

            // Logical grouping for C++: same stem in same directory
            std::string directory = fs::path(rel_path).parent_path().string();
            std::string logical_key = directory.empty() ? stem : directory + "/" + stem;

            if (files.count(logical_key)) {
                files[logical_key].paths.push_back(path);
                // Prefer .hpp as the representative extension/filename if pairing
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
                
                // Also aggregate package if not set
                if (sf.package.parts.empty()) {
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
            for (const auto& p : sf.paths) {
                // Extract "Transliterated from:" header (first one wins or aggregate)
                if (sf.transliterated_from.empty()) {
                    sf.transliterated_from = PortingAnalyzer::extract_transliterated_from(p);
                }

                // Get file stats and aggregate
                FileStats stats = PortingAnalyzer::analyze_file(p);
                sf.line_count += stats.line_count;
                sf.code_lines += stats.code_lines;
                sf.is_stub = sf.is_stub || stats.is_stub; // If any part is NOT a stub, maybe it's not a stub? 
                                                         // Actually, usually if BOTH are stubs it's a stub.
                                                         // Let's say if ALL are stubs, it's a stub.
                
                sf.todos.insert(sf.todos.end(), stats.todos.begin(), stats.todos.end());

                // Run lint checks
                auto lints = PortingAnalyzer::lint_file(p);
                sf.lint_errors.insert(sf.lint_errors.end(), lints.begin(), lints.end());
            }
            
            // Refine stub detection: if we have significant code lines across both, it's not a stub
            if (sf.code_lines > 50) sf.is_stub = false;
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

        // Documentation statistics
        int source_doc_lines = 0;
        int target_doc_lines = 0;
        int source_doc_comments = 0;
        int target_doc_comments = 0;
        float doc_similarity = 0.0f;     // Cosine similarity of doc word frequencies

        // Compute doc gap ratio: 0 = no gap, 1 = completely missing
        float doc_gap_ratio() const {
            if (source_doc_lines == 0) return 0.0f;  // Source has no docs, no gap
            if (target_doc_lines == 0) return 1.0f;  // Target missing all docs
            float ratio = 1.0f - (static_cast<float>(target_doc_lines) / source_doc_lines);
            return std::max(0.0f, ratio);  // Clamp to 0 if target has more
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
                         tgt_file.transliterated_from.ends_with("/" + src_file.stem + ".rs")) {
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
                const auto& src_file = source.files.at(m.source_path);
                const auto& tgt_file = target.files.at(m.target_path);

                auto src_tree = parser.parse_file(src_file.paths, string_to_language(source.language));
                auto tgt_tree = parser.parse_file(tgt_file.paths, string_to_language(target.language));

                // Normalize ASTs: Flatten namespaces/packages to reduce structural noise
                if (src_tree) src_tree->flatten_node_type(82);
                if (tgt_tree) tgt_tree->flatten_node_type(82);

                m.similarity = ASTSimilarity::combined_similarity(
                    src_tree.get(), tgt_tree.get());

                // Extract documentation statistics
                auto src_docs = parser.extract_comments_from_file(src_file.paths, string_to_language(source.language));
                auto tgt_docs = parser.extract_comments_from_file(tgt_file.paths, string_to_language(target.language));

                m.source_doc_lines = src_docs.total_doc_lines;
                m.target_doc_lines = tgt_docs.total_doc_lines;
                m.source_doc_comments = src_docs.doc_comment_count;
                m.target_doc_comments = tgt_docs.doc_comment_count;
                m.doc_similarity = src_docs.doc_cosine_similarity(tgt_docs);
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
