#include "ast_parser.hpp"
#include "similarity.hpp"
#include "tree_lstm.hpp"
#include "codebase.hpp"
#include <iostream>
#include <filesystem>
#include <iomanip>

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
    std::cerr << "AST Distance - Cross-language AST comparison and dependency analysis\n\n";
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
    std::cerr << "      Full analysis: AST similarity + dependencies + rankings\n\n";
    std::cerr << "  " << program << " --missing <src_dir> <src_lang> <tgt_dir> <tgt_lang>\n";
    std::cerr << "      Show files missing from target, ranked by importance\n\n";
    std::cerr << "  Languages: rust, kotlin, cpp\n\n";
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
    target.print_summary();

    // Compare
    std::cout << "\nComparing codebases...\n";
    CodebaseComparator comp(source, target);
    comp.find_matches();

    std::cout << "Computing AST similarities...\n";
    comp.compute_similarities();

    comp.print_report();

    // Porting recommendations
    std::cout << "\n=== Porting Recommendations ===\n\n";

    auto ranked = comp.ranked_for_porting();
    int incomplete = 0;
    for (const auto& m : ranked) {
        if (m.similarity < 0.6) incomplete++;
    }

    std::cout << "Incomplete ports (similarity < 60%): " << incomplete << "\n";
    std::cout << "Missing files: " << comp.unmatched_source.size() << "\n\n";

    if (incomplete > 0) {
        std::cout << "Top priority to complete:\n";
        int shown = 0;
        for (const auto& m : ranked) {
            if (m.similarity < 0.6 && shown++ < 10) {
                std::cout << "  " << std::setw(30) << std::left << m.source_qualified
                          << " sim=" << std::fixed << std::setprecision(2) << m.similarity
                          << " deps=" << m.source_dependents << "\n";
            }
        }
    }

    if (!comp.unmatched_source.empty()) {
        std::cout << "\nTop priority to create:\n";
        // Sort unmatched by dependents
        std::vector<const SourceFile*> missing;
        for (const auto& path : comp.unmatched_source) {
            missing.push_back(&source.files.at(path));
        }
        std::sort(missing.begin(), missing.end(),
            [](const SourceFile* a, const SourceFile* b) {
                return a->dependent_count > b->dependent_count;
            });

        int shown = 0;
        for (const auto* sf : missing) {
            if (shown++ < 10) {
                std::cout << "  " << std::setw(30) << std::left << sf->qualified_name
                          << " deps=" << sf->dependent_count << "\n";
            }
        }
    }
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

int main(int argc, char* argv[]) {
    if (argc < 3) {
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
