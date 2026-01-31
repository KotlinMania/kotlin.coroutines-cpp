#pragma once

#include <string>
#include <optional>
#include <regex>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

/**
 * Port-lint support for tracking Rust -> Kotlin provenance.
 * 
 * Supports these comment annotations:
 * 
 * PROVENANCE:
 *   // port-lint: source core/src/codex.rs
 *   
 * SUPPRESSION:
 *   // port-lint: ignore-duplicate
 *   // port-lint: ignore
 * 
 * Based on tools/port_linter.py
 */

namespace port_lint {

/**
 * Extract port-lint source annotation from a Kotlin file.
 * 
 * Searches the first 50 lines for:
 *   // port-lint: source <path>
 * 
 * Returns the Rust source path if found (e.g., "core/src/codex.rs")
 */
inline std::optional<std::string> extract_source_annotation(const fs::path& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return std::nullopt;
    }
    
    // Regex: // port-lint: source <path>
    std::regex pattern(R"(//\s*port-lint:\s*source\s+(.+))", std::regex::icase);
    
    std::string line;
    int line_count = 0;
    while (std::getline(file, line) && line_count++ < 50) {
        std::smatch match;
        if (std::regex_search(line, match, pattern)) {
            std::string path_str = match[1].str();
            // Trim whitespace
            path_str.erase(0, path_str.find_first_not_of(" \t\r\n"));
            path_str.erase(path_str.find_last_not_of(" \t\r\n") + 1);
            return path_str;
        }
    }
    
    return std::nullopt;
}

/**
 * Check if a type/function definition has a port-lint suppression comment.
 * 
 * Checks for:
 *   // port-lint: ignore-duplicate
 *   // port-lint: ignore
 * 
 * Scans:
 * 1. The line itself (inline comment)
 * 2. Lines above (for comments before annotations like @Serializable)
 */
inline bool has_suppression(const std::vector<std::string>& lines, int line_num) {
    if (line_num <= 0 || line_num > static_cast<int>(lines.size())) {
        return false;
    }
    
    // Regex: // port-lint: ignore or ignore-duplicate
    std::regex pattern(R"(//\s*port-lint:\s*ignore(?:-duplicate)?)", std::regex::icase);
    
    // Check current line (line_num is 1-indexed, vector is 0-indexed)
    int idx = line_num - 1;
    if (std::regex_search(lines[idx], pattern)) {
        return true;
    }
    
    // Scan backwards through annotation lines
    // In Kotlin:
    //   // port-lint: ignore-duplicate
    //   @Serializable
    //   @SerialName("foo")
    //   data class Foo(...)
    int scan_idx = idx - 1;
    while (scan_idx >= 0) {
        std::string prev_line = lines[scan_idx];
        // Trim
        size_t start = prev_line.find_first_not_of(" \t\r\n");
        if (start != std::string::npos) {
            prev_line = prev_line.substr(start);
        }
        
        // Found suppression comment
        if (std::regex_search(prev_line, pattern)) {
            return true;
        }
        
        // Annotation line (@...) - continue scanning
        if (!prev_line.empty() && prev_line[0] == '@') {
            scan_idx--;
            continue;
        }
        
        // Empty line or comment - continue scanning
        if (prev_line.empty() || prev_line.substr(0, 2) == "//") {
            scan_idx--;
            continue;
        }
        
        // Hit other code - stop scanning
        break;
    }
    
    return false;
}

/**
 * Read file lines into vector for suppression checking.
 */
inline std::vector<std::string> read_file_lines(const fs::path& file_path) {
    std::vector<std::string> lines;
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return lines;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    
    return lines;
}

} // namespace port_lint
