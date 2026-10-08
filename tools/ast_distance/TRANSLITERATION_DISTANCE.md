# Transliteration Distance

`ast_distance` compares ports by first making the source look as much like the
target language as deterministic parser rules allow, then scoring the translated
buffer against the target buffer and target AST.

This is not a full compiler. It is a source-to-source normalization pass whose
job is to erase language syntax noise while preserving porting evidence.

## Core Algorithm

1. Parse the source file with tree-sitter.
2. Walk the source CST/AST and emit a target-shaped intermediate buffer.
3. Apply ordered rewrite rules to named spans, not raw regex over the whole file.
4. Preserve unmapped source spans with explicit fallback wrappers so missing rule
   coverage is visible in the score.
5. Parse the translated buffer as the target language.
6. Compare translated-source text, translated-source AST, target text, and target
   AST with cosine and structural metrics.
7. Report function/type parity beside the transliteration score.

The result is a deterministic "how close would this source be if transliterated
mechanically" score.

## Rule Packs

Rules should be language-pair specific:

- `rust_to_kotlin`: Rust items, impl blocks, traits, enums, pattern matching,
  ownership sugar, Result/Option idioms, test annotations, and snake_case to
  camelCase names.
- `cpp_to_kotlin`: namespaces, classes, methods, pointers/references, templates,
  includes, and RAII idioms.

Each rule has:

- Source node type and optional parent/field constraints.
- Target emission template.
- Child span mapping.
- Confidence value.
- Fallback behavior.

Example shape:

```text
rust function_item
  name: snake_case -> camelCase
  params: typed_identifier -> name: Type
  return_type: -> : Type
  body: block -> block
```

## Historical scoring proposal

The weighted formula in this section is a historical proposal, not the current
Kotlin-to-C++ CLI scoring contract. The local CLI reports positional exact-token
cosine after in-memory emission as its primary score. Structural metrics,
normalized logic, symbol correspondence, coverage, and unsupported fractions
are separate evidence. See [the current CLI contract](README.md#inspecting-a-literal-transliteration).

The comparison should report separate sub-scores:

- `translated_text_cosine`: token cosine between emitted buffer and target text.
- `translated_ast_cosine`: node/type cosine between parsed emitted buffer and target AST.
- `span_rule_coverage`: percentage of source spans handled by specific rules.
- `fallback_penalty`: penalty for generic or raw-source fallback spans.
- `symbol_parity`: function/type/test parity from the existing detailed report path.

Final score:

```text
score =
    0.35 * translated_text_cosine +
    0.35 * translated_ast_cosine +
    0.20 * symbol_parity +
    0.10 * span_rule_coverage -
    fallback_penalty
```

The weights can be tuned, but fallback use must remain visible. A tool that
silently falls back to raw text would lie.

## Implementation Path

1. Add a `transliteration_engine` module that produces:
   - translated target-shaped buffer
   - span map from source byte ranges to target byte ranges
   - rule hit/miss counts
2. Add `--transliterate <src_file> <src_lang> <target_lang>` to inspect the
   generated buffer.
3. Add `--translit-distance <src_file> <src_lang> <tgt_file> <tgt_lang>` to score
   translated-source against target.
4. Fold transliteration distance into `--deep` reports as another detailed column.
5. Use report gaps to drive rule authoring: highest-frequency fallback node types
   become the next deterministic translation rules.

## Why This Can Become a Translation Tool

Once rule coverage is high enough, the emitted buffer stops being just a scoring
artifact and becomes a draft port. Because rules are parser-driven and ordered,
the output is reproducible. Because fallbacks are reported, the tool knows where
it is not yet a translator.

## Implemented Kotlin → C++ rule pack

The native `transliteration_engine` now implements the source parse → emitted
buffer → target parse pipeline for a bounded Kotlin → C++ subset. This is an
additional metric, distinct from the existing in-memory common-node comparison.
The other language-pair rule packs above remain design work; invoking them fails
explicitly.

Supported emission includes package-to-namespace declarations, typed ordinary
functions, plain final classes with typed ordinary instance methods, default
parameters, explicit return types, expression-body returns,
primitive types, type arguments, basic local bindings, statement `if`/`else`,
`while`, ordinary calls/member access, return/throw/break/continue, basic literals
and arithmetic/comparison/logical/assignment expressions. Name lowering changes
camelCase identifiers to snake_case. `val` emits `const`; `var` remains mutable.
KDoc and ordinary comments retain their narrative and C++ comment delimiters.
AST comment-scoped rules lower `@param`/`@see` identifiers and KDoc symbol links
(`\ref` with C++ qualified/name syntax), including labeled links and companion
static references. Primitive type links share the emitter's Kotlin/C++ type map:
`[Int]`/`[kotlin.Int]` become `\c int`, `[String]` becomes `\c std::string`, and
multi-word C++ types use inline code markup. Labels and ordinary class names stay
intact. Equivalent Doxygen/inline code delimiters do not change correspondence,
but a changed referenced primitive type does. Narrative casing/spelling and
Markdown URLs are preserved.
Fenced and inline examples retain their original text; Kotlin-only example syntax
produces explicit documentation diagnostics rather than invented example code.
The pack does not claim to rewrite every KDoc tag or its embedded algorithms.
Line comments inside bodies retain their newline so they cannot swallow code.

Unmapped nodes emit `__ast_distance_unmapped__(node_type, original_source)` and
record the original line and byte span. Suspend functions, generic function
signatures, extension receivers, nullable/function types, protected/combined modifiers,
named arguments, varargs, constructor/inheritance/field/interface/generic and
file-private class declarations, ranges, lambdas, inference-dependent expression
return types and additional Kotlin idioms currently require rules. They are
visible fallbacks rather than invented algorithms. This engine does not generate
C++ coroutines or claim to implement the coroutine DSL/IR lowering.

Inspect and capture a complete generated buffer and rule receipt:

```sh
./ast_distance --transliterate source.kt kotlin cpp > emitted.cpp 2> emitted.rules.txt
./ast_distance --translit-distance source.kt kotlin target.cpp cpp > distance.txt
```

The span map currently covers complete top-level parsed declarations/comments,
including classes and functions with their bodies, rather than claiming a separate map for
all nested expressions. Offsets refer to original source bytes and assembled
emitted bytes. Methods are matched within their enclosing emitted class span,
retaining owner identity and declaration order; each emitted overload is used
once. A class with any unsupported member remains wholly unsupported, preventing
an unsupported overload from borrowing another method’s emitted body. Rule
hit/miss counts count visited rewrite nodes. Coverage is the
fraction of top-level executable/declaration source bytes fully handled without
a fallback anywhere in that declaration. Comments, package/import metadata are
excluded from the coverage denominator. Imports are metadata only in this pack;
resolving external APIs/includes is outside this emitter.

The report shows token cosine on target-grammar leaves, parsed target-shaped AST
histogram cosine, function-name coverage, ordered normalized logic, coverage,
fallback penalty, parser-error status, full maps/diagnostics, function locations,
missing/extra callable names, differing ordered logic sequences and the emitted
buffer. Overloads match one-to-one using emitted source spans and strongest
compatible callable evidence. Known incompatible callable owners do not match.
The standalone function-name subscore is not a type/test/API proof; `--deep`
provides the complete extracted symbol inventory beside implementation metrics.

An earlier weighted design additionally multiplied its score by ordered
normalized logic. This formula is retained only to explain the historical
proposal; the current CLI does not use it as its primary score:

```text
base = .35 * translated_text_cosine + .35 * translated_ast_cosine
     + .20 * function_name_parity + .10 * span_rule_coverage
score = max(0, base - fallback_penalty) * normalized_logic
```

Generated fallback argument strings are excluded from translated text vocabulary.
The primary literal score includes ordered comment/KDoc words at their actual
positions alongside executable tokens. Missing, reordered or changed narrative,
parameter documentation, references and examples reduce correspondence even when
the executable body is unchanged. Only comment delimiters, markup punctuation,
and anchored `Transliterated from:` / `port-lint:` metadata differ without a
narrative penalty. Source reference/tag syntax uses the bounded C++ lowering
rules above. Comments are translated, not dropped to simplify parsing.

Documentation correspondence is also reported separately to locate failures;
it is not excluded from primary fidelity. Executable AST and normalized-logic
metrics remain explicitly code-only diagnostics so copied prose cannot establish
an implemented algorithm. Separate `documentation_misses` expose unsupported
example translation rules while preserving their text. Parser errors make the
result provisional. None of these metrics establish compiler equivalence or
semantic correctness.

Native and captured CLI regression tests exercise faithful emission, byte/line
maps, expression returns, namespace identity, overloads/defaults, while loops,
comment preservation, missing documentation, tiny operator/literal changes,
vocabulary stuffing and unsupported signature/body fallbacks. The native emitter
test compiles with `-Wall -Wextra -Wpedantic -Werror`.

```sh
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --target ast_distance transliteration_engine_test -j4
ctest --test-dir build -R transliteration_engine --output-on-failure
```
