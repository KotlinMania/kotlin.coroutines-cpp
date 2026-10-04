# ASTDistance repair and integration review

Date: 2026-10-04

The local tool is now built from the sources in `tools/ast_distance`, with
vendored tree-sitter runtime/grammars and licenses. The CMake target replaces the
previous imported prebuilt executable and refreshes the ignored
`tools/ast_distance/ast_distance` executable after a successful build. Builds do
not fetch grammars. Project warning gates remain enabled.

Source provenance: `/Volumes/games/ASTDistance`, commits `0f97023`, `11d0785`,
`200ee14`, `230b05f`, `f1b82d8`, `0b1204a`, `564c9e4`, `05dc06c` and `7932358`. Unrelated in-flight Python/Rust aliases and the
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
passed, including sharing and suspension. All eight new extraction, captured CLI,
normalized logic, deep-inventory, emitted-buffer and identity tests passed under the project's warning
settings. Tests check deliberately missing files/symbols/bodies and valid inline
companion implementations, rather than merely reproducing parser internals.

The real `SharingStarted.kt` inventory has ten implemented Kotlin functions,
including both factory overloads, and the C++ source has nineteen definitions.
Seven source functions have strict matches; the duration factory and the
`equals`/`hashCode` names remain real API/naming gaps. Extra continuation frame
functions remain visible. Manual suspend lowering and C++ ownership syntax are
not certified equivalent by an aggregate AST score.

See `tools/ast_distance/QUALITY_OF_LIFE.md`, `DEEP_INVENTORY.md` and
`TRANSLITERATION_DISTANCE.md` for algorithms and limitations.

## AST emission, documentation and identity follow-up

The implemented native `transliteration_engine` parses Kotlin, emits a C++-shaped
buffer with deterministic AST-scoped rules, reparses it as C++, and scores that
buffer against the target. It reports separate target-token/AST similarity,
ordered algorithm logic, function-name coverage, executable rule coverage,
fallback penalties and documentation correspondence. Complete differing logic
sequences, top-level byte span maps and explicit fallbacks remain reviewable.
Invented/omitted operations, literals, branches, calls and parameter evidence
lower scores even when vocabulary overlaps. Unsupported syntax is preserved in
visible wrappers; it is not silently translated by an invented algorithm.

KDoc parameter and symbol references lower to C++ identifiers/Doxygen syntax.
Primitive references use the same type map as code emission. Narrative is
preserved. Kotlin examples without a deterministic translation rule retain their
text and produce separate documentation diagnostics. Required provenance metadata
is excluded from narrative correspondence. Comments cannot raise implementation
scores, and raw documentation line amount is labeled separately from correspondence.

File pairing now requires exact declared namespace/package components, retaining
case, underscores and component boundaries. Provenance must identify the source
relative path or an exact physical path/component-boundary suffix. Source line
ranges and inline comment closers normalize as metadata. Contradictory markers,
companion identities, unrelated namespace declarations and ambiguous filename
candidates remain unmatched with identity evidence. Empty namespaces cannot claim
unrelated implementations appearing later in a file. Header-only implementations
retain their companion identity.

`--deep` includes an emitted-distance table and persists
`deep_transliteration_evidence.txt`, including combined physical-file order,
callable locations, missing/extra callables, parser/rule diagnostics, full differing
logic sequences, maps and emitted buffers beside `deep_symbol_inventory.txt`.

Final full Release validation passed **31/31 project tests** (23 coroutine/IR plus
8 ASTDistance regressions), and the canonical checkout passed **9/9** including
its separate inherited audit test. Both project executable paths were refreshed
from the source build and their SHA-256 hashes agree. No coroutine production
source was changed by this tool follow-up; a source search found no built-in C++
coroutine keywords/runtime in `src/kotlinx/coroutines` or the emitter.

The real SharingStarted function report has a provisional required body score of
0.320 and 7/10 strict callable matches. The emitted-buffer report shows zero
executable rule coverage because class/suspend/annotation rules remain unsupported,
with seven executable fallbacks and one documentation example diagnostic. This is
a rule-pack limitation, not proof that all sharing algorithms are missing. The
full emitted report makes those limitations explicit rather than certifying
continuation/DSL/IR lowering from an aggregate histogram. Other language-pair
emitter packs also remain unsupported; existing Rust/Kotlin analysis is retained.
