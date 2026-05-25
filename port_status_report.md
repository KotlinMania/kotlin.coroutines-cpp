# Code Port - Progress Report

**Generated:** 2026-05-22
**Source:** tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src
**Target:** src/kotlinx/coroutines

## Executive Summary

| Metric | Count | Percentage |
|--------|-------|------------|
| Function parity | 618/900 matched (target 1812) | 68.7% |
| Class/type parity | 0/0 matched | N/A |
| Combined symbol parity | 618/900 matched (target 1812) | 68.7% |
| Average function body similarity | 0.24 | inline-code cosine |
| Average documentation similarity | 0.55 | doc text cosine |
| Missing source functions | 0 | 0% parity until ported |
| Missing source classes/types | 0 | 0% parity until ported |
| Missing source symbol files | 0 | 0 symbols |
| Cheat/scoring failures | 57 | forced to 0% |
| Total source files | 111 | 100% |
| Target units (paired) | 191 | - |
| Target files (total) | 280 | - |
| Porting progress | 111 | 100.0% (matched) |
| Missing files | 0 | 0.0% |

## Port Quality Analysis

**Average Function Similarity:** 0.24

Similarity in this report is the required function-by-function body/parameter score. Class/type parity and symbol deficits are reported beside it; whole-file shape is diagnostic only.

**Work Distribution:**
- Critical (<0.60): 109 files (98.2% of matched)
- Needs review (0.60-0.84): 1 files (0.9% of matched)

## Worst Function Scores First

Every matched file is listed from lowest function body/parameter similarity upward. Missing symbol names are not capped.

| Rank | Source | Target | Function similarity | Functions | Missing functions | Types | Missing types | Tests | Symbol deficit | Priority |
|------|--------|--------|---------------------|-----------|-------------------|-------|---------------|-------|----------------|----------|
| 1 | `flow.Channels` | `flow.Channels [STUB]` | 0.00 | 12/12 matched (target 19) | _none_ | 0/0 matched | _none_ | - | 0 | 14001210.0 |
| 2 | `CoroutineStart` | `CoroutineStart [STUB]` | 0.00 | 1/1 matched (target 3) | _none_ | 0/0 matched | _none_ | - | 0 | 1000110.0 |
| 3 | `channels.Deprecated` | `channels.Deprecated [STUB]` | 0.00 | 0/44 matched (target 0) | `consume`, `consumeEach`, `consumesAll`, `elementAt`, `elementAtOrNull`, `first`, `firstOrNull`, `indexOf`, `last`, `lastIndexOf`, `lastOrNull`, `single`, `singleOrNull`, `drop`, `dropWhile`, `filter`, `filterIndexed`, `filterNot`, `filterNotNull`, `filterNotNullTo`, `take`, `takeWhile`, `toChannel`, `toCollection`, `toMap`, `toMutableList`, `toSet`, `flatMap`, `map`, `mapIndexed`, `mapIndexedNotNull`, `mapNotNull`, `withIndex`, `distinct`, `distinctBy`, `toMutableSet`, `any`, `count`, `maxWith`, `minWith`, `none`, `requireNoNulls`, `zip`, `consumes` | 0/0 matched | _none_ | - | 44 | 444410.0 |
| 4 | `EventLoop.common` | `native.EventLoop [STUB]` | 0.00 | 0/33 matched (target 0) | `processNextEvent`, `processUnconfinedEvent`, `shouldBeProcessedFromContext`, `dispatchUnconfined`, `delta`, `incrementUseCount`, `decrementUseCount`, `limitedParallelism`, `shutdown`, `currentOrNull`, `resetEventLoop`, `setEventLoop`, `delayToNanos`, `delayNanosToMillis`, `scheduleResumeAfterDelay`, `scheduleInvokeOnTimeout`, `dispatch`, `enqueue`, `enqueueImpl`, `dequeue`, `enqueueDelayedTasks`, `closeQueue`, `schedule`, `shouldUnpark`, `scheduleImpl`, `resetAll`, `rescheduleAllDelayed`, `compareTo`, `timeToExecute`, `scheduleTask`, `dispose`, `toString`, `run` | 0/0 matched | _none_ | - | 33 | 333310.0 |
| 5 | `JobSupport` | `JobImpl [STUB]` | 0.00 | 60/75 matched (target 132) | `loopOnState`, `notifyHandlers`, `toCancellationException`, `invokeOnCompletionInternal`, `registerSelectForOnJoin`, `cancelInternal`, `cancelImpl`, `cancelMakeCompleting`, `toString`, `allocateList`, `getContinuationCancellationCause`, `onAwaitInternalRegFunc`, `onAwaitInternalProcessResFunc`, `handlesException`, `getString` | 0/0 matched | _none_ | - | 15 | 157510.0 |
| 6 | `channels.BufferedChannel` | `channels.BufferedChannel [STUB]` | 0.00 | 91/106 matched (target 164) | `sendImpl`, `receiveOnNoWaiterSuspend`, `receiveCatchingOnNoWaiterSuspend`, `receiveImpl`, `onClosedHasNext`, `hasNextOnNoWaiterSuspend`, `onClosedHasNextNoWaiterSuspend`, `invokeCloseHandler`, `updateSendersCounterIfLower`, `updateReceiversCounterIfLower`, `toStringDebug`, `checkSegmentStructureInvariants`, `onCancellationChannelResultImplDoNotCall`, `onCancellationImplDoNotCall`, `createSegmentFunction` | 0/0 matched | _none_ | - | 15 | 160610.0 |
| 7 | `CancellableContinuationImpl` | `CancellableContinuationImpl [STUB]` | 0.00 | 34/43 matched (target 113) | `getStackTraceElement`, `callCancelHandlerSafely`, `invokeOnCancellationInternal`, `multipleHandlersError`, `tryResumeImpl`, `alreadyResumedError`, `getExceptionalResult`, `nameString`, `invokeHandlers` | 0/0 matched | _none_ | - | 9 | 94310.0 |
| 8 | `selects.SelectOld` | `selects.SelectOld [STUB]` | 0.00 | 0/7 matched (target 0) | `getResult`, `handleBuilderException`, `initSelectResult`, `selectOld`, `selectUnbiasedOld`, `resumeUndispatched`, `resumeUndispatchedWithException` | 0/0 matched | _none_ | - | 7 | 70710.0 |
| 9 | `Builders.common` | `Builders.common [STUB]` | 0.00 | 7/13 matched (target 44) | `invoke`, `trySuspend`, `tryResume`, `afterCompletion`, `afterResume`, `getResult` | 0/0 matched | _none_ | - | 6 | 61310.0 |
| 10 | `internal.ChannelFlow` | `internal.ChannelFlow [STUB]` | 0.00 | 10/16 matched (target 31) | `collectWithContextUndispatched`, `withUndispatchedContextCollector`, `emit`, `withContextUndispatched`, `resumeWith`, `getStackTraceElement` | 0/0 matched | _none_ | - | 6 | 61610.0 |
| 11 | `intrinsics.Undispatched` | `intrinsics.Undispatched [STUB]` | 0.00 | 0/6 matched (target 0) | `startCoroutineUndispatched`, `startUndispatchedOrReturn`, `startUndispatchedOrReturnIgnoreTimeout`, `startUndspatched`, `notOwnTimeout`, `dispatchExceptionAndMakeCompleting` | 0/0 matched | _none_ | - | 6 | 60610.0 |
| 12 | `channels.Channel` | `channels.Channel [STUB]` | 0.00 | 14/19 matched (target 78) | `receiveOrNull`, `equals`, `hashCode`, `next0`, `Channel` | 0/0 matched | _none_ | - | 5 | 51910.0 |
| 13 | `operators.Share` | `flow.Share [STUB]` | 0.00 | 5/10 matched (target 7) | `configureSharing`, `launchSharing`, `launchSharingDeferred`, `fuse`, `collect` | 0/0 matched | _none_ | - | 5 | 51010.0 |
| 14 | `selects.Select` | `selects.Select [STUB]` | 0.00 | 21/26 matched (target 73) | `onTimeout`, `register`, `processResultAndInvokeBlockRecoveringException`, `tryResume`, `TrySelectDetailedResult` | 0/0 matched | _none_ | - | 5 | 52610.0 |
| 15 | `sync.Mutex` | `sync.Mutex [STUB]` | 0.00 | 11/16 matched (target 42) | `Mutex`, `tryResume`, `resume`, `trySelect`, `selectInRegistrationPhase` | 0/0 matched | _none_ | - | 5 | 51610.0 |
| 16 | `internal.NamedDispatcher` | `internal.NamedDispatcher [STUB]` | 0.00 | 0/4 matched (target 0) | `isDispatchNeeded`, `dispatch`, `dispatchYield`, `toString` | 0/0 matched | _none_ | - | 4 | 40410.0 |
| 17 | `operators.Emitters` | `flow.Emitters [STUB]` | 0.00 | 4/8 matched (target 5) | `unsafeTransform`, `ensureActive`, `emit`, `invokeSafely` | 0/0 matched | _none_ | - | 4 | 40810.0 |
| 18 | `flow.Builders` | `flow.FlowBuilders [STUB]` | 0.00 | 8/11 matched (target 17) | `create`, `collectTo`, `toString` | 0/0 matched | _none_ | - | 3 | 31110.0 |
| 19 | `internal.SafeCollector.common` | `internal.SafeCollector.common [STUB]` | 0.00 | 1/4 matched (target 5) | `transitiveCoroutineParent`, `unsafeFlow`, `collect` | 0/0 matched | _none_ | - | 3 | 30410.0 |
| 20 | `operators.Errors` | `flow.Errors [STUB]` | 0.00 | 3/6 matched (target 3) | `catchImpl`, `isCancellationCause`, `isSameExceptionAs` | 0/0 matched | _none_ | - | 3 | 30610.0 |
| 21 | `operators.Zip` | `flow.Zip [STUB]` | 0.00 | 3/6 matched (target 7) | `combineUnsafe`, `combineTransformUnsafe`, `nullArrayFactory` | 0/0 matched | _none_ | - | 3 | 30610.0 |
| 22 | `flow.SharingStarted` | `flow.SharingStarted [STUB]` | 0.00 | 3/5 matched (target 12) | `equals`, `hashCode` | 0/0 matched | _none_ | - | 2 | 20510.0 |
| 23 | `flow.StateFlow` | `flow.StateFlow [STUB]` | 0.00 | 17/19 matched (target 45) | `MutableStateFlow`, `fuse` | 0/0 matched | _none_ | - | 2 | 21910.0 |
| 24 | `internal.LockFreeTaskQueue` | `internal.LockFreeTaskQueue [STUB]` | 0.00 | 14/16 matched (target 28) | `wo`, `withState` | 0/0 matched | _none_ | - | 2 | 21610.0 |
| 25 | `channels.ConflatedBufferedChannel` | `channels.ConflatedBufferedChannel [STUB]` | 0.00 | 6/7 matched (target 9) | `registerSelectForSend` | 0/0 matched | _none_ | - | 1 | 10710.0 |
| 26 | `internal.Combine` | `internal.Combine [STUB]` | 0.00 | 1/2 matched (target 3) | `combineInternal` | 0/0 matched | _none_ | - | 1 | 10210.0 |
| 27 | `internal.DispatchedTask` | `internal.DispatchedTask [STUB]` | 0.00 | 9/10 matched (target 18) | `runUnconfinedEventLoop` | 0/0 matched | _none_ | - | 1 | 11010.0 |
| 28 | `internal.InlineList` | `internal.InlineList [STUB]` | 0.00 | 1/2 matched (target 3) | `plus` | 0/0 matched | _none_ | - | 1 | 10210.0 |
| 29 | `operators.Context` | `flow.Context [STUB]` | 0.00 | 5/6 matched | `checkFlowContext` | 0/0 matched | _none_ | - | 1 | 10610.0 |
| 30 | `operators.Delay` | `flow.Delay [STUB]` | 0.00 | 5/6 matched (target 8) | `timeoutInternal` | 0/0 matched | _none_ | - | 1 | 10610.0 |
| 31 | `sync.Semaphore` | `sync.Semaphore [STUB]` | 0.00 | 18/19 matched (target 53) | `Semaphore` | 0/0 matched | _none_ | - | 1 | 11910.0 |
| 32 | `AbstractCoroutine` | `AbstractCoroutine [STUB]` | 0.00 | 9/9 matched (target 14) | _none_ | 0/0 matched | _none_ | - | 0 | 910.0 |
| 33 | `CloseableCoroutineDispatcher` | `CloseableCoroutineDispatcher [ZERO]` | 0.00 | 0/0 matched (target 1) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 34 | `CompletableJob` | `CompletableJob [ZERO]` | 0.00 | 0/0 matched (target 2) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 35 | `CoroutineContext.common` | `CoroutineContext.common [ZERO]` | 0.00 | 0/0 matched (target 19) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 36 | `CoroutineDispatcher` | `CoroutineDispatcher [STUB]` | 0.00 | 7/7 matched (target 19) | _none_ | 0/0 matched | _none_ | - | 0 | 710.0 |
| 37 | `Debug.common` | `Debug.common [ZERO]` | 0.00 | 0/0 matched (target 4) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 38 | `Deferred` | `Deferred [ZERO]` | 0.00 | 0/0 matched (target 4) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 39 | `Dispatchers.common` | `Dispatchers [ZERO]` | 0.00 | 0/0 matched (target 18) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 40 | `Exceptions.common` | `Exceptions [ZERO]` | 0.00 | 0/0 matched (target 13) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 41 | `Runnable.common` | `native.Runnable [ZERO]` | 0.00 | 0/0 matched (target 4) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 42 | `Waiter` | `Waiter [ZERO]` | 0.00 | 0/0 matched (target 2) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 43 | `channels.Produce` | `channels.Produce [STUB]` | 0.00 | 4/4 matched (target 12) | _none_ | 0/0 matched | _none_ | - | 0 | 410.0 |
| 44 | `flow.FlowCollector` | `flow.FlowCollector [ZERO]` | 0.00 | 0/0 matched (target 1) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 45 | `internal.Concurrent.common` | `internal.Concurrent.common [STUB]` | 0.00 | 1/1 matched (target 13) | _none_ | 0/0 matched | _none_ | - | 0 | 110.0 |
| 46 | `internal.ConcurrentLinkedList` | `internal.ConcurrentLinkedList [STUB]` | 0.00 | 13/13 matched (target 60) | _none_ | 0/0 matched | _none_ | - | 0 | 1310.0 |
| 47 | `internal.CoroutineExceptionHandlerImpl.common` | `internal.CoroutineExceptionHandlerImpl [STUB]` | 0.00 | 1/1 matched (target 11) | _none_ | 0/0 matched | _none_ | - | 0 | 110.0 |
| 48 | `internal.FlowCoroutine` | `internal.FlowCoroutine [STUB]` | 0.00 | 3/3 matched (target 6) | _none_ | 0/0 matched | _none_ | - | 0 | 310.0 |
| 49 | `internal.LocalAtomics.common` | `internal.LocalAtomics.common [ZERO]` | 0.00 | 0/0 matched (target 4) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 50 | `internal.LockFreeLinkedList.common` | `internal.LockFreeLinkedList.common [ZERO]` | 0.00 | 0/0 matched (target 14) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 51 | `internal.Merge` | `internal.Merge [STUB]` | 0.00 | 5/5 matched (target 33) | _none_ | 0/0 matched | _none_ | - | 0 | 510.0 |
| 52 | `internal.NullSurrogate` | `internal.NullSurrogate [ZERO]` | 0.00 | 0/0 matched (target 3) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 53 | `internal.OnUndeliveredElement` | `internal.OnUndeliveredElement [STUB]` | 0.00 | 2/2 matched (target 3) | _none_ | 0/0 matched | _none_ | - | 0 | 210.0 |
| 54 | `internal.StackTraceRecovery.common` | `internal.StackTraceRecovery [ZERO]` | 0.00 | 0/0 matched (target 4) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 55 | `internal.ThreadContext.common` | `internal.ThreadContext [ZERO]` | 0.00 | 0/0 matched (target 6) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 56 | `internal.ThreadLocal.common` | `internal.ThreadLocal [ZERO]` | 0.00 | 0/0 matched (target 3) | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 57 | `operators.Merge` | `flow.Merge [STUB]` | 0.00 | 8/8 matched (target 19) | _none_ | 0/0 matched | _none_ | - | 0 | 810.0 |
| 58 | `flow.Migration` | `flow.Migration` | 0.01 | 1/26 matched (target 1) | `observeOn`, `publishOn`, `subscribeOn`, `onErrorResume`, `onErrorResumeNext`, `subscribe`, `flatMap`, `concatMap`, `merge`, `flatten`, `compose`, `skip`, `forEach`, `scanFold`, `onErrorReturn`, `startWith`, `concatWith`, `combineLatest`, `delayFlow`, `delayEach`, `switchMap`, `scanReduce`, `publish`, `replay`, `cache` | 0/0 matched | _none_ | - | 25 | 252609.9 |
| 59 | `terminal.Collect` | `flow.Collect` | 0.03 | 1/6 matched (target 2) | `collect`, `launchIn`, `collectIndexed`, `collectLatest`, `emitAll` | 0/0 matched | _none_ | - | 5 | 50609.7 |
| 60 | `internal.Scopes` | `internal.Scopes` | 0.05 | 1/5 matched (target 4) | `getStackTraceElement`, `afterCompletion`, `afterCompletionUndispatched`, `afterResume` | 0/0 matched | _none_ | - | 4 | 40509.5 |
| 61 | `operators.Transform` | `flow.Transform` | 0.05 | 4/12 matched (target 6) | `filterNot`, `filterIsInstance`, `withIndex`, `onEach`, `scan`, `runningFold`, `runningReduce`, `chunked` | 0/0 matched | _none_ | - | 8 | 81209.5 |
| 62 | `operators.Lint` | `flow.Lint` | 0.07 | 4/11 matched (target 4) | `cancel`, `catch`, `retry`, `retryWhen`, `toList`, `toSet`, `count` | 0/0 matched | _none_ | - | 7 | 71109.3 |
| 63 | `Yield` | `Yield` | 0.08 | 1/1 matched (target 7) | _none_ | 0/0 matched | _none_ | - | 0 | 109.2 |
| 64 | `Await` | `Await` | 0.09 | 1/6 matched (target 4) | `joinAll`, `await`, `disposeAll`, `invoke`, `toString` | 0/0 matched | _none_ | - | 5 | 50609.1 |
| 65 | `terminal.Reduce` | `flow.Reduce` | 0.11 | 8/8 matched (target 26) | _none_ | 0/0 matched | _none_ | - | 0 | 808.9 |
| 66 | `terminal.Count` | `flow.Count` | 0.14 | 1/1 matched (target 6) | _none_ | 0/0 matched | _none_ | - | 0 | 108.6 |
| 67 | `CoroutineScope` | `CoroutineScope` | 0.15 | 3/6 matched (target 12) | `plus`, `coroutineScope`, `currentCoroutineContext` | 0/0 matched | _none_ | - | 3 | 30608.5 |
| 68 | `Job` | `Job` | 0.16 | 7/14 matched (target 35) | `plus`, `Job`, `Job0`, `disposeOnCompletion`, `cancelAndJoin`, `orCancellation`, `invoke` | 0/0 matched | _none_ | - | 7 | 71408.4 |
| 69 | `channels.Broadcast` | `channels.Broadcast` | 0.16 | 3/8 matched (target 11) | `broadcast`, `cancel`, `cancelInternal`, `openSubscription`, `onStart` | 0/0 matched | _none_ | - | 5 | 50808.4 |
| 70 | `CancellableContinuation` | `CancellableContinuation` | 0.17 | 3/7 matched (target 37) | `suspendCancellableCoroutineReusable`, `getOrCreateCancellableContinuation`, `invoke`, `toString` | 0/0 matched | _none_ | - | 4 | 40708.3 |
| 71 | `internal.AbstractSharedFlow` | `internal.AbstractSharedFlow` | 0.19 | 3/4 matched (target 14) | `increment` | 0/0 matched | _none_ | - | 1 | 10408.1 |
| 72 | `channels.ChannelCoroutine` | `channels.ChannelCoroutine` | 0.20 | 2/2 matched (target 20) | _none_ | 0/0 matched | _none_ | - | 0 | 208.0 |
| 73 | `terminal.Logic` | `flow.Logic` | 0.20 | 3/3 matched (target 5) | _none_ | 0/0 matched | _none_ | - | 0 | 308.0 |
| 74 | `channels.Channels.common` | `channels.Channels.common` | 0.21 | 4/6 matched (target 13) | `receiveOrNull`, `onReceiveOrNull` | 0/0 matched | _none_ | - | 2 | 20607.9 |
| 75 | `operators.Distinct` | `flow.Distinct` | 0.24 | 3/3 matched (target 12) | _none_ | 0/0 matched | _none_ | - | 0 | 307.6 |
| 76 | `CoroutineName` | `CoroutineName` | 0.25 | 1/1 matched (target 4) | _none_ | 0/0 matched | _none_ | - | 0 | 107.5 |
| 77 | `CompletionState` | `CompletionState` | 0.26 | 3/5 matched (target 6) | `toString`, `makeResumed` | 0/0 matched | _none_ | - | 2 | 20507.4 |
| 78 | `internal.MainDispatcherFactory` | `internal.MainDispatcherFactory` | 0.28 | 1/1 matched (target 3) | _none_ | 0/0 matched | _none_ | - | 0 | 107.2 |
| 79 | `internal.SystemProps.common` | `internal.SystemProps.common` | 0.30 | 1/1 matched (target 10) | _none_ | 0/0 matched | _none_ | - | 0 | 107.0 |
| 80 | `flow.Flow` | `flow.Flow` | 0.31 | 1/1 matched (target 4) | _none_ | 0/0 matched | _none_ | - | 0 | 13000107.0 |
| 81 | `flow.SharedFlow` | `flow.SharedFlow` | 0.31 | 28/31 matched (target 51) | `MutableSharedFlow`, `fuse`, `fuseSharedFlow` | 0/0 matched | _none_ | - | 3 | 33106.9 |
| 82 | `CoroutineExceptionHandler` | `CoroutineExceptionHandler` | 0.32 | 2/4 matched (target 3) | `handlerException`, `CoroutineExceptionHandler` | 0/0 matched | _none_ | - | 2 | 20406.8 |
| 83 | `Delay` | `Delay` | 0.32 | 3/4 matched (target 21) | `toDelayMillis` | 0/0 matched | _none_ | - | 1 | 10406.8 |
| 84 | `operators.Limit` | `flow.Limit` | 0.34 | 7/8 matched (target 13) | `emitAbort` | 0/0 matched | _none_ | - | 1 | 10806.6 |
| 85 | `MainCoroutineDispatcher` | `MainCoroutineDispatcher` | 0.34 | 3/3 matched (target 4) | _none_ | 0/0 matched | _none_ | - | 0 | 306.6 |
| 86 | `terminal.Collection` | `flow.Collection` | 0.34 | 3/3 matched (target 8) | _none_ | 0/0 matched | _none_ | - | 0 | 306.6 |
| 87 | `CompletableDeferred` | `CompletableDeferred` | 0.34 | 5/6 matched (target 12) | `CompletableDeferred` | 0/0 matched | _none_ | - | 1 | 10606.6 |
| 88 | `internal.LimitedDispatcher` | `internal.LimitedDispatcher` | 0.35 | 10/10 matched (target 14) | _none_ | 0/0 matched | _none_ | - | 0 | 1006.5 |
| 89 | `selects.OnTimeout` | `selects.OnTimeout` | 0.35 | 1/2 matched (target 7) | `register` | 0/0 matched | _none_ | - | 1 | 10206.5 |
| 90 | `internal.DispatchedContinuation` | `internal.DispatchedContinuation` | 0.36 | 18/18 matched (target 31) | _none_ | 0/0 matched | _none_ | - | 0 | 1806.4 |
| 91 | `channels.BroadcastChannel` | `channels.BroadcastChannel` | 0.40 | 8/9 matched (target 51) | `BroadcastChannel` | 0/0 matched | _none_ | - | 1 | 10906.0 |
| 92 | `Unconfined` | `Unconfined` | 0.42 | 4/4 matched (target 8) | _none_ | 0/0 matched | _none_ | - | 0 | 405.8 |
| 93 | `selects.SelectUnbiased` | `selects.SelectUnbiased` | 0.42 | 4/4 matched (target 9) | _none_ | 0/0 matched | _none_ | - | 0 | 405.8 |
| 94 | `internal.ThreadSafeHeap` | `internal.ThreadSafeHeap` | 0.42 | 14/14 matched (target 20) | _none_ | 0/0 matched | _none_ | - | 0 | 1405.8 |
| 95 | `internal.SendingCollector` | `internal.SendingCollector` | 0.42 | 1/1 matched (target 2) | _none_ | 0/0 matched | _none_ | - | 0 | 105.8 |
| 96 | `Timeout` | `Timeout` | 0.44 | 7/7 matched (target 14) | _none_ | 0/0 matched | _none_ | - | 0 | 705.6 |
| 97 | `internal.FlowExceptions.common` | `internal.FlowExceptions` | 0.49 | 2/2 matched (target 4) | _none_ | 0/0 matched | _none_ | - | 0 | 205.1 |
| 98 | `Supervisor` | `Supervisor` | 0.50 | 4/4 matched (target 11) | _none_ | 0/0 matched | _none_ | - | 0 | 405.0 |
| 99 | `internal.NopCollector` | `internal.NopCollector` | 0.52 | 1/1 matched | _none_ | 0/0 matched | _none_ | - | 0 | 104.8 |
| 100 | `intrinsics.Cancellable` | `intrinsics.Cancellable` | 0.53 | 3/3 matched (target 15) | _none_ | 0/0 matched | _none_ | - | 0 | 304.7 |
| 101 | `internal.Symbol` | `internal.Symbol` | 0.53 | 2/2 matched (target 3) | _none_ | 0/0 matched | _none_ | - | 0 | 1000204.7 |
| 102 | `selects.WhileSelect` | `selects.WhileSelect` | 0.54 | 1/1 matched | _none_ | 0/0 matched | _none_ | - | 0 | 104.6 |
| 103 | `NonCancellable` | `NonCancellable` | 0.55 | 7/7 matched (target 36) | _none_ | 0/0 matched | _none_ | - | 0 | 704.5 |
| 104 | `Guidance` | `Guidance` | 0.59 | 2/2 matched | _none_ | 0/0 matched | _none_ | - | 0 | 204.1 |
| 105 | `internal.Synchronized.common` | `internal.SynchronizedObject` | 0.74 | 1/1 matched (target 5) | _none_ | 0/0 matched | _none_ | - | 0 | 102.6 |
| 106 | `channels.BufferOverflow` | `channels.BufferOverflow [STUB]` | 1.00 | 0/0 matched | _none_ | 0/0 matched | _none_ | - | 0 | 2000000.0 |
| 107 | `Annotations` | `Annotations` | 1.00 | 0/0 matched | _none_ | 0/0 matched | _none_ | - | 0 | 0.0 |
| 108 | `CompletionHandler.common` | `CompletionHandler [STUB]` | 1.00 | 0/0 matched | _none_ | 0/0 matched | _none_ | - | 0 | 0.0 |
| 109 | `SchedulerTask.common` | `SchedulerTask.common [STUB]` | 1.00 | 0/0 matched | _none_ | 0/0 matched | _none_ | - | 0 | 0.0 |
| 110 | `internal.InternalAnnotations.common` | `internal.InternalAnnotations.common [STUB]` | 1.00 | 0/0 matched | _none_ | 0/0 matched | _none_ | - | 0 | 0.0 |
| 111 | `internal.ProbesSupport.common` | `internal.ProbesSupport [STUB]` | 1.00 | 0/0 matched | _none_ | 0/0 matched | _none_ | - | 0 | 0.0 |

## Cheat Detection / Scoring Failures

- `flow.Channels` -> `flow.Channels [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `CoroutineStart` -> `CoroutineStart [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `channels.Deprecated` -> `channels.Deprecated [STUB]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `EventLoop.common` -> `native.EventLoop [STUB]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `JobSupport` -> `JobImpl [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `channels.BufferedChannel` -> `channels.BufferedChannel [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `CancellableContinuationImpl` -> `CancellableContinuationImpl [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `selects.SelectOld` -> `selects.SelectOld [STUB]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `Builders.common` -> `Builders.common [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.ChannelFlow` -> `internal.ChannelFlow [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `intrinsics.Undispatched` -> `intrinsics.Undispatched [STUB]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `channels.Channel` -> `channels.Channel [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `operators.Share` -> `flow.Share [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `selects.Select` -> `selects.Select [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `sync.Mutex` -> `sync.Mutex [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.NamedDispatcher` -> `internal.NamedDispatcher [STUB]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `operators.Emitters` -> `flow.Emitters [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `flow.Builders` -> `flow.FlowBuilders [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.SafeCollector.common` -> `internal.SafeCollector.common [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `operators.Errors` -> `flow.Errors [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `operators.Zip` -> `flow.Zip [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `flow.SharingStarted` -> `flow.SharingStarted [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `flow.StateFlow` -> `flow.StateFlow [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.LockFreeTaskQueue` -> `internal.LockFreeTaskQueue [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `channels.ConflatedBufferedChannel` -> `channels.ConflatedBufferedChannel [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.Combine` -> `internal.Combine [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.DispatchedTask` -> `internal.DispatchedTask [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.InlineList` -> `internal.InlineList [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `operators.Context` -> `flow.Context [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `operators.Delay` -> `flow.Delay [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `sync.Semaphore` -> `sync.Semaphore [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `AbstractCoroutine` -> `AbstractCoroutine [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `CloseableCoroutineDispatcher` -> `CloseableCoroutineDispatcher [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `CompletableJob` -> `CompletableJob [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `CoroutineContext.common` -> `CoroutineContext.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `CoroutineDispatcher` -> `CoroutineDispatcher [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `Debug.common` -> `Debug.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Deferred` -> `Deferred [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Dispatchers.common` -> `Dispatchers [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Exceptions.common` -> `Exceptions [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Runnable.common` -> `native.Runnable [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Waiter` -> `Waiter [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `channels.Produce` -> `channels.Produce [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `flow.FlowCollector` -> `flow.FlowCollector [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.Concurrent.common` -> `internal.Concurrent.common [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.ConcurrentLinkedList` -> `internal.ConcurrentLinkedList [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.CoroutineExceptionHandlerImpl.common` -> `internal.CoroutineExceptionHandlerImpl [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.FlowCoroutine` -> `internal.FlowCoroutine [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.LocalAtomics.common` -> `internal.LocalAtomics.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.LockFreeLinkedList.common` -> `internal.LockFreeLinkedList.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.Merge` -> `internal.Merge [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.NullSurrogate` -> `internal.NullSurrogate [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.OnUndeliveredElement` -> `internal.OnUndeliveredElement [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies
- `internal.StackTraceRecovery.common` -> `internal.StackTraceRecovery [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.ThreadContext.common` -> `internal.ThreadContext [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.ThreadLocal.common` -> `internal.ThreadLocal [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `operators.Merge` -> `flow.Merge [STUB]`: function-by-function score forced to 0. target contains TODO/stub/placeholder markers in function bodies

### Critical Ports (Similarity < 0.60, Worst First)

These files need significant work:

- `flow.Channels` -> `flow.Channels [STUB]` (0.00, 14 deps)
- `CoroutineStart` -> `CoroutineStart [STUB]` (0.00, 1 deps)
- `channels.Deprecated` -> `channels.Deprecated [STUB]` (0.00)
- `EventLoop.common` -> `native.EventLoop [STUB]` (0.00)
- `JobSupport` -> `JobImpl [STUB]` (0.00)
- `channels.BufferedChannel` -> `channels.BufferedChannel [STUB]` (0.00)
- `CancellableContinuationImpl` -> `CancellableContinuationImpl [STUB]` (0.00)
- `selects.SelectOld` -> `selects.SelectOld [STUB]` (0.00)
- `Builders.common` -> `Builders.common [STUB]` (0.00)
- `internal.ChannelFlow` -> `internal.ChannelFlow [STUB]` (0.00)
- `intrinsics.Undispatched` -> `intrinsics.Undispatched [STUB]` (0.00)
- `channels.Channel` -> `channels.Channel [STUB]` (0.00)
- `operators.Share` -> `flow.Share [STUB]` (0.00)
- `selects.Select` -> `selects.Select [STUB]` (0.00)
- `sync.Mutex` -> `sync.Mutex [STUB]` (0.00)
- `internal.NamedDispatcher` -> `internal.NamedDispatcher [STUB]` (0.00)
- `operators.Emitters` -> `flow.Emitters [STUB]` (0.00)
- `flow.Builders` -> `flow.FlowBuilders [STUB]` (0.00)
- `internal.SafeCollector.common` -> `internal.SafeCollector.common [STUB]` (0.00)
- `operators.Errors` -> `flow.Errors [STUB]` (0.00)
- `operators.Zip` -> `flow.Zip [STUB]` (0.00)
- `flow.SharingStarted` -> `flow.SharingStarted [STUB]` (0.00)
- `flow.StateFlow` -> `flow.StateFlow [STUB]` (0.00)
- `internal.LockFreeTaskQueue` -> `internal.LockFreeTaskQueue [STUB]` (0.00)
- `channels.ConflatedBufferedChannel` -> `channels.ConflatedBufferedChannel [STUB]` (0.00)
- `internal.Combine` -> `internal.Combine [STUB]` (0.00)
- `internal.DispatchedTask` -> `internal.DispatchedTask [STUB]` (0.00)
- `internal.InlineList` -> `internal.InlineList [STUB]` (0.00)
- `operators.Context` -> `flow.Context [STUB]` (0.00)
- `operators.Delay` -> `flow.Delay [STUB]` (0.00)
- `sync.Semaphore` -> `sync.Semaphore [STUB]` (0.00)
- `AbstractCoroutine` -> `AbstractCoroutine [STUB]` (0.00)
- `CloseableCoroutineDispatcher` -> `CloseableCoroutineDispatcher [ZERO]` (0.00)
- `CompletableJob` -> `CompletableJob [ZERO]` (0.00)
- `CoroutineContext.common` -> `CoroutineContext.common [ZERO]` (0.00)
- `CoroutineDispatcher` -> `CoroutineDispatcher [STUB]` (0.00)
- `Debug.common` -> `Debug.common [ZERO]` (0.00)
- `Deferred` -> `Deferred [ZERO]` (0.00)
- `Dispatchers.common` -> `Dispatchers [ZERO]` (0.00)
- `Exceptions.common` -> `Exceptions [ZERO]` (0.00)
- `Runnable.common` -> `native.Runnable [ZERO]` (0.00)
- `Waiter` -> `Waiter [ZERO]` (0.00)
- `channels.Produce` -> `channels.Produce [STUB]` (0.00)
- `flow.FlowCollector` -> `flow.FlowCollector [ZERO]` (0.00)
- `internal.Concurrent.common` -> `internal.Concurrent.common [STUB]` (0.00)
- `internal.ConcurrentLinkedList` -> `internal.ConcurrentLinkedList [STUB]` (0.00)
- `internal.CoroutineExceptionHandlerImpl.common` -> `internal.CoroutineExceptionHandlerImpl [STUB]` (0.00)
- `internal.FlowCoroutine` -> `internal.FlowCoroutine [STUB]` (0.00)
- `internal.LocalAtomics.common` -> `internal.LocalAtomics.common [ZERO]` (0.00)
- `internal.LockFreeLinkedList.common` -> `internal.LockFreeLinkedList.common [ZERO]` (0.00)
- `internal.Merge` -> `internal.Merge [STUB]` (0.00)
- `internal.NullSurrogate` -> `internal.NullSurrogate [ZERO]` (0.00)
- `internal.OnUndeliveredElement` -> `internal.OnUndeliveredElement [STUB]` (0.00)
- `internal.StackTraceRecovery.common` -> `internal.StackTraceRecovery [ZERO]` (0.00)
- `internal.ThreadContext.common` -> `internal.ThreadContext [ZERO]` (0.00)
- `internal.ThreadLocal.common` -> `internal.ThreadLocal [ZERO]` (0.00)
- `operators.Merge` -> `flow.Merge [STUB]` (0.00)
- `flow.Migration` -> `flow.Migration` (0.01)
- `terminal.Collect` -> `flow.Collect` (0.03)
- `internal.Scopes` -> `internal.Scopes` (0.05)
- `operators.Transform` -> `flow.Transform` (0.05)
- `operators.Lint` -> `flow.Lint` (0.07)
- `Yield` -> `Yield` (0.08)
- `Await` -> `Await` (0.09)
- `terminal.Reduce` -> `flow.Reduce` (0.11)
- `terminal.Count` -> `flow.Count` (0.14)
- `CoroutineScope` -> `CoroutineScope` (0.15)
- `Job` -> `Job` (0.16)
- `channels.Broadcast` -> `channels.Broadcast` (0.16)
- `CancellableContinuation` -> `CancellableContinuation` (0.17)
- `internal.AbstractSharedFlow` -> `internal.AbstractSharedFlow` (0.19)
- `channels.ChannelCoroutine` -> `channels.ChannelCoroutine` (0.20)
- `terminal.Logic` -> `flow.Logic` (0.20)
- `channels.Channels.common` -> `channels.Channels.common` (0.21)
- `operators.Distinct` -> `flow.Distinct` (0.24)
- `CoroutineName` -> `CoroutineName` (0.25)
- `CompletionState` -> `CompletionState` (0.26)
- `internal.MainDispatcherFactory` -> `internal.MainDispatcherFactory` (0.28)
- `internal.SystemProps.common` -> `internal.SystemProps.common` (0.30)
- `flow.Flow` -> `flow.Flow` (0.31, 13 deps)
- `flow.SharedFlow` -> `flow.SharedFlow` (0.31)
- `CoroutineExceptionHandler` -> `CoroutineExceptionHandler` (0.32)
- `Delay` -> `Delay` (0.32)
- `operators.Limit` -> `flow.Limit` (0.34)
- `MainCoroutineDispatcher` -> `MainCoroutineDispatcher` (0.34)
- `terminal.Collection` -> `flow.Collection` (0.34)
- `CompletableDeferred` -> `CompletableDeferred` (0.34)
- `internal.LimitedDispatcher` -> `internal.LimitedDispatcher` (0.35)
- `selects.OnTimeout` -> `selects.OnTimeout` (0.35)
- `internal.DispatchedContinuation` -> `internal.DispatchedContinuation` (0.36)
- `channels.BroadcastChannel` -> `channels.BroadcastChannel` (0.40)
- `Unconfined` -> `Unconfined` (0.42)
- `selects.SelectUnbiased` -> `selects.SelectUnbiased` (0.42)
- `internal.ThreadSafeHeap` -> `internal.ThreadSafeHeap` (0.42)
- `internal.SendingCollector` -> `internal.SendingCollector` (0.42)
- `Timeout` -> `Timeout` (0.44)
- `internal.FlowExceptions.common` -> `internal.FlowExceptions` (0.49)
- `Supervisor` -> `Supervisor` (0.50)
- `internal.NopCollector` -> `internal.NopCollector` (0.52)
- `intrinsics.Cancellable` -> `intrinsics.Cancellable` (0.53)
- `internal.Symbol` -> `internal.Symbol` (0.53, 1 deps)
- `selects.WhileSelect` -> `selects.WhileSelect` (0.54)
- `NonCancellable` -> `NonCancellable` (0.55)
- `Guidance` -> `Guidance` (0.59)

## Incorrect Ports (Missing Types)

These files are matched (often via `// port-lint`) but appear to be missing one or more type declarations
present in the Rust source file.

| Source | Target | Missing types | Examples |
|--------|--------|---------------|----------|
| _None detected_ | | | |

## High Priority Missing Files

No missing files detected.

## Documentation Gaps

**Documentation coverage:** 9424 / 9370 lines (101%)

Documentation gaps (>20%), complete list:

- `operators.Delay` - 70% gap (236 → 71 lines)
- `CancellableContinuation` - 41% gap (377 → 223 lines)
- `JobSupport` - 51% gap (231 → 114 lines)
- `operators.Share` - 57% gap (204 → 87 lines)
- `flow.Migration` - 52% gap (216 → 103 lines)
- `selects.Select` - 28% gap (402 → 290 lines)
- `channels.Produce` - 47% gap (226 → 120 lines)
- `Job` - 22% gap (477 → 372 lines)
- `flow.StateFlow` - 51% gap (196 → 96 lines)
- `EventLoop.common` - 93% gap (99 → 7 lines)
- `flow.Builders` - 44% gap (201 → 113 lines)
- `CoroutineScope` - 26% gap (320 → 238 lines)
- `operators.Emitters` - 66% gap (105 → 36 lines)
- `operators.Merge` - 46% gap (142 → 77 lines)
- `flow.SharedFlow` - 24% gap (245 → 186 lines)
- `CoroutineDispatcher` - 28% gap (213 → 154 lines)
- `CoroutineExceptionHandler` - 88% gap (67 → 8 lines)
- `operators.Zip` - 42% gap (133 → 77 lines)
- `terminal.Collect` - 75% gap (65 → 16 lines)
- `channels.Deprecated` - 100% gap (48 → 0 lines)
- `operators.Lint` - 71% gap (55 → 16 lines)
- `operators.Transform` - 51% gap (74 → 36 lines)
- `Builders.common` - 47% gap (75 → 40 lines)
- `internal.DispatchedTask` - 41% gap (66 → 39 lines)
- `Deferred` - 29% gap (84 → 60 lines)
- `Dispatchers.common` - 31% gap (62 → 43 lines)
- `terminal.Logic` - 21% gap (77 → 61 lines)
- `internal.DispatchedContinuation` - 21% gap (66 → 52 lines)
- `intrinsics.Undispatched` - 40% gap (25 → 15 lines)
- `CloseableCoroutineDispatcher` - 53% gap (17 → 8 lines)
- `SchedulerTask.common` - 54% gap (13 → 6 lines)
- `channels.Broadcast` - 100% gap (6 → 0 lines)

