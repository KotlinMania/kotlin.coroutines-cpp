# Flow builder source corrections — 2026-10-07

## Direct collection source continuation

776cc877 replaces every remaining handwritten builder continuation with direct
source collection bodies. FlowBuilders.hpp:138,153,164,175 now translates the
suspending function, iterable, array and iterator bodies. Builders.cpp:11,27
holds concrete Int/Long collection loops; :43,52 holds the public factories.
The header contains their declarations at :320,324. No builder frame class or
coroutine macro remains. Generic bodies require header definitions for arbitrary
public element/iterator types; concrete private range logic lives in .cpp.

Actual function/container arguments retain their existing C++ values in the
compiler-created frame. The vector keeps its preexisting snapshot-per-collection
projection. Arrays and iterated storage remain borrowed; iterator cursor ownership
remains shared across collections. Iterator next/advance occurs before emission,
and inclusive ranges stop at last before incrementing, including maximum values.
The function result is unboxed into the actual value and its owning result box
is deleted before downstream suspension. No runtime frame or self-cycle is added.

1edafccb and e3a4796d add owning Continuation projections for public flow at :99
and suspending function as_flow at :214, retaining the raw and ordinary overloads.
The vararg/initializer-list factory now directly uses its source collection body,
rather than calling as_flow. Existing owning C++ vector storage is retained for
initializer-list values. The Fibonacci example uses compiler-authorable suspension
and a bounded fixed-width example; BigInteger/documentation parity remains open.

The actual BuildersTest now retains supplied continuation owners in the changed
asynchronous callees and collector. It checks callable owner cleanup after the
flow is destroyed during producer suspension, downstream suspension, failure and
cancellation. 265e4e5c repairs the older callback fixture to collect the real
public callback_flow, replacing its old namespace/private-hook access and checking
actual IllegalStateException transport. These tests have no fresh execution yet.

2b91dc2f repairs root CMake integration: disabling the in-tree frontend build no
longer skips frontend lowering on the core or coroutine executables. An external
frontend is required instead. Public module documentation distinguishes LLVM-only
explicit ABI marker regions from compiler-authored bodies/generic builders.
The actual Builders.cpp unit was already in the core source inventory, so no
new source-registration mechanism was needed.

Verification:

- Actual Builders.cpp strict source check exits 1; annotated collection entries
  are recognized, but the existing plugin emits GNU label-address code and
  dependencies fail under the unchanged warning policy.
- The final actual BuildersTest and public overload probe both exit 1 on generated
  frame/template/local declaration and dependency diagnostics. The probe uses
  raw/owned functions, owned flow, containers, iterators, varargs and both ranges.
  Its earlier unqualified flow-name ambiguity was corrected. Final checks have
  no ambiguous/conversion/old callback private-hook diagnostics in that search.
  Failure of compilation prevents executable ownership/cancellation validation.
- Native-disabled root configuration exits 0. External-frontend configuration,
  with the existing explicit diagnostic-tool path, exits 0. Missing external
  frontend configuration exits 1 at the required plugin check. Both successful
  compile inventories put frontend and LLVM stages on Builders.cpp/BuildersTest.
- Fresh core build exits 2 in the metadata plugin's LLVM/Clang dependency headers.
  No warnings were suppressed; existing binaries are not claimed as new validation.
- Both exact full-root deep scans exit 0 after the final source changes and without
  concurrent source edits. Final reports remain at c53d9f3a. Builders measures
  15/23 bodies, 4/4 types, similarity 0.11, with 32 target bodies and five target
  types. The previous checkpoint measured 0.12, 39 target bodies and eight types.
  Name/body scores do not establish completed translation; source receiver/type
  representations and inventory matching still leave eight named gaps.
- Library totals: 832/2918 bodies, 359/560 types, similarity 0.26, 123 scoring
  failures. Compiler totals: 592/7657 bodies, 174/1727 types, similarity 0.36,
  24 failures. Compiler reports remain unchanged. All 37 ranged source references
  resolve, and changed builder source has no manual frames/macros or prohibited
  markers. This check establishes bounded provenance/structure only.

Receipts under build/ir-recovery: builders-source-authoring.log,
builders-public-probe-final.log, builders-consumer-final.log,
builders-root-configure.log, builders-external-configure-final.log,
builders-missing-frontend-configure-final.log, builders-core-build.log,
builders-complete-library-deep.log, builders-complete-compiler-deep.log and
builders-source-reference-check.json. The first external configuration attempts
stopped at the test-only diagnostic tool lookup; the final receipts supply its
actual existing path. Production does not use that serialized-IR diagnostic path.

Remaining source gaps include Sequence, Native primitive-array/range bindings,
vector receiver identity correspondence, ChannelFlowBuilder block to_string,
KDoc parity, local class/lambda integration and executable lowering. Both complete
C++/MLX and Native shared-state GPU acceptance remain unproven. The full goal is
active; this batch changes library authoring/CMake, not compiler lowering source.

## Historical factory-selection checkpoint

The complete Kotlin flow/Builders.kt, its C++ FlowBuilders.hpp counterpart and
the inherited ChannelFlow Kotlin hooks were read. The source file imports
internal.unsafeFlow as flow; its asFlow and flowOf bodies therefore do not call
the public SafeFlow builder declared in the same file.

Eight C++ factory paths now call internal::unsafe_flow: ordinary and suspend
functions, vector, array, iterator, single-value flow_of and the two ranges.
The vararg/initializer-list path delegates through the corrected vector factory.
Previously these paths added a SafeCollector that their Kotlin bodies do not
construct. Public flow retains SafeFlow. Its source-private SafeFlow type is
final, its block is immutable and its constructor moves the supplied callable.
The genuinely empty source EmptyFlow object is also final.

CallbackFlowBuilder::collect_to at FlowBuilders.hpp:515 now directly awaits the
parent collect_to, checks the actual producer channel's closed-for-send state and
throws IllegalStateException with the exact source diagnostic. The previous
std::logic_error transport and handwritten continuation frame are removed.
The source explanatory comment uses the C++ await_close name; diagnostic strings
retain Kotlin spelling as part of the compatibility contract. Both builder
block fields are immutable as their source val declarations require.

ChannelFlowBuilder and CallbackFlowBuilder now have the source declaration
namespace kotlinx::coroutines::flow. Their base is internal::ChannelFlow, imported
by the source file. Factories use these actual types. Their create and collect_to
overrides inherit protected visibility from ChannelFlow.kt:102-104. No other
production references to the old class namespace were found. Generic private
types still require header definitions for arbitrary C++ element types; the
NOTE(port) states that language adaptation. The source's named-anonymous-object
and channel-chain comments are retained.

The remote-call KDoc example is restored to the suspend-function as_flow overload
and expressed using the existing Continuation ABI, rather than placed on the
ordinary-function overload. Thirty-seven ranged source references resolve within
the Kotlin source bounds. Receipt: build/ir-recovery/flow-builders-final-source-check.json.
This verifies references and selected structure, not exact whole-file fidelity.

Fresh verification:

- Actual channel-flow and collect/reduce strict consumer compilations after the
  builder-selection edit both exit 1. Receipts: flow-builders-source-channel-fixture.log
  and flow-builders-source-collect-fixture.log under build/ir-recovery.
- Final explicit int builder instantiation through the existing frontend/LLVM
  modules reaches CallbackFlowBuilder's annotated collect_to. Compilation exits
  1 on generated unresolved T, frame parse failure, exception-context initializer
  and strict dependency/label diagnostics. Receipt:
  build/ir-recovery/flow-builders-final-callback-fixture.log.
- Final actual channel-flow consumer compilation also exits 1. Receipt:
  build/ir-recovery/flow-builders-final-channel-fixture.log. No successful fresh
  executable, callback suspension or ownership cleanup result is claimed.
- Both full-root deep inventories are regenerated after source edits. Restoring
  declaration namespaces changes the flow builder pair from 11/23 to 15/23
  functions, with 4/4 types and body similarity 0.12. The intermediate selection
  repair measured 0.04 (previously 0.05); both measurements are retained in Git.
  Source corrections are not altered to raise a score. The final library measures
  831/2918 functions, 359/560 types, average body similarity 0.26 and documentation
  similarity 0.38, with 123 scoring failures. Symbol presence does not establish
  complete body translation or execution.

At that checkpoint, remaining gaps included sequence surface and range/array mappings,
vector capture/identity correspondence, manual frames in other as_flow overloads,
ChannelFlowBuilder block to_string, broader KDoc/example correspondence and
compiler lowering. These are unfinished implementation work. No warning
suppression, alternate callback frame or fallback was introduced. Both complete
MLX executable acceptance paths remain unverified.
