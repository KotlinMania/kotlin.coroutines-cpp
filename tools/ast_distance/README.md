# ASTDistance

Cross-language AST similarity measurement tool for verifying source code ports.

Inspired by the [ASTERIA paper](https://arxiv.org/abs/2108.06082) which uses Tree-LSTM for binary code similarity detection, this tool measures similarity between Rust and Kotlin source files to help verify porting accuracy.

## Features

- **Tree-sitter parsing** for Rust and Kotlin ASTs
- **Normalized node types** that map across languages
- **Multiple similarity metrics**:
  - Cosine similarity of node type histograms
  - Structure similarity (size, depth)
  - Jaccard similarity of node sets
  - Normalized tree edit distance
- **Function-level comparison** with similarity matrix
- **Tree-LSTM encoder** (C++ port from Stanford TreeLSTM)

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Usage

### Compare two files
```bash
./ast_distance <rust_file> <kotlin_file>
```

### Compare functions between files
```bash
./ast_distance --compare-functions <rust_file> <kotlin_file>
```

### Dump AST structure
```bash
./ast_distance --dump <file> <rust|kotlin>
```

## Example Output

```
=== AST Similarity Report ===
Tree 1: size=3377, depth=23
Tree 2: size=1102, depth=25

Similarity Metrics:
  Cosine (histogram):    0.9901
  Structure:             0.6232
  Jaccard:               0.3228
  Edit Distance (norm):  0.2615
  Combined Score:        0.5647
```

Function comparison shows which Rust functions map to which Kotlin functions:
- `render_sparkline ↔ renderSparkline`: 0.916 (excellent match)
- `symbol_for_height ↔ symbolForHeight`: 0.815 (good match)

## Architecture

```
include/
  tree.hpp          - Tree data structure
  tensor.hpp        - Lightweight tensor ops
  tree_lstm.hpp     - Binary Tree-LSTM (ported from Stanford)
  node_types.hpp    - Normalized AST node types
  ast_parser.hpp    - Tree-sitter based parser
  similarity.hpp    - Similarity metrics

src/
  main.cpp          - CLI tool

vendor/
  treelstm/         - Stanford TreeLSTM (Lua reference)
```

## References

- [ASTERIA: Deep Learning-based AST-Encoding for Cross-platform Binary Code Similarity Detection](https://arxiv.org/abs/2108.06082)
- [Stanford TreeLSTM](https://github.com/stanfordnlp/treelstm)
- [tree-sitter](https://tree-sitter.github.io/tree-sitter/)

## License

MIT
