# ASTDistance repair and integration review

Date: 2026-10-04

The local tool is now built from the sources in `tools/ast_distance`, with
vendored tree-sitter runtime/grammars and licenses. The CMake target replaces the
previous imported prebuilt executable and refreshes the ignored
`tools/ast_distance/ast_distance` executable after a successful build. Builds do
not fetch grammars. Project warning gates remain enabled.

Source provenance: `/Volumes/games/ASTDistance`, commits `0f97023`, `11d0785`,
`200ee14`, `230b05f` and `f1b82d8`. Unrelated in-flight Python/Rust aliases and the
separate symbol-audit command in that checkout were excluded. Existing local
port-lint compatibility functions were retained.

## Verified repairs

- C++ definitions are extracted once, including qualified methods, constructors,
  destructors and operators. Declarations/pure virtual clauses are not counted
  as implemented bodies.
- Offset-preserving adaptation permits the vendored Kotlin grammar to inspect
  `fun interface` without changing original source lines or text. Remaining
  parse errors and unmapped constructs are visible.
- Strict callable matching uses names, known owners and one-to-one candidates.
  Repeated methods and implemented overloads are counted individually. Ownerless
  lowering remains explicit review evidence.
- Body normalization preserves operators, literals, control flow, calls and
  parameter evidence. Kotlin expression bodies normalize to implicit returns;
  generic type delimiters are distinct from comparison operators. Primitive
  types and common declaration order/inference differences are normalized.
- Native/CLI adversarial cases verify that copied vocabulary in comments,
  strings and unused bindings does not rescue a fake implementation. Comments
  do not alter the body metric. Small operator changes remain visible even when
  a numerical score remains high.
- Full symbol reports and valid JSON replace capped output. Short bodies,
  populated enums and small structs are not labeled missing implementations by
  a length heuristic. Abstract/marker/base roles and grammar failures carry
  review categories rather than proven-stub claims.
- Companion-header definitions are inspected and reported separately from
  unresolved translation-unit findings. The project-wide review has 63
  unresolved review candidates and 38 located companion definitions; these
  numbers are inventories, not a count of verified defects.
- Deep reports include extracted types, API declarations, functions,
  properties/constants, enum entries and aliases, including unmatched files.
  A missing root fails explicitly. Type coverage uses the selected language
  grammars in either direction and recognizes C++ aliases.
- Rust-to-Kotlin combined production/test symbol totals still use the original
  primary-plus-supplementary calculations and are printed. Excellent/good/
  critical distribution bookkeeping is preserved, with excellent counts used.
- Output capture, redirects and pipelines are supported, as explicitly
  authorized by the user. Substantive source/parity checks remain.

## Validation

Fresh Release build of the project passed. All 23 existing coroutine/IR tests
passed, including sharing and suspension. All five new extraction, captured CLI,
normalized logic and deep-inventory tests passed under the project's warning
settings. Tests check deliberately missing files/symbols/bodies and valid inline
companion implementations, rather than merely reproducing parser internals.

The real `SharingStarted.kt` inventory has ten implemented Kotlin functions,
including both factory overloads, and the C++ source has nineteen definitions.
Seven source functions have strict matches; the duration factory and the
`equals`/`hashCode` names remain real API/naming gaps. Extra continuation frame
functions remain visible. Manual suspend lowering and C++ ownership syntax are
not certified equivalent by an aggregate AST score.

See `tools/ast_distance/QUALITY_OF_LIFE.md`, `DEEP_INVENTORY.md` and
`TRANSLITERATION_DISTANCE.md` for algorithms and limitations. The planned
AST-driven target-buffer emission path is separate from the existing common-node
body metric; it must not be described as implemented by node histograms alone.
