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
  - File matching by provenance paths and namespace/package identity
  - Porting priority ranking
  - Detailed function/type symbol parity in default reports
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

Default codebase reports are intentionally detailed: terminal output and generated
markdown reports include per-file function parity, type parity, and the complete
missing-symbol names needed for porting work.

See [TRANSLITERATION_DISTANCE.md](TRANSLITERATION_DISTANCE.md) for the implemented bounded Kotlin-to-C++
parser-guided translated-buffer distance model and its explicit unsupported rules.

Optional `.ast_distance_config.json` files define the local port roots and
`reexport_modules` patterns for declarations-only wiring files that should be
reported as consult-only instead of prioritized as direct logic ports. If the
file is missing, the first comparison command writes a stub using the source
and target paths from that comparison.

```json
{
  "type": "port",
  "name": "my-port",
  "source": {
    "path": "tmp/upstream/src",
    "lang": "rust"
  },
  "target": {
    "path": "src/commonMain/kotlin/my/package",
    "lang": "kotlin"
  },
  "checks": {
    "deep": true,
    "missing": true,
    "todos": true,
    "lint": true
  },
  "reexport_modules": [
    "mod.rs",
    "lib.rs"
  ]
}
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

## Callable extraction and implementation review

See [QUALITY_OF_LIFE.md](QUALITY_OF_LIFE.md) for complete symbol reports,
`--json`, callable extraction details, grammar limitations and regression tests.
Symbol "stub" reports identify review candidates rather than asserting missing
behavior from file/body length. Stdout/stderr redirection and automated capture
are supported.

## Inspecting a literal transliteration

```sh
./ast_distance --transliterate source.kt kotlin cpp > emitted.cpp 2> rules.txt
./ast_distance --translit-distance source.kt kotlin target.cpp cpp > distance.txt
./ast_distance --compare-functions source.kt kotlin target.cpp cpp > functions.txt
```

For Kotlin-to-C++ file comparisons, the primary score is literal cosine against
an in-memory C++ buffer emitted from Kotlin AST-scoped replacement rules. Vector
coordinates are `(token position, exact token spelling)`: the score is the number
of exact tokens at the same position divided by `sqrt(emittedTokens * targetTokens)`.
Whitespace and comments are excluded from implementation tokens; identifiers,
operators and complete string/character literals retain exact target spelling.
Reordering calls or declarations lowers this score even with identical word counts.
There is no alignment, identifier folding, weighted AST blend or logic multiplier.
An insertion can shift every subsequent coordinate; this is literal closeness,
not a proof of semantic equivalence. Documentation uses the same ordered cosine
separately after C++ reference/markup replacement and never raises code similarity.

`--translit-distance` reports this score with the complete emitted buffer.
The default Kotlin-to-C++ file comparison shows it as the primary literal report;
other languages and `--compare-functions` retain their legacy structural metrics.
Coverage, unsupported spans, parser errors, symbol matching, AST histograms and
ordered logic remain independent diagnostics. A provisional score does not establish
parity for unsupported generic, extension or suspend lowering rules. Fallback
statements cannot earn credit by matching their own generated placeholder.

The emitted buffer comes from AST-scoped rules. Its report preserves source/emitted
spans and explicit unsupported constructs. Ordered logic retains operations,
constants, calls, branches and parameter evidence so invented or omitted logic
lowers similarity. Documentation references lower separately to C++ names and
markup; documentation never raises the implementation score. Numerical similarity
is accompanied by full differing sequences and parser/rule limitations.

For Kotlin/C++ comparisons, provenance must identify the source path and declared
namespace/package components must agree exactly. Same basenames in another
namespace, contradictory companion metadata and ambiguous source-set ties are
reported instead of silently paired. See [DEEP_INVENTORY.md](DEEP_INVENTORY.md)
for the complete inventory and deep emitted-evidence receipts.
