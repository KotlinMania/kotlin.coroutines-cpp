# ASTDistance false-report repair

Date: 2026-10-07. Source translation remains the active project goal.

The user authorized correcting analyzer bugs that produce false reports. Two
reported library findings were reproduced before repair:

* Kotlin `flow/Channels.kt:98-99` was rejected after adding context parameter
  syntax: the ordinary constructor parameter `context` was treated as a reserved
  keyword. The constructor regression failed in `kotlin_grammar_native` before
  the grammar change. `grammar.js` now retains `context` as a soft identifier and
  preserves both context-list and identifier parses until subsequent tokens
  distinguish them. Byte spans and the original Kotlin text remain unchanged.
* `AbstractFlow::collect` was classified as declaration-only even though
  `flow/Flow.hpp:284` defines its collection frame. The inventory lacked the
  existing coroutine statement-macro parser adaptation, and its owner comparison
  treated `AbstractFlow<T>` as different from `AbstractFlow`. The new deep CLI
  regression reproduced this before repair. `src/deep_inventory.cpp:43,86`
  corrects those two causes. Owner matching alone ignores template arguments;
  body and signature comparisons retain them.

Shared identifier diagnostics in `include/kotlin_grammar_compat.hpp:22`, consumed
by function extraction, emission and inventory, also expose Tree-sitter's
classification of hard keywords as identifiers. This catches the malformed
`context() fun broken() {}` fixture without changing tokens. The diagnostic
describes a CST classification requiring review, not an assertion that Kotlin
source is invalid. Valid declarations can reach it through other grammar
limitations. These remaining classifications are preserved in the inventories.
Broad lexer experiments that disturbed valid infix syntax or parsing performance
were removed. The regression includes the actual `mask and permissions` shape
from `concurrent/src/internal/LockFreeLinkedList.kt:86`.

Abstract/interface signatures deliberately do not contribute function bodies.
They remain in the complete inventory, and generated summary/priority reports
now explicitly explain this counting scope. Pure virtual `Flow::collect` and
`AbstractFlow::collectSafely` remain declaration-only. Name/owner presence does
not establish parameter, overload, body or execution fidelity.

Validation uses the standalone CMake analyzer build in `build/ast-identity`.
All nine native/CLI suites complete with zero failures. The final test receipt
is `build/ir-recovery/ast-false-report-tests.log`; the build receipt is
`ast-false-report-build.log` in the same directory. Both complete roots are
refreshed with the built `tools/ast_distance/ast_distance --deep`: all of
`tmp/kotlinx.coroutines` and all of `tmp/kotlin`, each against the complete `src`
tree. Full CLI receipts are `ast-false-report-library-deep.log` and
`ast-false-report-compiler-deep.log`. Generated evidence remains in the existing
`docs/audits/project-wide/library` and `compiler` directories.

The repaired library inventory records `PRESENT function AbstractFlow::collect`
at `Flow.hpp:284`. Compared with the preceding committed inventory, the complete
library scan removes source parser errors from 24 files and introduces no new
source files with Tree-sitter ERROR/MISSING nodes. Hard-keyword classification
reviews are a separate, explicit remaining grammar limitation.

The dependency repair order remains Channels (65 dependents), Flow (28), then
Concurrent (14). Low body similarity, missing source APIs, unsupported emission,
and scoring failures remain required work. Analyzer repair does not complete the
source translation or either standalone/Native MLX acceptance path.
