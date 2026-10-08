# Continuation argument ordering — 2026-10-07

The annotated typed emit_all_impl at Channels.hpp:59 now has its shared
Continuation as the final parameter, after the two explicit supplied-owner
arguments. Existing four-argument raw and shared completion overloads forward
to this body. Fresh actual consumer compiler checks no longer report the
trailing-continuation defect and instead reach the still-unresolved generated
frame parse. Details and current receipts: COLLECT_SOURCE_AUTHORING_REPAIR.md.

# Translated documentation and immutable source fields — 2026-10-07

The complete common Flow Channels.kt and Channels.hpp were read before editing.
This remains the first dependency-impact group, with 65 dependents. All five
source KDoc blocks remain present; their references now use actual C++ method,
namespace and constant names. The consumeEach shorthand is translated to the
existing channels::consume_each continuation-call interface, including the real
collector emit call. The example's executable validity is not established by
documentation presence. Source branch comments identify direct collection,
additional buffering, fast repeated-consumption rejection and efficient receiving.

Channels.hpp:271-272 now declares channel_ and consume_ const, matching the two
private source val fields. The supplied shared owner and underlying channel remain
the same objects; the pointee is not made immutable. The atomic consumed field
continues to implement source getAndSet. No new continuation helpers or manual
state machines were added. Twenty-one ranged provenance references resolve to
existing Kotlin files and valid bounds. Receipt:
build/ir-recovery/flow-channels-doc-reference-check.json. This validates presence
and references, not exact whole-file translation or runtime behavior.

The full NopCollector Kotlin/header pair was read after its warning appeared in
the actual channel-flow consumer. NopCollector.hpp:17 is final as the source
object specifies. Its genuinely empty emit hook retains unused parameter names
as comments, avoiding warnings without suppression or dummy argument reads.
The source `does nothing` comment is retained. The existing typed specialization
is documented as a projection of Kotlin's contravariant Any collector; it does
not convert arbitrary C++ values. The canonical port-lint source path is restored.

Fresh verification:

- NopCollector<int>, NopCollector<std::string> and a move-only unique_ptr element
  instantiate with Clang C++20 -Wall -Wextra -Wpedantic -Werror: exit 0.
  Receipt: build/ir-recovery/nop-collector-source-strict.log.
- The actual CMake test_channel_as_flow_smoke target build exits 2 in compilation
  of compiler-plugin dependencies: LLVM/Clang 23 headers expose unused-parameter
  warnings. Receipt: flow-channels-doc-immutable-build.log under build/ir-recovery.
- Direct strict syntax compilation of the actual consumer using existing
  frontend/LLVM modules exits 1. NopCollector's two warnings are gone. Remaining
  dependency warnings, GNU label extensions, generated missing exception-context
  initializer and unresolved template T prevent an executable. Receipt:
  build/ir-recovery/flow-channels-doc-final-fixture.log. This does not verify
  suspended emission, ownership cleanup or the translated example at runtime.
- Both full-root deep inventories are refreshed after the source edits.
  Their body/parameter metrics and documentation metrics remain distinct.

The analyzer now rebuilds successfully in build/ast-identity and includes comment
words in primary literal fidelity. Earlier analyzer-build limitations recorded
below are historical; the compiler-plugin rebuild failure above is still current.
Channel diagnostic interpolation still formats a numeric address rather than the
actual channel string. Broader source/lowering and both MLX acceptance paths
remain incomplete.

# Current typed emission source body

The complete common Channels.kt and the C++ pair were reread. Channels.hpp:61
now contains the typed source emit_all_impl algorithm. The callback binding
bind_emit_all, EmitAllArguments manual storage and emit_all_erased entry are
removed. Channels.cpp is deleted because its only remaining body was that erased
callback loop. The generic public element type requires this header definition;
CMake's existing source glob owns regeneration. This supersedes the concrete
callback-loop migration described below.

The body checks collector activity before try, creates the actual channel
iterator, waits for has_next, deletes the owning bool result before emission,
reads the next typed element, and waits for the real collector emit call. It
preserves the caught cause and conditional consumed-channel cleanup on normal and
exceptional completion. Existing raw/owning overloads delegate to this body;
supplied owners bind their actual receiver pointers and remain owned arguments.
Raw borrowers are never adopted. Iterator and element are now ordinary local
variables for the frontend to spill, with no manual resume state or storage class.

Compilation of the actual test_channel_as_flow_smoke.cpp with the current strict
warning flags and previously built frontend/LLVM modules returns one. Existing
AbstractFlow generated unresolved T, exception-context initializer, label-address
and dependency diagnostics prevent a working executable. The typed emission
replacement has not executed and its retained iterator/element/owner cleanup
remains unverified. No suppression or alternate runtime frame was introduced.
Receipt: build/ir-recovery/flow-channels-typed-fixture-build.log. Historical
execution evidence below does not verify the current body. Complete source parity,
channel diagnostic text and both MLX acceptance paths remain unfinished.

Both absolute full-root deep scans finish with exit zero after these edits,
using the previous analyzer executable; strict analyzer rebuilding remains
unsuccessful. Channels retains 12/12 functions, 1/1 types and 65 dependents,
body similarity 0.22. Target function count is 18 (previously 19), target types
one (previously two). Whole-library totals are 824/2918 functions, 359/560 types,
body similarity 0.26 and 123 scoring failures. The one-function coverage decrease
comes from removal of the pool's unused private loop duplicate; its expanded
source call sites remain in allocate/close. The report is retained unchanged.
Compiler totals remain 592/7657 functions, 174/1727 types, similarity 0.36 and
24 failures. Deep receipts: flow-channels-typed-{library,compiler}-deep.log.
Root CMake regeneration also exits zero and removes both deleted Flow.cpp and
Channels.cpp from the core build commands. Receipt: flow-channels-typed-configure.log.
These checks do not establish execution of the new typed suspension body.

# Current source authoring migration — warning-visible build

The complete common Flow Channels.kt and C++ pair were reread. The first oracle
priority remains this group (65 dependents). Channels.cpp:16 replaces the manual
EmitAllContinuation class with an annotated concrete suspend entry. Its source
loop creates the iterator inside try, awaits has_next and emit_next, retains the
caught cause and runs conditional consumed-channel cleanup. The existing typed
bindings remain unchanged; actual supplied owners remain owned and raw arguments
remain borrowed. No new runtime continuation class or manual label/spill fields
were introduced. The receiving scope deletes the transferred bool box before
emission, on both immediate and resumed results.

This is an incomplete plugin migration, not fresh runtime validation. Compiling
the actual Channels.cpp with the current CMake target flags (-Wall -Wextra
-Wpedantic -Werror), and the previously built frontend/LLVM modules, reaches
emit_all_erased frame generation but returns one. The generated exception context
has a missing previous initializer and generated address-of-label expressions
trigger -Wgnu-label-as-value. No suppression or warning-policy reduction is used.
The current plugin modules themselves cannot be rebuilt because the unsuppressed
Clang/LLVM dependency diagnostics documented in WARNING_SUPPRESSION_REMOVAL.md
remain exposed. Existing runtime-test results below predate this migration.

ChannelAsFlow.mark_consumed at Channels.hpp:214 now directly mirrors the source
consume/atomic-get-and-set/check sequence. The separate concrete consumption helper
is removed. The concrete .cpp includes only the ABI types it consumes. The
public generic bindings remain in Channels.hpp; removing its unnecessary include
from .cpp prevents parsing unrelated channel implementation templates there.
It does not fix the outstanding warnings in those templates or dependencies.
Receipt: build/ir-recovery/flow-channels-authored-build.log. Full executable
ownership/failure/cancellation checks and both MLX acceptance paths remain
unverified for this new body. The diagnostic channel string mismatch also remains.

Both absolute full-root deep commands completed with exit zero after the final
source edits. They use the previously built ast_distance executable because the
strict analyzer rebuild is still unsuccessful. Flow Channels retains 12/12 source
function names and 1/1 source types, with body similarity 0.22 (previously 0.21)
and 65 dependents. Extra target functions decreased from 24 to 19 and types from
3 to 2. Library totals remain 825/2918 functions, 359/560 types, similarity 0.26
and 123 scoring failures. Compiler totals remain 592/7657, 174/1727, similarity
0.36 and 24 failures. These are incomplete translation measurements, not runtime
verification. Receipts: flow-channels-authored-{library,compiler}-deep.log under
build/ir-recovery. No prohibited comments or suppression controls occur in the
edited Channels source/header pair.

The historical checkpoints below describe the earlier handwritten frame, which
has now been removed. They do not establish execution of the current source.

# Flow channel emission source repair

Date: 2026-10-07. Complete common flow/Channels.kt and the existing C++ pair
read before editing. This is the first dependency-priority group, with 65
reported dependents. The public signatures and ChannelAsFlow constructor,
consumption, fusion and produce/collect selection retain their source contract.

`flow/Channels.cpp:28` now holds the concrete private EmitAllContinuation.
`Channels.cpp:47` mirrors source Channels.kt:28-41: create the iterator inside
try; suspend at has_next; unbox/delete the transferred bool result; emit each
next element; catch and preserve the failure; cancel_consumed in finally only
when consume is true. A finally exception supersedes the body exception and
cancellation executes once. Source ensure_active remains before the try/finally
boundary at `Channels.hpp:95`.

The old public header contained a generic continuation class and four repeated
frame startup implementations. The public generic overloads now bind typed
channel, collector, iterator and element storage at `Channels.hpp:88` to the
single concrete entry at `Channels.cpp:102`. This type erasure carries the actual
C++ types through the existing erased Continuation ABI; the iteration algorithm
and resume/failure branches remain in .cpp. The typed storage stays in a header
because arbitrary public element types cannot be explicitly instantiated. Shared
arguments retain their original owners; raw channel/collector arguments remain
borrowed. Both suspend points continue using the existing mandatory LLVM markers.

A regression exposed a real exception/resource retention bug. The old frame
cleared its iterator and owners but retained cause_ after failure, so an externally
owned completed frame kept an exception's captured C++ resource alive. At
`Channels.cpp:74`, termination now clears all bindings and the exception as well
as the suspended self root. These resources are released before releasing frame
interception, including when another owner keeps the completed frame alive.

The actual channel/flow executable built against the committed Channels.hpp
reported an ownership failure at test_channel_as_flow_smoke.cpp:464 and returned
one. The new frame returns zero for the same regression. It verifies channel and
collector identity/lifetime through two suspended emits, then clears legitimate
caller-owned exception references and checks resource release while the completed
frame survives. Twelve iterator/cleanup combinations additionally cover iterator
creation, has_next and next failures with consumption enabled/disabled and thrown
cancellation cleanup. Original failure identity and cancellation's original cause
are inspected. Existing repeated-suspension, buffered flow, queued producer,
actual channel cancellation and borrowed receiver cases continue to execute.

All nine focused CTest executables ran with zero failures (0.37 seconds). The
final actual channel/flow executable and both concrete Flow/Channels frames were
compiled with address and undefined-behavior sanitizers and the existing Clang
and LLVM plugins; execution returned zero without diagnostics. Receipts are
build/ir-recovery/flow-channels-*.log, including the committed-header reproduction.
These C++ executables need no Kotlin compiler or linked Native runtime; they do
not establish the complete Native interop/MLX acceptance paths or complete
library correctness. Twenty-nine provenance ranges in the two library files
were validated; neither file contains prohibited source markers.

Both full-root deep inventories are refreshed after the final source/test edit.
ChannelAsFlow's channel diagnostic interpolation remains a source mismatch:
Channels.hpp:267 formats an address instead of the actual channel string. The
ReceiveChannel interface has no polymorphic Any.toString projection. This
existing consumed-dependency gap remains incomplete, explicitly recorded in
API_AUDIT.md; this repair does not establish whole-file parity.

Final full-root library measurement: 772/2918 functions, 343/560 types, average
body similarity 0.26, 122 cheat/scoring failures. Channels retains 65 dependent
groups and body similarity 0.22 despite 12/12 matched function names. These
measurements do not establish complete source parity. Both deep commands
returned zero; the compiler/prerequisite report was refreshed without content
changes from this library slice.


## Protected hooks and concrete consumption check

The complete Kotlin Channels.kt and C++ header/source pair were reread before
this change. ChannelAsFlow.create and collectTo inherit protected visibility
from Kotlin ChannelFlow.kt:102-104. Their C++ overrides now preserve that
visibility at Channels.hpp:223 and :238. drop_channel_operators, produce_impl
and collect retain their existing interface accessibility.

The source markConsumed algorithm at Channels.kt:104-108 now lives concretely
in Channels.cpp:15. The generic receiver binds its actual consume flag and
atomic consumed field at Channels.hpp:217. It still checks consume before the
atomic get-and-set and throws the exact source IllegalStateException message
on a second consumption attempt. No resource ownership or suspension behavior
changes. The private generic class still requires a header definition for
arbitrary C++ element types; its NOTE(port) documents that language adaptation.

An initial relocation of the class into internal changed the source declaration
namespace and caused seven missing-function reports. The namespace was restored
to flow in commit 8f9547b2. Those reports were truthful; no analyzer matching rule
was changed. The final inventory restores 12/12 Flow Channels function names.
Its measured body similarity is 0.21, previously 0.22; moving a concrete body
behind a typed binding does not establish full source parity.

The final core and two focused executables build with exit zero. CTest runs
both test_channel_as_flow_smoke and test_channel_consumption with zero failures
in 0.70 seconds. A direct ordinary Clang syntax check verifies that create and
collect_to cannot be called publicly, while conversion to Flow and the public
produce_impl surface remain available. Thirty-one ranged source references
across the pair resolve with valid bounds; no banned source markers occur.
Receipts are build/ir-recovery/channel-private-final-{build,tests,access}.log.

Both required complete-root deep scans finish with exit zero. Final library
evidence is 820/2918 function names, 359/560 types, body similarity 0.26 and
123 scoring failures. The compiler/prerequisite report remains 591/7657,
174/1727, body similarity 0.36 and 24 failures. Logs are
channel-private-final-library-deep.log and channel-private-compiler-deep.log.
These bounded checks do not prove whole-library parity or either full MLX GPU
acceptance path. Channel diagnostic interpolation at Channels.hpp:267 and the
Native Throwable class-name contract remain incomplete source dependencies.
