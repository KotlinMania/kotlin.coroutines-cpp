# High Priority Ports - Action Plan

## Files by Impact

Priority = deps * 1,000,000 + SymDeficit * 10,000 + SrcSymbols * 100 + (1 - function similarity) * 10

Dependency fanout is ranked first so the ladder favors ports that clear downstream compilation failures fastest.

This list is complete and includes function/type detail for every matched file. Function similarity is the required body/parameter comparison; file-level shape does not rescue a port.

| Rank | Source | Target | Function similarity | Deps | Functions | Missing functions | Types | Missing types | SymDeficit | SrcSymbols | Priority |
|------|--------|--------|------------|------|-----------|-------------------|-------|---------------|-----------|------------|----------|
| 1 | `flow.Channels` | `flow.Channels` | 0.37 | 14 | 12/12 matched (target 20) | _none_ | 0/0 matched | _none_ | 0 | 12 | 14001206.0 |
| 2 | `flow.Flow` | `flow.Flow` | 0.31 | 13 | 1/1 matched (target 4) | _none_ | 0/0 matched | _none_ | 0 | 1 | 13000107.0 |
| 3 | `channels.BufferOverflow` | `channels.BufferOverflow [STUB]` | 1.00 | 2 | 0/0 matched | _none_ | 0/0 matched | _none_ | 0 | 0 | 2000000.0 |
| 4 | `internal.Symbol` | `internal.Symbol` | 0.53 | 1 | 2/2 matched (target 3) | _none_ | 0/0 matched | _none_ | 0 | 2 | 1000204.7 |
| 5 | `CoroutineStart` | `CoroutineStart` | 0.64 | 1 | 1/1 matched (target 3) | _none_ | 0/0 matched | _none_ | 0 | 1 | 1000103.6 |
| 6 | `channels.Deprecated` | `channels.Deprecated [STUB]` | 0.00 | 0 | 0/44 matched (target 0) | `consume`, `consumeEach`, `consumesAll`, `elementAt`, `elementAtOrNull`, `first`, `firstOrNull`, `indexOf`, `last`, `lastIndexOf`, `lastOrNull`, `single`, `singleOrNull`, `drop`, `dropWhile`, `filter`, `filterIndexed`, `filterNot`, `filterNotNull`, `filterNotNullTo`, `take`, `takeWhile`, `toChannel`, `toCollection`, `toMap`, `toMutableList`, `toSet`, `flatMap`, `map`, `mapIndexed`, `mapIndexedNotNull`, `mapNotNull`, `withIndex`, `distinct`, `distinctBy`, `toMutableSet`, `any`, `count`, `maxWith`, `minWith`, `none`, `requireNoNulls`, `zip`, `consumes` | 0/0 matched | _none_ | 44 | 44 | 444410.0 |
| 7 | `EventLoop.common` | `native.EventLoop [STUB]` | 0.00 | 0 | 0/33 matched (target 0) | `processNextEvent`, `processUnconfinedEvent`, `shouldBeProcessedFromContext`, `dispatchUnconfined`, `delta`, `incrementUseCount`, `decrementUseCount`, `limitedParallelism`, `shutdown`, `currentOrNull`, `resetEventLoop`, `setEventLoop`, `delayToNanos`, `delayNanosToMillis`, `scheduleResumeAfterDelay`, `scheduleInvokeOnTimeout`, `dispatch`, `enqueue`, `enqueueImpl`, `dequeue`, `enqueueDelayedTasks`, `closeQueue`, `schedule`, `shouldUnpark`, `scheduleImpl`, `resetAll`, `rescheduleAllDelayed`, `compareTo`, `timeToExecute`, `scheduleTask`, `dispose`, `toString`, `run` | 0/0 matched | _none_ | 33 | 33 | 333310.0 |
| 8 | `flow.Migration` | `flow.Migration` | 0.01 | 0 | 1/26 matched (target 1) | `observeOn`, `publishOn`, `subscribeOn`, `onErrorResume`, `onErrorResumeNext`, `subscribe`, `flatMap`, `concatMap`, `merge`, `flatten`, `compose`, `skip`, `forEach`, `scanFold`, `onErrorReturn`, `startWith`, `concatWith`, `combineLatest`, `delayFlow`, `delayEach`, `switchMap`, `scanReduce`, `publish`, `replay`, `cache` | 0/0 matched | _none_ | 25 | 26 | 252609.9 |
| 9 | `channels.BufferedChannel` | `channels.BufferedChannel` | 0.40 | 0 | 91/106 matched (target 164) | `sendImpl`, `receiveOnNoWaiterSuspend`, `receiveCatchingOnNoWaiterSuspend`, `receiveImpl`, `onClosedHasNext`, `hasNextOnNoWaiterSuspend`, `onClosedHasNextNoWaiterSuspend`, `invokeCloseHandler`, `updateSendersCounterIfLower`, `updateReceiversCounterIfLower`, `toStringDebug`, `checkSegmentStructureInvariants`, `onCancellationChannelResultImplDoNotCall`, `onCancellationImplDoNotCall`, `createSegmentFunction` | 0/0 matched | _none_ | 15 | 106 | 160606.0 |
| 10 | `JobSupport` | `JobImpl` | 0.27 | 0 | 60/75 matched (target 132) | `loopOnState`, `notifyHandlers`, `toCancellationException`, `invokeOnCompletionInternal`, `registerSelectForOnJoin`, `cancelInternal`, `cancelImpl`, `cancelMakeCompleting`, `toString`, `allocateList`, `getContinuationCancellationCause`, `onAwaitInternalRegFunc`, `onAwaitInternalProcessResFunc`, `handlesException`, `getString` | 0/0 matched | _none_ | 15 | 75 | 157507.3 |
| 11 | `CancellableContinuationImpl` | `CancellableContinuationImpl` | 0.35 | 0 | 34/43 matched (target 113) | `getStackTraceElement`, `callCancelHandlerSafely`, `invokeOnCancellationInternal`, `multipleHandlersError`, `tryResumeImpl`, `alreadyResumedError`, `getExceptionalResult`, `nameString`, `invokeHandlers` | 0/0 matched | _none_ | 9 | 43 | 94306.5 |
| 12 | `operators.Transform` | `flow.Transform` | 0.05 | 0 | 4/12 matched (target 6) | `filterNot`, `filterIsInstance`, `withIndex`, `onEach`, `scan`, `runningFold`, `runningReduce`, `chunked` | 0/0 matched | _none_ | 8 | 12 | 81209.5 |
| 13 | `Job` | `Job` | 0.16 | 0 | 7/14 matched (target 35) | `plus`, `Job`, `Job0`, `disposeOnCompletion`, `cancelAndJoin`, `orCancellation`, `invoke` | 0/0 matched | _none_ | 7 | 14 | 71408.4 |
| 14 | `operators.Lint` | `flow.Lint` | 0.07 | 0 | 4/11 matched (target 4) | `cancel`, `catch`, `retry`, `retryWhen`, `toList`, `toSet`, `count` | 0/0 matched | _none_ | 7 | 11 | 71109.3 |
| 15 | `internal.ChannelFlow` | `internal.ChannelFlow` | 0.26 | 0 | 10/16 matched (target 32) | `collectWithContextUndispatched`, `withUndispatchedContextCollector`, `emit`, `withContextUndispatched`, `resumeWith`, `getStackTraceElement` | 0/0 matched | _none_ | 6 | 16 | 61607.4 |
| 16 | `Builders.common` | `Builders.common` | 0.17 | 0 | 7/13 matched (target 44) | `invoke`, `trySuspend`, `tryResume`, `afterCompletion`, `afterResume`, `getResult` | 0/0 matched | _none_ | 6 | 13 | 61308.3 |
| 17 | `intrinsics.Undispatched` | `intrinsics.Undispatched [STUB]` | 0.00 | 0 | 0/6 matched (target 0) | `startCoroutineUndispatched`, `startUndispatchedOrReturn`, `startUndispatchedOrReturnIgnoreTimeout`, `startUndspatched`, `notOwnTimeout`, `dispatchExceptionAndMakeCompleting` | 0/0 matched | _none_ | 6 | 6 | 60610.0 |
| 18 | `selects.Select` | `selects.Select` | 0.38 | 0 | 21/26 matched (target 73) | `onTimeout`, `register`, `processResultAndInvokeBlockRecoveringException`, `tryResume`, `TrySelectDetailedResult` | 0/0 matched | _none_ | 5 | 26 | 52606.2 |
| 19 | `channels.Channel` | `channels.Channel` | 0.29 | 0 | 14/19 matched (target 78) | `receiveOrNull`, `equals`, `hashCode`, `next0`, `Channel` | 0/0 matched | _none_ | 5 | 19 | 51907.1 |
| 20 | `sync.Mutex` | `sync.Mutex` | 0.39 | 0 | 11/16 matched (target 44) | `Mutex`, `tryResume`, `resume`, `trySelect`, `selectInRegistrationPhase` | 0/0 matched | _none_ | 5 | 16 | 51606.1 |
| 21 | `channels.Broadcast` | `channels.Broadcast` | 0.13 | 0 | 3/8 matched (target 11) | `broadcast`, `cancel`, `cancelInternal`, `openSubscription`, `onStart` | 0/0 matched | _none_ | 5 | 8 | 50808.7 |
| 22 | `terminal.Collect` | `flow.Collect` | 0.03 | 0 | 1/6 matched (target 2) | `collect`, `launchIn`, `collectIndexed`, `collectLatest`, `emitAll` | 0/0 matched | _none_ | 5 | 6 | 50609.7 |
| 23 | `operators.Emitters` | `flow.Emitters` | 0.16 | 0 | 4/8 matched (target 5) | `unsafeTransform`, `ensureActive`, `emit`, `invokeSafely` | 0/0 matched | _none_ | 4 | 8 | 40808.4 |
| 24 | `CancellableContinuation` | `CancellableContinuation` | 0.17 | 0 | 3/7 matched (target 37) | `suspendCancellableCoroutineReusable`, `getOrCreateCancellableContinuation`, `invoke`, `toString` | 0/0 matched | _none_ | 4 | 7 | 40708.3 |
| 25 | `Await` | `Await` | 0.12 | 0 | 2/6 matched (target 5) | `await`, `disposeAll`, `invoke`, `toString` | 0/0 matched | _none_ | 4 | 6 | 40608.8 |
| 26 | `internal.Scopes` | `internal.Scopes` | 0.05 | 0 | 1/5 matched (target 4) | `getStackTraceElement`, `afterCompletion`, `afterCompletionUndispatched`, `afterResume` | 0/0 matched | _none_ | 4 | 5 | 40509.5 |
| 27 | `flow.SharedFlow` | `flow.SharedFlow` | 0.31 | 0 | 28/31 matched (target 51) | `MutableSharedFlow`, `fuse`, `fuseSharedFlow` | 0/0 matched | _none_ | 3 | 31 | 33106.9 |
| 28 | `flow.Builders` | `flow.FlowBuilders` | 0.21 | 0 | 8/11 matched (target 17) | `create`, `collectTo`, `toString` | 0/0 matched | _none_ | 3 | 11 | 31107.9 |
| 29 | `CoroutineScope` | `CoroutineScope` | 0.15 | 0 | 3/6 matched (target 12) | `plus`, `coroutineScope`, `currentCoroutineContext` | 0/0 matched | _none_ | 3 | 6 | 30608.5 |
| 30 | `operators.Errors` | `flow.Errors` | 0.15 | 0 | 3/6 matched (target 3) | `catchImpl`, `isCancellationCause`, `isSameExceptionAs` | 0/0 matched | _none_ | 3 | 6 | 30608.5 |
| 31 | `operators.Zip` | `flow.Zip` | 0.20 | 0 | 3/6 matched (target 7) | `combineUnsafe`, `combineTransformUnsafe`, `nullArrayFactory` | 0/0 matched | _none_ | 3 | 6 | 30608.0 |
| 32 | `internal.SafeCollector.common` | `internal.SafeCollector.common` | 0.05 | 0 | 1/4 matched (target 5) | `transitiveCoroutineParent`, `unsafeFlow`, `collect` | 0/0 matched | _none_ | 3 | 4 | 30409.5 |
| 33 | `flow.StateFlow` | `flow.StateFlow` | 0.32 | 0 | 17/19 matched (target 45) | `MutableStateFlow`, `fuse` | 0/0 matched | _none_ | 2 | 19 | 21906.8 |
| 34 | `internal.LockFreeTaskQueue` | `internal.LockFreeTaskQueue` | 0.29 | 0 | 14/16 matched (target 28) | `wo`, `withState` | 0/0 matched | _none_ | 2 | 16 | 21607.1 |
| 35 | `channels.Channels.common` | `channels.Channels.common` | 0.21 | 0 | 4/6 matched (target 13) | `receiveOrNull`, `onReceiveOrNull` | 0/0 matched | _none_ | 2 | 6 | 20607.9 |
| 36 | `CompletionState` | `CompletionState` | 0.26 | 0 | 3/5 matched (target 6) | `toString`, `makeResumed` | 0/0 matched | _none_ | 2 | 5 | 20507.4 |
| 37 | `flow.SharingStarted` | `flow.SharingStarted` | 0.29 | 0 | 3/5 matched (target 12) | `equals`, `hashCode` | 0/0 matched | _none_ | 2 | 5 | 20507.1 |
| 38 | `internal.NamedDispatcher` | `internal.NamedDispatcher` | 0.26 | 0 | 2/4 matched | `isDispatchNeeded`, `dispatchYield` | 0/0 matched | _none_ | 2 | 4 | 20407.4 |
| 39 | `CoroutineExceptionHandler` | `CoroutineExceptionHandler` | 0.32 | 0 | 2/4 matched (target 3) | `handlerException`, `CoroutineExceptionHandler` | 0/0 matched | _none_ | 2 | 4 | 20406.8 |
| 40 | `sync.Semaphore` | `sync.Semaphore` | 0.40 | 0 | 18/19 matched (target 53) | `Semaphore` | 0/0 matched | _none_ | 1 | 19 | 11906.0 |
| 41 | `internal.DispatchedTask` | `internal.DispatchedTask` | 0.35 | 0 | 9/10 matched (target 18) | `runUnconfinedEventLoop` | 0/0 matched | _none_ | 1 | 10 | 11006.5 |
| 42 | `channels.BroadcastChannel` | `channels.BroadcastChannel` | 0.40 | 0 | 8/9 matched (target 51) | `BroadcastChannel` | 0/0 matched | _none_ | 1 | 9 | 10906.0 |
| 43 | `operators.Limit` | `flow.Limit` | 0.34 | 0 | 7/8 matched (target 13) | `emitAbort` | 0/0 matched | _none_ | 1 | 8 | 10806.6 |
| 44 | `channels.ConflatedBufferedChannel` | `channels.ConflatedBufferedChannel` | 0.42 | 0 | 6/7 matched (target 9) | `registerSelectForSend` | 0/0 matched | _none_ | 1 | 7 | 10705.8 |
| 45 | `operators.Delay` | `flow.Delay` | 0.16 | 0 | 5/6 matched (target 8) | `timeoutInternal` | 0/0 matched | _none_ | 1 | 6 | 10608.4 |
| 46 | `operators.Context` | `flow.Context` | 0.23 | 0 | 5/6 matched | `checkFlowContext` | 0/0 matched | _none_ | 1 | 6 | 10607.7 |
| 47 | `CompletableDeferred` | `CompletableDeferred` | 0.34 | 0 | 5/6 matched (target 12) | `CompletableDeferred` | 0/0 matched | _none_ | 1 | 6 | 10606.6 |
| 48 | `internal.AbstractSharedFlow` | `internal.AbstractSharedFlow` | 0.19 | 0 | 3/4 matched (target 14) | `increment` | 0/0 matched | _none_ | 1 | 4 | 10408.1 |
| 49 | `Delay` | `Delay` | 0.32 | 0 | 3/4 matched (target 21) | `toDelayMillis` | 0/0 matched | _none_ | 1 | 4 | 10406.8 |
| 50 | `internal.Combine` | `internal.Combine` | 0.18 | 0 | 1/2 matched (target 3) | `combineInternal` | 0/0 matched | _none_ | 1 | 2 | 10208.2 |
| 51 | `internal.InlineList` | `internal.InlineList` | 0.24 | 0 | 1/2 matched (target 4) | `plus` | 0/0 matched | _none_ | 1 | 2 | 10207.6 |
| 52 | `selects.OnTimeout` | `selects.OnTimeout` | 0.35 | 0 | 1/2 matched (target 7) | `register` | 0/0 matched | _none_ | 1 | 2 | 10206.5 |
| 53 | `internal.DispatchedContinuation` | `internal.DispatchedContinuation` | 0.36 | 0 | 18/18 matched (target 31) | _none_ | 0/0 matched | _none_ | 0 | 18 | 1806.4 |
| 54 | `internal.ThreadSafeHeap` | `internal.ThreadSafeHeap` | 0.42 | 0 | 14/14 matched (target 20) | _none_ | 0/0 matched | _none_ | 0 | 14 | 1405.8 |
| 55 | `internal.ConcurrentLinkedList` | `internal.ConcurrentLinkedList` | 0.32 | 0 | 13/13 matched (target 61) | _none_ | 0/0 matched | _none_ | 0 | 13 | 1306.8 |
| 56 | `internal.LimitedDispatcher` | `internal.LimitedDispatcher` | 0.35 | 0 | 10/10 matched (target 14) | _none_ | 0/0 matched | _none_ | 0 | 10 | 1006.5 |
| 57 | `operators.Share` | `flow.Share` | 0.41 | 0 | 10/10 matched (target 25) | _none_ | 0/0 matched | _none_ | 0 | 10 | 1005.9 |
| 58 | `AbstractCoroutine` | `AbstractCoroutine` | 0.62 | 0 | 9/9 matched (target 14) | _none_ | 0/0 matched | _none_ | 0 | 9 | 903.8 |
| 59 | `terminal.Reduce` | `flow.Reduce` | 0.11 | 0 | 8/8 matched (target 26) | _none_ | 0/0 matched | _none_ | 0 | 8 | 808.9 |
| 60 | `operators.Merge` | `flow.Merge` | 0.35 | 0 | 8/8 matched (target 19) | _none_ | 0/0 matched | _none_ | 0 | 8 | 806.5 |
| 61 | `selects.SelectOld` | `selects.SelectOld` | 0.30 | 0 | 7/7 matched (target 12) | _none_ | 0/0 matched | _none_ | 0 | 7 | 707.0 |
| 62 | `Timeout` | `Timeout` | 0.44 | 0 | 7/7 matched (target 14) | _none_ | 0/0 matched | _none_ | 0 | 7 | 705.6 |
| 63 | `NonCancellable` | `NonCancellable` | 0.55 | 0 | 7/7 matched (target 36) | _none_ | 0/0 matched | _none_ | 0 | 7 | 704.5 |
| 64 | `CoroutineDispatcher` | `CoroutineDispatcher` | 0.59 | 0 | 7/7 matched (target 19) | _none_ | 0/0 matched | _none_ | 0 | 7 | 704.1 |
| 65 | `internal.Merge` | `internal.Merge` | 0.55 | 0 | 5/5 matched (target 33) | _none_ | 0/0 matched | _none_ | 0 | 5 | 504.5 |
| 66 | `channels.Produce` | `channels.Produce` | 0.34 | 0 | 4/4 matched (target 12) | _none_ | 0/0 matched | _none_ | 0 | 4 | 406.6 |
| 67 | `Unconfined` | `Unconfined` | 0.42 | 0 | 4/4 matched (target 8) | _none_ | 0/0 matched | _none_ | 0 | 4 | 405.8 |
| 68 | `selects.SelectUnbiased` | `selects.SelectUnbiased` | 0.42 | 0 | 4/4 matched (target 9) | _none_ | 0/0 matched | _none_ | 0 | 4 | 405.8 |
| 69 | `Supervisor` | `Supervisor` | 0.50 | 0 | 4/4 matched (target 11) | _none_ | 0/0 matched | _none_ | 0 | 4 | 405.0 |
| 70 | `terminal.Logic` | `flow.Logic` | 0.20 | 0 | 3/3 matched (target 5) | _none_ | 0/0 matched | _none_ | 0 | 3 | 308.0 |
| 71 | `operators.Distinct` | `flow.Distinct` | 0.24 | 0 | 3/3 matched (target 12) | _none_ | 0/0 matched | _none_ | 0 | 3 | 307.6 |
| 72 | `MainCoroutineDispatcher` | `MainCoroutineDispatcher` | 0.34 | 0 | 3/3 matched (target 4) | _none_ | 0/0 matched | _none_ | 0 | 3 | 306.6 |
| 73 | `terminal.Collection` | `flow.Collection` | 0.34 | 0 | 3/3 matched (target 8) | _none_ | 0/0 matched | _none_ | 0 | 3 | 306.6 |
| 74 | `internal.FlowCoroutine` | `internal.FlowCoroutine` | 0.45 | 0 | 3/3 matched (target 6) | _none_ | 0/0 matched | _none_ | 0 | 3 | 305.5 |
| 75 | `intrinsics.Cancellable` | `intrinsics.Cancellable` | 0.53 | 0 | 3/3 matched (target 15) | _none_ | 0/0 matched | _none_ | 0 | 3 | 304.7 |
| 76 | `channels.ChannelCoroutine` | `channels.ChannelCoroutine` | 0.20 | 0 | 2/2 matched (target 20) | _none_ | 0/0 matched | _none_ | 0 | 2 | 208.0 |
| 77 | `Guidance` | `Guidance` | 0.44 | 0 | 2/2 matched | _none_ | 0/0 matched | _none_ | 0 | 2 | 205.6 |
| 78 | `internal.FlowExceptions.common` | `internal.FlowExceptions` | 0.49 | 0 | 2/2 matched (target 4) | _none_ | 0/0 matched | _none_ | 0 | 2 | 205.1 |
| 79 | `internal.OnUndeliveredElement` | `internal.OnUndeliveredElement` | 0.58 | 0 | 2/2 matched (target 3) | _none_ | 0/0 matched | _none_ | 0 | 2 | 204.2 |
| 80 | `Yield` | `Yield` | 0.08 | 0 | 1/1 matched (target 7) | _none_ | 0/0 matched | _none_ | 0 | 1 | 109.2 |
| 81 | `terminal.Count` | `flow.Count` | 0.14 | 0 | 1/1 matched (target 6) | _none_ | 0/0 matched | _none_ | 0 | 1 | 108.6 |
| 82 | `CoroutineName` | `CoroutineName` | 0.25 | 0 | 1/1 matched (target 4) | _none_ | 0/0 matched | _none_ | 0 | 1 | 107.5 |
| 83 | `internal.MainDispatcherFactory` | `internal.MainDispatcherFactory` | 0.28 | 0 | 1/1 matched (target 3) | _none_ | 0/0 matched | _none_ | 0 | 1 | 107.2 |
| 84 | `internal.SystemProps.common` | `internal.SystemProps.common` | 0.30 | 0 | 1/1 matched (target 10) | _none_ | 0/0 matched | _none_ | 0 | 1 | 107.0 |
| 85 | `internal.SendingCollector` | `internal.SendingCollector` | 0.42 | 0 | 1/1 matched (target 2) | _none_ | 0/0 matched | _none_ | 0 | 1 | 105.8 |
| 86 | `internal.CoroutineExceptionHandlerImpl.common` | `internal.CoroutineExceptionHandlerImpl` | 0.48 | 0 | 1/1 matched (target 11) | _none_ | 0/0 matched | _none_ | 0 | 1 | 105.2 |
| 87 | `internal.NopCollector` | `internal.NopCollector` | 0.52 | 0 | 1/1 matched | _none_ | 0/0 matched | _none_ | 0 | 1 | 104.8 |
| 88 | `selects.WhileSelect` | `selects.WhileSelect` | 0.54 | 0 | 1/1 matched | _none_ | 0/0 matched | _none_ | 0 | 1 | 104.6 |
| 89 | `internal.Concurrent.common` | `internal.Concurrent.common` | 0.69 | 0 | 1/1 matched (target 20) | _none_ | 0/0 matched | _none_ | 0 | 1 | 103.1 |
| 90 | `internal.Synchronized.common` | `internal.SynchronizedObject` | 0.74 | 0 | 1/1 matched (target 5) | _none_ | 0/0 matched | _none_ | 0 | 1 | 102.6 |
| 91 | `internal.ThreadLocal.common` | `internal.ThreadLocal [ZERO]` | 0.00 | 0 | 0/0 matched (target 8) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 92 | `Debug.common` | `Debug.common [ZERO]` | 0.00 | 0 | 0/0 matched (target 4) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 93 | `CloseableCoroutineDispatcher` | `CloseableCoroutineDispatcher [ZERO]` | 0.00 | 0 | 0/0 matched (target 1) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 94 | `Exceptions.common` | `Exceptions [ZERO]` | 0.00 | 0 | 0/0 matched (target 13) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 95 | `Deferred` | `Deferred [ZERO]` | 0.00 | 0 | 0/0 matched (target 4) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 96 | `internal.LockFreeLinkedList.common` | `internal.LockFreeLinkedList.common [ZERO]` | 0.00 | 0 | 0/0 matched (target 14) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 97 | `Runnable.common` | `Runnable [STUB]` | 0.00 | 0 | 0/0 matched (target 6) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 98 | `internal.NullSurrogate` | `internal.NullSurrogate [ZERO]` | 0.00 | 0 | 0/0 matched (target 3) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 99 | `Waiter` | `Waiter [ZERO]` | 0.00 | 0 | 0/0 matched (target 2) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 100 | `Dispatchers.common` | `Dispatchers [ZERO]` | 0.00 | 0 | 0/0 matched (target 18) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 101 | `internal.LocalAtomics.common` | `internal.LocalAtomics.common [ZERO]` | 0.00 | 0 | 0/0 matched (target 4) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 102 | `internal.ProbesSupport.common` | `internal.ProbesSupport.common [ZERO]` | 0.00 | 0 | 0/0 matched (target 2) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 103 | `CompletableJob` | `CompletableJob [ZERO]` | 0.00 | 0 | 0/0 matched (target 2) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 104 | `flow.FlowCollector` | `flow.FlowCollector [ZERO]` | 0.00 | 0 | 0/0 matched (target 1) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 105 | `CoroutineContext.common` | `CoroutineContext.common [ZERO]` | 0.00 | 0 | 0/0 matched (target 19) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 106 | `internal.ThreadContext.common` | `internal.ThreadContext [ZERO]` | 0.00 | 0 | 0/0 matched (target 7) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 107 | `internal.StackTraceRecovery.common` | `internal.StackTraceRecovery [ZERO]` | 0.00 | 0 | 0/0 matched (target 9) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 108 | `SchedulerTask.common` | `SchedulerTask.common [STUB]` | 1.00 | 0 | 0/0 matched | _none_ | 0/0 matched | _none_ | 0 | 0 | 0.0 |
| 109 | `Annotations` | `Annotations` | 1.00 | 0 | 0/0 matched | _none_ | 0/0 matched | _none_ | 0 | 0 | 0.0 |
| 110 | `internal.InternalAnnotations.common` | `internal.InternalAnnotations.common` | 1.00 | 0 | 0/0 matched | _none_ | 0/0 matched | _none_ | 0 | 0 | 0.0 |
| 111 | `CompletionHandler.common` | `CompletionHandler [STUB]` | 1.00 | 0 | 0/0 matched | _none_ | 0/0 matched | _none_ | 0 | 0 | 0.0 |

## Cheat Detection / Scoring Failures

- `channels.Deprecated` -> `channels.Deprecated [STUB]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `EventLoop.common` -> `native.EventLoop [STUB]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `intrinsics.Undispatched` -> `intrinsics.Undispatched [STUB]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `internal.ThreadLocal.common` -> `internal.ThreadLocal [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Debug.common` -> `Debug.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `CloseableCoroutineDispatcher` -> `CloseableCoroutineDispatcher [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Exceptions.common` -> `Exceptions [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Deferred` -> `Deferred [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.LockFreeLinkedList.common` -> `internal.LockFreeLinkedList.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Runnable.common` -> `Runnable [STUB]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.NullSurrogate` -> `internal.NullSurrogate [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Waiter` -> `Waiter [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Dispatchers.common` -> `Dispatchers [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.LocalAtomics.common` -> `internal.LocalAtomics.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.ProbesSupport.common` -> `internal.ProbesSupport.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `CompletableJob` -> `CompletableJob [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `flow.FlowCollector` -> `flow.FlowCollector [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `CoroutineContext.common` -> `CoroutineContext.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.ThreadContext.common` -> `internal.ThreadContext [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.StackTraceRecovery.common` -> `internal.StackTraceRecovery [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only

## Critical Issues (Function Similarity < 0.60 with Dependencies)

These files need immediate attention:

- **flow.Channels** → `flow.Channels`
  - Function similarity: 0.37
  - Dependencies: 14
  - Functions: 12/12 matched (target 20)
  - Missing functions: _none_
  - Types: 0/0 matched
  - Missing types: _none_
  - Lint issues: 3

- **flow.Flow** → `flow.Flow`
  - Function similarity: 0.31
  - Dependencies: 13
  - Functions: 1/1 matched (target 4)
  - Missing functions: _none_
  - Types: 0/0 matched
  - Missing types: _none_
  - Lint issues: 2

- **internal.Symbol** → `internal.Symbol`
  - Function similarity: 0.53
  - Dependencies: 1
  - Functions: 2/2 matched (target 3)
  - Missing functions: _none_
  - Types: 0/0 matched
  - Missing types: _none_

## Missing Files (by Dependents)

No missing files detected.

