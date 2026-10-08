# Collect and scopedFlow source authoring repair — 2026-10-07

The complete Kotlin terminal/Collect.kt, FlowCoroutine.kt, SafeCollector.common.kt
and flow/Builders.kt were read with their affected C++ headers before editing.
This repairs source structure in the top Flow Channels dependency chain.

`flow/internal/FlowCoroutine.hpp:49` now implements scoped_flow through the actual
unsafe_flow builder imported by Kotlin FlowCoroutine.kt:11,46-49. The existing
flow_scope call and actual collector/block arguments remain unchanged. The
invented FlowImpl wrapper and unused FlowCollectorImpl helper are removed by
deleting internal/FlowImpl.hpp; its two production includes are removed/replaced.
The remaining unsafe_flow anonymous-object implementation corresponds to actual
SafeCollector.common.kt:104-110. No replacement wrapper or fallback was added.

`flow/Collect.hpp:41` replaces the handwritten CollectFrame with an annotated
suspend body calling the actual upstream collect operation. Manual label,
self-retention and invoke_suspend dispatch are removed. The existing ownership
binding retains upstream and supplied collector owners as function arguments;
a supplied collector owner binds the original receiver pointer. A raw collector
remains borrowed. The Clang frontend and mandatory LLVM plugin must construct
the frame and preserve owners through suspension, failure and cancellation.
That lifetime requirement is not yet verified for this new body. The adaptation
still differs from Kotlin's literal tail delegation because C++ supplied shared
owners must remain alive; no tail-frame elimination is claimed.

The compiler check found that the existing annotated channel emission function
had additional owner arguments after its completion parameter. `Channels.hpp:59`
now puts the shared Continuation last as required by the authoring ABI. Forwarding
overloads are updated and the existing four-argument raw/shared completion calls
remain available. The ownership arguments remain the same supplied objects.
The former trailing-continuation diagnostic disappears; the compiler now reaches
frame generation for this entry, which is still unsuccessful.

Verification:

- Fresh strict syntax compilations of the actual channel-flow and collect/reduce
  consumers using existing frontend and LLVM modules both exit 1. Receipts:
  build/ir-recovery/source-collect-final-channel-fixture.log and
  build/ir-recovery/source-collect-final-collect-fixture.log.
- Generated frames still fail to parse. The diagnostics expose unresolved generic
  T, missing exception-context previous initialization, GNU label extensions and
  other dependency warnings under -Wall -Wextra -Wpedantic -Werror. No warning
  suppression or handwritten-frame fallback was introduced. No fresh successful
  executable result is claimed for these changed paths.
- Forty-five ranged source references across Collect.hpp, Channels.hpp and
  FlowCoroutine.hpp resolve to existing Kotlin sources and valid line bounds.
  Receipt: build/ir-recovery/source-collect-provenance.json. This validates source
  bounds, not full transliteration or runtime fidelity.
- Both complete-root deep inventories are refreshed after source edits under
  docs/audits/project-wide/library and compiler.

Final measured library evidence: 827/2918 functions, 359/560 types, average body
similarity 0.26 and documentation similarity 0.38, with 123 scoring failures.
Collect retains 8/8 function matches and body similarity 0.13; FlowCoroutine
retains 3/3 functions and 1/1 type with body similarity 0.19. Flow Channels remains
12/12 functions, 1/1 type and body similarity 0.22. Extra target declarations
decrease, but these measurements still show substantial body gaps.

Existing comments remain visible. Collect and FlowCoroutine still contain Kotlin
notation in KDoc examples; their complete example translation remains unfinished.
Channel diagnostic text, broader flow algorithm correspondence, strict plugin
rebuilding and both MLX acceptance paths also remain incomplete. Historical
execution of the removed handwritten CollectFrame does not validate this body.
