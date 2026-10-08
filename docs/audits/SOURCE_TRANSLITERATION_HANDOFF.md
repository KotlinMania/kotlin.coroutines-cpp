# Source transliteration handoff — 2026-10-08

This is the current continuation handoff, updated at Sydney's explicit request
following an interrupted ChannelFlow source-edit turn. Read this document and
inspect the current worktree before continuing. Older versions remain in Git.

## Continuation update after the handoff

## Native timing implementation — 2026-10-08

Continuation from `16c4a550` translates all six Native Timing.kt functions in
src/kotlin/system/Timing.hpp/.cpp. The actual Porting.cpp steady-clock selection
and duration casts implement the three external source getters for standalone
C++. Inline measurements retain actual callable types, one invocation and wrapped
Long subtraction. No Kotlin runtime exports/checker guards or substitute runtime
are provided. Native's real GCUnsafeCall boundary remains a separately linked
requirement. The actual get_time_nanos dependency is now available for the pending
MonotonicTimeSource translation. Source and assertion-enabled test are registered.

Strict Clang/O1 ASan/UBSan execution exit0 for100 cross-unit clock brackets,
single block invocation, move-only capture and original exception identity.
otool shows libc++, libSystem and ASan only. Fresh relevant deep scan exits0:
Native reference root3/1234 bodies,1/258 types,0.67 aggregate similarity; Timing
3/3 source bodies,0.34, all six symbols PRESENT,zero scoring failures. External
getters are declarations in Kotlin, not three additional source bodies.
See NATIVE_WORKER_DISPATCHER_SOURCE_REPAIR.md for exact scope and remaining gaps.
Continue Monotonic/ValueTimeMark/generated object boundary, Worker/Future and
actual compiler lowering. Both complete docking-ring/MLX paths remain unverified.

## Comparable marks and source interfaces — 2026-10-08

Continuation from `5312f3a0` translates Native Comparable.kt's public interface,
ComparableTimeMark's abstract API and concrete minus/compare defaults, and the
abstract TimeSource/WithComparableMarks surfaces. Covariant pointer returns retain
the caller-owned delete/adopt contract. Nullable equality refers to the real Any
contract without imposing compiler-owned object representation on ordinary C++.
The value-mark generated equality/hash/text and actual Kotlin boundary remain open.
The actual default source and assertion-enabled regression are registered in CMake.
Strict Clang and O1 ASan/UBSan covariance/ordering/error-propagation execution exit0.

Fresh relevant deep scans exit0: time14/44 bodies,4/13 types,0.50; TimeSource
pair7/18,4/7,0.36. Native runtime reference root against src/kotlin is0/1234 bodies,
1/258 types. Comparable1/1 types,0/0 bodies is an abstract surface measurement.
Both scans have zero scoring failures; existing method scope mismatches remain.
See NATIVE_WORKER_DISPATCHER_SOURCE_REPAIR.md for evidence and limitations.
Continue Monotonic/ValueTimeMark, actual timing/object boundaries and Worker/Future.
The worker's provisional monotonic expression is not compilable yet. Complete
compiler/library and both MLX paths remain unfinished; keep the full goal active.

## TimeMark defaults — 2026-10-08

Continuation from `3ac53464` adds TimeMark.hpp/.cpp from TimeSource.kt:128-195,
246-250. Public defaults live in .cpp with the private AdjustedTimeMark. Further
adjustments combine against the original mark. Returned raw pointers are
caller-owned (delete/adopt); this permits later covariant source overrides.
Original shared owners are retained; raw/stack marks stay borrowed and must
outlive adjustments. The library source and assertion-enabled test are registered.
Strict Clang compilation and O1 ASan/UBSan ownership/adjustment execution exit0.

Current time-root deep scan:12/44 bodies,1/13 types,0.46 similarity,zero scoring
failures. TimeSource pair5/18,1/7,0.27. Body/symbol scope disagreements for base
TimeMark methods and AdjustedTimeMark::plus are recorded in
NATIVE_WORKER_DISPATCHER_SOURCE_REPAIR.md; they do not certify whole-time parity.
Continue the actual comparable/value-mark API, TimeSource and Native monotonic
operations, then Worker/Future. The provisional worker value-mark call is not
yet reconciled. Full compiler/library and standalone/Native MLX remain unfinished.

## Native monotonic arithmetic dependency — 2026-10-08

Continuation from `39965bab` adds the actual eight longSaturatedMath.kt operations
in src/kotlin/time/LongSaturatedMath.hpp/.cpp, including half-duration addition,
infinity checks and overflowing finite differences. Private helpers live in .cpp;
unsigned wrapping preserves Kotlin Long behavior without C++ signed overflow.
The actual library source and assertion-enabled test are registered in CMake.
Strict O0/O1 ASan/UBSan compilation and boundary execution both exit0.

The three pinned time references were restored unchanged from tmp/kotlin fee29910.
Fresh deep scans exit0: time root7/44 bodies,0/13 types,0.64 similarity; selected
math pair7/8, with isSaturated PRESENT in symbol inventory but missing in body
pairing because of its primitive extension receiver. Unsupported Kotlin infix/if
emission remains in evidence. Compiler286/7207 bodies,132/1630 types,0.28 with10
scoring failures; frontend14/7207 bodies,6/1630 types,0.04 with zero failures.
The denominator grows because three genuine sparse references are now visible.

Continue the actual TimeSource/TimeMark and Native monotonic source translation,
then Worker/Future/runtime dependencies; none is supplied by the arithmetic test.
Reconcile WorkerDispatcher's provisional ValueTimeMark authoring expression with
that actual translated API. Complete standalone/Native MLX and shared-frame
acceptance remains unverified. See NATIVE_WORKER_DISPATCHER_SOURCE_REPAIR.md.

## Native WorkerDispatcher source body — 2026-10-08

Continuation from `c5c168f3` replaces the empty native/MultithreadedDispatchers.cpp
with WorkerDispatcher's source constructor, dispatch, delay, timeout, schedule and
close operations from MultithreadedDispatchers.kt:20-76. The new Native header holds
the interface and opaque private field; private implementation stays in .cpp.
DisposableBlock drops its atomic runnable owner on disposal. The recursive delay
operation preserves the source monotonic mark,100ms quantum, microsecond conversion,
disposal check and final nonnegative executeAfter wait. Cancellation owns the actual
handle; resumption uses resume_undispatched. Existing owning continuations retain
their owner; raw receivers remain borrowed. The scheduled once-only callback moves
its lease into invocation to avoid a completed callback/cancellation-handle cycle.
No lifetime result is established yet.

native/CoroutineContext.cpp now delegates DefaultExecutor to this source class.
Its detached timers and non-source early-resume/synchronous branches are removed.
Delay.hpp no longer pulls private continuation implementation and intrinsic headers
into its declaration-only interface. Strict Clang syntax passes the actual new
header and rewritten Native context. The WorkerDispatcher implementation fails on
the missing Worker.hpp and two existing CancellableContinuationImpl unused-parameter
diagnostics. Worker/Future and TimeSource/TimeMark production dependencies remain
untranslated; no fake Worker, clock, runtime or helper test replaces them.

Worker/Future sources were read in tmp/kotlin. TimeSource.kt is tracked in its pinned
fee29910 Git tree but absent from the sparse working tree; git show supplied the actual
contract. Continue those dependency translations, including actual runtime consumers.
MultiWorkerDispatcher and the Native fixed-pool factory remain incomplete; the common
thread-pool implementation is not a source replacement. Do not claim complete Native
dispatcher or retained-fixture execution. Both complete Native/standalone MLX paths
and full compiler/library translation remain open.

The refreshed library deep scan records670/2918 bodies,181/560 types,0.24 similarity,
11 scoring failures. Native MultithreadedDispatchers:5/19 bodies,2/3 types,0.09;
local helpers still have lexical-scope matching gaps. Native CoroutineContext pairs
with UndispatchedCoroutine, so its9/10 bodies,2/2 types,0.39 do not certify DefaultExecutor.
See NATIVE_WORKER_DISPATCHER_SOURCE_REPAIR.md for source references and verification.

## Function-address dependency closure — 2026-10-08

Continuation from `b28cd6d6` repairs the missing libc++ variant helper dependency
in CompilerFrameLowering.cpp:125. The actual visitor previously followed direct
calls and constructor/destructor targets only. It now includes referenced function
addresses and follows the actual initializer of each referenced variable once,
using canonical Clang declaration identity. Unevaluated and decltype operands stay
excluded. This is Clang integration infrastructure; no new Kotlin IR identity,
variant/Result runtime, application ownership rule or state machine was substituted.
Compared with IrToBitcode.kt:2265-2276, function targets remain the actual resolved
declaration rather than a reconstructed name or shape.

The rebuilt LLVM23.1.2 frontend compiles original retained input.cpp at O1 with the
mandatory LLVM pass. Its object has no undefined variant dispatch helper references.
The new function_address_dependencies compiler fixture compiles strictly and runs
under ASan/UBSan at O0 and O1. Five modes check function-pointer tables, both variant
alternatives, same catch/current-exception identity across repeated suspension,
failure/cancellation, completion once, owned-object destruction and frame release.
Both success modes return46. Both optimization levels are registered in the driver;
the final O0/O1 driver block also executes successfully. The full driver passes its
earlier runtime gates and the larger retained CMake extraction/compilation. Linking
now fails only on missing actual Yield/Delay/Duration definitions; the variant helper
failure is gone. Twelve retained compilation warnings remain unsuppressed. No larger
retained runtime or later forced-include/termination/rejection gate is claimed.
This focused execution does not establish the complete retained fixture or either
complete standalone/Native MLX GPU path.

Continue actual Yield/Delay/Duration dependencies. The Native DefaultExecutor in
native/CoroutineContext.cpp still substitutes detached timers for the source's
WorkerDispatcher delegation; native/MultithreadedDispatchers.kt:20-76's production
WorkerDispatcher class is missing. Translate those dependencies, not test-only
link replacements. The latest full-driver output records the remaining link gate.

All three deep scans finish with unchanged aggregates: compiler286/7163 bodies,
132/1617 types,0.28 similarity,10 scoring failures; coroutine665/2918 bodies,
179/560 types,0.24 similarity,12 failures; frontend14/7163 bodies,6/1617 types,
0.04 similarity,zero scoring failures. Existing source/pairing/scoring limitations
remain recorded. See RESUME_ADDRESS_SOURCE_REPAIR.md for the detailed evidence.

## Ordinary full-expression lowering repair — 2026-10-08

Continuation from `eee1d016` closes the six retained-handler extraction conflicts
described in the preceding checkpoint below. NativeSuspendLowering.cpp:611 now
reports the actual source/replacement ranges. They reveal ordinary assert macros
with exception_ptr temporaries being sliced despite containing no suspension;
macro-body UnaryOperator operands then address a spelling range in assert.h rather
than the caller expression. Compared with NativeSuspendFunctionLowering.kt:207-249,
emit_statement at :2097-2105 keeps the whole nonsuspending expression together and
uses existing declaration-to-frame rebinding. Native C++ owns its full-expression
temporary lifetime. Operands before a suspending sibling still use the existing
retained slicing. The Clang-only evaluated-expression inspection at :933 preserves
callable capture/class lowering, including omitted initializers, without visiting
unevaluated expressions or inventing compiler IR identities.

The LLVM23.1.2 frontend rebuild succeeds; original retained fixture extraction
exits zero for all six formerly failing handlers. The full authoring regression
again passes the earlier direct/default/expression/qualified-local/restricted and
CMake ordinary/nested callable stages. Retained input.cpp and main.cpp now compile
through the production CMake frontend plus LLVM module pass, with twelve unsuppressed
warnings. Linking fails: the actual nineteen-unit bounded archive lacks Yield,
Delay and Duration definitions, and input.cpp.o contains unresolved libc++
variant helper instantiations. CompilerFrameLowering.cpp:622-627 now instantiates
missing actual template definitions before traversing their bodies, closing the
dependency-walk ordering gap. A clean rebuild still exposes helper references for
both variant alternatives; this change does not close the imported-definition/code
generation gap. The final full driver reports only the exception_ptr alternative
at the same link gate; investigate the clean-build/full-driver emission difference
rather than treating either result as reliable helper closure.
Continue actual source dependency closure and template/lambda
instantiation/import repair. Keep the fixture, overlap checking and actual
Result/variant ownership intact; do not supply substitute runtime definitions.
No retained runtime or later forced-include/termination/rejection gates are claimed.

All three deep scans complete with unchanged aggregate counts: compiler reference
286/7163 bodies,132/1617 types,0.28 similarity,10 scoring failures; coroutine root
665/2918 bodies,179/560 types,0.24 similarity,12 failures; frontend14/7163 bodies,
6/1617 types,0.04 similarity,zero scoring failures. Existing source/pairing/scoring
limitations remain recorded in their generated inventories. Neither complete
standalone/Native MLX GPU path nor full compiler/library translation is established.
See RESUME_ADDRESS_SOURCE_REPAIR.md for the detailed receipt and current evidence.

## Job initialization and retained field type identity — 2026-10-08

Continuation from `4fc388b6` reads the actual JobImpl source contract at
JobSupport.kt:1425-1450. The former C++ getter lazily computed handlesException
with two mutable flags and followed arbitrary ChildHandle parents. Kotlin assigns
its val once after initParentJob and walks only ChildHandleNode.job links.
JobImpl.cpp:20 now computes handles_exception_ immediately after parent attachment;
:29 only reads that initialized property. JobSupport.cpp:527 implements the private
handles_exception source loop beside the actual private ChildHandleNode and Impl
classes. The walk checks the exact node type at each parent handle and retains
the same node job through its existing owning C++ parent accessor. A private friend
declaration allows that source method to access the opaque implementation without
exposing the internal node type or creating another parent/runtime abstraction.

JobImpl.hpp now contains the slim class interface and source provenance; property
and completion bodies are in JobImpl.cpp. The existing shared_from_this factory
adaptation remains explicit: create establishes shared ownership, then runs the
source parent/property initialization. Direct construction still requires the
existing deferred init_parent_job step. JobSupport's actual default
handleJobException = false body moved from the header into JobSupport.cpp:513;
its unused argument is unnamed, without warning suppression or changed behavior.

The new registered test_job_impl_initialization target translates the first three
CompletableJobTest.kt:8-45 completion cases and adds a regression that reads the
initialized property for the first time after child and intermediate parent links
are detached. Actual JobSupport/JobImpl objects supply the handling and unhandled
chains. A small C++ test subclass exposes the existing protected completion
operation; it does not replace any job state or exception algorithm. The main is
a separate ordinary C++ entry, following the repository's existing test pattern.
The target keeps assertions enabled in Release with -UNDEBUG. Fresh root CMake
configuration registers it with both frontend and mandatory LLVM module plugins,
using LLVM23.1.2 and Native runtime OFF.

JobImpl.cpp and the test executable compile with -Wall/-Wextra/-Wpedantic/-Werror.
All four test functions execute under ASan/UBSan successfully, including the cached
property after detachment, normal/exceptional completion and child waiting.
The dependency archive now contains nineteen actual source units: the earlier
context/interceptor/continuation/cancellation/Symbol units, plus JobImpl, JobSupport,
CancellableContinuationImpl, ContinuationBindings, Job, Native Exceptions,
EventLoop.common, DispatchedTask, Native StackTraceRecovery and ThreadContext,
LockFreeLinkedList.common, CoroutineDispatcher, IntrinsicsNative and LimitedDispatcher.
No stale earlier-namespace archive or substitute runtime was linked. The binary
links libc++, libSystem and Clang's ASan runtime, without Kotlin/JVM libraries.
This is bounded actual-source execution, not a strict full-core build. JobSupport
and the added dependencies report twenty-two unsuppressed warnings, in addition
to the earlier eight context/continuation warnings. The fresh production core
build still exits with strict unused-parameter diagnostics in CancellableContinuationImpl,
Select, Builders and their consumers. Those diagnostics remain actionable.

With these actual dependencies, the authoring driver passes the previously blocked
restricted-receiver compile/runtime gate and reaches the CMake callable cases.
They exposed an actual field-type mismatch in NativeSuspendLowering.cpp:653:
reference and retained-object/array storage used unqualified type.getAsString,
while optional value slots already used Clang's fully qualified resolved type.
Compared with CoroutinesVarSpillingLowering.kt:53-66's field.type = variable.type,
all three storage categories now share that resolved type and cv-qualification.
Generated callable type bindings keep their existing actual declaration mapping.
The captured unique_ptr<Tracked> field no longer loses its std namespace; no
ownership category or caller-written type was replaced.

The final full driver passes its prior controls, direct and default-callable cases,
Unit/default arguments, expression slicing, all46 qualified local/condition cases,
restricted receiver, and CMake ordinary and nested lambda compile/runtime assertions.
The nested fixture checks capture identity, immediate/resumed failures, copied
receivers, captured arrays and cleanup. The driver then stops during extraction
of retained_locals/input.cpp:240,262,283,304,323,352 with overlapping/macro-expanded
source-region diagnostics in exception-handler functions. No later retained-local
CMake or runtime gate is claimed. Continue the source exception-handler/declaration
rewrite against Kotlin's actual exception lowering and inspect the conflicting
source ranges; do not change the fixture or weaken overlap checking to hide it.

All three production/compiler deep scans completed against the final sources.
Compiler reference:286/7163 bodies,132/1617 types,0.28 similarity,10 scoring
failures. Coroutine root:665/2918 bodies,179/560 types,0.24 similarity,12 failures.
Clang frontend:14/7163 bodies,6/1617 types,0.04 similarity,zero scoring failures.
The JobImpl split is compared against the whole JobSupport.kt source unit, yielding
2/91 bodies and1/18 types for that pairing; it does not certify all JobSupport or
JobImpl algorithms. Source pairing and the sparse compiler/tool limitations remain
material, and none of these counts establishes complete translation.

An additional full common-test-to-C++-test-root deep scan completes at114/1353
bodies,142/192 types,0.31 similarity and83 scoring failures. Investigation of the
new executable first exposed a mixed namespace from global main. Separating that
entry follows the existing harness layout and makes the test-body namespace pair
eligible. The tool then scores the new harness0/6 bodies and0/1 types against the
whole CompletableJobTest class: these standalone adapted assertion functions do
not provide the Kotlin TestBase/class API. It also changes that source unit's
selected target pairing, so the test-root aggregate is not an improvement claim.
The three asserted completion cases execute, but the complete source test class
and remaining coroutine test-framework cases are still untranslated; the new
executable is explicit regression evidence only. Do not force the counts with a
fake or duplicate source class. Existing incomplete test files and scoring failures
remain genuine work. The full goal stays active, with the retained exception-handler
source-range failures as the next concrete compiler repair.


## Resolved callable references and unevaluated queries — 2026-10-08

Continuation from `9c28cca4` closes the moved default-expression callable's lost
namespace binding. Compared against AbstractFunctionReferenceLowering.kt:261-288
and DefaultArgumentStubGenerator.kt:91-108, Kotlin retains non-local declaration
symbols while moving bodies and remapping locals. The Clang printing adapter now
shares SuspendFunctionAnalyzer.cpp:162's actual resolved declaration-reference
printer with AbstractFunctionReferenceLowering.cpp:130. Capture mappings take
precedence; local variables, parameters and non-type template parameters keep
their original binding. Non-local namespace/class/enum references retain their
qualification and explicit template arguments. Declaration initializer detection
also recognizes these references, because Clang's declaration printer otherwise
bypasses expression helpers. No source classifier, replacement IR object or
alternate coroutine frame was introduced.

The factory regression keeps unqualified source calls in their original namespace
and exercises both a local initializer and a return expression after relocation.
Its existing assertions verify one factory evaluation, the retained current frame
and the resumed result 42. Factory, owned and borrowed default cases now execute
under ASan/UBSan. The driver initially reached a missing Symbol constructor at
link time; compiling the actual internal/Symbol.cpp strictly and adding its object
to the bounded actual-source dependency archive resolved that dependency. It does
not establish a fresh full-core build. The previously documented eight dependency
warnings remain unsuppressed; no strict full-dependency build is claimed.

The next source defect was StaticAssertDecl's operands: Clang does not expose them
as DeclStmt children. NativeSuspendLowering.cpp:386 now visits those declaration
operands with the original type-query binding rules. That revealed two further
integration mismatches. CompilerFrameLowering.cpp:106 now excludes unevaluated
query/decltype callees from executable dependency expansion; querying std::declval
must not instantiate its intentionally invalid executable definition. Finally,
NativeSuspendLowering.cpp:373 preserves the owning AST's resolved nondependent
noexcept value rather than recomputing it against imported helper declarations.
This is explicit C++ adaptation for source type/ownership preservation, not a
claim that Kotlin has StaticAssertDecl or std::declval nodes.

The unchanged qualified_locals.cpp fixture now passes -Wall/-Wextra/-Wpedantic/
-Werror compilation with the Clang frontend and mandatory LLVM module pass, then
ASan/UBSan execution. All six qualified-local outcome cases and forty conditional
expression cases run. They assert retained object/resource identity, repeated
suspension, resumed failure/cancellation, immediate and partial-construction
failure, static identity, structured-binding behavior and ordered cleanup. This
supersedes the previous receipt's unexecuted qualified-local checks. The full
Native shared-frame and real MLX GPU acceptance paths remain unproved.

The final authoring driver passes its warning/rejection controls, direct-entry
extraction and in-process checks, all35 default-callable outcomes, the Unit-tail
fixture, factory/owned/borrowed defaults, expression slicing, and all46 qualified
local/conditional outcomes. It then stops at restricted_receiver.cpp link time:
actual JobImpl::create is absent from the bounded dependency archive. No later
restricted-receiver runtime or CMake lambda/retained-local gates are claimed.
Continue by reading the actual JobImpl/JobSupport source and matching upstream
Kotlin, then resolve the current production dependency build. Do not link a stale
core archive with the earlier continuation namespace, substitute a job/runtime,
or weaken the receiver fixture. The qualified-local executable links only libc++,
libSystem and Clang's ASan runtime; this is bounded standalone evidence, not the
complete ordinary C++/MLX GPU path.

All three required deep scans completed, with library/frontend scans refreshed
after the final source changes. Compiler reference:286/7163 bodies,132/1617 types,
0.28 similarity,10 scoring failures. Coroutine root:663/2918 bodies,178/560 types,
0.24 similarity,12 failures. Clang frontend:14/7163 bodies,6/1617 types,0.04
similarity,zero scoring failures. These counts remain substantially incomplete.
The sparse Kotlin compiler checkout, source-emission/receiver-helper matching
limitations and existing scoring failures remain unresolved; Clang integration
has no literal Kotlin file twin, so successful concrete fixes cannot be counted
as completion of the actual full Kotlin IR emitter or source-class dependencies.
The complete translation goal remains active.


## Callable declaration ownership and invoke identity — 2026-10-08

Continuation from `827b1c9c` repairs the production Clang integration of
AbstractFunctionReferenceLowering.kt:261-288's specific invoke declaration and
parent remapping. The previous turn's saved default-callable failure came from
an intermediate frontend: the final tail-only default lambda already passes.
A newly added non-tail default lambda reproduced the real retained-frame defect.
Its implicit Clang closure record ends at the capture introducer; using that
range truncated generated frame construction. getLambdaContextDecl is optional
and was null in the concrete default-parameter case.

CompilerFrameLowering.cpp:118 now resolves the actual containing declaration
from the host AST ancestry when the explicit lambda context is absent. At :215
it preserves the owning function/class/variable declaration, its namespaces and
its complete source boundary; prototype/variable/record semicolons are restored.
This is Clang infrastructure for the source parent-remapping contract, not an
invented Kotlin IR owner or alternate frame. Runtime checks then exposed a second
mismatch: sibling lambdas in one complete class can have identical invoke names
and canonical types. At :423 the importer also compares original source identity
before selecting the replacement body. It no longer installs a sibling's original
body over the requested lowered invoke. Existing explicit template-instantiation
entry selection stays separate. Temporary debugging diagnostics were removed.

The existing default-callable fixture now covers a tail default plus non-tail
function-prototype, function-definition, namespace, global initializer, member
initializer and member-parameter contexts. Its runtime branch is enabled only
by KXS_TEST_DEFAULT_CALLABLE_RUNTIME; the normal syntax gate still checks the
source declarations. The test driver registers strict ordinary in-compiler
compilation and execution of all35 outcome cases. Seven contexts each exercise
immediate success, resumed success, resumed failure, resumed cancellation and
immediate failure; the tail entry forwards the parent, the six non-tail entries
hand off their current frame, and each completes once. Result boxes have explicit
unique_ptr deletion. Empty closure contexts are tested; arbitrary captured-object
and temporary callable ownership is not established by this fixture.

ContinuationImpl.cpp:6 removes its unused DispatchedContinuation include after
comparison with the actual Native ContinuationImpl.kt imports/body. No runtime
algorithm or substitute implementation was added. Fresh actual context_impl.cpp,
ContinuationInterceptor.cpp, ContinuationImpl.cpp and the stdlib
CancellationException.cpp form the bounded runtime dependency archive used here.
They build without Kotlin tools/runtime. The context/interceptor/continuation
units report eight existing unused-parameter warnings under -Wall/-Wextra/
-Wpedantic; those warnings remain unsuppressed and are not a strict dependency
build pass. CancellationException and the generated fixture compile strictly.

The frontend builds against LLVM23.1.2 with Native runtime OFF. The default-callable
fixture passes -Wall/-Wextra/-Wpedantic/-Werror compilation and ASan/UBSan execution.
otool lists libc++, libSystem and Clang's ASan runtime, with no Kotlin/JVM link.
The Unit-tail fixture also passes fresh sanitizer execution, and the earlier
expression-slicing fixture now executes its comma temporary destruction, borrowed
resource identity, repeated suspension, resumed failure/cancellation and ordered
cleanup assertions successfully against the same actual dependency units. This
supersedes the last checkpoint's lack of runtime evidence for those bounded cases;
it does not prove a fresh full-core build, MLX GPU execution or actual Native
shared-frame interoperability. An additional strict qualified_locals.cpp compile
fails before import on unremapped structured-binding names head/resource in
static_assert(noexcept(...)); no qualified-local runtime pass is claimed here.
That source query/declaration remapping remains another lowering gap.

The full authoring driver advances through its warning controls, source rejection
checks, new syntax gate, direct extraction/in-process execution,35 default-callable
cases and Unit fixture. It now stops at default_arguments/factory.cpp:22:
a relocated nonsuspending default-expression lambda prints make() in the global
caller frame, losing its actual defaults::make symbol context. Continue the
AbstractFunctionReferenceLowering.cpp:74 BoundValues printer's ordinary declaration
reference remapping, compared with the existing DefaultArgumentReferences printer
and DefaultArgumentStubGenerator.kt:91-108. Do not fix that fixture by manually
qualifying its upstream-context source or by replacing the default expression.

Final compiler/library/frontend deep scans completed. Compiler reference root:
286/7163 bodies,132/1617 types,0.28 similarity,10 scoring failures,695 sparse files,
193 paired units/285 target files. Coroutine root:663/2918 bodies,178/560 types,
0.24 similarity,12 failures,412 paired units/614 target files. Frontend root:
14/7163 bodies,6/1617 types,0.04 similarity,0 scoring failures,15 paired units/
25 target files. Clang AST infrastructure changes do not certify additional Kotlin
IR/classifier/metadata parity. Current reports remain in build/source-continuation/
{compiler,library,frontend}-source-distance. Build/runtime evidence is in
build/source-continuation/default-callable-runtime; the full driver failure is in
default-callable-full-regression.log. Full transliteration, general callable/IR
lowering, both required MLX/Native executable paths and complete lifetime contracts
remain unfinished. The original goal remains active.

## Tail expression containers and C++ temporary retention — 2026-10-08

Continuation from `9cd3e1b3` returns to the production frontend's state-machine
selection. TailSuspendCallsCollector.kt:64-79 gives an expression container's last
statement its enclosing tail position. TailSuspendCallsCollector.cpp:45,78 now
maps built-in C++ comma expressions to that rule: the left operand is non-tail
and the right operand inherits the enclosing state. Nested containers, conditions,
try regions and ordinary enclosing calls retain their source visitor state.
Overloaded comma remains an ordinary call. is_unit_read at :160 additionally
unwraps parentheses and Clang cleanup nodes, matching the existing transparent
AST traversal; a parenthesized null Unit result no longer forces a needless frame.

KotlinxSuspendPlugin.cpp:499 tracks actual comma-prefix evaluation when selecting
a direct entry. Destructor-bearing materialized/bound temporaries in that prefix
require retained frame storage across the tail operand. By-value arguments of the
tail call do not acquire that prefix state. This is a NOTE(port) C++ lifetime
adaptation to NativeSuspendFunctionLowering.kt:55-91, not a substitute state
machine. Existing borrowed references and captured object ownership are unchanged.
NativeSuspendLowering.cpp:1102 now saves the actual PrintingPolicy while enabling
canonical default-argument types: Clang's PrintAsCanonical bit-field cannot bind
to SaveAndRestore<bool>. This repairs a concrete production plugin build error.

The Native-OFF LLVM23.1.2 frontend and collector targets build. All26 AST cases
pass, including nested comma tails, non-tail prefixes, overloaded comma, try and
conditional state, and parenthesized Unit returns. The existing driver now checks
nine direct entries and seven suspending variants through28 immediate/resumed/
immediate-failure/resumed-failure cases. Its direct portion was run separately
from the broad driver's earlier failing gate. Both extracted diagnostic output
and ordinary in-compiler compilation/execution pass with -Wall/-Wextra/-Wpedantic/
-Werror. ASan/UBSan direct execution passes and confirms exact caller continuation
forwarding, one completion and released continuation handles. No cancellation or
retained resource-destruction claim follows from those direct cases.

The direct executable links fresh actual context_impl.cpp and
ContinuationInterceptor.cpp dependencies only. Their separate strict compile
fails on four existing unused-parameter diagnostics; ordinary dependency
compilation with -Wall/-Wextra/-Wpedantic succeeds and reports all four warnings.
No warning was suppressed or source dependency replaced. This bounded executable
is not a fresh full-core build. The available core archives still expose the older
kotlinx::coroutines ABI; linking current frame fixtures against them fails on
current kotlin::coroutines context/continuation definitions.

unit_tail.cpp and expression_slicing.cpp compile to LLVM with strict warnings,
mandatory frontend/pass injection and O1 ASan/UBSan instrumentation. The Unit
comma entry has no generated frame; comma_temporary owns the actual immovable
TemporaryArgument in its frame and emits its cleanup paths. Its O0 module has
blockaddress stores and indirectbr dispatch with consumed marker calls. O1 folds
its single-destination dispatch into a direct resume branch; the stored block
address remains. Added destruction/failure/cancellation runtime assertions are
not freshly executed. expression_slicing now includes the actual continuation
and DSL headers it uses rather than the unrelated cancellable DSL umbrella.
The full driver still fails earlier in suspend_default_callable.cpp:8 while
parsing a generated default-lambda frame with a truncated/unbalanced enclosing
source context. That lowering dependency remains a concrete next repair.

All final deep scans completed. Compiler reference root:286/7163 bodies,
132/1617 types,0.28 similarity,10 scoring failures,695 sparse source files,
193 paired units/285 target files. Coroutine root:663/2918 bodies,178/560 types,
0.24 similarity,12 failures,412 paired units/614 target files. A third scan pairs
the full pinned compiler corpus with the actual frontend root:14/7163 bodies,
6/1617 types,0.04 similarity,0 scoring failures,15 paired units/25 target files.
TailSuspendCallsCollector reports8/14 bodies,2/2 types,0.13 similarity. Its reported
missing receiver helpers must be compared with actual is_unit_read and
is_return_if_suspended_call; IR returnable-block symbols and IR-specific body
visitor contracts remain untranslated. The broad root metrics do not certify
this frontend's full source parity. Reports are under build/source-continuation/
{compiler,library,frontend}-source-distance; bounded execution/IR logs are in
{tail-container-focused,tail-runtime-fixtures}, and the broad failure is in
tail-container-regression.log.

Continue the default-callable source-context repair and the remaining actual IR
container/returnable-block, spilling and compiler dependency translations. Any
identity/hash/text operations already exist in AnyIdentity.cpp, AnyToString.cpp
and native/Runtime.cpp; the collection gap is arbitrary element operations at
the actual std::any erasure boundary, not absence of those Any methods. Complete
standalone C++/MLX and actual Native shared-frame GPU handoffs, retained resource
identity/cleanup and full-library translation remain unproved. The original goal
remains active.

## Standard and Native scalar identity catalogs — 2026-10-08

Continuation from `437fc2e7` translates StandardClassIds.kt's178 scalar properties,
six public naming/arity bodies and22 private concrete naming helpers into
StandardClassIds.hpp:9 and StandardClassIds.cpp. Package/class, annotation,
annotation-parameter and callable catalogs preserve source nesting. FunctionN,
SuspendFunctionN, KFunctionN and KSuspendFunctionN build their exact source names,
including negative inputs without introducing validation. Unsigned IDs derive
from the actual signed ID's short name. Nested Map entries and JsExport annotation
IDs derive from their actual parent IDs. ParameterNames.retentionValue returns
the same retained Name reference as value, preserving the source alias.

NativeRuntimeNames.hpp:9/.cpp now translate all31 public scalar properties:
six atomic class IDs, five callable IDs and20 annotation IDs, plus the two private
package properties and two private String.callableId helper bodies. Callables:24
and Annotations:38 reuse the actual StandardClassIds and CallableId contracts.
The source standard atomic package is kotlin.concurrent.atomics; Native runtime
atomics are in kotlin.concurrent. CompareAndSetAt versus compareAndSet names and
the top-level AtomicArray factory versus member methods remain distinct. Native
annotation aliases retain their separate qualified names; Escapes.Nothing is a
nested ID from the actual Escapes class. These identities do not install annotation
effects or provide actual IR declarations, JVM metadata or Native object layout.

All algorithms and private helpers stay in .cpp; headers contain declarations and
public nested types. Source provenance accompanies each translated function and
class. Object properties use retained first-access getters, marked NOTE(port),
and C++ keyword collisions use _name suffixes. No replacement set/map/container,
IR descriptor, coroutine state machine or Kotlin runtime dependency was introduced.
StandardClassIds still lacks17 set/map/derived collection properties, its Collections
object, primitiveArrayId and inverseMap. NativeRuntimeNames still lacks its two
primitive-to-atomic maps. Their actual collection dependency remains untranslated;
these absent contracts are not represented by stubs or stand-in algorithms.

CMakeLists.txt:48 registers StandardClassIds on the three production LLVM targets;
:104-107 connects the Native catalog's actual dependencies to the Konan fixture;
:129-137 connects both catalogs to the existing naming fixture. An initial link
exposed an incorrectly edited source-list occurrence; the CMake registration was
corrected and all five affected targets then built with Native runtime OFF on
LLVM23.1.2. Strict -Wall/-Wextra/-Werror debug ASan/UBSan compile and execution,
strict release syntax, six focused CTests and the existing LLVM module-generation
fixture pass. Added cases exercise source package distinctions, nested IDs, arity
names, alias identity, top-level/member callables and annotation aliases. No
warnings are suppressed. This is bounded catalog/LLVM dependency evidence, not
actual IR classification or complete coroutine/Native/MLX executable acceptance.

Both final-root deep scans completed. Compiler corpus remains695 sparse files:
286/7163 bodies,132/1617 types,0.28 similarity,10 scoring failures,193 paired units/
285 target files. StandardClassIds reports7/30 explicit bodies,4/5 types,0.04 body
score,206 target bodies. Its Collections type and two private collection-dependent
helpers are genuinely absent; most translated String receiver helpers remain
unmatched against C++ free-function forms. NativeRuntimeNames reports0/2 bodies,
3/3 types,0.00,35 target bodies: both actual private callable_id overloads remain
unmatched. Object-property inventories also report present getters as missing.
Source object/function emission falls back, generated source has parse errors and
normalized logic0; target parse errors are absent. Counts therefore do not certify
property/constructor/metadata parity. Full coroutine root remains663/2918 bodies,
178/560 types,0.24,12 failures,412 paired units/614 target files. Reports in
build/source-continuation/{compiler-source-distance,library-source-distance}
remain the oracle. The full transliteration/state-machine goal remains active.

The next missing dependency chain is the real collection/object and HashMap
implementation, the catalog sets/maps and Grouping/byFqNameParts. Actual callable,
InlineClassesSupport/classifier/cache/frame consumers and complete executable
acceptance still require translation and wiring.

## Callable identity and special-name source contracts — 2026-10-08

Continuation from `0c543eee` translates core/names CallableId.kt:45-141 and
SpecialNames.kt:18-119. CallableId.hpp:9 exposes all six public constructors,
package/class/callable/class-ID properties, locality, debug and single qualified
names, copy, equals/hash/text and the three extension contracts. CallableId.cpp:11,
13 keeps private companion name/class-ID calculations out of the header. The
local package is built from the actual SpecialNames.LOCAL property. Nullable
class/path values use optional and nullable extension receivers borrow pointers,
following the existing compiler naming value boundary. Ordinary application
objects gain no compiler superclass or Native runtime representation.

Callable equality compares package/class/callable name and intentionally ignores
class-ID locality and pathToLocal. Hash accumulation preserves source Int wrapping
and null's zero contribution. Copy preserves the original class ID and debug
path; with_class_id constructs a fresh ID and drops that path as the source does.
Debug naming selects pathToLocal first; normal naming selects the actual ClassId
qualified name when present. Text replaces package dots with slashes, preserving
class dots and the leading slash for the root package. Compiler opt-in locality
metadata and actual Native object identity are not supplied by these value APIs.

SpecialNames.hpp:8 exposes the source anonymous string constant,18 retained
Name/FqName object properties and all six function bodies. SpecialNames.cpp owns
the private anonymous-parameter prefix and implementations. Properties use
first-access retained storage to avoid cross-unit initialization hazards, marked
NOTE(port). THIS maps to this_name to escape the C++ keyword. Index validation
retains the source diagnostic, anonymous parameter recognition uses its exact
prefix test, and nullable safeIdentifier overloads preserve the distinction
between identifier construction and safe-identifier validation.

CMakeLists.txt:46-47 registers both units on all three production LLVM targets;
:125-131 adds them to the existing naming fixture. Native-OFF configure and
kxs_fq_name_test/KotlinxCoroutinePass/kxs-inject/kxs_codegen_test builds pass.
Strict debug -Wall/-Wextra/-Werror ASan/UBSan compile and execution, strict release
syntax, six focused CTests and the existing actual LLVM module-generation fixture
pass. The extended fixture exercises pre-main local-name construction, nullable
and nested IDs, differing class locality/debug paths with equal hashes, copy and
class replacement, root formatting, special-name errors and nullable overloads.
No warnings are suppressed. These tests cover naming dependencies, not actual IR
classifier execution or complete coroutine/Native/MLX acceptance.

Both final-root deep scans completed. Compiler corpus remains695 sparse files:
279/7163 bodies,127/1617 types,0.28 body similarity,10 scoring failures,192 paired
units/283 target files. CallableId reports7/8 explicit bodies,1/1 type,0.34 score;
its actual private calculate_class_id implementation is unmatched against the
source companion owner. Companion property matching also leaves LOCAL_NAME and
PACKAGE_FQ_NAME_FOR_LOCAL unrecognized despite their private/public implementations.
SpecialNames reports6/6 explicit bodies,1/1 type,0.41 score. These body counts do
not certify property/constructor/metadata parity. Source class/object emission
falls back, generated source has parse errors and normalized logic0; target parse
errors are absent. Full coroutine root remains663/2918 bodies,178/560 types,0.24,
12 failures,411 paired units/612 target files. Generated reports remain the oracle.

The production targets compile these dependencies but real callable/IR consumers
remain to be translated and wired. HashMap and its collection/object dependencies,
fresh-map Grouping/byFqNameParts, StandardClassIds/NativeRuntimeNames callables,
InlineClassesSupport, classifier/cache/frame lowering and both complete executable
paths remain open. The full translation/state-machine goal remains active.

## Numbers and Native HashMap sizing dependency — 2026-10-08

Continuation from `490cb209` translates the26 public scalar operations in Native
Numbers.kt:12-265, together with the six private intrinsic/runtime helpers that
supply them. Numbers.hpp keeps the public declarations and short source comments;
Numbers.cpp owns implementations and private helpers. Int/Long bit counts, zero
cases, highest/lowest set bits and rotations follow the pinned source. Unsigned
arithmetic preserves Kotlin wrapping and masked shifts without signed-overflow
or shift-width undefined behavior, including INT_MIN rotation counts. Float/Double
classification and raw/canonical bits use the actual Operator.cpp implementations,
Primitives.kt NaN definitions and scalar reinterpret contracts. Companion from_bits
functions call private source intrinsic implementations. Native GCUnsafeCall and
TypedIntrinsic compiler metadata/exported Native ABI are not established here.

HashMapFunctions.hpp/.cpp translate HashMap.kt:599-601 computeHashSize and
computeShift in the existing private-companion namespace convention. Sizing uses
capacity.coerceAtLeast(1), wrapping multiplication by3, and the actual highest-bit
operation; shift uses the actual leading-zero operation plus1. These are source
algorithms, not a replacement HashMap. The917-line concrete HashMap, storage,
hash/equality, view and iterator contracts remain untranslated. Its source
AbstractMutableCollection and AbstractMutableSet bases have been opened; the
AbstractCollection/AbstractSet dependencies must also be translated. Fresh-map
Grouping operations and byFqNameParts remain absent, followed by actual
InlineClassesSupport/classifier/cache/frame consumers.

CMakeLists.txt:52-53 registers both implementation units on KotlinxCoroutinePass,
kxs-inject and kxs_codegen_test; :85 registers kxs_numbers_test. Native-OFF
LLVM23.1.2 configure and all four affected target builds succeeded. Strict debug
-Wall/-Wextra/-Werror ASan/UBSan compile and execution, strict release syntax,
six focused CTests and the existing actual LLVM module-generation fixture pass.
The numeric fixture covers all single-bit positions, zero/sign bits, extreme
rotation counts, wrapping map sizing, NaN payload/canonicalization, negative zero,
infinity and finite limits. No warnings are suppressed. This is scalar dependency
and existing LLVM fixture evidence, not full coroutine/Native/MLX acceptance.

Both final deep scans completed. Restoring Native/Wasm collection sources expands
the compiler corpus from673 to695 sparse files:266/7163 matched bodies,
125/1617 types,0.28 body similarity,10 scoring failures,190 paired units/279 target
files. Scope changed; these totals do not measure improvement against the earlier
corpus. Numbers reports6/20 explicit bodies,0/0 types,0.05 body score,32 target
bodies. Fourteen Int/Long receiver and Float/Double companion bodies remain
unmatched against the actual C++ overload/namespace forms. The extractor records
Kotlin extension receivers as parents (symbol_extraction.cpp:662-688); source
emission also falls back on function nodes. Target parser errors are absent, while
normalized source logic/text is provisional. Do not infer absent implementation
or completed parity solely from that score.

The scanner explicitly rejects HashMap.kt [kotlin.collections] versus
HashMapFunctions.hpp [kotlin.collections.hash_map.detail] with IDENTITY_MISMATCH.
The two private sizing bodies therefore have no certified match count, and the
HashMap source file remains missing in the oracle. This is an observed namespace
pairing limitation, not permission to report full HashMap parity. Full coroutine
root remains663/2918 bodies,178/560 types,0.24 similarity,12 failures,409 paired
units/608 target files. Generated evidence and priorities remain under
build/source-continuation/{compiler-source-distance,library-source-distance}.
The full transliteration/state-machine goal remains active.

## Grouping destination operations — 2026-10-08

Continuation from `e5392502` translates Grouping.kt's public interface and all
five destination-taking bodies: aggregateTo, both foldTo overloads, reduceTo and
eachCountTo. Grouping.hpp:71,93,119,132,143,156 exposes those contracts. Public
Kotlin generics remain header templates; private boxing/projection helpers are
necessary for the public generic variance and reuse existing collection codecs.
The key out-type exposes genuine supertypes while the source element type remains
invariant. MutableMap<in K,R> destinations accept real key supertypes and preserve
R's invariant type. Mutation goes through the public typed put method, decoding
the projected key through its existing codec; protected map mutation dispatch
was not made public. The original destination is returned by reference.

The aggregation loop follows source order: iterate, select key, look up current
accumulator, distinguish missing key from present-null, call operation, store
result. Fold selects an initial value only for a new key; reduce uses the first
element only for a new key. NullableMapValue's existing nullable representation
preserves pointer/shared/optional/Any nulls without an extra nullable layer.
Non-null accumulator casts use bad_any_cast in the private C++ codec boundary.
Count uses uint32 addition/bit_cast to preserve Kotlin Int wrapping without C++
signed overflow; Primitives.kt:975-976 declares PLUS and IntrinsicGenerator.kt:
560-567 selects ordinary LLVM add for integer inputs. This does not translate the
whole intrinsic generator or establish source Native exception representation.

The four fresh-map operations aggregate/fold/fold/reduce remain absent until
mutableMapOf's concrete dependency is translated. Source Maps.kt:85 constructs
LinkedHashMap; Native/Wasm HashMap.kt:917 aliases LinkedHashMap to the actual
insertion-ordered HashMap implementation. That917-line source, its array/hash/view
and iterator contracts, groupingBy adapters and actual enum values input remain
required for byFqNameParts. No std::map substitute, fake map implementation or
placeholder was introduced. InlineClassesSupport/classifier/cache/frame work
continues after these source dependencies.

Native-OFF CMake configure and kxs_grouping_test build pass. Strict debug
-Wall/-Wextra/-Werror ASan/UBSan compile and execution pass. Strict release header
syntax passes when included from a translation unit; compiling the header as
main initially triggered pragma-once-outside-header, corrected by checking its
actual include form without suppressing the warning. Five focused CTests pass.
The new fixture executes source iterator/key covariance and nullable/non-null
accumulator conversions. Eight explicit instantiations compile the destination
algorithms against actual abstract MutableMap interfaces: int, borrowed pointer,
shared owner, Any, optional, projected Any keys, reduce and count. Those map
algorithms are NOT executed because concrete source maps are still untranslated.
The covariance provider is an ordinary fixture, not an IR descriptor/classifier.
This is bounded source/type evidence, not complete coroutine/Native/MLX acceptance.

Both final-root deep scans completed. Compiler source remains673 sparse files:
253/6847 bodies,111/1575 types,0.31 average,5 scoring failures. Grouping:5/9 explicit
bodies,1/1 type,0.08 reported body score. Target has9 bodies/5 types including
representation bridges. Kotlin emission falls back on interface/function/annotation
nodes, has generated parse errors and normalized logic/text0; target parse errors
are absent. Thus the score is provisional. Full coroutine root:663/2918 bodies,
178/560 types,0.24,12 failures,406 paired units/603 target files. The reports,
missing inventories and priorities in build/source-continuation/{compiler-source-distance,
library-source-distance} remain the current oracle; neither root is complete.

CMakeLists.txt:83 registers the header-only dependency's focused target. The
production catalog does not yet call Grouping or its destination algorithms;
this checkpoint does not claim that wiring. API rows and continuation evidence
are updated. The full translation/state-machine goal remains active.

## Native primitive catalog and runtime-name dependency — 2026-10-08

Continuation from `6f9875bf` translates InlineClasses.kt:50-70 into
InlineClasses.hpp:8-18 and InlineClasses.cpp. The ten KonanPrimitiveType values
retain their source ClassId and BinaryType.Primitive constructor properties.
Enum property getters follow the existing Variance convention; their references
borrow private, retained catalog values. CHAR maps to SHORT while remaining a
distinct ClassId and Primitive box. NonNullNativePtr maps to POINTER and Vector128
to VECTOR128 through the actual KonanFqNames operations. No LLVM shape-based
classification or fabricated IR declaration was introduced.

KonanFqNames.hpp:14/.cpp translate all26 scalar object properties and the three
source constants. Local-static getters preserve retained name values; C++ keyword
collisions use thread_local_name/volatile_name. gcUnsafeCall is computed through
NativeRuntimeNames::Annotations::gc_unsafe_call_class_id(), not an independently
guessed string. NativeRuntimeNames.hpp:7,10/.cpp translate only the two private
package properties and GCUnsafeCall annotation ClassId (source:11-12,49); the
other atomic/annotation/callable/map APIs remain absent. InternalKotlinNativeApi
metadata and compiler opt-in behavior remain untranslated.

The new pre-main catalog test exposed a real cross-unit initialization defect
in the preceding PrimitiveType implementation: the executable aborted with
"Class name must not be root: kotlin" because the catalog read unconstructed
builtin names. konan_global_before.log records that failure. PrimitiveType.hpp
now has a minimal constexpr literal/atomic constructor; the eight instances are
constinit. Their owned properties are constructed/published on first access in
PrimitiveType.cpp, with losing candidates destroyed and winning storage freed
by the instance destructor. This deliberate C++ initialization adaptation is
marked NOTE(port); it changes eager name-property construction timing. It retains
canonical instance/name references and the source FqName PUBLICATION behavior.
No alternate state machine/runtime is involved. Source NUMBER_TYPES and generated
enum APIs still require translation.

CMakeLists.txt:48-50 registers the new units on all three production LLVM targets;
:83 registers kxs_konan_primitive_test. Native-OFF Release configure and all five
affected target builds pass using LLVM23.1.2. Strict release syntax and strict
ASan/UBSan debug compiles pass. Four focused CTests pass. Fresh sanitizer runs for
both primitive fixtures pass, including pre-main catalog construction and the
separate fixture's16-reader first-publication race. Existing actual LLVM module
codegen/verification also passes. Compiler metadata tests establish neither
actual IR classifier execution nor the two complete Native/MLX acceptance paths.

Both final-root deep scans completed after the initialization repair. Restoring
core/compiler.common.native/name expands the sparse source corpus669→673 files.
Compiler:248/6847 bodies,110/1575 types,0.31 average,5 scoring failures.
InlineClasses:0/30 explicit bodies,1/3 types,0.00; eight target bodies implement
only enum properties/constructors. KonanFqNames:0/0 explicit source bodies versus
26 target getters,1/1 type; scorer forces0 because it inventories properties
without getter bodies. This is the added scoring failure, not a completed-body
claim. NativeRuntimeNames:0/2 bodies,2/3 types,0.00. Generated emission falls back
on enum/object/property nodes and reports parse errors; target parse errors are
absent. Normalized logic/text remain0 and provisional. Full library:663/2918
bodies,178/560 types,0.24,12 failures,404 paired units/601 target files. Evidence
and priority inventories are under build/source-continuation/{compiler-source-distance,
library-source-distance}; these measured criteria do not certify completion.

Next: actual byFqNameParts grouping and its concrete collection dependencies
(Map interface exists; Grouping.kt, Maps.kt and HashMap/LinkedHashMap bodies are
not translated), then InlineClassesSupport and actual IR classifier hooks/cache
keys consumed by frame/call lowering. All30 InlineClasses explicit algorithms,
the two missing support types, remaining name/enum APIs and complete executable
acceptance remain required. The full translation/state-machine goal stays active.

## Primitive-name catalog dependency — 2026-10-08

Continuation from `50c511a2` translates `PrimitiveType.kt:11-28,34-58` into
`org/jetbrains/kotlin/builtins/PrimitiveType.hpp:13` and its matching `.cpp`.
All eight named instances, primitive/array Name properties, lazily published
FqName properties and both exact short-name lookups follow the pinned source.
Nullable lookup results borrow canonical immutable instances. Property-bearing
Kotlin enum instances use a concrete singleton class; generated enum APIs
(name/ordinal/values/entries/valueOf/comparison) remain untranslated. NUMBER_TYPES
still requires actual setOf/LinkedHashSet implementation; no alternate set API
or placeholder was added. StandardNames.hpp:8 and .cpp translate only the two
built-in package properties at StandardNames.kt:73-81, with static getters to
avoid translation-unit initialization order. The remaining object is incomplete.

Lazy getters also reference SafePublicationLazyImpl's actual CAS algorithm,
LazyJVM.kt:114-133. Multiple candidates may compute, but all callers receive the
winner. Selected libc++ rejects atomic<shared_ptr>; the final implementation uses
atomic owned FqName pointers. unique_ptr frees losing candidates; the instance
destructor frees the two winning boxes. Getter references are borrowed for the
instance lifetime. This is internal C++ compiler metadata ownership, not a Native
GC object/frame representation or a replacement coroutine state machine.

CMakeLists.txt:46-47 registers both source units on kxs-inject,
KotlinxCoroutinePass and kxs_codegen_test; :80 registers the focused contract test.
Native-OFF Release configure and all four target builds pass with LLVM23.1.2.
Strict -Wall/-Wextra/-Werror debug sanitizer compile and release syntax pass.
Three focused CTests pass; primitive_type_sanitized passes ASan/UBSan. The new
fixture races16 first readers, checks published identity for both properties,
all eight names/array names and exact rejection of qualified/unsigned/incorrect
names. Existing actual LLVM codegen fixture emits/verifies its module. These
checks do not establish actual IR classification or either complete acceptance
path in docking_ring.md.

Both final-root deep scans completed after the storage repair. Restoring the
pinned builtin directory expands the sparse Kotlin corpus661→669 files; this
changes the measurement scope. Compiler root:248/6845 bodies,106/1568 types,
0.34 average,4 scoring failures. PrimitiveType:2/2 explicit bodies,1/1 type,
0.08 reported body similarity; target has8 bodies. StandardNames:0/19 bodies,
1/2 types,0.00. Both generated emitters fall back on the enclosing enum/object
and report generated parse errors; target parse errors are absent. Their
normalized logic/text scores are0 and provisional. The inventories do not count
missing enum-generated/property/set contracts as missing explicit bodies, so
2/2 cannot imply complete PrimitiveType translation. Evidence is in
build/source-continuation/compiler-source-distance/{scan.log,
deep_symbol_inventory.txt,deep_transliteration_evidence.txt,port_status_report.md}.
Full coroutine root:663/2918 bodies,178/560 types,0.24,12 scoring failures,
400 paired units/594 target files; ChannelFlow and Channels remain unchanged.

Next source consumers are KonanFqNames and KonanPrimitiveType in InlineClasses.kt,
including its source CHAR→SHORT mapping and structural byFqNameParts grouping.
Their actual collection dependencies, InlineClassesSupport, real IR classifier
hooks/cache keys and VariableManager/frame/call consumers remain required. No
fake IR descriptors or LLVM-pointer shape classification was introduced. The
full translation/state-machine goal remains active.

**Current qualified-name/class-ID continuation — 2026-10-08:** From48959b4a,
scalar FqName/FqNameUnsafe and ClassId contracts needed by the primitive catalog
are translated and registered on the production LLVM consumers. UTF-16 names,
quoted-dot parent parsing, cached child metadata, root diagnostics, prefix
boundaries, structural equality/hash and class-ID slash/backtick/locality behavior
follow the pinned source. C++ immutable value handles share private backing;
the safe-view cache avoids a strong ownership cycle. This documented internal
representation does not supply Kotlin/Native object identity or frame/root ABI.
The ROOT companion property uses a static root() getter to avoid C++ cross-unit
initialization order; a catalog-style initializer before main is exercised.
Generated ClassId components/copy/equality/hash reference the actual IR generator;
Boolean's1231/1237 hashes are preserved after source inspection corrected the
initial term. Strict debug/release syntax, Native-OFF LLVM builds, existing codegen
fixture, CTest and ASan/UBSan pass; the test includes64 upstream prefix cases.

FqName pathSegments/fromSegments and Unsafe pathSegments/collectSegmentsOf are
still absent pending actual List/ArrayList translation; no different collection
API or stub was added. ClassIdBasedLocality metadata/opt-in warning policy and
unused Unsafe SPLIT_BY_DOTS cache remain untranslated. Deep FqName:12/14 bodies,
1/1 type,0.71; Unsafe15/17,1/1,0.22; ClassId8/9,1/2,0.57. Its private nested
extension helper is implemented but not paired by lexical-scope name matching.
No target name parse errors; unsupported source emission and unrelated source
parser errors remain. Sparse compiler root is now661 source files:246/6807,
104/1554,0.36,4 failures. Full library remains663/2918,178/560,0.24,12 failures.
Both root reports/inventories are refreshed in build/source-continuation/
{compiler,library}-source-distance; source-scope changes prevent direct completion
percentage comparisons. Next translate the actual primitive catalog, list/metadata
dependencies, InlineClassesSupport and IR classification/type cache, then variable
and full frame/root/call/exception consumers. Both complete MLX/Native paths remain
unproven. See RESUME_ADDRESS_SOURCE_REPAIR.md's first section. Goal active.

**Binary-result checkpoint 43eb5c38 — 2026-10-08:** From 6cf05519,
BinaryType.hpp/.cpp translate native/base/.../BinaryType.kt:8-20: all nine
primitive kinds, the sealed Primitive/Reference family, typed lazy reference
sequences, nullability and primitive extraction. The internal common family
erases its out-type; Reference<T> retains the actual typed Sequence and Primitive
needs no fabricated Nothing object. This deliberate C++ adaptation is documented
in source. kotlin/sequences/Sequence.hpp translates the real stdlib interface,
using the existing Iterator covariance/ownership boundary. It never materializes
the sequence. Production LLVM targets include BinaryType.cpp.

Strict compile with/without NDEBUG, Native-OFF plugin/helper builds, the existing
code-generation fixture, new CTest and ASan/UBSan pass. The bounded fixture checks
nine primitive results, lazy provider identity and delayed mutation, covariance,
retention/release, borrowed elements and constrained-once propagation. It does
not create or classify fake IR classes. Actual IR classification, type caching
and frame consumers remain unwired.

Both root deep scans completed. BinaryType:1/1 explicit body,4/4 types,similarity
0.07; its normalized logic is provisional because emission falls back for the
source sealed class, enum and star-projected when function. Sequence:0/0 explicit
bodies,1/1 type; the tool forces a zero score because the source interface is
abstract and the C++ covariance bridge has a body. Both target units parse
without errors. Restoring libraries/stdlib/src/kotlin/collections at the same
pinned revision expands the sparse compiler corpus to652 Kotlin source files:
211/6715 bodies,101/1542 types,similarity0.34,4 scoring failures. These counts
cannot be compared as completion percentages with the earlier604-file corpus.
Full kotlinx.coroutines remains663/2918 bodies,178/560 types,0.24,12 failures.
Evidence is in build/source-continuation/{compiler,library}-source-distance.

Next translate KonanPrimitiveType and its real ClassId/FqName catalog dependencies,
then InlineClassesSupport and IrTypeInlineClassesSupport. DataLayout must use
that classifier and Runtime's structurally keyed IR cache, not an inferred LLVM
pointer test. VariableManager and complete frame/root/call/exception consumers,
generation-state entry and both complete MLX/Native acceptance paths remain open.
Goal active.

**Debug-bridge checkpoint 6cf05519 — 2026-10-08:** From c233adf7, four
actual debug operations from llvmDebugInfoC/DebugInfoC.cpp:253-286 are translated
in DebugInfoC.hpp/.cpp and registered on all three LLVM consumers. They preserve
the real DIBuilder, borrowed LLVM metadata/storage, source default debug options
and ordered declaration expressions. The compiler integration test verifies
storage/metadata identity, declarations before the terminator and an actual
LLVM23 module. Strict syntax, Native-OFF plugin/helper builds, CTest and ASan/UBSan
pass. The initial multi-target build regenerated CMake, then did not know the new
test target; its subsequent explicit build and execution pass.

Correction to the previous receipt: tmp/kotlin is sparse. The pinned Git tree
fee29910d8dddd2b1f7b44036c00533cee493351 contains the previously reported missing
sources. Restored native/base, kotlin-native/llvmDebugInfoC and
compiler/ir/backend.native reveal BinaryType.kt, InlineClasses.kt and
IrTypeInlineClassesSupport.kt at that same revision. Existing untracked Native
stdlib files were preserved. Binary classification, IR-keyed caches,
VariableManager's consumers, frame allocation/root operations and public
object-result calls still need translation; the new bridge alone does not
implement them.

Three deep scans completed. Full kotlinx.coroutines root:663/2918 bodies,
178/560 types,body similarity0.24,12 scoring failures. ChannelFlow remains
18/19,6/6,0.25; flow Channels remains12/12,0.22. The C++ bridge scan cannot pair
the files: upstream's LLVM conversion namespace is extracted as its package,
while our bindings use the compiler namespace. Direct comparison also uses
strict C++ names rather than mapping upstream PascalCase to snake_case; its
inventory sees28 source/4 target functions but certifies no matched bodies.
The current sparse tmp/kotlin root scan reports196/6201 bodies,88/1441 types,
similarity0.36,3 scoring failures across604 source files. This is the checked-out
compiler corpus, not the full pinned Git tree. CodeGenerator remains73/145,
6/15,0.24; ContextUtils38/51,31/36,0.47; LlvmUtils21/49,7/10,0.20.
Source conversion macros produce parser missing-semicolon diagnostics. Record
these tool limitations, not a four-body parity claim. Evidence is in
build/source-continuation/{debug,library,compiler}-source-distance. Actual frame/root/
call/exception consumers, generation-state entry and both complete standalone
MLX/Native handoff acceptance paths remain unfinished. Goal active.

**Current aggregate/runtime-type continuation — 2026-10-08:** From 62a3a77b,
ConstArray, Zero, const_value, RuntimeAware type/object-return/Nothing extensions
and unsigned extraction at LlvmUtils.kt:45-54,82-110 are translated. Array
metadata retains the actual shared element list while LLVM emission retains
its eager snapshot; source type diagnostics and explicit object-result flags
are preserved. Strict compile with/without NDEBUG, Native-OFF LLVM consumers,
existing code-generation fixture and bounded ASan/UBSan aggregate/import checks
pass. Scoped deep LlvmUtils:21/49 bodies,7/10 types,similarity0.20; target parsing
has no errors, generated source criteria remain provisional. VariableManager /
DataLayout tracing did not find binary-classification and LLVM debug bridge
definitions in the sparse working tree. This was incorrectly described as a
missing pinned snapshot; the current receipt corrects it. Do not infer enums, debug
options, IR cache equality or frame allocator callbacks. Other source work is
available. Actual frame/root/call/exception consumers, generation-state entry,
Runtime loader/caches, full-root measurements and both complete acceptance paths
remain open. See RESUME_ADDRESS_SOURCE_REPAIR.md's first section. Goal active.

**LLVM type/constant checkpoint 62a3a77b — 2026-10-08:** From 0ed4ddd4,
nine concrete constant classes, eleven LLVM types, source factories/raw-value
wrappers, literal/null properties, struct/packed/flexible-array helpers and
LlvmUtils Struct/type-string dependencies are translated. Shared Struct element
lists retain identity; emitted aggregates retain the source eager snapshot.
Signed conversion, UTF-16 and target intptr widths follow the source. Source
assertion diagnostics are preserved under the documented C++ NDEBUG mapping.
Strict compile with/without NDEBUG, Native-OFF LLVM targets, existing LLVM fixture
and bounded ASan/UBSan constant/import checks pass. Scoped deep ContextUtils:
38/51 bodies,31/36 types,similarity0.47; LlvmUtils:19/49,5/10,0.19. Two existing
empty-vector default spellings were made explicit after reproducing bundled
C++ grammar failures in the previous headers; target parse errors are now absent.
Generated source/class emission remains provisional. Continue actual frame and
allocation/root consumers, VariableManager, public object-result slot handling
and exception emission. Generation-state entry, Runtime loader/caches, full-root
measurements and both complete runtime acceptance paths remain open. See
RESUME_ADDRESS_SOURCE_REPAIR.md's first section. Goal active.

**Lazy runtime-binding checkpoint 0ed4ddd4 — 2026-10-08:** From 63eeae2f,
all forty ContextUtils.kt:445-504 lazy imports are translated, including volatile
heap-reference, safepoint, mark traversal, RC/ObjC and continuation bindings.
Thirty-eight getters are public; two type providers remain private. The explicit
compiler boundary now requires should_optimize so thread-state getters select
source optimized/_debug names. Source symbol/object-result metadata, actual
module import, descriptor identity and failed-lazy retry are retained. Strict
compile, Native-OFF LLVM consumer builds, existing LLVM fixture and a bounded
ASan/UBSan declaration harness pass. The harness executes all public getters and
both optimization policies; private providers are only compiled. Scoped deep
ContextUtils:16/51 bodies,22/36 types,similarity0.21,target 113 bodies; property/body
counting differs and source class emission remains provisional. Next translate
constant/type helpers, actual frame/allocation/root operations, VariableManager,
public object-result calls and exception emission. Generation-state entry,
Runtime loading/caches, full-root measurements and both complete runtime paths
remain open. See RESUME_ADDRESS_SOURCE_REPAIR.md's first section. Goal active.

**Runtime metadata checkpoint 63eeae2f — 2026-10-08:** From b5b16a95,
Runtime.hpp/.cpp translate compiler LLVM metadata: named/touch-global type lookup,
layout/target snapshots, header/frame types, lazy ObjC/block metadata, ABI layout
and pointer/alignment/string/byte-order properties. CodegenLlvmHelpers now takes a
borrowed Runtime and implements RuntimeAware; runtime imports use its actual
module and layout copying uses source snapshots. Native-OFF LLVM targets rebuild,
strict syntax compilation and bounded ASan/UBSan metadata/import fixtures pass.
Scoped deep Runtime:7/7 explicit bodies,2/2 types,similarity0.36;
ContextUtils:16/51 bodies,22/36 types,0.21. Unsupported source class emission keeps
normalized logic/span criteria provisional. Runtime's phase-context bitcode loader
and IR-keyed caches remain missing. Continue source frame allocation/root updates,
VariableManager, object-result calls and exception emission; do not substitute
callback hooks or pointer-keyed maps for missing compiler dependencies. Both full
acceptance paths and full-root measurements remain open. See the first section of
RESUME_ADDRESS_SOURCE_REPAIR.md. Goal active.

**Runtime-function import checkpoint b5b16a95 — 2026-10-08:** From df67badd,
CodegenLlvmHelpers binds actual supplied compiler/runtime LLVM modules, copies
layout/target and imports twenty-four eager source runtime symbols with actual
signatures, attributes and explicit object-result flags. Private function,
memset/intrinsic/runtime import operations and global-type helpers are translated.
No Native runtime implementation/link or Kotlin compiler dependency was added.
Strict compilation, LLVM target builds and a bounded ASan/UBSan declaration
fixture pass; that fixture does not exercise actual Native bitcode/runtime.
Scoped deep:ContextUtils16/51 bodies,22/36 types,similarity0.20;
LlvmUtils18/49 bodies,4/10 types,similarity0.19. Generation-state/RuntimeAware,
external prototypes/tracking and later lazy bindings remain absent. Continue
allocation/root operations, VariableManager, public object-result calls and
exception emission. Both complete acceptance paths and full-root measurements
remain open; see RESUME_ADDRESS_SOURCE_REPAIR.md's first section. Goal active.

**Module/annotation checkpoint df67badd — 2026-10-08:** From
61e3e6fe, BasicLlvmHelpers now reads the actual LLVM target triple and runtime
annotation operands with source lazy snapshots, grouping and pointer-policy
branches. get_as_c_string/get_operands retain source termination checks and
operand order. Strict compilation, plugin/helper builds and an LLVM-backed
ASan/UBSan annotation/snapshot harness pass on the opaque-pointer path. Older
typed pointers and zero-filled empty-string representation remain unverified.
Explicit line-free port-lint headers restore deep pairing for the edited files:
ContextUtils12/51 bodies,21/36 types,similarity0.17; LlvmUtils16/49 bodies,4/10
types,similarity0.17. Continue runtime imports, allocation/root operations,
VariableManager and public object-result calls. Full-root comparison and both
complete acceptance paths remain open. See RESUME_ADDRESS_SOURCE_REPAIR.md's
first section; the full goal remains active.

**Lifetime/slot checkpoint 61e3e6fe — 2026-10-08:** From
7e9d2aec, ContextUtils.hpp/.cpp translate every SlotType and Lifetime variant
from ContextUtils.kt:21-130. Static slot identities remain borrowed; dynamically
created parameter slots are owned by their creating lifetime. The same mutable
parameter-index array is retained by ParametersField and ParamsIfArena. Actual
source diagnostics and constructor properties are preserved. CMake registers
the implementation in existing LLVM targets. Strict compilation, plugin/helper
builds and an ASan/UBSan array identity/retention/release harness pass. The scoped
LLVM deep scan measures ContextUtils 12/51 bodies,20/36 types,similarity0.17;
source-emission criteria remain provisional. Continue actual allocation/root
operations and VariableManager, public object-result call selection, genThrow
and LLVM/runtime imports. The public call path is still unwired; full-root
comparison and both complete acceptance paths remain open. See
RESUME_ADDRESS_SOURCE_REPAIR.md's first section. The full goal remains active.

**Call/invoke checkpoint 7e9d2aec — 2026-10-08:** From
f98313b8, private call_raw and None/Caller/Local handler variants now mirror
CodeGenerator.kt's nounwind, caller-cleanup and local-unwind decisions. The LLVM
boundary borrows the actual supplied cleanup block. Public call/result slots,
Native frame/cleanup generation and ExceptionHandler.genThrow remain absent;
private call_raw has no production caller yet. Bit operations, integer extension/
truncation and signed/unsigned shifts are translated. Strict source compilation,
plugin/helper builds and the existing module-verifying fixture pass. Scoped deep
comparison measures CodeGenerator 73/145 bodies,6/15 types,similarity0.24.
Continue actual slot/frame/root/exception dependencies, VariableManager and the
connected expression emitter. Full-root comparison and both complete runtime
acceptance paths remain open. See RESUME_ADDRESS_SOURCE_REPAIR.md's first
section; the full goal remains active.

**LLVM value/build checkpoint f98313b8 — 2026-10-08:** From bc4c1b51,
14 LLVM value operations and the ten distinct nested attribute object types are
translated against the actual Kotlin source. Six connected implementations pass
strict syntax compilation. The root CMake build reconfigured and built
KotlinxCoroutinePass and kxs_codegen_test with LLVM 23.1.2, Native runtime OFF.
The code-generation fixture exited zero and verified/emitted its LLVM module.
SDK includes now use SYSTEM classification without weakening project warning
options; the injector handles LLVM 23 CondBrInst and preserves the older branch
API. Only LLVM 23 was freshly built. Scoped deep comparison records CodeGenerator
61/145 bodies, LlvmAttributes 2/6 bodies and 13/13 types; matching/emission
limitations and remaining source gaps are recorded in
RESUME_ADDRESS_SOURCE_REPAIR.md's first section. Focused checks support ongoing
translation; the earlier blanket check deferral is superseded. Continue actual
call/exception/frame/root operations, VariableManager, IR-derived signatures and
the connected expression/initializer emitter. Full-root measurements and both
complete runtime acceptance paths remain open; the full goal remains active.

**Typed-signature checkpoint bc4c1b51 — 2026-10-08:** From f542b371,
parameter/return type descriptors, singleton attribute kinds and supplied-LLVM
function signatures are translated. Signatures preserve separate function,
return and parameter attributes, vararg state and explicit object-return metadata.
Pointer/declaration/definition secondary constructors retain the actual signature
as their provider; nounwind checks now use the source kind cache. The two new
implementation files are registered in the existing LLVM targets. No configure,
build, test, AST emission or scan was run. Continue the IR-derived signature
factory (type conversion, ABI attributes and object-return slot parameter),
function prototypes/target attributes and exception/call/frame/root operations,
then VariableManager and the connected expression driver. Bridge debug metadata
remains absent. See RESUME_ADDRESS_SOURCE_REPAIR.md's first section. The full
goal remains active; executable acceptance remains deferred.

**Current callable/attribute continuation — 2026-10-08:** From d63d98ba,
LlvmCallable, function/pointer/declaration/definition operations and their actual
attribute providers are translated. Calls and invokes preserve provider attributes;
external declaration copying retains all attributes while call sites retain the
source enum/integer subset. Lazy LLVM properties preserve source snapshots.
ConstValue/ConstPointer and GEP/extract operations supply further storage
prerequisites. Typed FunctionGenerationContext borrows the actual definition;
conditional blocks now retain source location ranges. CMake registers the three
new implementations in the existing LLVM targets. No configure/build/test/scan
was run. Continue signatures/attributes, exception handlers and CodeGenerator's
actual call/frame/root-update operations, then VariableManager and the connected
expression driver. Signature-based constructors and bridge debug metadata are
still absent. See RESUME_ADDRESS_SOURCE_REPAIR.md's first section. The full
goal remains active; acceptance stays deferred until the translation is ready.

**Current LLVM memory/location continuation — 2026-10-08:** The continuation
from 1da27774 translates parameter reads, loads/stores with optional ordering
and alignment, location descriptors/ranges, recursive inline debug metadata,
block-location maps, builder restoration and zero-line reuse. Typed suspension
entries carry their actual IR start offsets through CodeContext.location.
The production resume-label injector now calls the translated load/store methods.
CMake already registers these implementation files and links the shared LLVM
package; no source-list or runtime-dependency change was needed. No configure,
build, AST emission, runtime check or deep scan was run. Continue actual frame
allocation, Native reference-update operations and concrete variable/function
contexts, followed by the connected typed expression driver and initializer
emission. See RESUME_ADDRESS_SOURCE_REPAIR.md's first section. The full goal
remains active; executable acceptance stays deferred until the translation is ready.

**Current typed-IR context continuation — 2026-10-08:** CodeContext.hpp now
mirrors IrToBitcode's full abstract code-generation context. Private inner scopes
forward actual operations to their outer context; suspension-point lookup binds
the actual IrVariable to its LLVM resume block address. Typed suspendable/
suspension-point evaluator entries use actual IR getters and source scope lifecycle,
normal/resume evaluation and phi-join order. The existing LLVM operand adapter
remains the production marker-injection entry. No normalized Clang-to-IR driver
calls the new typed overloads yet. General function/variable/debug/exception
contexts, IrType and partial aggregate initialization still need translation.
Read RESUME_ADDRESS_SOURCE_REPAIR.md's first section for source references and
CMake registration. No build, AST emission, runtime check or deep scan was run.
Continue the connected source translation; the full goal remains active.

**Source checkpoint 79e1c3c9:** The analyzer's
local, overload and suspension walks now use Clang's semantic initializer list
once, including selected member defaults and the array-filler expression.
Backward liveness, Native suspension/materialization discovery and extended-owner
reservation use that same evaluated tree. Tail collection follows defaults as
non-tail initialization operands. See RESUME_ADDRESS_SOURCE_REPAIR.md's first
section for source references and Kotlin provenance. No build, AST emission,
runtime check or deep scan was run. Next source work must translate actual
suspending aggregate/array emission: partial construction, member-default
receiver binding, implicit initializer expressions and repeated array fillers.
The initializer_list backing-array and immovable/default/access dependencies
remain open. The full translation goal remains active; acceptance is deferred.

**Source checkpoint 365a046b:** Nonsuspending
aggregate/array declaration lists preserve native element construction order.
Clang-marked temporaries whose extending declaration is the containing variable
now receive owning fields before that aggregate. Source and semantic-expression
rewriters construct those referents at their actual operands; existing lexical,
jump and terminal cleanup destroys the aggregate before its referents. Borrowed
external references stay borrowed. Suspension/materialization discovery and
overload deferral now follow selected member-default expressions consistently.
No build, AST emission, runtime check or deep scan was run. Read the first
section of RESUME_ADDRESS_SOURCE_REPAIR.md for source locations and provenance.
Next connected source work includes suspending aggregate/array lists, direct
immovable subobject construction, initializer_list backing arrays and implicit
member expression emission. Default parameter/access context, local integration
and optimized spilling also remain unfinished. The full translation goal stays
active; acceptance work remains deferred.

**Source checkpoint 425fb3bf:** Suspension discovery
and liveness now identify a selected default by its actual statement plus the
enclosing CXXDefaultArgExpr/CXXDefaultInitExpr use path. Nested defaults retain
all enclosing uses. Loop analysis merges the same occurrence; separate calls
using a shared declaration AST keep separate snapshots. Source locations and
Kotlin provenance are in RESUME_ADDRESS_SOURCE_REPAIR.md's first section.
CMake source review confirms the existing frontend/LLVM chain consumes the
edited analyzer. No build, AST emission, runtime check or deep scan was run.
Optimized spilling, declaration access context, immovable default parameter
construction, implicit bindings and aggregate/member expression slicing remain
unfinished. Continue translating those connected compiler dependencies; the
full translation goal remains active and acceptance work remains deferred.

**Source checkpoint ca15c58b:** Constructors and ordinary/
continuation calls now pass selected defaults through the connected expression
emitter. Suspension/materialization discovery sees the selected default AST;
ordinary call suffixes contain its lowered values explicitly. Named declaration
bindings and substituted type locations retain their originating resolved context,
and semantic printing preserves class-template scopes and explicit template
arguments. Operator receivers are excluded from authored parenthesized arguments.
No compilation, runtime check or deep scan was run. Occurrence identity for shared
default ASTs, implicit operator/literal bindings, private/protected access context,
immovable by-value parameter construction and aggregate/member initialization
remain unfinished. Read the new first section of RESUME_ADDRESS_SOURCE_REPAIR.md.
The full translation goal remains active; acceptance work is deferred.


**Source checkpoint 535f304a:** Indirect source jumps now evaluate their
address once, select only source address-taken labels and execute the existing
label-specific object/catch cleanup. The liveness visitor uses the same addressed
label set. Attributed control statements now carry branch/fallthrough/loop hints
onto their lowered operations; ordinary attributed calls retain their source form,
and common call policies surround the complete lowered operation. Generated
storage calls inherit that policy region, an explicit NOTE(port) deviation.
Tail collection preserves state through attribute wrappers. Other attribute
contracts, call-policy isolation, default-expression identity, optimized spilling
and earlier aggregate/local integration remain unfinished. Read the new first
section of RESUME_ADDRESS_SOURCE_REPAIR.md. No compilation, runtime check or deep
scan was run; source translation continues before acceptance work.


**Source checkpoint 8e57823f:** The liveness visitor now saturates actual Clang
label targets and propagates direct/indirect jump successors. NativeSuspendLowering
records each label's lexical declarations and catch handlers before emission;
direct goto releases objects absent from that target and preserves active
objects, including retained reference owners. Labelled statements now enter the
normal suspension lowering. Source locations and remaining gaps are recorded in
RESUME_ADDRESS_SOURCE_REPAIR.md's new first section. Indirect-goto cleanup and
attributed control-statement emission remain unfinished. Builds, runtime checks
and deep scans remain deferred; this is source progress, not acceptance evidence.



**Source checkpoint efde2c6c:** Compilation, runtime checks and deep scans are
now deferred at the user's direction until the connected translation is ready.
SuspendFunctionAnalyzer.cpp:284 replaces CFG suspension discovery with the
Native evaluated-call walk. Its private LivenessAnalysisVisitor at :342 ports
the common Kotlin backward visitor, including catch-live propagation, branch
merging, loop fixed points and jump targets. compute_liveness at :576 connects
its results to existing suspension metadata. C++ switch/for/range-for and lexical
function boundaries are adapted explicitly. Native frame storage still preserves
C++ lifetimes independently; optimized spill allocation, unstructured label/goto
analysis and shared default-expression identity remain incomplete. No build,
runtime or scan result is claimed for this source checkpoint.


**Latest compiler continuation:** ea4a97db and d97189ee construct aggregate
InitListExpr locals directly in aligned owning spill storage in
NativeSuspendLowering.cpp:1284-1298, preserving braces at the destination
new-expression. qualified_locals adds an immovable aggregate with a const
unique_ptr member, repeated-suspension resource identity checks and exactly one
destruction across completion/failure/cancellation. Strict fixture syntax and
actual AST emission pass; final strict lowering checking has only external
dependency diagnostics. Fresh plugin build exits 2; the older frontend fails
at the existing alias. No fresh runtime validates the repair. Extended lifetimes
of aggregate reference-member temporaries, aggregate/array expression slicing,
local nominal/dependent integration and full executable acceptance remain
unfinished. Read RESUME_ADDRESS_SOURCE_REPAIR.md's new top section. The full
goal remains active.

**Latest compiler continuation:** 2e4eb601 retains pure materialized constructor
reference arguments in NativeSuspendLowering.cpp:994-1006 and lowers the selected
constructor directly through its Clang functional-cast wrapper at :829-834.
qualified_locals adds an immovable ConstructorCondition that borrows literal
17 before a sibling suspends, with referent identity/value checks in its
destructor. Strict fixture syntax and actual Clang AST emission pass; strict
lowering checking has only external dependency diagnostics. Fresh plugin build
exits 2; the older frontend fails at the existing alias. No fresh runtime
validates the repair. Local nominal/dependent integration, aggregate/array
materialization and full executable acceptance remain unfinished. Read
RESUME_ADDRESS_SOURCE_REPAIR.md's new top section. Both full-root deep scans
exit 0 and leave generated reports unchanged. The full goal remains active.

**Latest compiler continuation:** deb8848c and ae390be1 preserve materialized
temporaries in non-suspending operands of split expressions at
NativeSuspendLowering.cpp:758-770,824-828. Existing call/branch slicing retains
their objects through selected sibling suspension. Pure materialized reference
arguments are retained at :1108-1130. qualified_locals adds skipped/executed
logical operands, both conditional arms, an immovable receiver and a borrowed
integer literal across suspension, with completion/failure/cancellation checks.
Final strict fixture syntax and actual Clang AST emission pass; direct strict
lowering checking has only external dependency diagnostics. Fresh plugin build
exits 2; the older frontend fails at the existing alias. No fresh runtime
validates the repair. Local nominal/dependent integration and broader
constructor/aggregate materialization remain unfinished. Read
RESUME_ADDRESS_SOURCE_REPAIR.md's new top section. Both final full-root deep scans
exit 0 and leave generated reports unchanged. The full goal remains active.

**Latest compiler continuation:** 0f60aa5b and ee88e8f5 preserve owned argument
temporaries through logical suspend-call completion in
NativeSuspendLowering.cpp:649-663,1121-1154. Sliced record prvalues use direct
owning construction for immovable types; borrowed glvalues keep their original
ownership. Transient argument cleanup follows the completion join, and owned
temporaries remain through the enclosing full expression. expression_slicing
adds two immovable objects, a borrowed argument across two suspensions, exact
reverse destruction assertions, and completion/immediate failure/resumed
failure/second-suspension cancellation modes. Its registered executable now
uses strict warning flags. Default syntax/Python parsing pass; direct strict
lowering checking has only external dependency diagnostics. Fresh plugin build
exits 2; the older frontend fails strict dependency/generated diagnostics.
No fresh runtime validates the repair. Local nominal/dependent import and full
executable acceptance remain unfinished. Read RESUME_ADDRESS_SOURCE_REPAIR.md's
new top section. Both full-root deep scans exit 0 and leave generated reports
unchanged. The full goal remains active.

**Latest compiler continuation:** 1e50f64a removes the 64-bit cutoff in retained
constant-value rewriting at NativeSuspendLowering.cpp:277-330. Wider values are
assembled in their actual integer type; negative values use -1 - complement,
including the signed minimum. Enum arithmetic uses its declared underlying
type before casting back. qualified_locals adds 128-bit positive/minimum/maximum
values, wider scoped enum constants, template/type assertions and retained
address checks across suspension. Final strict fixture syntax exits 0; direct
strict lowering checking has only external dependency diagnostics. Fresh plugin
build exits 2; the older frontend rejects the existing alias. No fresh runtime
validates this repair. Local enum nominal/lexical identity, broader local and
dependent import, and non-integral constants remain unfinished. Read
RESUME_ADDRESS_SOURCE_REPAIR.md's new top section. Both full-root deep scans
exit 0 and leave generated reports unchanged. The full goal remains active.

**Latest compiler continuation:** 0f47180e bounds helper parsing for in-class and
local method bodies to their complete enclosing lexical declaration in
CompilerFrameLowering.cpp:181-211. Declaration reuse at :48-68 maps offsets back
through the rewrite before applying that boundary and includes later members
while excluding the rewritten body. Local classes are not namespace-hoisted.
The registered late-include regression adds an in-class suspend method that
uses a later field/accessor. Default source syntax and Python parsing pass;
strict source syntax fails on existing coroutine dependency unused parameters.
Strict importer checking has only external dependency diagnostics; fresh plugin
build exits 2. The older frontend rejects generated GNU label code. No fresh
runtime validates the repair. Instantiated local-method scope and broader
dependent/local import remain unfinished. Read RESUME_ADDRESS_SOURCE_REPAIR.md's
new top section. Both full-root scans exit 0 and leave generated reports
unchanged. The full translation/state-machine goal remains active.

**Latest compiler continuation:** 077818e0 and 01b09e0d repair instantiated body
delivery in KotlinxSuspendPlugin.cpp:250-298. Canonical declaration/body tracking
prevents repeat lowering and rejects re-entry before body replacement.
CompilerFrameLowering.cpp:467,507,524 propagates failed consumer callbacks.
unit_tail adds recursive constexpr-selected template specializations and owning
frame cleanup assertions; its registered compile now has strict warning flags.
The first strict driver check caught a deprecated LLVM helper, corrected in the
second checkpoint. Final direct checks exit 1 in dependency headers without
source-local diagnostics; fresh plugin build exits 2. The older frontend reaches
the recursive case but rejects generated GNU label code. No fresh runtime
validates the repair. CMake's mandatory frontend/pass wiring was reviewed and
needed no change. Lexical class/lambda and broader dependent import remain
incomplete. Read RESUME_ADDRESS_SOURCE_REPAIR.md's new top section. The full
transliteration/state-machine goal remains active. Both full-root deep scans
completed with exit 0 and left generated reports unchanged.

**Latest compiler continuation:** 7d077d9a preserves resolved constexpr branch
selection in NativeSuspendLowering.cpp:495,1427, overload readiness, tail
collection, continuation edits and storage eligibility. Discarded arms create no
runtime branch storage; init objects retain their actual construction/cleanup
scope. Unresolved conditions defer until instantiation, whose broader revisit/
import pipeline remains incomplete. qualified_locals adds a false/no-else init
counter and an owning selected-arm guard across suspension, with discarded
calls/local class and cleanup assertions. Strict source syntax/AST emission exits
0. Direct strict compiler checks exit 1 in dependency headers, with no source-local
diagnostics; fresh plugin build exits 2. The older frontend rejects the alias.
No fresh runtime validates the repair. Read RESUME_ADDRESS_SOURCE_REPAIR.md's
new top section. Both full-root scans exit 0; compiler detail evidence refreshes,
while aggregate reports remain unchanged. The full goal remains active.

**Latest compiler continuation:** 12589756 preserves retained integral/enum
constant reads through Clang's non-ODR-use and constant-evaluation contracts.
NativeSuspendLowering.cpp:277,324,377 emits typed constant values in ordinary
expressions and template/type argument locations; address/reference uses retain
the actual stored object. qualified_locals adds integer limits, an enum constant,
template/static assertion uses and address identity across suspension. Strict
source syntax/AST emission exits 0. Final strict lowering syntax exits 1 in
dependency headers, with no source-local diagnostics; fresh plugin build exits
2. The older frontend rejects the alias declaration. No fresh runtime validates
this repair. Non-integral/wider extension constants and broader dependent/lexical
integration remain incomplete. Read RESUME_ADDRESS_SOURCE_REPAIR.md's new top
section. Both full-root scans exit 0 and leave generated reports unchanged.
The full transliteration/state-machine goal remains active.

**Latest compiler continuation:** 05a52132 preserves concrete decltype types and
original operand categories for C++ type queries. Suspend discovery, tail edits,
overload deferral and source checking respect unevaluated operands; local static
assertions retain their actual typed source. qualified_locals adds type/array/
reference/noexcept assertions and an ordinary unannotated query function.
Strict fixture syntax exits 0. Two Clang visitor API mismatches were repaired;
the repeated compiler translation-unit check exits 1 in dependency headers with
no source-local diagnostics. Fresh plugin build exits 2; the older frontend
rejects the alias declaration. No fresh runtime validates this repair. Dependent
types, constexpr-value assertions, evaluated polymorphic typeid, local classes
and nested invoke lexical integration remain incomplete. Read
RESUME_ADDRESS_SOURCE_REPAIR.md's new top section. Both full-root scans exit 0;
compiler detail evidence refreshes while aggregate reports remain unchanged.
The full goal remains active.

**Latest compiler continuation:** e522dd8c lowers copied-array decomposition in
NativeSuspendLowering.cpp:594,1156. The actual array source is evaluated once;
Clang's element AST emits native array construction, including nested copies
and native partial-construction cleanup. Array storage at :486 now destroys
elements in reverse order at every dimension. qualified_locals adds copy
independence/identity, source/copy counts and a throwing second copy with cleanup
order assertions. Strict source syntax/AST dump exit 0. Direct strict lowering
syntax exits 1 in dependency headers with no source-local diagnostics; fresh
plugin build exits 2. The older frontend rejects the alias declaration. No fresh
runtime validates this repair. Dependent decomposition, local classes and nested
invoke lexical integration remain incomplete. Read RESUME_ADDRESS_SOURCE_REPAIR.md's
new top section. Both full-root scans exit 0 and leave generated reports unchanged.
The full transliteration/state-machine goal remains active.

**Latest compiler continuation:** 90204a13 implements actual structured binding
registration in NativeSuspendLowering.cpp:1042,1123. Compiler-provided tuple
holding variables use the same variable lifetime path; member/array bindings
refer to their actual owner, including bit-fields. Implicit xvalue casts retain
rvalue get selection. qualified_locals adds array identity, bit-field mutation,
user get evaluation counts and an owning tuple resource. Source syntax and AST
dump exit 0. Final direct strict lowering syntax exits 1 in dependency headers,
with no source-local diagnostics; fresh plugin build exits 2. The older frontend
rejects the alias declaration. No fresh runtime validates the new bindings.
Copied array/dependent decomposition and broader lexical declaration integration
remain incomplete. Read RESUME_ADDRESS_SOURCE_REPAIR.md's new top section.
Both full-root scans exit 0 and leave generated reports unchanged.
The full transliteration/state-machine goal remains active.

**Latest compiler continuation:** c57aa777 implements local alias binding in
NativeSuspendLowering.cpp:335,462,1002. Each actual alias declaration receives a
unique frame name; type uses follow declaration identity and spill types retain
canonical cv-qualified identity. qualified_locals adds chained/typedef aliases
and nested shadowing across suspension. Ordinary strict source syntax exits 0;
strict lowering syntax exits 1 in dependency headers, with no source-local
diagnostics. Fresh plugin build exits 2 in dependency headers. The older plugin
rejects the fixture's local alias. No fresh runtime validation exists. Root
CMake frontend/LLVM integration was reviewed again. Full local class and nested
invoke lexical binding remain incomplete. Read RESUME_ADDRESS_SOURCE_REPAIR.md's
new top section. Both full-root scans exit 0 and leave generated reports unchanged.
The full transliteration/state-machine goal remains active.

**Latest builder continuation:** 776cc877 replaces all remaining FlowBuilders
manual frames/macros with source collection bodies. Concrete ranges now live in
Builders.cpp; function/container/iterator generic bodies remain in the header.
1edafccb and e3a4796d add owning flow/as_flow authoring projections and actual
ownership regressions. 265e4e5c uses the source public callback factory in its
suspension fixture. 2b91dc2f requires frontend lowering for core/tests even with
the in-tree plugin disabled; supplied external frontend configures successfully,
and missing frontend fails. Actual source, consumer and public probe checks exit
1; fresh core build exits 2 in dependency headers. No fresh runtime validation
exists. Final full-root scans exit 0 and keep c53d9f3a reports: Builders 15/23,
4/4 types, similarity 0.11. Read FLOW_BUILDERS_SOURCE_REPAIR.md's new top section.
The full translation/state-machine goal remains active.


**Latest compiler continuation:** d7feb4fb repairs retained bindings in actual
static initializers and extends qualified_locals. a4e0c319 adapts generated and
macro suspension regions to ordinary C++ marker branches; mandatory LLVM
injection creates their actual blockaddresses, stores the supplied field and
erases pairing IDs/conditions. Strict standalone frontend emission and test-input
IR verification pass; actual core syntax check still fails on dependency warnings.
Fresh frontend/LLVM builds exit 2 in dependency headers. Older installed pass
leaves the new markers unresolved, so neither repair has fresh runtime validation.
Both final full-root scans exit 0 and leave reports unchanged. Read
RESUME_ADDRESS_SOURCE_REPAIR.md and the updated IR specification. Local class and
lambda lexical declaration integration remains incomplete; the full goal is active.


**Latest source continuation:** 1bf6abd0, 050fca1f and 5570b19b translate Limit
operators directly from Limit.kt:17-140. Limit.hpp:43,62,100,154,200,252,297 now
contains no handwritten continuation frames or coroutine macros. Predicate boxes
are consumed before emission; abort ownership and cancellation follow the source.
Strict actual Limit.cpp and test_limit_suspension.cpp checks exit 1 on local
class/lambda namespace integration, generated code and dependency diagnostics.
No fresh executable validation exists. The consumed IR/CMake review remains in
MERGE_SOURCE_REPAIR.md; this batch changes library source, not compiler/CMake.
Both final full-root scans exit 0; reports are committed in 285a8a24. Limit is
8/8 matched bodies at similarity 0.07, with 23 target bodies and four types.
Read LIMIT_SOURCE_AUTHORING_REPAIR.md for source locations and exact receipts.
The full transliteration/state-machine goal remains active.


**Latest source continuation:** 7eb83ee8 translates running_fold, running_reduce
and chunked in Transform.hpp:704,799,893. Transform now has no handwritten frame
classes or coroutine macros. Typed source collectors retain actual owners,
operation-result boxes are consumed before emission, and each collection has its
own accumulator/buffer. 7687daed repairs duplicate grouped static declarations in
lowering and extends the existing qualification regression. Strict actual source
consumer and static fixture exit 1; fresh plugin build exits 2 in LLVM/Clang
headers. No runtime validation of the new bodies/repair exists. Both full-root
scans exit 0; reports are in 5345dbb0. Transform stays 12/13 at 0.07, target 61
bodies and 7 types. Read the new MERGE_SOURCE_REPAIR.md top section and current
API_AUDIT.md row. The full transliteration/state-machine goal remains active.


**Latest source continuation:** 8a49a5c5 translates with_index and on_each in
Transform.hpp:544,590, removing their manual frames. 7b70ebcf preserves declared
cv-qualification in spill fields; fd57ae36 emits loop continuation targets only
when authored continue statements reference them. 0c735ac3 adds an executable
qualification/identity/cleanup regression to the existing plugin harness. Actual
consumer and fixture checks exit 1; fresh plugin build exits 2 in LLVM/Clang
headers. Neither compiler repair has fresh executable validation. Both final
full-root scans exit 0 and leave d73e201c reports unchanged. Transform is 12/13
at 0.07, target 76 bodies and 8 types. Read MERGE_SOURCE_REPAIR.md's new top section
for exact receipts. The full transliteration/state-machine goal remains active.


**Current source continuation:** 825918fa and 07e682b0 replace filter,
filter_not and optional map_not_null manual frames with annotated source bodies
in Transform.hpp:170,220,452. 10da6bc6 initializes generated saved exception state
at NativeSuspendLowering.cpp:1140. Strict actual consumer exits 1; fresh plugin
build exits 2 in its LLVM/Clang dependency. The existing plugin does not contain
the new exception-state fix. Native-disabled CMake configuration exits 0 and
retains only the standalone handoff test. Both full-root scans exit 0; reports
are in 11232702. Transform remains 12/13 at 0.07 (target 82 bodies,9 types).
Read the new top section of MERGE_SOURCE_REPAIR.md for receipts and limitations.
Continue source transliteration and consumed lowering; the full goal is active.


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
