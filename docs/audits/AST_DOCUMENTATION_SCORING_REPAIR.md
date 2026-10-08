# Translated documentation scoring repair — 2026-10-07

The native transliteration engine excluded C++ comment nodes from token cosine.
Consequently, missing or changed KDoc could leave the primary literal fidelity
score unchanged. This was a measurement defect: meaningful comments are part of
the source translation contract.

`tools/ast_distance/src/transliteration_engine.cpp` now includes ordered
translated comment words at their source positions alongside executable tokens.
Bounded reference and parameter-tag rules produce C++ names and markup. Only
comment delimiters, markup punctuation and anchored Transliterated from / port-lint
metadata are excluded. Narrative and examples remain visible and scored.
Unsupported Kotlin example translation produces documentation diagnostics;
`tools/ast_distance/src/main.cpp:3072` marks such evidence provisional.

Code-only AST and normalized-logic diagnostics remain separate. The project-wide
average function-body metric is still a body/parameter diagnostic; it has not
been relabeled as a documentation-inclusive score. Documentation correspondence
and amount are reported beside it. Positional literal cosine is sensitive to
insertions shifting subsequent words and does not establish semantic equivalence.

Verification:

- Rebuilt ast_distance and transliteration_engine_test using build/ast-identity:
  exit 0. The native test uses -UNDEBUG -Wall -Wextra -Wpedantic -Werror.
- Native and captured CLI CTest regressions: two tests, zero failures, exit 0.
  Receipt: build/ir-recovery/ast-documentation-inclusive-tests.log.
- Regressions require missing, reordered, extra and changed documentation to
  reduce primary fidelity. CLI fixtures retain identical code with missing or
  changed KDoc and require normalized logic to remain 1. Unsupported inline
  Kotlin example syntax must report a documentation miss and provisional evidence.
- Both full-root deep reports were regenerated with the rebuilt analyzer.
  Scope and generated evidence live under docs/audits/project-wide.

Remaining bounded emitter limitations include generic and suspend declarations,
extension receivers and embedded example algorithms. Their text and diagnostics
are retained. This correction does not claim complete automatic translation of
KDoc examples or the coroutine implementation.
