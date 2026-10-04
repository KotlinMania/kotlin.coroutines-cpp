# Sharing handoff verification

## Inherited checkpoint (e9e35d76)

The inherited ten-file patch was reviewed on 2026-10-04 against Kotlin sources
under `tmp/kotlinx.coroutines` and the continuation lowering design in
`docs/suspension` and `docs/architecture/docking_ring.md`. Its parent is
`6f630dd95`. This checkpoint preserves the inherited repairs; it does not
certify complete suspension or behavioral parity.

Fresh Apple Clang 21 Debug configuration: `build-sharing-review`, optional
Clang suspend plugin, kxs injector, and AST-distance targets disabled. The core
and all test targets build. CTest passes 19/20, including the IR pipeline test.
`test_share_hot_flow_smoke` aborts after the subscription-count, eager, and lazy
cases pass; replay reset after unsubscribe remains unresolved. Raw build/test
receipts and the original patch are retained in ignored `tmp/sharing-handoff/`.

## Inherited repairs and limitations

- `Distinct.hpp:79` moves distinct state off the stack. `Limit.hpp:37`,
  `internal/SafeCollector.hpp:163`, and `internal/ChannelFlow.hpp:212` retain
  collectors/channel/scope via Job callbacks. This avoids some dangling stack
  references but does not establish operation lifetime when the context has no
  Job, or release these resources when collection finishes before its Job.
- Kotlin `flow/operators/Limit.kt:10-11` aliases `unsafeFlow` to `flow`; the
  builder changes in the inherited patch follow that alias. `emitAbort` still
  needs to wait for suspended final emission before throwing, and collection
  must catch owned aborts on asynchronous completion.
- `BufferedChannel.hpp:2083` and `:2138` restore cancellation callbacks, and
  iterator failed resume deletes its result box. Callback ownership and
  exception routing require further upstream comparison; these edits alone do
  not establish complete BufferedChannel parity.
- `JobSupport.hpp:315` and `internal/ScopeCoroutine.hpp:56` now expose the
  virtual `is_scoped_coroutine()` hook used by context validation. The
  `get_is_scoped_coroutine()` compatibility accessor remains available.
- `SharingStarted.cpp:124` removes the added buffer. Its polling timeouts still
  diverge from `SharingStarted.kt:171-185` and must use cancellable `delay`.
- `Share.hpp:182` still uses a synchronous launch body and blocking joins.
  Kotlin `flow/operators/Share.kt:189-237` uses conditional start and
  `collectLatest` with suspension tied to coroutine completion.
- Concurrent merge paths retaining stack references, starting OS threads, or
  discarding results through `NoopContinuation` remain a separate unresolved
  finding from the adverse review.

AST-distance is intended as a Kotlin-library/C++ source comparison alongside
the compiler lowering oracle, not a replacement for suspension tests. No fresh
score was obtained: its execution guard rejected this tool process tree. Old
scores and board completion labels are not validation of this checkpoint.

Sydney explicitly authorized Codex to take over writing, verify/commit the
inherited patch, then fix sharing. This supersedes the previous Limit-card
reviewer-only assignment without dispatching another writer.


## Sharing repair following the checkpoint

The repair follows `flow/operators/Share.kt:189-237`,
`SharingStarted.kt:148-185`, `flow/terminal/Collect.kt:82-97`, and
`flow/internal/Merge.kt:19-34`. It uses the current Continuation ABI and explicit
heap frames. Kotlin/Native's compiler/runtime remain the lowering oracle; this
is not a claim of binary compatibility or completed automatic variable spilling.

| Repaired path | Current C++ evidence | Upstream behavior |
|---|---|---|
| Sharing launch | `flow/Share.hpp:189`, `:261` | DEFAULT for Eagerly, UNDISPATCHED otherwise; eager collection, lazy first-subscriber wait, or command collectLatest |
| WhileSubscribed commands | `flow/SharingStarted.cpp:195` | delay(stopTimeout), optional STOP and delay(replayExpiration), then RESET; dropWhile and distinct remain; no injected buffer or polling sleep |
| Lazy commands | `flow/SharingStarted.cpp:146` | Collection frame owns count collector and started flag until cancellation |
| Latest collection | `flow/Collect.cpp:50`, `:120`; `flow/internal/Merge.hpp:65` | Action completion precedes Unit emission; buffer(0) rendezvous; cancel previous child, suspend on join, then UNDISPATCHED launch within coroutineScope |
| Hot-flow loops | `flow/SharedFlow.hpp:311`; `flow/StateFlow.hpp:373` | Wait, resume loop, check cancellation, emit, and free subscription slot on exceptional exit |
| Producer/scope handoff | `channels/Produce.hpp:187`; `flow/internal/ChannelFlow.cpp:73`; `flow/internal/ChannelFlow.hpp:230`, `:327` | Suspension leaves producer active; owned SendingCollector/channel survive; scope waits for child completion |
| Cold-flow lifetime | `flow/FlowBuilders.hpp:146`; `flow/internal/SafeCollector.hpp:147` | Iterable frame retains index across emit; SafeCollector is released in collection's finally path |
| Cancellable join | `JobSupport.cpp:597` | Completion resumes a cancellable continuation; caller cancellation disposes completion registration; completed join checks caller activity |
| Completion ordering | `JobSupport.cpp:1485`; `AbstractCoroutine.hpp:182` | Finalization invokes handlers; AbstractCoroutine afterResume or deferred child completion resumes scope once, rather than both |
| Channel cancellation state | `CancellableContinuationImpl.hpp:884`, `:1518` | CAS the actual SegmentBase into the NotCompleted state union, so cancellation clears waiter cells before iterator destruction |

`BaseContinuationImpl::resume_with` retains current and completion frames while
unrolling the parent chain, replacing Kotlin GC's lifetime protection during
release/transfer. ChannelAsFlow's producer overload keeps its actual shared
completion, rather than converting it to a borrowed raw pointer.

Root `CMakeLists.txt` enables `kxs_enable_suspend` for the production core and
C++ test targets. The launcher removes no-op markers and preserves the computed
labels, spilled fields, sentinel checks and resumed Result handling already in
source. It does not synthesize missing frames.

### Validation coverage

- Sharing smoke tests now use a suspended, cancellable upstream and verify cache
  reset, a second subscriber receiving a restarted upstream value, and stateIn
  reset to its initial value.
- Virtual dispatcher tests cover cancellation of the stop and expiration timers,
  initial STOP/RESET suppression, duplicate START suppression, zero expiration,
  infinite stop timeout, suspended STOP emission, and resumed emission failure.
- A single-thread virtual test keeps cancelled child cleanup suspended for 50ms.
  The next action must not launch until cleanup finishes, and collection completes
  exactly once.
- Lazy command collection emits START once over multiple count changes and reports
  exactly one cancellation completion.
- The old Collect/Reduce smoke fixture ignored emit's suspension result. It now
  uses the translated iterable flow, which resumes the emission loop correctly.

Build/test receipts are retained under ignored `tmp/sharing-handoff/`. The final
validation commands use Debug `build-sharing-review` and an optimized
AddressSanitizer `build-sharing-asan` core built with
`-fsanitize=address -fno-omit-frame-pointer`. Leak detection is disabled for the
ASan runs; these receipts establish address-safety for the exercised paths, not
leak freedom or universal race freedom.

### Remaining findings outside this repair

- Concurrent merge implementations still use raw threads, stack captures and
  NoopContinuation; the legacy collectTo fallback does not confer suspension
  parity on those implementations.
- Limit emitAbort/take asynchronous abort handling, and Distinct/Limit collector
  ownership in contexts without a Job, remain unresolved adverse-review findings.
- Deferred stateIn(scope), other unchecked emitter loops, global dispatcher
  interception, result ownership and GC/ABI compatibility need separate audits.
- BufferedChannel callback exception routing and all cancellation races are not
  certified by the focused regression tests.
- No fresh ast_distance score is available: its redirect/process guard rejects
  the execution environment and computer-use Terminal access is denied. No guard
  was bypassed. Historical AST coverage is not a receipt for this repair, and
  unsupported 0/0 type coverage is not 100% parity.

`FLOW_IMPLEMENTATION_STATUS.md` never existed, per the corrected reviewer
handoff. AGENTS.md, docking_ring.md, the IR parity audit, this handoff audit,
TRANSLITERATION_STATUS.md, port_status_report.md and API_AUDIT.md were read in
full. The failed read that had been summarized as successful is not evidence.


### Checkpoint validation and remaining list dependency

Debug: 21/21 CTest tests passed, including test_ir_pipeline (30.94s total).
Optimized ASan: the focused five tests passed on rerun (1.50s total).
An earlier ASan Collect/Reduce smoke run timed out in legacy collectLatest;
600 subsequent direct ASan executions passed. This intermittent stall is not
claimed resolved by reruns. Inspection found that the existing
LockFreeLinkedList.common.cpp publishes a node before initializing its links,
uses unconditional stores during removal, and leaves close/help methods empty.
The actual concurrent Kotlin list algorithm is a required follow-up dependency
for reliable completion registration. This checkpoint is not the final repair.
