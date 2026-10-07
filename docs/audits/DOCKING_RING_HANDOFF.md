# Docking-ring handoff — 2026-10-07

## Current priority supersedes this compiler resume point

Sydney now requires source translation first: finish the faithful kotlinx.coroutines
C++ source before resuming docking-ring/compiler/MLX integration. Use
[SOURCE_TRANSLITERATION_HANDOFF.md](SOURCE_TRANSLITERATION_HANDOFF.md) as the
current resume record. The compiler stopping point and completed results below
remain preserved for the later integration phase.

This is the current resume record for the Ren-profile `kotlinmania` board.
Implementation resumed after the board review. The compiler-object class binding
is now integrated and verified without Kotlin runtime dependencies. This record
names the next actual source consumer; no other worker is assigned.

## Project purpose and first priority

Bring Kotlin's coroutine authoring and faithfully translated operations to normal
C++, usable without Kotlin installed. Connect C++ and Kotlin/Native directly
through the actual Native coroutine state-machine contracts, targeting unsafe
MLX C++ and real GPU operations. Existing C++ functions, classes, ownership and
destruction remain C++. See [the docking-ring design](../architecture/docking_ring.md).

For the later compiler integration phase, first priority remains actual IR declarations, concrete parameter descriptors,
one-time symbol/owner binding and suspension scopes. The named dependency chains
and stopping conditions are in [the dependency ledger](IR_IDENTITY_DEPENDENCIES.md)
and [the implementation plan](../architecture/ir_identity_and_scopes.md).

## Completed records: preserve and reuse

Four bounded verification records were closed after checking current source and
execution evidence. All four are linked
as completed prerequisites of `t_16bf1579`, so their results are available in its
downstream handoff context:

| Card | Completed evidence | Limit that remains open |
|---|---|---|
| `t_18f3c424` — Verify translated ClassKind names and predicates | Six source enum values/eight operations; 48 identical Native/C++ observations; five relevant identities unchanged; bodies already integrated into compiler targets | Actual IR class construction and required measured criteria |
| `t_d52f115d` — Verify consumed Long-to-text formatting for compiler objects | 393 C++ sanitizer digit/error observations, bases 2–36, signed limits and invalid-radix rules; 20 relevant identities unchanged | Direct Kotlin execution comparison, remaining source APIs and required measured criteria |
| `t_8c63c73f` — Verify Native class-name lookup and GC rooting | Real Native 2.4.10 build after splitting Native transport from common getter bodies; 72 agreeing observations on 12 actual classes/three getters before and after GC | Required measured criteria and shared-frame/MLX acceptance; compiler-object binding is verified separately below |
| `t_fb87d1ed` — Verify compiler-owned class binding without Kotlin runtime | Actual Any/KClass instances execute across two translation units/generic specializations; diagnostics removed; production compiler library integrated; Native-OFF build/dependencies verified; five main/three standalone tests have zero failures | Constant-constructor lowering, real IR construction/scopes, required measured criteria and complete MLX demonstrations |

These are completed verification slices. They do not certify whole source files,
the complete port, a shared Native coroutine frame or an MLX GPU demonstration.
The formatter reference is `std::to_chars` plus source error rules; it is not a
Native comparison. Native class-name tests use actual Native metadata and roots.

Earlier compiler prerequisite records already remain done:

- `t_c3f18fbf`: compiler Name behavior.
- `t_44eb8956`: declaration marker hierarchy and IR parameter kinds.
- `t_eae7fa04`: typed descriptor visitor result transport.
- `t_6956e97e`: absent-source singletons.
- `t_d18445de`: visibility singletons.
- `t_a61c3cf1`: Native ArrayIterator behavior.
- `t_a25cfd84`: Native array algorithms and direct C++ array handoffs.
- `t_66ba98ea`: Native IntArray algorithms and direct C++ handoffs.

Existing completed coroutine repairs retain their exact historical evidence and
scope. Their old results do not certify today's dirty source or completion of
the new compiler path. Reuse completed pieces. Reopen one only for changed source,
contrary execution evidence or a specifically demonstrated remaining source gap.

## Resume here: actual collection and IR consumers

Actual translated compiler objects need class identity/name/subtype information
without linking Kotlin/Native runtime entries into standalone compiler builds.
The bounded Clang binding is recorded in the dependency ledger. It emits private
C++ class metadata from real translated declarations; this data is distinct from
Native `TypeInfo` and is not Native object/frame layout. Application classes need
no Kotlin base, storage or annotations.

The actual Any/KClass consumer now builds and executes canonical identity across
two translation units and generic specializations, names, subtype, hash and text.
Temporary plugin diagnostics are removed. KotlinxCompilerObjects supplies the
same translated bodies to the production compiler targets and the object test.
Five main focused tests and three Native-OFF focused tests execute with zero
failures. Thirty-six selected target build records and ten binary/shared-library
dependency lists exclude Kotlin compiler/runtime dependencies. The ordinary C++
example executes 42/43/82. The separately linked actual Native name regression
retains 72 observations. Receipts: `build/ir-recovery/ir-class-binding-integration/`.
This supersedes the earlier crashing/unexecuted consumer and cleanup checkpoints.
The `KCLASS_IMPL` constant-constructor intrinsic remains unfinished separately.

Next source action: return this real Any implementation to actual collection
ancestors and the Native ArrayList dependency of TransformIfNeeded
(transform.kt:126-137). Translate the genuine constructor/backing/set and consumed
ancestor behavior, with actual equality/hash/text dispatch; do not substitute
standard-library behavior or fabricate a partial collection. Then execute the
real copy-on-change consumer and return to concrete descriptors, one-time
symbol/owner binding and Native scopes/buildStateMachine. IR objects' missing
Any ancestry must be integrated through their actual source hierarchy, not imposed
on ordinary application classes. Record each expansion in the dependency ledger
before writing it; preserve completed binding and Native rooted transport checks.
Do not expand into general reflection or GC work.

## Latest collection-to-array source checkpoint

The consumed shared Collections.kt:514-545 copy loops, Native Arrays.kt:85,87,89
forwarding/allocation bodies and Native emptyArray entry are translated and
compiled into KotlinxCompilerObjects. Native and shared functions remain in
separate source-matched files. This supplies the internal Any? instantiation,
not a complete generic array ABI or a fabricated backing list. Actual ancestor/
ArrayList execution remains unfinished.

Ten allocation/empty-array observations agree between sanitizer-instrumented C++
and Native 2.4.10 using the unchanged pinned allocation body (actual modifier
removed and name changed only for the test). This does not execute either
collection copy loop. Both production and Native-OFF compiler builds exit 0;
five main and three standalone tests record zero failures, and ordinary C++
retains 42/43/82. Six relevant files retain 29 checked pinned provenance ranges
and no prohibited labels. Proof: build/ir-recovery/ir-collection-conversion/,
with final source split records in source-split/.

Next resume action remains the genuine AbstractCollection/AbstractMutableCollection/
AbstractMutableList and Native ArrayList ancestry, including actual element
equality/hash/text. Return these available array conversion bodies to those
ancestors, execute the actual TransformIfNeeded copy-on-change consumer, then
return to real parameter descriptors, symbol binding and suspension scopes.
No new card is Done for this unfinished consumer.

## Remaining main deliverables

- `t_16bf1579` — Priority 1: Kotlin IR declaration identity and suspension scopes; finish coroutine lowering. Keep open for real construction, descriptors, binding, scopes and production integration.
- `t_1834dcec` — Repair verified Kotlin algorithm and suspension/IR drift. Keep open for the remaining production algorithms and suspension/lifetime work.
- `t_0dd3c2ad` — Epic: Audit production coroutine IR lowering, continuation ABI and state machines. Keep open for full source/execution/measurement acceptance.

Neither complete MLX GPU demonstration is established: standalone ordinary C++
authoring with retained resources, and direct Native/C++ shared-state-machine
handoffs in both directions with failure, cancellation and cleanup.

The main cards remain blocked as dispatch reservations, with their existing
owners/priorities preserved. They do not await a technical decision. Ren's
`t_f8a8ff12` — Port test JobTest (lifecycle, cancellation, state transitions) — was
observed running in its separate workspace; this reconciliation does not change
or dispatch that work.

## Current oracle and evidence

Both complete-root `tools/ast_distance/ast_distance --deep` commands were rerun
after collection-to-array source/integration changes. Each exited 0. Inventories cover compiler 672
and library 354 source files, 587 paired C++ units and 763 physical C++ files.
These are inventory counts, not completion estimates.

ClassKind, TypeInfoNames and StringNumberConversions still retain provisional
score/normalized-logic/span zeros, unsupported emission and generated parse
errors; target parse errors are absent in those groups. Getter extraction remains
unsupported for the former two. StringNumberConversions still reports 0/17
functions. KClassImpl reports 4/11 functions and 1/2 types, body similarity 0.10.
No required criterion is waived by closing a verification record.

The new shared conversion group has 2/54 matched functions; Native Arrays now
has 6/10. ArrayIntrinsics reports 1/9 in the body table while the deep symbol
inventory explicitly records emptyArray as PRESENT. All three groups retain
provisional normalized-logic/span zeros, unsupported emission and generated
parse errors; target parse errors are absent. Missing source APIs remain real
gaps. The external entry's inventory presence is not a body-parity result.

Current generated outputs are in `project-wide/compiler/` and
`project-wide/library/`. Exact scan commands/results, rerun results, binary/source
hashes, closure provenance, board snapshots and mutations are in
`build/ir-recovery/kanban-handoff-2026-10-07/`. Current checkout is
`solace/sharing-transliteration`, HEAD
`727c198a1fa4229360f74660660e91852b2c028c`, with extensive uncommitted/untracked
work; HEAD alone does not identify its source. See
`source-and-evidence-identities.json` and `closure-verification.json` there.

Before shortening each main card body, its complete previous body was copied to
a durable board attachment named `pre-handoff-body-2026-10-07.md`. Attachments
6/7/8 belong to `t_16bf1579`/`t_1834dcec`/`t_0dd3c2ad`, respectively. Verify
their stored bytes against `history-attachment-receipts.json`. Historical
comments, events, run records, owners, priorities and titles remain preserved.
