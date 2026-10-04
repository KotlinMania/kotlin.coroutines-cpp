# Sharing handoff verification

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
