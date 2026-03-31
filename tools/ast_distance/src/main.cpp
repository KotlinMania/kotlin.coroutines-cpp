#include "ast_parser.hpp"
#include "similarity.hpp"
#include "tree_lstm.hpp"
#include "codebase.hpp"
#include "porting_utils.hpp"
#include "task_manager.hpp"
#include "symbol_analysis.hpp"
#include "symbol_extraction.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <cstdio>
#include <memory>
#include <regex>
#include <set>
#include <sstream>
#include <ctime>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <cstring>
#include <fcntl.h>
#include <tree_sitter/api.h>

using namespace ast_distance;

struct GuardrailsContext {
    bool active = false;
    std::string task_file;
    int agent = 0;
    bool override_mode = false;
};

static GuardrailsContext g_guardrails;

// Forward declarations for guardrails helpers defined later in this file.
static void print_agent_activity_section(const TaskManager& tm, const std::string& task_file, int current_agent);

Language parse_language(const std::string& lang_str) {
    if (lang_str == "rust") return Language::RUST;
    if (lang_str == "kotlin") return Language::KOTLIN;
    if (lang_str == "cpp") return Language::CPP;
    if (lang_str == "python") return Language::PYTHON;
    throw std::runtime_error("Unknown language: " + lang_str + " (use rust, kotlin, cpp, or python)");
}

const char* language_name(Language lang) {
    switch (lang) {
        case Language::RUST: return "Rust";
        case Language::KOTLIN: return "Kotlin";
        case Language::CPP: return "C++";
        case Language::PYTHON: return "Python";
    }
    return "Unknown";
}

void print_usage(const char* program) {
    std::cerr << "AST Distance - Cross-language AST comparison and porting analysis\n\n";
    std::cerr << "Usage:\n";
    std::cerr << "  " << program << " [--agent <number>] [--task-file <tasks.json>] [--override] <command>\n";
    std::cerr << "      Guardrails: when a task system is initialized, commands require --agent.\n\n";
    std::cerr << "  " << program << " <file1> <lang1> <file2> <lang2>\n";
    std::cerr << "      Compare AST similarity between two files\n\n";
    std::cerr << "  " << program << " --compare-functions <file1> <lang1> <file2> <lang2>\n";
    std::cerr << "      Compare functions between files with similarity matrix\n\n";
    std::cerr << "  " << program << " --dump <file> <rust|kotlin|cpp|python>\n";
    std::cerr << "      Dump AST structure of a file\n\n";
    std::cerr << "  " << program << " --scan <directory> <rust|kotlin|cpp|python>\n";
    std::cerr << "      Scan directory and show file list with import counts\n\n";
    std::cerr << "  " << program << " --deps <directory> <rust|kotlin|cpp|python>\n";
    std::cerr << "      Build and show dependency graph\n\n";
    std::cerr << "  " << program << " --rank <src_dir> <src_lang> <tgt_dir> <tgt_lang>\n";
    std::cerr << "      Rank files by porting priority (dependents + similarity)\n\n";
    std::cerr << "  " << program << " --deep <src_dir> <src_lang> <tgt_dir> <tgt_lang>\n";
    std::cerr << "      Full analysis: AST + deps + TODOs + lint + line ratios\n\n";
    std::cerr << "  " << program << " --numpy-mlx <numpy_dir> <mlx_dir>\n";
    std::cerr << "      Python-focused report: compare two Python codebases and audit residual NumPy usage\n\n";
    std::cerr << "  " << program << " --emberlint <path>\n";
    std::cerr << "      Fast multi-language lint: NumPy usage + common MLX graph breakers (casts, conversions, operators)\n\n";
    std::cerr << "  " << program << " --missing <src_dir> <src_lang> <tgt_dir> <tgt_lang>\n";
    std::cerr << "      Show files missing from target, ranked by importance\n\n";
    std::cerr << "  " << program << " --todos <directory>\n";
    std::cerr << "      Scan for TODO comments with tags and context\n\n";
    std::cerr << "  " << program << " --lint <directory>\n";
    std::cerr << "      Run lint checks (unused params, missing guards)\n\n";
    std::cerr << "  " << program << " --stats <directory>\n";
    std::cerr << "      Show file statistics (line counts, stubs, TODOs)\n\n";
    std::cerr << "Symbol Analysis:\n";
    std::cerr << "  " << program << " --symbols <kotlin_root> <cpp_root>\n";
    std::cerr << "      Run symbol analysis (duplicates + stubs)\n\n";
    std::cerr << "  " << program << " --symbols-duplicates <kotlin_root> <cpp_root>\n";
    std::cerr << "      Show duplicate class/struct definitions\n\n";
    std::cerr << "  " << program << " --symbols-stubs <kotlin_root> <cpp_root>\n";
    std::cerr << "      Show stub files/classes\n\n";
    std::cerr << "  " << program << " --symbols-symbol <kotlin_root> <cpp_root> <symbol> [--json]\n";
    std::cerr << "      Analyze a specific symbol (optionally output JSON)\n\n";
    std::cerr << "  " << program << " --symbol-parity <rust_root> <kotlin_root>\n";
    std::cerr << "      Rust->Kotlin symbol parity analysis\n\n";
    std::cerr << "  " << program << " --symbol-parity <rust_root> <kotlin_root> --json\n";
    std::cerr << "      Output symbol parity as JSON\n\n";
    std::cerr << "  " << program << " --symbol-parity <rust_root> <kotlin_root> --kind <function|struct|enum|trait>\n";
    std::cerr << "      Filter by symbol kind\n\n";
    std::cerr << "  " << program << " --symbol-parity <rust_root> <kotlin_root> --file <path>\n";
    std::cerr << "      Filter to specific source file\n\n";
    std::cerr << "  " << program << " --symbol-parity <rust_root> <kotlin_root> --missing-only\n";
    std::cerr << "      Show only missing symbols (concise mode)\n\n";
    std::cerr << "  " << program << " --import-map <kotlin_root>\n";
    std::cerr << "      Build type registry and show missing imports per file\n\n";
    std::cerr << "  " << program << " --import-map <kotlin_root> --summary\n";
    std::cerr << "      Show only per-file unresolved counts\n\n";
    std::cerr << "  " << program << " --import-map <kotlin_root> --file <path>\n";
    std::cerr << "      Show imports for a specific file\n\n";
    std::cerr << "  " << program << " --import-map <kotlin_root> --json\n";
    std::cerr << "      Output as JSON\n\n";
    std::cerr << "Compiler Error Analysis:\n";
    std::cerr << "  " << program << " --compiler-fixup <kotlin_root> <error_file>\n";
    std::cerr << "      Parse compiler errors and suggest import fixes\n\n";
    std::cerr << "  " << program << " --compiler-fixup <kotlin_root> <error_file> --json\n";
    std::cerr << "      Output as JSON\n\n";
    std::cerr << "  " << program << " --compiler-fixup <kotlin_root> <error_file> --verbose\n";
    std::cerr << "      Show alternative imports for ambiguous references\n\n";
    std::cerr << "Swarm Task Management:\n";
    std::cerr << "  " << program << " --init-tasks <src_dir> <src_lang> <tgt_dir> <tgt_lang> <task_file>\n";
    std::cerr << "      Generate task file from missing/incomplete ports\n\n";
    std::cerr << "  " << program << " --tasks <task_file>\n";
    std::cerr << "      Show task status summary\n\n";
    std::cerr << "  " << program << " --assign <task_file> [agent_number]\n";
    std::cerr << "      Assign highest-priority pending task to an agent session\n";
    std::cerr << "      If agent_number is omitted, a new one is allocated and printed\n";
    std::cerr << "      Outputs complete porting instructions and AGENTS.md guidelines\n\n";
    std::cerr << "  " << program << " --complete <task_file> <source_qualified>\n";
    std::cerr << "      Mark a task as completed\n\n";
    std::cerr << "  " << program << " --release <task_file> <source_qualified>\n";
    std::cerr << "      Release an assigned task back to pending\n\n";
    std::cerr << "  Languages: rust, kotlin, cpp, python\n\n";
    std::cerr << "Port-Lint Headers:\n";
    std::cerr << "  Add a header comment to each ported file to enable accurate source tracking.\n";
    std::cerr << "  This allows --deep analysis to match files by explicit declaration rather\n";
    std::cerr << "  than heuristic name matching, improving accuracy and enabling documentation\n";
    std::cerr << "  gap detection.\n\n";
    std::cerr << "  Format:\n";
    std::cerr << "    // port-lint: source <relative-path-to-source-file>\n";
    std::cerr << "    ## port-lint: source <relative-path-to-source-file>   (Python)\n\n";
    std::cerr << "  Example (Kotlin):\n";
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

struct NumpyMlxAudit {
    int numpy_imports = 0;
    int numpy_refs = 0;
    int mlx_imports = 0;
    int mlx_refs = 0;
};

static int count_occurrences(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return 0;
    int count = 0;
    size_t pos = 0;
    while (true) {
        pos = haystack.find(needle, pos);
        if (pos == std::string::npos) break;
        count++;
        pos += needle.size();
    }
    return count;
}

static NumpyMlxAudit audit_numpy_mlx_python_file(const std::string& path) {
    NumpyMlxAudit a;

    std::ifstream file(path);
    if (!file.is_open()) return a;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string s = buffer.str();

    // Imports
    a.numpy_imports += count_occurrences(s, "import numpy");
    a.numpy_imports += count_occurrences(s, "from numpy import");

    a.mlx_imports += count_occurrences(s, "import mlx");
    a.mlx_imports += count_occurrences(s, "from mlx");
    a.mlx_imports += count_occurrences(s, "_mlx_numpy");

    // References (heuristic)
    a.numpy_refs += count_occurrences(s, "numpy.");
    a.numpy_refs += count_occurrences(s, "np.");

    a.mlx_refs += count_occurrences(s, "mlx.");
    a.mlx_refs += count_occurrences(s, "mx.");
    a.mlx_refs += count_occurrences(s, "_mlx_numpy.");

    return a;
}

void cmd_numpy_mlx(const std::string& numpy_dir, const std::string& mlx_dir) {
    std::cout << "=== NumPy -> MLX (Python) Deep Report ===\n\n";
    std::cout << "NumPy source: " << numpy_dir << "\n";
    std::cout << "MLX target:   " << mlx_dir << "\n\n";

    Codebase source(numpy_dir, "python");
    source.scan();
    source.extract_imports();
    source.build_dependency_graph();

    Codebase target(mlx_dir, "python");
    target.scan();
    target.extract_imports();
    target.build_dependency_graph();

    CodebaseComparator comp(source, target);
    comp.find_matches();
    comp.compute_similarities();

    int files_with_numpy = 0;
    int total_numpy_imports = 0;
    int total_numpy_refs = 0;
    int total_mlx_imports = 0;
    int total_mlx_refs = 0;

    struct Row {
        std::string qualified;
        std::string rel_path;
        NumpyMlxAudit audit;
        float similarity = -1.0f;
    };
    std::vector<Row> rows;
    rows.reserve(comp.matches.size());

    for (const auto& m : comp.matches) {
        auto a = NumpyMlxAudit{};
        const auto& tgt_file = target.files.at(m.target_path);
        for (const auto& tgt_path : tgt_file.paths) {
            auto part = audit_numpy_mlx_python_file(tgt_path);
            a.numpy_imports += part.numpy_imports;
            a.numpy_refs += part.numpy_refs;
            a.mlx_imports += part.mlx_imports;
            a.mlx_refs += part.mlx_refs;
        }

        total_numpy_imports += a.numpy_imports;
        total_numpy_refs += a.numpy_refs;
        total_mlx_imports += a.mlx_imports;
        total_mlx_refs += a.mlx_refs;
        if (a.numpy_imports > 0 || a.numpy_refs > 0) files_with_numpy++;

        Row r;
        r.qualified = m.target_qualified;
        r.rel_path = tgt_file.relative_path;
        r.audit = a;
        r.similarity = m.similarity;
        rows.push_back(std::move(r));
    }

    std::cout << "Matched files: " << comp.matches.size() << "\n";
    std::cout << "Unmatched:     " << comp.unmatched_source.size() << " source, "
              << comp.unmatched_target.size() << " target\n\n";

    std::cout << "=== Residual NumPy Usage In Target ===\n";
    std::cout << "Files with NumPy patterns: " << files_with_numpy << "\n";
    std::cout << "NumPy imports (heuristic): " << total_numpy_imports << "\n";
    std::cout << "NumPy refs (heuristic):    " << total_numpy_refs << "\n\n";

    std::cout << "=== MLX Usage In Target (Informational) ===\n";
    std::cout << "MLX imports (heuristic): " << total_mlx_imports << "\n";
    std::cout << "MLX refs (heuristic):    " << total_mlx_refs << "\n\n";

    if (files_with_numpy > 0) {
        std::cout << "Top files still referencing NumPy (target):\n";
        std::cout << std::setw(40) << std::left << "File"
                  << std::setw(8) << "Sim"
                  << std::setw(8) << "Imp"
                  << std::setw(8) << "Refs"
                  << "Path\n";
        std::cout << std::string(80, '-') << "\n";

        int shown = 0;
        for (const auto& r : rows) {
            if (r.audit.numpy_imports == 0 && r.audit.numpy_refs == 0) continue;
            if (shown++ >= 50) {
                std::cout << "... and " << (files_with_numpy - 50) << " more\n";
                break;
            }
            std::cout << std::setw(40) << std::left << r.qualified.substr(0, 38)
                      << std::setw(8) << std::fixed << std::setprecision(2) << r.similarity
                      << std::setw(8) << r.audit.numpy_imports
                      << std::setw(8) << r.audit.numpy_refs
                      << r.rel_path << "\n";
        }
        std::cout << "\n";
    }

    comp.print_report();
}

struct EmberLintCounts {
    int python_numpy_imports = 0;
    int python_numpy_refs = 0;
    int python_precision_casts = 0;
    int python_tensor_conversions = 0;
    int python_operators = 0;
    int cpp_pybind_numpy_include = 0;
    int cpp_py_array_t = 0;
    int dep_numpy_lines = 0;
};

static bool should_skip_scan_path(const std::filesystem::path& p) {
    auto s = p.string();
    return s.find("/target/") != std::string::npos ||
           s.find("/build/") != std::string::npos ||
           s.find("/build_") != std::string::npos ||
           s.find("/_deps/") != std::string::npos ||
           s.find("/.git/") != std::string::npos ||
           s.find("/.venv/") != std::string::npos ||
           s.find("/__pycache__/") != std::string::npos;
}

static EmberLintCounts emberlint_scan_file(const std::filesystem::path& path) {
    EmberLintCounts c;
    std::ifstream f(path);
    if (!f.is_open()) return c;
    std::stringstream buffer;
    buffer << f.rdbuf();
    std::string s = buffer.str();

    auto ext = path.extension().string();

    if (ext == ".py") {
        // Semantic checks (tree-sitter-python):
        // - NumPy imports and alias-aware references (AST-based, ignores comments/strings)
        // - precision-reducing casts: float(...), int(...), bool(...)
        // - graph-breaking conversions: .numpy(), .item(), .tolist(), numpy.array/asarray(...)
        // - python operators on expressions (excluding subscript indices and obvious string concatenation)
        static thread_local TSParser* parser = nullptr;
        if (parser == nullptr) {
            parser = ts_parser_new();
            if (parser != nullptr) {
                (void)ts_parser_set_language(parser, tree_sitter_python());
            }
        }

        if (parser != nullptr && ts_parser_set_language(parser, tree_sitter_python())) {
            TSTree* tree = ts_parser_parse_string(parser, nullptr, s.c_str(), static_cast<uint32_t>(s.size()));
            if (tree != nullptr) {
                TSNode root = ts_tree_root_node(tree);

                auto node_text = [&](TSNode n) -> std::string {
                    uint32_t start = ts_node_start_byte(n);
                    uint32_t end = ts_node_end_byte(n);
                    if (end > start && end <= s.size()) {
                        return s.substr(start, end - start);
                    }
                    return "";
                };

                // Collect NumPy aliases and count from-imports.
                std::set<std::string> numpy_aliases;
                int from_numpy_imports = 0;

                auto is_numpy_module = [](const std::string& mod) -> bool {
                    // Avoid false positives like "numpy_to_mlx".
                    return mod == "numpy" || mod.rfind("numpy.", 0) == 0;
                };

                std::function<void(TSNode)> collect_imports = [&](TSNode n) {
                    const char* t = ts_node_type(n);
                    if (strcmp(t, "import_statement") == 0) {
                        uint32_t cc = ts_node_child_count(n);
                        for (uint32_t i = 0; i < cc; ++i) {
                            TSNode ch = ts_node_child(n, i);
                            if (!ts_node_is_named(ch)) continue;
                            const char* ct = ts_node_type(ch);
                            if (strcmp(ct, "dotted_name") == 0) {
                                std::string mod = node_text(ch);
                                if (is_numpy_module(mod)) {
                                    // `import numpy` or `import numpy.linalg` binds name `numpy`.
                                    numpy_aliases.insert("numpy");
                                }
                            } else if (strcmp(ct, "aliased_import") == 0) {
                                std::string mod;
                                std::string alias;
                                uint32_t icc = ts_node_child_count(ch);
                                for (uint32_t j = 0; j < icc; ++j) {
                                    TSNode ich = ts_node_child(ch, j);
                                    if (!ts_node_is_named(ich)) continue;
                                    const char* it = ts_node_type(ich);
                                    if (strcmp(it, "dotted_name") == 0) {
                                        mod = node_text(ich);
                                    } else if (strcmp(it, "identifier") == 0) {
                                        alias = node_text(ich);
                                    }
                                }
                                if (!mod.empty() && is_numpy_module(mod)) {
                                    // `import numpy as np` or `import numpy.linalg as la`
                                    if (!alias.empty()) numpy_aliases.insert(alias);
                                    else numpy_aliases.insert("numpy");
                                }
                            }
                        }
                    } else if (strcmp(t, "import_from_statement") == 0) {
                        // Ignore relative imports (`from . import ...`).
                        bool has_relative = false;
                        uint32_t cc = ts_node_child_count(n);
                        for (uint32_t i = 0; i < cc; ++i) {
                            TSNode ch = ts_node_child(n, i);
                            if (!ts_node_is_named(ch)) continue;
                            if (strcmp(ts_node_type(ch), "relative_import") == 0) {
                                has_relative = true;
                                break;
                            }
                        }
                        if (!has_relative) {
                            // First dotted_name child is the module.
                            TSNode module_dn{};
                            bool found = false;
                            for (uint32_t i = 0; i < cc; ++i) {
                                TSNode ch = ts_node_child(n, i);
                                if (!ts_node_is_named(ch)) continue;
                                if (strcmp(ts_node_type(ch), "dotted_name") == 0) {
                                    module_dn = ch;
                                    found = true;
                                    break;
                                }
                            }
                            if (found) {
                                std::string mod = node_text(module_dn);
                                if (is_numpy_module(mod)) {
                                    from_numpy_imports++;
                                }
                            }
                        }
                    }

                    uint32_t cc = ts_node_child_count(n);
                    for (uint32_t i = 0; i < cc; ++i) {
                        collect_imports(ts_node_child(n, i));
                    }
                };

                collect_imports(root);
                c.python_numpy_imports += static_cast<int>(numpy_aliases.size()) + from_numpy_imports;

                std::function<bool(TSNode, const char*)> subtree_has_type = [&](TSNode n, const char* want) -> bool {
                    if (strcmp(ts_node_type(n), want) == 0) return true;
                    uint32_t cc = ts_node_child_count(n);
                    for (uint32_t i = 0; i < cc; ++i) {
                        if (subtree_has_type(ts_node_child(n, i), want)) return true;
                    }
                    return false;
                };

                std::function<void(TSNode, const std::string&, bool)> walk =
                    [&](TSNode n, const std::string& current_fn, bool in_subscript_index) {
                        const char* t = ts_node_type(n);
                        std::string type_s(t);

                        std::string fn = current_fn;
                        if (type_s == "function_definition") {
                            TSNode name_node = ts_node_child_by_field_name(n, "name", 4);
                            if (!ts_node_is_null(name_node)) {
                                fn = node_text(name_node);
                            } else {
                                // Fallback: first identifier child
                                uint32_t ncc = ts_node_child_count(n);
                                for (uint32_t i = 0; i < ncc; ++i) {
                                    TSNode ch = ts_node_child(n, i);
                                    if (strcmp(ts_node_type(ch), "identifier") == 0) {
                                        fn = node_text(ch);
                                        break;
                                    }
                                }
                            }
                        }

                        bool in_stringy_fn = (!fn.empty() &&
                                              (fn.find("hash") != std::string::npos ||
                                               fn.find("Hash") != std::string::npos ||
                                               fn.find("str") != std::string::npos ||
                                               fn.find("Str") != std::string::npos));

                        if (type_s == "attribute") {
                            TSNode obj_node = ts_node_child_by_field_name(n, "object", 6);
                            if (!ts_node_is_null(obj_node) && strcmp(ts_node_type(obj_node), "identifier") == 0) {
                                std::string obj = node_text(obj_node);
                                if (numpy_aliases.count(obj)) {
                                    c.python_numpy_refs++;
                                }
                            }
                        }

                        if (type_s == "call") {
                            TSNode func_node = ts_node_child_by_field_name(n, "function", 8);
                            if (!ts_node_is_null(func_node)) {
                                const char* ft = ts_node_type(func_node);
                                if (strcmp(ft, "identifier") == 0) {
                                    std::string callee = node_text(func_node);
                                    if (callee == "float" || callee == "int" || callee == "bool") {
                                        // Avoid flagging obvious constant parsing (e.g. float("3") / float(3)).
                                        TSNode args = ts_node_child_by_field_name(n, "arguments", 9);
                                        if (ts_node_is_null(args)) {
                                            // Tree-sitter-python uses argument_list; be robust.
                                            uint32_t cc2 = ts_node_child_count(n);
                                            for (uint32_t i = 0; i < cc2; ++i) {
                                                TSNode ch = ts_node_child(n, i);
                                                if (ts_node_is_named(ch) && strcmp(ts_node_type(ch), "argument_list") == 0) {
                                                    args = ch;
                                                    break;
                                                }
                                            }
                                        }

                                        bool count_cast = false;
                                        if (!ts_node_is_null(args) && ts_node_named_child_count(args) > 0) {
                                            TSNode first = ts_node_named_child(args, 0);
                                            const char* at = ts_node_type(first);
                                            bool is_literal = (strcmp(at, "integer") == 0 ||
                                                               strcmp(at, "float") == 0 ||
                                                               strcmp(at, "string") == 0 ||
                                                               strcmp(at, "true") == 0 ||
                                                               strcmp(at, "false") == 0 ||
                                                               strcmp(at, "none") == 0);
                                            count_cast = !is_literal;
                                        }
                                        if (count_cast) {
                                            c.python_precision_casts++;
                                        }
                                    }
                                } else if (strcmp(ft, "attribute") == 0) {
                                    TSNode attr_name_node = ts_node_child_by_field_name(func_node, "attribute", 9);
                                    TSNode attr_obj_node = ts_node_child_by_field_name(func_node, "object", 6);
                                    std::string attr_name;
                                    if (!ts_node_is_null(attr_name_node)) {
                                        attr_name = node_text(attr_name_node);
                                    }

                                    if (attr_name == "numpy") {
                                        c.python_tensor_conversions++;
                                    } else if (attr_name == "item" || attr_name == "tolist") {
                                        c.python_tensor_conversions++;
                                    } else if (attr_name == "array" || attr_name == "asarray") {
                                        // numpy.array/asarray(...)
                                        if (!ts_node_is_null(attr_obj_node) &&
                                            strcmp(ts_node_type(attr_obj_node), "identifier") == 0) {
                                            std::string obj = node_text(attr_obj_node);
                                            if (numpy_aliases.count(obj)) {
                                                c.python_tensor_conversions++;
                                            }
                                        }
                                    }
                                }
                            }
                        } else if (type_s == "binary_operator") {
                            // operator token is an unnamed child.
                            std::string op;
                            uint32_t cc = ts_node_child_count(n);
                            for (uint32_t i = 0; i < cc; ++i) {
                                TSNode ch = ts_node_child(n, i);
                                if (!ts_node_is_named(ch)) {
                                    op = ts_node_type(ch);
                                    break;
                                }
                            }

                            bool is_op = (op == "+" || op == "-" || op == "*" || op == "/" ||
                                          op == "//" || op == "%" || op == "**" || op == "@");
                            if (is_op && !in_subscript_index && !in_stringy_fn) {
                                bool is_non_tensor_op = false;
                                if (op == "+") {
                                    // Heuristic: avoid string concatenation.
                                    if (ts_node_named_child_count(n) >= 2) {
                                        TSNode left = ts_node_named_child(n, 0);
                                        TSNode right = ts_node_named_child(n, ts_node_named_child_count(n) - 1);
                                        if (subtree_has_type(left, "string") || subtree_has_type(right, "string")) {
                                            is_non_tensor_op = true;
                                        }
                                    }
                                }
                                if (!is_non_tensor_op) {
                                    c.python_operators++;
                                }
                            }
                        }

                        // Recurse
                        if (type_s == "subscript") {
                            // Only treat the index/slice expression as safe for operators.
                            TSNode index = ts_node_child_by_field_name(n, "subscript", 9);
                            if (ts_node_is_null(index)) {
                                index = ts_node_child_by_field_name(n, "index", 5);
                            }
                            if (ts_node_is_null(index)) {
                                index = ts_node_child_by_field_name(n, "slice", 5);
                            }

                            uint32_t cc2 = ts_node_child_count(n);
                            for (uint32_t i = 0; i < cc2; ++i) {
                                TSNode ch = ts_node_child(n, i);
                                bool child_is_index = (!ts_node_is_null(index) && ts_node_eq(ch, index));
                                walk(ch, fn, in_subscript_index || child_is_index);
                            }
                            return;
                        }

                        uint32_t cc2 = ts_node_child_count(n);
                        for (uint32_t i = 0; i < cc2; ++i) {
                            walk(ts_node_child(n, i), fn, in_subscript_index);
                        }
                    };

                walk(root, "", false);
                ts_tree_delete(tree);
            }
        }
    } else if (ext == ".cpp" || ext == ".cc" || ext == ".h" || ext == ".hpp") {
        // Only count real includes / real typed usage, not string literals in tools.
        static const std::regex include_numpy(R"(^\s*#\s*include\s*[<"]pybind11/numpy\.h[>"])",
                                             std::regex_constants::multiline);
        static const std::regex py_array_t(R"(\bpy::array_t\s*<)");

        c.cpp_pybind_numpy_include += static_cast<int>(
            std::distance(std::sregex_iterator(s.begin(), s.end(), include_numpy), std::sregex_iterator()));
        c.cpp_py_array_t += static_cast<int>(
            std::distance(std::sregex_iterator(s.begin(), s.end(), py_array_t), std::sregex_iterator()));
    } else {
        // Dependency/config files: detect actual dependencies (not docstring conventions or comments).
        if (path.filename() == "pyproject.toml" ||
            path.filename() == "environment.yml" ||
            path.filename() == "requirements.txt" ||
            path.string().find("requirements/") != std::string::npos) {
            std::istringstream lines(s);
            std::string line;

            auto strip_comment = [](const std::string& line2) -> std::string {
                auto p = line2.find('#');
                if (p == std::string::npos) return line2;
                return line2.substr(0, p);
            };

            if (path.filename() == "pyproject.toml") {
                bool in_dep_array = false;
                while (std::getline(lines, line)) {
                    std::string no_comment = strip_comment(line);
                    if (no_comment.find_first_not_of(" \t\r\n") == std::string::npos) continue;

                    if (!in_dep_array) {
                        static const std::regex dep_start(R"(^\s*(dependencies|requires|optional-dependencies)\s*=\s*\[)");
                        if (std::regex_search(no_comment, dep_start)) {
                            in_dep_array = true;
                        } else {
                            continue;
                        }
                    }

                    if (no_comment.find("\"numpy") != std::string::npos ||
                        no_comment.find("'numpy") != std::string::npos) {
                        c.dep_numpy_lines++;
                    }

                    if (no_comment.find(']') != std::string::npos) {
                        in_dep_array = false;
                    }
                }
            } else if (path.filename() == "environment.yml") {
                static const std::regex env_dep(R"(^\s*-\s*numpy(\b|[<>=!~]))");
                while (std::getline(lines, line)) {
                    std::string no_comment = strip_comment(line);
                    if (std::regex_search(no_comment, env_dep)) {
                        c.dep_numpy_lines++;
                    }
                }
            } else {
                // requirements*.txt and requirements/...
                static const std::regex req_dep(R"(^\s*numpy(\b|[<>=!~\[]))", std::regex_constants::icase);
                while (std::getline(lines, line)) {
                    std::string no_comment = strip_comment(line);
                    if (no_comment.find_first_not_of(" \t\r\n") == std::string::npos) continue;
                    if (std::regex_search(no_comment, req_dep)) {
                        c.dep_numpy_lines++;
                    }
                }
            }
        }
    }

    return c;
}

void cmd_emberlint(const std::string& root) {
    std::cout << "=== EmberLint (fast checks) ===\n\n";
    std::cout << "Path: " << root << "\n\n";

    EmberLintCounts total;

    struct Hit {
        std::string rel;
        EmberLintCounts c;
    };
    std::vector<Hit> hits;

    std::filesystem::path rp(root);
    if (std::filesystem::is_regular_file(rp)) {
        auto c = emberlint_scan_file(rp);
        total.python_numpy_imports += c.python_numpy_imports;
        total.python_numpy_refs += c.python_numpy_refs;
        total.python_precision_casts += c.python_precision_casts;
        total.python_tensor_conversions += c.python_tensor_conversions;
        total.python_operators += c.python_operators;
        total.cpp_pybind_numpy_include += c.cpp_pybind_numpy_include;
        total.cpp_py_array_t += c.cpp_py_array_t;
        total.dep_numpy_lines += c.dep_numpy_lines;
        hits.push_back(Hit{rp.filename().string(), c});
    } else {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(rp)) {
            if (!entry.is_regular_file()) continue;
            if (should_skip_scan_path(entry.path())) continue;

            auto c = emberlint_scan_file(entry.path());
            total.python_numpy_imports += c.python_numpy_imports;
            total.python_numpy_refs += c.python_numpy_refs;
            total.python_precision_casts += c.python_precision_casts;
            total.python_tensor_conversions += c.python_tensor_conversions;
            total.python_operators += c.python_operators;
            total.cpp_pybind_numpy_include += c.cpp_pybind_numpy_include;
            total.cpp_py_array_t += c.cpp_py_array_t;
            total.dep_numpy_lines += c.dep_numpy_lines;

            bool has_hard_issue =
                (c.python_numpy_imports != 0 ||
                 c.python_numpy_refs != 0 ||
                 c.python_precision_casts != 0 ||
                 c.python_tensor_conversions != 0 ||
                 c.cpp_pybind_numpy_include != 0 ||
                 c.cpp_py_array_t != 0 ||
                 c.dep_numpy_lines != 0);
            if (!has_hard_issue) {
                continue;
            }

            hits.push_back(Hit{std::filesystem::relative(entry.path(), rp).string(), c});
        }
    }

    std::cout << "Summary:\n";
    std::cout << "  Python NumPy imports: " << total.python_numpy_imports << "\n";
    std::cout << "  Python NumPy refs:    " << total.python_numpy_refs << "\n";
    std::cout << "  Precision casts:      " << total.python_precision_casts << "\n";
    std::cout << "  Tensor conversions:   " << total.python_tensor_conversions << "\n";
    std::cout << "  Python operators:     " << total.python_operators << "\n";
    std::cout << "  C++ pybind NumPy:     " << total.cpp_pybind_numpy_include << "\n";
    std::cout << "  C++ py::array_t:      " << total.cpp_py_array_t << "\n";
    std::cout << "  Dep/config 'numpy':   " << total.dep_numpy_lines << "\n\n";

    if (!hits.empty()) {
        std::sort(hits.begin(), hits.end(), [](const Hit& a, const Hit& b) {
            auto score = [](const EmberLintCounts& c) {
                return 10 * c.python_numpy_imports +
                       3 * c.python_numpy_refs +
                       8 * c.python_precision_casts +
                       8 * c.python_tensor_conversions +
                       20 * c.cpp_pybind_numpy_include +
                       5 * c.cpp_py_array_t +
                       c.dep_numpy_lines;
            };
            return score(a.c) > score(b.c);
        });

        std::cout << "Top hits:\n";
        std::cout << std::setw(8) << "PyImp"
                  << std::setw(8) << "PyRef"
                  << std::setw(8) << "Cast"
                  << std::setw(8) << "Conv"
                  << std::setw(8) << "Ops"
                  << std::setw(8) << "CppInc"
                  << std::setw(8) << "ArrT"
                  << std::setw(8) << "Deps"
                  << "Path\n";
        std::cout << std::string(80, '-') << "\n";

        int shown = 0;
        for (const auto& h : hits) {
            if (shown++ >= 50) {
                std::cout << "... and " << (hits.size() - 50) << " more\n";
                break;
            }
            std::cout << std::setw(8) << h.c.python_numpy_imports
                      << std::setw(8) << h.c.python_numpy_refs
                      << std::setw(8) << h.c.python_precision_casts
                      << std::setw(8) << h.c.python_tensor_conversions
                      << std::setw(8) << h.c.python_operators
                      << std::setw(8) << h.c.cpp_pybind_numpy_include
                      << std::setw(8) << h.c.cpp_py_array_t
                      << std::setw(8) << h.c.dep_numpy_lines
                      << h.rel << "\n";
        }
        std::cout << "\n";
    }

    bool ok = (total.python_numpy_imports == 0 &&
               total.python_numpy_refs == 0 &&
               total.python_precision_casts == 0 &&
               total.python_tensor_conversions == 0 &&
               total.cpp_pybind_numpy_include == 0 &&
               total.cpp_py_array_t == 0 &&
               total.dep_numpy_lines == 0);
    if (ok) {
        std::cout << "OK: no issues found.\n";
    } else {
        std::cout << "FAIL: issues found.\n";
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
    (void)incomplete_count;
    
    std::cout << "\n=== Generating Reports ===\n\n";
    
    // Calculate statistics
    int total_source = source.files.size();
    int total_target_logical = target.files.size();
    int total_target_physical = 0;
    for (auto const& [key, val] : target.files) {
        (void)key;
        total_target_physical += static_cast<int>(val.paths.size());
    }
    int matched = comp.matches.size();
    float completion_pct = (static_cast<float>(matched) / static_cast<float>(total_source)) * 100.0f;
    
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
        report << "| Target units (paired) | " << total_target_logical << " | - |\n";
        report << "| Target files (total) | " << total_target_physical << " | - |\n";
        report << "| Porting progress | " << matched << " | "
               << std::fixed << std::setprecision(1)
               << completion_pct << "% (matched) |\n";
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

	        report << "## Incorrect Ports (Missing Types)\n\n";
	        report << "These files are matched (often via `// port-lint`) but appear to be missing one or more type declarations\n";
	        report << "present in the Rust source file.\n\n";
	        report << "| Source | Target | Missing types | Examples |\n";
	        report << "|--------|--------|---------------|----------|\n";
	        int shown_incorrect = 0;
	        for (const auto& m : ranked) {
	            if (m.source_type_count == 0) continue;
	            if (m.type_coverage >= 1.0f) continue;
	            if (shown_incorrect++ >= 25) break;
	            report << "| `" << m.source_qualified << "` | `" << m.target_qualified << "` | "
	                   << (m.source_type_count - m.matched_type_count) << "/" << m.source_type_count << " | ";
	            if (!m.missing_types.empty()) {
	                // Show up to 3 missing type names
	                int shown_names = 0;
	                for (const auto& name : m.missing_types) {
	                    if (shown_names++ >= 3) break;
	                    if (shown_names > 1) report << ", ";
	                    report << "`" << name << "`";
	                }
	                if (static_cast<int>(m.missing_types.size()) > 3) {
	                    report << " …";
	                }
	            } else {
	                report << "-";
	            }
	            report << " |\n";
	        }
	        if (shown_incorrect == 0) {
	            report << "| _None detected_ | | | |\n";
	        }
	        report << "\n";
	        
	        report << "## High Priority Missing Files\n\n";
	        if (missing.empty()) {
	            report << "No missing files detected.\n\n";
	        } else {
	            report << "| Rank | Source file | Deps | Path |\n";
	            report << "|------|------------|------|------|\n";
	            int shown_missing = 0;
	            for (const auto* sf : missing) {
	                if (shown_missing++ >= 20) break;
	                report << "| " << shown_missing << " | `" << sf->qualified_name << "` | "
	                       << sf->dependent_count << " | `" << sf->relative_path << "` |\n";
	            }
	            if (missing.size() > 20) {
	                report << "\n... and " << (missing.size() - 20) << " more missing files.\n";
	            }
	            report << "\n";
	        }
	        
	        report << "## Documentation Gaps\n\n";
	        float doc_coverage_pct = 0.0f;
	        bool docs_missing = false;
	        if (total_src_doc_lines > 0) {
	            doc_coverage_pct = 100.0f * total_tgt_doc_lines / total_src_doc_lines;
	            docs_missing = doc_coverage_pct < 85.0f;
	        }
	        if (docs_missing) {
	            report << "There is missing documentation that is hurting overall scoring.\n\n";
	        }
	        report << "**Documentation coverage:** " << total_tgt_doc_lines << " / " 
	               << total_src_doc_lines << " lines (";
	        if (total_src_doc_lines > 0) {
	            report << std::fixed << std::setprecision(0)
	                   << (100.0f * total_tgt_doc_lines / total_src_doc_lines) << "%)\n\n";
	        } else {
	            report << "N/A)\n\n";
	        }
	        
	        report << "Top documentation gaps (>20%):\n\n";
	        if (doc_gaps.empty()) {
	            report << "No significant documentation gaps found.\n\n";
	        } else {
	            int shown_docs = 0;
	            for (const auto& [gap, m] : doc_gaps) {
	                if (shown_docs++ >= 15) break;
	                report << "- `" << m->source_qualified << "` - " 
	                       << std::fixed << std::setprecision(0) << (gap * 100) << "% gap ("
	                       << m->source_doc_lines << " → " << m->target_doc_lines << " lines)\n";
	            }
	            if (doc_gaps.size() > 15) {
	                report << "\n... and " << (doc_gaps.size() - 15) << " more files with doc gaps.\n";
	            }
	            report << "\n";
	        }
	        
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

	        report << "## Missing Files (Top by Dependents)\n\n";
	        if (missing.empty()) {
	            report << "No missing files detected.\n\n";
	        } else {
	            report << "| Rank | Source file | Deps | Path |\n";
	            report << "|------|------------|------|------|\n";
	            int shown_missing = 0;
	            for (const auto* sf : missing) {
	                if (shown_missing++ >= 20) break;
	                report << "| " << shown_missing << " | `" << sf->qualified_name << "` | "
	                       << sf->dependent_count << " | `" << sf->relative_path << "` |\n";
	            }
	            if (missing.size() > 20) {
	                report << "\n... and " << (missing.size() - 20) << " more missing files.\n";
	            }
	            report << "\n";
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
               << completion_pct << "% (" << total_target_physical << "/" << total_source << " files)\n";
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
	            if (sf->dependent_count >= 10 && p2_count < 10) {
	                p2_count++;
	                report << p2_count << ". **" << sf->qualified_name << "** (" 
	                       << sf->dependent_count << " deps)\n";
	                report << "   - Path: `" << sf->relative_path << "`\n";
	                report << "   - Essential for " << sf->dependent_count << " other files\n\n";
	            }
	        }
	        if (p2_count == 0) {
	            report << "No missing high-value files detected.\n\n";
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

    auto ranked = comp.ranked_for_porting();

    // Agent-scoped dashboard (guardrails): in task mode, lock the view to the assigned file
    // and its direct imports to prevent "dashboard skipping" and out-of-scope prioritization.
    bool guard_active = (g_guardrails.active && g_guardrails.agent > 0);
    if (guard_active && !g_guardrails.override_mode) {
        TaskManager tm(g_guardrails.task_file);
        if (tm.load()) {
            print_agent_activity_section(tm, g_guardrails.task_file, g_guardrails.agent);

            std::string agent_id = std::to_string(g_guardrails.agent);
            const PortTask* focus = nullptr;
            for (const auto& t : tm.tasks) {
                if (t.status == TaskStatus::ASSIGNED && t.assigned_to == agent_id) {
                    focus = &t;
                    break;
                }
            }

            std::cout << "\n=== Agent Scope Dashboard ===\n\n";
            std::cout << "Task file: " << g_guardrails.task_file << "\n";
            std::cout << "You are agent #" << g_guardrails.agent << "\n\n";

            if (!focus) {
                std::cout << "No task assigned to agent #" << g_guardrails.agent << ".\n";
                std::cout << "Run: ast_distance --assign " << g_guardrails.task_file
                          << " " << g_guardrails.agent << "\n\n";
                std::cout << "Note: full project dashboard is locked in agent scope.\n";
                std::cout << "Use --override if you need full-project output.\n";
                return;
            }

            // Build relative_path -> logical key maps for locating the focus file.
            std::map<std::string, std::string> src_rel_to_key;
            for (const auto& [k, sf] : source.files) {
                src_rel_to_key[sf.relative_path] = k;
            }
            std::map<std::string, std::string> tgt_rel_to_key;
            for (const auto& [k, sf] : target.files) {
                tgt_rel_to_key[sf.relative_path] = k;
            }

            std::string focus_src_key;
            auto it_src = src_rel_to_key.find(focus->source_path);
            if (it_src != src_rel_to_key.end()) focus_src_key = it_src->second;

            std::string focus_tgt_key;
            auto it_tgt = tgt_rel_to_key.find(focus->target_path);
            if (it_tgt != tgt_rel_to_key.end()) focus_tgt_key = it_tgt->second;

            std::filesystem::path focus_source_path =
                std::filesystem::path(tm.source_root) / focus->source_path;
            std::filesystem::path focus_target_path =
                std::filesystem::path(tm.target_root) / focus->target_path;

            std::cout << "Assigned task: " << focus->source_qualified << "\n";
            std::cout << "Source: " << focus_source_path.string();
            if (!std::filesystem::exists(focus_source_path)) std::cout << " (missing?)";
            std::cout << "\n";
            std::cout << "Target: " << focus_target_path.string();
            if (std::filesystem::exists(focus_target_path)) std::cout << " (exists)";
            else std::cout << " (missing)";
            std::cout << "\n\n";

            // Scope: focus file + its direct imports, plus the target file + its imports (if it exists).
            std::set<std::string> scope_source_keys;
            std::set<std::string> scope_target_keys;
            if (!focus_src_key.empty()) {
                scope_source_keys.insert(focus_src_key);
                const auto& sf = source.files.at(focus_src_key);
                scope_source_keys.insert(sf.depends_on.begin(), sf.depends_on.end());
            }
            if (!focus_tgt_key.empty()) {
                scope_target_keys.insert(focus_tgt_key);
                const auto& tf = target.files.at(focus_tgt_key);
                scope_target_keys.insert(tf.depends_on.begin(), tf.depends_on.end());
            }

            std::cout << "Scope: " << scope_source_keys.size()
                      << " source files (focus + imports), " << scope_target_keys.size()
                      << " target files (focus + imports)\n";

            // Determine which scoped imports are already assigned to other agents.
            std::map<std::string, int> assigned_to_agent;
            for (const auto& t : tm.tasks) {
                if (t.status != TaskStatus::ASSIGNED) continue;
                int a = 0;
                try {
                    a = std::stoi(t.assigned_to);
                } catch (...) {
                    continue;
                }
                assigned_to_agent[t.source_qualified] = a;
            }

            std::map<std::string, int> blocked_by;
            for (const auto& key : scope_source_keys) {
                if (key == focus_src_key) continue;
                const auto& sf = source.files.at(key);
                auto ita = assigned_to_agent.find(sf.qualified_name);
                if (ita != assigned_to_agent.end() && ita->second != g_guardrails.agent) {
                    blocked_by[key] = ita->second;
                }
            }

            if (!blocked_by.empty()) {
                std::cout << "\nBlocked imports (assigned to other agents):\n";
                for (const auto& [key, a] : blocked_by) {
                    const auto& sf = source.files.at(key);
                    std::cout << "  - " << sf.qualified_name << " (agent #" << a << ")\n";
                }
            }

            // Match lookup by source logical key.
            std::map<std::string, const CodebaseComparator::Match*> match_by_source;
            for (const auto& m : comp.matches) {
                match_by_source[m.source_path] = &m;
            }
            std::set<std::string> unmatched_source(
                comp.unmatched_source.begin(), comp.unmatched_source.end());

            auto format_fn_parity = [](const CodebaseComparator::Match* m) -> std::string {
                if (!m || m->source_function_count <= 0) return "-";
                return std::to_string(m->matched_function_count) + "/" +
                       std::to_string(m->source_function_count);
            };

            auto print_scope_row = [&](const std::string& key, bool is_focus) {
                const auto& sf = source.files.at(key);
                const CodebaseComparator::Match* m = nullptr;
                auto it = match_by_source.find(key);
                if (it != match_by_source.end()) m = it->second;

                bool missing = unmatched_source.count(key) > 0;
                std::string status;
                if (is_focus) status = "FOCUS";
                if (missing) status = status.empty() ? "MISSING_FILE" : status + ",MISSING_FILE";
                if (m && m->is_stub) status = status.empty() ? "STUB(FAIL)" : status + ",STUB(FAIL)";
                if (m && m->todo_count > 0) status = status.empty() ? "TODO(FAIL)" : status + ",TODO(FAIL)";
                if (m && m->source_function_count > 0 &&
                    m->matched_function_count < m->source_function_count) {
                    status = status.empty() ? "MISSING_FUNCTIONS" : status + ",MISSING_FUNCTIONS";
                }
                auto itb = blocked_by.find(key);
                if (itb != blocked_by.end()) {
                    std::string b = "BLOCKED(agent #" + std::to_string(itb->second) + ")";
                    status = status.empty() ? b : status + "," + b;
                }

                int dependents = sf.dependent_count;
                std::string sim_str = "-";
                int todos = 0;
                int lint = 0;
                if (m && !missing) {
                    std::ostringstream sim;
                    sim << std::fixed << std::setprecision(2) << m->similarity;
                    sim_str = sim.str();
                    todos = m->todo_count;
                    lint = m->lint_count;
                }

                std::cout << std::setw(30) << std::left << sf.qualified_name.substr(0, 28)
                          << std::setw(11) << dependents
                          << std::setw(11) << sim_str
                          << std::setw(16) << format_fn_parity(m)
                          << std::setw(8) << todos
                          << std::setw(6) << lint
                          << status
                          << "\n";
            };

            std::cout << "\n=== Scope Work Items ===\n\n";
            std::cout << std::setw(30) << std::left << "File"
                      << std::setw(11) << "Dependents"
                      << std::setw(11) << "Similarity"
                      << std::setw(16) << "Function parity"
                      << std::setw(8) << "TODOs"
                      << std::setw(6) << "Lint"
                      << "Status\n";
            std::cout << std::string(90, '-') << "\n";

            if (!focus_src_key.empty()) {
                print_scope_row(focus_src_key, true);
            } else {
                std::cout << std::setw(30) << std::left << focus->source_qualified.substr(0, 28)
                          << std::setw(11) << focus->dependent_count
                          << std::setw(11) << "-"
                          << std::setw(16) << "-"
                          << std::setw(8) << 0
                          << std::setw(6) << 0
                          << "FOCUS,UNKNOWN_SOURCE_PATH\n";
            }

            std::vector<std::string> others;
            for (const auto& k : scope_source_keys) {
                if (k == focus_src_key) continue;
                others.push_back(k);
            }
            std::sort(others.begin(), others.end(),
                      [&](const std::string& a, const std::string& b) {
                          return source.files.at(a).dependent_count >
                                 source.files.at(b).dependent_count;
                      });
            for (const auto& k : others) {
                print_scope_row(k, false);
            }

            // Deterministic next action hint for the focus file.
            std::cout << "\n=== Next Action (Focus) ===\n\n";
            const CodebaseComparator::Match* focus_match = nullptr;
            if (!focus_src_key.empty()) {
                auto it = match_by_source.find(focus_src_key);
                if (it != match_by_source.end()) focus_match = it->second;
            }

            if (!focus_match || unmatched_source.count(focus_src_key) > 0) {
                std::cout << "Target file is missing. Create it and port the full implementation.\n";
            } else if (focus_match->is_stub) {
                std::cout << "Replace the stub with a real implementation (stubs are a failure mode).\n";
            } else if (focus_match->todo_count > 0) {
                std::cout << "Remove TODOs and finish the implementation (TODOs are a failure mode).\n";
            } else if (focus_match->source_function_count > 0 &&
                       focus_match->matched_function_count < focus_match->source_function_count) {
                std::cout << "Port missing functions to reach per-file parity.\n";
            } else if (focus_match->similarity < 0.85f) {
                std::cout << "Improve similarity to >= 0.85 (whole-file + identifier-content + parity).\n";
            } else {
                std::cout << "Looks complete. If you have validated behavior and tests, mark it complete.\n";
            }
            std::cout << "\nMark complete with:\n";
            std::cout << "  ast_distance --agent " << g_guardrails.agent << " --complete "
                      << g_guardrails.task_file << " " << focus->source_qualified << "\n";

            std::cout << "\nNote: full project dashboard is locked in agent scope.\n";
            std::cout << "Use --override to view the full project view.\n";
            return;
        }
    }

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

		    int incomplete = 0;
		    int func_gap_files = 0;
		    for (const auto& m : ranked) {
	        if (m.similarity < 0.6) incomplete++;
	        if (m.source_function_count > 0 &&
	            m.matched_function_count < m.source_function_count) {
	            func_gap_files++;
	        }
	    }

	    int total_src_doc_lines = 0;
	    int total_tgt_doc_lines = 0;
	    for (const auto& m : comp.matches) {
	        total_src_doc_lines += m.source_doc_lines;
	        total_tgt_doc_lines += m.target_doc_lines;
	    }
	    const float kDocCoverageWarnPct = 85.0f;
	    float doc_coverage_pct = 0.0f;
	    if (total_src_doc_lines > 0) {
	        doc_coverage_pct = 100.0f * static_cast<float>(total_tgt_doc_lines) /
	                           static_cast<float>(total_src_doc_lines);
	    }
	    bool docs_missing = (total_src_doc_lines > 0 && doc_coverage_pct < kDocCoverageWarnPct);

	    std::cout << "\n=== Big Picture ===\n\n";
	    std::cout << "- Missing files: " << comp.unmatched_source.size() << "\n";
	    std::cout << "- Incomplete ports (similarity < 60%): " << incomplete << "\n";
	    std::cout << "- Stub files: " << stub_count << "\n";
	    std::cout << "- Files missing functions: " << func_gap_files << "\n";
	    if (total_src_doc_lines > 0) {
	        int pct = static_cast<int>(doc_coverage_pct + 0.5f);
	        std::cout << "- Documentation coverage: " << total_tgt_doc_lines << " / "
	                  << total_src_doc_lines << " lines (" << pct << "%)\n";
		    } else {
		        std::cout << "- Documentation coverage: N/A (source has no docs)\n";
		    }
		    std::cout << "\nPrimary focus: ";
		    if (!comp.unmatched_source.empty()) {
		        std::cout << "create missing files (highest deps first)\n";
		    } else if (stub_count > 0) {
		        std::cout << "replace stub files with real implementations\n";
		    } else if (func_gap_files > 0) {
		        std::cout << "port missing functions to reach per-file parity\n";
		    } else if (incomplete > 0) {
		        std::cout << "improve incomplete ports (similarity < 60%)\n";
		    } else if (docs_missing) {
		        std::cout << "port missing documentation\n";
		    } else {
		        std::cout << "no major gaps detected\n";
		    }

			    // Show files with issues
			    std::cout << "\n=== Files with Issues ===\n\n";
		    std::cout << std::setw(30) << std::left << "File"
		              << std::setw(11) << "Similarity"
		              << std::setw(11) << "LineRatio"
		              << std::setw(14) << "FunctionParity"
		              << std::setw(6) << "TODOs"
		              << std::setw(6) << "Lint"
		              << "Status\n";
		    std::cout << std::string(90, '-') << "\n";

	    int shown = 0;
	    for (const auto& m : ranked) {
	        bool func_gap = (m.source_function_count > 0 &&
	                         m.matched_function_count < m.source_function_count);
	        if (m.todo_count == 0 && m.lint_count == 0 && !m.is_stub && !func_gap && m.similarity >= 0.6) {
	            continue;  // Skip files without issues
	        }
	        if (shown++ >= 20) {
	            std::cout << "... and " << (ranked.size() - 20) << " more files\n";
	            break;
        }

	        std::string status;
	        if (m.is_stub) status = "STUB";
	        else if (m.similarity < 0.4) status = "LOW_SIM";
	        else if (func_gap) status = "MISSING_FUNCS";
	        else if (m.lint_count > 0) status = "LINT";
	        else if (m.todo_count > 0) status = "TODO";

	        float ratio = 0.0f;
	        if (m.source_lines > 0) {
	            ratio = static_cast<float>(m.target_lines) / static_cast<float>(m.source_lines);
	        }

	        std::string funcs = "-";
	        if (m.source_function_count > 0) {
	            funcs = std::to_string(m.matched_function_count) + "/" +
	                    std::to_string(m.source_function_count);
	        }

		        std::cout << std::setw(30) << std::left << m.target_qualified.substr(0, 28)
		                  << std::setw(11) << std::fixed << std::setprecision(2) << m.similarity
		                  << std::setw(11) << std::fixed << std::setprecision(2) << ratio
		                  << std::setw(14) << funcs
		                  << std::setw(6) << m.todo_count
		                  << std::setw(6) << m.lint_count
		                  << status << "\n";
	    }

	    // Porting recommendations
	    std::cout << "\n=== Porting Recommendations ===\n\n";

	    std::cout << "Incomplete ports (similarity < 60%): " << incomplete << "\n";
	    std::cout << "Missing files: " << comp.unmatched_source.size() << "\n\n";

	    if (incomplete > 0) {
	        std::cout << "Top priority to complete:\n";
	        int shown_priority = 0;
	        for (const auto& m : ranked) {
	            if (m.similarity < 0.6 && shown_priority++ < 10) {
	                std::string funcs = "-";
	                if (m.source_function_count > 0) {
	                    funcs = std::to_string(m.matched_function_count) + "/" +
	                            std::to_string(m.source_function_count);
	                }
		                std::cout << "  " << std::setw(30) << std::left << m.source_qualified
		                          << " similarity=" << std::fixed << std::setprecision(2) << m.similarity
		                          << " function_parity=" << funcs
		                          << " dependents=" << m.source_dependents;
	                if (m.is_stub) std::cout << " [STUB]";
	                if (m.todo_count > 0) std::cout << " [" << m.todo_count << " TODOs]";
	                std::cout << "\n";
	            }
	        }
	    }

	    // Prepare missing files vector for report generation
	    std::vector<const SourceFile*> missing;
	    if (!comp.unmatched_source.empty()) {
	        std::cout << "\n=== Missing Files (Top by Dependents) ===\n\n";
	        // Sort unmatched by dependents
	        for (const auto& path : comp.unmatched_source) {
	            missing.push_back(&source.files.at(path));
	        }
	        std::sort(missing.begin(), missing.end(),
	            [](const SourceFile* a, const SourceFile* b) {
	                return a->dependent_count > b->dependent_count;
	            });

		        std::cout << std::setw(30) << std::left << "Source File"
		                  << std::setw(11) << "Dependents"
		                  << "Path\n";
		        std::cout << std::string(81, '-') << "\n";

	        int shown_missing = 0;
	        for (const auto* sf : missing) {
	            if (shown_missing++ >= 20) {
	                std::cout << "... and " << (missing.size() - 20) << " more missing files\n";
	                break;
	            }
		            std::cout << std::setw(30) << std::left << sf->qualified_name.substr(0, 28)
		                      << std::setw(11) << sf->dependent_count
		                      << sf->relative_path << "\n";
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

	    if (docs_missing) {
	        std::cout << "There is missing documentation that is hurting overall scoring.\n";
	    }
	    if (total_src_doc_lines > 0) {
	        int pct = static_cast<int>(doc_coverage_pct + 0.5f);
	        std::cout << "Documentation coverage: " << total_tgt_doc_lines << " / "
	                  << total_src_doc_lines << " lines (" << pct << "%)\n";
	    } else {
	        std::cout << "Documentation coverage: N/A (source has no docs)\n";
	    }
	    std::cout << "Files with >20% doc gap: " << doc_gaps.size() << "\n\n";

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

// ============ Agent Guardrails (Swarm Mode) ============

struct AgentState {
    long long last_used_epoch = 0;      // seconds since epoch
    long long last_score_epoch = 0;     // seconds since epoch
    float last_score = -1.0f;           // -1 = unknown
};

static long long now_epoch_seconds() {
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

static float activity_score_from_idle_minutes(long long idle_minutes) {
    if (idle_minutes <= 15) return 100.0f;
    if (idle_minutes >= 60) return 0.0f;
    // Linear decay from 100% at 15m idle to 0% at 60m idle.
    float t = static_cast<float>(idle_minutes - 15) / 45.0f;
    return 100.0f * (1.0f - t);
}

static std::string format_idle_minutes(long long idle_minutes) {
    if (idle_minutes < 0) idle_minutes = 0;
    long long h = idle_minutes / 60;
    long long m = idle_minutes % 60;
    if (h == 0) return std::to_string(m) + "m";
    std::ostringstream ss;
    ss << h << "h" << std::setw(2) << std::setfill('0') << m << "m";
    return ss.str();
}

static std::filesystem::path guardrails_agents_dir(const std::string& task_file) {
    std::filesystem::path base = std::filesystem::path(task_file).parent_path();
    if (base.empty()) base = std::filesystem::current_path();
    return base / ".cache" / "ast_distance" / "agents";
}

static std::filesystem::path agent_lock_path(const std::string& task_file, int agent) {
    auto dir = guardrails_agents_dir(task_file);
    return dir / ("agent_" + std::to_string(agent) + ".lock");
}

static std::filesystem::path agent_notice_path(const std::string& task_file, int agent) {
    auto dir = guardrails_agents_dir(task_file);
    return dir / ("agent_" + std::to_string(agent) + ".notice");
}

static std::filesystem::path agent_state_path(const std::string& task_file, int agent) {
    auto dir = guardrails_agents_dir(task_file);
    return dir / ("agent_" + std::to_string(agent) + ".state");
}

static AgentState load_agent_state(const std::filesystem::path& path) {
    AgentState st;
    std::ifstream f(path);
    if (!f.is_open()) return st;
    std::string line;
    while (std::getline(f, line)) {
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string k = line.substr(0, eq);
        std::string v = line.substr(eq + 1);
        try {
            if (k == "last_used_epoch") st.last_used_epoch = std::stoll(v);
            else if (k == "last_score_epoch") st.last_score_epoch = std::stoll(v);
            else if (k == "last_score") st.last_score = std::stof(v);
        } catch (...) {
            // Ignore malformed fields.
        }
    }
    return st;
}

static void save_agent_state(const std::filesystem::path& path, const AgentState& st) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream f(path);
    if (!f.is_open()) return;
    f << "last_used_epoch=" << st.last_used_epoch << "\n";
    f << "last_score_epoch=" << st.last_score_epoch << "\n";
    f << "last_score=" << st.last_score << "\n";
}

static void touch_agent_last_used(const std::string& task_file, int agent) {
    auto p = agent_state_path(task_file, agent);
    AgentState st = load_agent_state(p);
    st.last_used_epoch = now_epoch_seconds();
    save_agent_state(p, st);
}

static int read_pid_from_lockfile(const std::filesystem::path& path) {
    std::ifstream f(path);
    if (!f.is_open()) return -1;
    int pid = -1;
    f >> pid;
    return pid;
}

class AgentLock {
public:
    AgentLock(const std::string& task_file, int agent, bool override_wait)
        : fd_(-1), locked_(false), holder_pid_(-1) {
        lock_path_ = agent_lock_path(task_file, agent);
        std::filesystem::create_directories(lock_path_.parent_path());
        fd_ = open(lock_path_.string().c_str(), O_CREAT | O_RDWR, 0666);
        if (fd_ < 0) return;
        int flags = LOCK_EX;
        if (!override_wait) flags |= LOCK_NB;
        if (flock(fd_, flags) == 0) {
            locked_ = true;
            std::string pid_str = std::to_string(getpid()) + "\n";
            ftruncate(fd_, 0);
            lseek(fd_, 0, SEEK_SET);
            ::write(fd_, pid_str.c_str(), pid_str.size());
        } else {
            holder_pid_ = read_pid_from_lockfile(lock_path_);
        }
    }

    ~AgentLock() {
        if (fd_ >= 0) {
            if (locked_) flock(fd_, LOCK_UN);
            close(fd_);
        }
    }

    bool locked() const { return locked_; }
    int holder_pid() const { return holder_pid_; }
    const std::filesystem::path& path() const { return lock_path_; }

private:
    int fd_;
    bool locked_;
    int holder_pid_;
    std::filesystem::path lock_path_;
};

static void write_agent_notice(const std::string& task_file, int agent, const std::string& msg) {
    auto p = agent_notice_path(task_file, agent);
    std::filesystem::create_directories(p.parent_path());
    std::ofstream f(p, std::ios::app);
    if (!f.is_open()) return;
    f << msg << "\n";
}

static void print_and_clear_agent_notice(const std::string& task_file, int agent) {
    auto p = agent_notice_path(task_file, agent);
    std::ifstream f(p);
    if (!f.is_open()) return;
    std::stringstream buf;
    buf << f.rdbuf();
    std::string s = buf.str();
    if (!s.empty()) {
        std::cerr << "\n=== NOTICE (Agent #" << agent << ") ===\n\n";
        std::cerr << s << "\n";
    }
    std::error_code ec;
    std::filesystem::remove(p, ec);
}

static void print_agent_activity_section(const TaskManager& tm, const std::string& task_file, int current_agent) {
    std::cout << "\n=== Agent Activity ===\n\n";
    std::cout << "You are agent #" << current_agent << "\n\n";

    struct Row {
        int agent = 0;
        std::string task;
        std::string since;
        long long idle_min = 0;
        float score = 0.0f;
        float trend = 0.0f;
        bool has_trend = false;
    };
    std::vector<Row> rows;

    long long now = now_epoch_seconds();
    for (const auto& t : tm.tasks) {
        if (t.status != TaskStatus::ASSIGNED) continue;
        int agent = 0;
        try {
            agent = std::stoi(t.assigned_to);
        } catch (...) {
            continue; // Guardrails require numeric agent IDs.
        }
        AgentState st = load_agent_state(agent_state_path(task_file, agent));
        long long idle_s = (st.last_used_epoch > 0) ? (now - st.last_used_epoch) : (60LL * 60LL);
        long long idle_min = std::max(0LL, (idle_s + 30) / 60);
        float score = activity_score_from_idle_minutes(idle_min);

        float trend = 0.0f;
        bool has_trend = false;
        if (st.last_score >= 0.0f) {
            trend = score - st.last_score;
            has_trend = true;
        }

        // Update last reported score for trending.
        st.last_score = score;
        st.last_score_epoch = now;
        save_agent_state(agent_state_path(task_file, agent), st);

        Row r;
        r.agent = agent;
        r.task = t.source_qualified;
        r.since = t.assigned_at;
        r.idle_min = idle_min;
        r.score = score;
        r.trend = trend;
        r.has_trend = has_trend;
        rows.push_back(r);
    }

    if (rows.empty()) {
        std::cout << "No agents currently assigned.\n";
        return;
    }

    std::sort(rows.begin(), rows.end(),
              [](const Row& a, const Row& b) { return a.agent < b.agent; });

    std::cout << std::setw(8) << std::left << "Agent"
              << std::setw(34) << "Working On"
              << std::setw(10) << "Idle"
              << std::setw(10) << "Active"
              << "Trend\n";
    std::cout << std::string(76, '-') << "\n";
    for (const auto& r : rows) {
        std::ostringstream trend;
        if (r.has_trend) {
            int d = static_cast<int>(std::round(r.trend));
            if (d > 0) trend << "+" << d;
            else trend << d;
        } else {
            trend << "-";
        }

	        std::ostringstream score;
	        score << static_cast<int>(std::round(r.score)) << "%";

	        std::string agent_label = "#" + std::to_string(r.agent);
	        std::cout << std::setw(8) << std::left << agent_label
	                  << std::setw(34) << std::left << r.task.substr(0, 32)
	                  << std::setw(10) << format_idle_minutes(r.idle_min)
	                  << std::setw(10) << score.str()
	                  << trend.str()
	                  << "\n";
	    }
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
    target.extract_porting_data();  // TODOs, lint, line counts (task inclusion needs this)

    CodebaseComparator comp(source, target);
    comp.find_matches();
    std::cout << "Computing AST similarities...\n";
    comp.compute_similarities();

    // Build task list from missing files
    TaskManager tm(task_file);
    tm.source_root = src_dir;
    tm.target_root = tgt_dir;
    tm.source_lang = src_lang;
    tm.target_lang = tgt_lang;
    tm.agents_md_path = agents_md;

    // Add missing files and unacceptable ports as tasks (sorted by dependents).
    // NOTE: TODOs/stubs are an automatic failure mode for "complete" status,
    // so they must stay in the queue even if similarity looks high.
    std::set<std::string> pending_keys;

    for (const auto& path : comp.unmatched_source) {
        pending_keys.insert(path);
    }

    for (const auto& m : comp.matches) {
        bool func_gap = (m.source_function_count > 0 &&
                         m.matched_function_count < m.source_function_count);
        if (m.similarity < 0.85f || m.todo_count > 0 || m.lint_count > 0 || m.is_stub || func_gap) {
            pending_keys.insert(m.source_path);
        }
    }

    std::vector<const SourceFile*> pending_files;
    pending_files.reserve(pending_keys.size());
    for (const auto& key : pending_keys) {
        pending_files.push_back(&source.files.at(key));
    }

    std::sort(pending_files.begin(), pending_files.end(),
              [](const SourceFile* a, const SourceFile* b) {
                  return a->dependent_count > b->dependent_count;
              });

    for (const auto* sf : pending_files) {
        PortTask task;
        task.source_path = sf->relative_path;
        task.source_qualified = sf->qualified_name;

        // Generate expected Kotlin path
        std::string kt_path = sf->relative_path;
        // Convert .rs to .kt and adjust path
        if (kt_path.size() > 3 && kt_path.substr(kt_path.size() - 3) == ".rs") {
            // Convert filename from snake_case to PascalCase for Kotlin
            std::string stem = kt_path.substr(0, kt_path.size() - 3);
            size_t last_slash = stem.rfind('/');
            if (last_slash != std::string::npos) {
                std::string dir = stem.substr(0, last_slash + 1);
                std::string filename = stem.substr(last_slash + 1);
                kt_path = dir + SourceFile::to_pascal_case(filename) + ".kt";
            } else {
                kt_path = SourceFile::to_pascal_case(stem) + ".kt";
            }
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

void cmd_tasks(const std::string& task_file, int agent) {
    TaskManager tm(task_file);
    if (!tm.load()) {
        std::cerr << "Error: Could not load task file: " << task_file << "\n";
        return;
    }

    int pending, assigned, completed, blocked;
    tm.get_stats(pending, assigned, completed, blocked);

    print_agent_activity_section(tm, task_file, agent);

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

void cmd_assign(const std::string& task_file, int requested_agent, bool override_mode) {
    TaskManager tm(task_file);
    if (!tm.load()) {
        std::cerr << "Error: Could not load task file: " << task_file << "\n";
        return;
    }

    int agent = requested_agent;
    if (agent <= 0) {
        // Allocate a new agent number (monotonic) to avoid collisions/reuse confusion.
        int max_agent = 0;
        for (const auto& t : tm.tasks) {
            if (t.status != TaskStatus::ASSIGNED) continue;
            try {
                max_agent = std::max(max_agent, std::stoi(t.assigned_to));
            } catch (...) {
                // Ignore non-numeric IDs.
            }
        }
        auto dir = guardrails_agents_dir(task_file);
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
            if (ec) break;
            auto name = entry.path().filename().string();
            if (!name.starts_with("agent_")) continue;
            auto pos = name.find_first_of('.');
            auto num_str = name.substr(std::string("agent_").size(),
                                       pos == std::string::npos ? std::string::npos : pos - std::string("agent_").size());
            try {
                max_agent = std::max(max_agent, std::stoi(num_str));
            } catch (...) {
            }
        }
        agent = max_agent + 1;
    }

    if (agent <= 0) {
        std::cerr << "Error: Could not allocate agent number\n";
        return;
    }

    AgentLock agent_lock(task_file, agent, override_mode);
    if (!agent_lock.locked()) {
        int holder = agent_lock.holder_pid();
        std::ostringstream msg;
        msg << "Agent #" << agent << " appears to be in use";
        if (holder > 0) msg << " by pid " << holder;
        msg << ".\n"
            << "Re-run with --override to wait, or pick a different agent number.\n";
        std::cerr << "Error: " << msg.str();
        write_agent_notice(task_file, agent,
                           "Conflict: another PID attempted to use this agent number.\n" + msg.str());
        return;
    }

    print_and_clear_agent_notice(task_file, agent);
    touch_agent_last_used(task_file, agent);

    std::string agent_id = std::to_string(agent);

    // Check if agent already has an assigned task
    for (const auto& t : tm.tasks) {
        if (t.status == TaskStatus::ASSIGNED && t.assigned_to == agent_id) {
            std::cerr << "Agent #" << agent << " already has an assigned task: "
                      << t.source_qualified << "\n";
            std::cerr << "Complete it with: ast_distance --agent " << agent
                      << " --complete " << task_file
                      << " " << t.source_qualified << "\n";
            std::cerr << "Or release it with: ast_distance --agent " << agent
                      << " --release " << task_file
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

    print_agent_activity_section(tm, task_file, agent);

    // Print full assignment details
    tm.print_assignment(*task, agent);
}

void cmd_complete(const std::string& task_file, const std::string& source_qualified,
                  int agent, bool override_mode) {
    TaskManager tm(task_file);
    if (!tm.load()) {
        std::cerr << "Error: Could not load task file: " << task_file << "\n";
        return;
    }

    const PortTask* task_ptr = nullptr;
    for (const auto& t : tm.tasks) {
        if (t.source_qualified == source_qualified) {
            task_ptr = &t;
            break;
        }
    }
    if (!task_ptr) {
        std::cerr << "Task not found: " << source_qualified << "\n";
        return;
    }

    if (agent > 0 && task_ptr->status == TaskStatus::ASSIGNED &&
        task_ptr->assigned_to != std::to_string(agent) && !override_mode) {
        std::cerr << "Error: Agent #" << agent << " cannot complete a task assigned to agent "
                  << task_ptr->assigned_to << "\n";
        std::cerr << "Use --override only if you are explicitly taking over the task.\n";
        return;
    }

    std::filesystem::path source_path = std::filesystem::path(tm.source_root) / task_ptr->source_path;
    std::filesystem::path target_path = std::filesystem::path(tm.target_root) / task_ptr->target_path;

    if (!std::filesystem::exists(target_path) && !override_mode) {
        std::cerr << "Error: Cannot complete task - target file does not exist: " << target_path.string() << "\n";
        std::cerr << "Create the file and add a port-lint header first.\n";
        return;
    }

    if (std::filesystem::exists(target_path)) {
        FileStats stats = PortingAnalyzer::analyze_file(target_path.string());
        if (!stats.todos.empty() && !override_mode) {
            std::cerr << "Error: Cannot complete task - target file contains TODO markers\n";
            std::cerr << "TODOs are an automatic failure mode for porting completeness.\n";
            std::cerr << "Complete the TODOs or remove the file.\n";
            return;
        }
        if (stats.is_stub && !override_mode) {
            std::cerr << "Error: Cannot complete task - target file is detected as a stub\n";
            std::cerr << "Complete the real implementation before marking complete.\n";
            return;
        }

        // Require no stub bodies and high similarity.
        Language src_lang = parse_language(tm.source_lang);
        Language tgt_lang = parse_language(tm.target_lang);

        ASTParser parser;
        bool has_stubs = parser.has_stub_bodies_in_files({target_path.string()}, tgt_lang);
        if (has_stubs && !override_mode) {
            std::cerr << "Error: Cannot complete task - target file contains stub/TODO markers in function bodies\n";
            std::cerr << "The code is fake. Complete the real implementation before marking complete.\n";
            return;
        }

        auto src_tree = parser.parse_file(source_path.string(), src_lang);
        auto tgt_tree = parser.parse_file(target_path.string(), tgt_lang);
        if (!src_tree || !tgt_tree) {
            std::cerr << "Error: Cannot parse files for comparison\n";
            std::cerr << "Fix syntax errors or use --override.\n";
            return;
        }

        // Normalize ASTs: Flatten namespaces/packages to reduce structural noise.
        // Node type 82 is PACKAGE (includes C++ namespaces).
        //
        // For very small "module marker" files (e.g. Rust `mod foo;` roots),
        // flattening PACKAGE nodes removes the key structural signal needed for
        // the module-marker similarity heuristic, so only apply it to larger files.
        if (std::max(src_tree->size(), tgt_tree->size()) > 250) {
            src_tree->flatten_node_type(82);
            tgt_tree->flatten_node_type(82);
        }

        auto src_ids = parser.extract_identifiers_from_file(source_path.string(), src_lang);
        auto tgt_ids = parser.extract_identifiers_from_file(target_path.string(), tgt_lang);
        float file_sim = ASTSimilarity::combined_similarity_with_content(
            src_tree.get(), tgt_tree.get(), src_ids, tgt_ids);

        auto src_functions = parser.extract_function_infos_from_file(
            source_path.string(), src_lang);
        auto tgt_functions = parser.extract_function_infos_from_file(
            target_path.string(), tgt_lang);
        float fn_cov = CodebaseComparator::function_name_coverage(src_functions, tgt_functions).ratio;

        float similarity = file_sim * fn_cov;
        if (similarity < 0.85f && !override_mode) {
            std::cerr << "Error: Cannot complete task with low similarity: " << similarity << "\n";
            std::cerr << "Target exists but identifier content does not match source.\n";
            std::cerr << "Complete the port (aim for >= 0.85) or use --override.\n";
            return;
        }
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
    // Also include incomplete files
    for (const auto& m : comp.matches) {
        if (m.similarity < 0.85) {
            missing.push_back(&source.files.at(m.source_path));
        }
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
            // Convert filename from snake_case to PascalCase for Kotlin
            std::string stem = kt_path.substr(0, kt_path.size() - 3);
            size_t last_slash = stem.rfind('/');
            if (last_slash != std::string::npos) {
                std::string dir = stem.substr(0, last_slash + 1);
                std::string filename = stem.substr(last_slash + 1);
                kt_path = dir + SourceFile::to_pascal_case(filename) + ".kt";
            } else {
                kt_path = SourceFile::to_pascal_case(stem) + ".kt";
            }
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

void cmd_release(const std::string& task_file, const std::string& source_qualified,
                 int agent, bool override_mode) {
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

    if (agent > 0 && it->assigned_to != std::to_string(agent) && !override_mode) {
        std::cerr << "Error: Agent #" << agent << " cannot release a task assigned to agent "
                  << it->assigned_to << "\n";
        std::cerr << "Use --override only if you are explicitly taking over the task.\n";
        return;
    }
    
    // Check if target file exists - if so, require completion or deletion
    std::filesystem::path target_path = std::filesystem::path(tm.target_root) / it->target_path;
    if (std::filesystem::exists(target_path)) {
        std::filesystem::path source_path = std::filesystem::path(tm.source_root) / it->source_path;
        
        // Try to compute similarity - if we can't even parse, that's a HARD FAIL
        std::cerr << "Error: Cannot release task - target file already exists: " << target_path.string() << "\n";
        std::cerr << "Checking similarity...\n";

        FileStats stats = PortingAnalyzer::analyze_file(target_path.string());
        if (!stats.todos.empty() && !override_mode) {
            std::cerr << "Error: Cannot release task - target file contains TODO markers\n";
            std::cerr << "TODOs are an automatic failure mode for porting completeness.\n";
            std::cerr << "Complete the TODOs or delete the file to release.\n";
            return;
        }
        if (stats.is_stub && !override_mode) {
            std::cerr << "Error: Cannot release task - target file is detected as a stub\n";
            std::cerr << "Complete the real implementation or delete the file to release.\n";
            return;
        }
        
        // Parse and compare
        Language src_lang = parse_language(tm.source_lang);
        Language tgt_lang = parse_language(tm.target_lang);
        
        ASTParser parser;
        bool has_stubs = false;
        float similarity = 0.0f;

        // Require no stub bodies (e.g., Kotlin TODO(), Rust unimplemented!(), Python pass/...)
        has_stubs = parser.has_stub_bodies_in_files({target_path.string()}, tgt_lang);
        if (has_stubs) {
            std::cerr << "Error: Cannot release task - target file contains stub/TODO markers in function bodies\n";
            std::cerr << "The code is fake. Complete the real implementation or delete the file.\n";
            return;
        }

        auto src_tree = parser.parse_file(source_path.string(), src_lang);
        auto tgt_tree = parser.parse_file(target_path.string(), tgt_lang);

        if (!src_tree || !tgt_tree) {
            std::cerr << "Error: Cannot parse files for comparison\n";
            std::cerr << "This usually means the target file has syntax errors.\n";
            std::cerr << "Fix the errors or delete the file to release.\n";
            return;
        }

        // Normalize ASTs: Flatten namespaces/packages to reduce structural noise
        // Node type 82 is PACKAGE (includes C++ namespaces)
        src_tree->flatten_node_type(82);
        tgt_tree->flatten_node_type(82);

        auto src_ids = parser.extract_identifiers_from_file(source_path.string(), src_lang);
        auto tgt_ids = parser.extract_identifiers_from_file(target_path.string(), tgt_lang);
        float file_sim = ASTSimilarity::combined_similarity_with_content(
            src_tree.get(), tgt_tree.get(), src_ids, tgt_ids);

        // Parity penalty: missing functions should reduce score.
        auto src_functions = parser.extract_function_infos_from_file(
            source_path.string(), src_lang);
        auto tgt_functions = parser.extract_function_infos_from_file(
            target_path.string(), tgt_lang);
        float fn_cov = CodebaseComparator::function_name_coverage(src_functions, tgt_functions).ratio;

        similarity = file_sim * fn_cov;

        // Require >= 0.50 content-aware similarity to release.
        // (threshold lowered because content-aware scoring is much stricter)
        if (similarity < 0.50f) {
            std::cerr << "Error: Cannot release task with low similarity: " << similarity << "\n";
            std::cerr << "Target file exists but identifier content doesn't match source\n";
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

static bool guardrails_detect_shell_pipeline_peer_process() {
    // If ast_distance is part of a shell pipeline (cmd | other),
    // there will be a sibling process in our process group that's neither
    // our ancestor nor our child.
    //
    // We intentionally allow non-terminal stdout when the caller captures output
    // directly (e.g. a wrapper process reading from a pipe), because that is not
    // the same failure mode as shell filtering commands like `sed`/`grep`.
    const int self = static_cast<int>(getpid());
    const int pgid = static_cast<int>(getpgrp());

    FILE* fp = popen("ps -A -o pid= -o ppid= -o pgid= -o comm=", "r");
    if (!fp) {
        // Conservative fallback: if we cannot inspect the process table,
        // assume the pipe is a shell pipeline and refuse to run.
        return true;
    }

    struct Row {
        int pid = 0;
        int ppid = 0;
        int pgid = 0;
    };

    std::vector<Row> rows;
    rows.reserve(256);
    std::unordered_map<int, int> ppid_by_pid;

    char buf[4096];
    while (fgets(buf, sizeof(buf), fp)) {
        std::string line(buf);
        std::stringstream ss(line);
        Row r;
        if (!(ss >> r.pid >> r.ppid >> r.pgid)) {
            continue;
        }
        rows.push_back(r);
        ppid_by_pid[r.pid] = r.ppid;
    }
    pclose(fp);

    // Build ancestor chain for this process.
    std::unordered_set<int> ancestors;
    int cur = self;
    for (int i = 0; i < 64; ++i) {
        auto it = ppid_by_pid.find(cur);
        if (it == ppid_by_pid.end()) break;
        int parent = it->second;
        if (parent <= 0) break;
        if (!ancestors.insert(parent).second) break;
        cur = parent;
    }

    // Detect a peer process in the same process group.
    for (const auto& r : rows) {
        if (r.pgid != pgid) continue;
        if (r.pid == self) continue;
        if (ancestors.count(r.pid)) continue;
        if (r.ppid == self) continue;  // our own child
        return true;
    }

    return false;
}

int main(int argc, char* argv[]) {
    // Refuse to run when stdout or stderr is piped to another program via a shell pipeline.
    // This blocks `ast_distance ... | sed/grep/...` which has caused model-driven wrappers
    // to silently filter or truncate dashboards.
    // Skip this check in CI environments where stdout is captured by the runner.
    if (!getenv("CI")) {
        bool out_fifo = false;
        bool err_fifo = false;
        struct stat st;
        if (fstat(STDOUT_FILENO, &st) == 0 && S_ISFIFO(st.st_mode)) {
            out_fifo = true;
        }
        if (fstat(STDERR_FILENO, &st) == 0 && S_ISFIFO(st.st_mode)) {
            err_fifo = true;
        }

        if ((out_fifo || err_fifo) && guardrails_detect_shell_pipeline_peer_process()) {
            const char msg[] = "Error: stdout is piped to another program.\n"
                               "ast_distance does not support piping (|).\n"
                               "Run it directly in a terminal.\n";
            write(STDERR_FILENO, msg, sizeof(msg) - 1);
            return 2;
        }
    }

    int agent = 0;
    bool override_mode = false;
    std::string task_file_flag;

    std::vector<std::string> rest;
    rest.reserve(static_cast<size_t>(argc));
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--agent" && i + 1 < argc) {
            agent = std::stoi(argv[++i]);
        } else if (arg == "--task-file" && i + 1 < argc) {
            task_file_flag = argv[++i];
        } else if (arg == "--override") {
            override_mode = true;
        } else {
            rest.push_back(arg);
        }
    }

    std::vector<char*> argv_rebased;
    argv_rebased.reserve(rest.size() + 1);
    argv_rebased.push_back(argv[0]);
    for (auto& s : rest) {
        argv_rebased.push_back(const_cast<char*>(s.c_str()));
    }
    argc = static_cast<int>(argv_rebased.size());
    argv = argv_rebased.data();

    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string mode = argv[1];

    // Guardrails: if a task system exists, require --agent and lock the session number.
    GuardrailsContext guard;
    guard.agent = agent;
    guard.override_mode = override_mode;

    if (!task_file_flag.empty()) {
        guard.task_file = task_file_flag;
    } else if ((mode == "--tasks" || mode == "--assign" || mode == "--complete" ||
                mode == "--release") &&
               argc >= 3) {
        guard.task_file = argv[2];
    } else if (mode == "--init-tasks" && argc >= 7) {
        guard.task_file = argv[6];
    } else if (std::filesystem::exists("tasks.json")) {
        guard.task_file = "tasks.json";
    }

    if (!guard.task_file.empty()) {
        TaskManager tm(guard.task_file);
        guard.active = tm.load();
    }

    if (guard.active && mode != "--assign" && agent <= 0) {
        std::cerr << "Error: task system detected (" << guard.task_file << ").\n";
        std::cerr << "All commands require an agent session number.\n";
        std::cerr << "Get one with: ast_distance --assign " << guard.task_file << "\n";
        std::cerr << "Then re-run with: ast_distance --agent <number> ...\n";
        return 2;
    }

    std::unique_ptr<AgentLock> agent_lock;
    if (guard.active && mode != "--assign" && agent > 0) {
        agent_lock = std::make_unique<AgentLock>(guard.task_file, agent, override_mode);
        if (!agent_lock->locked()) {
            int holder = agent_lock->holder_pid();
            std::ostringstream msg;
            msg << "Agent #" << agent << " appears to be in use";
            if (holder > 0) msg << " by pid " << holder;
            msg << ".\n"
                << "Re-run with --override to wait, or pick a different agent number.\n";
            std::cerr << "Error: " << msg.str();
            write_agent_notice(guard.task_file, agent,
                               "Conflict: another PID attempted to use this agent number.\n" +
                                   msg.str());
            return 3;
        }
        print_and_clear_agent_notice(guard.task_file, agent);
        touch_agent_last_used(guard.task_file, agent);
    }

    g_guardrails = guard;

    try {
        if (mode == "--scan" && argc >= 4) {
            cmd_scan(argv[2], argv[3]);

        } else if (mode == "--deps" && argc >= 4) {
            cmd_deps(argv[2], argv[3]);

        } else if (mode == "--rank" && argc >= 6) {
            cmd_rank(argv[2], argv[3], argv[4], argv[5]);

        } else if (mode == "--deep" && argc >= 6) {
            cmd_deep(argv[2], argv[3], argv[4], argv[5]);

        } else if (mode == "--numpy-mlx" && argc >= 4) {
            cmd_numpy_mlx(argv[2], argv[3]);

        } else if (mode == "--emberlint" && argc >= 3) {
            cmd_emberlint(argv[2]);

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

        } else if (mode == "--symbols" && argc >= 4) {
            SymbolAnalysisOptions options;
            cmd_symbols(argv[2], argv[3], options);

        } else if (mode == "--symbols-duplicates" && argc >= 4) {
            SymbolAnalysisOptions options;
            options.duplicates = true;
            cmd_symbols(argv[2], argv[3], options);

        } else if (mode == "--symbols-stubs" && argc >= 4) {
            SymbolAnalysisOptions options;
            options.stubs = true;
            cmd_symbols(argv[2], argv[3], options);

        } else if (mode == "--symbols-symbol" && argc >= 5) {
            SymbolAnalysisOptions options;
            options.symbol = argv[4];
            if (argc >= 6 && std::string(argv[5]) == "--json") {
                options.json = true;
            }
            cmd_symbol_lookup(argv[2], argv[3], options);

        } else if (mode == "--symbol-parity" && argc >= 4) {
            SymbolParityOptions options;
            for (int i = 4; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg == "--json") {
                    options.json = true;
                } else if (arg == "--verbose" || arg == "-v") {
                    options.verbose = true;
                } else if (arg == "--missing-only" || arg == "--missing") {
                    options.missing_only = true;
                } else if (arg == "--include-stubs") {
                    options.include_stubs = true;
                } else if (arg == "--kind" && i + 1 < argc) {
                    options.filter_kind = argv[++i];
                } else if (arg == "--file" && i + 1 < argc) {
                    options.filter_file = argv[++i];
                }
            }
            cmd_symbol_parity(argv[2], argv[3], options);

        } else if (mode == "--import-map" && argc >= 3) {
            ImportMapOptions options;
            for (int i = 3; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg == "--json") {
                    options.json = true;
                } else if (arg == "--summary") {
                    options.summary_only = true;
                } else if (arg == "--file" && i + 1 < argc) {
                    options.filter_file = argv[++i];
                } else if (arg == "--min" && i + 1 < argc) {
                    options.min_unresolved = std::stoi(argv[++i]);
                }
            }
            cmd_import_map(argv[2], options);

        } else if (mode == "--compiler-fixup" && argc >= 4) {
            CompilerFixupOptions options;
            for (int i = 4; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg == "--json") {
                    options.json = true;
                } else if (arg == "--verbose" || arg == "-v") {
                    options.verbose = true;
                } else if (arg == "--file" && i + 1 < argc) {
                    options.filter_file = argv[++i];
                } else if (arg == "--min" && i + 1 < argc) {
                    options.min_errors = std::stoi(argv[++i]);
                }
            }
            cmd_compiler_fixup(argv[2], argv[3], options);

        // Swarm task management commands
        } else if (mode == "--init-tasks" && argc >= 7) {
            std::string agents_md = (argc >= 8) ? argv[7] : "";
            cmd_init_tasks(argv[2], argv[3], argv[4], argv[5], argv[6], agents_md);

        } else if (mode == "--tasks" && argc >= 3) {
            cmd_tasks(argv[2], agent);

        } else if (mode == "--assign" && argc >= 3) {
            int requested_agent = 0;
            if (argc >= 4) {
                requested_agent = std::stoi(argv[3]);
            } else if (agent > 0) {
                requested_agent = agent;
            }
            cmd_assign(argv[2], requested_agent, override_mode);

        } else if (mode == "--complete" && argc >= 4) {
            cmd_complete(argv[2], argv[3], agent, override_mode);

        } else if (mode == "--release" && argc >= 4) {
            cmd_release(argv[2], argv[3], agent, override_mode);

        } else if (mode == "--dump" && argc >= 4) {
            ASTParser parser;
            std::string filepath = argv[2];
            std::string lang_str = argv[3];
            Language lang;
            if (lang_str == "rust") lang = Language::RUST;
            else if (lang_str == "cpp") lang = Language::CPP;
            else if (lang_str == "python") lang = Language::PYTHON;
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
            auto funcs1 = parser.extract_function_infos(buffer1.str(), lang1);

            std::cout << "Found " << funcs1.size() << " " << language_name(lang1) << " functions\n";
            for (const auto& info : funcs1) {
                std::cout << "  - " << info.name << " (" << info.body_tree->size() << " nodes)";
                if (info.has_stub_markers) {
                    std::cout << " [TODO]";
                }
                std::cout << "\n";
            }

            std::cout << "\nExtracting functions from " << file2 << " (" << language_name(lang2) << ")...\n";
            std::ifstream stream2(file2);
            std::stringstream buffer2;
            buffer2 << stream2.rdbuf();
            auto funcs2 = parser.extract_function_infos(buffer2.str(), lang2);

            std::cout << "Found " << funcs2.size() << " " << language_name(lang2) << " functions\n";
            for (const auto& info : funcs2) {
                std::cout << "  - " << info.name << " (" << info.body_tree->size() << " nodes)";
                if (info.has_stub_markers) {
                    std::cout << " [TODO]";
                }
                std::cout << "\n";
            }

            // Compare function bodies (all pairs, then aggregate with matching)
            std::cout << "\n=== Function Similarity Matrix ===\n\n";
            std::cout << std::setw(20) << "";
            for (const auto& func2 : funcs2) {
                std::cout << std::setw(12) << func2.name.substr(0, 10);
            }
            std::cout << "\n";

            for (const auto& func1 : funcs1) {
                std::cout << std::setw(20) << func1.name.substr(0, 18);
                for (const auto& func2 : funcs2) {
                    float sim = 0.0f;
                    if (!func1.has_stub_markers && !func2.has_stub_markers) {
                        sim = ASTSimilarity::combined_similarity_with_content(
                            func1.body_tree.get(), func2.body_tree.get(),
                            func1.identifiers, func2.identifiers);
                    }
                    std::cout << std::setw(12) << std::fixed << std::setprecision(3) << sim;
                }
                std::cout << "\n";
            }

            auto function_summary = CodebaseComparator::compare_function_sets(funcs1, funcs2);
            if (function_summary.score >= 0.0f) {
                std::cout << "\nCompared " << function_summary.source_total << " source functions vs "
                          << function_summary.target_total << " target functions\n";
                std::cout << "Matched pairs: " << function_summary.matched_pairs
                          << " | Unmatched source: " << function_summary.unmatched_source
                          << " | Unmatched target: " << function_summary.unmatched_target << "\n";
                std::cout << "Function-wise overall score: "
                          << std::fixed << std::setprecision(3)
                          << function_summary.score << "\n";
                if (function_summary.has_source_stub || function_summary.has_target_stub) {
                    std::cout << "TODO/stub markers found in function bodies. Stubmed pairs contribute 0.\n";
                }
            }

        } else if (mode[0] != '-' && argc >= 5) {
            // Default: compare two files with explicit languages
            ASTParser parser;
            std::string file1 = argv[1];
            Language lang1 = parse_language(argv[2]);
            std::string file2 = argv[3];
            Language lang2 = parse_language(argv[4]);

            auto file_contains_macro_rules = [](const std::string& path) -> bool {
                std::ifstream in(path);
                if (!in) return false;
                std::string content(
                    (std::istreambuf_iterator<char>(in)),
                    std::istreambuf_iterator<char>());
                return content.find("macro_rules!") != std::string::npos;
            };

            bool macro_friendly = false;
            if (lang1 == Language::RUST) macro_friendly |= file_contains_macro_rules(file1);
            if (lang2 == Language::RUST) macro_friendly |= file_contains_macro_rules(file2);

            std::cout << "Parsing " << language_name(lang1) << " file: " << file1 << "\n";
            TreePtr tree1 = parser.parse_file(file1, lang1);

            std::cout << "Parsing " << language_name(lang2) << " file: " << file2 << "\n";
            TreePtr tree2 = parser.parse_file(file2, lang2);

            std::cout << "\n";
            auto report = ASTSimilarity::compare(tree1.get(), tree2.get(), macro_friendly);
            report.print();

            auto ids1 = parser.extract_identifiers_from_file(file1, lang1);
            auto ids2 = parser.extract_identifiers_from_file(file2, lang2);
            float content_score = ASTSimilarity::combined_similarity_with_content(
                tree1.get(), tree2.get(), ids1, ids2);

            std::cout << "\n=== Identifier Content Analysis ===\n";
            std::cout << "Identifiers:          "
                      << ids1.total_identifiers << " vs " << ids2.total_identifiers << "\n";
            std::cout << "Unique (raw):         "
                      << ids1.identifier_freq.size() << " vs " << ids2.identifier_freq.size() << "\n";
            std::cout << "Unique (canonical):   "
                      << ids1.canonical_freq.size() << " vs " << ids2.canonical_freq.size() << "\n";
            std::cout << "Raw cosine:           " << std::fixed << std::setprecision(4)
                      << ids1.identifier_cosine_similarity(ids2) << "\n";
            std::cout << "Canonical cosine:     " << std::fixed << std::setprecision(4)
                      << ids1.canonical_cosine_similarity(ids2) << "\n";
            std::cout << "Canonical jaccard:    " << std::fixed << std::setprecision(4)
                      << ids1.canonical_jaccard_similarity(ids2) << "\n";

            bool function_scored = false;
            CodebaseComparator::FunctionComparisonResult function_result;
            try {
                auto src_funcs = parser.extract_function_infos_from_file(file1, lang1);
                auto tgt_funcs = parser.extract_function_infos_from_file(file2, lang2);
                function_result = CodebaseComparator::compare_function_sets(src_funcs, tgt_funcs);
                function_scored = (function_result.score >= 0.0f);
            } catch (...) {
                function_scored = false;
            }

            bool file1_stubs = parser.has_stub_bodies_in_files({file1}, lang1);
            bool file2_stubs = parser.has_stub_bodies_in_files({file2}, lang2);

            if (function_scored) {
                file1_stubs = function_result.has_source_stub;
                file2_stubs = function_result.has_target_stub;

                // Blend function-level similarity with file-level structural metrics.
                // The function-level score (per-body identifier matching) is reliable
                // for matching but penalizes cross-language syntax differences.
                // File-level histogram cosine is the most stable cross-language metric.
                float fn_score = function_result.score;
                float file_hist = report.cosine_sim;  // histogram cosine at file level
                int max_funcs = std::max(function_result.source_total, function_result.target_total);
                float fn_coverage = (max_funcs > 0)
                    ? static_cast<float>(function_result.matched_pairs) / max_funcs
                    : 0.0f;

                // Final score:
                //   60% function-level similarity average (identifier-weighted)
                //   20% file-level histogram cosine (structural shape)
                //   20% function coverage ratio (are the same functions present?)
                content_score = 0.60f * fn_score +
                                0.20f * file_hist +
                                0.20f * fn_coverage;
            }

            if (function_scored) {
                std::cout << "Scored by function bodies: "
                          << function_result.source_total << " vs "
                          << function_result.target_total << " functions, matched "
                          << function_result.matched_pairs << "\n";
                if (function_result.has_source_stub || function_result.has_target_stub) {
                    std::cout << "Function-level TODO/stub markers found; affected matches scored as 0.\n";
                }
            }

            if (file1_stubs || file2_stubs) {
                content_score = 0.0f;
                std::cout << "\n*** STUB DETECTED ***\n";
                if (file1_stubs)
                    std::cout << "  " << file1 << " has TODO/stub/placeholder in function bodies\n";
                if (file2_stubs)
                    std::cout << "  " << file2 << " has TODO/stub/placeholder in function bodies\n";
                
                std::cout << "  Content-Aware Score forced to 0.0000\n";
            } else {
                std::cout << "\nContent-Aware Score:  " << std::fixed << std::setprecision(4)
                          << content_score << "\n";
            }

            std::cout << "\n=== " << language_name(lang1) << " AST Histogram ===\n";
            print_histogram(report.hist1);

            std::cout << "\n=== " << language_name(lang2) << " AST Histogram ===\n";
            print_histogram(report.hist2);

            // Show unmapped node types for diagnostics (only if any exist)
            auto& unmapped = parser.get_unmapped_node_types();
            if (!unmapped.empty()) {
                int total_unmapped = 0;
                for (const auto& [_, count] : unmapped) total_unmapped += count;
                std::cout << "\n=== Unmapped Node Types (" << unmapped.size()
                          << " types, " << total_unmapped << " nodes) ===\n";
                std::vector<std::pair<std::string, int>> sorted_unmapped(unmapped.begin(), unmapped.end());
                std::sort(sorted_unmapped.begin(), sorted_unmapped.end(),
                    [](const auto& a, const auto& b) { return a.second > b.second; });
                for (const auto& [type, count] : sorted_unmapped) {
                    std::cout << "  " << std::setw(40) << std::left << type
                              << ": " << count << "\n";
                }
            }

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
