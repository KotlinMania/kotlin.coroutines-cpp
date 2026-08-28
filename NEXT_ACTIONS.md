# Immediate Actions - High-Value Files

Based on AST analysis, here are the concrete next steps.

## Summary

- **Files Present:** 111/111 (100.0%)
- **Function parity:** 633/900 matched (target 1875) — 70.3%
- **Class/type parity:** 0/0 matched — N/A
- **Combined symbol parity:** 633/900 matched (target 1875) — 70.3%
- **Average inline-code cosine:** 0.28 (function body across 104 matched files)
- **Average documentation cosine:** 0.53 (doc text across 104 matched files)
- **Cheat-zeroed Files:** 20
- **Critical Issues:** 105 files with <0.60 function similarity

## Priority 1: Fix Incomplete High-Dependency Files

### 1. flow.Channels
- **Similarity:** 0.37 (needs 48% improvement)
- **Dependencies:** 14
- **Priority Score:** 14001206.0
- **Functions:** 12/12 matched (target 20)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Action:** Deep review - likely missing major functionality

### 2. flow.Flow
- **Similarity:** 0.31 (needs 54% improvement)
- **Dependencies:** 13
- **Priority Score:** 13000107.0
- **Functions:** 1/1 matched (target 4)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Action:** Deep review - likely missing major functionality

## Priority 2: Port Missing High-Value Files

Critical missing files (>10 dependencies):

No missing high-value files detected.

## Detailed Work Items

Every matched file is listed below with function and type symbol parity.

### 1. flow.Channels

- **Target:** `flow.Channels`
- **Similarity:** 0.37
- **Dependents:** 14
- **Priority Score:** 14001206.0
- **Functions:** 12/12 matched (target 20)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 3

### 2. flow.Flow

- **Target:** `flow.Flow`
- **Similarity:** 0.31
- **Dependents:** 13
- **Priority Score:** 13000107.0
- **Functions:** 1/1 matched (target 4)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 3. channels.BufferOverflow

- **Target:** `channels.BufferOverflow [STUB]`
- **Similarity:** 1.00
- **Dependents:** 2
- **Priority Score:** 2000000.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 4. internal.Symbol

- **Target:** `internal.Symbol`
- **Similarity:** 0.53
- **Dependents:** 1
- **Priority Score:** 1000204.7
- **Functions:** 2/2 matched (target 3)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 5. CoroutineStart

- **Target:** `CoroutineStart`
- **Similarity:** 0.64
- **Dependents:** 1
- **Priority Score:** 1000103.6
- **Functions:** 1/1 matched (target 3)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 19

### 6. channels.Deprecated

- **Target:** `channels.Deprecated [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 444410.0
- **Functions:** 0/44 matched (target 0)
- **Missing functions:** `consume`, `consumeEach`, `consumesAll`, `elementAt`, `elementAtOrNull`, `first`, `firstOrNull`, `indexOf`, `last`, `lastIndexOf`, `lastOrNull`, `single`, `singleOrNull`, `drop`, `dropWhile`, `filter`, `filterIndexed`, `filterNot`, `filterNotNull`, `filterNotNullTo`, `take`, `takeWhile`, `toChannel`, `toCollection`, `toMap`, `toMutableList`, `toSet`, `flatMap`, `map`, `mapIndexed`, `mapIndexedNotNull`, `mapNotNull`, `withIndex`, `distinct`, `distinctBy`, `toMutableSet`, `any`, `count`, `maxWith`, `minWith`, `none`, `requireNoNulls`, `zip`, `consumes`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 7. EventLoop.common

- **Target:** `native.EventLoop [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 333310.0
- **Functions:** 0/33 matched (target 0)
- **Missing functions:** `processNextEvent`, `processUnconfinedEvent`, `shouldBeProcessedFromContext`, `dispatchUnconfined`, `delta`, `incrementUseCount`, `decrementUseCount`, `limitedParallelism`, `shutdown`, `currentOrNull`, `resetEventLoop`, `setEventLoop`, `delayToNanos`, `delayNanosToMillis`, `scheduleResumeAfterDelay`, `scheduleInvokeOnTimeout`, `dispatch`, `enqueue`, `enqueueImpl`, `dequeue`, `enqueueDelayedTasks`, `closeQueue`, `schedule`, `shouldUnpark`, `scheduleImpl`, `resetAll`, `rescheduleAllDelayed`, `compareTo`, `timeToExecute`, `scheduleTask`, `dispose`, `toString`, `run`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 8. flow.Migration

- **Target:** `flow.Migration`
- **Similarity:** 0.01
- **Dependents:** 0
- **Priority Score:** 252609.9
- **Functions:** 1/26 matched (target 1)
- **Missing functions:** `observeOn`, `publishOn`, `subscribeOn`, `onErrorResume`, `onErrorResumeNext`, `subscribe`, `flatMap`, `concatMap`, `merge`, `flatten`, `compose`, `skip`, `forEach`, `scanFold`, `onErrorReturn`, `startWith`, `concatWith`, `combineLatest`, `delayFlow`, `delayEach`, `switchMap`, `scanReduce`, `publish`, `replay`, `cache`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 8

### 9. channels.BufferedChannel

- **Target:** `channels.BufferedChannel`
- **Similarity:** 0.40
- **Dependents:** 0
- **Priority Score:** 160606.0
- **Functions:** 91/106 matched (target 164)
- **Missing functions:** `sendImpl`, `receiveOnNoWaiterSuspend`, `receiveCatchingOnNoWaiterSuspend`, `receiveImpl`, `onClosedHasNext`, `hasNextOnNoWaiterSuspend`, `onClosedHasNextNoWaiterSuspend`, `invokeCloseHandler`, `updateSendersCounterIfLower`, `updateReceiversCounterIfLower`, `toStringDebug`, `checkSegmentStructureInvariants`, `onCancellationChannelResultImplDoNotCall`, `onCancellationImplDoNotCall`, `createSegmentFunction`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 14

### 10. JobSupport

- **Target:** `JobImpl`
- **Similarity:** 0.27
- **Dependents:** 0
- **Priority Score:** 157507.3
- **Functions:** 60/75 matched (target 132)
- **Missing functions:** `loopOnState`, `notifyHandlers`, `toCancellationException`, `invokeOnCompletionInternal`, `registerSelectForOnJoin`, `cancelInternal`, `cancelImpl`, `cancelMakeCompleting`, `toString`, `allocateList`, `getContinuationCancellationCause`, `onAwaitInternalRegFunc`, `onAwaitInternalProcessResFunc`, `handlesException`, `getString`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 11. CancellableContinuationImpl

- **Target:** `CancellableContinuationImpl`
- **Similarity:** 0.35
- **Dependents:** 0
- **Priority Score:** 94306.5
- **Functions:** 34/43 matched (target 113)
- **Missing functions:** `getStackTraceElement`, `callCancelHandlerSafely`, `invokeOnCancellationInternal`, `multipleHandlersError`, `tryResumeImpl`, `alreadyResumedError`, `getExceptionalResult`, `nameString`, `invokeHandlers`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 24

### 12. operators.Transform

- **Target:** `flow.Transform`
- **Similarity:** 0.05
- **Dependents:** 0
- **Priority Score:** 81209.5
- **Functions:** 4/12 matched (target 6)
- **Missing functions:** `filterNot`, `filterIsInstance`, `withIndex`, `onEach`, `scan`, `runningFold`, `runningReduce`, `chunked`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 13. Job

- **Target:** `Job`
- **Similarity:** 0.16
- **Dependents:** 0
- **Priority Score:** 71408.4
- **Functions:** 7/14 matched (target 35)
- **Missing functions:** `plus`, `Job`, `Job0`, `disposeOnCompletion`, `cancelAndJoin`, `orCancellation`, `invoke`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 14. operators.Lint

- **Target:** `flow.Lint`
- **Similarity:** 0.07
- **Dependents:** 0
- **Priority Score:** 71109.3
- **Functions:** 4/11 matched (target 4)
- **Missing functions:** `cancel`, `catch`, `retry`, `retryWhen`, `toList`, `toSet`, `count`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 15. internal.ChannelFlow

- **Target:** `internal.ChannelFlow`
- **Similarity:** 0.26
- **Dependents:** 0
- **Priority Score:** 61607.4
- **Functions:** 10/16 matched (target 32)
- **Missing functions:** `collectWithContextUndispatched`, `withUndispatchedContextCollector`, `emit`, `withContextUndispatched`, `resumeWith`, `getStackTraceElement`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 16. Builders.common

- **Target:** `Builders.common`
- **Similarity:** 0.17
- **Dependents:** 0
- **Priority Score:** 61308.3
- **Functions:** 7/13 matched (target 44)
- **Missing functions:** `invoke`, `trySuspend`, `tryResume`, `afterCompletion`, `afterResume`, `getResult`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 8

### 17. intrinsics.Undispatched

- **Target:** `intrinsics.Undispatched [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 60610.0
- **Functions:** 0/6 matched (target 0)
- **Missing functions:** `startCoroutineUndispatched`, `startUndispatchedOrReturn`, `startUndispatchedOrReturnIgnoreTimeout`, `startUndspatched`, `notOwnTimeout`, `dispatchExceptionAndMakeCompleting`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 18. selects.Select

- **Target:** `selects.Select`
- **Similarity:** 0.38
- **Dependents:** 0
- **Priority Score:** 52606.2
- **Functions:** 21/26 matched (target 73)
- **Missing functions:** `onTimeout`, `register`, `processResultAndInvokeBlockRecoveringException`, `tryResume`, `TrySelectDetailedResult`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 5

### 19. channels.Channel

- **Target:** `channels.Channel`
- **Similarity:** 0.29
- **Dependents:** 0
- **Priority Score:** 51907.1
- **Functions:** 14/19 matched (target 78)
- **Missing functions:** `receiveOrNull`, `equals`, `hashCode`, `next0`, `Channel`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 16

### 20. sync.Mutex

- **Target:** `sync.Mutex`
- **Similarity:** 0.39
- **Dependents:** 0
- **Priority Score:** 51606.1
- **Functions:** 11/16 matched (target 44)
- **Missing functions:** `Mutex`, `tryResume`, `resume`, `trySelect`, `selectInRegistrationPhase`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 21. channels.Broadcast

- **Target:** `channels.Broadcast`
- **Similarity:** 0.13
- **Dependents:** 0
- **Priority Score:** 50808.7
- **Functions:** 3/8 matched (target 11)
- **Missing functions:** `broadcast`, `cancel`, `cancelInternal`, `openSubscription`, `onStart`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 5

### 22. terminal.Collect

- **Target:** `flow.Collect`
- **Similarity:** 0.03
- **Dependents:** 0
- **Priority Score:** 50609.7
- **Functions:** 1/6 matched (target 2)
- **Missing functions:** `collect`, `launchIn`, `collectIndexed`, `collectLatest`, `emitAll`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 23. operators.Emitters

- **Target:** `flow.Emitters`
- **Similarity:** 0.16
- **Dependents:** 0
- **Priority Score:** 40808.4
- **Functions:** 4/8 matched (target 5)
- **Missing functions:** `unsafeTransform`, `ensureActive`, `emit`, `invokeSafely`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 24. CancellableContinuation

- **Target:** `CancellableContinuation`
- **Similarity:** 0.17
- **Dependents:** 0
- **Priority Score:** 40708.3
- **Functions:** 3/7 matched (target 37)
- **Missing functions:** `suspendCancellableCoroutineReusable`, `getOrCreateCancellableContinuation`, `invoke`, `toString`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 5

### 25. Await

- **Target:** `Await`
- **Similarity:** 0.12
- **Dependents:** 0
- **Priority Score:** 40608.8
- **Functions:** 2/6 matched (target 5)
- **Missing functions:** `await`, `disposeAll`, `invoke`, `toString`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 26. internal.Scopes

- **Target:** `internal.Scopes`
- **Similarity:** 0.05
- **Dependents:** 0
- **Priority Score:** 40509.5
- **Functions:** 1/5 matched (target 4)
- **Missing functions:** `getStackTraceElement`, `afterCompletion`, `afterCompletionUndispatched`, `afterResume`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 27. flow.SharedFlow

- **Target:** `flow.SharedFlow`
- **Similarity:** 0.31
- **Dependents:** 0
- **Priority Score:** 33106.9
- **Functions:** 28/31 matched (target 51)
- **Missing functions:** `MutableSharedFlow`, `fuse`, `fuseSharedFlow`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 28. flow.Builders

- **Target:** `flow.FlowBuilders`
- **Similarity:** 0.21
- **Dependents:** 0
- **Priority Score:** 31107.9
- **Functions:** 8/11 matched (target 17)
- **Missing functions:** `create`, `collectTo`, `toString`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 4

### 29. CoroutineScope

- **Target:** `CoroutineScope`
- **Similarity:** 0.15
- **Dependents:** 0
- **Priority Score:** 30608.5
- **Functions:** 3/6 matched (target 12)
- **Missing functions:** `plus`, `coroutineScope`, `currentCoroutineContext`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 3

### 30. operators.Errors

- **Target:** `flow.Errors`
- **Similarity:** 0.15
- **Dependents:** 0
- **Priority Score:** 30608.5
- **Functions:** 3/6 matched (target 3)
- **Missing functions:** `catchImpl`, `isCancellationCause`, `isSameExceptionAs`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 8

### 31. operators.Zip

- **Target:** `flow.Zip`
- **Similarity:** 0.20
- **Dependents:** 0
- **Priority Score:** 30608.0
- **Functions:** 3/6 matched (target 7)
- **Missing functions:** `combineUnsafe`, `combineTransformUnsafe`, `nullArrayFactory`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 9

### 32. internal.SafeCollector.common

- **Target:** `internal.SafeCollector.common`
- **Similarity:** 0.05
- **Dependents:** 0
- **Priority Score:** 30409.5
- **Functions:** 1/4 matched (target 5)
- **Missing functions:** `transitiveCoroutineParent`, `unsafeFlow`, `collect`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 33. flow.StateFlow

- **Target:** `flow.StateFlow`
- **Similarity:** 0.32
- **Dependents:** 0
- **Priority Score:** 21906.8
- **Functions:** 17/19 matched (target 45)
- **Missing functions:** `MutableStateFlow`, `fuse`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 5

### 34. internal.LockFreeTaskQueue

- **Target:** `internal.LockFreeTaskQueue`
- **Similarity:** 0.29
- **Dependents:** 0
- **Priority Score:** 21607.1
- **Functions:** 14/16 matched (target 28)
- **Missing functions:** `wo`, `withState`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 35. channels.Channels.common

- **Target:** `channels.Channels.common`
- **Similarity:** 0.21
- **Dependents:** 0
- **Priority Score:** 20607.9
- **Functions:** 4/6 matched (target 13)
- **Missing functions:** `receiveOrNull`, `onReceiveOrNull`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 36. CompletionState

- **Target:** `CompletionState`
- **Similarity:** 0.26
- **Dependents:** 0
- **Priority Score:** 20507.4
- **Functions:** 3/5 matched (target 6)
- **Missing functions:** `toString`, `makeResumed`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 37. flow.SharingStarted

- **Target:** `flow.SharingStarted`
- **Similarity:** 0.29
- **Dependents:** 0
- **Priority Score:** 20507.1
- **Functions:** 3/5 matched (target 12)
- **Missing functions:** `equals`, `hashCode`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 38. internal.NamedDispatcher

- **Target:** `internal.NamedDispatcher`
- **Similarity:** 0.26
- **Dependents:** 0
- **Priority Score:** 20407.4
- **Functions:** 2/4 matched
- **Missing functions:** `isDispatchNeeded`, `dispatchYield`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 39. CoroutineExceptionHandler

- **Target:** `CoroutineExceptionHandler`
- **Similarity:** 0.32
- **Dependents:** 0
- **Priority Score:** 20406.8
- **Functions:** 2/4 matched (target 3)
- **Missing functions:** `handlerException`, `CoroutineExceptionHandler`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 40. sync.Semaphore

- **Target:** `sync.Semaphore`
- **Similarity:** 0.40
- **Dependents:** 0
- **Priority Score:** 11906.0
- **Functions:** 18/19 matched (target 53)
- **Missing functions:** `Semaphore`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 3

### 41. internal.DispatchedTask

- **Target:** `internal.DispatchedTask`
- **Similarity:** 0.35
- **Dependents:** 0
- **Priority Score:** 11006.5
- **Functions:** 9/10 matched (target 18)
- **Missing functions:** `runUnconfinedEventLoop`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 4

### 42. channels.BroadcastChannel

- **Target:** `channels.BroadcastChannel`
- **Similarity:** 0.40
- **Dependents:** 0
- **Priority Score:** 10906.0
- **Functions:** 8/9 matched (target 51)
- **Missing functions:** `BroadcastChannel`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 8

### 43. operators.Limit

- **Target:** `flow.Limit`
- **Similarity:** 0.34
- **Dependents:** 0
- **Priority Score:** 10806.6
- **Functions:** 7/8 matched (target 13)
- **Missing functions:** `emitAbort`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 44. channels.ConflatedBufferedChannel

- **Target:** `channels.ConflatedBufferedChannel`
- **Similarity:** 0.42
- **Dependents:** 0
- **Priority Score:** 10705.8
- **Functions:** 6/7 matched (target 9)
- **Missing functions:** `registerSelectForSend`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 45. operators.Delay

- **Target:** `flow.Delay`
- **Similarity:** 0.16
- **Dependents:** 0
- **Priority Score:** 10608.4
- **Functions:** 5/6 matched (target 8)
- **Missing functions:** `timeoutInternal`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 8

### 46. operators.Context

- **Target:** `flow.Context`
- **Similarity:** 0.23
- **Dependents:** 0
- **Priority Score:** 10607.7
- **Functions:** 5/6 matched
- **Missing functions:** `checkFlowContext`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 4

### 47. CompletableDeferred

- **Target:** `CompletableDeferred`
- **Similarity:** 0.34
- **Dependents:** 0
- **Priority Score:** 10606.6
- **Functions:** 5/6 matched (target 12)
- **Missing functions:** `CompletableDeferred`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 48. internal.AbstractSharedFlow

- **Target:** `internal.AbstractSharedFlow`
- **Similarity:** 0.19
- **Dependents:** 0
- **Priority Score:** 10408.1
- **Functions:** 3/4 matched (target 14)
- **Missing functions:** `increment`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 49. Delay

- **Target:** `Delay`
- **Similarity:** 0.32
- **Dependents:** 0
- **Priority Score:** 10406.8
- **Functions:** 3/4 matched (target 21)
- **Missing functions:** `toDelayMillis`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 50. internal.Combine

- **Target:** `internal.Combine`
- **Similarity:** 0.18
- **Dependents:** 0
- **Priority Score:** 10208.2
- **Functions:** 1/2 matched (target 3)
- **Missing functions:** `combineInternal`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 51. internal.InlineList

- **Target:** `internal.InlineList`
- **Similarity:** 0.24
- **Dependents:** 0
- **Priority Score:** 10207.6
- **Functions:** 1/2 matched (target 4)
- **Missing functions:** `plus`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 3

### 52. selects.OnTimeout

- **Target:** `selects.OnTimeout`
- **Similarity:** 0.35
- **Dependents:** 0
- **Priority Score:** 10206.5
- **Functions:** 1/2 matched (target 7)
- **Missing functions:** `register`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 6

### 53. internal.DispatchedContinuation

- **Target:** `internal.DispatchedContinuation`
- **Similarity:** 0.36
- **Dependents:** 0
- **Priority Score:** 1806.4
- **Functions:** 18/18 matched (target 31)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 54. internal.ThreadSafeHeap

- **Target:** `internal.ThreadSafeHeap`
- **Similarity:** 0.42
- **Dependents:** 0
- **Priority Score:** 1405.8
- **Functions:** 14/14 matched (target 20)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 55. internal.ConcurrentLinkedList

- **Target:** `internal.ConcurrentLinkedList`
- **Similarity:** 0.32
- **Dependents:** 0
- **Priority Score:** 1306.8
- **Functions:** 13/13 matched (target 61)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 3

### 56. internal.LimitedDispatcher

- **Target:** `internal.LimitedDispatcher`
- **Similarity:** 0.35
- **Dependents:** 0
- **Priority Score:** 1006.5
- **Functions:** 10/10 matched (target 14)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 15

### 57. operators.Share

- **Target:** `flow.Share`
- **Similarity:** 0.41
- **Dependents:** 0
- **Priority Score:** 1005.9
- **Functions:** 10/10 matched (target 25)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 3

### 58. AbstractCoroutine

- **Target:** `AbstractCoroutine`
- **Similarity:** 0.62
- **Dependents:** 0
- **Priority Score:** 903.8
- **Functions:** 9/9 matched (target 14)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 3

### 59. terminal.Reduce

- **Target:** `flow.Reduce`
- **Similarity:** 0.11
- **Dependents:** 0
- **Priority Score:** 808.9
- **Functions:** 8/8 matched (target 26)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 6

### 60. operators.Merge

- **Target:** `flow.Merge`
- **Similarity:** 0.35
- **Dependents:** 0
- **Priority Score:** 806.5
- **Functions:** 8/8 matched (target 19)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 61. selects.SelectOld

- **Target:** `selects.SelectOld`
- **Similarity:** 0.30
- **Dependents:** 0
- **Priority Score:** 707.0
- **Functions:** 7/7 matched (target 12)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 7

### 62. Timeout

- **Target:** `Timeout`
- **Similarity:** 0.44
- **Dependents:** 0
- **Priority Score:** 705.6
- **Functions:** 7/7 matched (target 14)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 3

### 63. NonCancellable

- **Target:** `NonCancellable`
- **Similarity:** 0.55
- **Dependents:** 0
- **Priority Score:** 704.5
- **Functions:** 7/7 matched (target 36)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 9

### 64. CoroutineDispatcher

- **Target:** `CoroutineDispatcher`
- **Similarity:** 0.59
- **Dependents:** 0
- **Priority Score:** 704.1
- **Functions:** 7/7 matched (target 19)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 3

### 65. internal.Merge

- **Target:** `internal.Merge`
- **Similarity:** 0.55
- **Dependents:** 0
- **Priority Score:** 504.5
- **Functions:** 5/5 matched (target 33)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 8

### 66. channels.Produce

- **Target:** `channels.Produce`
- **Similarity:** 0.34
- **Dependents:** 0
- **Priority Score:** 406.6
- **Functions:** 4/4 matched (target 12)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 67. Unconfined

- **Target:** `Unconfined`
- **Similarity:** 0.42
- **Dependents:** 0
- **Priority Score:** 405.8
- **Functions:** 4/4 matched (target 8)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 7

### 68. selects.SelectUnbiased

- **Target:** `selects.SelectUnbiased`
- **Similarity:** 0.42
- **Dependents:** 0
- **Priority Score:** 405.8
- **Functions:** 4/4 matched (target 9)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 69. Supervisor

- **Target:** `Supervisor`
- **Similarity:** 0.50
- **Dependents:** 0
- **Priority Score:** 405.0
- **Functions:** 4/4 matched (target 11)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 5

### 70. terminal.Logic

- **Target:** `flow.Logic`
- **Similarity:** 0.20
- **Dependents:** 0
- **Priority Score:** 308.0
- **Functions:** 3/3 matched (target 5)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 71. operators.Distinct

- **Target:** `flow.Distinct`
- **Similarity:** 0.24
- **Dependents:** 0
- **Priority Score:** 307.6
- **Functions:** 3/3 matched (target 12)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 72. MainCoroutineDispatcher

- **Target:** `MainCoroutineDispatcher`
- **Similarity:** 0.34
- **Dependents:** 0
- **Priority Score:** 306.6
- **Functions:** 3/3 matched (target 4)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 73. terminal.Collection

- **Target:** `flow.Collection`
- **Similarity:** 0.34
- **Dependents:** 0
- **Priority Score:** 306.6
- **Functions:** 3/3 matched (target 8)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 74. internal.FlowCoroutine

- **Target:** `internal.FlowCoroutine`
- **Similarity:** 0.45
- **Dependents:** 0
- **Priority Score:** 305.5
- **Functions:** 3/3 matched (target 6)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 75. intrinsics.Cancellable

- **Target:** `intrinsics.Cancellable`
- **Similarity:** 0.53
- **Dependents:** 0
- **Priority Score:** 304.7
- **Functions:** 3/3 matched (target 15)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 3

### 76. channels.ChannelCoroutine

- **Target:** `channels.ChannelCoroutine`
- **Similarity:** 0.20
- **Dependents:** 0
- **Priority Score:** 208.0
- **Functions:** 2/2 matched (target 20)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 3

### 77. Guidance

- **Target:** `Guidance`
- **Similarity:** 0.44
- **Dependents:** 0
- **Priority Score:** 205.6
- **Functions:** 2/2 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 78. internal.FlowExceptions.common

- **Target:** `internal.FlowExceptions`
- **Similarity:** 0.49
- **Dependents:** 0
- **Priority Score:** 205.1
- **Functions:** 2/2 matched (target 4)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 79. internal.OnUndeliveredElement

- **Target:** `internal.OnUndeliveredElement`
- **Similarity:** 0.58
- **Dependents:** 0
- **Priority Score:** 204.2
- **Functions:** 2/2 matched (target 3)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 80. Yield

- **Target:** `Yield`
- **Similarity:** 0.08
- **Dependents:** 0
- **Priority Score:** 109.2
- **Functions:** 1/1 matched (target 7)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 4

### 81. terminal.Count

- **Target:** `flow.Count`
- **Similarity:** 0.14
- **Dependents:** 0
- **Priority Score:** 108.6
- **Functions:** 1/1 matched (target 6)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 3

### 82. CoroutineName

- **Target:** `CoroutineName`
- **Similarity:** 0.25
- **Dependents:** 0
- **Priority Score:** 107.5
- **Functions:** 1/1 matched (target 4)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 83. internal.MainDispatcherFactory

- **Target:** `internal.MainDispatcherFactory`
- **Similarity:** 0.28
- **Dependents:** 0
- **Priority Score:** 107.2
- **Functions:** 1/1 matched (target 3)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 84. internal.SystemProps.common

- **Target:** `internal.SystemProps.common`
- **Similarity:** 0.30
- **Dependents:** 0
- **Priority Score:** 107.0
- **Functions:** 1/1 matched (target 10)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 85. internal.SendingCollector

- **Target:** `internal.SendingCollector`
- **Similarity:** 0.42
- **Dependents:** 0
- **Priority Score:** 105.8
- **Functions:** 1/1 matched (target 2)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 86. internal.CoroutineExceptionHandlerImpl.common

- **Target:** `internal.CoroutineExceptionHandlerImpl`
- **Similarity:** 0.48
- **Dependents:** 0
- **Priority Score:** 105.2
- **Functions:** 1/1 matched (target 11)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 87. internal.NopCollector

- **Target:** `internal.NopCollector`
- **Similarity:** 0.52
- **Dependents:** 0
- **Priority Score:** 104.8
- **Functions:** 1/1 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 88. selects.WhileSelect

- **Target:** `selects.WhileSelect`
- **Similarity:** 0.54
- **Dependents:** 0
- **Priority Score:** 104.6
- **Functions:** 1/1 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 89. internal.Concurrent.common

- **Target:** `internal.Concurrent.common`
- **Similarity:** 0.69
- **Dependents:** 0
- **Priority Score:** 103.1
- **Functions:** 1/1 matched (target 20)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 90. internal.Synchronized.common

- **Target:** `internal.SynchronizedObject`
- **Similarity:** 0.74
- **Dependents:** 0
- **Priority Score:** 102.6
- **Functions:** 1/1 matched (target 5)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 91. internal.ThreadLocal.common

- **Target:** `internal.ThreadLocal [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 8)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 92. Debug.common

- **Target:** `Debug.common [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 4)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 93. CloseableCoroutineDispatcher

- **Target:** `CloseableCoroutineDispatcher [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 1)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 94. Exceptions.common

- **Target:** `Exceptions [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 13)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 4

### 95. Deferred

- **Target:** `Deferred [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 4)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 96. internal.LockFreeLinkedList.common

- **Target:** `internal.LockFreeLinkedList.common [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 14)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 97. Runnable.common

- **Target:** `Runnable [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 6)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 98. internal.NullSurrogate

- **Target:** `internal.NullSurrogate [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 3)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 99. Waiter

- **Target:** `Waiter [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 2)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 100. Dispatchers.common

- **Target:** `Dispatchers [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 18)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 101. internal.LocalAtomics.common

- **Target:** `internal.LocalAtomics.common [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 4)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 102. internal.ProbesSupport.common

- **Target:** `internal.ProbesSupport.common [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 2)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 103. CompletableJob

- **Target:** `CompletableJob [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 2)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 104. flow.FlowCollector

- **Target:** `flow.FlowCollector [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 1)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 105. CoroutineContext.common

- **Target:** `CoroutineContext.common [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 19)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 5

### 106. internal.ThreadContext.common

- **Target:** `internal.ThreadContext [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 7)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 4

### 107. internal.StackTraceRecovery.common

- **Target:** `internal.StackTraceRecovery [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 9)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 7

### 108. SchedulerTask.common

- **Target:** `SchedulerTask.common [STUB]`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 0.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 109. Annotations

- **Target:** `Annotations`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 0.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 110. internal.InternalAnnotations.common

- **Target:** `internal.InternalAnnotations.common`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 0.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 111. CompletionHandler.common

- **Target:** `CompletionHandler [STUB]`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 0.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

## Success Criteria

For each file to be considered "complete":
- **Similarity ≥ 0.85** (Excellent threshold)
- All public APIs ported
- All tests ported
- Documentation ported
- port-lint header present

