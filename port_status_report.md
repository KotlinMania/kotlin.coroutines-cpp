# Code Port - Progress Report

**Generated:** 2026-08-28
**Source:** tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src
**Target:** src/kotlinx/coroutines

## Executive Summary

| Metric | Count | Percentage |
|--------|-------|------------|
| Function parity | 633/900 matched (target 1875) | 70.3% |
| Class/type parity | 0/0 matched | N/A |
| Combined symbol parity | 633/900 matched (target 1875) | 70.3% |
| Average function body similarity | 0.28 | inline-code cosine |
| Average documentation similarity | 0.53 | doc text cosine |
| Missing source functions | 0 | 0% parity until ported |
| Missing source classes/types | 0 | 0% parity until ported |
| Missing source symbol files | 0 | 0 symbols |
| Cheat/scoring failures | 20 | forced to 0% |
| Total source files | 111 | 100% |
| Target units (paired) | 191 | - |
| Target files (total) | 280 | - |
| Porting progress | 111 | 100.0% (matched) |
| Missing files | 0 | 0.0% |

## Port Quality Analysis

**Average Function Similarity:** 0.28

Similarity in this report is the required function-by-function body/parameter score. Class/type parity and symbol deficits are reported beside it; whole-file shape is diagnostic only.

**Work Distribution:**
- Critical (<0.60): 105 files (94.6% of matched)
- Needs review (0.60-0.84): 4 files (3.6% of matched)

## Worst Function Scores First

Every matched file is listed from lowest function body/parameter similarity upward. Missing symbol names are not capped.

| Rank | Source | Target | Function similarity | Functions | Missing functions | Types | Missing types | Tests | Symbol deficit | Priority |
|------|--------|--------|---------------------|-----------|-------------------|-------|---------------|-------|----------------|----------|
| 1 | `channels.Deprecated` | `channels.Deprecated [STUB]` | 0.00 | 0/44 matched (target 0) | `consume`, `consumeEach`, `consumesAll`, `elementAt`, `elementAtOrNull`, `first`, `firstOrNull`, `indexOf`, `last`, `lastIndexOf`, `lastOrNull`, `single`, `singleOrNull`, `drop`, `dropWhile`, `filter`, `filterIndexed`, `filterNot`, `filterNotNull`, `filterNotNullTo`, `take`, `takeWhile`, `toChannel`, `toCollection`, `toMap`, `toMutableList`, `toSet`, `flatMap`, `map`, `mapIndexed`, `mapIndexedNotNull`, `mapNotNull`, `withIndex`, `distinct`, `distinctBy`, `toMutableSet`, `any`, `count`, `maxWith`, `minWith`, `none`, `requireNoNulls`, `zip`, `consumes` | 0/0 matched | _none_ | - | 44 | 444410.0 |
| 2 | `EventLoop.common` | `native.EventLoop [STUB]` | 0.00 | 0/33 matched (target 0) | `processNextEvent`, `processUnconfinedEvent`, `shouldBeProcessedFromContext`, `dispatchUnconfined`, `delta`, `incrementUseCount`, `decrementUseCount`, `limitedParallelism`, `shutdown`, `currentOrNull`, `resetEventLoop`, `setEventLoop`, `delayToNanos`, `delayNanosToMillis`, `scheduleResumeAfterDelay`, `scheduleInvokeOnTimeout`, `dispatch`, `enqueue`, `enqueueImpl`, `dequeue`, `enqueueDelayedTasks`, `closeQueue`, `schedule`, `shouldUnpark`, `scheduleImpl`, `resetAll`, `rescheduleAllDelayed`, `compareTo`, `timeToExecute`, `scheduleTask`, `dispose`, `toString`, `run` | 0/0 matched | _none_ | - | 33 | 333310.0 |
| 3 | `intrinsics.Undispatched` | `intrinsics.Undispatched [STUB]` | 0.00 | 0/6 matched (target 0) | `startCoroutineUndispatched`, `startUndispatchedOrReturn`, `startUndispatchedOrReturnIgnoreTimeout`, `startUndspatched`, `notOwnTimeout`, `dispatchExceptionAndMakeCompleting` | 0/0 matched | _none_ | - | 6 | 60610.0 |
| 4 | `CloseableCoroutineDispatcher` | `CloseableCoroutineDispatcher [ZERO]` | 0.00 | 0/0 matched (target 1) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 5 | `CompletableJob` | `CompletableJob [ZERO]` | 0.00 | 0/0 matched (target 2) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 6 | `CoroutineContext.common` | `CoroutineContext.common [ZERO]` | 0.00 | 0/0 matched (target 19) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 7 | `Debug.common` | `Debug.common [ZERO]` | 0.00 | 0/0 matched (target 4) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 8 | `Deferred` | `Deferred [ZERO]` | 0.00 | 0/0 matched (target 4) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 9 | `Dispatchers.common` | `Dispatchers [ZERO]` | 0.00 | 0/0 matched (target 18) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 10 | `Exceptions.common` | `Exceptions [ZERO]` | 0.00 | 0/0 matched (target 13) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 11 | `Runnable.common` | `Runnable [STUB]` | 0.00 | 0/0 matched (target 6) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 12 | `Waiter` | `Waiter [ZERO]` | 0.00 | 0/0 matched (target 2) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 13 | `flow.FlowCollector` | `flow.FlowCollector [ZERO]` | 0.00 | 0/0 matched (target 1) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 14 | `internal.LocalAtomics.common` | `internal.LocalAtomics.common [ZERO]` | 0.00 | 0/0 matched (target 4) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 15 | `internal.LockFreeLinkedList.common` | `internal.LockFreeLinkedList.common [ZERO]` | 0.00 | 0/0 matched (target 14) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 16 | `internal.NullSurrogate` | `internal.NullSurrogate [ZERO]` | 0.00 | 0/0 matched (target 3) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 17 | `internal.ProbesSupport.common` | `internal.ProbesSupport.common [ZERO]` | 0.00 | 0/0 matched (target 2) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 18 | `internal.StackTraceRecovery.common` | `internal.StackTraceRecovery [ZERO]` | 0.00 | 0/0 matched (target 9) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 19 | `internal.ThreadContext.common` | `internal.ThreadContext [ZERO]` | 0.00 | 0/0 matched (target 7) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 20 | `internal.ThreadLocal.common` | `internal.ThreadLocal [ZERO]` | 0.00 | 0/0 matched (target 8) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 21 | `flow.Migration` | `flow.Migration` | 0.01 | 1/26 matched (target 1) | `observeOn`, `publishOn`, `subscribeOn`, `onErrorResume`, `onErrorResumeNext`, `subscribe`, `flatMap`, `concatMap`, `merge`, `flatten`, `compose`, `skip`, `forEach`, `scanFold`, `onErrorReturn`, `startWith`, `concatWith`, `combineLatest`, `delayFlow`, `delayEach`, `switchMap`, `scanReduce`, `publish`, `replay`, `cache` | 0/0 matched | _none_ | - | 25 | 252609.9 |
| 22 | `terminal.Collect` | `flow.Collect` | 0.03 | 1/6 matched (target 2) | `collect`, `launchIn`, `collectIndexed`, `collectLatest`, `emitAll` | 0/0 matched | _none_ | - | 5 | 50609.7 |
| 23 | `internal.Scopes` | `internal.Scopes` | 0.05 | 1/5 matched (target 4) | `getStackTraceElement`, `afterCompletion`, `afterCompletionUndispatched`, `afterResume` | 0/0 matched | _none_ | - | 4 | 40509.5 |
| 24 | `operators.Transform` | `flow.Transform` | 0.05 | 4/12 matched (target 6) | `filterNot`, `filterIsInstance`, `withIndex`, `onEach`, `scan`, `runningFold`, `runningReduce`, `chunked` | 0/0 matched | _none_ | - | 8 | 81209.5 |
| 25 | `internal.SafeCollector.common` | `internal.SafeCollector.common` | 0.05 | 1/4 matched (target 5) | `transitiveCoroutineParent`, `unsafeFlow`, `collect` | 0/0 matched | _none_ | - | 3 | 30409.5 |
| 26 | `operators.Lint` | `flow.Lint` | 0.07 | 4/11 matched (target 4) | `cancel`, `catch`, `retry`, `retryWhen`, `toList`, `toSet`, `count` | 0/0 matched | _none_ | - | 7 | 71109.3 |
| 27 | `Yield` | `Yield` | 0.08 | 1/1 matched (target 7) | _none_ | 0/0 matched | _none_ | - | 0 | 109.2 |
| 28 | `terminal.Reduce` | `flow.Reduce` | 0.11 | 8/8 matched (target 26) | _none_ | 0/0 matched | _none_ | - | 0 | 808.9 |
| 29 | `Await` | `Await` | 0.12 | 2/6 matched (target 5) | `await`, `disposeAll`, `invoke`, `toString` | 0/0 matched | _none_ | - | 4 | 40608.8 |
| 30 | `channels.Broadcast` | `channels.Broadcast` | 0.13 | 3/8 matched (target 11) | `broadcast`, `cancel`, `cancelInternal`, `openSubscription`, `onStart` | 0/0 matched | _none_ | - | 5 | 50808.7 |
| 31 | `terminal.Count` | `flow.Count` | 0.14 | 1/1 matched (target 6) | _none_ | 0/0 matched | _none_ | - | 0 | 108.6 |
| 32 | `CoroutineScope` | `CoroutineScope` | 0.15 | 3/6 matched (target 12) | `plus`, `coroutineScope`, `currentCoroutineContext` | 0/0 matched | _none_ | - | 3 | 30608.5 |
| 33 | `operators.Errors` | `flow.Errors` | 0.15 | 3/6 matched (target 3) | `catchImpl`, `isCancellationCause`, `isSameExceptionAs` | 0/0 matched | _none_ | - | 3 | 30608.5 |
| 34 | `operators.Emitters` | `flow.Emitters` | 0.16 | 4/8 matched (target 5) | `unsafeTransform`, `ensureActive`, `emit`, `invokeSafely` | 0/0 matched | _none_ | - | 4 | 40808.4 |
| 35 | `Job` | `Job` | 0.16 | 7/14 matched (target 35) | `plus`, `Job`, `Job0`, `disposeOnCompletion`, `cancelAndJoin`, `orCancellation`, `invoke` | 0/0 matched | _none_ | - | 7 | 71408.4 |
| 36 | `operators.Delay` | `flow.Delay` | 0.16 | 5/6 matched (target 8) | `timeoutInternal` | 0/0 matched | _none_ | - | 1 | 10608.4 |
| 37 | `CancellableContinuation` | `CancellableContinuation` | 0.17 | 3/7 matched (target 37) | `suspendCancellableCoroutineReusable`, `getOrCreateCancellableContinuation`, `invoke`, `toString` | 0/0 matched | _none_ | - | 4 | 40708.3 |
| 38 | `Builders.common` | `Builders.common` | 0.17 | 7/13 matched (target 44) | `invoke`, `trySuspend`, `tryResume`, `afterCompletion`, `afterResume`, `getResult` | 0/0 matched | _none_ | - | 6 | 61308.3 |
| 39 | `internal.Combine` | `internal.Combine` | 0.18 | 1/2 matched (target 3) | `combineInternal` | 0/0 matched | _none_ | - | 1 | 10208.2 |
| 40 | `internal.AbstractSharedFlow` | `internal.AbstractSharedFlow` | 0.19 | 3/4 matched (target 14) | `increment` | 0/0 matched | _none_ | - | 1 | 10408.1 |
| 41 | `channels.ChannelCoroutine` | `channels.ChannelCoroutine` | 0.20 | 2/2 matched (target 20) | _none_ | 0/0 matched | _none_ | - | 0 | 208.0 |
| 42 | `operators.Zip` | `flow.Zip` | 0.20 | 3/6 matched (target 7) | `combineUnsafe`, `combineTransformUnsafe`, `nullArrayFactory` | 0/0 matched | _none_ | - | 3 | 30608.0 |
| 43 | `terminal.Logic` | `flow.Logic` | 0.20 | 3/3 matched (target 5) | _none_ | 0/0 matched | _none_ | - | 0 | 308.0 |
| 44 | `flow.Builders` | `flow.FlowBuilders` | 0.21 | 8/11 matched (target 17) | `create`, `collectTo`, `toString` | 0/0 matched | _none_ | - | 3 | 31107.9 |
| 45 | `channels.Channels.common` | `channels.Channels.common` | 0.21 | 4/6 matched (target 13) | `receiveOrNull`, `onReceiveOrNull` | 0/0 matched | _none_ | - | 2 | 20607.9 |
| 46 | `operators.Context` | `flow.Context` | 0.23 | 5/6 matched | `checkFlowContext` | 0/0 matched | _none_ | - | 1 | 10607.7 |
| 47 | `operators.Distinct` | `flow.Distinct` | 0.24 | 3/3 matched (target 12) | _none_ | 0/0 matched | _none_ | - | 0 | 307.6 |
| 48 | `internal.InlineList` | `internal.InlineList` | 0.24 | 1/2 matched (target 4) | `plus` | 0/0 matched | _none_ | - | 1 | 10207.6 |
| 49 | `CoroutineName` | `CoroutineName` | 0.25 | 1/1 matched (target 4) | _none_ | 0/0 matched | _none_ | - | 0 | 107.5 |
| 50 | `internal.NamedDispatcher` | `internal.NamedDispatcher` | 0.26 | 2/4 matched | `isDispatchNeeded`, `dispatchYield` | 0/0 matched | _none_ | - | 2 | 20407.4 |
| 51 | `internal.ChannelFlow` | `internal.ChannelFlow` | 0.26 | 10/16 matched (target 32) | `collectWithContextUndispatched`, `withUndispatchedContextCollector`, `emit`, `withContextUndispatched`, `resumeWith`, `getStackTraceElement` | 0/0 matched | _none_ | - | 6 | 61607.4 |
| 52 | `CompletionState` | `CompletionState` | 0.26 | 3/5 matched (target 6) | `toString`, `makeResumed` | 0/0 matched | _none_ | - | 2 | 20507.4 |
| 53 | `JobSupport` | `JobImpl` | 0.27 | 60/75 matched (target 132) | `loopOnState`, `notifyHandlers`, `toCancellationException`, `invokeOnCompletionInternal`, `registerSelectForOnJoin`, `cancelInternal`, `cancelImpl`, `cancelMakeCompleting`, `toString`, `allocateList`, `getContinuationCancellationCause`, `onAwaitInternalRegFunc`, `onAwaitInternalProcessResFunc`, `handlesException`, `getString` | 0/0 matched | _none_ | - | 15 | 157507.3 |
| 54 | `internal.MainDispatcherFactory` | `internal.MainDispatcherFactory` | 0.28 | 1/1 matched (target 3) | _none_ | 0/0 matched | _none_ | - | 0 | 107.2 |
| 55 | `internal.LockFreeTaskQueue` | `internal.LockFreeTaskQueue` | 0.29 | 14/16 matched (target 28) | `wo`, `withState` | 0/0 matched | _none_ | - | 2 | 21607.1 |
| 56 | `channels.Channel` | `channels.Channel` | 0.29 | 14/19 matched (target 78) | `receiveOrNull`, `equals`, `hashCode`, `next0`, `Channel` | 0/0 matched | _none_ | - | 5 | 51907.1 |
| 57 | `flow.SharingStarted` | `flow.SharingStarted` | 0.29 | 3/5 matched (target 12) | `equals`, `hashCode` | 0/0 matched | _none_ | - | 2 | 20507.1 |
| 58 | `internal.SystemProps.common` | `internal.SystemProps.common` | 0.30 | 1/1 matched (target 10) | _none_ | 0/0 matched | _none_ | - | 0 | 107.0 |
| 59 | `selects.SelectOld` | `selects.SelectOld` | 0.30 | 7/7 matched (target 12) | _none_ | 0/0 matched | _none_ | - | 0 | 707.0 |
| 60 | `flow.Flow` | `flow.Flow` | 0.31 | 1/1 matched (target 4) | _none_ | 0/0 matched | _none_ | - | 0 | 13000107.0 |
| 61 | `flow.SharedFlow` | `flow.SharedFlow` | 0.31 | 28/31 matched (target 51) | `MutableSharedFlow`, `fuse`, `fuseSharedFlow` | 0/0 matched | _none_ | - | 3 | 33106.9 |
| 62 | `CoroutineExceptionHandler` | `CoroutineExceptionHandler` | 0.32 | 2/4 matched (target 3) | `handlerException`, `CoroutineExceptionHandler` | 0/0 matched | _none_ | - | 2 | 20406.8 |
| 63 | `Delay` | `Delay` | 0.32 | 3/4 matched (target 21) | `toDelayMillis` | 0/0 matched | _none_ | - | 1 | 10406.8 |
| 64 | `internal.ConcurrentLinkedList` | `internal.ConcurrentLinkedList` | 0.32 | 13/13 matched (target 61) | _none_ | 0/0 matched | _none_ | - | 0 | 1306.8 |
| 65 | `flow.StateFlow` | `flow.StateFlow` | 0.32 | 17/19 matched (target 45) | `MutableStateFlow`, `fuse` | 0/0 matched | _none_ | - | 2 | 21906.8 |
| 66 | `operators.Limit` | `flow.Limit` | 0.34 | 7/8 matched (target 13) | `emitAbort` | 0/0 matched | _none_ | - | 1 | 10806.6 |
| 67 | `channels.Produce` | `channels.Produce` | 0.34 | 4/4 matched (target 12) | _none_ | 0/0 matched | _none_ | - | 0 | 406.6 |
| 68 | `MainCoroutineDispatcher` | `MainCoroutineDispatcher` | 0.34 | 3/3 matched (target 4) | _none_ | 0/0 matched | _none_ | - | 0 | 306.6 |
| 69 | `terminal.Collection` | `flow.Collection` | 0.34 | 3/3 matched (target 8) | _none_ | 0/0 matched | _none_ | - | 0 | 306.6 |
| 70 | `CompletableDeferred` | `CompletableDeferred` | 0.34 | 5/6 matched (target 12) | `CompletableDeferred` | 0/0 matched | _none_ | - | 1 | 10606.6 |
| 71 | `internal.DispatchedTask` | `internal.DispatchedTask` | 0.35 | 9/10 matched (target 18) | `runUnconfinedEventLoop` | 0/0 matched | _none_ | - | 1 | 11006.5 |
| 72 | `internal.LimitedDispatcher` | `internal.LimitedDispatcher` | 0.35 | 10/10 matched (target 14) | _none_ | 0/0 matched | _none_ | - | 0 | 1006.5 |
| 73 | `selects.OnTimeout` | `selects.OnTimeout` | 0.35 | 1/2 matched (target 7) | `register` | 0/0 matched | _none_ | - | 1 | 10206.5 |
| 74 | `CancellableContinuationImpl` | `CancellableContinuationImpl` | 0.35 | 34/43 matched (target 113) | `getStackTraceElement`, `callCancelHandlerSafely`, `invokeOnCancellationInternal`, `multipleHandlersError`, `tryResumeImpl`, `alreadyResumedError`, `getExceptionalResult`, `nameString`, `invokeHandlers` | 0/0 matched | _none_ | - | 9 | 94306.5 |
| 75 | `operators.Merge` | `flow.Merge` | 0.35 | 8/8 matched (target 19) | _none_ | 0/0 matched | _none_ | - | 0 | 806.5 |
| 76 | `internal.DispatchedContinuation` | `internal.DispatchedContinuation` | 0.36 | 18/18 matched (target 31) | _none_ | 0/0 matched | _none_ | - | 0 | 1806.4 |
| 77 | `flow.Channels` | `flow.Channels` | 0.37 | 12/12 matched (target 20) | _none_ | 0/0 matched | _none_ | - | 0 | 14001206.0 |
| 78 | `selects.Select` | `selects.Select` | 0.38 | 21/26 matched (target 73) | `onTimeout`, `register`, `processResultAndInvokeBlockRecoveringException`, `tryResume`, `TrySelectDetailedResult` | 0/0 matched | _none_ | - | 5 | 52606.2 |
| 79 | `sync.Mutex` | `sync.Mutex` | 0.39 | 11/16 matched (target 44) | `Mutex`, `tryResume`, `resume`, `trySelect`, `selectInRegistrationPhase` | 0/0 matched | _none_ | - | 5 | 51606.1 |
| 80 | `channels.BroadcastChannel` | `channels.BroadcastChannel` | 0.40 | 8/9 matched (target 51) | `BroadcastChannel` | 0/0 matched | _none_ | - | 1 | 10906.0 |
| 81 | `channels.BufferedChannel` | `channels.BufferedChannel` | 0.40 | 91/106 matched (target 164) | `sendImpl`, `receiveOnNoWaiterSuspend`, `receiveCatchingOnNoWaiterSuspend`, `receiveImpl`, `onClosedHasNext`, `hasNextOnNoWaiterSuspend`, `onClosedHasNextNoWaiterSuspend`, `invokeCloseHandler`, `updateSendersCounterIfLower`, `updateReceiversCounterIfLower`, `toStringDebug`, `checkSegmentStructureInvariants`, `onCancellationChannelResultImplDoNotCall`, `onCancellationImplDoNotCall`, `createSegmentFunction` | 0/0 matched | _none_ | - | 15 | 160606.0 |
| 82 | `sync.Semaphore` | `sync.Semaphore` | 0.40 | 18/19 matched (target 53) | `Semaphore` | 0/0 matched | _none_ | - | 1 | 11906.0 |
| 83 | `operators.Share` | `flow.Share` | 0.41 | 10/10 matched (target 25) | _none_ | 0/0 matched | _none_ | - | 0 | 1005.9 |
| 84 | `Unconfined` | `Unconfined` | 0.42 | 4/4 matched (target 8) | _none_ | 0/0 matched | _none_ | - | 0 | 405.8 |
| 85 | `selects.SelectUnbiased` | `selects.SelectUnbiased` | 0.42 | 4/4 matched (target 9) | _none_ | 0/0 matched | _none_ | - | 0 | 405.8 |
| 86 | `channels.ConflatedBufferedChannel` | `channels.ConflatedBufferedChannel` | 0.42 | 6/7 matched (target 9) | `registerSelectForSend` | 0/0 matched | _none_ | - | 1 | 10705.8 |
| 87 | `internal.ThreadSafeHeap` | `internal.ThreadSafeHeap` | 0.42 | 14/14 matched (target 20) | _none_ | 0/0 matched | _none_ | - | 0 | 1405.8 |
| 88 | `internal.SendingCollector` | `internal.SendingCollector` | 0.42 | 1/1 matched (target 2) | _none_ | 0/0 matched | _none_ | - | 0 | 105.8 |
| 89 | `Guidance` | `Guidance` | 0.44 | 2/2 matched | _none_ | 0/0 matched | _none_ | - | 0 | 205.6 |
| 90 | `Timeout` | `Timeout` | 0.44 | 7/7 matched (target 14) | _none_ | 0/0 matched | _none_ | - | 0 | 705.6 |
| 91 | `internal.FlowCoroutine` | `internal.FlowCoroutine` | 0.45 | 3/3 matched (target 6) | _none_ | 0/0 matched | _none_ | - | 0 | 305.5 |
| 92 | `internal.CoroutineExceptionHandlerImpl.common` | `internal.CoroutineExceptionHandlerImpl` | 0.48 | 1/1 matched (target 11) | _none_ | 0/0 matched | _none_ | - | 0 | 105.2 |
| 93 | `internal.FlowExceptions.common` | `internal.FlowExceptions` | 0.49 | 2/2 matched (target 4) | _none_ | 0/0 matched | _none_ | - | 0 | 205.1 |
| 94 | `Supervisor` | `Supervisor` | 0.50 | 4/4 matched (target 11) | _none_ | 0/0 matched | _none_ | - | 0 | 405.0 |
| 95 | `internal.NopCollector` | `internal.NopCollector` | 0.52 | 1/1 matched | _none_ | 0/0 matched | _none_ | - | 0 | 104.8 |
| 96 | `intrinsics.Cancellable` | `intrinsics.Cancellable` | 0.53 | 3/3 matched (target 15) | _none_ | 0/0 matched | _none_ | - | 0 | 304.7 |
| 97 | `internal.Symbol` | `internal.Symbol` | 0.53 | 2/2 matched (target 3) | _none_ | 0/0 matched | _none_ | - | 0 | 1000204.7 |
| 98 | `selects.WhileSelect` | `selects.WhileSelect` | 0.54 | 1/1 matched | _none_ | 0/0 matched | _none_ | - | 0 | 104.6 |
| 99 | `NonCancellable` | `NonCancellable` | 0.55 | 7/7 matched (target 36) | _none_ | 0/0 matched | _none_ | - | 0 | 704.5 |
| 100 | `internal.Merge` | `internal.Merge` | 0.55 | 5/5 matched (target 33) | _none_ | 0/0 matched | _none_ | - | 0 | 504.5 |
| 101 | `internal.OnUndeliveredElement` | `internal.OnUndeliveredElement` | 0.58 | 2/2 matched (target 3) | _none_ | 0/0 matched | _none_ | - | 0 | 204.2 |
| 102 | `CoroutineDispatcher` | `CoroutineDispatcher` | 0.59 | 7/7 matched (target 19) | _none_ | 0/0 matched | _none_ | - | 0 | 704.1 |
| 103 | `AbstractCoroutine` | `AbstractCoroutine` | 0.62 | 9/9 matched (target 14) | _none_ | 0/0 matched | _none_ | - | 0 | 903.8 |
| 104 | `CoroutineStart` | `CoroutineStart` | 0.64 | 1/1 matched (target 3) | _none_ | 0/0 matched | _none_ | - | 0 | 1000103.6 |
| 105 | `internal.Concurrent.common` | `internal.Concurrent.common` | 0.69 | 1/1 matched (target 20) | _none_ | 0/0 matched | _none_ | - | 0 | 103.1 |
| 106 | `internal.Synchronized.common` | `internal.SynchronizedObject` | 0.74 | 1/1 matched (target 5) | _none_ | 0/0 matched | _none_ | - | 0 | 102.6 |
| 107 | `channels.BufferOverflow` | `channels.BufferOverflow [STUB]` | 1.00 | 0/0 matched | _none_ | 0/0 matched | _none_ | - | 0 | 2000000.0 |
| 108 | `Annotations` | `Annotations` | 1.00 | 0/0 matched | _none_ | 0/0 matched | _none_ | - | 0 | 0.0 |
| 109 | `CompletionHandler.common` | `CompletionHandler [STUB]` | 1.00 | 0/0 matched | _none_ | 0/0 matched | _none_ | - | 0 | 0.0 |
| 110 | `SchedulerTask.common` | `SchedulerTask.common [STUB]` | 1.00 | 0/0 matched | _none_ | 0/0 matched | _none_ | - | 0 | 0.0 |
| 111 | `internal.InternalAnnotations.common` | `internal.InternalAnnotations.common` | 1.00 | 0/0 matched | _none_ | 0/0 matched | _none_ | - | 0 | 0.0 |

## Cheat Detection / Scoring Failures

- `channels.Deprecated` -> `channels.Deprecated [STUB]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `EventLoop.common` -> `native.EventLoop [STUB]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `intrinsics.Undispatched` -> `intrinsics.Undispatched [STUB]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `CloseableCoroutineDispatcher` -> `CloseableCoroutineDispatcher [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `CompletableJob` -> `CompletableJob [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `CoroutineContext.common` -> `CoroutineContext.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Debug.common` -> `Debug.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Deferred` -> `Deferred [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Dispatchers.common` -> `Dispatchers [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Exceptions.common` -> `Exceptions [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Runnable.common` -> `Runnable [STUB]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Waiter` -> `Waiter [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `flow.FlowCollector` -> `flow.FlowCollector [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.LocalAtomics.common` -> `internal.LocalAtomics.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.LockFreeLinkedList.common` -> `internal.LockFreeLinkedList.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.NullSurrogate` -> `internal.NullSurrogate [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.ProbesSupport.common` -> `internal.ProbesSupport.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.StackTraceRecovery.common` -> `internal.StackTraceRecovery [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.ThreadContext.common` -> `internal.ThreadContext [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.ThreadLocal.common` -> `internal.ThreadLocal [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only

### Critical Ports (Similarity < 0.60, Worst First)

These files need significant work:

- `channels.Deprecated` -> `channels.Deprecated [STUB]` (0.00)
- `EventLoop.common` -> `native.EventLoop [STUB]` (0.00)
- `intrinsics.Undispatched` -> `intrinsics.Undispatched [STUB]` (0.00)
- `CloseableCoroutineDispatcher` -> `CloseableCoroutineDispatcher [ZERO]` (0.00)
- `CompletableJob` -> `CompletableJob [ZERO]` (0.00)
- `CoroutineContext.common` -> `CoroutineContext.common [ZERO]` (0.00)
- `Debug.common` -> `Debug.common [ZERO]` (0.00)
- `Deferred` -> `Deferred [ZERO]` (0.00)
- `Dispatchers.common` -> `Dispatchers [ZERO]` (0.00)
- `Exceptions.common` -> `Exceptions [ZERO]` (0.00)
- `Runnable.common` -> `Runnable [STUB]` (0.00)
- `Waiter` -> `Waiter [ZERO]` (0.00)
- `flow.FlowCollector` -> `flow.FlowCollector [ZERO]` (0.00)
- `internal.LocalAtomics.common` -> `internal.LocalAtomics.common [ZERO]` (0.00)
- `internal.LockFreeLinkedList.common` -> `internal.LockFreeLinkedList.common [ZERO]` (0.00)
- `internal.NullSurrogate` -> `internal.NullSurrogate [ZERO]` (0.00)
- `internal.ProbesSupport.common` -> `internal.ProbesSupport.common [ZERO]` (0.00)
- `internal.StackTraceRecovery.common` -> `internal.StackTraceRecovery [ZERO]` (0.00)
- `internal.ThreadContext.common` -> `internal.ThreadContext [ZERO]` (0.00)
- `internal.ThreadLocal.common` -> `internal.ThreadLocal [ZERO]` (0.00)
- `flow.Migration` -> `flow.Migration` (0.01)
- `terminal.Collect` -> `flow.Collect` (0.03)
- `internal.Scopes` -> `internal.Scopes` (0.05)
- `operators.Transform` -> `flow.Transform` (0.05)
- `internal.SafeCollector.common` -> `internal.SafeCollector.common` (0.05)
- `operators.Lint` -> `flow.Lint` (0.07)
- `Yield` -> `Yield` (0.08)
- `terminal.Reduce` -> `flow.Reduce` (0.11)
- `Await` -> `Await` (0.12)
- `channels.Broadcast` -> `channels.Broadcast` (0.13)
- `terminal.Count` -> `flow.Count` (0.14)
- `CoroutineScope` -> `CoroutineScope` (0.15)
- `operators.Errors` -> `flow.Errors` (0.15)
- `operators.Emitters` -> `flow.Emitters` (0.16)
- `Job` -> `Job` (0.16)
- `operators.Delay` -> `flow.Delay` (0.16)
- `CancellableContinuation` -> `CancellableContinuation` (0.17)
- `Builders.common` -> `Builders.common` (0.17)
- `internal.Combine` -> `internal.Combine` (0.18)
- `internal.AbstractSharedFlow` -> `internal.AbstractSharedFlow` (0.19)
- `channels.ChannelCoroutine` -> `channels.ChannelCoroutine` (0.20)
- `operators.Zip` -> `flow.Zip` (0.20)
- `terminal.Logic` -> `flow.Logic` (0.20)
- `flow.Builders` -> `flow.FlowBuilders` (0.21)
- `channels.Channels.common` -> `channels.Channels.common` (0.21)
- `operators.Context` -> `flow.Context` (0.23)
- `operators.Distinct` -> `flow.Distinct` (0.24)
- `internal.InlineList` -> `internal.InlineList` (0.24)
- `CoroutineName` -> `CoroutineName` (0.25)
- `internal.NamedDispatcher` -> `internal.NamedDispatcher` (0.26)
- `internal.ChannelFlow` -> `internal.ChannelFlow` (0.26)
- `CompletionState` -> `CompletionState` (0.26)
- `JobSupport` -> `JobImpl` (0.27)
- `internal.MainDispatcherFactory` -> `internal.MainDispatcherFactory` (0.28)
- `internal.LockFreeTaskQueue` -> `internal.LockFreeTaskQueue` (0.29)
- `channels.Channel` -> `channels.Channel` (0.29)
- `flow.SharingStarted` -> `flow.SharingStarted` (0.29)
- `internal.SystemProps.common` -> `internal.SystemProps.common` (0.30)
- `selects.SelectOld` -> `selects.SelectOld` (0.30)
- `flow.Flow` -> `flow.Flow` (0.31, 13 deps)
- `flow.SharedFlow` -> `flow.SharedFlow` (0.31)
- `CoroutineExceptionHandler` -> `CoroutineExceptionHandler` (0.32)
- `Delay` -> `Delay` (0.32)
- `internal.ConcurrentLinkedList` -> `internal.ConcurrentLinkedList` (0.32)
- `flow.StateFlow` -> `flow.StateFlow` (0.32)
- `operators.Limit` -> `flow.Limit` (0.34)
- `channels.Produce` -> `channels.Produce` (0.34)
- `MainCoroutineDispatcher` -> `MainCoroutineDispatcher` (0.34)
- `terminal.Collection` -> `flow.Collection` (0.34)
- `CompletableDeferred` -> `CompletableDeferred` (0.34)
- `internal.DispatchedTask` -> `internal.DispatchedTask` (0.35)
- `internal.LimitedDispatcher` -> `internal.LimitedDispatcher` (0.35)
- `selects.OnTimeout` -> `selects.OnTimeout` (0.35)
- `CancellableContinuationImpl` -> `CancellableContinuationImpl` (0.35)
- `operators.Merge` -> `flow.Merge` (0.35)
- `internal.DispatchedContinuation` -> `internal.DispatchedContinuation` (0.36)
- `flow.Channels` -> `flow.Channels` (0.37, 14 deps)
- `selects.Select` -> `selects.Select` (0.38)
- `sync.Mutex` -> `sync.Mutex` (0.39)
- `channels.BroadcastChannel` -> `channels.BroadcastChannel` (0.40)
- `channels.BufferedChannel` -> `channels.BufferedChannel` (0.40)
- `sync.Semaphore` -> `sync.Semaphore` (0.40)
- `operators.Share` -> `flow.Share` (0.41)
- `Unconfined` -> `Unconfined` (0.42)
- `selects.SelectUnbiased` -> `selects.SelectUnbiased` (0.42)
- `channels.ConflatedBufferedChannel` -> `channels.ConflatedBufferedChannel` (0.42)
- `internal.ThreadSafeHeap` -> `internal.ThreadSafeHeap` (0.42)
- `internal.SendingCollector` -> `internal.SendingCollector` (0.42)
- `Guidance` -> `Guidance` (0.44)
- `Timeout` -> `Timeout` (0.44)
- `internal.FlowCoroutine` -> `internal.FlowCoroutine` (0.45)
- `internal.CoroutineExceptionHandlerImpl.common` -> `internal.CoroutineExceptionHandlerImpl` (0.48)
- `internal.FlowExceptions.common` -> `internal.FlowExceptions` (0.49)
- `Supervisor` -> `Supervisor` (0.50)
- `internal.NopCollector` -> `internal.NopCollector` (0.52)
- `intrinsics.Cancellable` -> `intrinsics.Cancellable` (0.53)
- `internal.Symbol` -> `internal.Symbol` (0.53, 1 deps)
- `selects.WhileSelect` -> `selects.WhileSelect` (0.54)
- `NonCancellable` -> `NonCancellable` (0.55)
- `internal.Merge` -> `internal.Merge` (0.55)
- `internal.OnUndeliveredElement` -> `internal.OnUndeliveredElement` (0.58)
- `CoroutineDispatcher` -> `CoroutineDispatcher` (0.59)

## Incorrect Ports (Missing Types)

These files are matched (often via `// port-lint`) but appear to be missing one or more type declarations
present in the Rust source file.

| Source | Target | Missing types | Examples |
|--------|--------|---------------|----------|
| _None detected_ | | | |

## High Priority Missing Files

No missing files detected.

## Documentation Gaps

**Documentation coverage:** 10070 / 9370 lines (107%)

Documentation gaps (>20%), complete list:

- `CancellableContinuation` - 41% gap (377 → 223 lines)
- `JobSupport` - 51% gap (231 → 114 lines)
- `flow.Migration` - 52% gap (216 → 103 lines)
- `selects.Select` - 28% gap (402 → 290 lines)
- `operators.Delay` - 45% gap (236 → 130 lines)
- `Job` - 22% gap (477 → 372 lines)
- `flow.StateFlow` - 51% gap (196 → 96 lines)
- `EventLoop.common` - 93% gap (99 → 7 lines)
- `channels.Produce` - 38% gap (226 → 139 lines)
- `operators.Share` - 41% gap (204 → 120 lines)
- `CoroutineScope` - 26% gap (320 → 238 lines)
- `flow.Builders` - 39% gap (201 → 122 lines)
- `operators.Emitters` - 66% gap (105 → 36 lines)
- `operators.Merge` - 46% gap (142 → 77 lines)
- `operators.Zip` - 48% gap (133 → 69 lines)
- `flow.SharedFlow` - 24% gap (245 → 186 lines)
- `CoroutineDispatcher` - 28% gap (213 → 154 lines)
- `CoroutineExceptionHandler` - 88% gap (67 → 8 lines)
- `channels.Deprecated` - 100% gap (48 → 0 lines)
- `terminal.Collect` - 74% gap (65 → 17 lines)
- `operators.Lint` - 71% gap (55 → 16 lines)
- `operators.Transform` - 51% gap (74 → 36 lines)
- `internal.DispatchedTask` - 41% gap (66 → 39 lines)
- `Deferred` - 29% gap (84 → 60 lines)
- `Builders.common` - 28% gap (75 → 54 lines)
- `Dispatchers.common` - 31% gap (62 → 43 lines)
- `terminal.Logic` - 21% gap (77 → 61 lines)
- `internal.DispatchedContinuation` - 21% gap (66 → 52 lines)
- `intrinsics.Undispatched` - 48% gap (25 → 13 lines)
- `CloseableCoroutineDispatcher` - 53% gap (17 → 8 lines)
- `SchedulerTask.common` - 54% gap (13 → 6 lines)
- `channels.Broadcast` - 100% gap (6 → 0 lines)

