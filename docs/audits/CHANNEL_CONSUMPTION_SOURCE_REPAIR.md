# Channel consumption source repair

Date: 2026-10-07. Source authority is the complete common Flow Channels.kt and
channels/Channels.common.kt, read before editing their C++ pair. Flow Channels is
first in the dependency report, with 65 dependents. Its emission finally path
depends on channels.cancelConsumed, whose previous C++ body forwarded every cause
unchanged.

`channels/Channels.common.cpp:11` implements the actual nullable cause mapping:
null remains null; an existing CancellationException keeps its exception_ptr
identity; every other cause is retained inside a new CancellationException with
the source message "Channel was consumed, consumer had failed". The concrete
mapping stays in .cpp; the element-type wrapper is at `Channels.hpp:187`.
`flow/Channels.hpp:336` now throws the declared IllegalStateException on repeated
consumption instead of a generic std::logic_error.

The consume overloads at `channels/Channels.hpp:215,232` retain return values,
references and move-only results. Cleanup after successful block execution sits
outside the block's catch. A cancellation failure therefore supersedes the block
outcome without triggering a second cancellation. On block failure, the original
cause is supplied to cleanup and rethrown if cleanup finishes normally.

consume_each at :266 uses the source ChannelIterator.has_next/next loop. It waits
at an empty open channel through the erased Continuation ABI instead of breaking
out of a try_receive loop. The inline action's lowered overload supports repeated
action suspension, resumed failure and source ordering. The ordinary nonsuspending
C++ action overload at :333 adapts its callback into that same path. Both paths
use the existing ContinuationImpl and LLVM-injected DSL. Termination releases the
iterator, spilled element and captured action after cancellation/finally. Raw
channel arguments remain borrowed and must remain valid throughout suspension.

to_list at :360 constructs its vector through consume_each and returns it only
after the channel completes. It waits for later elements and propagates channel
failure. The receiving caller owns and deletes the vector box for both immediate
and resumed completion. Failure releases the partially collected values.

The complete library and eight targeted executables build. Eight CTest executables
finish with zero failures: test_channel_consumption, test_channel_as_flow_smoke,
BuildersTest, test_sync, test_suspension_core, test_continuation_dispatch,
test_sharing_suspension and test_collect_reduce_smoke. The new test uses real
BufferedChannels for repeated receive suspension, repeated inline action
suspension, original failure propagation, cancellation wrapping/identity,
exactly-once cleanup, finally failure precedence, reference and move-only block
results, immediate/resumed vector results, capture cleanup, and collection's
single-consumer guard. Shared C++ values retain their resource identity across
suspension; deleting the completed vector or failing collection releases them.

Build receipts are `build/ir-recovery/channel-consumption-final-build.log` and
`channel-consumption-resource-build.log`; execution is recorded in
`channel-consumption-final-tests.log`. The same test and concrete cause helper
also compile directly with Clang, the existing LLVM module plugin and the core
C++ archive, with AddressSanitizer/UndefinedBehaviorSanitizer, without invoking
Kotlin tooling or linking Native runtime libraries. This bounded component check
does not establish the complete ordinary C++/MLX and actual Native handoff paths.
Receipts are `channel-consumption-sanitizer-final-build.log` and
`channel-consumption-sanitizer-final-tests.log`.

Both full-root deep reports are refreshed after the final edits. Function and
type name totals remain 781/2918 and 341/560, average body similarity 0.26 and
scoring failures 122. Common Channels remains 4/6 matched body names; similarity
is 0.04 (previously 0.05). The detailed evidence marks its in-memory Kotlin
emission as unsupported/provisional and records no emitted logic for the affected
functions. That explains why the normalized logic measurement cannot establish
their fidelity; this repair does not override or replace the generated scores.
The reports still identify absent deprecated receiveOrNull/onReceiveOrNull APIs.

This is not a complete Common Channels port. Kotlin inline nonlocal returns need
the actual authoring/lowering contract; std::function callbacks do not express
them. Full KDoc examples and deprecated API dependencies are incomplete. Existing
channel factory/blocking helpers in the mixed header require separate source
reconciliation. ChannelAsFlow diagnostic text still renders a numeric pointer
instead of the source channel text, as recorded in API_AUDIT.md.
