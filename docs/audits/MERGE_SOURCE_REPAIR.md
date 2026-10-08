# Merge source translation and private suspension bodies

## Fold/reduce/chunk source continuation — 2026-10-07

7eb83ee8 continues the complete Transform.kt translation at Transform.hpp:704,
799,893. The handwritten RunningFoldFrame, RunningReduceFrame, ChunkedFrame and
nested emission frames are replaced with typed source collectors and annotated
collection/emission bodies. Transform now contains no handwritten frame classes
or coroutine macros. All remaining source operations are expressed directly:
initial emission precedes fold collection, the reduction's first element bypasses
the operation, full chunks reset after successful emission, and a partial chunk
is emitted only after normal upstream completion. Each collection has its own
accumulator/buffer; the supplied operation has an actual shared owner. Nullable
reduction elements remain distinct from the uninitialized sentinel. The receivers
retain their existing owner, downstream collectors remain borrowed, and operation
result boxes are owned and deleted before downstream emission. Existing public
callable projections remain. KDoc examples and source provenance are updated.
This is unfinished source authoring, not complete Transform/source/API parity.

7687daed repairs NativeSuspendLowering.cpp:994 to emit a grouped static local
declaration once, preserving normal C++ static storage rather than emitting the
whole declaration repeatedly for each variable. The existing qualified_locals
fixture now checks two grouped statics, stable identities across calls, and
started/completed counters on normal completion and failure/cancellation paths.
The existing plugin reproduces duplicate started/finished definitions in the
actual generated helper source. That receipt establishes the old defect; it does
not prove execution of the repaired source.

Fresh strict compilation of the actual test_transform_suspension.cpp exits 1.
The new fold/reduce/chunk collector entries are discovered; instantiated local
methods still fail namespace-context integration alongside generated/dependency
diagnostics. The actual qualified fixture also exits 1, including the duplicate
statics and GNU address-of-label diagnostic from the old plugin. Fresh plugin
build exits 2 in KotlinxClassMetadataPlugin's LLVM/Clang unused-parameter warnings.
No warnings are relaxed and no fresh executable/lifetime result is established.
Receipts under build/ir-recovery: transform-fold-reduce-chunk-source-consumer.log,
qualified-grouped-static-source-consumer.log and grouped-static-source-plugins.log.
Neither complete ordinary C++/MLX nor real Native GPU handoff acceptance is proved.

Both exact full-root scans exit 0 with no simultaneous source edits; refreshed
reports are in 5345dbb0. Receipts use
transform-fold-reduce-chunk-{library,compiler}-deep.log. Transform remains 12/13
matched bodies at similarity 0.07, target inventory 61 bodies and 7 types.
Library totals remain 831/2918 bodies and 359/560 types at 0.26, with 123 scoring
failures. Compiler totals remain 592/7657 bodies and 174/1727 types at 0.36, with
24 failures. All 52 ranged Transform and 32 ranged lowering references resolve;
the changed source and fixture have no prohibited markers. The full goal remains
active; name matching and source annotations do not establish completion.

## Indexed/action source and spill qualification continuation — 2026-10-07

8a49a5c5 continues the full Transform.kt translation at Transform.hpp:544,590.
with_index's source collector now holds its per-collection index and tail emits
the checked index/value; its annotated collect entry retains the actual collector
owner through upstream collection. The prior WithIndexFrame continuation and
manual label/self-cycle are removed. on_each now owns the supplied action and
expresses the source action-before-emit sequence with annotated suspension;
OnEachFrame and its manual spill/label/cycle are removed. Ordinary and suspending
callable projections retain the existing API. Other Transform manual bodies remain.

The consumed CoroutinesVarSpillingLowering.kt:53-66 assigns each spill field the
variable's declared type. 7b70ebcf repairs NativeSuspendLowering.cpp:483 to retain
C++ const/volatile qualification, including concrete and dependent field types,
instead of stripping it and changing overload resolution when reparsing the body.
fd57ae36 records whether an authored continue targets each lowered loop and emits
its increment/condition target only when referenced (:1248,1275,1303,1337).
The new no-continue loop fixture exposed the old unused-label diagnostic; this
repair preserves normal fallthrough rather than relaxing warnings.

0c735ac3 adds qualified_locals.cpp and registers it in the existing executable
plugin harness. It checks distinct cv-qualified overloads, three immovable
ordinary objects, stable addresses over two suspensions, and destruction after
completion, immediate/resumed failure and a resumed CancellationException.
Those are test requirements, not successful executable evidence. Python harness
syntax parses; a standalone strict standard-library probe accepts the required
optional<const/volatile Value> emplacement/reset storage. Neither establishes
compiler lowering or either complete MLX acceptance path.

Actual test_transform_suspension.cpp compilation exits 1. The new qualified
fixture also exits 1 with the existing plugin, which reports generated GNU
address-of-label and unused-loop-label diagnostics. A fresh final plugin build
exits 2 in KotlinxClassMetadataPlugin on LLVM/Clang unused-parameter diagnostics,
so the cv and loop-target repairs have not run in a fresh plugin. Strict warning
policy is unchanged. Receipts under build/ir-recovery:
transform-index-action-source-consumer.log, qualified-locals-source-consumer.log,
qualified-locals-source-plugins.log and qualified-locals-final-plugins.log.
Generic local-class/lambda declaration integration remains incomplete.

Both exact full-root scans finish after the final source change, exit 0, and make
no further report changes relative to d73e201c. No source edits run during either
scan. Final receipts are qualified-locals-final-{library,compiler}-deep.log.
Transform remains 12/13 bodies at similarity 0.07, with target inventory 76 bodies
and 8 types. Library totals are 831/2918 bodies and 359/560 types at 0.26, with
123 scoring failures; compiler totals are 592/7657 bodies and 174/1727 types at
0.36, with 24 failures. All 33 ranged Transform and 32 ranged lowering references
resolve; the edited source and new fixture have no prohibited markers. The full
translation goal remains active, with no fresh runtime acceptance claim.

## Predicate and nullable-map source continuation — 2026-10-07

825918fa and 07e682b0 continue the actual Transform.kt translation after reading
the complete source and existing C++ file. Transform.hpp:170,220,452 now express
filter, filter_not and optional map_not_null as annotated predicate/transform,
owning result unboxing and conditional downstream emission. Their handwritten
FilterFrame, FilterNotFrame and MapNotNullFrame classes, labels and self-retention
cycles are removed. Supplied callables have owned storage, completion uses the
existing retaining boundary, and collectors remain borrowed. Nullable results
use move construction rather than imposing a new move-assignment requirement.
Source KDoc and overload provenance replace stale manual-frame explanations.
Other Transform handwritten frames and source/API gaps remain.

The consumed lowering review compared declaration context handling with
LocalDeclarationPopupLowering.kt and exception/spill handling with the Native
lowering prerequisites. CompilerFrameLowering.cpp still rejects instantiated
local classes/lambdas because its typed helper requires namespace context.
C++ local classes and closure types cannot be repaired merely by dropping that
check. Commit 10da6bc6 initializes the generated handler's saved previous
exception explicitly at NativeSuspendLowering.cpp:1140. This source fix has not
been exercised by a freshly built plugin.

The fresh actual test_transform_suspension.cpp check exits 1 with strict warning
flags and the existing plugins. New lambdas at Transform.hpp:173,223,456 are
recognized, but namespace-context integration, generated GNU labels and
source/dependency diagnostics remain. The old plugin still emits the missing
previous-field initializer diagnostic. A fresh plugin build exits 2 in
KotlinxClassMetadataPlugin on LLVM/Clang unused-parameter diagnostics. No warning
policy is relaxed and no successful runtime/lifetime evidence is asserted.
Receipts: transform-filter-source-consumer.log and
transform-filter-source-plugins.log under build/ir-recovery.

CMake's Native-disabled configuration exits 0; its 49-test inventory retains
kxs_plugin_handoff and excludes kxs_kotlin_native_handoff. The public interface
still links the actual translated core; production authoring remains in the
Clang frontend and LLVM module plugin. Receipts use
transform-filter-source-{configure,test-inventory}.log. Neither full ordinary
C++/MLX execution nor the real Native shared-state-machine GPU boundary is
established.

Both exact full-root scans exit 0 after source commits, without concurrent source
edits. Reports are committed in 11232702; receipts use
transform-filter-source-{library,compiler}-deep.log. Transform stays 12/13
matched bodies and similarity 0.07, with target 82 bodies and 9 types (previously
94 and 12). Library totals remain 831/2918 bodies,359/560 types, similarity 0.26
and 123 scoring failures. Compiler totals remain 592/7657 bodies,174/1727 types,
similarity 0.36 and 24 failures. Both edited source files have no prohibited
markers. The full transliteration goal remains active.

## Consumed map dependency continuation — 2026-10-07

06da4bbf continues the source repair into the actual map dependency after reading
the full pinned Transform.kt, Emitters.kt:44-51 and the existing typed bodies.
Transform.hpp:124 replaces unsafe_transform's manual CollectFrame with its
source collector binding and an annotated collection body at :140. It keeps
upstream ownership and a borrowed downstream collector. Transform.hpp:472
replaces MapFrame with the source transform-then-emit lambda: the supplied
callable is moved into owned storage, the result is received as an owning R box,
and that box is deleted before the source emit. The raw completion projection
retains the actual supplied continuation. No manual label, spill or frame cycle
remains in these two bodies; other Transform bodies still use handwritten frames.

Fresh strict actual test_transform_suspension.cpp and public Merge overload
probe checks both exit 1 using the existing plugins. The instantiated
TransformCollector member and map lambda report the same missing namespace
context integration; generated template/frame and dependency diagnostics also
remain. Receipts are merge-map-source-{consumer,instantiation}.log. The full
Native-disabled core build exits 2 on its plugin dependency's LLVM/Clang header
diagnostics (source-continuation-core-build.log). No fresh executable, capture
retention, terminal cleanup or MLX acceptance result is established.

Both final full-root deep scans exit 0, with no concurrent source changes;
reports are in 9c1a935f and receipts use merge-map-source-{library,compiler}-deep.log.
Transform stays 12/13 matched bodies at similarity 0.07, with target inventory
falling from 100 to 94 bodies and 13 to 12 types. Public/internal Merge retain
the measurements below. Library totals remain 831/2918 bodies, 359/560 types,
similarity 0.26 and 123 scoring failures. The 22 ranged Transform references
resolve and the file contains no prohibited markers. These are measured partial
source changes, not completed translation or successful runtime validation.

## Continued transliteration and compiler integration — 2026-10-07

Source commits d573b064, c9f3038b and 2cbadce9 follow the checkpoint below.
Internal Merge.hpp:76 now directly cancels/joins before starting the next
UNDISPATCHED transform. Its private typed MergeCollector at :200 directly checks
Job cancellation, acquires the semaphore and launches a child whose annotated
lambda at :232 contains collection and permit cleanup. The generic binding is
outside the suspend body, reflecting Native lowering's local-declaration
prerequisite; no handwritten frame/label/spill logic is added. Existing .cpp
callable entries remain consumed by older helper regressions, but the production
collectors no longer call them. The scoped collection lambda is annotated with
C++20-compatible GNU function-attribute syntax.

Public flow/Merge.hpp:66 replaces flatten_concat's two handwritten frames with
the source unsafe_flow/collect/emit_all body. :122,132,148,156 call the real map
then flatten_concat/flatten_merge, replacing duplicated stack MapCollectors.
Suspending flat-map overloads and both default-concurrency projections exist.
:184,193 expose raw/owned transform_latest continuation bindings. :213,251
translate map_latest and flat_map_latest with suspending transforms followed by
actual emit/emit_all. Their receiving lambda owns and deletes the result box
before the next suspension. Ordinary C++ transforms remain supported by boxing
their actual result. DEFAULT_CONCURRENCY is immutable as the source val is.
Public Merge.cpp now has canonical source provenance and its actual header path.

The IR/CMake review read NativeSuspendFunctionLowering.kt,
CoroutinesVarSpillingLowering.kt, AbstractSuspendFunctionsLowering.kt, the local
declaration prerequisite and IrToBitcode.kt:2281-2340 alongside the consumed
frontend, frame importer, LLVM injector and CMake modules. The pipeline remains
Clang frontend plus mandatory in-compiler LLVM address injection, with persistent
label, blockaddress/indirectbr and separate normal/resumed Result handling.
Generic lambda/frame declaration integration and C++ closure lifetime coverage
are incomplete; source annotations do not establish capture retention.

CMake commit 4bc60d81 links the public package interface to the translated core,
exports the advertised coroutines/coroutines_headers names, imports the core and
Threads dependency in the installed config, and registers the real Native
handoff only when KOTLIN_NATIVE_RUNTIME_AVAILABLE is explicitly enabled.
build/source-continuation configures with that option OFF and the selected LLVM
23.1.2 compiler/package. Generated export metadata links kotlinx::coroutines to
kotlinx::kotlinx-coroutines-core. Its test inventory retains kxs_plugin_handoff
and excludes the real Native handoff and GC boundary suites. Configuration is
not an executable build or complete installed-package execution check.

Fresh builds of KotlinxSuspendPlugin and KotlinxCoroutinePass both exit 2 on
LLVM/Clang dependency unused-parameter diagnostics under the existing strict
warning policy. No warning suppression or alternate compiler/runtime was added.
Receipts: source-continuation-plugins-build.log and source-continuation-ir-build.log
under build/ir-recovery. Configuration receipts use source-continuation-.

Strict checks using the existing plugins of the actual channel consumer, actual
test_transform_suspension.cpp and the concrete internal/public operator probes
each exit 1. The earlier local-declaration and non-coroutine scoped-lambda
diagnostics are absent from the current internal probe; generated template T,
frame parsing, GNU label diagnostics and instantiated lambda namespace-context
failures remain. Receipts: merge-source-bodies-{consumer,instantiation}.log and
merge-operator-source-{consumer,instantiation}.log. The public probe exercises
ordinary/suspending concat, merge with default/explicit concurrency, map_latest
and flat_map_latest. No fresh runtime or either full MLX acceptance path is
established, including retained identities/cleanup under resumed cancellation.

Both exact full-root deep scans exit 0 without simultaneous source edits.
Reports are committed in 26378eb2; receipts are
merge-operator-source-{library,compiler}-deep.log. Internal Merge remains 9/9
bodies, 3/3 types, with similarity 0.27. Public Merge is 8/9 bodies with similarity
0.10; the Iterable.merge contract is still reported missing against its vector
projection. Broader iterable identity, docs/examples and map dependency source
translation remain incomplete. Library totals remain 831/2918 bodies,
359/560 types, similarity 0.26 and 123 scoring failures. No analyzer criteria
were modified. All 29 internal and 20 public ranged source references resolve;
these three changed source files contain no prohibited markers.

## Current source-authoring checkpoint — 2026-10-07

The tree was dirty on entry after the ChannelFlow handoff. Commit **53c62d23**
preserves those unfinished changes before further editing; **3194132a** aligns
the helper's immutable captures and the omitted source UNDISPATCHED comment.
The complete pinned Merge.kt and ChannelFlow.kt and their C++ pairs were read.
The historical executable evidence below predates this migration.

All four Merge consumers now use their actual source operations. Transform
children call transform directly at Merge.hpp:86; limited-merge children call
flow.collect directly at :271. TransformLatestCollector::collect at :62 is an
annotated owning entry that retains the actual collector through its upstream
collection. ChannelFlowTransformLatest::flow_collect at :127 directly suspends
the existing scoped collection; ChannelFlowMerge::collect_to at :174 directly
suspends collection with its retained MergeCollector. These member entries retain
an existing shared receiver owner; raw scopes and downstream collectors remain
borrowed. Production src now contains no collect_channel_flow references.
Its declaration and body were removed from ChannelFlow.hpp/.cpp only after the
last consumers were translated.

Merge.cpp:22,38 replace MergeEmitContinuation and MergeChildContinuation with
annotated source bodies. The first checks the optional Job, suspends semaphore
acquisition and launches the child. The second suspends actual collection and
releases the permit on success or exception. The duplicated catch/success release
expresses Kotlin's finally at source :64-68 in C++; compiler lowering must
preserve it across resumption. No handwritten label, spill field or retention
cycle remains in this pair. Existing typed callable adaptations
transform_latest_emit, acquire_and_launch_merge_inner and collect_merge_child
remain; removing one adapter does not establish complete body correspondence.
All three source merge classes are final. Source val members and immutable
lambda captures are const, while previous_flow remains mutable.

Strict compilation of actual Merge.cpp, actual test_channel_consumption.cpp and
explicit instantiations of ChannelFlowTransformLatest<int,int>,
ChannelFlowMerge<int> and ChannelLimitedFlowMerge<int> each exits **1**. Commands
use the existing LLVM/Clang plugins with -std=c++20 -Wall -Wextra -Wpedantic
-Werror -ferror-limit=0; no warnings were suppressed and no plugin was rebuilt.
Receipts under build/ir-recovery are merge-direct-source.log,
merge-direct-consumer.log and merge-direct-instantiation.log. The concrete probe
is tmp/merge-direct-instantiation.cpp. Include roots are include,
src/kotlinx/coroutines and src; plugin wiring follows the
[LLVM code-generation build contract](../architecture/llvm_codegen_contracts.md#build-contract).

The frontend discovers the annotated entries. Actual consumer and instantiation
diagnostics include "suspend local declaration is not a variable" for the local
MergeCollector class in collect_to. The instantiation also reports "suspension
functions can only be called within coroutine body" on its scoped lambda's
annotated member call. Generated GNU address-of-label code, missing exception
context previous initialization, generated-frame parsing failures, and dependency
unused parameters also prevent compilation. transform_latest_emit's authoring
continuation produces an unused-parameter diagnostic in the actual source check.
These are recorded compiler/source integration gaps, not successful lowering.
No fresh executable verifies owner/resource retention, destruction, repeated
suspension, resumed failure/cancellation or permit release. Both complete
standalone/Native MLX GPU acceptance paths remain unverified.

merge-direct-source-references.json records valid bounds for 29 Merge.hpp,
3 Merge.cpp, 49 ChannelFlow.hpp and 11 ChannelFlow.cpp ranged references, with
no prohibited markers in those four files. This is reference evidence only.
Both exact full-root ast_distance --deep commands complete with exit **0**,
without source edits while scanning. Receipts are merge-direct-library-deep.log
and merge-direct-compiler-deep.log; generated library evidence is committed in
**b9d8d99a**. Merge remains 9/9 matched bodies and 3/3 source types with body
similarity **0.26**; target inventory drops from 30 to 24 bodies and 8 to 6 types.
ChannelFlow remains 18/19 bodies, 6/6 types and similarity **0.25**, with 38 target
bodies. Library totals remain 831/2918 bodies, 359/560 types, similarity 0.26,
documentation similarity 0.38 and 123 scoring failures. Compiler totals are
592/7657 bodies, 174/1727 types, similarity 0.36 and 24 scoring failures; that
report is unchanged by this batch. No analyzer criteria were changed. Missing,
provisional and failed criteria remain authoritative for further source repair.

## Historical source and executable evidence

Date: 2026-10-07. Continuing the leading Flow/channel dependency closure under Kanban t_8700df29 and source umbrella t_1834dcec. The complete pinned flow/internal/Merge.kt and both existing C++ files were read before edits. Worktree checkpoint was clean 207f357f; every source/test/report batch was committed before the next edit.

## Implemented source correspondence

- Merge.hpp:158,248 now mirror Merge.kt:47-49,85-87 through the actual inherited get_collect_to_fun property. The duplicated raw-this producer lambdas are removed. The source public produce overload supplies DEFAULT and SUSPEND, including when the flow overflow property differs. Existing shared receivers survive queued dispatch; borrowed receivers remain borrowed.
- Merge.cpp:90-99 now translates source :23-33 as an annotated suspend function: cancel the previous Job with ChildCancelledException, call previous_flow->join(), then launch the next transform. The compiler generates the retained frame and resume dispatch; the handwritten TransformLatestEmitContinuation is removed. Merge.hpp:53 keeps only the required typed collector/value bindings. Its emit uses retain_continuation for the actual caller, replacing the old unconditionally borrowed completion alias. The next child still starts UNDISPATCHED.
- Merge.cpp:11 translates source :55-70 as a concrete private MergeEmitContinuation: check the actual optional Job, acquire the semaphore, then invoke the source child launch. The typed collector at Merge.hpp:169 holds the real flow, scope, semaphore, SendingCollector and Job. It binds the concrete algorithm; it contains no separate suspension labels or resume dispatch.
- Merge.cpp:49 translates source :64-68 as a concrete private MergeChildContinuation: collect the actual inner flow and release exactly one semaphore permit in the source finally, on immediate or resumed completion/failure. Terminal release clears collector/flow/semaphore captures even when another owner retains the completed frame.
- The single-call outer collection, transform child and limited-merge child bodies use the existing concrete collect_channel_flow implementation. Their duplicated generic continuation classes are removed from Merge.hpp. The existing collect_in_scope still waits for actual children. Mandatory address injection and Native ContinuationImpl supply the current Continuation ABI; no alternate state machine or decision protocol was added.
- Source generic callable/value bindings remain in the header because consumers instantiate the actual element types. Concrete frame dependencies are private to Merge.cpp. The create overrides inherit protected visibility from ChannelFlow, while produce_impl remains public. The source constructor/defaults, cancellation check, UNDISPATCHED/DEFAULT choices and BUFFERED capacities are preserved.

## Executed evidence

Before-control 9adb53da compiles and exits 1 at test_channel_consumption.cpp:1157: releasing the merge owner before queued start also releases the receiver and its captured resource. Receipt: build/ir-recovery/merge-lambda-before-tests.log. d162c034 restores the source collectToFun capture.

The production translation is in b1aac0ee and 5f0fe2a3. The expanded fixture at test_channel_consumption.cpp:1107 covers both merge producer overrides through queued start, normal rendezvous collection, cancellation before start and cancellation during send, source default overflow behavior and receiver/resource release. The fixture at :1180 uses real producer-Job join and semaphore acquisition, checks that the next launch waits, and executes child finally for immediate/resumed success/failure. Holding the completed child frame externally does not retain its source captures. An explicit-bool typo in the added regression was corrected in 33ce1c83; it was a test compilation issue, not a production defect.

The complete core and seven focused test targets build. All seven CTest executables complete with zero failures in 2.07 seconds. Receipts: build/ir-recovery/merge-final-build.log and merge-final-tests.log. The actual Merge.cpp, Produce.cpp, ChannelFlow.cpp and common/internal/OnUndeliveredElement.cpp and expanded fixture also compile and execute under AddressSanitizer/UndefinedBehaviorSanitizer with stack-use-after-return detection and no diagnostics. The fresh dependency archive is not wholly instrumented. Receipts: merge-sanitizer-build.log and merge-sanitizer-tests.log.

All 44 ranged provenance references in the Merge pair resolve to existing Kotlin source bounds; neither source file contains prohibited comment markers. Bounds alone do not establish body equivalence.

Both full-root deep scans complete with exit zero. Current library evidence: 820/2918 matched functions, 359/560 types, average body similarity 0.26 and 123 scoring failures. Merge remains 9/9 function names and 3/3 source types; its measured body correspondence increases from 0.20 to 0.26. Compiler/prerequisite evidence remains 591/7657 functions, 174/1727 types, body similarity 0.36 and 24 scoring failures. Receipts: merge-library-deep.log and merge-compiler-deep.log. Generated reports are committed in f64b10c0; no analyzer criteria were changed.

The whole library translation is still incomplete. This source repair does not establish complete automatic compiler spilling or actual shared Native/C++ frame layout, and the required standalone/Native MLX GPU acceptance paths remain unverified. The source goal and Kanban cards remain open.

## Kotlin-style join authoring repair

Production commits b3b58afa and 51cc0e99 expose the source Job.kt:288 authoring declaration at Job.hpp:278 and the owned continuation ABI adapter at Job.cpp:22. The no-argument declaration is consumed by the existing implicit-continuation lowering and has no runtime fallback. An ordinary unlowered call is rejected during code generation. The adapter forwards the generated frame to the existing virtual raw-continuation join. CMake already enables both frontend lowering and mandatory LLVM injection for the core target.

The first production build fails with incomplete smart-pointer receiver syntax in join-authoring-build.log. The consumed non-tail member call exposed an implicit operator-> extraction defect. NativeSuspendLowering.cpp now emits the actual resolved arrow operator, retains its pointer result before suspension, and replaces only the receiver range before the final arrow token. Discarded expressions explicitly discard their values, preserving Unit statement behavior without nodiscard errors.

The expanded source regression in f0f00a48 exercises an absent/complete previous Job, immediate next-transform failure, real producer-Job suspension, and caller cancellation before/during the wait. Cancellation never launches the next transform; completing the previous Job after caller cancellation never duplicates completion. The complete core and seven focused targets build, and all seven CTest executables finish with zero failures in 1.79 seconds. Receipts: join-authoring-repaired-build.log, join-focused-build.log and join-focused-tests.log under build/ir-recovery. The changed concrete source helpers and fixture execute under AddressSanitizer/UndefinedBehaviorSanitizer with stack-use-after-return detection and no diagnostics; the dependency archive is not wholly instrumented (join-sanitizer-build.log and join-sanitizer-tests.log).

The actual production LLVM output join-authoring.ll contains the compiler-generated transform_latest_emit continuation frame, stored blockaddress, indirectbr dispatch, and owned-continuation Job.join call. Both initial/resumed result extraction sites are present; no unresolved injection-marker call remains. The unlowered-call negative control fails with the intended join authoring diagnostic (join-outside-suspend.log). The real Kotlin/Native handoff regression completes with zero failures in 13.80 seconds (join-native-handoff-tests.log). This verifies that existing boundary, not the required full shared-frame/MLX GPU acceptance paths.

Both full-root deep scans finish with exit zero after the source changes (join-library-deep.log and join-compiler-deep.log). Library measurements remain 820/2918 functions, 359/560 types, body similarity 0.26 and 123 scoring failures. Compiler/prerequisite measurements remain 591/7657 functions, 174/1727 types, body similarity 0.36 and 24 scoring failures. Refreshed generated evidence is in 22dd2d7d. The library and full compiler translation remain incomplete.

### Final compiler integration verification

CompilerFrameLowering.cpp:120 completes the host's pending instantiations with parsing-phase semantics before importing its frame. The broader compiler regression originally fails inside libc++ std::variant; the same failure reproduces with the pre-change plugin (join-plugin-before/retained-build.log). Completing host instantiation resolves that failure. CompilerFrameLowering.cpp:157 restricts main-file namespace-scope helpers to declarations already reached by the parser and preserves shorthand namespace brace structure. The small late-include control fails with the old plugin's premature header import/NameConflict and succeeds after the scope repair. The permanent regression lives in test_suspend_plugin.py:30. Reused imported definitions also retain an already-deduced return type with their body at CompilerFrameLowering.cpp:450. Two unsuccessful declaration guards were removed rather than left in production.

The retained-local fixture's Done originally catches only std::runtime_error and therefore misses the actual translated CancellationException. Commit 15e1d252 catches CancellationException explicitly and verifies that exact exception type in the cancelled delay branch, preserving the failed/completion/resource checks. This fixes the test's exception-ancestry assumption rather than changing production cancellation.

Final core and seven focused targets build with exit zero (join-final-core-build.log). All seven library CTest executables finish with zero failures in 2.02 seconds (join-final-library-tests.log). All four compiler/interoperability executables finish with zero failures in 309.17 seconds (join-final-plugin-tests.log): analyzer liveness, tail-call collection, the full generated/CMake retained-local application in ordinary and forced-include builds, and the actual Kotlin/Native handoff. The compiler application also exercises sanitizer checks, repeated suspension, cancellation, source receiver/reference identity, exception cleanup and noexcept termination. The changed concrete library helpers and expanded channel fixture separately execute under AddressSanitizer/UndefinedBehaviorSanitizer with stack-use-after-return detection and no diagnostics (join-final-sanitizer-build.log and join-final-sanitizer-tests.log). The dependency archive remains only partly instrumented.

Final production IR is join-final-authoring.ll: the generated transform_latest_emit frame stores a blockaddress, dispatches with indirectbr, forwards its actual owned continuation to Job.join, and extracts initial/resumed results. There are zero unresolved injection-marker calls. Receipts: join-final-ir-build.log and the actual LLVM file under build/ir-recovery. Both final full-root deep scans complete with exit zero after the final source/compiler changes (join-final-library-deep.log and join-final-compiler-deep.log); they retain the measured totals above. Kanban t_8700df29 and the full-source goal remain open. The required complete standalone/Native shared-frame MLX GPU demonstrations remain unverified.
