# High Priority Ports - Action Plan

## Files by Impact

Priority = deps * 1,000,000 + SymDeficit * 10,000 + SrcSymbols * 100 + (1 - function similarity) * 10

Dependency fanout is ranked first so the ladder favors ports that clear downstream compilation failures fastest.

This list is complete and includes function/type detail for every matched file. Function similarity is the required body/parameter comparison; file-level shape does not rescue a port.

| Rank | Source | Target | Function similarity | Deps | Functions | Missing functions | Types | Missing types | SymDeficit | SrcSymbols | Priority |
|------|--------|--------|------------|------|-----------|-------------------|-------|---------------|-----------|------------|----------|
| 1 | `flow.Channels` | `flow.Channels [STUB]` | 0.00 | 14 | 12/12 matched (target 19) | _none_ | 0/0 matched | _none_ | 0 | 12 | 14001210.0 |
| 2 | `flow.Flow` | `flow.Flow` | 0.31 | 13 | 1/1 matched (target 4) | _none_ | 0/0 matched | _none_ | 0 | 1 | 13000107.0 |
| 3 | `channels.BufferOverflow` | `channels.BufferOverflow [STUB]` | 1.00 | 2 | 0/0 matched | _none_ | 0/0 matched | _none_ | 0 | 0 | 2000000.0 |
| 4 | `internal.Symbol` | `internal.Symbol` | 0.53 | 1 | 2/2 matched (target 3) | _none_ | 0/0 matched | _none_ | 0 | 2 | 1000204.7 |
| 5 | `CoroutineStart` | `CoroutineStart [STUB]` | 0.00 | 1 | 1/1 matched (target 3) | _none_ | 0/0 matched | _none_ | 0 | 1 | 1000110.0 |
| 6 | `channels.Deprecated` | `channels.Deprecated [STUB]` | 0.00 | 0 | 0/44 matched (target 0) | `consume`, `consumeEach`, `consumesAll`, `elementAt`, `elementAtOrNull`, `first`, `firstOrNull`, `indexOf`, `last`, `lastIndexOf`, `lastOrNull`, `single`, `singleOrNull`, `drop`, `dropWhile`, `filter`, `filterIndexed`, `filterNot`, `filterNotNull`, `filterNotNullTo`, `take`, `takeWhile`, `toChannel`, `toCollection`, `toMap`, `toMutableList`, `toSet`, `flatMap`, `map`, `mapIndexed`, `mapIndexedNotNull`, `mapNotNull`, `withIndex`, `distinct`, `distinctBy`, `toMutableSet`, `any`, `count`, `maxWith`, `minWith`, `none`, `requireNoNulls`, `zip`, `consumes` | 0/0 matched | _none_ | 44 | 44 | 444410.0 |
| 7 | `EventLoop.common` | `native.EventLoop [STUB]` | 0.00 | 0 | 0/33 matched (target 0) | `processNextEvent`, `processUnconfinedEvent`, `shouldBeProcessedFromContext`, `dispatchUnconfined`, `delta`, `incrementUseCount`, `decrementUseCount`, `limitedParallelism`, `shutdown`, `currentOrNull`, `resetEventLoop`, `setEventLoop`, `delayToNanos`, `delayNanosToMillis`, `scheduleResumeAfterDelay`, `scheduleInvokeOnTimeout`, `dispatch`, `enqueue`, `enqueueImpl`, `dequeue`, `enqueueDelayedTasks`, `closeQueue`, `schedule`, `shouldUnpark`, `scheduleImpl`, `resetAll`, `rescheduleAllDelayed`, `compareTo`, `timeToExecute`, `scheduleTask`, `dispose`, `toString`, `run` | 0/0 matched | _none_ | 33 | 33 | 333310.0 |
| 8 | `flow.Migration` | `flow.Migration` | 0.01 | 0 | 1/26 matched (target 1) | `observeOn`, `publishOn`, `subscribeOn`, `onErrorResume`, `onErrorResumeNext`, `subscribe`, `flatMap`, `concatMap`, `merge`, `flatten`, `compose`, `skip`, `forEach`, `scanFold`, `onErrorReturn`, `startWith`, `concatWith`, `combineLatest`, `delayFlow`, `delayEach`, `switchMap`, `scanReduce`, `publish`, `replay`, `cache` | 0/0 matched | _none_ | 25 | 26 | 252609.9 |
| 9 | `channels.BufferedChannel` | `channels.BufferedChannel [STUB]` | 0.00 | 0 | 91/106 matched (target 164) | `sendImpl`, `receiveOnNoWaiterSuspend`, `receiveCatchingOnNoWaiterSuspend`, `receiveImpl`, `onClosedHasNext`, `hasNextOnNoWaiterSuspend`, `onClosedHasNextNoWaiterSuspend`, `invokeCloseHandler`, `updateSendersCounterIfLower`, `updateReceiversCounterIfLower`, `toStringDebug`, `checkSegmentStructureInvariants`, `onCancellationChannelResultImplDoNotCall`, `onCancellationImplDoNotCall`, `createSegmentFunction` | 0/0 matched | _none_ | 15 | 106 | 160610.0 |
| 10 | `JobSupport` | `JobImpl [STUB]` | 0.00 | 0 | 60/75 matched (target 132) | `loopOnState`, `notifyHandlers`, `toCancellationException`, `invokeOnCompletionInternal`, `registerSelectForOnJoin`, `cancelInternal`, `cancelImpl`, `cancelMakeCompleting`, `toString`, `allocateList`, `getContinuationCancellationCause`, `onAwaitInternalRegFunc`, `onAwaitInternalProcessResFunc`, `handlesException`, `getString` | 0/0 matched | _none_ | 15 | 75 | 157510.0 |
| 11 | `CancellableContinuationImpl` | `CancellableContinuationImpl [STUB]` | 0.00 | 0 | 34/43 matched (target 113) | `getStackTraceElement`, `callCancelHandlerSafely`, `invokeOnCancellationInternal`, `multipleHandlersError`, `tryResumeImpl`, `alreadyResumedError`, `getExceptionalResult`, `nameString`, `invokeHandlers` | 0/0 matched | _none_ | 9 | 43 | 94310.0 |
| 12 | `operators.Transform` | `flow.Transform` | 0.05 | 0 | 4/12 matched (target 6) | `filterNot`, `filterIsInstance`, `withIndex`, `onEach`, `scan`, `runningFold`, `runningReduce`, `chunked` | 0/0 matched | _none_ | 8 | 12 | 81209.5 |
| 13 | `Job` | `Job` | 0.16 | 0 | 7/14 matched (target 35) | `plus`, `Job`, `Job0`, `disposeOnCompletion`, `cancelAndJoin`, `orCancellation`, `invoke` | 0/0 matched | _none_ | 7 | 14 | 71408.4 |
| 14 | `operators.Lint` | `flow.Lint` | 0.07 | 0 | 4/11 matched (target 4) | `cancel`, `catch`, `retry`, `retryWhen`, `toList`, `toSet`, `count` | 0/0 matched | _none_ | 7 | 11 | 71109.3 |
| 15 | `selects.SelectOld` | `selects.SelectOld [STUB]` | 0.00 | 0 | 0/7 matched (target 0) | `getResult`, `handleBuilderException`, `initSelectResult`, `selectOld`, `selectUnbiasedOld`, `resumeUndispatched`, `resumeUndispatchedWithException` | 0/0 matched | _none_ | 7 | 7 | 70710.0 |
| 16 | `internal.ChannelFlow` | `internal.ChannelFlow [STUB]` | 0.00 | 0 | 10/16 matched (target 31) | `collectWithContextUndispatched`, `withUndispatchedContextCollector`, `emit`, `withContextUndispatched`, `resumeWith`, `getStackTraceElement` | 0/0 matched | _none_ | 6 | 16 | 61610.0 |
| 17 | `Builders.common` | `Builders.common [STUB]` | 0.00 | 0 | 7/13 matched (target 44) | `invoke`, `trySuspend`, `tryResume`, `afterCompletion`, `afterResume`, `getResult` | 0/0 matched | _none_ | 6 | 13 | 61310.0 |
| 18 | `intrinsics.Undispatched` | `intrinsics.Undispatched [STUB]` | 0.00 | 0 | 0/6 matched (target 0) | `startCoroutineUndispatched`, `startUndispatchedOrReturn`, `startUndispatchedOrReturnIgnoreTimeout`, `startUndspatched`, `notOwnTimeout`, `dispatchExceptionAndMakeCompleting` | 0/0 matched | _none_ | 6 | 6 | 60610.0 |
| 19 | `selects.Select` | `selects.Select [STUB]` | 0.00 | 0 | 21/26 matched (target 73) | `onTimeout`, `register`, `processResultAndInvokeBlockRecoveringException`, `tryResume`, `TrySelectDetailedResult` | 0/0 matched | _none_ | 5 | 26 | 52610.0 |
| 20 | `channels.Channel` | `channels.Channel [STUB]` | 0.00 | 0 | 14/19 matched (target 78) | `receiveOrNull`, `equals`, `hashCode`, `next0`, `Channel` | 0/0 matched | _none_ | 5 | 19 | 51910.0 |
| 21 | `sync.Mutex` | `sync.Mutex [STUB]` | 0.00 | 0 | 11/16 matched (target 42) | `Mutex`, `tryResume`, `resume`, `trySelect`, `selectInRegistrationPhase` | 0/0 matched | _none_ | 5 | 16 | 51610.0 |
| 22 | `operators.Share` | `flow.Share [STUB]` | 0.00 | 0 | 5/10 matched (target 7) | `configureSharing`, `launchSharing`, `launchSharingDeferred`, `fuse`, `collect` | 0/0 matched | _none_ | 5 | 10 | 51010.0 |
| 23 | `channels.Broadcast` | `channels.Broadcast` | 0.16 | 0 | 3/8 matched (target 11) | `broadcast`, `cancel`, `cancelInternal`, `openSubscription`, `onStart` | 0/0 matched | _none_ | 5 | 8 | 50808.4 |
| 24 | `terminal.Collect` | `flow.Collect` | 0.03 | 0 | 1/6 matched (target 2) | `collect`, `launchIn`, `collectIndexed`, `collectLatest`, `emitAll` | 0/0 matched | _none_ | 5 | 6 | 50609.7 |
| 25 | `Await` | `Await` | 0.09 | 0 | 1/6 matched (target 4) | `joinAll`, `await`, `disposeAll`, `invoke`, `toString` | 0/0 matched | _none_ | 5 | 6 | 50609.1 |
| 26 | `operators.Emitters` | `flow.Emitters [STUB]` | 0.00 | 0 | 4/8 matched (target 5) | `unsafeTransform`, `ensureActive`, `emit`, `invokeSafely` | 0/0 matched | _none_ | 4 | 8 | 40810.0 |
| 27 | `CancellableContinuation` | `CancellableContinuation` | 0.17 | 0 | 3/7 matched (target 37) | `suspendCancellableCoroutineReusable`, `getOrCreateCancellableContinuation`, `invoke`, `toString` | 0/0 matched | _none_ | 4 | 7 | 40708.3 |
| 28 | `internal.Scopes` | `internal.Scopes` | 0.05 | 0 | 1/5 matched (target 4) | `getStackTraceElement`, `afterCompletion`, `afterCompletionUndispatched`, `afterResume` | 0/0 matched | _none_ | 4 | 5 | 40509.5 |
| 29 | `internal.NamedDispatcher` | `internal.NamedDispatcher [STUB]` | 0.00 | 0 | 0/4 matched (target 0) | `isDispatchNeeded`, `dispatch`, `dispatchYield`, `toString` | 0/0 matched | _none_ | 4 | 4 | 40410.0 |
| 30 | `flow.SharedFlow` | `flow.SharedFlow` | 0.31 | 0 | 28/31 matched (target 51) | `MutableSharedFlow`, `fuse`, `fuseSharedFlow` | 0/0 matched | _none_ | 3 | 31 | 33106.9 |
| 31 | `flow.Builders` | `flow.FlowBuilders [STUB]` | 0.00 | 0 | 8/11 matched (target 17) | `create`, `collectTo`, `toString` | 0/0 matched | _none_ | 3 | 11 | 31110.0 |
| 32 | `operators.Errors` | `flow.Errors [STUB]` | 0.00 | 0 | 3/6 matched (target 3) | `catchImpl`, `isCancellationCause`, `isSameExceptionAs` | 0/0 matched | _none_ | 3 | 6 | 30610.0 |
| 33 | `operators.Zip` | `flow.Zip [STUB]` | 0.00 | 0 | 3/6 matched (target 7) | `combineUnsafe`, `combineTransformUnsafe`, `nullArrayFactory` | 0/0 matched | _none_ | 3 | 6 | 30610.0 |
| 34 | `CoroutineScope` | `CoroutineScope` | 0.15 | 0 | 3/6 matched (target 12) | `plus`, `coroutineScope`, `currentCoroutineContext` | 0/0 matched | _none_ | 3 | 6 | 30608.5 |
| 35 | `internal.SafeCollector.common` | `internal.SafeCollector.common [STUB]` | 0.00 | 0 | 1/4 matched (target 5) | `transitiveCoroutineParent`, `unsafeFlow`, `collect` | 0/0 matched | _none_ | 3 | 4 | 30410.0 |
| 36 | `flow.StateFlow` | `flow.StateFlow [STUB]` | 0.00 | 0 | 17/19 matched (target 45) | `MutableStateFlow`, `fuse` | 0/0 matched | _none_ | 2 | 19 | 21910.0 |
| 37 | `internal.LockFreeTaskQueue` | `internal.LockFreeTaskQueue [STUB]` | 0.00 | 0 | 14/16 matched (target 28) | `wo`, `withState` | 0/0 matched | _none_ | 2 | 16 | 21610.0 |
| 38 | `channels.Channels.common` | `channels.Channels.common` | 0.21 | 0 | 4/6 matched (target 13) | `receiveOrNull`, `onReceiveOrNull` | 0/0 matched | _none_ | 2 | 6 | 20607.9 |
| 39 | `flow.SharingStarted` | `flow.SharingStarted [STUB]` | 0.00 | 0 | 3/5 matched (target 12) | `equals`, `hashCode` | 0/0 matched | _none_ | 2 | 5 | 20510.0 |
| 40 | `CompletionState` | `CompletionState` | 0.26 | 0 | 3/5 matched (target 6) | `toString`, `makeResumed` | 0/0 matched | _none_ | 2 | 5 | 20507.4 |
| 41 | `CoroutineExceptionHandler` | `CoroutineExceptionHandler` | 0.32 | 0 | 2/4 matched (target 3) | `handlerException`, `CoroutineExceptionHandler` | 0/0 matched | _none_ | 2 | 4 | 20406.8 |
| 42 | `sync.Semaphore` | `sync.Semaphore [STUB]` | 0.00 | 0 | 18/19 matched (target 53) | `Semaphore` | 0/0 matched | _none_ | 1 | 19 | 11910.0 |
| 43 | `internal.DispatchedTask` | `internal.DispatchedTask [STUB]` | 0.00 | 0 | 9/10 matched (target 18) | `runUnconfinedEventLoop` | 0/0 matched | _none_ | 1 | 10 | 11010.0 |
| 44 | `channels.BroadcastChannel` | `channels.BroadcastChannel` | 0.40 | 0 | 8/9 matched (target 51) | `BroadcastChannel` | 0/0 matched | _none_ | 1 | 9 | 10906.0 |
| 45 | `operators.Limit` | `flow.Limit` | 0.34 | 0 | 7/8 matched (target 13) | `emitAbort` | 0/0 matched | _none_ | 1 | 8 | 10806.6 |
| 46 | `channels.ConflatedBufferedChannel` | `channels.ConflatedBufferedChannel [STUB]` | 0.00 | 0 | 6/7 matched (target 9) | `registerSelectForSend` | 0/0 matched | _none_ | 1 | 7 | 10710.0 |
| 47 | `operators.Context` | `flow.Context [STUB]` | 0.00 | 0 | 5/6 matched | `checkFlowContext` | 0/0 matched | _none_ | 1 | 6 | 10610.0 |
| 48 | `operators.Delay` | `flow.Delay [STUB]` | 0.00 | 0 | 5/6 matched (target 8) | `timeoutInternal` | 0/0 matched | _none_ | 1 | 6 | 10610.0 |
| 49 | `CompletableDeferred` | `CompletableDeferred` | 0.34 | 0 | 5/6 matched (target 12) | `CompletableDeferred` | 0/0 matched | _none_ | 1 | 6 | 10606.6 |
| 50 | `internal.AbstractSharedFlow` | `internal.AbstractSharedFlow` | 0.19 | 0 | 3/4 matched (target 14) | `increment` | 0/0 matched | _none_ | 1 | 4 | 10408.1 |
| 51 | `Delay` | `Delay` | 0.32 | 0 | 3/4 matched (target 21) | `toDelayMillis` | 0/0 matched | _none_ | 1 | 4 | 10406.8 |
| 52 | `internal.Combine` | `internal.Combine [STUB]` | 0.00 | 0 | 1/2 matched (target 3) | `combineInternal` | 0/0 matched | _none_ | 1 | 2 | 10210.0 |
| 53 | `internal.InlineList` | `internal.InlineList [STUB]` | 0.00 | 0 | 1/2 matched (target 3) | `plus` | 0/0 matched | _none_ | 1 | 2 | 10210.0 |
| 54 | `selects.OnTimeout` | `selects.OnTimeout` | 0.35 | 0 | 1/2 matched (target 7) | `register` | 0/0 matched | _none_ | 1 | 2 | 10206.5 |
| 55 | `internal.DispatchedContinuation` | `internal.DispatchedContinuation` | 0.36 | 0 | 18/18 matched (target 31) | _none_ | 0/0 matched | _none_ | 0 | 18 | 1806.4 |
| 56 | `internal.ThreadSafeHeap` | `internal.ThreadSafeHeap` | 0.42 | 0 | 14/14 matched (target 20) | _none_ | 0/0 matched | _none_ | 0 | 14 | 1405.8 |
| 57 | `internal.ConcurrentLinkedList` | `internal.ConcurrentLinkedList [STUB]` | 0.00 | 0 | 13/13 matched (target 60) | _none_ | 0/0 matched | _none_ | 0 | 13 | 1310.0 |
| 58 | `internal.LimitedDispatcher` | `internal.LimitedDispatcher` | 0.35 | 0 | 10/10 matched (target 14) | _none_ | 0/0 matched | _none_ | 0 | 10 | 1006.5 |
| 59 | `AbstractCoroutine` | `AbstractCoroutine [STUB]` | 0.00 | 0 | 9/9 matched (target 14) | _none_ | 0/0 matched | _none_ | 0 | 9 | 910.0 |
| 60 | `operators.Merge` | `flow.Merge [STUB]` | 0.00 | 0 | 8/8 matched (target 19) | _none_ | 0/0 matched | _none_ | 0 | 8 | 810.0 |
| 61 | `terminal.Reduce` | `flow.Reduce` | 0.11 | 0 | 8/8 matched (target 26) | _none_ | 0/0 matched | _none_ | 0 | 8 | 808.9 |
| 62 | `CoroutineDispatcher` | `CoroutineDispatcher [STUB]` | 0.00 | 0 | 7/7 matched (target 19) | _none_ | 0/0 matched | _none_ | 0 | 7 | 710.0 |
| 63 | `Timeout` | `Timeout` | 0.44 | 0 | 7/7 matched (target 14) | _none_ | 0/0 matched | _none_ | 0 | 7 | 705.6 |
| 64 | `NonCancellable` | `NonCancellable` | 0.55 | 0 | 7/7 matched (target 36) | _none_ | 0/0 matched | _none_ | 0 | 7 | 704.5 |
| 65 | `internal.Merge` | `internal.Merge [STUB]` | 0.00 | 0 | 5/5 matched (target 33) | _none_ | 0/0 matched | _none_ | 0 | 5 | 510.0 |
| 66 | `channels.Produce` | `channels.Produce [STUB]` | 0.00 | 0 | 4/4 matched (target 12) | _none_ | 0/0 matched | _none_ | 0 | 4 | 410.0 |
| 67 | `Unconfined` | `Unconfined` | 0.42 | 0 | 4/4 matched (target 8) | _none_ | 0/0 matched | _none_ | 0 | 4 | 405.8 |
| 68 | `selects.SelectUnbiased` | `selects.SelectUnbiased` | 0.42 | 0 | 4/4 matched (target 9) | _none_ | 0/0 matched | _none_ | 0 | 4 | 405.8 |
| 69 | `Supervisor` | `Supervisor` | 0.50 | 0 | 4/4 matched (target 11) | _none_ | 0/0 matched | _none_ | 0 | 4 | 405.0 |
| 70 | `internal.FlowCoroutine` | `internal.FlowCoroutine [STUB]` | 0.00 | 0 | 3/3 matched (target 6) | _none_ | 0/0 matched | _none_ | 0 | 3 | 310.0 |
| 71 | `terminal.Logic` | `flow.Logic` | 0.20 | 0 | 3/3 matched (target 5) | _none_ | 0/0 matched | _none_ | 0 | 3 | 308.0 |
| 72 | `operators.Distinct` | `flow.Distinct` | 0.24 | 0 | 3/3 matched (target 12) | _none_ | 0/0 matched | _none_ | 0 | 3 | 307.6 |
| 73 | `MainCoroutineDispatcher` | `MainCoroutineDispatcher` | 0.34 | 0 | 3/3 matched (target 4) | _none_ | 0/0 matched | _none_ | 0 | 3 | 306.6 |
| 74 | `terminal.Collection` | `flow.Collection` | 0.34 | 0 | 3/3 matched (target 8) | _none_ | 0/0 matched | _none_ | 0 | 3 | 306.6 |
| 75 | `intrinsics.Cancellable` | `intrinsics.Cancellable` | 0.53 | 0 | 3/3 matched (target 15) | _none_ | 0/0 matched | _none_ | 0 | 3 | 304.7 |
| 76 | `internal.OnUndeliveredElement` | `internal.OnUndeliveredElement [STUB]` | 0.00 | 0 | 2/2 matched (target 3) | _none_ | 0/0 matched | _none_ | 0 | 2 | 210.0 |
| 77 | `channels.ChannelCoroutine` | `channels.ChannelCoroutine` | 0.20 | 0 | 2/2 matched (target 20) | _none_ | 0/0 matched | _none_ | 0 | 2 | 208.0 |
| 78 | `internal.FlowExceptions.common` | `internal.FlowExceptions` | 0.49 | 0 | 2/2 matched (target 4) | _none_ | 0/0 matched | _none_ | 0 | 2 | 205.1 |
| 79 | `Guidance` | `Guidance` | 0.59 | 0 | 2/2 matched | _none_ | 0/0 matched | _none_ | 0 | 2 | 204.1 |
| 80 | `internal.Concurrent.common` | `internal.Concurrent.common [STUB]` | 0.00 | 0 | 1/1 matched (target 13) | _none_ | 0/0 matched | _none_ | 0 | 1 | 110.0 |
| 81 | `internal.CoroutineExceptionHandlerImpl.common` | `internal.CoroutineExceptionHandlerImpl [STUB]` | 0.00 | 0 | 1/1 matched (target 11) | _none_ | 0/0 matched | _none_ | 0 | 1 | 110.0 |
| 82 | `Yield` | `Yield` | 0.08 | 0 | 1/1 matched (target 7) | _none_ | 0/0 matched | _none_ | 0 | 1 | 109.2 |
| 83 | `terminal.Count` | `flow.Count` | 0.14 | 0 | 1/1 matched (target 6) | _none_ | 0/0 matched | _none_ | 0 | 1 | 108.6 |
| 84 | `CoroutineName` | `CoroutineName` | 0.25 | 0 | 1/1 matched (target 4) | _none_ | 0/0 matched | _none_ | 0 | 1 | 107.5 |
| 85 | `internal.MainDispatcherFactory` | `internal.MainDispatcherFactory` | 0.28 | 0 | 1/1 matched (target 3) | _none_ | 0/0 matched | _none_ | 0 | 1 | 107.2 |
| 86 | `internal.SystemProps.common` | `internal.SystemProps.common` | 0.30 | 0 | 1/1 matched (target 10) | _none_ | 0/0 matched | _none_ | 0 | 1 | 107.0 |
| 87 | `internal.SendingCollector` | `internal.SendingCollector` | 0.42 | 0 | 1/1 matched (target 2) | _none_ | 0/0 matched | _none_ | 0 | 1 | 105.8 |
| 88 | `internal.NopCollector` | `internal.NopCollector` | 0.52 | 0 | 1/1 matched | _none_ | 0/0 matched | _none_ | 0 | 1 | 104.8 |
| 89 | `selects.WhileSelect` | `selects.WhileSelect` | 0.54 | 0 | 1/1 matched | _none_ | 0/0 matched | _none_ | 0 | 1 | 104.6 |
| 90 | `internal.Synchronized.common` | `internal.SynchronizedObject` | 0.74 | 0 | 1/1 matched (target 5) | _none_ | 0/0 matched | _none_ | 0 | 1 | 102.6 |
| 91 | `internal.NullSurrogate` | `internal.NullSurrogate [ZERO]` | 0.00 | 0 | 0/0 matched (target 3) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 92 | `Exceptions.common` | `Exceptions [ZERO]` | 0.00 | 0 | 0/0 matched (target 13) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 93 | `CompletableJob` | `CompletableJob [ZERO]` | 0.00 | 0 | 0/0 matched (target 2) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 94 | `Runnable.common` | `native.Runnable [ZERO]` | 0.00 | 0 | 0/0 matched (target 4) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 95 | `CoroutineContext.common` | `CoroutineContext.common [ZERO]` | 0.00 | 0 | 0/0 matched (target 19) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 96 | `Debug.common` | `Debug.common [ZERO]` | 0.00 | 0 | 0/0 matched (target 4) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 97 | `Deferred` | `Deferred [ZERO]` | 0.00 | 0 | 0/0 matched (target 4) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 98 | `Waiter` | `Waiter [ZERO]` | 0.00 | 0 | 0/0 matched (target 2) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 99 | `Dispatchers.common` | `Dispatchers [ZERO]` | 0.00 | 0 | 0/0 matched (target 18) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 100 | `internal.LockFreeLinkedList.common` | `internal.LockFreeLinkedList.common [ZERO]` | 0.00 | 0 | 0/0 matched (target 14) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 101 | `internal.LocalAtomics.common` | `internal.LocalAtomics.common [ZERO]` | 0.00 | 0 | 0/0 matched (target 4) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 102 | `CloseableCoroutineDispatcher` | `CloseableCoroutineDispatcher [ZERO]` | 0.00 | 0 | 0/0 matched (target 1) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 103 | `flow.FlowCollector` | `flow.FlowCollector [ZERO]` | 0.00 | 0 | 0/0 matched (target 1) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 104 | `internal.ThreadContext.common` | `internal.ThreadContext [ZERO]` | 0.00 | 0 | 0/0 matched (target 6) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 105 | `internal.StackTraceRecovery.common` | `internal.StackTraceRecovery [ZERO]` | 0.00 | 0 | 0/0 matched (target 4) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 106 | `internal.ThreadLocal.common` | `internal.ThreadLocal [ZERO]` | 0.00 | 0 | 0/0 matched (target 3) | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 107 | `SchedulerTask.common` | `SchedulerTask.common [STUB]` | 1.00 | 0 | 0/0 matched | _none_ | 0/0 matched | _none_ | 0 | 0 | 0.0 |
| 108 | `Annotations` | `Annotations` | 1.00 | 0 | 0/0 matched | _none_ | 0/0 matched | _none_ | 0 | 0 | 0.0 |
| 109 | `internal.ProbesSupport.common` | `internal.ProbesSupport [STUB]` | 1.00 | 0 | 0/0 matched | _none_ | 0/0 matched | _none_ | 0 | 0 | 0.0 |
| 110 | `internal.InternalAnnotations.common` | `internal.InternalAnnotations.common [STUB]` | 1.00 | 0 | 0/0 matched | _none_ | 0/0 matched | _none_ | 0 | 0 | 0.0 |
| 111 | `CompletionHandler.common` | `CompletionHandler [STUB]` | 1.00 | 0 | 0/0 matched | _none_ | 0/0 matched | _none_ | 0 | 0 | 0.0 |

## Cheat Detection / Scoring Failures

- `flow.Channels` -> `flow.Channels [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `CoroutineStart` -> `CoroutineStart [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `channels.Deprecated` -> `channels.Deprecated [STUB]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `EventLoop.common` -> `native.EventLoop [STUB]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `channels.BufferedChannel` -> `channels.BufferedChannel [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `JobSupport` -> `JobImpl [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `CancellableContinuationImpl` -> `CancellableContinuationImpl [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `selects.SelectOld` -> `selects.SelectOld [STUB]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `internal.ChannelFlow` -> `internal.ChannelFlow [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `Builders.common` -> `Builders.common [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `intrinsics.Undispatched` -> `intrinsics.Undispatched [STUB]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `selects.Select` -> `selects.Select [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `channels.Channel` -> `channels.Channel [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `sync.Mutex` -> `sync.Mutex [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `operators.Share` -> `flow.Share [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `operators.Emitters` -> `flow.Emitters [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.NamedDispatcher` -> `internal.NamedDispatcher [STUB]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `flow.Builders` -> `flow.FlowBuilders [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `operators.Errors` -> `flow.Errors [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `operators.Zip` -> `flow.Zip [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.SafeCollector.common` -> `internal.SafeCollector.common [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `flow.StateFlow` -> `flow.StateFlow [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.LockFreeTaskQueue` -> `internal.LockFreeTaskQueue [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `flow.SharingStarted` -> `flow.SharingStarted [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `sync.Semaphore` -> `sync.Semaphore [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.DispatchedTask` -> `internal.DispatchedTask [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `channels.ConflatedBufferedChannel` -> `channels.ConflatedBufferedChannel [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `operators.Context` -> `flow.Context [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `operators.Delay` -> `flow.Delay [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.Combine` -> `internal.Combine [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.InlineList` -> `internal.InlineList [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.ConcurrentLinkedList` -> `internal.ConcurrentLinkedList [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `AbstractCoroutine` -> `AbstractCoroutine [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `operators.Merge` -> `flow.Merge [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `CoroutineDispatcher` -> `CoroutineDispatcher [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.Merge` -> `internal.Merge [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `channels.Produce` -> `channels.Produce [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.FlowCoroutine` -> `internal.FlowCoroutine [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.OnUndeliveredElement` -> `internal.OnUndeliveredElement [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.Concurrent.common` -> `internal.Concurrent.common [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.CoroutineExceptionHandlerImpl.common` -> `internal.CoroutineExceptionHandlerImpl [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.NullSurrogate` -> `internal.NullSurrogate [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Exceptions.common` -> `Exceptions [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `CompletableJob` -> `CompletableJob [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Runnable.common` -> `native.Runnable [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `CoroutineContext.common` -> `CoroutineContext.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Debug.common` -> `Debug.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Deferred` -> `Deferred [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Waiter` -> `Waiter [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Dispatchers.common` -> `Dispatchers [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.LockFreeLinkedList.common` -> `internal.LockFreeLinkedList.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.LocalAtomics.common` -> `internal.LocalAtomics.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `CloseableCoroutineDispatcher` -> `CloseableCoroutineDispatcher [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `flow.FlowCollector` -> `flow.FlowCollector [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.ThreadContext.common` -> `internal.ThreadContext [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.StackTraceRecovery.common` -> `internal.StackTraceRecovery [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.ThreadLocal.common` -> `internal.ThreadLocal [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only

## Critical Issues (Function Similarity < 0.60 with Dependencies)

These files need immediate attention:

- **flow.Channels** → `flow.Channels [STUB]`
  - Function similarity: 0.00
  - Dependencies: 14
  - Functions: 12/12 matched (target 19)
  - Missing functions: _none_
  - Types: 0/0 matched
  - Missing types: _none_
  - Scoring failure: target contains TODO/stub/placeholder markers in function bodies
  - TODOs: 1
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

- **CoroutineStart** → `CoroutineStart [STUB]`
  - Function similarity: 0.00
  - Dependencies: 1
  - Functions: 1/1 matched (target 3)
  - Missing functions: _none_
  - Types: 0/0 matched
  - Missing types: _none_
  - Scoring failure: target contains TODO/stub/placeholder markers in function bodies
  - TODOs: 4
  - Lint issues: 19

## Missing Files (by Dependents)

No missing files detected.

