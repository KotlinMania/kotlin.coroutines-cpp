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
