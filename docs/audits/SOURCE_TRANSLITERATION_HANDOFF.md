# Source transliteration handoff — 2026-10-07

This is the current continuation handoff, updated at Sydney's explicit request
following an interrupted ChannelFlow source-edit turn. Read this document and
inspect the current worktree before continuing. Older versions remain in Git.

## Continuation update after the handoff

**Latest source continuation:** 06da4bbf translates the actual map dependency in
Transform.hpp:124,140,472, replacing unsafe_transform CollectFrame and MapFrame
with typed source bodies and owning result unboxing. The complete Transform.kt
and consumed Emitters.kt body were read. Other Transform manual bodies remain.
The final strict actual consumer and public overload probe exit 1; frontend
local-class/lambda namespace integration, generated frames/templates and
dependency diagnostics remain. The Native-disabled full core build exits 2 in
its plugin dependency. No fresh executable evidence exists. Final full-root
reports are committed in 9c1a935f; both scans exit 0. Transform remains 12/13
bodies and similarity 0.07. See MERGE_SOURCE_REPAIR.md for exact receipts.

**Continued transliteration:** d573b064 directly translates internal collector
cancel/join/acquire/child cleanup bodies. c9f3038b and 2cbadce9 replace public
Merge manual frames and duplicated stack mappers with the source operations and
add suspending flat-map/latest transforms with explicit result-box ownership.
4bc60d81 wires the installed C++ runtime package and gates Native tests explicitly.
IR lowering and CMake integration were reviewed against pinned compiler source;
the standalone OFF configuration succeeds, but strict fresh plugin builds fail
on LLVM/Clang dependency diagnostics. Existing-plugin source consumers also fail
on generated frames/template/lambda-context diagnostics. Read the current top
section of MERGE_SOURCE_REPAIR.md for exact source locations and receipts.
Both full-root deep scans finish with exit 0; reports are in 26378eb2. Internal
Merge similarity is 0.27; public Merge remains incomplete at 8/9 bodies and 0.10.
Continue source translation; neither runtime acceptance path is complete.

**Latest checkpoint:** 53c62d23 preserves the four dirty source files found on
entry; 3194132a completes immutable-capture/comment alignment. All four Merge
consumers now use direct source operations, so collect_channel_flow is deleted
from ChannelFlow.hpp/.cpp and has no remaining src references. Two handwritten
Merge continuation classes are replaced by annotated suspend bodies. Existing
typed callable/scope/context adaptations remain. Read the current top sections
of MERGE_SOURCE_REPAIR.md and CHANNEL_FLOW_SCOPE_AND_SPILLS.md before continuing.

Actual Merge.cpp, actual test_channel_consumption.cpp and explicit instantiations
of all three Merge classes each exit 1 under strict compilation. There is no
fresh executable evidence. The local MergeCollector declaration is rejected by
lowering; an annotated member call inside the scope lambda is also rejected in
the instantiation, alongside generated label/exception-context and dependency
diagnostics. No warnings were suppressed or plugins rebuilt. Receipts use
merge-direct- under build/ir-recovery; the instantiation probe is
tmp/merge-direct-instantiation.cpp. Keep source translation ahead of compiler
work; do not restore manual frames to obtain a successful check.

Both full-root deep scans exit 0 with no simultaneous source edits. Refreshed
library reports are in b9d8d99a: 831/2918 bodies, 359/560 types, similarity 0.26,
123 scoring failures; Merge remains 9/9 bodies and 3/3 types at similarity 0.26;
ChannelFlow remains 18/19 and 6/6 at 0.25. Compiler report is unchanged at
592/7657 bodies and 174/1727 types, similarity 0.36, 24 failures. Continue real
source repair from the current oracle, preserving its missing/provisional/error
findings. The full translation objective is unfinished. The goal API returns
no active app goal in this resumed session; no completion status was set.

The following describes the preceding ChannelFlow checkpoint:

Source commit e8581c99 follows the original handoff checkpoint ca037a93.
The remaining ChannelFlow header collection adapter call is removed. A raw
virtual collect entry forwards to an annotated owning overload that retains the
existing flow owner and suspends scoped collection. Its scope body directly
calls emit_all with produce_impl's owned channel; the owned emit_all projection
forwards to the source emit_all_impl consume=true body. Zero adapter calls remain
in ChannelFlow.hpp; four Merge consumers keep the adapter alive. The original
handoff sequence below is historical where it mentions that header consumer.

Fresh strict actual consumer and concrete class instantiation both exit 1:
channel-flow-direct-scoped-consumer.log and channel-flow-direct-instantiation.log.
No runtime or lifetime success is established. Updated current source audit is
CHANNEL_FLOW_SCOPE_AND_SPILLS.md. Full-root refresh receipts use the prefix
channel-flow-direct-final-. Read current reports for their final outcomes rather
than relying on the earlier counts below.

## Objective and authority

Workspace: `/Volumes/stuff/Projects/kotlinmania/kotlin.coroutines-cpp`.
Branch: `solace/sharing-transliteration`.
Source checkpoint: `ca037a93` (work in progress, described below).
The app goal was verified active while preparing this handoff. It has no token
budget. Do not mark it complete or blocked: meaningful source work remains.
The preceding implementation turn made concrete progress; this handoff turn
preserves the interrupted state rather than completing the implementation.

**Read the active objective file before doing more work:**
`/Users/sydney/.codex/attachments/c6cc8eb3-eae6-4392-8ed6-5beb312b1205/goal-objective.md`.
The older objective file is
`/Users/sydney/.codex/attachments/053db042-23fc-41ff-a092-3d43960bcaca/goal-objective.md`.
The active file's historical HEAD, dirty counts and measurements are stale;
its source-first assignment and product requirements remain binding.

The immediate assignment is faithful Kotlin-to-C++ translation of the whole
kotlinx.coroutines library. Library source translation takes priority over
compiler infrastructure, interoperability development, measurement enhancements
and administrative work. Ground truth is `tmp/kotlinx.coroutines`; use
`tmp/kotlin` only for an actual consumed compiler/stdlib dependency. Preserve
classes, functions, algorithms, signatures, defaults, branch order and meaningful
comments/KDoc. The user wants the two files to look like the same thing in
different languages. Similar-purpose implementations do not satisfy the goal.

Read repository `AGENTS.md` and workspace
`/Volumes/stuff/Projects/kotlinmania/AGENTS.md`. User instructions override
historical skill recipes. The applied skill is
`/Users/sydney/.codex/skills/kotlinmania-porting/SKILL.md`; it keeps source
translation in the main agent loop. No subagents are authorized for this task.

Persistent user constraints:

- Always commit before changing; preserve unrelated work and Ren's JobTest.
  Snapshot unfinished changes honestly. Do not stash, reset, clean, create a
  linked worktree, push or open a PR without the relevant task authorization.
- No stubs, placeholders, invented helpers/state machines, substitute aliases,
  fallback algorithms, or TODO/FIXME/XXX/HACK source comments. Genuine empty or
  identity functions are acceptable only when the matching source has them.
- Replace manual frames/helpers with compiler lowering and Kotlin-shaped source.
  Use existing Continuation ABI during source translation. Do not remove real
  Kotlin continuation classes just because they are continuation classes.
- No warning suppression. Strict checks retain
  `-Wall -Wextra -Wpedantic -Werror`.
- Translate comments and examples as well as code; do not ignore docstrings in
  ast_distance. Add canonical port-lint file provenance and per-function/class
  Transliterated from paths and line ranges.
- Methods/variables snake_case; classes CamelCase; constants uppercase. Public
  interfaces in headers, concrete private implementations in .cpp; required
  generic definitions stay available to every instantiation.
- Preserve C++ ownership. Retaining a borrowed pointer/reference does not adopt
  it. Erased result boxes need actual owning unbox/free adapters.
- No question tool; ask ordinary chat only if ambiguity prevents useful work.
  Give meaningful commentary at least every 60 seconds during ongoing work.
- Do not call git diff a test, use “pass” as a verdict, or claim symbol presence
  proves completed source translation.

Existing Kanban scope: source card `t_8700df29`, umbrella `t_1834dcec`;
compiler card `t_16bf1579`, tracking audit `t_0dd3c2ad`. Existing source card was
updated in the preceding turn. Do not create duplicates or dispatch workers.
The available local command for authorized card updates is:
`hermes kanban --board kotlinmania comment t_8700df29 --author codex 'message'`.

## Current interrupted work: resume here

Only ChannelFlow.hpp was dirty when Sydney interrupted the implementation to
request this handoff. It is now safely committed as **ca037a93**, titled
“Checkpoint direct ChannelFlow suspend calls for handoff”. Its changes are
unfinished and not covered by refreshed audits/deep measurements yet.

Full matching Kotlin `flow/internal/ChannelFlow.kt` and the current C++
ChannelFlow.hpp/.cpp were read before editing. The pending source change removes
three uses of the existing collect_channel_flow callable adapter:

1. `ChannelFlow<T>::get_collect_to_fun` now returns a lambda that directly calls
   `collect_to(scope, std::move(completion))`, matching source :54-56. The lambda
   still captures the actual receiver and an existing shared receiver owner.
2. `ChannelFlowOperator<S,T>::collect_to` is annotated suspend. It retains an
   existing receiver owner and the source SendingCollector, directly executes
   `dsl::suspend(flow_collect(collector.get(), completion.get()))`, and returns
   erased Unit. The old nested callable/adapter call is removed (source :151-152).
3. `collect_with_context_undispatched` is annotated suspend, with a trailing
   owning shared Continuation parameter. It obtains the original-context
   collector, directly suspends the source with_context_undispatched call and
   returns erased Unit (source :144-148). Its caller retains the supplied
   continuation through the existing retain_continuation boundary.

These edits use existing compiler authoring rather than adding another frame.
Local owners are intended to survive through generated-frame storage. This has
NOT been verified at runtime. Inspect actual generated frame lifetime behavior;
source-authored locals alone do not establish retained ownership or cleanup.

The adapter still exists in ChannelFlow.cpp:58 and in its header declaration.
One header consumer remains in ChannelFlow<T>::collect. Four further uses are in
flow/internal/Merge.hpp. Do not delete the adapter until its real consumers are
translated. Search afresh with `rg -n collect_channel_flow src`.

The last investigation was tracing producer-lambda owner retention through
CoroutineStart/intrinsics before claiming that direct tail forwarding preserves
captures. Evidence inspected:

- `intrinsics/Cancellable.cpp:58`: start_coroutine creates the actual continuation
  and starts its intercepted entry.
- `intrinsics/IntrinsicsNative.cpp:16-77`: CreatedContinuation and
  RestrictedCreatedContinuation retain block_ while suspended and clear it in
  release_intercepted. A local copy keeps captures during inline completion.
- `internal/ScopeCoroutine.hpp:99-107,149+`: start_undispatched_or_return invokes
  its supplied block; this needs separate lifetime tracing before replacing the
  remaining scoped lambda adapter. Do not assume arbitrary temporary closure
  ownership is implemented by the frontend.

For annotated lambdas, the existing compiler README and fixtures use
`[] [[suspend]] (...)`, which requires C++23 for front attributes. The library
uses C++20. A scratch Clang syntax probe of trailing GNU
`__attribute__((annotate("suspend")))` did not report an attribute syntax error,
but exited 1 for the unused probe variable; it establishes neither plugin
lowering nor runtime behavior. No production lambda annotation was added in
this checkpoint. Read `src/tests/ir/fixtures/suspend_lambda.cpp`,
`nested_suspend_lambda.cpp` and the frontend README before choosing syntax.

## Current compilation evidence

Fresh strict actual consumer check after ca037a93's source edits:
`build/ir-recovery/channel-flow-direct-source-consumer.log`, exit **1**.
The completed tool session was 51560; no process needs polling/restarting.
No known build, scan or test process remains live from these turns.

Exact command from repository root:

```bash
/opt/homebrew/opt/llvm/bin/clang++ -std=c++20 \
  -Wall -Wextra -Wpedantic -Werror -ferror-limit=0 \
  -I include -I src/kotlinx/coroutines -I src \
  -fpass-plugin=build/ir-recovery/lib/KotlinxCoroutinePass.so \
  -Xclang -load -Xclang build/ir-recovery/lib/KotlinxSuspendPlugin.so \
  -Xclang -add-plugin -Xclang kotlinx-suspend \
  -fsyntax-only src/tests/src/suspend/test_channel_as_flow_smoke.cpp
```

The check reused existing frontend/LLVM modules, not freshly rebuilt plugins.
Diagnostics include unused parameters in JobSupport,
CancellableContinuationImpl, Select, BufferedChannel and Builders; GNU
address-of-label errors in source/manual and generated frames; missing
exception-context `previous` initialization; unresolved generated `T`; and
“generated frame could not be parsed” at AbstractFlow::collect. A frontend
remark is discovery evidence, not a completed lowering test. No fresh
executable/runtime result verifies this checkpoint. Do not rerun old binaries
and attribute their results to the changed source.

The source-first objective allows faithful uncompiled drafts. Keep compiler
limitations explicit; do not suppress warnings, restore handwritten frames,
introduce fallback behavior or drift into a compiler project to get a green
result. Broader source and lifetime repairs remain possible.

## Last committed and measured ChannelFlow work

Before ca037a93, worktree was clean at 88cb3c0a. Commits:

- **c659aa94**: removed handwritten CollectContinuation from ChannelFlow.cpp,
  including label, macro yield and self-retention cycle. The existing
  collect_channel_flow entry is now annotated suspend, calls
  `dsl::suspend(collect(completion.get()))` and returns nullptr.
- **ad3cf58f**: intermediate full-root deep refresh after frame removal.
- **b330b6ee**: restored source val const fields in ChannelFlow, upstream flow
  in ChannelFlowOperator and retained fields in UndispatchedContextCollector;
  made ChannelFlowOperatorImpl, UndispatchedContextCollector and ChannelAsFlow
  final; restored omitted drop-channel-operators and ATOMIC producer KDoc.
- **1512bda2**: audit/API checkpoint with strict compilation limitations.
- **88fc085c**, **88cb3c0a**: final reports and corrected reference count.

Receipts:
`channel-flow-source-authoring-syntax.log` (actual ChannelFlow.cpp, exit 1),
`channel-flow-authoring-final-consumer.log` (actual consumer, exit 1),
`channel-flow-authoring-final-{library,compiler}-deep.log` (both exit 0), and
`channel-flow-authoring-final-source-references.json`, all in build/ir-recovery.
Reference check: ChannelFlow.hpp 47, ChannelFlow.cpp 12, Channels.hpp 22 ranges
resolve with valid bounds; no prohibited markers in these files. These are
reference checks, not fidelity or runtime tests.

Audit: `docs/audits/CHANNEL_FLOW_SCOPE_AND_SPILLS.md`, current checkpoint at top;
`docs/audits/API_AUDIT.md` has its current row. Historical runtime evidence in
those documents predates source-authoring migration and cannot certify it.

## Latest measured source state

Reports under `docs/audits/project-wide/{library,compiler}` are authoritative for
**88cb3c0a**, and stale for ca037a93 until refreshed. Library measurements:
831/2918 matched bodies, 359/560 types, average body similarity 0.26,
documentation similarity 0.38, 123 scoring failures. ChannelFlow: 18/19 bodies,
6/6 types, body similarity 0.24; missing ChannelFlowOperator::toString.
The target ChannelFlow body inventory fell from 42 to 38 after manual class
removal. Names matched do not certify source correspondence.

Leading production priorities by fanout remain Flow Channels (65), Flow (28),
internal Concurrent (14), Native Exceptions (6), Native CoroutineContext (6),
CoroutineStart (2). Read current high_priority_ports.md, port_status_report.md,
deep_symbol_inventory.txt and deep_transliteration_evidence.txt. Investigate
false reports against actual source; do not waive or hide genuine findings.

After relevant source changes, run BOTH exact full-root CLI scans from separate
report directories, with no source edits while either scan runs:

```bash
root=/Volumes/stuff/Projects/kotlinmania/kotlin.coroutines-cpp
(cd "$root/docs/audits/project-wide/library" && \
 "$root/tools/ast_distance/ast_distance" --deep \
 "$root/tmp/kotlinx.coroutines" kotlin "$root/src" cpp)
(cd "$root/docs/audits/project-wide/compiler" && \
 "$root/tools/ast_distance/ast_distance" --deep \
 "$root/tmp/kotlin" kotlin "$root/src" cpp)
```

Commit generated evidence before further source changes. Scan completion does
not certify compiled or executed behavior. Preserve raw missing/zero/provisional
criteria and errors. Successful name matching is not whole-function translation.

## Earlier changes to preserve

These are partial source repairs, not completed subsystems. Detailed audits are
the source of exact before-controls, receipts and remaining gaps:

- **FlowBuilders** (fa88e983, 6c79f751, cdb29f5e; final audit bae6bfee): eight
  imported-flow factory paths use internal::unsafe_flow; public flow constructs
  SafeFlow. Source final/val declarations restored. CallbackFlowBuilder's
  handwritten frame removed; source parent suspension then closed-channel check
  uses IllegalStateException and original diagnostic. Builder classes moved to
  source flow namespace with inherited protected hooks. Remote-call example
  moved to the matching suspend overload. Strict consumers/instantiation exit 1.
  Latest measured Builders 15/23 bodies, 4/4 types, similarity .12. Sequence,
  array/range types, vector capture identity, other manual as_flow frames,
  block text and broader examples remain. See FLOW_BUILDERS_SOURCE_REPAIR.md.
- **Collect/scoped flow** (4ba36029, d532ba30): invented FlowImpl/FlowCollectorImpl
  removed, scoped_flow uses actual unsafe_flow, CollectFrame replaced with
  annotated source collection. Channels emit_all_impl owning continuation moved
  to final argument for frontend recognition. Strict checks remain unsuccessful.
  See COLLECT_SOURCE_AUTHORING_REPAIR.md and ABSTRACT_FLOW_SOURCE_REPAIR.md.
- **EventLoop**: source base queue/use-count, thread-local ownership and delay
  conversions; seven selected base KDocs. Bounded syntax/resource evidence
  exists. Native factory, base dispatch, custom BlockingEventLoop and broader
  timers/workers remain incomplete. See EVENT_LOOP_SOURCE_REPAIR.md.
- **DispatchedTask**: checked casts, original failure precedence, source
  unconfined-loop/finally, actual Native recovery, final run and 10/10 KDocs.
  Concrete syntax evidence is bounded. See DISPATCHED_TASK_SOURCE_REPAIR.md.
- **Native context**: inline callable wrappers preserve move-only captures;
  genuine Native identity/empty hooks retain source behavior. See
  NATIVE_CONTEXT_INLINE_SOURCE_REPAIR.md.
- **Exceptions/context**: actual CancellationException namespace imported from
  kotlin::coroutines::cancellation; cause/null message retained; source equality
  and UTF-16 hash; source context namespaces, polymorphic keys, ordering and
  structural equality. Throwable metadata/text and JobCancellationException
  to_string remain required. Do not replace them with what()/RTTI/pointer text.
  See NATIVE_EXCEPTIONS_CONSTRUCTORS.md, NATIVE_JOB_CANCELLATION_EQUALITY.md,
  STDLIB_COROUTINE_NAMESPACES.md and COROUTINE_CONTEXT_POLYMORPHIC_KEYS.md.
- **ast_distance documentation scoring**: primary token_cosine includes comment
  words in source order; documentation is NOT ignored. Separate code-only/body
  and documentation diagnostics remain distinct. Provenance/markup normalization
  is bounded; unsupported examples retain findings. Analyzer builds under
  build/ast-identity; its native/CLI checks succeeded in the recorded repair.
  Do not claim it cannot rebuild because compiler plugins fail strict checks.
  See AST_DOCUMENTATION_SCORING_REPAIR.md and tools/ast_distance/README.md.

Other existing source/audit work includes producer cancellation/finally,
channel receive/select, owned select arguments, timeout, actual Unit/enum text,
CoroutineStart typed forwarding, builders/scopes and continuation resume/unroll.
Read their current files and relevant API audit rows before touching them.
No complete JobSupport/dispatcher/worker/stdlib parity is claimed.

## Designs fully read and product requirements

The user explicitly requested reading these; they were read fully in manageable
chunks before the most recent source work:

- docs/architecture/docking_ring.md (688 lines)
- docs/suspension/IR_SUSPEND_LOWERING_SPEC.md (648 lines)
- docs/architecture/ir_identity_and_scopes.md (835 lines)
- docs/suspension/CLANG_SUSPEND_EXTRACTION.md
- docs/suspension/SUSPEND_IMPLEMENTATION.md and README.md

Their compiler priority is subordinate to the newer active source-first objective;
their eventual product requirements remain binding:

- Ordinary C++ library, plugins and applications build/run with no installed
  Kotlin compiler/JVM/Native runtime. Normal C++ classes, stdlib values and MLX
  calls retain actual types and ownership; no Kotlin Any inheritance requirement.
- Explicit Native boundary uses the actual matching runtime/continuation/result/
  GC contracts. Only real Kotlin GC objects acquire roots. Borrowed C++ pointers
  are not owned, rooted Native objects or alternate runtime state.
- CMake frontend and mandatory LLVM module injection run inside selected Clang,
  before optimization, using its matching LLVM package. No Python compile
  launcher, production serialized/reparsed IR or marker runtime fallback.
- Real IR declaration/symbol-owner identity and CodeContext scope delegation;
  reads of exact suspension ID declarations resolve actual owning block addresses.
  Captured fields and return rewrites use actual declarations, not names/indices.
- Tail calls avoid unnecessary frames; non-tail calls use source state-machine
  construction, liveness and spill save/restore. Frame label, marker, decision
  and completion outcome are distinct. Resumed failures are checked before work.
- Mandatory plugin owns address stores and indirectbr dispatch. Authors do not
  manually construct frames or save locals. Preserve C++ lifetime/cleanup on
  repeated suspension, completion, failure and cancellation.
- Full completion requires BOTH standalone ordinary C++/real MLX GPU execution
  without Kotlin transitive dependencies and direct Native↔C++ shared-state-machine
  handoffs with real MLX GPU execution, both directions, identity and cleanup.
  Scalar, array, callback/StableRef and isolated dispatch tests establish neither.

Later compiler continuation is preserved in DOCKING_RING_HANDOFF.md,
IR_IDENTITY_DEPENDENCIES.md and IR_HANDOFF_REVIEW.md. Do not translate the entire
compiler now. Required prerequisites must name their source consumer and return
back to that consumer. Existing plugin strict rebuild failures remain separate
from source drafts and analyzer build success.

## Concrete continuation sequence

1. Read active objective and this handoff, inspect status/log and ca037a93 diff.
   Preserve checkpoint; do not claim its audits/reports/runtime are current.
2. Complete review of direct producer-lambda capture/continuation ownership and
   source function correspondence. Inspect IntrinsicsNative/Cancellable/
   CoroutineStart actual consumers. Fix discovered source differences without
   adding a manual frame or changing borrowed ownership.
3. Continue the remaining ChannelFlow::collect source lambda/scoped entry and
   Merge consumers only after fully reading matching sources and lifetime paths.
   Remove callable adapter only when no production consumer remains. Keep source
   declaration/protected hook, diagnostics and KDoc parity visible.
4. Strictly compile actual changed implementation/instantiations and consumer;
   record exact failure or execution. Do not suppress warnings or substitute
   old executable results. Ownership regression fixtures already exist in
   test_channel_as_flow_smoke and test_channel_consumption; runtime evidence
   must be rebuilt from the changed source before using it.
5. Validate provenance bounds/source comments, update API_AUDIT and current
   CHANNEL_FLOW_SCOPE_AND_SPILLS checkpoint with actual locations and limitations.
   Refresh both full-root deep reports for ca037a93/final source, inspect outcomes,
   commit results and update existing source card accurately.
6. Continue real library source repairs in refreshed oracle order. Keep the
   whole-library and both eventual product acceptance requirements open.

No user answer, external approval or new agent is needed to continue this work.
