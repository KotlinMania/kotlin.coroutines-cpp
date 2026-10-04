# Callable and implementation-review reports

Build from this source checkout with CMake and the vendored tree-sitter grammars:

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
```

`--compare-functions source.kt kotlin target.cpp cpp` extracts implemented C++
function definitions once, resolves qualified declarators, constructors,
destructors, operators and conversion operators, and excludes declarations and
pure virtual clauses. Parameter spellings and source lines distinguish overloads
in the inventory. Matching retains the existing strict case/underscore
canonicalization and similarity metrics. When owner context matches, repeated names are constrained to that owner.
Otherwise they use the existing similarity-ranked one-to-one matcher with an
ambiguity warning for multiple candidates. Owner/signature context is not
semantic proof. Helper/frame functions remain visible as unmatched targets.

The vendored Kotlin grammar predates `fun interface`. Function extraction blanks
that single modifier in parser input outside comments, strings, character
literals and backticks. This is a byte-offset-preserving compatibility adapter:
original source supplies text, parameters and line locations. The CLI announces
adapted locations. Remaining Kotlin or C++ parse errors generate a warning that
the inventory may be incomplete. In particular, suspend macros and an explicit
C++ template instantiation can trigger grammar diagnostics. Name coverage and
body similarity are provisional measurements, not behavioral certification.

All symbol reports now print the complete inventory by default:

```sh
./ast_distance --symbols upstream/common/src src/kotlinx/coroutines
./ast_distance --symbols-stubs upstream/common/src src/kotlinx/coroutines --json
./ast_distance --symbols-duplicates upstream/common/src src/kotlinx/coroutines --json
./ast_distance --symbols-symbol upstream/common/src src/kotlinx/coroutines StateMarker --json
```

The legacy `--symbols-stubs` spelling is retained. Its findings are **review
candidates**, not verified missing implementations. Tree-sitter distinguishes
populated enums/short structs, abstract interfaces, inherited markers,
destructor-only bases, empty types, declaration-only translation units,
implementation elsewhere, and parser errors. No length threshold decides whether
code implements behavior. A `.cpp` with only includes or empty namespaces needs
companion-header/platform/upstream review. An empty marker or destructor-only
base needs upstream role review. Neither is automatically a defect, and the
absence of a finding does not certify the file's algorithms. Explicit template
instantiation evidence is retained even when this grammar reports a missing
identifier. Directory exclusions apply relative to the requested root, so roots
inside build/test locations are not silently skipped.

JSON reports have `schema_version`, `classification`, `findings`, and
`duplicates`. Each finding includes file, line, name, category, and reason. Arrays
are complete and deterministic. JSON strings are escaped. Findings respect the
requested report mode. Plain-text findings include source lines and reasons.

Normal stdout/stderr redirection, pipelines, subprocess capture and CTest are
supported. Process-tree filtering and the redirect guard were removed by the
user's explicit request. Existing `redirect_guard` configuration keys are ignored;
other configuration and substantive scoring checks are unchanged. The captured
CLI regression verifies redirected comparison output, complete JSON and human
inventories, and failure on a missing input. Native tests cover declarator forms,
overloads, grammar adaptation, classifications and deterministic reporting.

## Normalization and drift evidence

Function bodies use the canonical identifier cosine, normalized node histogram,
and ordered normalized logic. The ordered sequence retains exact operator
identity, literal values, control-flow kinds, parameter types, and all variable
and call spellings. It removes parser wrappers and normalizes snake/camel spelling,
primitive parameter types, Kotlin/C++ parameter name/type order and integer literal
separators/width suffixes. It does not map operation synonyms, ignore collection
calls, hide helper frames, or count comments as logic. Kotlin jump expressions are
distinguished as return/throw/break/continue. Grammar soft-keyword identifiers use
original source spelling instead of the parser node name.

The combined function score is `(0.40 * identifier_cosine + 0.20 * AST_cosine +
0.40 * ordered_logic_similarity) * ordered_logic_similarity`. For bodies with no
identifiers it is `(0.40 * AST_cosine + 0.60 * ordered_logic_similarity) *
ordered_logic_similarity`. Ordered logic uses normalized Levenshtein distance up
to 2,000 tokens; larger sequences retain every token and use adjacent-token
multiset overlap to bound runtime. Multiplication prevents a vocabulary/shape
match from rescuing missing ordered logic. Missing source functions still count
as zero in the aggregate. Documentation similarity is a separate metric.

The CLI exposes the ordered-logic score and complete normalized sequences for
different matched functions, and explicitly identifies exactly equal sequences.
Exact normalized sequence equality is evidence about these selected parser
features, not proof of program equivalence. One changed token in a large function
can leave a high aggregate score; the difference remains visible. Representation
changes such as explicit C++ local types versus Kotlin inferred types, manual
suspend state machines, ownership adapters, unsupported macros and pointer/type
syntax can lower similarity. Scores should be read alongside provenance,
function coverage, parser diagnostics and the displayed logic evidence.

Native and captured CLI tests cover faithful branch/loop examples, changed
comparison/arithmetic operators, constants/strings, calls, parameter type/count,
omitted branches and distinct control-flow keywords. They also exercise fake
implementations stuffed with source vocabulary in comments, strings and unused
bindings. Comments do not alter the body metric, and the stuffed implementations
remain well below the faithful implementations and the existing excellent
threshold. The excellent/good/critical distribution bookkeeping remains intact;
excellent counts are included in both status and summary reports.
