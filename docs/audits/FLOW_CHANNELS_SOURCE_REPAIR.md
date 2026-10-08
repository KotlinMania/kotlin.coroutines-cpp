# Current source authoring migration — warning-visible build

The complete common Flow Channels.kt and C++ pair were reread. The first oracle
priority remains this group (65 dependents). Channels.cpp:29 replaces the manual
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

The concrete .cpp includes only the ABI and exception types it consumes. The
public generic bindings remain in Channels.hpp; removing its unnecessary include
from .cpp prevents parsing unrelated channel implementation templates there.
It does not fix the outstanding warnings in those templates or dependencies.
Receipt: build/ir-recovery/flow-channels-authored-build.log. Full executable
ownership/failure/cancellation checks and both MLX acceptance paths remain
unverified for this new body. The diagnostic channel string mismatch also remains.

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
