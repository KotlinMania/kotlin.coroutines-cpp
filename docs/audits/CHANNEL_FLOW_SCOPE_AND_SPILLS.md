# ChannelFlow source scope and private spill repair

## Adapter removal after Merge migration — 2026-10-07

Checkpoint 53c62d23 removes the four remaining Merge consumers and then removes
collect_channel_flow's declaration/body. No references remain under src.
ChannelFlow's actual scoped collection remains at ChannelFlow.hpp:300;
collect_in_scope remains at ChannelFlow.cpp:76 and the undispatched context
adaptations remain. Source compiler authoring and owned/borrowed boundaries are
still unverified under strict compilation. See MERGE_SOURCE_REPAIR.md for all
three fresh unsuccessful compile receipts and the exact diagnostic limits.
The old runtime evidence below does not certify these source changes.

Both full-root deep scans exit 0 after source commit 3194132a; reports are in
b9d8d99a. ChannelFlow stays 18/19 matched bodies and 6/6 types, similarity 0.25;
its target body inventory falls from 39 to 38 on adapter removal. The missing
upstream-prefixed to_string, diagnostic representation and broader source gaps
remain. Reference bounds are valid for 49 header and 11 implementation ranges.
Receipts use the merge-direct- prefix under build/ir-recovery.

## Direct source calls checkpoint — 2026-10-07

The complete source Channels.kt and ChannelFlow.kt were reread. ca037a93 removes
three collection-adapter calls: collect_to_fun forwards directly to collect_to;
ChannelFlowOperator.collect_to directly suspends flow_collect with the source
SendingCollector; collect_with_context_undispatched directly suspends the source
context call with original_context_collector. The two member bodies retain an
existing receiver owner and their local collector through compiler lowering.
Their owning continuation remains the final parameter. The producer-lambda
capture chain was inspected through Cancellable.cpp:58 and the actual
CreatedContinuation/RestrictedCreatedContinuation bodies in IntrinsicsNative.cpp:
block_ remains stored during suspension and is cleared on termination. This
source inspection does not establish fresh runtime retention behavior.

e8581c99 replaces the remaining ChannelFlow::collect adapter call. The raw virtual
entry at ChannelFlow.hpp:298 forwards to the annotated owning entry at :307.
That entry retains the existing flow owner, suspends the real scoped collection,
and its scope body directly calls emit_all(collector, produce_impl(scope)).
The owned channel/continuation projection at flow/Channels.hpp:204 forwards to
the same emit_all_impl with consume=true, preserving the channel owner and a
borrowed collector. This prevents a temporary produced channel from becoming a
raw dangling argument. Compiler lowering must retain and clean up the entry's
actual C++ owner; no replacement label or frame was authored.

ChannelFlow.hpp now has zero collect_channel_flow calls. Its declaration and
ChannelFlow.cpp entry remain because four actual Merge.hpp consumers still use
it. collect_in_scope and call_with_context_undispatched adaptations also remain;
this is not a claim that all source differences or helpers are removed.

Fresh strict actual consumer and explicit instantiation of ChannelFlowOperatorImpl<int>
and ChannelFlowOperator<int,int> both exit 1. The frontend recognizes the new
member entry, but generated code is rejected on GNU address-of-label diagnostics;
the actual consumer additionally exposes missing context initialization and
unresolved generated T, alongside existing unused-parameter dependency errors.
No warnings were suppressed and no executable ran for this checkpoint.
Receipts: build/ir-recovery/channel-flow-direct-scoped-consumer.log and
channel-flow-direct-instantiation.log. Earlier consumer diagnostics for ca037a93
are retained in channel-flow-direct-source-consumer.log. Existing plugin binaries
were reused, not rebuilt. Historical executable evidence below predates these
changes. Retention, destruction, repeated suspension, resumed failure and
cancellation still require fresh successful compilation and execution.

The final bounds check records 50 references in ChannelFlow.hpp and 24 in
flow/Channels.hpp; they resolve to source with valid line bounds and no prohibited
markers. This is source-reference evidence, not an algorithm or execution test.
Remaining gaps include source diagnostic string binding, upstream-prefixed
to_string, broader body/comment parity and both full MLX GPU acceptance paths.

Both final full-root ast_distance --deep commands exit 0, with no source edits
during scanning. Library totals remain 831/2918 bodies, 359/560 types, average
body similarity 0.26, documentation similarity 0.38 and 123 scoring failures.
ChannelFlow is 18/19 bodies, 6/6 types and body similarity 0.25 (previously 0.24);
Channels remains 12/12 bodies, 1/1 types and similarity 0.22. Added ownership
projections increase target bodies to 39 and 20 respectively; they are C++ ABI
adaptations, not additional Kotlin functions. Missing/provisional findings remain
required. Scan receipts are channel-flow-direct-final-{library,compiler}-deep.log.

## Earlier source authoring checkpoint — 2026-10-07

The complete pinned flow/Channels.kt and flow/internal/ChannelFlow.kt and the
existing C++ counterparts were read after rereading the docking-ring, IR lowering
and declaration/scope designs. The active objective keeps library source
translation ahead of compiler infrastructure.

Commit c659aa94 removes the handwritten CollectContinuation from
src/kotlinx/coroutines/flow/internal/ChannelFlow.cpp. The existing owning entry
at :58 is now annotated suspend and directly calls
`dsl::suspend(collect(completion.get()))`, then returns Unit's erased value.
There are no manually authored labels, yield macros or frame retention cycle in
that entry. The existing typed call bindings remain: collect_channel_flow is
still a C++ adaptation, not a source Kotlin declaration. Removing that adapter
and restoring direct source lambda/member calls remains unfinished. No new
adapter was introduced. Callable captures and the owning continuation must stay
alive through suspension and release on termination through compiler lowering.
The former executable evidence below predates this change and cannot establish
its new retention/destruction behavior.

Commit b330b6ee restores source val immutability for ChannelFlow context/capacity/
overflow, ChannelFlowOperator's upstream flow and UndispatchedContextCollector's
context/precomputed count/function reference. ChannelFlowOperatorImpl (:387),
UndispatchedContextCollector (:462) and ChannelAsFlow (flow/Channels.hpp:203)
are final, as their Kotlin declarations are. Missing source KDoc for
ChannelFlow.drop_channel_operators (:132-139) and produce_impl (:164-173) is now
translated; the latter preserves the #1825 reason for ATOMIC start and the
on_completion/finally cleanup explanation. Source field comments are restored.

Fresh strict Clang/LLVM 23 checks use the existing frontend and mandatory LLVM
module plugins, with -Wall -Wextra -Wpedantic -Werror and no suppression.
ChannelFlow.cpp compilation exits 1; the frontend recognizes the authored entry
but generated GNU address-of-label code is rejected, alongside existing unused
parameter diagnostics in dependencies. The actual test_channel_as_flow_smoke.cpp
consumer also exits 1 on dependency/generated-frame diagnostics. No executable
was built or run for this checkpoint. Receipts are
build/ir-recovery/channel-flow-source-authoring-syntax.log and
channel-flow-authoring-final-consumer.log. Full fresh runtime retention, result,
failure/cancellation and cleanup checks remain required.

The source-reference receipt records 47 ranged references in ChannelFlow.hpp
and 12 in ChannelFlow.cpp with valid source bounds and no prohibited source
markers. This verifies references, not complete transliteration. Both complete
MLX GPU acceptance paths, real IR declaration/scopes integration, diagnostic
string representation, the missing upstream-prefixed to_string and remaining
source/body/KDoc mismatches are still unfinished.

Both final full-root ast_distance --deep commands exit 0. Library measurements
remain 831/2918 function bodies, 359/560 types, body similarity 0.26,
documentation similarity 0.38 and 123 scoring failures. ChannelFlow remains
18/19 bodies and 6/6 types, body similarity 0.24; its target body inventory drops
from 42 to 38 after removing the handwritten class. No measurement criterion
was changed. Final receipts are channel-flow-authoring-final-{library,compiler}-deep.log.
The final source-bound check covers all three edited library files: 47, 12 and
22 ranged references respectively. These checks do not certify runtime behavior.

Historical checkpoints below retain their original evidence.

Date: 2026-10-07. Read complete common flow Channels.kt, Flow.kt,
flow/internal/ChannelFlow.kt, CoroutineScope.kt, internal/Scopes.kt and
intrinsics/Undispatched.kt before repairing the dependency used by the leading
Flow/channel priority groups.

`src/kotlinx/coroutines/flow/internal/ChannelFlow.cpp:121` now creates the source
ScopeCoroutine over the actual caller and uses start_undispatched_or_return.
The substitute ScopeCompletion decision machine and its claimed Native
SafeContinuation provenance are removed. The scope's caller frame and supplied
continuation retain their actual identities. Immediate entry returns or throws;
suspension resumes through the real scope. Attached children must finish before
scope completion. Child completion preserves the compiler caller's interceptor.

`ChannelFlow.cpp:18` contains the concrete private CollectContinuation for the
single suspend-call bodies at source lines 54-56, 118-121, 144-148 and 151-152.
The typed header bindings at `ChannelFlow.hpp:255,278,342,499` retain actual
flow/channel/collector owners and invoke their source operations. The private
algorithm and compiler labels are no longer duplicated in local header classes.
This continues to use the existing Continuation ABI and mandatory LLVM markers.
It introduces no alternative suspension decision protocol. Completed callbacks
are cleared at :39 on immediate or resumed success and failure, even if the
completed frame is held independently. Borrowed arguments remain borrowed.

The existing StackFrameContinuation/undispatched-context implementation in this
pair is preserved and included in this source checkpoint. It links the actual
caller, supplies the requested context, forwards resumeWith and returns the
source null stack trace. It remains a concrete private implementation in .cpp.

`src/tests/src/suspend/test_channel_consumption.cpp:150` checks direct return,
exact failure, suspended return, scope/caller/continuation/Job identity, waiting
for attached children and queued interception of successful or failed child
completion. Restoring the old ScopeCompletion path in a direct executable
reproduces failure. A replay with earlier identity assertions omitted reaches
the interception assertion and exits 1 at fixture line 253. Receipts are
`build/ir-recovery/channel-source-old-scope-build.log` and
`channel-source-old-scope-test.log`; its source fixture is adjacent.

The actual producer test at `test_channel_as_flow_smoke.cpp:562` retains the
completed continuation while requiring release of the flow and a captured
resource. It covers ordinary and channel-backed flow sources, success and
resumed failure. The old header frames reproduced a leak at test line 568
(exit 1, observed tool output); the concrete cleanup releases both owners.

Nine focused CTest executables finish with zero failures: BuildersTest,
test_sync, test_suspension_core, test_continuation_dispatch,
test_channel_as_flow_smoke, test_sharing_suspension, test_collect_reduce_smoke,
test_cancellable_start and test_channel_consumption. Build/execution receipts are
`channel-source-frames-regression-build.log` and
`channel-source-frames-focused-tests.log` under `build/ir-recovery`.
Two actual component executables, covering scope behavior and producer spills,
finish under AddressSanitizer and UndefinedBehaviorSanitizer with exit zero and
no diagnostics. Receipts are `channel-source-{scope,spills}-sanitizer-{build,tests}.log`.
Eighty-three ranged provenance references across the two library files resolve
with valid bounds; no prohibited source comment markers occur there. This
verifies references, not complete source parity or the full Native/MLX paths.

The initial complete-root scans falsely classified ChannelFlow.kt as a missing
file. Function prototypes in the parent namespace were included when computing
its implementation namespace. The corrected tool separates function/type
forwards from definitions, preserving declaration-only evidence and exact
per-callable scope checks. Pointer-returning prototypes are recognized; function
pointer variables, mixed prototype/variable declarations and foreign function
bodies still invalidate the unit's
namespace. Positive standalone/companion and negative definition fixtures are
in `tools/ast_distance/tests/identity_pairing_test.cmake`. All nine AST tool tests
finish with zero failures. The prior full-scan rejection is retained in
`channel-source-ast-before-library.log`; tool receipts are
`channel-source-ast-forward-{build,tests}.log`.

ChannelFlow diagnostic behavior remains incomplete: classSimpleName uses C++
RTTI text; ChannelFlowOperator's upstream-prefixed toString is absent; the
channel diagnostic still lacks actual source object/string representation.
Private generic collector wrappers also remain in the header. Neither full
symbol coverage nor these focused repairs establish complete file translation.

The corrected full-root scans complete with exit zero. Library measurements are
796/2918 matched body names, 354/560 types, average body similarity 0.26 and
122 scoring failures. ChannelFlow is now recognized as 18/19 body names and
6/6 types, with body similarity 0.24. ChannelFlowOperator.toString remains
missing. The increase in counted symbols principally corrects false file
exclusions, rather than proving additional algorithms translated. Final scan
receipts are `channel-source-frames-{library,compiler}-deep.log`, and refreshed
artifacts remain under `docs/audits/project-wide/{library,compiler}`.


## Consumed producer lambda and surface repair — 2026-10-07

The complete pinned ChannelFlow.kt was read before this repair. Its source collectToFun property at :54-56 now has the actual get_collect_to_fun getter at ChannelFlow.hpp:144,255. produce_impl at :270 consumes this shared lambda, preserving source ATOMIC start. The previous duplicated producer binding is removed. The getter captures the actual receiver, keeps an existing shared owner through queued start and the existing concrete suspended-call frame, and leaves raw receivers borrowed. No new frame algorithm or decision protocol was introduced.

ChannelFlow.hpp:136 repeats the fuse defaults inherited by Kotlin from FusibleFlow at source :26-30. C++ callers through concrete ChannelFlow/ChannelFlowOperatorImpl now retain the same default context, capacity and overflow contract. ChannelFlowOperatorImpl::drop_channel_operators at :388 is public as its inherited source contract requires. The create and flow_collect overrides remain protected.

The committed before-control 938b9706 is rejected by Clang for all three real gaps: too few fuse arguments, protected drop_channel_operators and missing get_collect_to_fun. Receipt: build/ir-recovery/channel-flow-surface-before-build.log. The repaired test_channel_consumption.cpp:943 checks zero/one/two-argument fusion, actual upstream identity and lambda receiver retention. :966 invokes that lambda with a real ProducerCoroutine/ProducerScope through actual rendezvous send. It checks resource identity, source pending-send behavior after close, immediate send failure, suspended cancellation, once-only invocation/completion and receiver/capture release. A preceding regression wrongly expected close to abort an already pending send; Channel.kt:144-146,234-236 explicitly allows that send to complete. The corrected regression exercises that behavior rather than altering production code.

The fresh complete core and seven focused targets build, and all seven CTest executables execute with zero failures (0.51 seconds). Current receipts: channel-flow-surface-final-{build,tests}.log under build/ir-recovery. The expanded fixture, actual ChannelFlow.cpp and common/internal/OnUndeliveredElement.cpp also compile and execute under AddressSanitizer/UndefinedBehaviorSanitizer with detect_stack_use_after_return=1 and no diagnostics. The fresh dependency archive is not wholly instrumented. Receipts: channel-flow-surface-sanitizer-{build,tests}.log.

Both final full-root deep scans exit zero. Library evidence records 818/2918 matched functions, 358/560 types, body similarity 0.26 and 123 scoring failures. ChannelFlow retains 18/19 function names and 6/6 types, with body similarity 0.25; ChannelFlowOperator.toString remains missing. Compiler/prerequisite evidence remains 591/7657 functions, 174/1727 types, body similarity 0.36 and 24 scoring failures. Generated evidence is unchanged by the final test-only corrections and remains committed in 4c2bb297. Receipts: channel-flow-surface-{library,compiler}-deep.log. No analyzer criteria were modified.

The docking-ring, IR lowering and declaration/scope designs were reread together with the full pinned NativeSuspendFunctionLowering.kt and CoroutinesVarSpillingLowering.kt, source IrToBitcode suspension scopes and the actual module injector. Existing library frames use the persistent-label/current-frame/Result contract and mandatory LLVM injection. Complete IR declaration identity/scopes, automatic spill lowering and direct shared Native/C++ frames remain unfinished compiler work. This repair does not establish either complete standalone or Native/MLX GPU acceptance path. Kanban t_8700df29 and source umbrella t_1834dcec remain open for their remaining source requirements.


## Structural interceptor equality — 2026-10-07

The complete pinned ChannelFlow.kt and existing C++ header/source pair were read
before editing. Kotlin's second fast-path comparison at :165 uses structural
`==`. The previous C++ body compared two shared_ptr values, substituting pointer
identity for virtual equality. The translated expression at ChannelFlow.hpp:487
now obtains the new and collecting interceptors in source order, invokes
new_interceptor.equals(collect_interceptor) for a non-null left operand, and
compares the right operand to null when the left operand is null. It adds no
new dispatcher, frame, runtime or ownership abstraction.

The committed regression eb8b22fb builds against the preceding production
header and exits one at test_channel_as_flow_smoke.cpp:211. Equal but distinct
interceptors incorrectly create a producer and present a SendingCollector to
upstream. The repaired production commit is 6d308d3b. The final regression at
fixture :161 forces the first full-context comparison to differ by adding an
upstream CoroutineName; it then distinguishes the actual undispatched collector
from the actual channel-backed producer, rather than checking a helper alone.
It checks source upstream interceptor identity and one invocation. Eight cases
cover equal and unequal interceptors, immediate completion, suspended success,
resumed failure, and resumed CancellationException. Suspended cases discard
external flow and resource owners and require retention through suspension,
exact failure identity, and release after termination. Cancellation here is an
actual CancellationException delivered on resumption; this fixture does not
establish the complete prompt-cancellation race guarantee.

Final core/focused builds finish with exit zero. Three CTest executables finish
with zero failures in 0.34 seconds: test_channel_as_flow_smoke,
test_channel_consumption and test_continuation_dispatch. The expanded fixture,
ChannelFlow.cpp, Flow.cpp and Channels.cpp also compile and execute under
AddressSanitizer/UndefinedBehaviorSanitizer with detect_stack_use_after_return=1,
exit zero and no diagnostics. The linked fresh dependency archive is not wholly
instrumented. Receipts under build/ir-recovery are
channel-interceptor-before-{build,test}.log, channel-interceptor-build.log,
channel-interceptor-final-{build,tests}.log and
channel-interceptor-sanitizer-{build,tests}.log.

Sixty-four ranged source references across the ChannelFlow pair resolve with
valid bounds; neither file has prohibited source markers. Both complete-root
deep scans finish with exit zero. Library evidence remains 820/2918 function
names, 359/560 types, average body similarity 0.26 and 123 scoring failures.
ChannelFlow is 18/19 function names and 6/6 types, with body similarity 0.24;
its upstream-prefixed toString remains missing. Compiler/prerequisite evidence
remains 591/7657 functions, 174/1727 types, body similarity 0.36 and 24 failures.
Receipts are channel-interceptor-{library,compiler}-deep.log. No analyzer
criteria were changed. These checks do not establish whole-file correspondence
or either complete MLX GPU acceptance path; the full translation goal remains
active.
