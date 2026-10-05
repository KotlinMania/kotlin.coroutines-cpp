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
| t_eedecb8e | flow/operators/Share.kt:322-353 | Deferred sharing producer and receiving await/unbox frames retain collection, child job and typed results; underlying await uses the cancellable completion protocol | Implemented; focused local acceptance recorded below |
| t_16bf1579 | NativeSuspendFunctionLowering.kt:253-335; CoroutinesVarSpillingLowering.kt | Generator emits unconsumed sidecars, lacks faithful nested/value/argument/liveness/finally lowering, does not retain constructor parameters correctly; plugin target omits analyzer implementation | Queued |
| t_037fc89b | CancellableContinuation.kt:423-490; BufferedChannel.kt; StateFlow.kt; SharedFlow.kt; Share.kt:411-428 | JobSupport await, subscribed collection/action lifetime, flow stored references, reusable cache ownership and raw reusable ABI wrapper and three direct channel adapters and broadcast send and iterator prerequisites repaired; select typed receiving adapters, channel receiver lifetime, segment reclamation and publication races remain | Partially implemented; bounded evidence below |
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


## Current-card staleness revalidation — 2026-10-04

Sydney requested fresh ASTDistance measurement and verification that the remaining
repair claims describe current source. Checked clean solace/sharing-transliteration
at 2b6ef08663081d8b187abe18a2c5431a364dde20. Nineteen direct function comparisons were
run with the existing scorer, unmodified sources and repository configuration.
Sixteen exited successfully; reusable, compiler generator and liveness comparisons
were refused for namespace mismatches. Compiler comparisons also report Kotlin
parser errors. BufferedChannel, Builders and CoroutineScope inventories have
parser warnings; repeated-owner pairing warnings are retained. No failed
comparison is converted into a numerical score.

| Comparison | Source matches | Body score |
|---|---|---|
| Transform | 13/13 | 0.086 |
| Internal Merge | 9/9 | 0.260 |
| Internal Combine | 2/2 | 0.048 |
| Share | 12/13 | 0.216 |
| BufferedChannel | 100/111 | 0.296 |
| StateFlow | 17/19 | 0.286 |
| SharedFlow | 26/31 | 0.424 |
| Builders | 6/14 | 0.086 |
| CoroutineScope | 3/8 | 0.121 |
| Supervisor | 5/5 | 0.243 |
| Collect | 8/8 | 0.125 |
| Count | 2/2 | 0.026 |
| Collection | 3/3 | 0.112 |
| Validated Logic control | 3/3 | 0.018 |

JobSupport is split across its header and implementation: header 7/91, body
0.316; implementation 48/91, body 0.202. These inventories cannot be summed
or treated as a count of missing implementations. Scope's unmatched coroutineScope
is in Builders.hpp, not the scored CoroutineScope.hpp. All target helper and
unmatched-source evidence remains in the raw reports; named-function matching
is not proof of suspension or algorithmic parity. Logic's bounded validated
implementation demonstrates why a low aggregate score alone cannot support a
defect claim.

All seven active repair areas retain concrete source gaps on inspection against
their original Kotlin, rather than from score thresholds:

- t_f2155697: Confirmed current: Transform.hpp:92-93 returns collection with a stack TransformCollector; :142-155 suspending filter forwards the caller directly to predicate and has no retained post-predicate emission continuation. ASTDistance Transform 13/13 source matches, 32 unmatched target, body 0.086. Named matches do not restore the missing resumed branch.

- t_356e6dfc: Confirmed current: internal/Merge.hpp:296 blocking acquire, :304-310 raw worker thread/NoopContinuation collection, :343-347 outer collect result discarded and worker joins; limited merge :403-424 likewise. Original Merge.kt:51-71,89-94 uses coroutine acquire/launch and collect/finally release. Existing TransformLatest retained join at :107 is correct and excluded. ASTDistance 9/9, body 0.260; scope remains concurrent/limited merge.

- t_1f9908aa: Confirmed current: internal/Combine.hpp:155 collects into stack collector with nullptr in worker, :216 joins threads; :280 zip channel capacity1 versus original rendezvous restriction, :296 starts worker, :373 forwards collection completion without complete original context/transform/finally chain. Original Combine.kt launch/send/yield/receive/epoch/transform and zip contexts re-read. ASTDistance 2/2, body 0.048; pairing ambiguity warning retained.

- t_eedecb8e: Confirmed current: Share.hpp:282-324 launches synchronous block with stack collector and state reference, upstream collect :313 uses nullptr and ignores suspension; state_in :399-404 forwards await to parent so resumed path skips get_or_throw/StateFlow boxing. Original Share.kt:322-353 awaits then unwraps, child coroutine job retained. ASTDistance Share 12/13, body 0.216; both deferred entry functions are matched, not absent.

- t_037fc89b: Confirmed current: BufferedChannel.hpp:2043,2112,2167,3638 creates cancellable continuations over raw adapters without explicit intercepted delegate. StateFlow.hpp:501 stores &c into atomic slot; SharedFlow.hpp:797,802 stores raw continuation; Reusable.hpp:51-52 returns no-op-deleter shared_ptr. More precise JobSupport finding: .cpp:689-718 does NOT construct the defined AwaitContinuation; it registers ResumeAwaitOnCompletion directly on caller instead of original JobSupport.kt:1337-1347 intercepted AwaitContinuation/init/dispose/getResult. Channel receive placeholder claim on older t_d347fa2c is stale: receive suspend methods exist. Fresh AST: BufferedChannel100/111 body0.296, StateFlow17/19 body0.286, SharedFlow26/31 body0.424. JobSupport split hpp7/91 and cpp48/91 must not be summed or called missing implementation counts. Reusable comparison refused namespace mismatch; no score.

- t_d1b9ce81: Confirmed current Builders.hpp:336-369 with_context stack scope/direct block omits original context merge/ensureActive/dispatcher paths; :380-420 coroutine_scope/supervisor_scope use ContextScope. Qualification: separate Supervisor.hpp:110-117 DOES construct SupervisorCoroutine and start_undispatched_or_return; narrow defect to Builders overloads and reconcile actual ABI/call sites, not claim all supervisor implementations absent. No acceptance executed for alternate overload. AST Builders6/14 body0.086 and Scope3/8 body0.121 have parser warnings; Supervisor5/5 body0.243. Older t_c4d34d9a missing/stub/synchronous-only premise is stale (suspend async exists).

- t_16bf1579: Confirmed current plugin CMakeLists.txt:24-26 only compiles KotlinxSuspendPlugin.cpp, omitting SuspendFunctionAnalyzer.cpp. Generator .cpp:433-437 accepts constructor args without storing them; :451-495 traverses top-level marker statements, emits original call text/discards resumed value; .kx.cpp sidecar output :179 not consumed by production CMake path. Analyzer:43-44 excludes destructor CFG; :277 caps fixed point100. Both configured builds plugin/injector OFF. Production macro cleanup IS wired at root CMakeLists.txt:109-114, and verified Logic IR is existing retained frame cleanup, not automatic extraction. Strict compiler/liveness comparisons refused namespace mismatch with Kotlin parser warnings; no reliable direct score, no namespace rewrite to manufacture one.

Older card specifications were qualified without changing owners, statuses or
dependencies. t_d347fa2c's placeholder receive exception is obsolete: suspend
receive implementations exist. t_c4d34d9a's missing/stub/synchronous-only builder
premise is obsolete. t_fc954ad2 already has Count/Collection retained frames;
remaining Collect stack collectors still need attention. Those partial advances
are preserved, with no new runtime completion claim.

Exact commands, source/scorer hashes, complete stdout/stderr and findings are
under /Volumes/stuff/Projects/kotlinmania/automation-artifacts/2026-10-04-epic-staleness/.
This is static staleness verification; no runtime tests, compiler builds, source
implementation edits, scorer repairs, namespace overrides, worker dispatches or
pushes were performed. Earlier runtime receipts remain valid only for their
recorded bounded scenarios and revisions.


## Transform source-order repair and scorer correction — 2026-10-04

Sydney requested direct transliteration of the whole original Transform algorithm,
including suspension and state flow. Read Transform.kt and unsafeTransform in
Emitters.kt in full. The old delayed-filter probe emitted no value after a true
predicate resumed; the corrected probe emits one value and completes once with
Unit. Both executable receipts are retained. Iris's partial rewrite was saved in
3b693eef and 2d823163; Sydney then requested direct Codex takeover because of drift.
The desktop turn was stopped and the user archived its sessions. No fresh Iris
conversation, worker or model API dispatch was started. Codex reviewed and corrected
the preserved work directly.

| Original Transform operation | Reviewed C++ execution order |
|---|---|
| filter / filterNot | Await Boolean predicate; test true / false; await downstream emission; return Unit. |
| filterIsInstance | Pointer type test and successful emission preserve the represented branch; runtime-class and primitive/nullable reification remain unrepresented. |
| filterNotNull | Test pointer/optional presence; emit the non-null value; otherwise Unit. |
| map | Await transform; consume its owned result box; emit the result. |
| mapNotNull | Await nullable transform; test presence; emit only a present result. Optional, raw and shared-pointer overloads keep their distinct ownership contracts. |
| withIndex | Per collection zero index; copy old value, increment/wrap, check old value, emit indexed element. |
| onEach | Await action; emit the original value. |
| scan | Alias running_fold, before its definition. |
| runningFold | Per collection initial accumulator; await initial emission before collecting; await operation with previous state intact, assign successful result, emit. |
| runningReduce | Distinct disengaged optional sentinel, including nullable elements; first value assigns without operation; subsequent operation precedes assignment and emission. No default element construction. |
| chunked | Validate size before flow construction; initially absent buffer; allocate, append, await full emission, then clear; after successful collection emit the partial buffer. Failure never flushes. |

All original flow aliases use the existing unsafe_flow builder. Collection and
non-tail lambda/emission frames retain their continuations and release retention
on completion or failure; no new algorithm, thread, scheduler or continuation ABI
was introduced. C++ value/list copies remain an explicit representation adaptation.
The source-order ledger and original-test coverage ledger are in the artifacts.

Codex rejected a copied near-overflow indexing implementation that never called
production with_index. It was removed in favor of production guard checks and real
indexing paths; the full 2^31-emission boundary was not executed. Incorrect assertions
that upstream emission completed while downstream was suspended were corrected.
Delayed interleaving uses the retained source fixture, with separate stateful index
and fold checks. The new optional map_not_null implementation was constrained so raw
and shared-pointer callbacks cannot enter the wrong result-box path. Borrowed raw
results are copied without moving from caller-owned objects. Tests exercise those
lambda and std::function overloads. Docstrings describe the actual C++ index order,
shared initial-value contract, value emission, cancellation propagation and owned
result boxes, with no Kotlin references inside Doxygen.

Acceptance: 25 registered Transform test groups; full Debug 27/27 in 33.99s;
optimized ASan 27/27 in 35.46s. After correcting the Unit cancellation fixture to use
the real void specialization, the affected Transform target was rebuilt and rerun
in both configurations. The independent offline oracle compiled the exact original
Transform.kt bodies (only JVM facade metadata renamed). All 143 state snapshots
across 48 scenarios and ten operators match the final C++ linked to the instrumented
ASan core. Scenarios cover callback and downstream suspension, successful/empty
collection, callback failure, downstream failure and upstream failure. Actual Job
cancellation tests cover predicate, Unit action and reduce operation, including
finally-once and explicit frame/capture/upstream weak expiry. Four exact C++ Doxygen
examples compiled and ran under ASan. No ASan diagnostics were observed; no claim
of exhaustive leak detection is made.

Final Debug IR has 106 actual Transform frame invoke_suspend instantiations. All
180 marker calls were removed; the complete IR preserves 124 indirectbr instructions,
180 blockaddresses and every other byte. This verifies cleanup of the retained macro
state machines, not automatic lowering of arbitrary C++ bodies.

ASTDistance's extension-overload pairing incorrectly matched the original runtime
KClass argument to a receiver-only C++ overload. Canonical fix 9973f127 records CST
argument counts and actual suspend modifiers; it excludes omitted user arguments
while recognizing a real suspension ABI continuation. Native and CLI regressions
passed strict 8/8, optimized ASan 8/8, and integrated project strict 8/8. Exact four
source/test files were integrated in 00dc3d88; unrelated canonical dirty files were
preserved. Scored body tokens and weights were not changed. The unchanged baseline
now reports 12/13, aggregate body 0.084, matched average 0.091. Final Transform reports
12/13, aggregate body 0.072, matched average 0.078, and 88 unmatched helper/frame
methods. Forwarding overload selection and unnormalized continuation lowering remain
visible in the raw report. These low scores are not converted into semantic proof
or concealed with fillers.

The runtime-class overload and heterogeneous primitive/nullable reified APIs remain
absent. Original structured-child/channel and buffered-cancellation choreography
requires acceptance of the corresponding builder/adapter/merge repairs; this bounded
run does not claim those original tests passed. The original-test ledger distinguishes
23 nominal cases, nine such choreography cases, seven class-representation cases and
two IndexedTest cases belonging to Collect rather than Transform. The unfinished
parent audit t_0dd3c2ad also guards card closure. The seven-area epic is not complete.
The only workflow, .github/workflows/codeql.yml, retains weekly schedule and manual
dispatch and is unchanged. No push, PR, release, deployment or remote workflow change
was performed. Evidence is under
/Volumes/stuff/Projects/kotlinmania/automation-artifacts/2026-10-04-transform-repair/.


## Transform comment translation completion — 2026-10-04

Sydney requested closing the comparison further, explicitly allowing faithful
comment ports. Re-read the whole original Transform.kt and current Doxygen.
Completed the original running_fold vector-accumulation example, the running_reduce
reference to its initial-value sibling scan, and the full chunked map/printing
example using actual C++ FlowCollector and synchronous collection. Translated all
three buffer comments: no preallocation, allocation when needed, and cleanup
without allocating because the last full chunk may finish the flow. C++ contracts
and result-box ownership documentation remain intact. No Kotlin syntax or source
references appear inside Doxygen.

Four exact fenced examples compiled and ran against the optimized ASan core with
exit zero and no diagnostics. Their output values match the original examples;
the printing example produced exactly ab, cd and e on separate lines. Comparing
the pre-change and post-change C++ headers gives 100/100 function matches, body
score 1.000, AST similarity 1.000 and normalized logic 1.000; comment-stripped
source is unchanged. This is a comments-only change, so the previous algorithmic
acceptance is not presented as a fresh full-suite run.

ASTDistance intentionally excludes comments from normalized body ASTs and measures
documentation separately. Kotlin/C++ function coverage remains12/13 and body score
0.072. Documentation weighted score changed79.79% to79.62%, text cosine59.58% to
59.25%, and word-set overlap17.28% to17.86%. The extra C++ example vocabulary did
not increase the weighted word-frequency score; the more complete source examples
are retained instead of tuning prose or hiding ABI documentation. Scoring weights
and executable binary remain unchanged. Raw before/after reports, exact examples,
code-control comparison and receipts are under
/Volumes/stuff/Projects/kotlinmania/automation-artifacts/2026-10-04-transform-comments/.
The existing runtime-class representation and original choreography gaps remain.


### Ordered literal comparison checkpoint (2026-10-04)

ASTDistance now compares an AST-emitted in-memory C++ buffer with target C++
using positional exact-token cosine: equal spellings at equal positions divided
by the square root of the two token counts. Identifier/operator/literal spellings
remain exact; external whitespace is ignored. Documentation has a separate
ordered cosine after C++ reference replacement. Call order reversal with identical
AST histograms scores 15/17 (0.882353), while whitespace-only edits score 1.
The default Kotlin/C++ file report presents this literal score first;
`--compare-functions` retains its separately documented legacy structural metric.
No coroutine source, continuation ABI, runtime algorithm or existing body score
changed in this checkpoint. Strict, ASan and integrated strict suites pass 8/8 each.
The tested binary is copied into `tools/ast_distance/ast_distance` and canonical
ASTDistance; the receipt records exact SHA-256 and the canonical commit.

Transform literal emission currently reports coverage 0 and 17 unsupported spans:
its generic/extension/suspend rules require implementation before literal results
can guide its translation comparison. This is missing measurement support, not
a finding that the accepted runtime translation has zero behavioral parity.
Raw evidence and installation receipt are in
`automation-artifacts/2026-10-04-ast-literal-score/` at the workspace root.


### Production plugin direct/tail entry checkpoint (2026-10-04)

Read NativeSuspendFunctionLowering.kt, CoroutinesVarSpillingLowering.kt and
CoroutinesLivenessAnalysis.kt before changing the actual Clang plugin. The plugin
now links its analyzer and uses shared Clang/LLVM runtime registries when available.
Free direct/no-suspension entries and sole-tail entries preserve the original
continuation ABI and avoid an unnecessary frame, following native lowering's
non-tail decision. Parameter printing preserves array/reference declarators and
`noexcept`. Annotated callees are recognized; an identity wrapper around a marked
callee represents one suspension point. Site IDs follow source order instead of
CFG storage order. Liveness converges to the fixed point without an arbitrary
100-iteration cutoff. Restoring that cutoff fails the new actual 300-block CFG
regression. These are compiler-source changes, separate from ASTDistance scoring.

The real linked plugin extracts annotated input; generated C++ compiles and runs
against the actual runtime. Five direct/tail success/failure traces match a cached
Kotlin compiler oracle exactly. Tests check immediate/delayed completion, exactly
once invocation, caller-continuation forwarding/release, boxed payload cleanup,
and a borrowed array reference. Final integrated Debug CTest: 29/29 in 28.28s;
optimized ASan analyzer: 1/1 in 0.95s; optimized ASan generated handoff: exit 0.
Compiler and raw build/extraction/trace receipts are at workspace
`automation-artifacts/2026-10-04-production-lowering/`. Runtime source and the
manual computed-goto frame ABI are unchanged in this checkpoint.

Non-tail sidecars still require callee-continuation rebinding, constructor argument
storage, resumed value consumption, complete spill declaration/reference rewriting
and entry/resume lifetime cleanup. Automatic production sidecar consumption,
Kotlin/Native binary interoperability and GPU/hardware execution are not established
by these direct/tail tests. Compiler card t_16bf1579 retains these open findings.


### Deep suspension review evidence (2026-10-04)

ASTDistance --deep now persists callable-scoped suspension review leads beside
its positional literal metric. Physical Kotlin/C++ locations and nested local
frame await/begin macros, markers and computed gotos are reported; comments,
strings and preprocessor definitions cannot supply lowering evidence. Tail or
helper forwarding is distinguished from marker-only and absent local lowering.
Overload candidates and provisional parse evidence remain visible. These syntax
leads do not change scores or establish compiled IR, lifetime or cancellation parity.

The real original flow/operators scan shows Transform filter at Transform.hpp:192
with await macros at 207/210; its forwarding overloads at 240/250 are helper leads.
Share.hpp:388 state_in is marker-only. Merge.hpp:126 flat_map_concat accepts a
synchronous transform while the original callback contract is suspend; its collector
forwards emission directly. Share.hpp:345 SubscribedSharedFlow::collect forwards a
locally owned collector's raw pointer. These require callback/lifetime review in
the relevant cards; this measurement change does not repair those runtime paths.

Tool acceptance: strict CTest 8/8 in 1.56s, optimized ASan CTest 8/8 in 5.15s,
project integration CTest 8/8 in 0.70s. Canonical task-only source commit 7bc83f3;
inherited symbol-audit work preserved byte-for-byte and excluded. The exact tested
binary is installed in the canonical checkout and this project's tools/ast_distance.
Raw scan, regression logs, preservation proof and SHA receipt are at workspace
automation-artifacts/2026-10-04-deep-suspension-review/.


## Deferred sharing producer repair — 2026-10-05

Card t_eedecb8e has a bounded producer repair against Share.kt:333-353.
Share.hpp:285 retains the upstream, mutable state, collector and launch completion
in a computed-goto frame. Collection receives that frame and the empty-flow
check runs only after collection completes. The first value wraps the state with
the sharing child's job; later values update the same state. Resumed failure
completes the result exceptionally and is rethrown to cancel the sharing scope.

Instantiating the real Result<StateFlow> deferred exposed prerequisite defects
in CompletableDeferred.hpp: its cancellation override had the wrong name, the
completion-exception override was missing, construction registered a parent
before shared ownership existed, and completion attempted to pass a raw typed
value to JobSupport's polymorphic state API. Factories now attach the parent
after construction and completion/get_completed use the existing CompletedValue
representation. Updated comments describe the C++ construction and ownership.

The existing test_sharing_suspension fixture covers delayed first value, a second
update, cancellation of the sharing child without cancelling the parent, parent
cancellation before the first value, empty completion, failure before/after the
first value, and release of both retained frames. It also instantiates both
deferred factories and checks typed completion, repeated completion and parent
cancellation. No disposable harness or new test infrastructure was added.

Focused acceptance: test_sharing_suspension and test_share_hot_flow_smoke pass
2/2 in Debug (1.54s) and 2/2 in optimized ASan (1.89s). Raw build/test logs are
under workspace automation-artifacts/deferred-sharing-producer-20261005/. This
is producer acceptance, not completed state_in acceptance: Share.hpp:427 still
forwards await to the parent and unwraps only on immediate return; the generic
CompletableDeferred await exposes JobState rather than a typed ABI box, and
JobSupport await's raw continuation path lacks the Kotlin cancellable await
protocol. Those receiving-path repairs remain on the card.


## Deferred state_in receiving path — 2026-10-05

The receiving-path repair completes card t_eedecb8e after producer commit
5b1932e8. Share.hpp:429 retains the result deferred in StateInAwaitFrame, awaits
with its own continuation, and unwraps the Result on both direct and resumed
paths. The intermediate Result box is owned and freed; the returned StateFlow
shared-pointer box is owned by the receiving caller. The entry function retains
Kotlin Share.kt:322-328 configuration, deferred creation and sharing-launch order.

JobSupport.cpp:666 now uses the existing AwaitContinuation cancellation-cause
algorithm against JobSupport.kt:1272-1289 and :1337-1350. It intercepts the
delegate, initializes cancellability, registers the completion handler, disposes
that handler on waiter cancellation and calls get_result. The awaited job remains
owned through completion. The internal JobState pointer stays borrowed; a direct
cancellable return frees only its temporary pointer box. CompletableDeferred's
receiving frame converts that state into an owned typed ABI box before forwarding
completion. The already-completed fast path preserves Kotlin's behavior of not
checking the caller's job.

The lifetime regression found that typed exceptional resumption omitted
detach_child_if_non_reusable and retained completed frames in the waiter job.
CancellableContinuationImpl now detaches on successful exceptional transitions,
uses the supplied dispatch mode, and ignores the first late exceptional resume
after cancellation as in Kotlin resumeImpl/tryResumeImpl (:493-553). The exception
state is owned; cancellation/failure paths release the new await frames.

Existing sharing tests now cover immediate/resumed typed values and failures,
queued prompt cancellation, waiter cancellation followed by late completion,
child failure cause preservation, immediate/resumed state_in empty results,
continued updates, failure after the first value, and completion/frame release.
A cancelled state_in waiter leaves the independent sharing scope running. Both
returned boxes are consumed by their actual typed receiving paths. No new test
harness or target was added.

Focused verification: AsyncTest, test_continuation_dispatch,
test_share_hot_flow_smoke and test_sharing_suspension pass 4/4 in Debug (2.68s)
and 4/4 in optimized ASan (2.63s). Evidence is in workspace
automation-artifacts/state-in-await-20261005/. These checks accept the bounded
state_in/deferred-await repair; they do not certify all DeferredCoroutine/select
adapters or SubscribedSharedFlow's separate collector-retention path.


## Subscribed collection and action lifetime — 2026-10-05

This bounded slice advances t_037fc89b against Share.kt:411-428. Share.hpp:384
retains the delegated shared flow and the subscribed collector through the
collection suspend point. The ownership frame replaces the GC reference that
otherwise disappeared when the C++ local shared pointer returned. Existing
null-continuation forwarding is preserved for synchronous callers.

SubscribedFlowCollector.hpp:33 runs the action with a retained SafeCollector
and the current context, awaits its completion, releases that collector in the
success/failure cleanup paths, and only then enters the next subscribed
collector. It follows the original action/finally/nested-subscription order.
SafeCollector's existing native release hook is unchanged; no additional
collector algorithm or coroutine backend was introduced.

The existing virtual-time sharing regressions exercise SharedFlow and StateFlow
with two chained subscription actions. The first action suspends before emitting
and its downstream emission suspends again. Actions finish in registration
order before replay/current-value delivery; values shared during setup are
retained for SharedFlow and conflated to the newest StateFlow value. Tests drop
the subscribed flow while suspended and verify the collectors' captured action
stays alive until collection completion. Setup frames release while collection
still waits. Failure suppresses replay, cancellation releases the subscription
slot, and a different job's emission is rejected by the real SafeCollector.

Focused acceptance: test_sharing_suspension and test_share_hot_flow_smoke pass
2/2 in Debug (1.87s) and 2/2 in optimized ASan (1.85s). Build/test logs and source
hashes are in workspace automation-artifacts/subscription-lifetime-20261005/.
No CI/configuration files or new test targets/harnesses were changed. The card
remains open for direct channel/reusable adapters, other typed receiving
adapters, and raw SharedFlow/StateFlow slot references without a retaining job.

## StateFlowSlot owning atomic reference — 2026-10-05

This slice advances t_037fc89b against StateFlow.kt:245-309 and the native
WorkaroundAtomicReference operations in internal/Concurrent.kt:15-30.
StateFlowSlot now uses C++17 atomic shared-pointer operations for the original
null/NONE/PENDING/continuation states. A suspended continuation is owned by the
slot independently of a parent Job. A loaded owning reference survives the
successful make_pending CAS while resume runs. The static symbol identities
remain distinct from the owning continuation reference.

allocate_locked, free_locked, make_pending and await_pending preserve the
original state transitions and CAS branches. take_pending now performs the
original atomic exchange to NONE, with the allocated-state assertion, rather
than only changing PENDING through CAS. No StateFlow value, sequence or slot-array
algorithm, coroutine backend or continuation ABI was changed.

The existing sharing tests now collect without a Job, queue two updates before
dispatch, verify conflation, terminate through the real abort path, release the
collection frame and reuse the freed slot. A direct real-slot regression covers
PENDING before continuation installation and installation before wake, exactly
one completion, free-slot wake and subsequent allocation. Existing sharing
cancellation coverage remains in the same test target; no harness was added.

test_sharing_suspension and test_share_hot_flow_smoke pass 2/2 in Debug (1.92s)
and 2/2 in optimized ASan (1.93s). Evidence is in workspace
automation-artifacts/stateflow-slot-ownership-20261005/. SharedFlow raw slot,
resume and emitter references, channel/reusable adapters and other typed
receiving adapters remain on the card. These tests do not establish all-race
or interop acceptance.

## SharedFlow stored-reference ownership — 2026-10-05

This slice advances t_037fc89b against SharedFlow.kt:294-314 and :385-725,
plus AbstractSharedFlow.kt:69-90. Waiting slots now retain their continuations.
Resume arrays retain collectors and emitters while slot/buffer references are
cleared under lock and resumption runs outside it. The shared slot interface and
StateFlow's empty resume-array return use the same owning element type.

The circular buffer now stores owning value, emitter or NO_VALUE references.
Queued emitters own their value and continuation. try_take_value keeps its local
value reference alive after update_collector_index_locked removes the buffer
entry, and collection retains the value across downstream suspension. Buffer
growth, drop, replay reset, cancellation, promotion and cleanup retain their
original index and size transitions; reference removal releases C++ objects.

Emitter cancellation registration remains outside the lock. Its callback holds
a weak emitter reference to avoid an emitter/continuation/callback ownership
cycle. A queued emitter is retained by the buffer, and the registration local
retains it during registration. After promotion or removal the original disposal
identity check would be a no-op; an expired weak reference has that same effect.
The original cancellation buffer-identity check and cleanup_tail_locked remain.
The touched API documentation describes the C++ operations, with upstream
algorithm references recorded here rather than embedded Kotlin signatures.

Existing sharing regressions cover no-Job waiting subscribers with two buffered
values and slot reuse, rendezvous and buffered emitters behind a suspended
collector, cancellation of a queued emitter, delivery order and exactly-once
completion. Both capacities are checked with normal collection and with the
last collector terminating early, which resumes all queued emitters. Weak value
and frame checks verify release; replay snapshot ownership, drop, reset and
flow destruction are also checked. No harness, target or CI/configuration file
was added or changed.

test_sharing_suspension and test_share_hot_flow_smoke pass 2/2 in Debug (1.84s)
and 2/2 in optimized ASan (1.92s). Evidence is in workspace
automation-artifacts/sharedflow-ownership-20261005/. This accepts the bounded
stored-reference repair while the flow and collector remain valid through their
calls. It does not certify every race, arbitrary external receiver destruction,
or interop. Direct channel/reusable and other typed receiving adapters remain
on the card.


## Reusable continuation ownership prerequisite — 2026-10-05

This slice advances t_037fc89b against CancellableContinuation.kt:442-479,
DispatchedContinuation.kt:70-172 and CancellableContinuationImpl.kt:140-158,
:169-189 and :473-553. The actual helper file is
src/kotlinx/coroutines/dsl/CancellableReusable.hpp. Its get_or_create helper now
returns real shared ownership instead of a no-op-deleter pointer.

DispatchedContinuation's reusable atomic state now owns its continuation and
postponed cancellation cause. C++17 atomic shared-reference operations preserve
null-to-claimed publication, continuation-to-claimed CAS, claimed-to-continuation
publication, claimed-to-cause CAS, first-cause retention and invalidation to null.
A claimed continuation stays owned after its cached state is replaced, including
while reset rejects an idempotent result and the helper creates a replacement.

The existing interceptor release boundary still waits for an active claim and
detaches its published continuation. A CAS then clears that same published owner
to break the C++ delegate/cache cycle after completion. It does not clear a
postponed cause, which must remain available to get_result. This is C++ lifetime
cleanup at the terminating interceptor boundary, not a new reuse algorithm.

The completed-reference regression exposed missing ownership of allocated CCI
success states in resume/tryResume and completed-result cancellation transitions.
Those successful CAS paths now assign owned_state_. Cancellation keeps the old
state alive through its handler calls. The void exceptional tryResume state is
also owned. Reset installs Active and releases the preceding completed state in
both specializations, matching removal of the original atomic state reference.
The original resume, decision, CAS, cancellation and dispatch branches remain.

The existing test_continuation_dispatch target checks actual cached ownership,
same-instance reuse, idempotent rejection, first postponed cause, interceptor
release before postponed cause consumption, published-state invalidation,
release of previous values and handler captures, and real lowered-frame cleanup
on success and prompt cancellation. Its existing typed cancellation check now
also verifies handler capture release. The resumed native void-pointer test
consumes its actual native result; it does not add an extra typed adapter box.

AsyncTest, test_continuation_dispatch and test_sharing_suspension pass 3/3 in
Debug (0.99s) and 3/3 in optimized ASan (1.10s). Logs and source hashes are in
workspace automation-artifacts/reusable-ownership-20261005/. No new harness,
target, CI/configuration change or push was made.

This accepts the owning reusable cache/get-or-create prerequisite. The raw ABI
suspend_cancellable_coroutine_reusable wrapper still needs proper retention,
interception and typed result adaptation; direct BufferedChannel and other
DeferredCoroutine/select adapters remain separate repairs. No full channel,
all-race or interop acceptance is claimed.


## Raw reusable ABI wrapper interception — 2026-10-05

This slice advances t_037fc89b against CancellableContinuation.kt:442-479,
using the owning cache prerequisite from 15dfd9dd. The raw wrapper now retains
its lowered frame, obtains the native intercepted continuation and selects a
correctly typed delegate before get_or_create. It follows the original block,
exceptional release-claim and get_result order. It does not introduce the normal
factory's eager init_cancellability step into the reusable algorithm.

The typed result adapter owns the underlying frame. A typed dispatched delegate
uses that frame directly, so cancellation is checked before allocating the result
box and delivery does not dispatch twice through the native interceptor. Direct
and resumed typed results are owned T boxes; void completion returns nullptr.
A plain, non-intercepted continuation uses the original non-reusable mode.
The helper's documentation now describes these C++ ABI operations.

The native intercepted continuation owns one active typed reusable delegate.
Repeated calls with the same C++ result type claim the same CCI instance. A
result-type change releases the previous typed cache instead of retaining all
previous types and their completed values. C++ continuation template instances
cannot change specialization as the original erased generic instance can, so
cross-type instance reuse is not claimed. The original reusable atomic protocol
is unchanged; a mutex guards only the C++ typed bridge pointer bookkeeping.
The existing terminating interceptor release boundary releases that typed cache,
breaking its frame/delegate/CCI ownership cycle.

Existing CallFrame and QueueDispatcher regressions exercise void and typed
immediate returns, suspended delivery, same-type reuse, type-change cache release,
one queued dispatch, void failure, cancellation during the block and after value
resumption before dispatch, unexpected block failure, plain-continuation fallback,
and value/CCI/frame release. No harness or test target was added.

AsyncTest, test_continuation_dispatch and test_sharing_suspension pass 3/3 in
Debug (1.00s) and 3/3 in optimized ASan (1.03s). Logs and source hashes are in
workspace automation-artifacts/reusable-wrapper-20261005/. Direct BufferedChannel
send/receive/receive-catching, iterator/broadcast paths and other typed receiving
adapters remain on the card. This accepts the reusable wrapper, not those direct
call sites or every race. No CI/configuration change or push was made.


## Direct channel reusable adapters — 2026-10-05

This slice advances t_037fc89b against BufferedChannel.kt:141-164, 708-780,
2041-2210 and ChannelSegment:2803-2855, using the reusable wrapper from 5f09e202.
The send, receive and receive-catching no-waiter suspension paths now call that
factory, which retains/intercepts the compiler frame and adapts its typed result.
They preserve waiter registration, cell callbacks, undelivered handling and
getResult order. The redundant send/raw receive adapters and eager normal-factory
init_cancellability calls are removed. ReceiveCatching owns its actual CCI.

Reference-valued regressions exposed necessary prerequisites: store_element must
copy the local element before rendezvous rather than consume it; each segment
element register must own its replacement and loaded value; try_receive must
consume its temporary E box. Element retrieval still loads then clears, and
cleaning publishes null, following the original register operations. Logical
cell states, CAS branches and memory ordering are retained. Waiter owner loads
and stores use C++17 atomic shared-pointer operations. Buffer expansion releases
the sender owner after resumption at its terminal transitions. Closed-channel
resumption recognizes the actual typed receive and void send CCI instances.
Close sweeps retain available owned waiter references across terminal cell
cleanup and later resumption, preserving reverse cell traversal, undelivered
callback order and FIFO resume order. This does not establish race safety for
raw cell publication or late owner installation. Touched register docstrings
describe the C++ operations and storage.

Existing CallFrame/QueueDispatcher checks exercise capacities zero and one: six
sender variants and twelve plain/catching receiver variants cover normal queued
delivery, cancellation before matching and cancellation after matching before
dispatch. They check exactly one completion, typed result boxes, undelivered
counts and waiter/frame release while the channel remains alive. Four close
cases check normal and exceptional plain/catching receives, including exception
identity. Two cancellation cases check queued senders' FIFO completion, reverse
undelivered order, exact cause and value/frame release. No harness or target was
added.

A rejected test assumption is recorded explicitly: original trySend failure
can leave its element in an INTERRUPTED_SEND cell until segment reclamation;
that path does not call the undelivered handler. No extra cell-cleaning branch
was added to satisfy an immediate weak-reference assertion. The C++ channel
currently allocates raw segments and its destructor only closes; segment
reclamation remains a confirmed lifetime gap. Iterator/broadcast, select and
other DeferredCoroutine typed adapters also remain. No full channel, all-race,
segment reclamation or interop acceptance is claimed.

AsyncTest, test_continuation_dispatch and test_channel_as_flow_smoke pass 3/3
in Debug (1.15s) and 3/3 in optimized ASan (1.41s). Final focused build/test
logs and source hashes are recorded in workspace
automation-artifacts/channel-direct-adapters-20261005/receipt.json. No
CI/configuration change or push was made.


## Broadcast channel send adapter — 2026-10-05

This bounded t_037fc89b slice follows BufferedChannel.kt:218-236 and the inline
sendImpl loop at 244-349. send_broadcast now uses the existing normal cancellable
factory, which retains the compiler frame, intercepts the typed continuation,
initializes cancellability before the block and obtains get_result afterwards.
The unsupported undelivered-handler check is inside that block as in the
original. The invalid cross-template continuation cast and unconditional
COROUTINE_SUSPENDED returns are removed.

SendBroadcast owns its bool CCI and provides actual shared waiter ownership to
the existing segment registration. The send path follows the original segment
load, counter acquisition, lookup, closed-status and cell-result branches.
RESULT_FAILED retries the loop with the same waiter and factory instance; it
does not recurse into a new suspension factory. Rendezvous cleans the previous
segment before success; the closed suspension and RESULT_CLOSED branches keep
their original cleanup. Channel cancellation resumes false through the existing
SendBroadcast closed-waiter case; job cancellation remains exceptional. The
Boolean ABI result is an owned box on immediate and resumed paths. No channel
CAS/publication or segment-reclamation algorithm was added.

The existing test_continuation_dispatch target covers eight suspended broadcast
cases at capacities zero and one: normal delivery, job cancellation before
matching, job cancellation after matching before dispatch, and channel
cancellation. Checks cover one queued dispatch, exact completion count, Boolean
versus exceptional results and frame/value release while the channel stays
alive. Immediate buffered send, normal/exceptional closed-channel false results,
unsupported-handler rejection, immediate rendezvous and retry past an
interrupted receiver are also exercised. No harness or target was added.

The iterator remains a separate repair: has_next_on_no_waiter_suspend constructs
a raw HasNextContinuationAdapter and directly initializes a reusable CCI,
bypassing interception and the reusable factory. The public iterator API returns
a unique_ptr, while its cell waiter is the raw iterator itself and cannot supply
shared waiter ownership. Its result is also an owning raw E box without a
destructor release path. These differ from the original inner iterator's
retained object and value references. No iterator lifetime/publication acceptance
is claimed. Raw segment reclamation, late waiter-owner installation and other
DeferredCoroutine/select typed adapters remain recorded on the card.

AsyncTest, test_continuation_dispatch and test_channel_as_flow_smoke pass 3/3
in Debug (0.95s) and 3/3 in optimized ASan (1.12s). Final source hashes and
focused build/test logs are in workspace
automation-artifacts/channel-broadcast-adapter-20261005/receipt.json. No
CI/configuration change or push was made.


## Iterator reusable adapter and stored waiter — 2026-10-05

This bounded t_037fc89b slice follows BufferedChannel.kt:1573-1744. The public
iterator() signature remains unique_ptr<ChannelIterator<E>>. Its unique handle
owns an internal shared BufferedChannelIterator, which supplies actual shared
waiter ownership to channel-cell registration. Dropping the public handle while
has_next is suspended therefore does not destroy the raw cell waiter. The
facade only forwards has_next/next; it adds no iterator algorithm.

has_next_on_no_waiter_suspend now calls the repaired reusable factory instead
of constructing a raw HasNextContinuationAdapter and eagerly initializing a
normal CCI. It retains/intercepts the compiler frame, uses a typed reusable
Boolean continuation and returns its owned Boolean box or suspension marker.
The original receiveResult/continuation store, clear, receive-cell callbacks,
undelivered handler and getResult order remain. The iterator owns the actual CCI
while pending, with no duplicate raw continuation alias. The non-original
try_resume_has_next failure cleanup is removed: Kotlin retains the retrieved
element even when tryResume fails. The iterator destructor releases an
unconsumed E box; next still clears receiveResult before returning the element.
Immediate has_next retains its original no-cancellation-check behavior.

Existing CallFrame/QueueDispatcher regressions cover ten suspended iterator
cases: normal delivery, cancellation before matching, cancellation after
matching before dispatch, normal close and exceptional close, each with the
public handle retained or dropped during suspension. They verify one queued
dispatch/completion, undelivered counts, exception identity, idempotent has_next,
next and closed-next behavior, and frame/value release after handle release.
Non-cancelled-before-match terminal paths also release the frame while the
public handle remains alive. Immediate tests verify missing-next rejection,
repeated has_next, next release and destructor release of an unconsumed buffered
reference while the channel stays alive, with an already cancelled job.
No harness or target was added.

Before-match cancellation leaves the original iterator continuation field
intact; this slice does not invent a cancellation callback that clears it.
Frame release is checked after that iterator handle is dropped. The channel
receiver remains borrowed and must stay valid through iterator operations and
callbacks; retaining the inner iterator's outer channel across independent
channel destruction requires separate ownership work. Raw segment reclamation,
raw waiter publication/late owner installation, iterator continuation-field
races and other DeferredCoroutine/select typed adapters remain. No full channel,
arbitrary channel destruction, all-race or interop acceptance is claimed.

AsyncTest, test_continuation_dispatch and test_channel_as_flow_smoke pass 3/3
in Debug (1.53s) and 3/3 in optimized ASan (1.22s). Final source hashes and
focused build/test logs are recorded in workspace
automation-artifacts/channel-iterator-adapter-20261005/receipt.json. No
CI/configuration change or push was made.


## Iterator continuation reference race — 2026-10-05

This bounded t_037fc89b slice follows the original nullable iterator continuation
field at BufferedChannel.kt:1605-1613 and its accesses at 1658-1731. The original
marks the field BenignDataRace: a loaded reference stays valid under GC while
another path clears the field. C++ used concurrent ordinary shared_ptr reads,
moves and assignments in invoke_on_cancellation versus send/close completion.
Concurrent access to that same shared_ptr object is a C++ data race.

All post-construction accesses now use C++17 atomic shared-pointer operations.
Completion loads an owning local reference, then separately stores null before
updating receiveResult and resuming. Registration loads an owning local and
invokes only when non-null. Initial and immediate-retrieval field stores are
atomic as well. The original read-then-clear order is retained; no exchange,
new locking protocol, cell algorithm, cancellation callback or memory-management
architecture is introduced. A registration that observes null still does
nothing; a registration that observes the continuation keeps it alive through
its call while completion clears the stored reference.

The existing test_continuation_dispatch target coordinates 200 registration
versus send/normal-close iterations using the existing on_receive_enqueued
override point, after cell and waiter-owner publication. Registration and
completion run concurrently. Either the original immediate-result decision or
one queued dispatcher task is accepted, with exactly one result, correct bool,
next value and completed-frame release. Tests also retain all earlier prompt
cancellation, close, public-handle release and value ownership checks. No target
or separate test harness was added. Debug/ASan concurrency checks do not replace
ThreadSanitizer or establish race freedom for the rest of the channel.

The borrowed outer-channel lifetime is independent of this field fix: the
receiver is kept valid, and independent receiver destruction is not accepted.
A distinct publication gap remains: original cell CAS publishes an owning
reference to its Waiter, while C++ publishes a raw state pointer and installs
the separate waiter_ref afterwards. A concurrent terminal operation can clear
that owner before late registration installs it. The present tests intentionally
start completion after existing owner publication to isolate the continuation
field race; they do not accept or repair that gap. Raw segment reclamation and
other DeferredCoroutine/select typed adapters also remain.

AsyncTest, test_continuation_dispatch and test_channel_as_flow_smoke pass 3/3
in Debug (0.65s) and 3/3 in optimized ASan (0.93s). Final source hashes and
focused build/test logs are in workspace
automation-artifacts/iterator-continuation-race-20261005/receipt.json. No
CI/configuration change or push was made.


## DeferredCoroutine typed await receiver — 2026-10-05

The owning-cell publication refactor remains reserved pending its scope decision.
This independent t_037fc89b repair follows Builders.common.kt:94-101, whose
DeferredCoroutine.await awaits awaitInternal then returns its typed value.
C++ previously exposed the borrowed internal JobState pointer as if it were an
owned T result box, so typed callers could misread or delete the job state.

DeferredCoroutine now uses a retained AwaitValueFrame, matching the already
accepted CompletableDeferred receiving-frame pattern. It retains the deferred
and caller, awaits the existing JobSupport protocol with computed-goto macros,
then extracts CompletedValue<T> and returns an owned T box. Immediate and resumed
paths use the same conversion. Failures clear frame self-retention and propagate
the original cause. No await algorithm, dispatch/cancellation branch, parent
registration, channel cell representation or public signature changes. The
updated await docstring describes C++ result ownership.

The existing test_continuation_dispatch target verifies shared-reference typed
results on suspended success, exact deferred failure, caller cancellation before
completion and prompt cancellation after completion before dispatch. The outer
deferred handle is dropped before delivery; the receiving frame retains it,
then both deferred object and caller frame release after one queued completion.
Result-box copies return to the completion-state reference count after use.
Repeated immediate awaits with an already cancelled caller preserve original
fast-path no-cancellation-check behavior; immediate failure keeps cause identity.
Existing actual async DEFAULT/LAZY/UNDISPATCHED value checks now also consume
typed await boxes. No target or separate harness was added.

AsyncTest, test_continuation_dispatch and test_channel_as_flow_smoke pass 3/3
in Debug (0.76s) and 3/3 in optimized ASan (1.32s). One exploratory dispatch run
timed out after the new deferred checks, in the existing iterator concurrency
check. Its source cause was not established; a direct run and 35 diagnostic
repetitions passed, followed by the final focused suites. The observation is
recorded explicitly rather than treated as race-freedom evidence. No
ThreadSanitizer run or all-channel race acceptance is claimed. Logs, diagnostic
outputs and source hashes are in workspace
automation-artifacts/deferred-coroutine-await-20261005/receipt.json.

Underlying JobSupport completion state is a raw atomic JobState pointer and
its default destructor does not reclaim that allocated state. This slice checks
release of additional result-box references, not complete reclamation of the
underlying completion payload. That state-reclamation gap is separate from this
receiving adapter. Select typed receiving adapters, borrowed channel receiver
lifetime, raw segment reclamation and raw waiter publication/late owner
installation remain. No CI/configuration change or push was made.
