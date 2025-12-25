#include "ast_parser.hpp"
#include "similarity.hpp"
#include "tree_lstm.hpp"
#include "codebase.hpp"
#include <iostream>
#include <filesystem>
#include <iomanip>

using namespace ast_distance;

void print_usage(const char* program) {
    std::cerr << "AST Distance - Cross-language AST comparison and dependency analysis\n\n";
    std::cerr << "Usage:\n";
    std::cerr << "  " << program << " <rust_file> <kotlin_file>\n";
    std::cerr << "      Compare AST similarity between two files\n\n";
    std::cerr << "  " << program << " --compare-functions <rust_file> <kotlin_file>\n";
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

        } else if (mode == "--compare-functions" && argc >= 4) {
            ASTParser parser;
            std::string rust_file = argv[2];
            std::string kotlin_file = argv[3];

            std::cout << "Extracting functions from " << rust_file << "...\n";
            std::ifstream rs_stream(rust_file);
            std::stringstream rs_buffer;
            rs_buffer << rs_stream.rdbuf();
            auto rust_funcs = parser.extract_functions(rs_buffer.str(), Language::RUST);

            std::cout << "Found " << rust_funcs.size() << " Rust functions\n";
            for (const auto& [name, tree] : rust_funcs) {
                std::cout << "  - " << name << " (" << tree->size() << " nodes)\n";
            }

            std::cout << "\nExtracting functions from " << kotlin_file << "...\n";
            std::ifstream kt_stream(kotlin_file);
            std::stringstream kt_buffer;
            kt_buffer << kt_stream.rdbuf();
            auto kotlin_funcs = parser.extract_functions(kt_buffer.str(), Language::KOTLIN);

            std::cout << "Found " << kotlin_funcs.size() << " Kotlin functions\n";
            for (const auto& [name, tree] : kotlin_funcs) {
                std::cout << "  - " << name << " (" << tree->size() << " nodes)\n";
            }

            // Compare functions with similar names
            std::cout << "\n=== Function Similarity Matrix ===\n\n";
            std::cout << std::setw(20) << "";
            for (const auto& [kt_name, _] : kotlin_funcs) {
                std::cout << std::setw(12) << kt_name.substr(0, 10);
            }
            std::cout << "\n";

            for (const auto& [rs_name, rs_tree] : rust_funcs) {
                std::cout << std::setw(20) << rs_name.substr(0, 18);
                for (const auto& [kt_name, kt_tree] : kotlin_funcs) {
                    float sim = ASTSimilarity::combined_similarity(
                        rs_tree.get(), kt_tree.get());
                    std::cout << std::setw(12) << std::fixed << std::setprecision(3) << sim;
                }
                std::cout << "\n";
            }

        } else if (mode[0] != '-') {
            // Default: compare two files
            ASTParser parser;
            std::string rust_file = argv[1];
            std::string kotlin_file = argv[2];

            std::cout << "Parsing Rust file: " << rust_file << "\n";
            TreePtr rust_tree = parser.parse_file(rust_file, Language::RUST);

            std::cout << "Parsing Kotlin file: " << kotlin_file << "\n";
            TreePtr kotlin_tree = parser.parse_file(kotlin_file, Language::KOTLIN);

            std::cout << "\n";
            auto report = ASTSimilarity::compare(rust_tree.get(), kotlin_tree.get());
            report.print();

            std::cout << "\n=== Rust AST Histogram ===\n";
            print_histogram(report.hist1);

            std::cout << "\n=== Kotlin AST Histogram ===\n";
            print_histogram(report.hist2);

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
