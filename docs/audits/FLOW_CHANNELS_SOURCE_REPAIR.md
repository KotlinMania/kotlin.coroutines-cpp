# Flow channel emission source repair

Date: 2026-10-07. Complete common flow/Channels.kt and the existing C++ pair
read before editing. This is the first dependency-priority group, with 65
reported dependents. The public signatures and ChannelAsFlow constructor,
consumption, fusion and produce/collect selection retain their source contract.

`flow/Channels.cpp:16` now holds the concrete private EmitAllContinuation.
`Channels.cpp:35` mirrors source Channels.kt:28-41: create the iterator inside
try; suspend at has_next; unbox/delete the transferred bool result; emit each
next element; catch and preserve the failure; cancel_consumed in finally only
when consume is true. A finally exception supersedes the body exception and
cancellation executes once. Source ensure_active remains before the try/finally
boundary at `Channels.hpp:95`.

The old public header contained a generic continuation class and four repeated
frame startup implementations. The public generic overloads now bind typed
channel, collector, iterator and element storage at `Channels.hpp:88` to the
single concrete entry at `Channels.cpp:90`. This type erasure carries the actual
C++ types through the existing erased Continuation ABI; the iteration algorithm
and resume/failure branches remain in .cpp. The typed storage stays in a header
because arbitrary public element types cannot be explicitly instantiated. Shared
arguments retain their original owners; raw channel/collector arguments remain
borrowed. Both suspend points continue using the existing mandatory LLVM markers.

A regression exposed a real exception/resource retention bug. The old frame
cleared its iterator and owners but retained cause_ after failure, so an externally
owned completed frame kept an exception's captured C++ resource alive. At
`Channels.cpp:62`, termination now clears all bindings and the exception as well
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
