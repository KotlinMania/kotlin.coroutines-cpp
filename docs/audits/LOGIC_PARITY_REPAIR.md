# Kotlin algorithm and suspension parity repairs

Date: 2026-10-04. Branch: solace/sharing-transliteration.
Ground truth is the checked-in Kotlin library and Kotlin compiler/runtime.
The active repair umbrella is Kotlinmania Kanban t_1834dcec. Codex performs the
coroutine repairs locally; these cards do not request Hermes worker runs.

## Confirmed findings and tracking

| Card | Kotlin ground truth | Confirmed C++ divergence | State |
|---|---|---|---|
| t_59e8d50a | Native ContinuationImpl.kt:104-115; CancellableContinuation.kt:423-435; Yield.kt:145-166 | Context interception bypassed; queued tasks had non-owning handles; yield skipped YieldContext; synthetic native Unconfined dispatcher resumed inline; terminated handler captures retained frames | Verified in this checkpoint |
| t_79ca4abe | flow/operators/Limit.kt:17-140 | Source-ordered Limit algorithms with retained macro frames, suspended predicates, emitAbort, owned abort and cancellation paths | Implemented; local acceptance recorded below |
| t_51240f83 | flow/terminal/Logic.kt | any/all/none return a synchronous bool, discard suspended predicates/collection, and swallow unrelated aborts | Queued |
| t_f2155697 | flow/operators/Transform.kt | Suspended filter/map/onEach/fold/reduce results bypass remaining algorithm; runningFold skips collection after suspended initial emission; stack collectors and chunk finalization do not survive suspension | Queued |
| t_356e6dfc | flow/internal/Merge.kt:47-94 | Concurrent and limited merge use OS threads, synchronous semaphore acquisition and joins, and discard collection suspension | Queued |
| t_1f9908aa | flow/internal/Combine.kt:16-138 | combine workers/polling replace coroutine launch/send/receive/yield; resumed transform loses batching state; zip uses capacity 1 rather than rendezvous, and lacks upstream context/cancellation and suspend-transform structure | Queued |
| t_eedecb8e | flow/operators/Share.kt:322-353 | Deferred stateIn collects without a continuation, uses stack state/collector, and resumed await bypasses unwrapping | Queued |
| t_16bf1579 | NativeSuspendFunctionLowering.kt:253-335; CoroutinesVarSpillingLowering.kt | Generator emits unconsumed sidecars, lacks faithful nested/value/argument/liveness/finally lowering, does not retain constructor parameters correctly; plugin target omits analyzer implementation | Queued |
| t_037fc89b | CancellableContinuation.kt:423-490; BufferedChannel.kt; StateFlow.kt; SharedFlow.kt | Raw channel/await/reusable adapters bypass interception; raw flow slots and reusable pointers lack Kotlin GC reference ownership | Queued |
| t_ee048b83 | AsyncTest.kt:266-298; JobTest.kt:141-157; CollectLatestTest.kt:18-21; Builders.common.kt:79-111 | Simplified tests replace suspend/finally order; suspend async overload and erased value unboxing are absent; IR fixture omits prebuilt-library sanitizer link flags | Verified in this checkpoint |
| t_d1b9ce81 | Builders.common.kt:140-173; CoroutineScope.kt:280-287; Supervisor.kt:50-66 | withContext and scope builders substitute stack scopes for scope coroutines and omit dispatcher/child-waiting branches | Queued |
| t_e0acb2be | ASTDistance AST identity and receiver matching | Empty companions and valid extension-to-free-function lowering cause matching false alarms; actual source-path marker equivalence needs proof | Verified, committed 9d9d49ef; 8/8 strict and 8/8 ASan tests |

The command strategy, regular sharing launch and transformLatest cancel/join path
already use suspending delay/join and UNDISPATCHED launch. They are not reported
as new failures. JobSupport.join itself follows its Kotlin fast/tail-suspend
algorithm; the reproduced dispatcher failure was in interception underneath it.
The IR pipeline cleans markers from pre-existing macro state machines. That is
not automatic suspend extraction or proof of Kotlin/Native binary/GC ABI parity.

## Interception repair

- ContinuationImpl.cpp:12 implements cached context interception against Native
  ContinuationImpl.kt:104-107. Line 24 releases the interceptor and stores the
  completed sentinel against lines 109-115.
- ContinuationInterceptor.hpp:19 exposes the erased virtual interception entry,
  and line 24 exposes release. CoroutineDispatcher.hpp:137 provides its erased
  override alongside the public typed template. The default interceptor release
  remains a no-op as in the Kotlin interface.
- CancellableContinuationImpl.hpp:1768 and :1807 retain typed dispatched delegates
  before converting completed values to the erased ABI; the void paths follow
  the same dispatch decision. Prompt cancellation runs before boxing.
- common/DispatchedTaskDispatch.hpp:110 and :140 enqueue owning task handles.
  This models Kotlin GC queue retention without changing the scheduling branch.
- Yield.cpp:43 follows interception, safeIsDispatchNeeded, dispatchYield,
  YieldContext, and yieldUndispatched in source order. native/Dispatchers.cpp:101
  returns the canonical Unconfined object.
- ContinuationImpl.cpp:40 provides direct-entry termination cleanup. Existing
  macro frames enter through start instead of bypassing interception release on
  an immediate result or exception. Resumption still uses resume_with and
  invoke_suspend; suspension state still uses computed-goto DSL labels.
- JobSupport.cpp:166 and :197 relinquish one-shot handler captures after invocation
  or disposal. Published intrusive nodes are still retired under the existing
  node policy. The explicit ownership notes describe the GC adaptation.

No built-in C++ coroutine machinery is introduced. The repaired receiving ABI
adapters own unboxing and deletion of non-Unit result boxes. Remaining adapters
and ownership gaps are recorded separately on the queued cards.

## Evidence and validation

Before repairs, tmp/logic-parity-review/probe.stdout.txt reproduced take returning
complete after downstream suspension, any returning false after predicate
suspension, filter resuming its parent without emitting, interception returning
the frame itself, Unconfined yield throwing, and join resuming before queue drain.
The probe is evidence, not an aggregate semantic score.

The registered test_continuation_dispatch checks dispatcher join/delay, queued
cancellation, typed result boxing, typed on-cancellation handling, pending
Unconfined work, immediate yield, disposed join handlers, interceptor caching,
release on immediate success/failure, and frame lifetime after resumed completion.
Full validation passed: 32/32 registered tests in Release with AddressSanitizer
and 24/24 in a separate Debug build (ASTDistance disabled there). Both run the IR
fixture, whose eight internal checks cover Make, Ninja Multi-Config and optimized
ASan. Receipts: tmp/logic-parity-review/full-runtime-ctest.txt and debug-ctest.txt.
These gates cover the current repairs; the confirmed queued gaps remain open.

## Suspend builder and test dependency repair

Builders.hpp:276 restores async's erased suspend-block overload, with a lazy
Deferred coroutine at :262. Native newCoroutineContext actuals are public in
CoroutineScope.hpp and implemented at native/CoroutineContext.cpp:29 and :39;
suspend launch and async both call that helper. The helper merges contexts and
inserts Dispatchers.Default only when the merged context has no interceptor,
matching Native CoroutineContext.kt:32-40.

Continuation.hpp:218 owns non-Unit result unboxing and deletion in the receiving
adapter. CoroutineStart.hpp:395 and :423 route immediate erased results through
the same adapter used for resumed completion. The async regressions check DEFAULT,
LAZY and UNDISPATCHED start, resumed value 42, immediate value 99, and default
interceptor insertion. Existing synchronous builder and structured scope gaps
remain queued; this change does not certify all builder implementations.

Only the three identified test methods were replaced with retained macro frames.
Their Kotlin assertion order and finally behavior are preserved. The IR fixture
retains baseline-versus-transformed execution, result/exception/independent-frame
assertions, Make and Ninja coverage, optimized ASan and dependency rebuild checks.
It now receives the prebuilt library's link flags so moving runtime definitions
out of headers does not cause sanitizer-runtime link failures.

Additional optimized ASan stress passed 25 executions each of
 test_continuation_dispatch, test_share_hot_flow_smoke and test_sharing_suspension
(75 executions total). Receipt: tmp/logic-parity-review/runtime-stress-ctest.txt.
Fresh SharingStarted IR contains 10 indirectbr instructions and 14 blockaddress
references both before and after cleanup; its 14 marker calls become zero.
This confirms preservation of the existing computed-goto resume dispatch, not
completion of the queued automatic extraction/spilling compiler work.

## Limit transliteration — 2026-10-04 local continuation

Card t_79ca4abe remains part of the local Codex umbrella t_1834dcec. The live
card transfers the historical Sol reservation to Codex. Iris confirmed no
overlapping writer in this scope; her current session supplied read-only source
guidance. Ren's JobTest card identifies a separate scratch workspace. The main
checkout began clean at f97e3039 on solace/sharing-transliteration.

The implementation follows the original Limit.kt declarations and control flow.
The suspension guide, IR specification, docking-ring design and Native Kotlin
lowering sources were read before production changes. In particular,
NativeSuspendFunctionLowering.kt:253-335 supplies the immediate/suspended/resumed
result branches, and CoroutinesVarSpillingLowering.kt:68-105 supplies the retained
live-state model. C++ frame members hold those live values; existing macros
generate resume addresses, sentinel propagation and resumed failure checks.

| Kotlin source | C++ entry in flow/Limit.hpp | Preserved algorithm and state |
|---|---|---|
| Limit.kt:17-26 | drop:41 | Validate count; keep skipped count per collection; tail-emit after the prefix |
| Limit.kt:30-40 | drop_while:85 | Keep matched flag; await predicate before deciding; set matched before emitting the first retained value |
| Limit.kt:47-68 | take:162 | Separate ownership marker and consumed count; tail-call ordinary emit or emitAbort; catch only its own abort |
| Limit.kt:70-73 | detail::emit_abort:213 | Await downstream emit, then throw the ownership-marked abort; resumed failure propagates before the throw |
| Limit.kt:81-90 | take_while:251 | collectWhile predicate awaits the user predicate, then emission, then returns true; false excludes the value |
| Limit.kt:112-120 | transform_while:295 | Safe flow builder exposes its retained SafeCollector; transform remains a tail call inside collectWhile |
| Limit.kt:123-140 | collect_while:307 | Await predicate; false throws collector-owned abort; catch checks ownership and then coroutineContext.ensureActive |

The count-validation messages also match Kotlin. Existing synchronous C++
predicates are adapted to the same Boolean-result ABI. Suspended predicates
receive the current frame and return a heap Boolean or the suspension sentinel;
the receiving frame consumes and deletes the Boolean box on both immediate and
resumed paths. Collection retention is released at termination, including
exceptional completion, rather than waiting for the enclosing Job to finish.
The existing AbstractFlow::collect retained SafeCollector boundary in
internal/SafeCollector.hpp was verified and needed no edits.

`test_limit_suspension` registers 18 checks. It covers immediate counts and
predicates, final emission suspension, resumed failure, nested abort ownership,
upstream suspension/finally, no-Job collection lifetime, delayed predicates and
downstream emissions, foreign aborts, collectWhile's post-abort cancellation
check, take's distinct catch contract, genuine cancellable predicate and final
emit suspension, capture release before Job completion, resume-before-return,
multi-emission transforms, and independent/repeated collections. These are
focused Limit acceptance checks and selected upstream cases; they do not claim
completion of the separate upstream test-porting epic or other operator cards.

ASTDistance before/after receipts are in
`../../../automation-artifacts/2026-10-04-limit-repair/ast-before.txt` and
`ast-after.txt`. Strict source-function pairing improves from 7/8 to 8/8 by
restoring emitAbort. The body score changes from 0.092 to 0.050. The report lists
additional lowered C++ frame functions, unmapped parser constructs and ambiguous
repeated-name pairing. This is a remaining measurement limitation, not a claim
of high textual or full compiler parity; source/control-flow mapping and
executable continuation evidence remain separate.

Final local gates passed with the existing strict compiler flags:

- `cmake --build build-debug-parity -j4` and
  `ctest --test-dir build-debug-parity --output-on-failure`: 25/25 passed.
- `cmake --build build -j4` and
  `ctest --test-dir build --output-on-failure`: 33/33 passed. This configuration
  is Release (`-O3`) with AddressSanitizer; it includes eight ASTDistance tests.
- Both configurations include the eight-check IR pipeline fixture and the new
  18-check Limit executable. The baseline Limit check failed before production
  changes because take completed during its last suspended emit.
- The new test translation unit's raw/cleaned IR has 56 indirectbr instructions
  and 74 blockaddress references in both forms. Cleanup removes 74 marker calls
  and preserves every other byte. All 52 instantiated Limit invoke_suspend
  bodies contain saved resume addresses and indirect dispatch. This establishes
  the existing macro/cleanup contract, not automatic compiler-frame generation.

Build/test logs, full CTest transcripts, source/binary hashes, before/after AST
reports, raw/cleaned IR and a repeatable IR-check script are retained under
`/Volumes/stuff/Projects/kotlinmania/automation-artifacts/2026-10-04-limit-repair/`.
Iris's second read-only source comparison reported no confirmed suspension,
ownership or Boolean-result divergence; the executable receipts above are the
acceptance evidence.

Workflow audit: `.github/workflows/codeql.yml` is the only workflow file and
retains its weekly schedule plus workflow_dispatch, with no push/pull_request
triggers. No remote workflow state was changed. Fetch succeeded; the active
local branch was preserved. No PR, push, merge or deployment was performed.
The eight other production repair cards remain open under the umbrella.
