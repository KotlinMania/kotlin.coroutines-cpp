# Source transliteration handoff — 2026-10-07

Sydney changed the immediate focus: finish faithful Kotlin-to-C++ source
translation first. This supersedes IR identity/scopes as the current first
priority. Docking-ring/compiler/MLX integration resumes after this source phase;
its required behavior and completed evidence are preserved.

## Current assignment

Port the actual kotlinx.coroutines sources under tmp/kotlinx.coroutines to
src/kotlinx/coroutines, including complete functions/classes, parameters/defaults,
algorithms and comments. Read each matching Kotlin and C++ source in full before
editing. Preserve line order and naming rules wherever possible. Public generic
APIs may require templates; keep non-public logic in .cpp when that is possible
without inventing replacement abstractions. Suspend signatures retain the current
Continuation ABI. No stubs, placeholders, cheap aliases or fallback behavior.

A compile failure is evidence of unfinished integration, not justification for
simplifying an upstream algorithm. Source transliteration is the primary work;
build repairs, compiler/stdlib dependency expansion and MLX demos come later.
Translate genuine dependencies that belong to the library source without turning
them into an independent compiler or collection project. Existing normal C++ and
Native compatibility requirements remain constraints.

## Starting evidence and repair order

Use project-wide/library/high_priority_ports.md and NEXT_ACTIONS.md for live
priorities, with deep_symbol_inventory.txt and deep_transliteration_evidence.txt
for function/type gaps and measurement limitations. The latest library scan
covers 354 source files against 587 paired units/763 physical C++ files. These
counts identify the inventory; they are not completion estimates. Both full-root
--deep scans are required after relevant source changes.

The highest current production fanout groups are flow.Channels, flow.Flow and
internal.Concurrent. Start by reading the full Channels.kt and its current
Channels.hpp/.cpp, then reconcile actual function bodies rather than treating
12/12 name matches as completion. The required function similarity is currently
0.23; the corresponding Flow and Concurrent groups are 0.04 and 0.31. Preserve
these findings and verify differences directly against source. Adjust order when
the refreshed oracle or actual source evidence warrants it. Tests remain below
production source in the translation order; Ren owns the separate JobTest card.

## Evidence and completion discipline

Every translated class/function retains exact source provenance and copied
comments. Update API audit/source gap records with actual file:line references.
Remove banned source comments by implementing the missing behavior. Keep real
unresolved dependencies explicit in audit evidence; do not present incomplete
files as complete. Source drafts may remain uncompiled when they faithfully
translate upstream and consistently follow the repo's conventions.

Use existing source-repair card t_1834dcec as priority 100. Preserve the compiler
card t_16bf1579 at priority 99 for later integration and the tracking audit
t_0dd3c2ad at 98. Blocked is a dispatch reservation for this owning Codex chat,
not a technical decision request. Do not dispatch another worker or overwrite
Ren's separate workspace.

## Preserved compiler stopping point

DOCKING_RING_HANDOFF.md retains all twelve completed compiler prerequisites.
The latest consumed Any? collection conversion bodies are compiled, with ten
matching Native/C++ allocation observations, but actual collection copying and
Native ArrayList/ancestry remain unfinished. The later compiler continuation is
real collection ancestry/ArrayList, followed by actual IR parameter descriptors,
symbol-owner binding and suspension scopes. Neither full MLX demonstration is
complete. Do not reopen completed work merely because the larger goal is open.

Receipt for the source-priority transition:
build/ir-recovery/source-first-2026-10-07/.

## First Channels source edit

The full 157-line Kotlin source, 371-line C++ header and 16-line C++ source
were read before edits. markConsumed now uses atomic exchange(true), directly
matching Kotlin getAndSet(true), and is private as in the source. Five complete
KDoc blocks are copied verbatim; source ranges include the actual produceIn
154-157 ending. Redundant constructor casts and an inaccurate translation-unit
linkage-anchor claim are removed. This is partial source reconciliation, not
whole-file completion.

Next source work: reconcile all twelve actual Kotlin functions and ChannelAsFlow
against the C++ bodies. additionalToStringProps currently substitutes a numeric
pointer for actual channel text. The raw collect_to overload discards suspension
results; compare it with actual ChannelFlow/ProducerScope source before changing
that boundary. Classify extra ownership/lowering helpers against real source
contracts rather than treating name inventory matches as complete translation.

A strict syntax-only Clang invocation exits 1 with existing included-header
unused-parameter errors (first CoroutineContext.hpp:78, then ContinuationImpl).
The record is a compilation diagnostic, not a test, and no compilation success
is claimed. Do not divert into warning suppression or compiler repairs. Complete
source behavior against upstream and keep its uncompiled status visible.
Evidence: build/ir-recovery/source-first-2026-10-07/channels-source-start.json
and channels-syntax.log.
