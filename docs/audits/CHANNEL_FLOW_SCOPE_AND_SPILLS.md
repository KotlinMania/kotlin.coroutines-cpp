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
The typed header bindings at `ChannelFlow.hpp:245,262,326,481` retain actual
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
