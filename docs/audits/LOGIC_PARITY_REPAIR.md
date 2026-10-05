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
| t_51240f83 | flow/terminal/Logic.kt:34-42,71-79,107 | Source-ordered any/all retained frames over collect_while; none awaits any; erased Boolean results, suspension and owned-abort cancellation preserved | Implemented; bounded local acceptance below |
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

## ASTDistance score repair — 2026-10-04

Sydney authorized canonical scorer debugging after the Limit checkpoint. New
card t_1cfd2fcb records the bounded repair; t_e0acb2be remains the completed
historical namespace/provenance repair. Canonical commit
30b232a86107c0667f59e189152666376601375c repairs three reproduced defects:

- Ordered logic previously omitted both increment and decrement: `++x` and
  `--x` scored identically at 1.0. Exact operators and prefix/postfix position
  now remain in the sequence; the changed-operator probe scores 0.833333,
  while its faithful comparison remains 1.0.
- Local collector methods lacked enclosing-function identity. Kotlin's
  collectWhile emit at Limit.kt:125 was incorrectly paired with drop's emit
  at Limit.hpp:57. It now pairs with collect_while's emit at Limit.hpp:354;
  lexical paths constrain matching before body ranking, in either direction.
- Unexpanded coroutine_begin/coroutine_end calls omitted parser-visible
  semicolons and mixed neighboring declaration/method evidence. A bounded
  syntax adapter adds a terminator only in available whitespace, preserving
  bytes, lines, original arguments and scored calls. The CLI announces each
  adapted line and leaves macro expansion unverified. Unknown macros,
  malformed bodies and layouts without available whitespace remain errors.

The unchanged Limit runtime source now pairs 8/8 with no parser errors. Its
corrected body score is **0.045**, compared with the historical **0.050**. The
wrong collector pairing had inflated the earlier score. No weights, missing
source penalties, call/operation terms, or helper/frame evidence were suppressed.
The remaining low value reflects a normalization boundary: this scorer does not
provide a verified correspondence between Kotlin suspension points and the C++
continuation/frame/ownership representation. It cannot certify or refute Kotlin
algorithm parity from this aggregate alone. The executable Limit and IR receipts
above remain independent evidence.

Nine committed scorer source/docs/test files were copied into tools/ast_distance
and verified by SHA-256 against the canonical commit. Inherited canonical
symbol-audit/Python-Rust edits were neither copied nor committed; their working
content was verified against the initial hashes. Canonical work used
session/2026-10-04-ast-limit-score, preserving the existing dirty tree. Project
work remained on solace/sharing-transliteration with no runtime source edits.

Validation passed:

- Canonical strict Release: 9/9 native/CLI tests, 1.58s.
- Canonical optimized AddressSanitizer: 9/9 tests, 6.09s.
- Integrated project scorer strict Release: 8/8 tests, 1.50s.
- Full project optimized AddressSanitizer: 33/33 CTests, 36.09s, including
  test_limit_suspension and test_ir_pipeline.

Regression coverage includes lexical-owner adversaries and reverse matching,
increment/decrement changes, update omission/position, macro payload changes,
preserved source offsets, lexical exclusions, and remaining malformed/unknown
syntax. Existing operator/literal/call/control-flow drift, vocabulary-stuffing,
namespace/provenance and missing-source checks continue to pass.

Before/after probes, complete comparison output, all build/CTest logs, source
copy manifests and source/binary hashes are retained under
`/Volumes/stuff/Projects/kotlinmania/automation-artifacts/2026-10-04-ast-limit-score/`.
Only local commits and developer executables were produced; no push, release,
deployment or workflow changes were performed.

## Limit control-flow and documentation verification — 2026-10-04

Sydney requested algorithmic control-flow verification independently of the
similarity score, and API documentation describing C++ rather than retaining
Kotlin references. Follow-up card t_fb2e14d7 covers this bounded work. The
original complete Limit.kt and current Limit.hpp were reviewed again. No new
algorithm divergence was confirmed: branch order, tail emissions, non-tail
predicate/emission suspension, owned abort handling, and the distinct take versus
collectWhile cancellation contracts remain as described above.

Current C++ entry lines after documentation restoration are drop:43,
drop_while:96, take:177, detail::emit_abort:228, take_while:275,
transform_while:345 and collect_while:363. Earlier line references in the
implementation and scorer sections are historical receipts from their commits.

The API docstrings now retain the original operator contracts, including count
validation, exclusion of the first false predicate value, multiple/skipped
transform emissions, and retention of emissions from the terminating transform.
They describe actual C++ callable signatures, Boolean-result ownership,
std::invalid_argument, continuation lifetime and collector restrictions. The
download-progress example uses the existing continuation macros and emits its
final progress before returning false. Source provenance remains separate plain
metadata comments, outside Doxygen blocks. A string/raw-string-aware lexical
comparison confirms all non-comment tokens in both edited source files are
unchanged. Documentation line shifts can change macro label names, so the
affected target and IR tests were rebuilt and run.

Independent executable comparison used the repository's original Limit.kt
function bodies with cached Kotlin JVM compiler 2.4.0 and coroutines 1.11.0.
Only the JVM facade annotation was renamed from FlowKt to ParityLimit to avoid
shadowing dependency builders; no algorithm bodies changed. Bytecode inspection
confirms calls to the compiled original operators rather than dependency copies.
The C++ harness uses production Limit.hpp and the existing retained source-flow
test fixture. Kotlin and C++ produced exactly **26 matching state snapshots**
across eight scenarios: drop, take, dropWhile, takeWhile, transformWhile with two
suspended emissions per input, failure during take's final downstream emission,
suspended predicate failure, and nested take ownership. Each snapshot records
emitted values, completed upstream emits, upstream finally calls, predicate
calls, completion calls, failure state and the pending suspension kind. This
demonstrates matching observed control flow for these scenarios; it is not a
proof for every input, scheduler, Kotlin backend or coroutine primitive.

Validation for this documentation follow-up:

- Rebuilt Debug Limit executable: all 18 suspension/ownership/lifetime/
  cancellation checks pass; focused Limit and IR CTests 2/2, 27.05s.
- Rebuilt optimized AddressSanitizer Limit executable: all 18 checks pass;
  focused Limit and IR CTests 2/2, 26.86s.
- The exact Doxygen method excerpt compiles in a retained frame under optimized
  AddressSanitizer and passes two suspended progress emissions, retaining the
  final progress and excluding the subsequent input.
- ASTDistance still pairs 8/8 with body score 0.045. The unsupported suspension
  normalization boundary remains; the score is not semantic acceptance evidence.

Reproducible Kotlin/C++ harnesses, original-source hash/adaptation receipt,
bytecode verification, full state traces, lexical/documentation checks, build and
test logs are under
`/Volumes/stuff/Projects/kotlinmania/automation-artifacts/2026-10-04-limit-doc-parity/`.
The prior full Debug 25/25 and optimized ASan 33/33 gates are historical results
above; this follow-up reran the affected Limit/IR gates, not the whole suite.
No other production files, continuation ABI or algorithms changed. The parent
repair and other operator/compiler cards remain open. Workflow shape remains
unchanged: only codeql.yml, with weekly schedule and manual dispatch. No push,
PR, deployment, live model call or remote workflow change was made.


## Logic transliteration and independent corrections — 2026-10-04

Sydney explicitly delegated this bounded implementation to Iris's existing
Antigravity conversation under Codex review. The original complete Logic.kt,
Limit.kt and BooleanTerminationTest.kt were authoritative. Iris held the
implementation lease for Logic.hpp, the new suspension regression and its CMake
registration; ownership returned to Codex after her completion report. Local
safety checkpoint 3efab2f4 preserved the implementation before final acceptance.
Iris subsequently committed the documentation/assertion corrections at 6f84fdd4,
including the Logic.cpp provenance relocation. Both commits are preserved.

The C++ algorithms follow original source order. any initializes found to false,
awaits each predicate inside existing collect_while, records a true match and
returns the inverted predicate to collection. all records a false counterexample
and returns the predicate to collection. none awaits any and negates its result;
it does not implement a separate loop. Retained ContinuationImpl macro frames
preserve suspended local state and the existing computed-goto/erased ABI.
Predicate and delegated Boolean boxes are consumed by their callers. Existing
collect_while supplies owned-abort checking and the post-abort ensureActive check;
foreign aborts and resumed failures propagate. No replacement algorithm, synchronous
shortcut or compiler-lowering claim is introduced.

Codex reviewed those branches and strengthened the real cancellable-predicate
regression for each operator: independently observable weak handles now prove
that captured state and the upstream flow survive suspension and expire after
Job cancellation and terminal completion, without manually resuming the
predicate. Stale source-pause comments were corrected. The API documentation
uses C++ contracts, callable signatures, live completion requirements and actual
result ownership; provenance is plain metadata outside Doxygen. Three exact
Doxygen examples compiled and ran under optimized AddressSanitizer. Production
non-comment tokens remain identical to the safety checkpoint.

The new regression has 23 named checks: all 12 original BooleanTerminationTest
cases, plus 11 groups for immediate/delayed results, short circuit, upstream
suspension/finally, resumed errors, foreign abort, owned-abort cancellation,
automatic cancellable-predicate failure, captured/upstream lifetimes, all nine
interleaved operator pairs, and none's delegated Boolean consumption.

An independent offline comparison compiled the exact original Logic.kt and
Limit.kt bodies with cached Kotlin JVM compiler 2.4.0 and coroutines 1.11.0.
Only JVM facade annotations were renamed to avoid dependency shadowing;
bytecode confirms calls to these original compiled operators. Production C++
matched all 30 state snapshots across 12 scenarios: each operator with a match
at value two, exhaustion, empty source, and suspended predicate failure. Snapshots
include predicate calls, completed source emissions, finally count, completion,
failure, Boolean outcome and pending suspension kind. This is bounded observed
control-flow agreement, not universal coroutine/backend parity.

Final Debug (3/3, 26.10s) and optimized ASan (3/3, 26.36s) focused gates cover test_logic_suspension,
test_limit_suspension and test_ir_pipeline. Exact final results and timings are
retained in codex-final-debug-ctest.log and codex-final-asan-ctest.log. The earlier
full Debug 25/25 and ASan 33/33 results belong to previous repairs; this isolated
Logic change reruns affected gates. No ASan diagnostics were observed; macOS ASan
output is not a LeakSanitizer certificate. Explicit weak-handle tests provide
bounded lifetime evidence.

Generated Debug IR contains 148 actual Logic frame invoke_suspend instantiations,
279 indirectbr instructions and 282 blockaddresses. All 282 marker calls are
removed; every other byte of the IR is preserved. Function matching uses the
actual demangled method identity, excluding allocator/helper names containing an
enclosing invoke_suspend name. Macro cleanup remains cleanup of existing state
machines, not automatic extraction of arbitrary suspending C++.

ASTDistance extracts 3 Kotlin and 23 C++ functions: 3/3 original operator matches,
zero unmatched source and 20 unmatched target helper/frame methods. Its body
score is 0.018 versus 0.081 for the previous incorrect synchronous implementation.
Unsupported suspension/frame normalization remains visible; no weights or
helper evidence were suppressed. This score is not semantic acceptance proof.
The handoff baseline probe demonstrates a compile-time API mismatch, not an
executed baseline runtime regression.

Evidence, source/binary hashes, full raw/cleaned IR, exact offline harnesses,
traces, documentation examples, scorer output and build/test logs are retained at
/Volumes/stuff/Projects/kotlinmania/automation-artifacts/2026-10-04-iris-logic-handoff/.
The original Iris receipt's claims of zero leaks, zero unmatched target functions
and a clean uncommitted tree were corrected explicitly. The umbrella and seven
other originally linked production repairs remain open. The only workflow is
.github/workflows/codeql.yml, with weekly schedule and manual dispatch; it is
unchanged. No push, PR, deployment, release, new Hermes worker or model API
dispatch was performed. Iris collaboration used the expressly authorized
existing desktop conversation.
