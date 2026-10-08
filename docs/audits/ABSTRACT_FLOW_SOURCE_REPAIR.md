# Current typed source authoring migration

Date: 2026-10-07. Complete common Flow.kt and the existing C++ pair reread.
Flow remains second in dependency priority (28 groups). The manual CollectFrame
and callback-based collect_abstract_flow entry are removed. Flow.cpp is deleted:
its only implementation was that source-invented frame. CMake's existing
CONFIGURE_DEPENDS source glob owns the source-list refresh.

AbstractFlow.collect at Flow.hpp:299 now directly constructs the typed
SafeCollector, invokes collect_safely and releases interception on both successful
and exceptional paths. The generic source body requires a header definition for
arbitrary element types. The current source try/finally is expressed as catch
cleanup/rethrow plus successful cleanup outside the catch, so a cleanup failure
is not caught and cleanup is not repeated. The raw virtual entry at :290 retains
the caller through the existing ownership binding and calls the owning overload.
An existing shared flow owner remains a local through suspension; stack and raw
flow receivers remain borrowed. Frame creation, local spilling and resume dispatch
are assigned to the existing Clang frontend and mandatory LLVM injection.

This migration is not verified by execution. Compilation of the actual
src/tests/src/suspend/test_channel_as_flow_smoke.cpp with -Wall -Wextra -Wpedantic
-Werror and the previously built compiler modules exits one. The frontend finds
the actual collect specialization but its generated frame contains an unresolved
T identifier, an incomplete exception-context initializer and GNU label-address
diagnostics. Dependency unused-parameter/macro diagnostics remain visible too.
No warning suppression is introduced. Current plugin rebuilding still fails on
the installed SDK diagnostics recorded in WARNING_SUPPRESSION_REMOVAL.md.
Receipt: build/ir-recovery/abstract-flow-authored-fixture-build.log. Earlier runtime
receipts below predate the removal and do not prove current execution, retention,
cleanup or cancellation behavior. Both required MLX acceptance paths remain
unverified; complete source parity remains unfinished.

The historical checkpoints below describe earlier implementations.

# AbstractFlow collection source repair

Date: 2026-10-07. Complete common flow/Flow.kt read alongside the C++ pair
and its consumed Native SafeCollector source. Flow is second in dependency
priority, with 28 dependent groups. Channels, the first group, was read and its
existing iterator/catch/finally algorithm was retained.

`flow/Flow.hpp:295` constructs the source SafeCollector with the actual caller
context and binds public generic element types to the existing Continuation ABI.
`flow/Flow.cpp:16` holds the concrete collection frame; `:32` invokes
collect_safely and releases the collector in both normal and exceptional finally
paths. The successful cleanup is outside the collection catch, so a cleanup
exception does not execute cleanup twice. This is the lowering of common
Flow.kt:223-230 through the existing Clang/LLVM suspension markers.

The old header-local frame kept flow_owner and safe_collector after termination
if another owner retained the completed continuation. The regression added to
`src/tests/src/suspend/test_channel_as_flow_smoke.cpp:565` deliberately retains
that actual frame: before the source change, the executable reported an ownership
failure at line 573. After the change, actual flow/capture weak references expire
on normal completion, resumed failure and cancellation while the frame remains
owned. `Flow.cpp:47` clears concrete frame bindings and its suspension root at
termination; `Flow.hpp:301` retains an existing shared flow owner during
suspension. Stack/raw receivers remain borrowed. The added stack-flow case at
`:583` exercises immediate success/failure, original exception identity, no
premature destruction and direct return without completing the caller again.

Verification receipts are under build/ir-recovery/abstract-flow-*.log. All nine
focused CTest executables ran with zero failures (2.30 seconds). The actual
channel/flow executable and new Flow.cpp were also compiled with address and
undefined-behavior sanitizers and the existing Clang suspension and LLVM plugins;
execution returned zero with no diagnostics. No Kotlin compiler or Native runtime
was needed for this C++ executable. This does not establish the complete Native
interop/GPU acceptance paths or full flow parity.

Provenance ranges in both changed library files were validated against the pinned
Kotlin files; neither contains prohibited comment markers. Public collect and
abstract collect_safely signatures remain the source API. Native SafeCollector's
release_intercepted body is genuinely empty in its source; it is unchanged.

A reproduced measurement defect introduced by the valid private helper split was
fixed in ast_distance; see AST_DISTANCE_PRIVATE_COMPANION_REPAIR.md. Both complete
root deep reports are refreshed after source/tool changes. Name coverage does not
establish body parity; outstanding source gaps remain represented by the oracle.

SafeCollector ancestry follow-up (2026-10-07): common SafeCollector.kt:92-97
checks the actual ScopeCoroutine<*> type before following its parent. The old
C++ JobSupport/is_scoped_coroutine flag test allowed an unrelated job to bypass
this check. SafeCollector.cpp:22,31 now checks the erased identity of actual
ScopeCoroutine specializations. ScopeCoroutine.hpp:40 can only be constructed
by ScopeCoroutine<T>; it supplies no algorithm or alternate state machine.
The source's final scoped property is preserved. The regression at
test_channel_consumption.cpp:376 failed at :388 before the fix and now exercises
real void*/int scope chains, unrelated flag-bearing jobs, null/identical parents,
Native context rejection, downstream exception identity and real relationship
cleanup. Ten focused executables finished with zero failures before checkpoint
e42543fc. Receipts: safe-collector-ancestry-focused-{build,tests}.log and
safe-collector-ancestry-before-tests.log. This source fix is included in that
full-tree recovery commit; it does not close every SafeCollector mismatch.

SafeCollector checked-cast follow-up (2026-10-07): the nullable collection Job
cast and non-null emission Job cast at common SafeCollector.kt:32-33 now use
checked RTTI reference casts at SafeCollector.cpp:65,68. Previously, nullable
C++ dynamic_pointer_cast results converted malformed non-Job elements into
absent Jobs. A forged Job-key emission into an empty collection context could
therefore reach the downstream collector. The new regression at
src/tests/src/suspend/test_channel_consumption.cpp:471 reproduces that behavior
before the source change, failing at :496. Receipt:
build/ir-recovery/safe-collector-cast-before-tests.log.

The collection cast runs first, accepts actual null, and rejects a non-Job
object. The emission cast then requires an actual Job. Shared-pointer bindings
retain the original context owners; no borrowed object is adopted. Failed
validation occurs before the Native lastEmissionContext assignment or downstream
emission. C++ uses std::bad_cast for its checked RTTI cast, explicitly documented
with NOTE(port); this does not claim a completed Native Throwable/exception
interop mapping. The regression exercises malformed collection and emission
elements, repeated failed validation, matching real Jobs and canonical empty
contexts. Existing actual ScopeCoroutine ancestry cases remain exercised.

The consumed Native class surface is also aligned: SafeCollector.hpp:70 marks
the class final, matching Kotlin's closed class; :84 exposes the source's
read-only collector property. Constructor parameters and the private collector
field follow the source names in snake_case. Getter provenance at :48,50,84
names the exact Native property lines. The regression verifies original collector
and context identity plus actual context element counts, including malformed
contexts whose constructor fold still follows the Native source. No ownership
policy changes accompany these property bindings.

After the final header/source/fixture changes, the core and ten focused
executables build and all ten CTests report zero failures (2.46 seconds).
The changed SafeCollector.cpp and full regression fixture are directly
instrumented with AddressSanitizer and UndefinedBehaviorSanitizer, using the
existing frontend and mandatory LLVM plugins; execution exits zero without
diagnostics. Receipts: safe-collector-cast-focused-{build,tests}.log and
safe-collector-cast-sanitizer-{build,tests}.log. All thirteen ranged provenance
references across the SafeCollector pair have valid source bounds; neither file
contains prohibited markers. Receipt: safe-collector-cast-provenance.log.

Both complete-root deep scans finish with exit zero after final source changes.
Library totals remain 811/2918 body names, 359/560 types, average body similarity
0.26 and 123 scoring failures; the extra C++ property getter is not presented as
new upstream function parity. The compiler/prerequisite scan is refreshed and
unchanged. The inventory still lists the checkContext extension under its Kotlin
receiver while the C++ concrete algorithm resides on SafeCollectorBase; the
source/type-erasure structural gap is visible and no matching rule is weakened.
These bounded checks do not establish full SafeCollector source parity or the
required Native/MLX product acceptance paths. Deep receipts:
safe-collector-cast-{library,compiler}-deep.log.
