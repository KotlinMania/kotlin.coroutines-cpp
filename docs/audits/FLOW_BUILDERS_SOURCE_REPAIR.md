# Flow builder source corrections — 2026-10-07

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
- Both full-root deep inventories are regenerated after source edits. The flow
  builder pair remains 11/23 functions and 4/4 types, body similarity 0.04
  (previously 0.05). The decrease is retained; symbol presence does not establish
  body translation, and source corrections are not altered to raise a score.

Remaining actual gaps include sequence surface, source range/array type mappings,
vector capture/identity correspondence, manual frames in other as_flow overloads,
ChannelFlowBuilder block to_string, broader KDoc/example correspondence and
compiler lowering. These are unfinished implementation work. No warning
suppression, alternate callback frame or fallback was introduced. Both complete
MLX executable acceptance paths remain unverified.
