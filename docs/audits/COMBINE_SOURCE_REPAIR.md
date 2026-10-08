# Combine and zip source repair

Date: 2026-10-07. The complete common flow/internal/Combine.kt,
flow/operators/Zip.kt and flow/internal/FlowCoroutine.kt were read before editing.
The C++ Combine pair was clean before this work. Zip.hpp and the two operational
regression executables were also read before their required binding/test changes.

The former combine implementation launched std::threads, polled try_send and
try_receive, supplied null continuations to upstream collect, and detached
threads on destruction or transform suspension. The former zip implementation
used the same substitutions and a one-element channel instead of a rendezvous
channel. Both algorithms and their thread/frame helpers are removed.

`src/kotlinx/coroutines/flow/internal/Combine.cpp:129` now holds the concrete
combine body corresponding to source :17-80. It enters the existing flow_scope,
initializes latestValues with the actual UNINITIALIZED symbol, creates a channel
of source size and LocalAtomicInt, and launches one real child coroutine per
flow. Source child collection at :89 uses a retained collector and finally
closes the result channel when the last child exits. Each update sends and yields
through actual continuation operations. The batch loop suspends at its first
receive, updates latestValues and the absent count, stops on a duplicate source
in the current Byte epoch, and executes the actual null/copy array-factory
branches. Transform suspension retains its input array without consuming another
batch. Cleanup clears completed captures and spills even when the frame remains
independently held. Shared flows remain owned; raw downstream collectors stay
borrowed.

The generic bindings at `Combine.hpp:87,104` now expose arrayFactory and invoke
the concrete algorithm. The null-factory overload supports existing FunctionN
callers. The factory transfers an allocated vector to the frame, which owns it
through transform suspension and deletes it on completion/failure. The original
stack AnyCollector adapter could dangle after upstream collection suspended;
the typed binding now retains that exact adapter and upstream through the single
collect-call frame at `Combine.cpp:234`.

Zip's scope body at `Combine.cpp:357` uses the existing coroutineScope entry,
produces its second flow with a real rendezvous channel, creates Job(), and
cancels only that collection Job when second completes. First-flow collection
runs undispatched in scopeContext + collectJob. The collector restores the
original scopeContext for each receive/transform/emit sequence. The private frame
at :264 retains both suspend points and the actual result box. AbortFlowException
is checked against collectJob; finally cancels second on normal or exceptional
termination. No detached worker or polling loop remains in this pair.

The source nullable Any transport is represented by std::any inside the tagged
ChannelResult. It retains nullable payloads without using a Kotlin GC surrogate;
this is an explicit C++ value representation adaptation. Generic suspend zip
bindings at `Combine.hpp:194` and `flow/Zip.hpp:49` use the existing erased ABI.
The transform supplies an owning R box; the emit adapter unboxes/deletes it, or
frame cleanup deletes an unconsumed box. Ordinary C++ transforms bind to this same
algorithm. The two-input combine and combineUnsafe builders also now use the
source unsafeFlow; combineTransformUnsafe continues to use source safeFlow.

`src/tests/src/suspend/test_channel_consumption.cpp:312` exercises actual
channel-backed child flows, repeated suspension, batching during a paused
transform, both factories, Byte epoch wrap over 260 additional batches, exact
resumed failure, and completed-frame capture release. Its zip regression at :407
covers second completion while transform or downstream emit remains suspended,
original downstream Job identity/activity, separate resumed transform/emission
failure, and actual resource identity/release. The existing smoke executable now
uses real continuations and channel-backed flows instead of null continuations
and ignored suspended returns. It exercises final combined values, paired zip
values, both early-termination directions and exact failures from either source.
The different-owner AbortFlowException case reproduced a copied exception
(exit 1 at smoke line 126). The shared check_ownership extension is now concrete in FlowExceptions.common.cpp:11.
It preserves an active receiver's exact exception_ptr, matching Kotlin's
`throw this`, and distinguishes an unrelated active exception from the receiver.
Zip invokes that helper without a local exception-copy workaround. Receipts are
combine-zip-ownership-before-{build,tests}.log. Native constructor messages now
match source exactly for both AbortFlowException and ChildCancelledException.
The required dependency files were clean and read completely before editing.
The existing common companion had a different Kotlin file identity and no
implementation. The common header/extensions now identify FlowExceptions.common.kt;
Native classes/constructors live under native/flow/internal/FlowExceptions.hpp/.cpp.
The common expect class declarations include those actual definitions. This
removes the old conflicting file provenance without relaxing identity gates.
check_index_overflow's existing std::overflow_error mapping remains an explicit
C++ exception representation deviation from Kotlin ArithmeticException.

The complete core library and ten focused executables build. Ten CTests finish
with zero failures: BuildersTest, test_sync, test_suspension_core,
test_continuation_dispatch, test_channel_as_flow_smoke, test_sharing_suspension,
test_collect_reduce_smoke, test_cancellable_start, test_channel_consumption and
test_combine_zip_smoke. Receipts under build/ir-recovery are
combine-zip-final-build.log, combine-zip-smoke-build.log and
combine-zip-focused-tests.log. Combine.cpp, common/Native FlowExceptions implementations and both regression executables
are directly compiled with AddressSanitizer/UndefinedBehaviorSanitizer and the
existing Clang/LLVM plugins; execution finishes with exit zero and no diagnostics.
Receipts are combine-zip-sanitizer-{build,tests}.log and
combine-zip-smoke-sanitizer-{build,tests}.log. No Kotlin compiler or Native
runtime is invoked/linked in these component checks. Eighty-seven ranged
provenance references across the seven library files resolve; no prohibited
markers occur there. This does not establish the full Native/MLX acceptance paths.

Both complete-root deep scans finish with exit zero after final source/test
edits. Receipts are combine-zip-{library,compiler}-deep.log. The current library
report is 799/2918 matched body names, 358/560 types, average body similarity
0.26 and 123 scoring failures. Combine is now 2/2 body names and 1/1 types,
with body similarity 0.03. This small score is not overridden by the source repair.
The detailed evidence for both combineInternal and zipImpl shows unsupported
Kotlin function_declaration emission, an unmapped emitted source buffer and
normalized logic 0.000000. The split common FlowExceptions unit is now
2/2 body names and 2/2 types; Native is 2/2 types and 0/0 source body names.
The Native unit is newly scored ZERO because its Kotlin primary constructors
are represented as class declarations while C++ defines two explicit constructor
bodies. This adds one scoring failure; it does not establish a missing Native
constructor algorithm. Its initializer correspondence requires constructor-aware
measurement, not a fabricated Kotlin function or changed source provenance. The public names are recognized; emitted source/body
comparison remains provisional. The concrete correspondence is inspectable in
source and the executed cases above; name coverage is not a completion verdict.

## Public transform and array repair checkpoint

The public Zip.kt source was read in full again for this repair. Suspended value
transforms are now exposed for two through five typed inputs at
`flow/Zip.hpp:66,156,230,313`. They bind to combineUnsafe's source null-array-factory
path at `flow/internal/Combine.hpp:149`. The previously synchronous-only two-input
body is replaced by this binding; ordinary C++ transforms enter the same ABI
algorithm. The concrete lowered frame at `flow/Zip.cpp:15`, entered at :56,
corresponds to the source `emit(transform(it))`. It awaits transform before
awaiting emit, transfers its owning result box to the typed consumer and clears
captures at termination. The frame references the source input array, which the
actual combine frame retains until this entire operation completes. This is an
existing ContinuationImpl/Clang-plugin frame, not a separate polling algorithm.

The vector input surface at `flow/Zip.hpp:401` projects the source vararg/Iterable
flowArray and supplies the source copied-array factory. The earlier combine_all
null-factory route and inaccurate comment claiming that source route are replaced.
combine_all is a compatibility spelling that forwards to combine. Array
combineTransform at :444 invokes source safeFlow and the copied-array factory;
FunctionN combineTransform still uses source nullArrayFactory. The existing
vector adaptation carries typed values by value; std::any requires copyable
inputs, and this does not establish all Kotlin generic/covariant array behavior.

The shared typed result binding at `Combine.hpp:140` now recognizes Unit's
existing erased-null representation, used by ResultBoxCompletion. Both combine
and zip emit Unit without dereferencing a null box. Other results continue to
transfer an owning R box, unboxed and deleted at the receiving adapter. The
ordinary C++ Unit adapters also return the canonical null representation.

The public regressions at `test_combine_zip_smoke.cpp:92` execute all four typed
arities and a six-source array transform, separately suspending transform and
downstream emit. They check exact resumed transform/emission failures, actual
resource identity and release after completion while both frames remain held.
Unit cases exercise immediate and suspended transforms in both combine and zip.
Array cases execute ordinary combine_all, safe combineTransform and empty inputs
that must not invoke the transform. A fixture initially attempted to recollect
already consumed hot channels; its failure correctly identified an empty input
stream. The second collection now constructs fresh channels.

The full core library and the same ten focused executables build; ten CTests
finish with zero failures (2.54 seconds). Receipts are
build/ir-recovery/combine-transform-focused-{build,tests}.log. The smoke fixture,
Zip.cpp and Combine.cpp are directly compiled with AddressSanitizer and
UndefinedBehaviorSanitizer plus the existing mandatory LLVM/Clang plugins;
execution finishes with exit zero and no diagnostics. Receipts are
combine-transform-sanitizer-{build,tests}.log. Sixty-eight ranged provenance
references across the three changed library files resolve within source bounds;
no prohibited markers occur in those files. These checks do not establish the
full Native/MLX product acceptance paths or whole-library completion.

Both complete-root deep scans are refreshed after the final source/test edits.
The library report now records 803/2918 matched body names, 358/560 types,
average body similarity 0.26 and 123 scoring failures. Public Zip measures
13/18 matched names and body similarity 0.08. Its unresolved names include
common extension/top-level duplicate combineTransform and helpers physically
under flow/internal/Combine.hpp. These findings remain visible; matching names
or exercised behavior does not establish full source parity. The existing
Native primary-constructor scoring limitation remains documented above and
has not been hidden with altered provenance or synthetic source functions.
Scan receipts are combine-transform-{library,compiler}-deep.log.

Broader typed/nullability coverage and full Kotlin test transliteration remain
incomplete. This checkpoint does not declare public Zip.kt or the library complete.
