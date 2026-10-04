# Kotlin algorithm and suspension parity repairs

Date: 2026-10-04. Branch: solace/sharing-transliteration.
Ground truth is the checked-in Kotlin library and Kotlin compiler/runtime.
The active repair umbrella is Kotlinmania Kanban t_1834dcec. Codex performs the
coroutine repairs locally; these cards do not request Hermes worker runs.

## Confirmed findings and tracking

| Card | Kotlin ground truth | Confirmed C++ divergence | State |
|---|---|---|---|
| t_59e8d50a | Native ContinuationImpl.kt:104-115; CancellableContinuation.kt:423-435; Yield.kt:145-166 | Context interception bypassed; queued tasks had non-owning handles; yield skipped YieldContext; synthetic native Unconfined dispatcher resumed inline; terminated handler captures retained frames | Verified in this checkpoint |
| t_79ca4abe | flow/operators/Limit.kt:48-73,127-138 | take aborts before a suspended final emit resumes; predicates lack suspend signatures; collection lifetimes and post-abort cancellation check diverge | Queued |
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
