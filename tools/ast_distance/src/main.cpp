#include "ast_parser.hpp"
#include "similarity.hpp"
#include "tree_lstm.hpp"
#include "codebase.hpp"
#include "porting_utils.hpp"
#include "task_manager.hpp"
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <ctime>

using namespace ast_distance;

Language parse_language(const std::string& lang_str) {
    if (lang_str == "rust") return Language::RUST;
    if (lang_str == "kotlin") return Language::KOTLIN;
    if (lang_str == "cpp") return Language::CPP;
    throw std::runtime_error("Unknown language: " + lang_str + " (use rust, kotlin, or cpp)");
}

const char* language_name(Language lang) {
    switch (lang) {
        case Language::RUST: return "Rust";
        case Language::KOTLIN: return "Kotlin";
        case Language::CPP: return "C++";
    }
    return "Unknown";
}

void print_usage(const char* program) {
    std::cerr << "AST Distance - Cross-language AST comparison and porting analysis\n\n";
    std::cerr << "Usage:\n";
    std::cerr << "  " << program << " <file1> <lang1> <file2> <lang2>\n";
    std::cerr << "      Compare AST similarity between two files\n\n";
    std::cerr << "  " << program << " --compare-functions <file1> <lang1> <file2> <lang2>\n";
    std::cerr << "      Compare functions between files with similarity matrix\n\n";
    std::cerr << "  " << program << " --dump <file> <rust|kotlin|cpp>\n";
    std::cerr << "      Dump AST structure of a file\n\n";
    std::cerr << "  " << program << " --scan <directory> <rust|kotlin|cpp>\n";
    std::cerr << "      Scan directory and show file list with import counts\n\n";
    std::cerr << "  " << program << " --deps <directory> <rust|kotlin|cpp>\n";
    std::cerr << "      Build and show dependency graph\n\n";
    std::cerr << "  " << program << " --rank <src_dir> <src_lang> <tgt_dir> <tgt_lang>\n";
    std::cerr << "      Rank files by porting priority (dependents + similarity)\n\n";
    std::cerr << "  " << program << " --deep <src_dir> <src_lang> <tgt_dir> <tgt_lang>\n";
    std::cerr << "      Full analysis: AST + deps + TODOs + lint + line ratios\n\n";
    std::cerr << "  " << program << " --missing <src_dir> <src_lang> <tgt_dir> <tgt_lang>\n";
    std::cerr << "      Show files missing from target, ranked by importance\n\n";
    std::cerr << "  " << program << " --todos <directory>\n";
    std::cerr << "      Scan for TODO comments with tags and context\n\n";
    std::cerr << "  " << program << " --lint <directory>\n";
    std::cerr << "      Run lint checks (unused params, missing guards)\n\n";
    std::cerr << "  " << program << " --stats <directory>\n";
    std::cerr << "      Show file statistics (line counts, stubs, TODOs)\n\n";
    std::cerr << "Swarm Task Management:\n";
    std::cerr << "  " << program << " --init-tasks <src_dir> <src_lang> <tgt_dir> <tgt_lang> <task_file>\n";
    std::cerr << "      Generate task file from missing/incomplete ports\n\n";
    std::cerr << "  " << program << " --tasks <task_file>\n";
    std::cerr << "      Show task status summary\n\n";
    std::cerr << "  " << program << " --assign <task_file> <agent_id>\n";
    std::cerr << "      Assign highest-priority pending task to an agent\n";
    std::cerr << "      Outputs complete porting instructions and AGENTS.md guidelines\n\n";
    std::cerr << "  " << program << " --complete <task_file> <source_qualified>\n";
    std::cerr << "      Mark a task as completed\n\n";
    std::cerr << "  " << program << " --release <task_file> <source_qualified>\n";
    std::cerr << "      Release an assigned task back to pending\n\n";
    std::cerr << "  Languages: rust, kotlin, cpp\n\n";
    std::cerr << "Port-Lint Headers:\n";
    std::cerr << "  Add a header comment to each ported file to enable accurate source tracking.\n";
    std::cerr << "  This allows --deep analysis to match files by explicit declaration rather\n";
    std::cerr << "  than heuristic name matching, improving accuracy and enabling documentation\n";
    std::cerr << "  gap detection.\n\n";
    std::cerr << "  Format (Kotlin porting from Rust):\n";
    std::cerr << "    // port-lint: source <relative-path-to-rust-file>\n\n";
    std::cerr << "  Example:\n";
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

void generate_reports(const Codebase& source, const Codebase& target,
                      const CodebaseComparator& comp,
                      const std::vector<CodebaseComparator::Match>& ranked,
                      const std::vector<const SourceFile*>& missing,
                      const std::vector<std::pair<float, const CodebaseComparator::Match*>>& doc_gaps,
                      int incomplete_count,
                      int total_src_doc_lines,
                      int total_tgt_doc_lines) {
    
    std::cout << "\n=== Generating Reports ===\n\n";
    
    // Calculate statistics
    int total_source = source.files.size();
    int total_target = target.files.size();
    int matched = comp.matches.size();
    float completion_pct = (static_cast<float>(total_target) / static_cast<float>(total_source)) * 100.0f;
    
    // Count quality distribution
    int excellent = 0, good = 0, critical = 0;
    float avg_similarity = 0.0f;
    for (const auto& m : comp.matches) {
        avg_similarity += m.similarity;
        if (m.similarity >= 0.85) excellent++;
        else if (m.similarity >= 0.60) good++;
        else critical++;
    }
    if (!comp.matches.empty()) {
        avg_similarity /= comp.matches.size();
    }
    
    // Get current date/time as string
    std::time_t now = std::time(nullptr);
    char date_buf[100];
    std::strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", std::localtime(&now));
    
    // 1. Generate port_status_report.md
    {
        std::ofstream report("port_status_report.md");
        report << "# Code Port - Progress Report\n\n";
        report << "**Generated:** " << date_buf << "\n";
        report << "**Source:** " << source.root_path << "\n";
        report << "**Target:** " << target.root_path << "\n\n";
        
        report << "## Executive Summary\n\n";
        report << "| Metric | Count | Percentage |\n";
        report << "|--------|-------|------------|\n";
        report << "| Total source files | " << total_source << " | 100% |\n";
        report << "| Ported to target | " << total_target << " | " 
               << std::fixed << std::setprecision(1) << completion_pct << "% |\n";
        report << "| Matched files | " << matched << " | "
               << std::fixed << std::setprecision(1) 
               << (static_cast<float>(matched) / total_source * 100.0f) << "% |\n";
        report << "| Missing files | " << comp.unmatched_source.size() << " | "
               << std::fixed << std::setprecision(1)
               << (static_cast<float>(comp.unmatched_source.size()) / total_source * 100.0f) << "% |\n\n";
        
        report << "## Port Quality Analysis\n\n";
        report << "**Average Similarity:** " << std::fixed << std::setprecision(2) << avg_similarity << "\n\n";
        report << "**Quality Distribution:**\n";
        report << "- Excellent (≥0.85): " << excellent << " files (" 
               << std::fixed << std::setprecision(1) << (static_cast<float>(excellent) / matched * 100.0f) << "% of matched)\n";
        report << "- Good (0.60-0.84): " << good << " files ("
               << std::fixed << std::setprecision(1) << (static_cast<float>(good) / matched * 100.0f) << "% of matched)\n";
        report << "- Critical (<0.60): " << critical << " files ("
               << std::fixed << std::setprecision(1) << (static_cast<float>(critical) / matched * 100.0f) << "% of matched)\n\n";
        
        report << "### Excellent Ports (Similarity ≥ 0.85)\n\n";
        report << "These files are well-ported and likely complete:\n\n";
        int shown = 0;
        for (const auto& m : ranked) {
            if (m.similarity >= 0.85 && shown++ < 15) {
                report << "- `" << m.target_qualified << "` (" << std::fixed << std::setprecision(2)
                       << m.similarity << ", " << m.source_dependents << " deps)\n";
            }
        }
        report << "\n";
        
        report << "### Critical Ports (Similarity < 0.60)\n\n";
        report << "These files need significant work:\n\n";
        for (const auto& m : ranked) {
            if (m.similarity < 0.60) {
                report << "- `" << m.source_qualified << "` → `" << m.target_qualified 
                       << "` (" << std::fixed << std::setprecision(2) << m.similarity;
                if (m.source_dependents > 0) report << ", " << m.source_dependents << " deps";
                report << ")\n";
            }
        }
        report << "\n";
        
        report << "## High Priority Missing Files\n\n";
        report << "Files with highest dependency counts:\n\n";
        int shown_missing = 0;
        for (const auto* sf : missing) {
            if (shown_missing++ < 20) {
                report << shown_missing << ". **" << sf->qualified_name << "** (" 
                       << sf->dependent_count << " deps)\n";
            }
        }
        report << "\n";
        
        report << "## Documentation Gaps\n\n";
        report << "**Documentation coverage:** " << total_tgt_doc_lines << " / " 
               << total_src_doc_lines << " lines (";
        if (total_src_doc_lines > 0) {
            report << std::fixed << std::setprecision(0)
                   << (100.0f * total_tgt_doc_lines / total_src_doc_lines) << "%)\n\n";
        } else {
            report << "N/A)\n\n";
        }
        
        report << "Files with significant documentation gaps (>80%):\n\n";
        int shown_docs = 0;
        for (const auto& [gap, m] : doc_gaps) {
            if (gap > 0.8f && shown_docs++ < 10) {
                report << "- `" << m->source_qualified << "` - " 
                       << std::fixed << std::setprecision(0) << (gap * 100) << "% gap ("
                       << m->source_doc_lines << " → " << m->target_doc_lines << " lines)\n";
            }
        }
        report << "\n";
        
        std::cout << "✅ Generated: port_status_report.md\n";
    }
    
    // 2. Generate high_priority_ports.md
    {
        std::ofstream report("high_priority_ports.md");
        report << "# High Priority Ports - Action Plan\n\n";
        
        report << "## Top 20 Files by Impact (Priority Score = Deps × (1 - Similarity))\n\n";
        report << "| Rank | Source | Target | Similarity | Deps | Priority |\n";
        report << "|------|--------|--------|------------|------|----------|\n";
        
        int rank = 1;
        for (const auto& m : ranked) {
            if (rank <= 20) {
                float priority = m.source_dependents * (1.0f - m.similarity);
                report << "| " << rank++ << " | `" << m.source_qualified << "` | `"
                       << m.target_qualified << "` | " << std::fixed << std::setprecision(2)
                       << m.similarity << " | " << m.source_dependents << " | "
                       << std::fixed << std::setprecision(1) << priority << " |\n";
            }
        }
        report << "\n";
        
        report << "## Critical Issues (Similarity < 0.60 with Dependencies)\n\n";
        bool has_critical = false;
        for (const auto& m : ranked) {
            if (m.similarity < 0.60 && m.source_dependents > 0) {
                if (!has_critical) {
                    report << "These files need immediate attention:\n\n";
                    has_critical = true;
                }
                report << "- **" << m.source_qualified << "** → `" << m.target_qualified << "`\n";
                report << "  - Similarity: " << std::fixed << std::setprecision(2) << m.similarity << "\n";
                report << "  - Dependencies: " << m.source_dependents << "\n";
                if (m.todo_count > 0) report << "  - TODOs: " << m.todo_count << "\n";
                if (m.lint_count > 0) report << "  - Lint issues: " << m.lint_count << "\n";
                report << "\n";
            }
        }
        if (!has_critical) {
            report << "No critical issues with dependencies.\n\n";
        }
        
        std::cout << "✅ Generated: high_priority_ports.md\n";
    }
    
    // 3. Generate NEXT_ACTIONS.md
    {
        std::ofstream report("NEXT_ACTIONS.md");
        report << "# Immediate Actions - High-Value Files\n\n";
        report << "Based on AST analysis, here are the concrete next steps.\n\n";
        
        report << "## Summary\n\n";
        report << "- **Current Progress:** " << std::fixed << std::setprecision(1) 
               << completion_pct << "% (" << total_target << "/" << total_source << " files)\n";
        report << "- **Matched Files:** " << matched << "\n";
        report << "- **Average Similarity:** " << std::fixed << std::setprecision(2) 
               << avg_similarity << "\n";
        report << "- **Critical Issues:** " << critical << " files with <0.60 similarity\n\n";
        
        report << "## Priority 1: Fix Incomplete High-Dependency Files\n\n";
        int p1_count = 0;
        for (const auto& m : ranked) {
            if (m.similarity < 0.85 && m.source_dependents >= 10 && p1_count++ < 10) {
                report << "### " << p1_count << ". " << m.source_qualified << "\n";
                report << "- **Similarity:** " << std::fixed << std::setprecision(2) 
                       << m.similarity << " (needs " << std::fixed << std::setprecision(0)
                       << ((0.85f - m.similarity) * 100.0f) << "% improvement)\n";
                report << "- **Dependencies:** " << m.source_dependents << "\n";
                report << "- **Priority Score:** " << std::fixed << std::setprecision(1)
                       << (m.source_dependents * (1.0f - m.similarity)) << "\n";
                if (m.todo_count > 0) report << "- **TODOs:** " << m.todo_count << "\n";
                report << "- **Action:** ";
                if (m.similarity < 0.60) report << "Deep review - likely missing major functionality\n";
                else if (m.similarity < 0.75) report << "Review and complete missing sections\n";
                else report << "Minor refinements needed\n";
                report << "\n";
            }
        }
        
        report << "## Priority 2: Port Missing High-Value Files\n\n";
        report << "Critical missing files (>10 dependencies):\n\n";
        int p2_count = 0;
        for (const auto* sf : missing) {
            if (sf->dependent_count >= 10 && p2_count++ < 10) {
                report << p2_count << ". **" << sf->qualified_name << "** (" 
                       << sf->dependent_count << " deps)\n";
                report << "   - Path: `" << sf->relative_path << "`\n";
                report << "   - Essential for " << sf->dependent_count << " other files\n\n";
            }
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
        report << "./ast_distance --init-tasks ../../" << source.root_path 
               << " " << source.language << " ../../" << target.root_path 
               << " " << target.language << " tasks.json ../../AGENTS.md\n\n";
        report << "# Get next high-priority task\n";
        report << "./ast_distance --assign tasks.json <agent-id>\n";
        report << "```\n";
        
        std::cout << "✅ Generated: NEXT_ACTIONS.md\n";
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

    comp.print_report();

    // Porting quality summary
    std::cout << "\n=== Porting Quality Summary ===\n\n";

    int total_todos = 0;
    int total_lint = 0;
    int stub_count = 0;
    int header_matched = 0;

    for (const auto& m : comp.matches) {
        total_todos += m.todo_count;
        total_lint += m.lint_count;
        if (m.is_stub) stub_count++;
        if (m.matched_by_header) header_matched++;
    }

    std::cout << "Matched by header:    " << header_matched << " / " << comp.matches.size() << "\n";
    std::cout << "Matched by name:      " << (comp.matches.size() - header_matched) << " / " << comp.matches.size() << "\n";
    std::cout << "Total TODOs in target: " << total_todos << "\n";
    std::cout << "Total lint errors:    " << total_lint << "\n";
    std::cout << "Stub files:           " << stub_count << "\n";

    // Show files with issues
    std::cout << "\n=== Files with Issues ===\n\n";
    std::cout << std::setw(30) << std::left << "File"
              << std::setw(8) << "Sim"
              << std::setw(8) << "Ratio"
              << std::setw(6) << "TODOs"
              << std::setw(6) << "Lint"
              << "Status\n";
    std::cout << std::string(70, '-') << "\n";

    auto ranked = comp.ranked_for_porting();
    int shown = 0;
    for (const auto& m : ranked) {
        if (m.todo_count == 0 && m.lint_count == 0 && !m.is_stub && m.similarity >= 0.6) {
            continue;  // Skip files without issues
        }
        if (shown++ >= 20) {
            std::cout << "... and " << (ranked.size() - 20) << " more files\n";
            break;
        }

        std::string status;
        if (m.is_stub) status = "STUB";
        else if (m.similarity < 0.4) status = "LOW_SIM";
        else if (m.lint_count > 0) status = "LINT";
        else if (m.todo_count > 0) status = "TODO";

        float ratio = 0.0f;
        if (m.source_lines > 0) {
            ratio = static_cast<float>(m.target_lines) / static_cast<float>(m.source_lines);
        }

        std::cout << std::setw(30) << std::left << m.target_qualified.substr(0, 28)
                  << std::setw(8) << std::fixed << std::setprecision(2) << m.similarity
                  << std::setw(8) << std::fixed << std::setprecision(2) << ratio
                  << std::setw(6) << m.todo_count
                  << std::setw(6) << m.lint_count
                  << status << "\n";
    }

    // Porting recommendations
    std::cout << "\n=== Porting Recommendations ===\n\n";

    int incomplete = 0;
    for (const auto& m : ranked) {
        if (m.similarity < 0.6) incomplete++;
    }

    std::cout << "Incomplete ports (similarity < 60%): " << incomplete << "\n";
    std::cout << "Missing files: " << comp.unmatched_source.size() << "\n\n";

    if (incomplete > 0) {
        std::cout << "Top priority to complete:\n";
        int shown_priority = 0;
        for (const auto& m : ranked) {
            if (m.similarity < 0.6 && shown_priority++ < 10) {
                std::cout << "  " << std::setw(30) << std::left << m.source_qualified
                          << " sim=" << std::fixed << std::setprecision(2) << m.similarity
                          << " deps=" << m.source_dependents;
                if (m.is_stub) std::cout << " [STUB]";
                if (m.todo_count > 0) std::cout << " [" << m.todo_count << " TODOs]";
                std::cout << "\n";
            }
        }
    }

    // Prepare missing files vector for report generation
    std::vector<const SourceFile*> missing;
    if (!comp.unmatched_source.empty()) {
        std::cout << "\nTop priority to create:\n";
        // Sort unmatched by dependents
        for (const auto& path : comp.unmatched_source) {
            missing.push_back(&source.files.at(path));
        }
        std::sort(missing.begin(), missing.end(),
            [](const SourceFile* a, const SourceFile* b) {
                return a->dependent_count > b->dependent_count;
            });

        int shown_missing = 0;
        for (const auto* sf : missing) {
            if (shown_missing++ < 10) {
                std::cout << "  " << std::setw(30) << std::left << sf->qualified_name
                          << " deps=" << sf->dependent_count << "\n";
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

    // Calculate total doc lines (moved out of block for report generation)
    int total_src_doc_lines = 0;
    int total_tgt_doc_lines = 0;
    for (const auto& m : comp.matches) {
        total_src_doc_lines += m.source_doc_lines;
        total_tgt_doc_lines += m.target_doc_lines;
    }

    if (doc_gaps.empty()) {
        std::cout << "No significant documentation gaps found.\n";
    } else {
        std::cout << std::setw(30) << std::left << "File"
                  << std::setw(12) << "Src Docs"
                  << std::setw(12) << "Tgt Docs"
                  << std::setw(10) << "Gap %"
                  << std::setw(10) << "DocSim"
                  << "\n";
        std::cout << std::string(74, '-') << "\n";

        int shown_docs = 0;
        for (const auto& [gap, m] : doc_gaps) {
            if (shown_docs++ >= 25) {
                std::cout << "... and " << (doc_gaps.size() - 25) << " more files with doc gaps\n";
                break;
            }

            std::string gap_str = std::to_string(static_cast<int>(gap * 100)) + "%";
            std::cout << std::setw(30) << std::left << m->source_qualified.substr(0, 28)
                      << std::setw(12) << m->source_doc_lines
                      << std::setw(12) << m->target_doc_lines
                      << std::setw(10) << gap_str
                      << std::setw(10) << std::fixed << std::setprecision(2) << m->doc_similarity
                      << "\n";
        }

        std::cout << "\nDocumentation coverage: " << total_tgt_doc_lines << " / " << total_src_doc_lines
                  << " lines (" << std::fixed << std::setprecision(0)
                  << (total_src_doc_lines > 0 ? (100.0f * total_tgt_doc_lines / total_src_doc_lines) : 0)
                  << "%)\n";
        std::cout << "Files with >20% doc gap: " << doc_gaps.size() << "\n";
    }

    // Generate markdown reports
    generate_reports(source, target, comp, ranked, missing, doc_gaps, 
                     incomplete, total_src_doc_lines, total_tgt_doc_lines);
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

    std::cout << "=== Missing from " << tgt_lang << " (ranked by dependents) ===\n\n";
    std::cout << std::setw(40) << std::left << "Source File"
              << std::setw(10) << "Deps"
              << "Path\n";
    std::cout << std::string(80, '-') << "\n";

    // Sort unmatched by dependents
    std::vector<const SourceFile*> missing;
    for (const auto& path : comp.unmatched_source) {
        missing.push_back(&source.files.at(path));
    }
    std::sort(missing.begin(), missing.end(),
        [](const SourceFile* a, const SourceFile* b) {
            return a->dependent_count > b->dependent_count;
        });

    for (const auto* sf : missing) {
        std::cout << std::setw(40) << std::left << sf->qualified_name.substr(0, 38)
                  << std::setw(10) << sf->dependent_count
                  << sf->relative_path << "\n";
    }

    std::cout << "\nTotal: " << missing.size() << " files missing\n";
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

    CodebaseComparator comp(source, target);
    comp.find_matches();

    // Build task list from missing files
    TaskManager tm(task_file);
    tm.source_root = src_dir;
    tm.target_root = tgt_dir;
    tm.source_lang = src_lang;
    tm.target_lang = tgt_lang;
    tm.agents_md_path = agents_md;

    // Add missing files as tasks (sorted by dependents)
    std::vector<const SourceFile*> missing;
    for (const auto& path : comp.unmatched_source) {
        missing.push_back(&source.files.at(path));
    }
    std::sort(missing.begin(), missing.end(),
        [](const SourceFile* a, const SourceFile* b) {
            return a->dependent_count > b->dependent_count;
        });

    for (const auto* sf : missing) {
        PortTask task;
        task.source_path = sf->relative_path;
        task.source_qualified = sf->qualified_name;

        // Generate expected Kotlin path
        std::string kt_path = sf->relative_path;
        // Convert .rs to .kt and adjust path
        if (kt_path.size() > 3 && kt_path.substr(kt_path.size() - 3) == ".rs") {
            kt_path = kt_path.substr(0, kt_path.size() - 3) + ".kt";
        }
        // Remove src/ prefix if present
        if (kt_path.rfind("src/", 0) == 0) {
            kt_path = kt_path.substr(4);
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

void cmd_tasks(const std::string& task_file) {
    TaskManager tm(task_file);
    if (!tm.load()) {
        std::cerr << "Error: Could not load task file: " << task_file << "\n";
        return;
    }

    int pending, assigned, completed, blocked;
    tm.get_stats(pending, assigned, completed, blocked);

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

void cmd_assign(const std::string& task_file, const std::string& agent_id) {
    TaskManager tm(task_file);
    if (!tm.load()) {
        std::cerr << "Error: Could not load task file: " << task_file << "\n";
        return;
    }

    // Check if agent already has an assigned task
    for (const auto& t : tm.tasks) {
        if (t.status == TaskStatus::ASSIGNED && t.assigned_to == agent_id) {
            std::cerr << "Agent " << agent_id << " already has an assigned task: "
                      << t.source_qualified << "\n";
            std::cerr << "Complete it with: ast_distance --complete " << task_file
                      << " " << t.source_qualified << "\n";
            std::cerr << "Or release it with: ast_distance --release " << task_file
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

    // Print full assignment details
    tm.print_assignment(*task);
}

void cmd_complete(const std::string& task_file, const std::string& source_qualified) {
    TaskManager tm(task_file);
    if (!tm.load()) {
        std::cerr << "Error: Could not load task file: " << task_file << "\n";
        return;
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

    CodebaseComparator comp(source, target);
    comp.find_matches();

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

    std::vector<const SourceFile*> missing;
    for (const auto& path : comp.unmatched_source) {
        missing.push_back(&source.files.at(path));
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

        // Generate expected Kotlin path
        std::string kt_path = sf->relative_path;
        if (kt_path.size() > 3 && kt_path.substr(kt_path.size() - 3) == ".rs") {
            kt_path = kt_path.substr(0, kt_path.size() - 3) + ".kt";
        }
        if (kt_path.rfind("src/", 0) == 0) {
            kt_path = kt_path.substr(4);
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

void cmd_release(const std::string& task_file, const std::string& source_qualified) {
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
    
    // Check if target file exists - if so, require completion or deletion
    std::filesystem::path target_path = std::filesystem::path(tm.target_root) / it->target_path;
    if (std::filesystem::exists(target_path)) {
        std::filesystem::path source_path = std::filesystem::path(tm.source_root) / it->source_path;
        
        // Try to compute similarity - if we can't even parse, that's a HARD FAIL
        std::cerr << "Error: Cannot release task - target file already exists: " << target_path.string() << "\n";
        std::cerr << "Checking similarity...\n";
        
        // Parse and compare
        Language src_lang = Language::RUST;  // default
        if (tm.source_lang == "rust") src_lang = Language::RUST;
        else if (tm.source_lang == "kotlin") src_lang = Language::KOTLIN;
        else if (tm.source_lang == "cpp") src_lang = Language::CPP;
        
        Language tgt_lang = Language::KOTLIN;  // default
        if (tm.target_lang == "rust") tgt_lang = Language::RUST;
        else if (tm.target_lang == "kotlin") tgt_lang = Language::KOTLIN;
        else if (tm.target_lang == "cpp") tgt_lang = Language::CPP;
        
        ASTParser parser;
        auto src_tree = parser.parse_file(source_path.string(), src_lang);
        auto tgt_tree = parser.parse_file(target_path.string(), tgt_lang);
        
        if (!src_tree || !tgt_tree) {
            std::cerr << "Error: Cannot parse files for comparison\n";
            std::cerr << "This usually means the target file has syntax errors.\n";
            std::cerr << "Fix the errors or delete the file to release.\n";
            return;
        }
        
        float similarity = ASTSimilarity::combined_similarity(src_tree.get(), tgt_tree.get());
        
        // Require >= 0.70 similarity to release
        if (similarity < 0.70f) {
            std::cerr << "Error: Cannot release task with low similarity: " << similarity << "\n";
            std::cerr << "Target file exists but is incomplete (< 0.70 similarity required)\n";
            std::cerr << "Either complete the port or delete the target file to release.\n";
            return;
        }
        
        std::cerr << "Warning: Releasing with partial port (similarity " << similarity << ")\n";
        std::cerr << "Consider completing it instead (use --complete).\n";
    }

    if (tm.release_task(source_qualified)) {
        tm.save();
        std::cout << "Released task: " << source_qualified << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string mode = argv[1];

    try {
        if (mode == "--scan" && argc >= 4) {
            cmd_scan(argv[2], argv[3]);

        } else if (mode == "--deps" && argc >= 4) {
            cmd_deps(argv[2], argv[3]);

        } else if (mode == "--rank" && argc >= 6) {
            cmd_rank(argv[2], argv[3], argv[4], argv[5]);

        } else if (mode == "--deep" && argc >= 6) {
            cmd_deep(argv[2], argv[3], argv[4], argv[5]);

        } else if (mode == "--missing" && argc >= 6) {
            cmd_missing(argv[2], argv[3], argv[4], argv[5]);

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

        // Swarm task management commands
        } else if (mode == "--init-tasks" && argc >= 7) {
            std::string agents_md = (argc >= 8) ? argv[7] : "";
            cmd_init_tasks(argv[2], argv[3], argv[4], argv[5], argv[6], agents_md);

        } else if (mode == "--tasks" && argc >= 3) {
            cmd_tasks(argv[2]);

        } else if (mode == "--assign" && argc >= 4) {
            cmd_assign(argv[2], argv[3]);

        } else if (mode == "--complete" && argc >= 4) {
            cmd_complete(argv[2], argv[3]);

        } else if (mode == "--release" && argc >= 4) {
            cmd_release(argv[2], argv[3]);

        } else if (mode == "--dump" && argc >= 4) {
            ASTParser parser;
            std::string filepath = argv[2];
            std::string lang_str = argv[3];
            Language lang;
            if (lang_str == "rust") lang = Language::RUST;
            else if (lang_str == "cpp") lang = Language::CPP;
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
            auto funcs1 = parser.extract_functions(buffer1.str(), lang1);

            std::cout << "Found " << funcs1.size() << " " << language_name(lang1) << " functions\n";
            for (const auto& [name, tree] : funcs1) {
                std::cout << "  - " << name << " (" << tree->size() << " nodes)\n";
            }

            std::cout << "\nExtracting functions from " << file2 << " (" << language_name(lang2) << ")...\n";
            std::ifstream stream2(file2);
            std::stringstream buffer2;
            buffer2 << stream2.rdbuf();
            auto funcs2 = parser.extract_functions(buffer2.str(), lang2);

            std::cout << "Found " << funcs2.size() << " " << language_name(lang2) << " functions\n";
            for (const auto& [name, tree] : funcs2) {
                std::cout << "  - " << name << " (" << tree->size() << " nodes)\n";
            }

            // Compare functions with similar names
            std::cout << "\n=== Function Similarity Matrix ===\n\n";
            std::cout << std::setw(20) << "";
            for (const auto& [name2, _] : funcs2) {
                std::cout << std::setw(12) << name2.substr(0, 10);
            }
            std::cout << "\n";

            for (const auto& [name1, tree1] : funcs1) {
                std::cout << std::setw(20) << name1.substr(0, 18);
                for (const auto& [name2, tree2] : funcs2) {
                    float sim = ASTSimilarity::combined_similarity(
                        tree1.get(), tree2.get());
                    std::cout << std::setw(12) << std::fixed << std::setprecision(3) << sim;
                }
                std::cout << "\n";
            }

        } else if (mode[0] != '-' && argc >= 5) {
            // Default: compare two files with explicit languages
            ASTParser parser;
            std::string file1 = argv[1];
            Language lang1 = parse_language(argv[2]);
            std::string file2 = argv[3];
            Language lang2 = parse_language(argv[4]);

            std::cout << "Parsing " << language_name(lang1) << " file: " << file1 << "\n";
            TreePtr tree1 = parser.parse_file(file1, lang1);

            std::cout << "Parsing " << language_name(lang2) << " file: " << file2 << "\n";
            TreePtr tree2 = parser.parse_file(file2, lang2);

            std::cout << "\n";
            auto report = ASTSimilarity::compare(tree1.get(), tree2.get());
            report.print();

            std::cout << "\n=== " << language_name(lang1) << " AST Histogram ===\n";
            print_histogram(report.hist1);

            std::cout << "\n=== " << language_name(lang2) << " AST Histogram ===\n";
            print_histogram(report.hist2);

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
            std::cout << "Doc text cosine:      " << std::fixed << std::setprecision(2)
                      << (doc_cosine * 100.0f) << "%\n";
            std::cout << "Doc text jaccard:     " << std::fixed << std::setprecision(2)
                      << (doc_jaccard * 100.0f) << "%\n";
            std::cout << "Unique doc words:     " << comments1.word_freq.size()
                      << " vs " << comments2.word_freq.size() << "\n";

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
