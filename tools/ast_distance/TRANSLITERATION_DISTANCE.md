# Transliteration Distance

ASTDistance should compare ports by first making the source look as much like the
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

## Scoring

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
