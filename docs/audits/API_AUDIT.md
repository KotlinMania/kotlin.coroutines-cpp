# kotlinx.coroutines C++ API Audit

This document tracks the API completeness of our C++ transliteration against the Kotlin source.

## Audit Methodology

For each public Kotlin API file, we compare:
1. **Properties** (val/var) → C++ virtual getters/setters
2. **Functions** (fun) → C++ virtual methods
3. **Suspend functions** (suspend fun) → Continuation ABI entries returning a result box or the suspension sentinel
4. **Extension functions** → C++ free functions
5. **Companion object members** → C++ static methods or free functions

Method names are converted from camelCase to snake_case per C++ conventions.

## Current scope

Use [project-wide deep reports](project-wide/README.md) for measured current parity and repair order. The historical API rows below describe recorded surface and wiring observations; their totals do not establish whole-library completion.

### Current library source translation

The Kotlin library source under `tmp/kotlinx.coroutines` is the translation
contract. Current measured gaps and repair order come from the refreshed
[full-library deep reports](project-wide/library/port_status_report.md).
The entries here record code already translated, rather than later work.

| Kotlin source | C++ reference | Current translated behavior |
|---|---|---|
| `flow/internal/ChannelFlow.kt:54-56,117-120,144-152`; `flow/Channels.kt:25-26` | `flow/internal/ChannelFlow.hpp:278,298,307,372,536`; `flow/Channels.hpp:204` | Direct source calls: producer lambda forwards collect_to, operator and undispatched bodies use annotated suspend authoring, scoped collection calls emit_all with the actual produced channel owner. Zero collection-adapter calls remain in this header; four Merge consumers remain. Actual strict consumer and concrete instantiation exit 1 on dependency/generated-frame errors; runtime ownership remains unverified. See CHANNEL_FLOW_SCOPE_AND_SPILLS.md. |
| `flow/internal/ChannelFlow.kt:42-56,61-67,106-115,136-141,179-192,203-212`; `flow/Channels.kt:95-137` | `flow/internal/ChannelFlow.cpp:58`; `flow/internal/ChannelFlow.hpp:132,164,387,462`; `flow/Channels.hpp:203` | Source authoring draft: handwritten CollectContinuation removed; existing owning callable entry uses annotated suspend body. Source val fields are const, three source-final classes final, and missing drop/ATOMIC producer KDoc restored. Strict implementation and real consumer checks exit 1 on dependency/generated-code diagnostics; former runtime evidence predates this migration. Typed adapter remains an unresolved source difference. See CHANNEL_FLOW_SCOPE_AND_SPILLS.md current checkpoint. |
| `flow/Builders.kt:52-193,305-349`; `flow/internal/ChannelFlow.kt:102-104` | `flow/FlowBuilders.hpp:87,119,137,232,245,293,362,436,446,470,515` | Source factory selection corrected: eight paths use imported unsafe_flow while public flow uses SafeFlow. Source block val fields immutable; source object/final types final. Callback parent suspension/check/IllegalStateException sequence authored for compiler lowering, manual frame removed. Builder classes use source flow namespace and inherited protected hooks. Fresh strict consumers and explicit callback instantiation remain unsuccessful at generated frames/dependencies; no current runtime claim. See FLOW_BUILDERS_SOURCE_REPAIR.md. |
| `EventLoop.common.kt:19-138,147-163` | `EventLoop.hpp:19`; `EventLoop.common.cpp:25,65,87,115,122` | Wired selected base queue/use-count, one owning thread-local slot and source delay conversion. Seven selected base KDoc blocks retained with C++ references. Strict concrete syntax and bounded retention/resource execution exit 0 using existing dependency archive. Native factory, base dispatch, BlockingEventLoop and wider timer/worker parity remain incomplete. See EVENT_LOOP_SOURCE_REPAIR.md. |
| `internal/DispatchedTask.kt:56,77-109,136-205,215-219`; `internal/DispatchedContinuation.kt:293-312` | `common/DispatchedTaskDispatch.hpp:48,103,129,145,170`; `internal/DispatchedTask.hpp:107,133`; `internal/DispatchedTask.cpp:9`; `internal/DispatchedContinuation.hpp:354` | Wired source checked casts, original failure precedence, actual recovery entry, source unconfined-loop/finally function and derived delegation. Concrete dispatch exception uses actual context text. All 10/10 task KDoc blocks are exact. Strict concrete syntax exit 0; actual start fixture remains blocked by three dependency warnings. Refreshed task body 0.25, 10/10 functions; full runtime/lowering remains unverified. See DISPATCHED_TASK_SOURCE_REPAIR.md. |
| Native `CoroutineContext.kt:43-44`; common `CoroutineContext.common.kt:24-26` | `common/CoroutineContextUtils.hpp:27,39` | Wired source Native identity wrappers accept the inline callable directly, including move-only captures, without type erasure or warning suppression. Fresh strict isolated compilation/execution exit 0; actual start-fixture syntax remains blocked by eight other dependency warnings. The RTTI debug-string fallback is still a source gap. See NATIVE_CONTEXT_INLINE_SOURCE_REPAIR.md for separate code and KDoc evidence. |
| `channels/Produce.kt:60-71,239-299`; `channels/ChannelCoroutine.kt:13` | `channels/Produce.hpp:105,159,187,200,220,289`; `channels/Produce.cpp:12,58`; `channels/ProducerScope.hpp:12` | Wired source producer sequence: actual new_coroutine_context, completion registration before start, source forwarding/default overloads, ProducerScope.channel receiver identity and suspended await_close try/finally. Cleanup runs once after completion/failure/cancellation; the current Job check runs before waiting. Seven focused checks and instrumented frame execution complete. Produce has 6/6 function names and 2/2 source types, body similarity 0.27; these do not establish whole-file completion. See PRODUCE_SOURCE_REPAIR.md. |
| `channels/BufferedChannel.kt:878-961,1504-1567`; `internal/OnUndeliveredElement.kt:8-36`; `channels/ConflatedBufferedChannel.kt:14-88` | `channels/BufferedChannel.hpp:1836,1861,2255,2286,2423,2825,2880,3018,3062`; `channels/ConflatedBufferedChannel.hpp:57,97,128`; `internal/OnUndeliveredElement.hpp:77,98` | Wired for exercised select/overflow types: source receive cell loop, owning result-box cleanup, borrowed pointee identity, prompt cancellation reporting, exception cause/suppression, conflated overflow and select-send branches. The complete core and seven focused targets now build: null handlers do not instantiate unused string operations. An installed opaque/erased-value handler still needs its actual value-string binding. See CHANNEL_SELECT_RECEIVE_SOURCE_REPAIR.md for current receipts and limits. |
| `internal/OnUndeliveredElement.kt:6-24`; `channels/Channel.kt:9,1425-1454` | `internal/OnUndeliveredElement.hpp:23,59,77`; `channels/Channel.hpp:163`; `test_select_arguments.cpp:39` | Wired nullable callable binding: channels import the actual internal alias. Default/null/copy construction requires no element text; installing a callable retains its typed string operation for the source failure diagnostic. This C++ callable adapter is not an additional Kotlin class. No fake Any/string fallback is supplied. |
| `channels/BufferedChannel.kt:131-139,178-181,708-733,762-776,1493-1496,1649-1674,1707-1720,2767-2793` | `channels/BufferedChannel.hpp:1991,2046,2057,2079,2163,2903,2913,2921,2928,2937,3438,3485` | Wired context-aware callbacks: receive/catching/iterator resume uses the source binders and actual wrapper reporting; closed select retains its original send exception while reporting handler failure; closed direct send completes through the existing cancellable continuation and suppresses the closing cause on handler failure. Direct/select runtime regressions execute against fresh core. General erased-value text and broader channel body parity remain open on Kanban t_8700df29. |
| Native `kotlin/Unit.kt:11-16`; `kotlin/Enum.kt:36-38` consumed by `SharingStarted.kt:11-32` | `Unit.hpp:11`; `Unit.cpp:9`; `flow/SharingStarted.hpp:54`; `flow/SharingStarted.cpp:36` | Actual kotlin Unit type imported by the library, source Unit text and SharingCommand enum names. Existing erased Unit value carriers remain an explicit ABI adaptation. Broader Any string binding remains incomplete. |
| `CoroutineStart.kt:356-362,370` | `CoroutineStart.cpp:7,24`; `CoroutineStart.hpp:361,369,384,409,422` | Typed source invoke at :384 directly forwards block/receiver/completion to the three typed intrinsics; full invoke/property KDoc restored. Current strict fixture syntax check fails on exposed dependency diagnostics, so historical runtime evidence does not verify this new overload. Wired source selection: DEFAULT, ATOMIC and UNDISPATCHED call their actual start intrinsics; LAZY does nothing. Custom continuation interceptors execute through Native wrappers. C++ callable/result binding retains moved owners, preserves all lvalue callable/receiver identities regardless of copyability, and hands boxes to the owning receiving adapter. The former dispatcher/cancellation/completion shortcut is removed. Broader lazy builder and coroutine authoring/lowering contracts remain incomplete. |
| `selects/OnTimeout.kt:16-17,26-27,34-42,45-60`; `selects/Select.kt:111,221,239` | `selects/OnTimeout.hpp:33,49,58,71`; `selects/OnTimeout.cpp:12,19,29`; `selects/Select.hpp:314,318`; `test_select_arguments.cpp:153,170,191,222,246` | Wired source timeout branches: original context Delay, canonical Kotlin Duration rounding, actual select owner retained by timer, cancellation/competing-clause disposal, and owned typed block results. Private concrete timeout implementation lives in .cpp. Focused runtime and sanitizer evidence is recorded in SELECT_TIMEOUT_SOURCE_REPAIR.md; complete select/library parity remains unfinished. |
| `selects/Select.kt:463-470,488-521,824-848,860-862` | `selects/Select.hpp:415,435,471,964,970,979`; `test_select_arguments.cpp:58,262,297` | Wired typed clause bindings on both SelectBuilder and SelectImplementation: actual parameter storage survives registration/re-registration; result boxes are owned by receiving adapters, pointer results remain borrowed; cancellation actions retain their parameter storage through delayed dispatch. Nullable default and broader select surface/algorithm parity remain incomplete. See SELECT_ARGUMENT_BINDINGS.md. |
| `channels/BufferedChannel.kt:241-349,1475-1501` | `channels/BufferedChannel.hpp:1806,2196,2208,2369`; `test_select_arguments.cpp:157,208` | Wired select-send cell loop: source counter/close/segment/CAS/retry branches, actual Waiter subobject, retained waiter and sender-index cancellation registration. Public result projects the original receiver to its SendChannel virtual interface before erasure; duplicate by-value get_on_send is removed. Buffer expansion, segment transitions, resource identity, cancellation and failure are exercised. Receive-select and broader channel parity remain incomplete. See CHANNEL_SELECT_SEND_SOURCE_REPAIR.md. |
| `AbstractCoroutine.kt:70,83,88-117,133-135` | `AbstractCoroutine.hpp:129,144,164,189,198,233,242,249`; `test_cancellable_start.cpp:591` | Typed suspend receiver start now forwards to the typed CoroutineStart entry using the actual Continuation<T>. Empty source hooks retain unused parameter names as comments; resume_with, on_completion_internal and handle_on_completion_exception are final as upstream specifies. All eight source KDoc blocks are restored exactly. Current strict fixture and explicit typed instantiation remain blocked by five dependency warnings; historical queued ownership execution predates this edit. See ABSTRACT_COROUTINE_SOURCE_REPAIR.md. |
| `intrinsics/Cancellable.kt:15-64`; `intrinsics/Undispatched.kt:13-31`; Native `IntrinsicsNative.kt:142-189,221-263,296-324` | `intrinsics/Cancellable.hpp:23`; `Cancellable.cpp:47,52,58`; `Undispatched.cpp:8`; `IntrinsicsNative.cpp:9,41,73,123,183,192,203,208,213` | Wired: fresh Native callable wrappers use restricted/context frames and source labels; typed arguments and captures survive suspension. Cancellable, Atomic and Undispatched starts use actual interception/completion behavior, with explicit borrowed/shared completion ownership and receiving-adapter box deletion. Generated frame creation delegates to actual create overloads. Native callback helper and broader compiler/runtime contracts remain incomplete. |
| `channels/Channels.common.kt:90-103,159-162,191-202`; `flow/Channels.kt:104-108` | `channels/Channels.hpp:187,215,232,266,333,360`; `channels/Channels.common.cpp:11`; `flow/Channels.hpp:203` | Wired: consume cleanup executes once and preserves finally failure precedence; non-cancellation causes are wrapped with their original cause; consume_each waits through receive and inline action suspension; to_list returns an owning vector box only after completion. Repeated consume_as_flow collection throws IllegalStateException. Raw channel arguments remain borrowed. Deprecated receive-or-null APIs and inline nonlocal returns remain incomplete. |
| Native `Exceptions.kt:9-14`; stdlib `CancellationException.kt:11-14`; Native `Throwable.kt:26-34` | `src/kotlin/coroutines/cancellation/CancellationException.hpp:22`; `CancellationException.cpp:11,27,40,43`; `Exceptions.hpp:57`; `native/Exceptions.cpp:18,23,28` | Wired constructor/factory repair: cancellation derives from the existing `IllegalStateException` type, preserves nullable messages and original cause identity, supports default construction, and exposes the source factory as `cancellation_exception`. Concrete bodies live in the actual stdlib `.cpp`; the library imports the same class. Cause-only construction and JobCancellationException text remain incomplete. Current bounded verification is in NATIVE_CANCELLATION_NAMESPACE.md; the registered fixture now builds and executes against the fresh core. |
| Native `Exceptions.kt:21-29`; Native `Any.kt:31` | `native/Exceptions.hpp:18,21,24,27`; `native/Exceptions.cpp:68,87,92,95`; `src/kotlin/coroutines/cancellation/CancellationException.cpp:46` | Wired equality: identity/type/message checks precede virtual equality on the other Job and cause, including shared operands and nested cancellation causes. The concrete Native class replaces the common inline definition; constructor/getter bodies live in `.cpp`. Jobs remain borrowed. The existing std::exception carrier supports CancellationException virtual equality and default identity for other thrown C++ objects; full Kotlin Throwable/Any projection remains incomplete. |
| concurrent `internal/OnDemandAllocatingPool.kt:14-102` | `concurrent/internal/OnDemandAllocatingPool.hpp:39,49,67,86,101,107`; `OnDemandAllocatingPool.cpp:17,22,31`; `test_concurrent_native.cpp:27` | Wired instantiable pool: source capacity/reservation/publication/close loops, sequentially consistent retained slot values, actual resource identity, single extraction and Kotlin null diagnostic spelling. Concrete control/list bodies live in .cpp; source inline loop call sites expand into allocate/close, and unused duplicate loop is removed under the actual package. Native Worker integration and full generic Any.toString parity remain incomplete. Source creation-failure/KDoc contradiction is retained and recorded in ON_DEMAND_POOL_SOURCE_REPAIR.md. |
| Native `internal/Concurrent.kt:7-28`; common `internal/Concurrent.common.kt:31-40` | `internal/Concurrent.hpp:18,22,32,43,77,83,89`; `internal/Concurrent.cpp:10` | Wired: Native set construction ignores the size hint; reference operations use sequential consistency and strong identity CAS; value/loop extensions forward to the actual field; lock actions preserve deduced and move-only results; concrete void locking lives in `.cpp`. `BenignDataRace` remains absent pending the actual Volatile field-annotation dependency. |
| `flow/Channels.kt:28-41,95-134`; `flow/internal/NopCollector.kt:5-9` | `flow/Channels.hpp:61,195,208,234,243,252,271`; `flow/internal/NopCollector.hpp:17` | Typed source authoring, unverified: actual iterator/element loop, has_next and emit suspension, bool-box deletion and source conditional cleanup. Callback erasure and manual spill storage removed; Channels.cpp deleted. Existing raw/owned entries retain supplied owners without adopting borrowers. Channel/consume fields are immutable as upstream specifies; all five KDoc blocks have translated C++ references and shorthand. Final source-empty NopCollector hook instantiates strictly for three element types. Strict consumer compilation still fails on generated/dependency diagnostics; no fresh emission execution claimed. See FLOW_CHANNELS_SOURCE_REPAIR.md. |
| `Job.kt:288` | `Job.hpp:272,278,282`; `Job.cpp:100`; `flow/internal/Merge.cpp:10` | Plugin-backed join() authoring plus existing virtual Continuation ABI. Non-tail source call receives the actual generated continuation and LLVM resume dispatch. Immediate completion/failure, actual join suspension and caller cancellation execute; unlowered calls are rejected. Four compiler/interoperability and seven library executables finish with zero failures. Historical execution predates the current Merge source migration, whose strict checks fail. See MERGE_SOURCE_REPAIR.md. |
| `Job.kt:509-512,519-521,555-556,562-564,584-586,602-604,610,627-629,644` | `Job.hpp:441,444,447,459,467,471`; `Job.cpp:29,35,48,53,58,63,69,74,79,86` | Plugin-backed cancel_and_join body calls cancel then plain join through mandatory CMake lowering. Existing continuation ownership bindings remain at explicit boundaries. Concrete ordinary Job/context extensions retain source defaults, diagnostics and cancellation cause. Real suspension, immediate completion and caller cancellation execute against the fresh core and instrumented source bodies. Job remains 10/24 function names with body similarity 0.14. See JOB_EXTENSIONS_SOURCE_REPAIR.md. |
| Native stdlib `ContinuationImpl.kt:118-127` | `ContinuationImpl.hpp:121`; `ContinuationImpl.cpp:14,20,25,30` | Wired concrete completed sentinel: one shared instance, source IllegalStateException diagnostic and to_string text. Bodies move out of the header. Both completed-state failure branches and singleton identity execute. The new frame-prefix warning control enables -Wall -Wextra -Werror without warning-disable flags. Source body parity remains incomplete. See JOB_EXTENSIONS_SOURCE_REPAIR.md. |
| `internal/Symbol.kt:10-14`; `flow/internal/NullSurrogate.kt:12,19,26`; `flow/internal/Combine.kt:86-88,124` | `internal/Symbol.hpp:26,31`; `Symbol.cpp:8,11,14`; `flow/internal/NullSurrogate.cpp:7,12,17`; `Combine.cpp:287,379` | Wired actual sentinel identity and source text, nullable erased-value unbox, real singleton symbols, zip null encoding before send and unbox after receive. Ordinary C++ payload identity and ownership remain intact. General typed nullable bindings remain incomplete. See SYMBOL_NULL_SOURCE_REPAIR.md. |
| `flow/internal/Merge.kt:9-95` | `flow/internal/Merge.hpp:76,138,181,200,232`; `Merge.cpp:10,22,38` | Source authoring, unverified: production collectors now directly cancel/join, check Job, acquire and launch the source collection/finally child. The generic collector binding is lifted out of the suspend body; scoped/child lambdas are annotated. Older .cpp callable entries remain used by helper regressions. Strict consumers fail on generated frames, template/lambda-context and dependency diagnostics; no fresh runtime retention/cleanup claim. Internal Merge is 9/9 bodies, 3/3 types and similarity 0.27. See MERGE_SOURCE_REPAIR.md. |
| `flow/operators/Merge.kt:19-30,42-43,66-80,135-138,162-163,188-189,212-213` | `flow/Merge.hpp:43,51,66,122,132,148,156,164,172,184,193,213,233,251,269` | Source composition: flatten_concat uses actual collect/emit_all; flat_map_concat/merge use map then flatten; handwritten concat frames and duplicate stack mappers removed. Missing suspending transform overloads and default concurrency added. Latest transforms own and delete returned result boxes before emission/inner collection. Ordinary C++ transforms remain supported. Strict consumer/overload probe exits 1 on generated/dependency diagnostics. Public Merge is 8/9 bodies, similarity 0.10; Iterable projection, dependency bodies and broader docs remain incomplete. See MERGE_SOURCE_REPAIR.md. |
| `sync/Semaphore.kt:170-220,280-352,361-388` | `sync/Semaphore.cpp:128,203,300`; `Semaphore.hpp:137` | FIFO waiter publication retains actual waiter identity; cancellation and selection use source queue states; suspended with_permit releases once on action success or failure. Segment reclamation remains incomplete. |
| `flow/internal/FlowCoroutine.kt:26-61`; `intrinsics/Undispatched.kt:43-93` | `flow/internal/FlowCoroutine.cpp:43`; `flow/internal/FlowCoroutine.hpp:49`; `internal/ScopeCoroutine.hpp:138` | Existing flow_scope block/child-completion wiring retained. scoped_flow now uses the source unsafe_flow builder; invented FlowImpl and unused FlowCollectorImpl removed. Fresh strict consumers fail on generated/dependency diagnostics, so no fresh execution of this builder change is claimed. See COLLECT_SOURCE_AUTHORING_REPAIR.md. |
| `CompletionState.kt:8-18`; Native `internal/StackTraceRecovery.kt:5-21` | `CompletionState.hpp`; `internal/StackTraceRecovery.cpp` | Actual success/exception states and recovered results; Native recovery keeps exception identity, following its identity implementations. |
| `CancellableContinuationImpl.kt:169-217,247-264,473-603` | `CancellableContinuationImpl.hpp:224,751`; `CancellableContinuationImpl.cpp` | Separate handled/resumed flags, actual Symbol resume token, atomic state/handle ownership, retained cancellation callbacks, idempotent token handling and dispatcher identity checks. Concrete void algorithms are in the implementation file. Debug representation and generic subtype surface parity remain incomplete. |
| `CancellableContinuation.kt:167-170,265-320` | `CancellableContinuation.hpp:180,276,327` | Value/context cancellation callback overloads are implemented. The inherited resume_with contract remains abstract instead of swallowing failures. |
| `flow/internal/SafeCollector.common.kt:32-33`; Native `flow/internal/SafeCollector.kt:16-24` | `flow/internal/SafeCollector.cpp:65,68`; `flow/internal/SafeCollector.hpp:48,50,70,84`; `test_channel_consumption.cpp:471` | Wired checked Job casts: nullable collection lookup accepts null; non-Job objects with the Job key are rejected before ancestry validation and downstream emission. C++ RTTI throws std::bad_cast and preserves original shared owners. The final Native class exposes the original collector and context/size properties. Native Throwable/exception interop remains incomplete. |
| `flow/terminal/Collect.kt:26-113`; Native `internal/SafeCollector.kt:16-27` | `flow/Collect.hpp:41,69,89,146,181,256,296,307`; `flow/Flow.hpp`; `flow/internal/SafeCollector.hpp` | Source authoring, unverified: terminal collection replaces handwritten CollectFrame with an annotated suspend call retaining supplied owners as arguments. Raw collector remains borrowed. Existing indexed overflow, launch builder and Native checks retained. Fresh strict consumer compilation fails at generated frames/dependencies; historical execution does not validate the changed body. See COLLECT_SOURCE_AUTHORING_REPAIR.md. |
| `Builders.common.kt:140-183,219-267`; `CoroutineScope.kt:279-288`; `Supervisor.kt:50-69`; Native `CoroutineContext.kt:48-53` | `Builders.hpp:351,429`; `Supervisor.hpp:100`; `DispatchedCoroutine.hpp:12`; `UndispatchedCoroutine.hpp:11`; `internal/ScopeCoroutine.hpp:62` | Public builders create the source coroutine objects. They wait for children; normal scope child failure cancels siblings, supervisor child failure is isolated. Dispatcher changes return through the caller's actual intercepted frame with cancellation checked before typed result boxing. |
| stdlib `CoroutineContext.kt:30-43,72-73`; `CoroutineContextImpl.kt:140-199` | `context_impl.cpp:152`; `CoroutineContext.hpp`; `CoroutineName.hpp` | Source context ordering, replacement/removal and structural comparison are translated. The interceptor stays last and empty removals return the canonical singleton. Hashing and the consumed polymorphic-key algorithms are now wired; definitions now use the actual kotlin::coroutines namespace, and library consumers import them. The refreshed compiler inventory recognizes the context/interceptor pairs; serialization and other body gaps remain incomplete. See STDLIB_COROUTINE_NAMESPACES.md. |
| `JobSupport.kt:127,954-962,1001-1008,1475,1575-1582` | `JobSupport.cpp:232,509,767,1608`; `AbstractCoroutine.hpp:182` | Attached parent links retain the real child, then release ownership on removal. Readers retain children atomically through in-flight callbacks. Terminal result boxes are deleted when the job is destroyed. Single-child enumeration follows the source branch. Retired intrusive-state reclamation and suppressed-exception behavior remain incomplete. |
| `flow/internal/ChannelFlow.kt:155-170`, interceptor equality at :165 | `flow/internal/ChannelFlow.hpp:487`; `test_channel_as_flow_smoke.cpp:161` | Wired source structural equality: the new interceptor invokes virtual equals on the collecting interceptor, with nullable-left handling. Equal distinct interceptors select the undispatched collector; unequal interceptors create the real channel producer. Immediate completion and suspended success/failure/cancellation preserve actual upstream context, receiver/resource identity and terminal release. See CHANNEL_FLOW_SCOPE_AND_SPILLS.md. |
| `flow/internal/ChannelFlow.kt:26-30,54-56,114-115,188` | `flow/internal/ChannelFlow.hpp:136,144,255,270,388`; `test_channel_consumption.cpp:943,966` | Wired source defaults, public drop_channel_operators and shared get_collect_to_fun producer lambda. The actual receiver/resources survive rendezvous suspension and release after completion or cancellation; immediate failure also releases captures. Existing Continuation ABI and LLVM address injection remain in use. Source diagnostics and complete file parity remain unfinished. See CHANNEL_FLOW_SCOPE_AND_SPILLS.md for before-control and fresh runtime evidence. |
| `flow/internal/ChannelFlow.kt:54-56,114-120,144-152`; `flow/Flow.kt:223-230` | `flow/internal/ChannelFlow.hpp:255,278,342,501`; `flow/internal/ChannelFlow.cpp:18,93,121`; `flow/Flow.hpp:198,290,299` | The producer lambda and suspended collection frames retain existing shared flow owners. Queued producer start, temporary produce_in wrapper, buffered downstream suspension, resumed failure and channel cancellation retain receiver/resource identities and release them on termination. Raw/stack receivers remain borrowed. The former ChannelFlow private suspend-call frame was removed in c659aa94 and is now an unverified compiler-authored entry; coroutineScope uses the actual caller and start_undispatched_or_return, preserving child-completion interception. See CHANNEL_FLOW_SCOPE_AND_SPILLS.md. Current AbstractFlow body is now source-authored and unverified under strict compilation; historical executable evidence predates this migration, as recorded in ABSTRACT_FLOW_SOURCE_REPAIR.md. |
| Native `Exceptions.kt:13-14` | `native/Exceptions.cpp`; `Exceptions.hpp` | Cancellation factory keeps the original cause identity; the owning receiving caller deletes the exception. |


The focused executable set is `BuildersTest`, `test_sync`,
`test_channel_as_flow_smoke`, `test_sharing_suspension`, `test_suspension_core`,
`test_collect_reduce_smoke` and `test_continuation_dispatch`. Its execution results are separate evidence from
required deep body/parameter criteria. No whole-library completion is claimed.

### Consumed collection-to-array source integration

| Pinned source | C++ under kotlinc_native_ref | Status | Evidence / required closure |
|---|---|---|---|
| shared Collections.kt:514-545 | kotlin/collections/CollectionToArray.cpp:13,32; CollectionToArray.hpp | Wired in compiler archive; actual collection execution open | Two complete source loops for the consumed Any? instantiation preserve allocation and iterator/index order. Real AbstractCollection/ArrayList integration remains required; no replacement collection supplied. |
| Native Arrays.kt:85,87,89 | kotlin/collections/Arrays.cpp:13,19,24; Arrays.hpp | Wired in compiler archive; allocation executed | Actual Native forwarding and reference-array null allocation. Ten combined allocation/empty-array observations agree with Native 2.4.10. Collection copy loops and other typed array instantiations remain unexecuted/untranslated respectively. The Native unused reference argument has no local name in its concrete C++ body. All warning suppression is subsequently removed; the current unsuppressed rebuild is unsuccessful. See WARNING_SUPPRESSION_REMOVAL.md. |
| Native ArrayIntrinsics.kt:31-35; Arrays.cpp:175-177 | kotlin/ArrayIntrinsics.cpp:23; ArrayIntrinsics.hpp:18 | Consumed Any? entry wired; empty behavior executed | One compiler-owned canonical zero-length storage, empty size/iterator/bounds behavior observed. No generic cross-type identity or Native object-layout acceptance. |

Both compiler builds exit 0; five main/three Native-OFF tests have zero failures.
Six files retain 29 pinned provenance ranges. Full-root deep scans cover
compiler 672/library 354 sources, 587 paired units/763 physical files. Shared
conversion 2/54 functions, Native Arrays 6/10; provisional normalized-logic/span
zeros and generated errors remain. Deep inventory records emptyArray PRESENT,
while the body table remains 1/9; external symbol presence is not body fidelity.
See
[the conversion checkpoint](IR_IDENTITY_DEPENDENCIES.md#collection-conversion-source-integration-checkpoint-2026-10-07)
and build/ir-recovery/ir-collection-conversion/source-split/.

### Compiler-owned class binding and standalone integration

| Pinned source | C++ under kotlinc_native_ref / compiler tooling | Status | Evidence / required closure |
|---|---|---|---|
| Native Any.kt:31,40-52; KClassImpl.kt:16-43,72-81 | kotlin/AnyIdentity.cpp:12; AnyToString.cpp:18; kotlin/native/internal/CompilerObjectType.cpp:17; CompilerClassNames.cpp:17 | Plugin-backed consumed compiler-object binding; actual instances execute | Real Any/KClass instances execute identity/name/subtype/hash/text across two translation units and generic specializations. Compiler-owned metadata is distinct from Native TypeInfo. Constant-constructor lowering and unconsumed methods remain unfinished. |
| RTTIGenerator.kt:198-269,583-613; TypeInfo.h:97-136 | clang_suspend_plugin/CompilerClassMetadataPlugin.cpp; CompilerClassInfo.hpp:24; clang_suspend_plugin/CMakeLists.txt:29 | Consumed C++ ABI binding integrated | KotlinxCompilerObjects supplies the same source bodies to production compiler targets and the actual-object test. Mandatory metadata stage compiles those internal targets. This private projection does not implement full Native RTTI, object layout, GC or coroutine frames. |
| TypeInfoNames.kt:13-54; Types.cpp:53-69 | kotlin/native/internal/TypeInfoNames.cpp; CompilerClassNames.cpp; separate TypeInfoNamesNative.cpp | Compiler names execute standalone; actual Native transport regression retained | Native-OFF build invokes no Kotlin compiler and selected production target/link dependencies exclude Native runtime. Real Native name test separately retains 72 observations before/after GC. |

Five focused tests in the main tree and three in the Native-OFF tree execute with
zero failures. Ordinary C++ still executes 42/43/82. Thirty-six selected target
build records and ten binary/shared-library dependency lists are checked; the
class test and ordinary executable link only libc++/libSystem. Eleven relevant
files retain 71 checked source ranges. Both full-root deep reports cover compiler
672/library 354, 586 paired units/759 physical files. Required Any/KClassImpl/
TypeInfoNames/StringNumberConversions provisional score/logic/span zeros and
generated errors remain; target parse errors are absent in those groups. This
does not complete actual IR construction, scopes, constant-constructor lowering
or either real MLX GPU demonstration. See
[the current checkpoint](IR_IDENTITY_DEPENDENCIES.md#compiler-owned-class-binding-integration-checkpoint-2026-10-07)
and `build/ir-recovery/ir-class-binding-integration/`.

### Earlier consumed Native class contracts

| Pinned source | C++ under kotlinc_native_ref | Status | Evidence / required closure |
|---|---|---|---|
| Native KClass.kt:16-49; TypeInfoHolder.kt:12-14; Native KDeclarationContainer.kt:12; shared KClassifier.kt:17 | kotlin/reflect/KClass.hpp:77; kotlin/native/internal/TypeInfoHolder.hpp:15; genuine marker headers | Surface; actual contracts compile | 25 actual-type checks/four real consumers. Invariant T : Any and one protected abstract star-projection boundary; no fabricated metadata or concrete interface instance. All four raw source types 1/1; provisional deep zeros retained. |
| Native KClassImpl.kt:16-43 | kotlin/native/internal/KClassImpl.hpp:31; KClassIdentity.cpp:11; KClassNames.cpp:11 | Consumed source bodies and actual Any text consumer compile | The real object/instance compiler bindings and constant-constructor lowering remain unfinished. No C++ KClass/Any instance or new production linking. Raw functions 4/11, types 1/2, body similarity 0.10; unconsumed source algorithms remain gaps. |
| NativePtr.kt:29-34; Primitives.kt:1851-1852 | kotlin/native/internal/KClassPointerHash.cpp:17 | Consumed nongeneric pointer hash compiles/executes | 21 observations agree with exact source bodies on actual Native metadata before/after GC plus null. Hash object has no unresolved external symbols. This does not implement the complete NativePtr class or C++ metadata binding. |
| Native Any.kt:46-52 | kotlin/AnyToString.cpp:18 | Actual consumer now compiles; link/instance acceptance unfinished | Missing KClassImpl header frontier is resolved. Actual object-to-type metadata and instance-query symbols remain undefined; no Native ObjHeader cast or invented class name supplied. |

Nine files retain 40 checked ranges/seven full source KDoc blocks. Both complete-root
reports cover compiler 672/library 354, 579 paired units/752 physical files. All five
affected deep groups retain provisional score/logic/span zeros and generated errors;
NativePtr remains reported missing. Return through actual metadata/Any to list and
IR consumers/Native scopes. See [the current checkpoint](IR_IDENTITY_DEPENDENCIES.md#consumed-native-class-contracts-checkpoint-2026-10-07).

### Earlier Native Any prerequisite

| Pinned source | C++ under kotlinc_native_ref | Status | Evidence / required closure |
|---|---|---|---|
| Native Any.kt:18-53 | kotlin/Any.hpp:21; AnyIdentity.cpp:12,17; AnyToString.cpp:18 | Surface; identity bodies compile; text body not compiled | The text body stops at actual KClassImpl.hpp. No fabricated metadata, executed Any instance, new IR inheritance or production linking. Raw functions/types 3/3 and 1/1, body similarity 0.53; deep score/logic/span zero remain provisional with generated errors. |
| Native StringNumberConversions.kt:43-55; Native-wasm text/Char.kt:228-233,236-237; ToString.cpp:31-66,104-106 | kotlin/text/StringNumberConversions.hpp:20; .cpp:24,34,47,71,77 | Consumed Long formatting bodies compile and execute | 393 sanitizer digit/error observations against std::to_chars and source error text. Only libc++, libSystem and sanitizer runtime linked; no pinned Native comparison claimed. Raw functions remain 0/17 and Char missing; deep provisional zeros/generated errors retained. |

The compiler Any class is internal tooling, not a required base/storage format for
ordinary application classes. Real metadata and link closure remain required,
then return to list ancestors/ArrayList, IR binding and Native scopes. Both complete
MLX GPU demonstrations remain unfinished. See [the current checkpoint](IR_IDENTITY_DEPENDENCIES.md#native-any-prerequisite-and-ordinary-c-design-checkpoint-2026-10-07).

### Earlier Native ArrayList prerequisite algorithms

| Pinned source | C++ under kotlinc_native_ref | Status | Evidence / required closure |
|---|---|---|---|
| shared MutableCollections.kt:225,233,235-245,290,298,300-323 | kotlin/collections/MutableCollections.hpp:23,38,80,91,102,113 | Six complete consumed source bodies compile | Five consumers use actual iterable/list interfaces and real IR declaration elements. No backing list or iterator fabricated; actual list execution/ancestor integration remain required. Raw functions 6/33 and provisional deep logic/span zeros remain visible. |
| Native RandomAccess.kt:8-11 | kotlin/collections/RandomAccess.hpp:14 | Complete genuine methodless source marker | Required source type test selects indexed compaction; this supplies no backing list. Raw 1/1 types; deep generated errors/provisional zeros remain recorded. |
| generated _ArraysNative.kt:1254-1257 | kotlin/collections/ArraysNative.hpp:23 | Complete generic range-copy body executes on compiler-owned storage | All 23 observations agree with exact pinned source bodies running Native 2.4.10, including error order, copied/reference identity and trailing values. Native copying intrinsic uses installed stdlib. Raw generated-array functions 9/240. |
| Native Arrays.kt:91 | kotlin/collections/Arrays.hpp:28 | Complete Native array-return body executes | Returns the same storage and preserves trailing slots. Native Arrays raw 3/10. The source ignores collection size; its C++ definition keeps the argument type without an unnecessary local name. C++ sanitizer execution does not establish Native heap layout, GC or coroutine-frame interop. |

Return these consumed dependencies to actual Native ArrayList and its ancestors,
then transformIfNeeded/class declaration identity. The actual copy consumer still
stops at missing ArrayList.hpp. Both full MLX GPU demonstrations remain unfinished.
See [the precise checkpoint](IR_IDENTITY_DEPENDENCIES.md#native-arraylist-prerequisite-algorithms-checkpoint-2026-10-07).

### Earlier actual IR class list-rewrite contracts

| Pinned source | C++ under kotlinc_native_ref | Status | Evidence / required closure |
|---|---|---|---|
| libraries/stdlib/native-wasm/src/kotlin/collections/List.kt:133-243 | kotlin/collections/MutableList.hpp:50 | Complete consumed abstract source contract compiles | Indexed mutation, both add_all overloads, mutable iterator/view narrowing and invariance; no backing instance fabricated. Raw 1/2 types and forced abstract-body function zero; existing List remains reported missing. |
| compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/util/transform.kt:31-38 | org/jetbrains/kotlin/ir/util/Transform.hpp:37,74; TransformInPlace.cpp:22 | Consumed source body/actual declaration adapter compile | Once-evaluated size, actual IrElementBase cast, checked result type and indexed set; actual traversal execution/production linking remain unfinished. |
| same transform.kt:126-137; _Collections.kt:1835-1837; Iterables.kt:24-26; Iterators.kt:38-42 | org/jetbrains/kotlin/ir/util/Transform.hpp:51,84; TransformIfNeeded.cpp:23 | Complete consumed copy-on-change source body written; not compiled | Source ArrayList copy only on first identity change, original list otherwise, actual iterator/index/error order. Nongeneric body stops at actual ArrayList.hpp. Forward declaration is not implementation or copy acceptance. |
| Native Collections.kt:108-113; shared src Collections.kt:507 | kotlin/collections/CollectionFunctions.cpp:12,21 | Two consumed source helper bodies compile | Actual negative-index check and message; C++ arithmetic-error category adaptation does not construct Native exception boxes. |
| generated IrClass.kt:74-84 | org/jetbrains/kotlin/ir/declarations/IrClassChildren.cpp:14,22 | Both actual class-child source bodies compile | Corrected include path and real mutable-list/parameter contracts resolve prior compilation frontier; copy/base renderer and actual instance/link closure remain required. |

28 actual-type checks/three consumer functions compile; no fake instances or
new runtime/CTest acceptance. 47 ranges/nine complete KDoc blocks resolve to ten
exact pinned sources. Four exact source materializations preserve existing files
and sparse configuration. New bodies remain outside production linking.

Both full-root deep reports cover compiler 668/library 354, 565 paired units/736
physical files. Raw transform 1/9 functions, Native Collections 1/11, abstract List
forced zero, missing shared Collections and Native ArrayList's 91 functions/three
types remain visible. All three affected deep groups retain provisional logic/span
zeros and generated errors. Transform header namespace/provenance rejection is
recorded; no criteria are waived. Return actual ArrayList constructor/set closure
to copy-on-change, then concrete parameters/variables, binding and Native scopes.
See [the current checkpoint](IR_IDENTITY_DEPENDENCIES.md#actual-ir-class-list-rewrite-checkpoint-2026-10-07).

### Earlier actual IR parameter and default-body contracts

| Pinned source | C++ under kotlinc_native_ref/org/jetbrains/kotlin | Status | Evidence / required closure |
|---|---|---|---|
| compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/IrTypeParameter.kt:22-41; IrValueParameter.kt:23-87 | ir/declarations/IrTypeParameter.hpp:19/.cpp:11; IrValueParameter.hpp:20/.cpp:11 | Full consumed interfaces/source traversal bodies compile | Actual symbols/types, mutable source fields, index -1 and checked nullable default-body traversal; concrete execution and production linking remain unfinished. Raw functions 1/2 and 1/4. |
| compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/declarations/impl/IrTypeParameterImpl.kt:26-50; IrValueParameterImpl.kt:27-55 | ir/declarations/impl/IrTypeParameterImpl.hpp:13/.cpp:14; IrValueParameterImpl.hpp:13/.cpp:14 | Complete constructor/property source bodies written; not compiled | Actual factory/fields/attribute owner, empty lists and symbol.bind(this); both bodies stop at actual missing empty_list. No instances or owner binding executed. |
| compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions/IrBody.kt:18-21; IrExpressionBody.kt:17-33 | ir/expressions/IrBody.hpp:15/.cpp:10; IrExpressionBody.hpp:15/.cpp:10 | Consumed transform/traversal source bodies compile | Actual body hierarchy, checked expression results and child order; raw functions 1/1 and 1/4. Base/renderer link closure remains required. |
| compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:131-136,154-159; compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:61-69 | ir/symbols/IrTypeParameterSymbol.hpp:19/.cpp:11; IrValueParameterSymbol.hpp:19/.cpp:11; impl/IrTypeParameterSymbolImpl.hpp:13; IrValueParameterSymbolImpl.hpp:13 | Abstract leaf bodies compile; generated concrete bodies stop at actual IR-based descriptors | Existing single owner/descriptor/signature state; no binding substitute. Generated type inventories 6/23 and 3/17. |
| TypeParameterDescriptor.java:29-61; TypeSystemContext.kt:22 | descriptors/TypeParameterDescriptor.hpp:27; types/model/TypeSystemContext.hpp:34 | Complete consumed abstract contract/methodless marker | Nine consumed marker types; whole source remains 9/32 types and 0/74 functions. Java source is a development compiler contract, not a target JVM dependency. |

Six new pinned source files preserve sparse configuration and existing sources.
22 C++ files have 267 checked ranges and eight full source comment blocks; the
isHidden block retains nested examples/question with its prohibited label changed.
Six strict bodies and 33 actual-type checks/four compiled consumers succeed;
four constructor/symbol implementation bodies and the class-child consumer remain
uncompiled at named genuine dependencies. Five existing compiler targets rebuild,
four focused CTest checks have zero failures, and Native-OFF ordinary_cpp executes
with OS libraries only. These are bounded compiler/example observations.

Both complete-root deep inventories are compiler 664/library 354, 560 paired units/
730 physical files. Forced constructor/property-only zeros, provisional logic/span
zeros, unsupported/generated errors and reported-missing container files remain
required. The next consumer is actual class traversal/transform, concrete parameter
and variable binding, then Native scopes/buildStateMachine. Both complete MLX GPU
demonstrations remain unfinished. See [the detailed checkpoint](IR_IDENTITY_DEPENDENCIES.md#actual-ir-parameter-and-default-body-checkpoint-2026-10-07).

### Earlier actual IR class and classifier contracts

| Pinned source | C++ under kotlinc_native_ref/org/jetbrains/kotlin | Status | Evidence / required closure |
|---|---|---|---|
| generated IrClass.kt:24-85 | ir/declarations/IrClass.hpp:28/.cpp:11; IrClassChildren.cpp:14,22 | Surface; typed symbol/visitor entry bodies compile | All source fields and source child order written; child compilation stops at real missing IrTypeParameter.hpp. Value parameters/lists/transforms/base renderer and actual instances/metadata remain unfinished. Raw visitor functions 0/3. |
| generated IrDeclarationContainer.kt:16-25; IrTypeParametersContainer.kt:14-16 | ir/declarations/IrDeclarationContainer.hpp:17; IrTypeParametersContainer.hpp:21 | Surface; real inheritance/type signatures compile | Actual mutable declarations and identity-retaining type-parameter list. Raw oracle reports both files missing; preserve and investigate matching before claiming measured parity. |
| generated IrDeclarationWithVisibility.kt:16-18; IrPossiblyExternalDeclaration.kt:14-16; IrMetadataSourceOwner.kt:27-34 | ir/declarations/IrDeclarationWithVisibility.hpp:16; IrPossiblyExternalDeclaration.hpp:15; IrMetadataSourceOwner.hpp:27 | Source abstract contracts | Typed visibility/name/external/metadata identities; no concrete declarations or Native object substitution. |
| generated IrSymbol.kt:91-94,112; source IrSymbol.kt:69-74,126-130 | ir/symbols/IrClassifierSymbol.hpp:24/.cpp:12; IrClassSymbol.hpp:32/.cpp:12,15 | Classifier body builds in three compiler targets; class-symbol body compiles only | Existing single virtual owner/descriptor boundary and actual TypeConstructorMarker; class-symbol link closure needs real class/base/child bodies. Generated raw inventory 4/23 types. |
| ClassDescriptor.java:20-122 and its three inherited interfaces | descriptors/ClassDescriptor.hpp:21; ClassifierDescriptor.hpp; ClassifierDescriptorWithTypeParameters.hpp; ClassOrPackageFragmentDescriptor.hpp | Complete abstract source contracts | Actual typed scope/constructor/type and nullability; no descriptor instances, value-class representation algorithms or alternate compiler objects. Java development-host contracts do not require a JVM in the target application. |
| ClassKind.kt:18-46 | descriptors/ClassKind.hpp:25/.cpp:21 | Eight consumed property operations execute | 48 observations agree with unchanged pinned Native source for all six kinds. Source enum/free-function mapping retains raw forced function zero and missing enum-member properties. |
| TypeSystemContext.kt:21 | types/model/TypeSystemContext.hpp:27 | Eighth consumed genuine methodless marker | Actual classifier marker inheritance; total 8/32 types and 0/74 body functions remain measured. |

The missing-container findings trace to foreign template forwards being treated
as full declaration namespaces; the exact diagnostics and source trace remain in
build/ir-recovery/ir-class-identity/oracle-namespace-investigation.json.

Nineteen contract/body files have 142 checked ranges and 18 exact KDoc blocks.
Four strict body compiles, 26 actual-type checks/two compiled interface calls and
four affected CTest checks are recorded. Native-OFF ordinary_cpp rebuilds/runs
with OS libraries only. Full-root inventories are compiler 659/library 354, 549
paired units/709 physical files. Provisional logic zeros, source/generated errors
and missing interfaces/algorithms remain required. Neither required MLX GPU demo
is complete. See [the source/execution checkpoint](IR_IDENTITY_DEPENDENCIES.md#actual-ir-class-contracts-checkpoint-2026-10-07).

### Actual Native class-name dependency

| Pinned source | C++ under kotlinc_native_ref/kotlin/native/internal | Status | Evidence / required closure |
|---|---|---|---|
| kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt:13-54 | TypeInfoNames.hpp:20/.cpp:66 | Three consumed getter algorithms executed on actual Native metadata | 72 comparisons with unchanged pinned source on twelve real Native class identities before/after GC; source non-null requirement, reflection flags, empty package, '.'/'$' delimiters and UTF-16 names retained. Explicit borrowed TypeInfo/optional UTF-16 ABI adaptations do not complete NativePtr/value-class boxing or C++ class metadata. |

Nineteen source ranges and three exact KDoc blocks were checked. Two optional
Native CMake checks have zero failures. The Native-OFF ordinary C++ application
builds/executes and links only OS libraries, with no TypeInfoNames unit in its
build graph. A separate all-target build stops at existing unlowered delay/yield
calls in JobTest.cpp:263,298 and AsyncTest.cpp:268; that failure is retained.
Full-root deep inventories are compiler 652/library 354, 534 paired C++ units/690
physical files. TypeInfoNames raw 1/1 types, 0/0 body functions (seven target
bodies), forced function score zero, documentation 0.214423 and provisional
logic/span zeros remain required evidence. No criterion is waived. Actual Any
ancestry/class-literal construction, attributes, variable binding/scopes and both
MLX GPU demonstrations remain unfinished. See
[the Native name checkpoint](IR_IDENTITY_DEPENDENCIES.md#actual-native-class-name-checkpoint-2026-10-07).

### Actual IR type and consumed enum operations

| Pinned source | C++ under kotlinc_native_ref/org/jetbrains/kotlin | Status | Evidence / remaining required behavior |
|---|---|---|---|
| compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/types/IrType.kt:16-99 | ir/types/IrType.hpp:44/.cpp:12 | Explicit interfaces/default bodies compile in three compiler targets | Actual self/type/symbol/classifier/argument/annotation contracts; full Any ancestry, sealed metadata and concrete equality remain unfinished. Self-equality probe rejects the missing Any conversion. Raw 8/8 types, 0/1 functions and provisional zeros retained. |
| core/compiler.common/src/org/jetbrains/kotlin/mpp/TypeRefMarker.kt:10-17 | mpp/TypeRefMarker.hpp:18 | Complete consumed methodless interface | Original KDoc/identity; no synthetic type instance. Raw 1/1 type, methodless inventory. |
| core/compiler.common/src/org/jetbrains/kotlin/types/model/TypeSystemContext.kt:19-20,24-26,29,34 | types/model/TypeSystemContext.hpp:13 | Seven complete consumed methodless interfaces | Actual marker diamonds; other 25 types/74 body-bearing functions remain unported. Raw 7/32 and whole-file zero retained. |
| core/language.model/src/org/jetbrains/kotlin/types/Variance.kt:8-44 and IrType.kt:46-54 | types/Variance.hpp:12/.cpp:38; ir/types/IrType.cpp:23 | Consumed source enum operations executed | 35 observations agree with Native machine code; all valid combinations covered. Enum/free-function mapping retains raw 0/4 member-function matches and normalized-logic/span zeros. |

63 source ranges/four KDoc blocks retain exact provenance. Three affected checks
have zero failures; strict sanitizer output agrees with Native and has no diagnostics.
The C++ operation executable loads only two OS runtime libraries. Concrete IR
instances, variable construction/binding/scopes and both required MLX GPU demos
remain unfinished. Complete-root deep inventories are compiler 652/library 354,
532 paired C++ units/687 physical files. See
[the actual type checkpoint](IR_IDENTITY_DEPENDENCIES.md#actual-ir-type-and-enum-operation-checkpoint-2026-10-07).

### Real IR value-access and suspension nodes

| Pinned source under compiler/ir/ir.tree/gen/org/jetbrains/kotlin/ir/expressions | C++ under kotlinc_native_ref/org/jetbrains/kotlin/ir/expressions | Status | Evidence / required closure |
|---|---|---|---|
| IrDeclarationReference.kt:16-18; IrValueAccessExpression.kt:16-20 | IrDeclarationReference.hpp:16; IrValueAccessExpression.hpp:17 | Surface; actual symbol/origin contracts compile | Actual symbol covariance and nullable origin; no alternate declaration identity. |
| IrGetValue.kt:16-19; IrSetValue.kt:17-30 | IrGetValue.hpp:15/.cpp:11; IrSetValue.hpp:15/.cpp:11 | Source traversal bodies compile; production integration unfinished | Real value symbol and child expression; exact visits/transforms. Raw functions remain 0/1 and 0/3 under actual generic/virtual dispatch spelling. |
| IrSuspendableExpression.kt:17-34; IrSuspensionPoint.kt:18-39 | IrSuspendableExpression.hpp:15/.cpp:11; IrSuspensionPoint.hpp:17/.cpp:12 | Source traversal bodies compile; production integration unfinished | Real IrVariable ID, normal/resumed children and source order; checked ID rewrite. Raw functions remain 0/3 each. |
| impl/IrGetValueImpl.kt:20-29; IrSetValueImpl.kt:21-31; IrSuspendableExpressionImpl.kt:19-28; IrSuspensionPointImpl.kt:20-30 | impl/IrGetValueImpl.hpp:15/.cpp:13; IrSetValueImpl.hpp:15/.cpp:13; IrSuspendableExpressionImpl.hpp:15/.cpp:13; IrSuspensionPointImpl.hpp:15/.cpp:13 | Complete constructor/property bodies compile; no concrete-instance execution | Mutable fields and attributeOwnerId=this; borrowed actual compiler-owned identities. Source constructor/property-only function inventories force raw zeros. |

Eight bodies, twenty-seven genuine type checks and four actual-interface consumers
compile. 177 source ranges/six KDoc blocks retain exact pin provenance. Actual
compiler linking still needs real IrDeclarationBase/rendering, and object/attribute/
variable/binding/scope closure remains incomplete. No new nodes were constructed.
Both full-root deep reports cover compiler 649/library 354 sources against 527
paired units and 680 physical C++ files; all ten new groups retain provisional
logic/span zeros and raw missing/extra findings. See
[the source checkpoint](IR_IDENTITY_DEPENDENCIES.md#real-value-access-and-suspension-node-source-checkpoint-2026-10-07).

### Frontend CMake integration checkpoint

Root builds now apply the existing frontend plus mandatory LLVM injection to the
core/tests after target registration. Requested authoring requires real packages
and shared registries. Two defining Clang includes close an optimized link failure;
translated Kotlin algorithms and APIs are unchanged. A separate Native-OFF Release
application builds the full C++ library/plugins, executes the existing ordinary-C++
lambda fixture and loads only libc++/libSystem. Four pipeline/three final focused
checks have zero failures. Actual declaration identity/scopes, direct shared Native
state machines and both MLX GPU demonstrations remain unfinished. See
[the source/build receipt](IR_IDENTITY_DEPENDENCIES.md#frontend-root-build-and-standalone-authoring-checkpoint-2026-10-07).

### Native property and object-identity dependencies

| Pinned source under kotlin-native/runtime/src/main | C++ under kotlinc_native_ref/kotlin | Status | Evidence / remaining behavior |
|---|---|---|---|
| kotlin/kotlin/reflect/{KAnnotatedElement,KCallable,KProperty,KType}.kt | reflect/KAnnotatedElement.hpp:15, KCallable.hpp:22,50, KProperty.hpp:23, KType.hpp:19 | Consumed actual abstract contracts compile | Four headers, eighteen type checks and five actual-interface consumer bodies; fourteen source ranges/nine KDoc blocks. Actual instances, constant evaluation and general/star variance remain open. Raw seven missing property families/nine methods, three represented property gaps and provisional logic/span zeros remain required findings. |
| kotlin/kotlin/native/Runtime.kt:93-100 and cpp/Natives.cpp:40-49 | native/Runtime.hpp:21, Runtime.cpp:14 | Consumed source identity-hash body executed | Ordinary C++ use links only system libraries; ten same-object comparisons against actual Native runtime and Kotlin intrinsic agree, including null and GC. Six ranges/one KDoc/runtime comments retain exact provenance. Any/class metadata, relocation/optimizer effects, pinned full-runtime and complete coroutine/MLX integration remain open. Whole-file body inventory 0/3, documentation 0.111290 and provisional logic/span zeros retained. |

See [the bounded dependency checkpoint](IR_IDENTITY_DEPENDENCIES.md#native-property-contracts-and-identity-hash-checkpoint-2026-10-07).
Compiler/library full-root scans cover 645/354 source files against 517 paired
C++ units and 662 physical target files. Abstract contracts and a helper execution
do not finish concrete IR declaration identity, scopes or either MLX demonstration.

### Native integer-array dependencies and boundary

| Pinned source | C++ under kotlinc_native_ref/kotlin | Status | Evidence / remaining behavior |
|---|---|---|---|
| runtime Arrays.kt:251-328 | IntArray.hpp:32, IntArray.cpp:30 | Consumed source bodies executed on compiler storage | Zero/initializer construction, get/set/size, actual private specialized iterator. C++ owning slots are not Native ArrayHeader/GC layout. |
| stdlib PrimitiveIterators.kt:43-347 | collections/PrimitiveIterators.hpp:43, PrimitiveIterators.cpp:12 | Source abstract contracts and forwarding bodies | Eight specialized iterators retain KDoc; Int iterator and generic covariance execute. Other concrete primitive-array iterators remain open. |
| generated _ArraysNative.kt:922-925,1079-1081,1163-1165,1296-1299,1425-1433,1547-1549,1652-1654 | collections/ArraysNative.hpp:27, ArraysNative.cpp:13 | Consumed IntArray source bodies executed | Copy/defaults/range/resize/fill; 5,367 combined observations match Native. Generated file currently measures 8/240 functions (seven consumed IntArray functions and one object-array resize). |
| stdlib AbstractList.kt:116-157 | collections/AbstractListFunctions.cpp:19 | Five actual companion bodies executed | Bounds and capacity overflow cases match Native; whole AbstractList, equality/hash and ancestor algorithms remain open. |
| runtime Arrays.cpp:519-590 | collections/NativeArrayUtil.hpp:39 | Bounded actual Native runtime declarations executed | Native-owned IntArray get/set/length/copy/overlap/fill; matching pinned full runtime/shared frames/bare-metal remain required. |

See [the integer-array ledger](IR_IDENTITY_DEPENDENCIES.md#native-integer-array-dependency-and-boundary-2026-10-06).
Raw zeros/missing findings remain visible; execution does not replace required measured criteria.

### Native object-array dependencies and boundary

| Pinned source | C++ under kotlinc_native_ref/kotlin | Status | Evidence / remaining behavior |
|---|---|---|---|
| runtime ArrayUtil.kt:20-25,34-36,75-82,91-93 and generated _ArraysNative.kt:849-853,1377-1385,1520-1522 | collections/ArrayUtil.hpp:73, ArrayUtil.cpp:11; Array.hpp internal constructor | Source bodies executed on compiler storage | Uninitialized slots, reset/fill/copy, defaults, range/resize and source bounds order. 930 combined existing/null-growth observations agree with Native. Compiler C++ storage is not Native object ABI. Wider array covariance and primitive classes/overloads remain open. |
| runtime ArrayIntrinsics.kt:18-20; native-wasm Arrays.kt:96-106; generated _ArraysNative.kt:1241-1243 | ArrayIntrinsics.hpp:39; collections/Arrays.hpp:14,31; ArraysNative.hpp:108 | Consumed source operations executed on compiler storage | Null allocation, range/size null copies and generated public resize. Readable null tail, nullable type idempotence, source errors, copy independence and retained references execute. Raw inventories 1/9, 2/10 and 8/240; normalized logic remains provisional zero. IrElementBase/key/map and shared state-machine integration remain open. |
| runtime ArrayUtil.kt:34-36,91-93; Arrays.cpp:123-172 | collections/NativeArrayUtil.hpp:16, NativeArrayUtil.cpp:11 | Bounded real Native array boundary executed | Mandatory runtime symbols; real Native array reset, overlap/copy/fill, object identity and rooted get. Allocation, full Native array surface, matching pinned runtime, shared coroutine frames and bare-metal execution remain open. |

See [the dependency ledger](IR_IDENTITY_DEPENDENCIES.md#native-array-storage-and-direct-array-boundary-2026-10-06).
Raw low/provisional-zero scores and false primitive-overload matches remain recorded;
execution evidence does not complete measured port criteria.

### Native collection interfaces needed by IR attributes

| Source under libraries/stdlib/native-wasm/src/kotlin/collections | C++ under src/kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/collections | Status | Evidence / remaining behavior |
|---|---|---|---|
| Iterator.kt:30-38,81-112 | MutableIterator.hpp:65 | Surface; source abstract operations and typed consumer boundaries compile | Mutable iteration/remove/set/add contracts; actual cursor/removal state remains untranslated. |
| Collections.kt:36-41, Collection.kt:92-160 | MutableCollection.hpp:129 | Surface; source abstract operations and typed consumer boundaries compile | Actual mutable iterable/collection API; no backing collection or bulk-mutation algorithm supplied. |
| Set.kt:40-98 | Set.hpp:68 | Surface; source abstract operations and typed consumer boundaries compile | Read-only/mutable set types, structural object requirements, source covariance/invariance. Concrete uniqueness/order/mutation remain open. |
| Map.kt:43-245 | Map.hpp:384 | Surface; source abstract operations and typed consumer boundaries compile | Actual map/entry hierarchy, nullable get/put/remove, views and projected put_all keys. Thirty-seven static checks include actual descriptor types. Native backing maps, equality/hash, iteration, ownership and actual entry conversion remain unverified. |

The [dependency checkpoint](IR_IDENTITY_DEPENDENCIES.md#native-collection-prerequisite-for-ir-attributes-2026-10-06)
records 140 pinned ranges and mandatory deep diagnostics. Type counts and source
surface do not complete the backing algorithms, IR attributes or bare-metal ABI.

### IR declaration and visitor checkpoint

| Source under compiler/ir/ir.tree | C++ under src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/ir | Status | Evidence / open behavior |
|---|---|---|---|
| gen/org/jetbrains/kotlin/ir/IrElement.kt:19-87 and IrStatement.kt:14 | IrElement.hpp:19, IrStatement.hpp:15 | Surface; actual abstract contracts compile | All mutable source properties and generic visitor/transformer operations. Concrete nodes, attributes and transformer closure remain open. |
| gen declarations/{IrSymbolOwner,IrDeclaration,IrDeclarationWithName,IrDeclarationParent,IrMutableAnnotationContainer}.kt and src declarations/IrAnnotationContainer.kt | declarations/IrSymbolOwner.hpp:16, IrDeclaration.hpp:23, IrDeclarationWithName.hpp:16, IrDeclarationParent.hpp:15, IrMutableAnnotationContainer.hpp:16, IrAnnotationContainer.hpp:14 | Surface; source abstract contracts compile | Actual symbol/descriptor/name/parent/annotation types and bases. No concrete owner or one-time binding execution yet. |
| gen declarations/IrValueDeclaration.kt:19-26 and gen symbols/IrSymbol.kt:146-152; src symbols/IrSymbol.kt:56-67,126-127 and gen IrSymbolOwner.kt:18 | declarations/IrValueDeclaration.hpp:24, IrValueDeclaration.cpp:15, IrSymbolOwner.cpp:13; symbols/IrValueSymbol.hpp:27, IrValueSymbol.cpp:20, IrSymbol.cpp:24 | Surface; typed source contracts and actual C++ boundary compiled in three compiler targets | Mutually typed symbol/owner access retains abstract virtual root dispatch, source types, generic bounds and borrowed identity. Fifteen genuine type checks compile; invalid owner/descriptor rejected. Descriptor narrowing now also uses the root dispatch. No concrete variable/binding/scope execution yet. Raw property-only zeros and 21 missing generated symbol interfaces remain open. |
| src declarations/IrDeclarationBase.kt:17-28; gen declarations/IrVariable.kt:21-45, declarations/impl/IrVariableImpl.kt:26-54; gen symbols/IrSymbol.kt:161-168, symbols/impl/IrSymbolImpl.kt:71-73 | declarations/IrDeclarationBase.hpp:20/.cpp:14, IrVariable.hpp:21/.cpp:13, impl/IrVariableImpl.hpp:19/.cpp:17; symbols/IrVariableSymbol.hpp:23/.cpp:13, impl/IrVariableSymbolImpl.hpp:15/.cpp:12 | Source bodies; variable and variable-symbol bodies compile, constructor/execution closure incomplete | All properties, nullable initializer traversal/mutation, parent error, defaults and constructor binding retained. Genuine expression/element ancestors now exist; map/key, IR-based descriptor/empty-list/render/type dependencies remain required. Raw 21/16 missing symbol types, three unmatched generic-dispatch operations and provisional logic zeros remain open. |
| src IrElementBase.kt:23-190; gen expressions/IrExpression.kt:19-24 and IrVarargElement.kt:16 | IrElementBase.hpp:51/.cpp:23, IrElementBaseAttributes.cpp:26,45; expressions/IrExpression.hpp:18/.cpp:11, IrVarargElement.hpp:15 | Core source bodies compile; actual instance/map/key integration incomplete | Dense identity lookup, nullable previous values, growth/removal and source defaults retained. Complete snapshot/copy bodies need genuine maps and executable keys. Mutable expression type and checked transform retained. Raw matches 8/11, 1/1 and methodless zero; key dependencies and provisional logic/span zeros remain open. No binding, concrete traversal or MLX execution claim. |
| src IrAttribute.kt:42-160 | IrAttribute.hpp:55,76,115,186,217,238 | Source-bodied key/flag/delegate draft; strict compilation stops at genuine Any dependency | Typed nullable get/set, Boolean flag distinction, actual key/delegate identity, weak debug owner, source diagnostics and property names retained. Any/WeakReference, concrete properties/maps and full generic bounds remain unverified; consumed Native property interfaces now compile. Raw 7/14 function matches, seven missing nested methods, three missing nested classes and provisional logic/span zeros; documentation 0.085679. 26 ranges/seven KDoc blocks checked. No concrete-key/node or production execution claim. |
| gen/org/jetbrains/kotlin/ir/visitors/IrTransformer.kt:19-311 and IrElementTransformerVoid.kt:19-590; src visitors/Deprecated.kt:28-33 | visitors/IrTransformer.hpp:111, IrElementTransformerVoid.hpp:21, IrElementTransformerVoid.cpp:11, Deprecated.hpp:19 | Surface; actual source bodies written, compilation closure incomplete | 89 generic visits and 181 no-context methods/helpers with exact provenance. Child-first rewrites and checked package/file conversions retained. Genuine IR hierarchy, covariance/star views, real node traversal and buildStateMachine integration remain open; strict transformer compilation stops at a missing node header. |
| gen/org/jetbrains/kotlin/ir/visitors/IrVisitor.kt:18-285 | visitors/IrVisitor.hpp:113 | Source bodies written; signatures compile | One abstract method and all 88 real delegation bodies compared with source. Concrete node hierarchy and generic star variance prevent full traversal acceptance. Raw namespace identity rejection retained. |
| gen/org/jetbrains/kotlin/ir/IrElement.kt:53,62,74,86 and IrVisitor.kt:20-284 | visitors/IrVisitorDispatch.hpp:119, IrVisitorDispatch.cpp:6 | C++ generic virtual boundary compiled in three targets | Actual typed references/data/results retained; no fake node, name/ID dispatch or traversal substitute. Concrete transport instantiation remains open. |

The [dependency ledger](IR_IDENTITY_DEPENDENCIES.md#ir-declaration-and-visitor-checkpoint-2026-10-06)
records all 333 source ranges, strict checks, source route evidence and unresolved
oracle findings. Pinned Native source/runtime and bare-metal handoff acceptance
remain mandatory. The broad implementation cards remain unfinished.

### Compiler declaration prerequisites

| Compiler source | C++ implementation | Status | Evidence |
|---|---|---|---|
| `core/names/src/org/jetbrains/kotlin/name/Name.java:22-135` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/name/Name.hpp:30`, `Name.cpp` | Wired in focused compiler test; IR consumer remains untranslated | All source methods use snake_case, with UTF-16 behavior and Java wrapping hashes. `kxs_ir_name_contract` and six matching pinned-Java observations. No Java AST parser; see [dependency ledger](IR_IDENTITY_DEPENDENCIES.md). |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/Named.java:22-25` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/Named.hpp:25` | Surface | Source abstract `get_name` contract; concrete descriptor source-bodied drafts now exist, with dependency closure and execution unfinished. |
| `core/compiler.common/src/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.kt:8-28` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/mpp/DeclarationSymbolMarkers.hpp:19` | Surface | Exact fifteen-marker source hierarchy; source interfaces contain no methods. |
| `compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/declarations/IrParameterKind.kt:8-13` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/ir/declarations/IrParameterKind.hpp:12` | Surface | Source enum order with ALL_CAPS C++ names. |
| `compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/IrSymbol.kt:36-138` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/ir/symbols/IrSymbol.hpp:58`, `.cpp` | Surface; source abstract dispatch and typed owner boundary compiled | `owner`, `descriptor`, `has_descriptor`, `is_bound`, `signature`, `private_signature`, `set_private_signature`, typed `bind` and `is_public_api`. No execution with real declarations yet. Opt-in metadata and property-scoring limitation remain open. |
| `compiler/ir/ir.tree/src/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.kt:19-85` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/ir/symbols/impl/IrSymbolImpl.hpp:37` | Source-bodied draft; not compiled | Owner binding/error logic and descriptor/signature paths written against real untranslated dependencies. The current probe first stops at genuine `ir/descriptors/IrBasedDescriptors.hpp`; IdSignature and rendering dependencies also remain untranslated. No binding acceptance inferred. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptor.java:24-40` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/DeclarationDescriptor.hpp:34`, `.cpp` | Surface; C++ boundary compiled | Original/containing accessors, typed `accept`, `accept_void`, genuine inherited annotation/validation contracts. Concrete descriptors and dynamic equality/routing execution remain open. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.java:19-49` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/DeclarationDescriptorVisitor.hpp:40`, `DescriptorVisitorDispatch.hpp` | Surface; fifteen-method C++ boundary compiled | All methods instantiated for value/reference/move-only/void result configurations. Focused transport test retains ownership/reference/null behavior; no actual descriptor routing claimed. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/annotations/Annotations.kt:22-24`, `AnnotatedImpl.java:21-33` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/annotations/Annotated.hpp:25`, `AnnotatedImpl.hpp:25`, `.cpp` | Surface; getter storage compiled | Borrowed source annotation collection. Full collection behavior remains untranslated and receives a raw zero in the deep report. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/ValidateableDescriptor.java:8-10` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/ValidateableDescriptor.hpp:12`, `.cpp` | Surface; source default compiled | Empty validation default is the genuine upstream implementation. |
| `core/compiler.common/src/org/jetbrains/kotlin/descriptors/SourceElement.java:21-37`, `SourceFile.java:21-32` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/SourceElement.hpp:28`, `SourceFile.hpp:28`, `.cpp` | Wired for source singleton contracts | Exact absent-source/file object chain and null name match pinned Java execution. Full concrete descriptor integration remains open. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorWithSource.java:21-28` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/DeclarationDescriptorWithSource.hpp:26` | Surface; strict compilation | Source getter and covariant original descriptor; actual consumers unfinished. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorNonRoot.java:21-27` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/DeclarationDescriptorNonRoot.hpp:25` | Surface; strict compilation | Source nonnull containing-declaration contract retained in Clang AST without a runtime guard. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/DeclarationDescriptorWithVisibility.java:21-24` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/DeclarationDescriptorWithVisibility.hpp:27` | Surface; strict compilation | Actual visibility getter; the base visibility dependency now compiles. Concrete accessibility and descriptor integration remain untranslated. |
| `core/compiler.common/src/org/jetbrains/kotlin/descriptors/Visibility.kt:10-34`, `Visibilities.kt:8-89` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/Visibility.hpp:20`, `Visibilities.hpp:14`, `.cpp` | Wired for source singleton behavior | Actual nine objects and partial order; 91 observations agree with unchanged pinned Kotlin. Full deep criteria retain unsupported class/object emission; no descriptor accessibility or native ABI completion inferred. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/DescriptorVisibility.kt:23-86` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/DescriptorVisibility.hpp:28`, `.cpp` | Surface; source base bodies compiled | Full genuine abstract base and implemented delegate/default methods. Concrete accessibility, DelegatedDescriptorVisibility, obsolete-API metadata and actual consumer execution remain open. |
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/Substitutable.kt:21-23` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/Substitutable.hpp:29` | Surface; typed method/bound compiled | Real NonRoot accepted, Name rejected. General out-variance conversions and concrete substitution unverified; raw deep zero retained with body-inventory limitation. |

### Native root/runtime boundary

| Native source | C++ implementation | Status | Evidence / open criteria |
|---|---|---|---|
| `kotlin-native/runtime/src/main/cpp/Memory.h:180-311` | `src/kotlinx/coroutines/KotlinGCBridge.hpp:24`, `.cpp:10` | Wired for bounded Native stack-root and return-slot execution | Actual mandatory runtime declarations, FrameOverlay and ObjHolder bodies; 41 pinned ranges. Native fixture prints native-roots=10 and native-return-slot=1. Volatile operations use real rooted slots. Installed runtime lacks pinned RegisterGlobal; matching runtime, heap/global storage, concurrent references, shared frames and bare-metal execution remain open. |

See [the Native runtime specification](../runtime-and-gc/KOTLIN_NATIVE_GC_SPECIFICATION.md)
and `build/ir-recovery/native-references/`. The test option no longer changes production
reference semantics. No no-op, weak-symbol or standalone substitute remains here.

### Native lazy primitive dependency

| Native source | C++ implementation | Status | Evidence / open criteria |
|---|---|---|---|
| `kotlin-native/runtime/src/main/kotlin/kotlin/concurrent/atomics/Atomics.native.kt:30-144,519-578` and common integer extensions `Atomics.common.kt:100-152` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/concurrent/atomics/Atomics.hpp:32`, `.cpp:23` | Wired for compiler-owned primitive storage | Full consumed AtomicInt API, genuine deprecated members, six common extensions and three Native retry functions; 42 unchanged-source Kotlin/Native observations agree with C++. Five focused compiler tests record zero failures; sanitizer execution exits 0. Raw whole-file parity 18/62 functions and 1/5 types; normalized logic remains provisional zero. Native object layout and reference/root ABI are unfinished. |

See [the dependency ledger](IR_IDENTITY_DEPENDENCIES.md) and
`build/ir-recovery/native-lazy/implementation-receipt.json`. Lock/CurrentThread,
reference atomics, Lazy and real parameter execution remain unfinished. The source
runtime operations for references cannot be substituted by a plain pointer store.

### Concrete parameter source checkpoint

| Compiler source | C++ implementation | Status | Evidence / remaining closure |
|---|---|---|---|
| `descriptors/impl/DeclarationDescriptorImpl.java:27-70` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorImpl.hpp:13`, `.cpp:50` | Surface; source-bodied draft | Header compiles; body requires actual DescriptorRenderer DEBUG_TEXT. Diagnostic representation deviations remain unverified. |
| `descriptors/impl/DeclarationDescriptorNonRootImpl.java:27-66` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/impl/DeclarationDescriptorNonRootImpl.hpp:13`, `.cpp` | Surface; body compiles independently | Actual stored owner/source, narrowed original and owner validation; linking and instance execution require root dependencies. |
| `descriptors/impl/VariableDescriptorImpl.java:31-123` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/impl/VariableDescriptorImpl.hpp:13`, `.cpp` | Surface; source-bodied draft | Nullable construction-time type and source update assertion preserved; genuine type update traversal and collection singletons remain untranslated. |
| `descriptors/impl/ValueParameterDescriptorImpl.kt:26-133` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/impl/ValueParameterDescriptorImpl.hpp:15`, `.cpp` | Surface; source-bodied draft | Actual parameter flags/copy/original, overridden-owner index lookup, visitor routing and lazy destructuring selection. Native Lazy, real visibility, map/type/substitution and real-instance execution remain required. Raw oracle file-identity conflict retained. |
| `descriptors/{MemberDescriptor,CallableMemberDescriptor}.java` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/MemberDescriptor.hpp:14`, `CallableMemberDescriptor.hpp:15`, `.cpp` | Surface; consumed typed contracts compile | Full source builder interface; genuine Kind.isReal body compiled in compiler targets. General builder variance and concrete owner implementations remain open. |
| `descriptors/Modality.kt:8-27` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/Modality.hpp:12`, `.cpp` | Wired for compiler helper | All eight flag combinations match unchanged pinned Kotlin source executed by Kotlin/Native on macOS ARM64. Companion qualification/emission limitations leave raw logic provisional zero. This is not bare-metal/runtime ABI acceptance. |

Source paths in this table are under `core/descriptors/src/org/jetbrains/kotlin/`.
Per-function provenance, strict compilation receipts and exact limits are in
[the dependency ledger](IR_IDENTITY_DEPENDENCIES.md). Both full-root deep runs were
refreshed after these changes; compiler 620 Kotlin files, library 354. Source-defined
false/null/empty behaviors are preserved; none establishes completion of the missing
dependencies. Four focused compiler tests recorded zero failures.

### Callable/parameter collection prerequisites

| Compiler source | C++ implementation | Status | Evidence |
|---|---|---|---|
| `core/descriptors/src/org/jetbrains/kotlin/descriptors/{CallableDescriptor.java,ValueDescriptor.java,VariableDescriptor.java,ParameterDescriptor.java,ValueParameterDescriptor.kt}` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/org/jetbrains/kotlin/descriptors/CallableDescriptor.hpp:51`, `ValueDescriptor.hpp:24`, `VariableDescriptor.hpp:25`, `ParameterDescriptor.hpp:23`, `ValueParameterDescriptor.hpp:24` | Surface; strict typed boundary compilation | Genuine hierarchy, source KDoc and source default `is_late_init` in `.cpp:22`. Consumed covariant collection/list/iterator types retain object views. Concrete descriptors, user-data semantics, substitution and real owner binding remain open. |
| `libraries/stdlib/native-wasm/src/kotlin/collections/{Collections,Collection,List,Iterator}.kt` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/collections/Collections.hpp`, `Iterator.hpp`, `CollectionElement.hpp` | Surface; consumed Native read-only contracts compiled | Full Native KDoc and per-function ranges; JVM serialization text removed. Actual abstract contracts remain abstract. Typed views retain source objects. Mutable/set/map algorithms and general variance unfinished; raw zeros and multi-source file-pairing limitations retained. |
| `kotlin-native/runtime/src/main/kotlin/kotlin/Array.kt:24-92` | `src/kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/Array.hpp:36`, `:124`, `:148` | Wired for focused Native iterator algorithm; runtime ABI unfinished | Native precheck/post-increment/index exception behavior. Nineteen C++ observations match an ARM64 Native executable running the unchanged pinned iterator class. Full pinned runtime, Native object storage/GC ABI, shared frames and bare-metal deployment unverified. JVM iterator/factory removed; raw provisional zero logic retained. |

### AbstractFlow collection

| Kotlin source | C++ implementation | Status | Evidence |
|---|---|---|---|
| `kotlinx-coroutines-core/common/src/flow/Flow.kt:223-230`, `AbstractFlow.collect` | `src/kotlinx/coroutines/flow/Flow.hpp:254,290,299` | Source-authored migration, unverified | Typed collect body directly constructs SafeCollector and invokes collect_safely with source cleanup; callback entry and manual CollectFrame removed, Flow.cpp deleted. Existing shared receiver owner is retained as a local; raw receivers remain borrowed. Strict fixture compilation finds the specialization but fails on generated unresolved T, exception initializer and label-address diagnostics. No current runtime success claimed. See ABSTRACT_FLOW_SOURCE_REPAIR.md. |

### emit_all failure guard

| Kotlin source | C++ implementation | Status | Evidence |
|---|---|---|---|
| `flow/Channels.kt:28-41`, initial `ensureActive()` | `src/kotlinx/coroutines/flow/Channels.hpp:88,95` | Wired | Checks the failure collector before entering channel consumption and cleanup. |
| `flow/terminal/Collect.kt:103-106`, `emitAll` | `src/kotlinx/coroutines/flow/Collect.hpp:216` | Wired | Rejects the failure collector before upstream collection. |
| `flow/operators/Emitters.kt:193-205`, `ensureActive`, `ThrowingCollector` | `src/kotlinx/coroutines/flow/internal/ThrowingCollector.hpp:18`, `:41` | Wired | Rethrows the original exception; typed C++ collector specialization preserves Kotlin's contravariant collector behavior. |

`test_channel_as_flow_smoke` verifies exact exception identity, rejection of an empty open channel without cancelling it, no upstream flow collection, and direct emission rejection. Full `on_completion` action/resume semantics remain a separate unresolved implementation gap; adding this helper does not establish that operator's parity.

### on_start suspension and collector boundary

| Kotlin source | C++ implementation | Status | Evidence |
|---|---|---|---|
| `flow/operators/Emitters.kt:70-81`, `onStart` | `src/kotlinx/coroutines/flow/Emitters.hpp:51`, frame `:75` | Wired | Suspend action runs through SafeCollector, retains it until action completion, releases it before direct upstream collection, and propagates resumed failures. Mandatory LLVM injection supplies frame resume addresses. |

The non-suspending overload adapts to the same suspension-aware implementation.
Runtime cases cover ordered action/upstream suspension, retained source ownership
when public wrappers are dropped, and an action resuming with the original failure
without starting upstream. These observations do not establish whole-Emitters parity.

## Core Interfaces

### Job (kotlinx.coroutines.Job)

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/Job.kt`  
**C++ Header**: `src/kotlinx/coroutines/Job.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `val parent: Job?` | `virtual std::shared_ptr<Job> get_parent()` | ✅ | |
| `val isActive: Boolean` | `virtual bool is_active()` | ✅ | |
| `val isCompleted: Boolean` | `virtual bool is_completed()` | ✅ | |
| `val isCancelled: Boolean` | `virtual bool is_cancelled()` | ✅ | |
| `fun getCancellationException()` | `virtual std::exception_ptr get_cancellation_exception()` | ✅ | |
| `fun start(): Boolean` | `virtual bool start()` | ✅ | |
| `fun cancel(cause: CancellationException?)` | `virtual void cancel(std::exception_ptr)` | ✅ | |
| `val children: Sequence<Job>` | `virtual std::vector<std::shared_ptr<Job>> get_children()` | ⚠️  | Returns vector, not lazy sequence |
| `fun attachChild(child: ChildJob)` | `virtual std::shared_ptr<ChildHandle> attach_child()` | ✅ | |
| `suspend fun join()` | `virtual void* join(Continuation<void*>*)` | ✅ | Continuation ABI suspend function |
| `val onJoin: SelectClause0` | `virtual selects::SelectClause0& on_join()` | ✅ | Select clause wired |
| `fun invokeOnCompletion(handler)` | `virtual std::shared_ptr<DisposableHandle> invoke_on_completion()` | ✅ | |
| `fun invokeOnCompletion(onCancelling, invokeImmediately, handler)` | `virtual std::shared_ptr<DisposableHandle> invoke_on_completion()` | ✅ | |

**Notes**:
- `children` returns `vector` instead of lazy `Sequence` - acceptable for C++
- `join()` uses Continuation ABI: `void* join(Continuation<void*>*)`

---

### CoroutineDispatcher (kotlinx.coroutines.CoroutineDispatcher)

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/CoroutineDispatcher.kt`  
**C++ Header**: `src/kotlinx/coroutines/CoroutineDispatcher.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `abstract fun dispatch(context, block)` | `virtual void dispatch(const CoroutineContext&, std::shared_ptr<Runnable>) const = 0` | ✅ | Core dispatch method |
| `open fun isDispatchNeeded(context): Boolean` | `virtual bool is_dispatch_needed(const CoroutineContext&) const` | ✅ | Dispatch optimization |
| `fun limitedParallelism(parallelism, name): CoroutineDispatcher` | `virtual std::shared_ptr<CoroutineDispatcher> limited_parallelism(int, const std::string&)` | ✅ | Parallelism control |
| `fun dispatchYield(context, block)` | `virtual void dispatch_yield(const CoroutineContext&, std::shared_ptr<Runnable>) const` | ✅ | Yield-aware dispatch |
| `final override fun <T> interceptContinuation(continuation): Continuation<T>` | `template <typename T> std::shared_ptr<Continuation<T>> intercept_continuation(...)` | ✅ | Continuation interception |
| `final override fun releaseInterceptedContinuation(continuation)` | `virtual void release_intercepted_continuation(...)` | ✅ | Continuation cleanup |
| `operator fun plus(other: CoroutineDispatcher): CoroutineDispatcher` | `virtual std::shared_ptr<CoroutineDispatcher> plus(...)` | ✅ | Dispatcher composition |
| `override fun toString(): String` | `virtual std::string to_string() const` | ✅ | Debug string |
| `override fun minusKey(key): CoroutineContext` | Inherited from `Element` | ✅ | Context manipulation |

**Status**: ✅ **WIRED** - Core dispatch, yield, intercept, and limited parallelism implemented

---

### Deferred<T> (kotlinx.coroutines.Deferred)

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/Deferred.kt`  
**C++ Header**: `src/kotlinx/coroutines/Deferred.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `val onAwait: SelectClause1<T>` | `virtual selects::SelectClause1<T>& on_await() = 0` | ✅ | Select clause wired |
| `suspend fun await(): T` | `virtual void* await(Continuation<void*>* continuation) = 0` | ✅ | Continuation ABI suspend function |
| `fun getCompleted(): T` | `virtual T get_completed() const = 0` | ✅ | |
| `fun getCompletionExceptionOrNull(): Throwable?` | `virtual std::exception_ptr get_completion_exception_or_null() const = 0` | ✅ | |

---

### Dispatchers (kotlinx.coroutines.Dispatchers)

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/Dispatchers.common.kt`  
**C++ Header**: `src/kotlinx/coroutines/Dispatchers.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `val Default: CoroutineDispatcher` | `static CoroutineDispatcher& get_default()` | ✅ | Shared worker thread pool |
| `val Main: MainCoroutineDispatcher` | `static MainCoroutineDispatcher& get_main()` | ✅ | Main UI thread dispatcher |
| `val Unconfined: CoroutineDispatcher` | `static CoroutineDispatcher& get_unconfined()` | ✅ | Unconfined dispatcher |
| `val IO: CoroutineDispatcher` | `static CoroutineDispatcher& get_io()` | ✅ | IO dispatcher |

**Status**: ✅ **WIRED** - Default, Main, Unconfined, and IO accessors implemented

---

### Delay (kotlinx.coroutines.Delay)

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/Delay.kt`  
**C++ Header**: `src/kotlinx/coroutines/Delay.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `suspend fun delay(timeMillis: Long)` | `delay(time_millis)` authoring; `void* delay(long long, Continuation<void*>*)` and shared-continuation ABI | Plugin-backed | Delay.hpp:172; Delay.cpp:99; current frame inserted for authoring calls; timer resumption and prompt cancellation tested |
| `suspend fun delay(duration: Duration)` | `delay(kotlin::time::Duration)` authoring and raw/shared-continuation ABI | Plugin-backed; conversion dependency tested | Delay.hpp:129, :132, :136; Delay.cpp:77, :111; Delay.cpp:23 ports to_delay_millis. Eleven upstream cases and Kotlin/Native boundary comparison verify duration conversion |
| `val invokeOnTimeout: (timeMillis: Long, block: Runnable, context: CoroutineContext) -> DisposableHandle` | `virtual std::shared_ptr<DisposableHandle> invoke_on_timeout(...)` | ✅ | Timeout callbacks |
| `fun scheduleResumeAfterDelay(timeMillis: Long, continuation: CancellableContinuation<Unit>)` | `virtual void schedule_resume_after_delay(...)` | ✅ | Low-level delay scheduling |

**Status**: ✅ **WIRED** - Delay interface and delay/await_cancellation free functions implemented

---

## Continuation interception repair (2026-10-04)

Duration dependency additions (2026-10-06), sourced from
`tmp/kotlin/libraries/stdlib/src/kotlin/time/Duration.kt`:

| Kotlin API | C++ reference | Evidence and remaining scope |
|---|---|---|
| `minus(Duration)` | `src/kotlin/time/Duration.hpp:27`; `Duration.cpp:200` | Actual addition/negation delegation; finite mixed-range subtraction compared with Kotlin/Native |
| `times(Int)` | `src/kotlin/time/Duration.hpp:30`; `Duration.cpp:202` | Actual overflow/range branches; integer extremes and zero compared with Kotlin/Native under sanitizers |
| `div(Int)` | `src/kotlin/time/Duration.hpp:33`; `Duration.cpp:237` | Actual infinity/zero and nanosecond remainder branches; integer extremes and zero compared with Kotlin/Native |
| `absoluteValue` | `src/kotlin/time/Duration.hpp:36`; `Duration.cpp:261` | Actual sign branch; boundary conversion results compared with Kotlin/Native |
| `compareTo(Duration)` | `src/kotlin/time/Duration.hpp:37`; `Duration.cpp:263` | Encoded sign/range branches translated; comparisons in both directions compared with Kotlin/Native |
| internal `truncateTo(DurationUnit)` | `src/kotlin/time/Duration.hpp:51`; `Duration.cpp:253` | Private implementation mirrors upstream modulo/conversion body; compiled, not directly exercised by this receipt |

The full Duration API is incomplete. Current measured criteria and unresolved
emission errors are recorded in `project-wide/compiler/deep_transliteration_evidence.txt`;
the executable comparison does not establish complete source parity.

| Kotlin runtime/API | C++ reference | Status | Scope |
|---|---|---|---|
| UpgradeCallableReferences.UpgradeTransformer.flatten_parameters / visit_function_expression | tools/clang_suspend_plugin/UpgradeCallableReferences.cpp:28, :65 | Plugin-backed for tested lambda entries | Restriction captured before roles flatten; extension origin, context-origin conversion and dispatch-receiver rejection retained |
| NativeFunctionReferenceLowering.postprocess_invoke | tools/clang_suspend_plugin/NativeFunctionReferenceLowering.cpp:28 | Plugin-backed for tested lambda entries | Closure restriction property sets the invoke flag; AST import copies the flag and preserves original captured declaration identities |
| AbstractFunctionReferenceLowering.build_class / build_invoke_method | tools/clang_suspend_plugin/AbstractFunctionReferenceLowering.cpp:33, :74 | Wired for tested nongeneric nested lambda construction | Indexed private fields and declaration-identity mapping; function/parameter annotations and restricted-invoke flag retained. Value/reference/move-only/this captures and identifier shadowing exercised. Copied receiver fields bind through their address with invoke constness; constructor lowering preserves the object dereference. Constant complete array fields copy elements with dimensions intact; array references retain their referents. Generic captures, SAM/reflection/named references and full declaration lifting remain incomplete |
| NativeSuspendFunctionsLowering.get_coroutine_base_class | tools/clang_suspend_plugin/NativeSuspendLowering.cpp:169 | Plugin-backed for named receivers and tested lambda invokes | Source OR branch selects restricted or ordinary base, used for both inheritance and constructor. Lambda restriction producer, flattening and invoke flag are wired; named-reference wrappers and reflection remain untranslated |
| ExpressionSlicer.slice_expression — field assignment child order | tools/clang_suspend_plugin/NativeSuspendLowering.cpp:671 | Plugin-backed for tested suspension-containing C++ assignments | Destination evaluation and retained identity precede value evaluation, following NativeSuspendFunctionLowering.kt:212-250, :301-305. Temporary receivers with reference members own storage through suspension and full-expression cleanup. C++ retained references adapt Kotlin retained receivers; full IrSetField and IR/type translation remain incomplete |
| NativeSuspendFunctionsLowering.is_pure | tools/clang_suspend_plugin/NativeSuspendLowering.cpp:730 | Wired into argument slicing | Constants and immutable nonvolatile declaration reads stay inline; calls, mutable reads and checked/explicit casts remain impure. Implicit conversions recurse. Mutable C++ parameters conservatively retain storage, unlike immutable Kotlin value parameters. Full ExpressionSlicer remains incomplete |
| NativeSuspendFunctionsLowering.slice_constructor_arguments | tools/clang_suspend_plugin/NativeSuspendLowering.cpp:752 | Plugin-backed for tested declarations and temporary call arguments | Kotlin suspension suffix and constructor first-child exception retain earlier mutable arguments. Shared operand translation feeds declaration emplacement and expression rewriting. Resolved defaults evaluate into retained storage in parameter order after explicit arguments. Immediate/resumed success and resumed failure are checked. Kotlin allocation, mask-based default dispatcher lowering and full ExpressionSlicer parity remain incomplete |
| FirSuspendCallChecker.check / check_non_local_return_usage / find_enclosing_suspend_function / is_in_scope_for_default_parameter_values | tools/clang_suspend_plugin/FirSuspendCallChecker.cpp:15, :49, :77, :91 | Wired for tested defaults and non-local suspension boundaries | Reverse declaration walk permits local variables, parameters and return-allowed inline lambdas before the selected suspend function. Ordinary lambdas and local-class initializers/methods cannot inherit its suspension permission. Default diagnostics retain the Kotlin distinction; suspend callable defaults remain accepted. General call context classification, receiver checks and full FIR checker parity remain incomplete |
| TailSuspendCalls / collect_tail_suspend_calls / visit_element / visit_when | tools/clang_suspend_plugin/TailSuspendCallsCollector.hpp:8; TailSuspendCallsCollector.cpp:135, :51, :55, :63; KotlinxSuspendPlugin.cpp:429 | Wired into direct-entry selection | Translated try/tail visitor state and call traversal replace the custom return walk. Generic child traversal and branch handling have separate source-matched methods. Unit metadata is explicit for erased returns. Returnable-block IR symbols and automatic Kotlin type metadata import remain incomplete; current execution evidence is recorded in the project-wide audit |
| SuspendableExpressionScope.add_resume_point / evaluate_suspendable_expression / evaluate_suspension_point | tools/kotlinc_native_ref/IrToBitcode_coroutines.cpp:16, :44, :59 | Compiled LLVM helpers; scope wired into mandatory injector | Real LLVM types replace void-pointer aliases and pseudocode. Address dispatch, normal/resume block emission and result phi merging share the translated FunctionGenerationContext. Complete Kotlin IR evaluation, source metadata and general declaration/scope resolution remain unported; execution evidence is recorded in the project-wide audit |
| continuation_block | tools/kotlinc_native_ref/IrToBitcode_coroutines.cpp:32 | Compiled internal merge-block generation | Restores Kotlin’s code(result) invocation after phi construction and preserves the source empty default callback. Coroutine paths exercise the default; CatchingScope’s nonempty handler callback, LocationInfo and full IrType lowering remain untranslated. Merge phis and null-label comparisons retain Kotlin’s empty LLVM instruction names |
| FunctionGenerationContext.br / cond_br / icmp_eq / indirect_br; PositionHolder insertion state; preserving_position / appending_to | tools/kotlinc_native_ref/CodeGenerator.cpp:121, :127, :140, :214; :52, :68, :75; tools/kxs_inject/CoroutineInjection.cpp:135 | Plugin-backed for mandatory dispatch emission | Translated LLVM operations and after-terminator state are used by the injector and expression helpers. Empty destination lists retain Kotlin's indirect branch. Scoped appending at CodeGenerator.cpp:236, :259 swaps builders and restores the original insertion position and terminator state on normal and exceptional exits. Runtime frame initialization, source-location maps and complete FunctionGenerationContext remain unported |
| FunctionGenerationContext.function / block_address | tools/kotlinc_native_ref/CodeGenerator.cpp:39, :83, :136; tools/kxs_inject/CoroutineInjection.cpp:135 | Plugin-backed function binding; compiled expression address generation | Context binds one LLVM function definition, matching Kotlin's function property. Resume addresses use that owner. Block creation inserts immediately after the current block, following Kotlin's LLVM placement algorithm. Function-scoped generation and address generation before initial positioning are exercised; full LlvmFunction signature/attribute and native frame/context initialization remain unported |
| FunctionGenerationContext.basic_block / if_then_else / if_then | tools/kotlinc_native_ref/CodeGenerator.cpp:96, :183, :203 | Compiled LLVM block layout and conditional generation | Kotlin's insertion/move order replaces append-to-function in coroutine expression helpers and after-terminator generation. Conditional value merging creates a phi and assigns both predecessors; effect-only generation adds an exit edge only if the body has not terminated. LocationInfo parameters/maps and complete expression/scope generation remain unported |
| FunctionGenerationContext.icmp_gt / icmp_ge / icmp_lt / icmp_le / icmp_ne / icmp_u_lt / icmp_u_le / icmp_u_gt / icmp_u_ge | tools/kotlinc_native_ref/CodeGenerator.cpp:145, :149, :153, :157, :161, :165, :169, :173, :177 | Compiled LLVM integer comparisons | Direct translations of CodeGenerator.kt:1003-1011 preserve signed/unsigned predicates and the default instruction name. Generated execution covers equal values, negative values and integer limits; complete FunctionGenerationContext remains untranslated |
| FunctionGenerationContext.switch_ | tools/kotlinc_native_ref/CodeGenerator.cpp:221 | Compiled LLVM integer switch emission | Preserves Kotlin's ordered case collection, explicit default destination and after-terminator state. The trailing underscore escapes a C++ keyword. Source call-site scope is runtime initializer dispatch; complete create_init_body and runtime initialization remain untranslated |
| FunctionGenerationContext.raw_ret | tools/kotlinc_native_ref/CodeGenerator.cpp:229 | Compiled LLVM return generation | Emits Kotlin's raw LLVM return and updates PositionHolder terminator state. Early-return conditional bodies use that state; protected source access is retained and exposed only by a test-derived context. ret_value / ret_void / on_return and their runtime cleanup remain untranslated |
| FunctionGenerationContext.phi / add_phi_incoming / assign_phis | tools/kotlinc_native_ref/CodeGenerator.cpp:102, :106, :115; IrToBitcode_coroutines.cpp:24 | Wired into compiled coroutine expression helpers | Translates source phi creation, ordered incoming value/block arrays and current-block assignment. Jump emits its branch before assigning the predecessor, including Kotlin's after-terminator unreachable-block creation. Complete expression evaluation and scope resolution remain unported |
| IrClass.is_restricted_suspension | tools/clang_suspend_plugin/RestrictSuspensionUtils.cpp:22 | Wired | Nullable class, direct annotation and superclass annotation predicates translated; Clang redeclaration adapter explicit |
| IrFunction.is_restricted_suspension_function | tools/clang_suspend_plugin/RestrictSuspensionUtils.cpp:37 | Wired | Explicit receiver role; preserves source nonlocal return from inline any, examining first parameter |
| IrClass.get_all_superclasses / collect_all_superclasses | tools/clang_suspend_plugin/IrTypeUtils.cpp:25, :11 | Wired | Recursive traversal with canonical class identity and set insertion; unrelated IrTypeUtils functions remain untranslated |
| ContinuationImpl constructors / context | ContinuationImpl.hpp:97, :101, :105; ContinuationImpl.cpp:87, :92, :97 | Wired; source requirements tested | Explicit or inherited context retained; null context rejected, matching `_context!!`; no empty-context substitution |
| BaseContinuationImpl.resume_with completion requirement | ContinuationImpl.cpp:21 | Wired; source requirement tested | Missing completion raises before invoke_suspend, matching `completion!!`; other base-runtime parity gaps remain |
| RestrictedContinuationImpl constructor / context | ContinuationImpl.hpp:87, :90; ContinuationImpl.cpp:104, :113 | Wired; runtime requirement tested | Nullable parent accepted; any present parent's context must be the canonical EmptyCoroutineContext. Named receivers and tested lambda invokes select this base; broader reference and closure ownership parity remain incomplete |
| ContinuationImpl.intercepted / releaseIntercepted | ContinuationImpl.cpp:118, :130 | Wired | Cached context interceptor, release hook and completed sentinel |
| Continuation<T>.intercepted intrinsic | intrinsics/IntrinsicsNative.cpp:8 | Wired for erased ABI | Only compiler-frame continuations are intercepted; ordinary continuations remain unchanged |
| ContinuationInterceptor default release | ContinuationInterceptor.cpp:8 | Wired | Kotlin default is a no-op |
| CoroutineDispatcher virtual erased interception | CoroutineDispatcher.hpp:137, :141; CoroutineDispatcher.cpp:41 | Wired | Exposes Continuation<void*> interception and release through the context interceptor, alongside the typed template |
| yield | Yield.hpp:100, :110; Yield.cpp:27 | Plugin-backed for explicit and implicit continuation calls | Clang lowers `yield()` to the real ABI using the current frame; both forms retain locals through repeated queued resumption. The no-argument intrinsic requires a lowered suspend body. The thread/event-loop compatibility implementation was removed. Dispatcher and Unconfined runtime branches retain Kotlin ordering; queued resumption checks cancellation |
| Dispatchers.Unconfined (native) | native/Dispatchers.cpp:101 | Wired | Returns the existing canonical Unconfined object |

Detailed confirmed gaps, Kanban cards and validation scope:
[LOGIC_PARITY_REPAIR.md](LOGIC_PARITY_REPAIR.md). These entries do not certify
automatic compiler lowering or the whole Kotlin/Native binary ABI.

## Builders

### launch, async, runBlocking

**Source**: `tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src/Builders.common.kt`  
**C++ Header**: `src/kotlinx/coroutines/CoroutineScope.hpp`, `src/kotlinx/coroutines/Builders.hpp`

| Kotlin API | C++ API | Status | Notes |
|------------|---------|--------|-------|
| `fun CoroutineScope.launch(context, start, block): Job` | `launch(scope, context, start, suspend_block)` | Wired | Suspend block receives a shared erased continuation: `Builders.hpp:55`, implementation `Builders.common.cpp:23`; legacy synchronous overloads remain |
| `fun CoroutineScope.async(context, start, block): Deferred<T>` | `async<T>(scope, context, start, suspend_block)` | Wired for erased suspend blocks | Builders.hpp:276; DEFAULT, LAZY and UNDISPATCHED; receiving adapter owns unboxing/deletion |
| `fun <T> runBlocking(context, block): T` | ✅ | ✅ | In Builders.hpp |
| `fun CoroutineScope.produce(context, capacity, start, onCompletion, block): ReceiveChannel<E>` | `produce(scope, context, capacity, start, on_completion, suspend_block)` | Wired | `channels/Produce.hpp:187` forwards to the source internal builder at :159; completion registration precedes start. Block-first overloads at :200,220 expose source defaults. |
| `fun <T> withContext(context, block): T` | `with_context<T>(context, block, completion)` | Wired | Builders.hpp:351; source fast paths and dispatcher-change decision loop. Queued return checks cancellation before typed result boxing; BuildersTest executes both return outcomes |
| `fun <T> withTimeout(timeMillis, block): T` | ❌ | ❌ MISSING | Timeout wrapper |
| `fun <T> withTimeoutOrNull(timeMillis, block): T?` | ❌ | ❌ MISSING | Nullable timeout |
| `suspend fun <T> coroutineScope(block): T` | `coroutine_scope<T>(block, completion)` | Wired | Builders.hpp:429; actual ScopeCoroutine starts the block, waits for children and propagates child failure. Child completion returns through the caller dispatcher |
| `suspend fun <T> supervisorScope(block): T` | `supervisor_scope<T>(block, completion)` | Wired | Supervisor.hpp:100; actual SupervisorCoroutine waits for children, isolates child failure and rethrows block failure |

Native context extensions: CoroutineScope.hpp:105 and :109 expose both
new_coroutine_context overloads; native/CoroutineContext.cpp:29 and :39 implement
Native CoroutineContext.kt:32-40. Suspend launch and async use those actuals.

**Status**: **PARTIAL** - The scoped builder paths above are translated and exercised; broader builder parity is measured by the deep reports.

---

## Sharing continuation audit (2026-10-04)

These entries describe the repaired paths, not whole-library completion. They are
additional to the core-interface counts below.

| Kotlin API / implementation | C++ reference | Status | Verified behavior / limit |
|---|---|---|---|
| `shareIn(scope, started, replay)` and `stateIn(scope, started, initialValue)` sharing launch | `flow/Share.hpp:261` | Wired | Eager uses DEFAULT; other strategies use UNDISPATCHED; command collection uses collectLatest, including cancel/join/reset |
| `StartedWhileSubscribed.command` | `flow/SharingStarted.cpp:195` | Wired | Cancellable dispatcher delay, STOP then expiration delay then RESET; zero expiration skips STOP |
| `StartedLazily.command` | `flow/SharingStarted.cpp:146` | Wired | Collector and started flag survive suspension; emits START only once |
| `collectLatest(action)` | `flow/Collect.hpp:175`, `flow/Collect.cpp:120` | Wired | mapLatest(action).buffer(0).collect; action resumes before its Unit emission |
| `ChannelFlow.collectTo` suspend ABI | `flow/internal/ChannelFlow.hpp:327` | Wired for operator/builder/channel paths | Producer and SendingCollector remain alive through suspension; concurrent merge now uses translated child coroutines and semaphore suspension; whole-file parity is still measured by the deep report |
| `SendingCollector.emit(value)` | `flow/internal/SendingCollector.hpp:24` | Wired | Direct channel.send forwarding preserves the channel's original exception and suspension; no separate closed-channel branch. Kotlin SendingCollector.kt:15; executable evidence in CHANNEL_CONSUMPTION_SOURCE_REPAIR.md. |
| `FlowCollector.combineInternal(flows, arrayFactory, transform)` | `flow/internal/Combine.hpp:87`; `flow/internal/Combine.cpp:129,440` | Wired for erased source types | Source flowScope, real child coroutines, send/yield, last-child close, suspending batch receive, Byte epoch loop and both array-factory branches. Private algorithm/frames are concrete in .cpp; see COMBINE_SOURCE_REPAIR.md for executed cases and remaining typed public overload gaps. |
| `zipImpl(first, second, suspend transform)` / `Flow.zip` | `flow/internal/Combine.hpp:194`; `flow/internal/Combine.cpp:264,357,464`; `flow/Zip.hpp:49` | Wired for erased suspend ABI | Actual rendezvous producer, separate collect Job, original downstream context, receive/transform/emit suspension, AbortFlowException ownership and second cancellation in finally. Returned R boxes are owned/unboxed/deleted by the typed adapter. |
| `combine` suspend value transforms (Function2..5) | `flow/Zip.hpp:66,156,230,313`; `flow/internal/Combine.hpp:149`; `flow/Zip.cpp:15,56` | Wired for exercised erased ABI | Source transform-then-emit order with suspension at both calls, exact resumed failure and owned result unboxing/deletion. Unit uses the existing null ABI. Ordinary C++ transforms enter the same implementation. |
| Array / Iterable `combine`, `combineTransform` | `flow/Zip.hpp:401,444`; `flow/internal/Combine.hpp:87` | Wired for copyable typed inputs | Copied array factory for both; unsafeFlow for combine, safeFlow for combineTransform. Vector projects vararg and Iterable flowArray. Existing combine_all forwards to combine. Six-source transform suspension and empty inputs executed; std::any still restricts inputs to copy-constructible types. |
| Native `JobCancellationException.hashCode` | `native/Exceptions.hpp:30`; `native/Exceptions.cpp:104`; `src/kotlin/coroutines/cancellation/CancellationException.hpp:55` | Wired for source message/Job/Throwable carriers | UTF-16 polynomial message hash, virtual Job and cause hash dispatch, nullable cause zero and Kotlin Int wraparound. Nested equal causes, Unicode, embedded NUL, override failures and non-owning Job identity executed. toString remains absent. |
| Consumed `Any.hashCode`, `EmptyCoroutineContext.hashCode`, `CombinedContext.hashCode` | `CoroutineContext.hpp:114`; `context_impl.cpp:53,105,210`; `native/Exceptions.cpp:72` | Wired for C++ identity/context storage | Default Native low-32-bit identity primitive, empty hash zero and actual left+element sum. Combined equality/hash is invariant under element order; signed overflow avoided through unsigned arithmetic. This does not establish full Any/Throwable reflection or context serialization parity. |
| Consumed `AbstractCoroutineContextKey`, `getPolymorphicElement`, `minusPolymorphicKey`; interceptor `get`/`minusKey`; dispatcher companion Key | `context_impl.hpp:52,76,98,103,111`; `context_impl.cpp:12,21,27,32,41`; `ContinuationInterceptor.cpp:14,26`; `CoroutineDispatcher.hpp:74`; `CoroutineDispatcher.cpp:26,32` | Wired for shared C++ context storage | Root/self key identity, topmost key inheritance, safe-cast short-circuiting, nullable casts and exact exception propagation follow source. Dispatcher subtype lookup/removal and context replacement execute in regressions. Keys remain borrowed. Definitions now use kotlin::coroutines; explicit library imports refer to the same types. The strict compiler inventory recognizes these pairs. See STDLIB_COROUTINE_NAMESPACES.md; this is not complete stdlib parity. |
| Native flow exception constructors / `AbortFlowException.checkOwnership` | `native/flow/internal/FlowExceptions.cpp:12,16`; `flow/internal/FlowExceptions.common.cpp:11`; `native/flow/internal/FlowExceptions.hpp:15` | Wired for exercised exception transport | Native messages and immutable owner match source; a mismatched owner rethrows the same active exception object. Matching owner returns; unrelated active exception does not replace the receiver. C++ first throws box a value. |
| `stateIn(scope)` deferred overload | `flow/Share.hpp:282` | Surface / existing wiring | Deferred sharing path is outside this repair; no new suspension-parity claim |
| `LockFreeLinkedListNode.next`, `addLast(node, permissionsBitmask)`, `addNext`, `removeOrNext`, `close`, `toString` | `internal/LockFreeLinkedList.hpp:18`, `:22`, `:26`, `:28`, `:29`, `:31`; `internal/LockFreeLinkedList.common.cpp:63` | Wired | Atomic publication, Removed marker, predecessor repair and permission closure follow the concurrent Kotlin source; C++ marker storage owns markers, while intrusive node reclamation still requires external lifetime management |
| `JobSupport.attachChild` and completion registration | `JobSupport.cpp:721`, `:1137`, `:1194`, `:1289` | Wired | Registration respects cancellation/child/completion closure; a published single-to-list promotion survives a losing state CAS; completing flag is set under the Finishing lock before publication |
| BufferedChannel waiter unwrap and suspended send handshake | `channels/BufferedChannel.hpp:433`, `:2022`, `:2805`, `:2884`; `channels/BufferedChannel.cpp:17` | Wired for exercised paths | Plain Waiter is preserved; WaiterEB is unwrapped conditionally; a concrete Unit/erased adapter and get_result handshake preserve send resumption; sender-first and receiver-first regressions exercise suspension in both directions |

---

## Summary Statistics

These counts describe the listed surfaces and earlier assessments. They are not
a fresh AST-distance measurement or a guarantee of semantic parity.

| Category | Implemented | Partial | Missing | Total |
|----------|-------------|---------|---------|-------|
| Job | 13 | 1 | 0 | 14 |
| CoroutineDispatcher | 9 | 0 | 0 | 9 |
| Deferred | 4 | 0 | 0 | 4 |
| Dispatchers | 4 | 0 | 0 | 4 |
| Delay | 3 | 0 | 0 | 3 |
| Builders | 3 | 4 | 2 | 9 |
| **TOTAL** | **36** | **5** | **2** | **43** |

**Listed-row coverage**: ~84% (36/43 marked implemented; not whole-library parity)

---

## Current work

Library source transliteration and its consumed state-machine, IR-lowering and
continuation-injection paths are the active port. The compiler-backed join source
repair and final eleven executable checks are recorded in MERGE_SOURCE_REPAIR.md.
The full-source priority order remains supplied by the deep reports.

### Earlier recorded surfaces and wiring
- ✅ `CoroutineDispatcher::dispatch()` / `is_dispatch_needed()` / `dispatch_yield()` / `limited_parallelism()`
- ✅ `Delay::delay()` / `schedule_resume_after_delay()` / `invoke_on_timeout()`
- ✅ `Dispatchers.Default`, `Dispatchers.Main`, `Dispatchers.Unconfined`, `Dispatchers.IO`
- ✅ Suspend `join()` and `await()` using Continuation ABI
- ✅ Select clause foundations (`on_join`, `on_await`)
- Mandatory in-process LLVM address injection is implemented; complete Kotlin IR state-machine construction and shared-frame handoff remain open.

---

## Notes for Implementation

### Suspend Functions
Suspend functions are lowered to Kotlin/Native Continuation ABI: `void* fn(args..., Continuation<void*>* cont)` returning either the result or `intrinsics::COROUTINE_SUSPENDED`. State machine address injection runs inside Clang through the mandatory `KotlinxCoroutinePass` LLVM module plugin (`-fpass-plugin`). Production compilation does not serialize/reparse IR or use a Python compile launcher. Compiler-driven suspension lowering and its remaining parity gaps are recorded in the architecture and deep audit evidence.

### Thread Safety
Thread safety and dispatcher behavior require implementation-specific verification. API presence and mutex/atomic use alone do not establish Kotlin parity.

### Error Handling
Uses `std::exception_ptr` and `kotlinx::coroutines::Result<T>` matching Kotlin Native's exception model.

---

**Last Updated**: October 2026  
Current measurements: [full-library deep reports](project-wide/library/port_status_report.md)
