# AST Distance

Cross-language AST similarity measurement and porting analysis tool.

Inspired by the [ASTERIA paper](https://arxiv.org/abs/2108.06082) which uses Tree-LSTM for binary code similarity detection, this tool measures similarity between Rust and Kotlin source files to help verify porting accuracy.

## Features

- **Tree-sitter parsing** for Rust, Kotlin, and C++ ASTs
- **Normalized node types** that map across languages
- **Multiple similarity metrics**:
  - Cosine similarity of node type histograms
  - Structure similarity (size, depth)
  - Jaccard similarity of node sets
  - Normalized tree edit distance
- **Codebase-level analysis**:
  - Dependency graph building
  - File matching by port-lint headers or name similarity
  - Porting priority ranking
  - Documentation gap detection
- **Quality checks**:
  - TODO scanning with context
  - Lint error detection
  - Stub file identification

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Usage

### Compare two files
```bash
./ast_distance <file1> <lang1> <file2> <lang2>
```

### Full codebase analysis
```bash
./ast_distance --deep <src_dir> <src_lang> <tgt_dir> <tgt_lang>
```

### Show missing files
```bash
./ast_distance --missing <src_dir> <src_lang> <tgt_dir> <tgt_lang>
```

### Scan for TODOs
```bash
./ast_distance --todos <directory>
```

### Run lint checks
```bash
./ast_distance --lint <directory>
```

## Port-Lint Headers

Add a header comment to each ported file to enable accurate source tracking:

```kotlin
// port-lint: source core/src/config.rs
package com.example.config

data class Config(...)
```

The header must appear in the first 50 lines. When present, the tool will:
- Match files explicitly instead of by name similarity
- Compare documentation coverage between source and target
- Report "Matched by header" vs "Matched by name" statistics

## Example Output

### File Comparison
```
=== AST Similarity Report ===
Tree 1: size=148, depth=8
Tree 2: size=162, depth=10

Similarity Metrics:
  Cosine (histogram):    0.9836
  Structure:             0.8568
  Combined Score:        0.7737

=== Documentation Comparison ===
Doc comment count: 2 vs 2 (diff: 0)
Doc lines:         4 vs 2 (diff: 2)
Doc text cosine:   100.00%
```

### Deep Analysis
```
=== Porting Quality Summary ===

Matched by header:    103 / 107
Matched by name:      4 / 107
Total TODOs in target: 56
Total lint errors:    356

=== Porting Recommendations ===

Top priority to create:
  core.error                     deps=51
  render.renderable              deps=19
  state.session                  deps=18
```

## Architecture

```
include/
  ast_parser.hpp      - Tree-sitter based parser
  codebase.hpp        - Codebase scanning and comparison
  imports.hpp         - Import/dependency extraction
  porting_utils.hpp   - TODO/lint/header analysis
  similarity.hpp      - Similarity metrics
  tree.hpp            - Tree data structure
  tree_lstm.hpp       - Binary Tree-LSTM encoder

src/
  main.cpp            - CLI tool
```

## References

- [ASTERIA: Deep Learning-based AST-Encoding](https://arxiv.org/abs/2108.06082)
- [Stanford TreeLSTM](https://github.com/stanfordnlp/treelstm)
- [tree-sitter](https://tree-sitter.github.io/tree-sitter/)

## License

MIT
