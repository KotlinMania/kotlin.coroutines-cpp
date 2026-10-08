# ChannelFlow source scope and private spill repair

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
before editing. Kotlin's second fast-path comparison at :164 uses structural
`==`. The previous C++ body compared two shared_ptr values, substituting pointer
identity for virtual equality. The translated expression at ChannelFlow.hpp:482
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
