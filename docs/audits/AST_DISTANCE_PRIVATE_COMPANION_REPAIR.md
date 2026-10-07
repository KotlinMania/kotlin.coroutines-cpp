# ast_distance private companion namespace repair

Date: 2026-10-07.

Moving the concrete AbstractFlow frame into Flow.cpp exposed a false report.
Flow.hpp declares public APIs in kotlinx::coroutines::flow; its source companion
contains concrete helpers in kotlinx::coroutines::flow::internal. The tool
rejected the pair as a companion namespace conflict, then reported Flow.kt and
its two types/collect API missing. The isolated beta.core header/internal source
fixture reproduces the conflict before the repair (receipt:
build/ir-recovery/abstract-flow-tool-before.log).

Codebase::extract_imports now retains the parent unit identity for companions
whose namespaces differ only by the exact trailing internal component. It does
not accept unrelated/sibling/case-changed namespaces, arbitrary suffixes, or
contradictory source provenance. An internal-only file without a parent companion
still has its actual internal namespace identity.

Function extraction and the deep symbol inventory now retain actual lexical
namespace evidence from their CST. Matching rejects namesakes under internal as
implementations of APIs in the parent package. Kotlin declaration snippets with
no package carry no package evidence; full-file pairing still performs its
existing namespace/default-package checks. Name normalization, overload matching,
body scoring and source provenance are unchanged.

The CLI fixture verifies both public header retention and the negative case: an
int next_value definition under beta::core::internal does not implement the
beta.core declaration. Function matching records zero strict pairs and the deep
inventory keeps the public API DECLARATION_ONLY. Existing unrelated companion,
empty namespace, foreign declaration, provenance and overload tests remain.
All nine tool CTests ran with zero failures (1.60 seconds).

Fresh library/compiler deep scans accompany this repair. The corrected inventory
records AbstractFlow::collect PRESENT at Flow.hpp:295 and collectSafely
DECLARATION_ONLY at Flow.hpp:270, as required for the abstract source contract.
The lexical checks also reject earlier cross-namespace namesakes; lower aggregate
name counts are measurement corrections and must not be hidden by relaxing
identity. Full function/body correspondence remains substantially incomplete.

Final complete-root library measurement: 772/2918 function names and 343/560
types matched, average function-body similarity 0.26, 122 cheat/scoring failures.
Flow retains 28 dependent groups, 1/1 body names and 2/2 types; its body score is
0.16. These are current measurements, not completion assertions. Both full-root
commands returned zero after the final tool change.
