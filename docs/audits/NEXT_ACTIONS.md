# Immediate Actions - High-Value Files

Based on AST analysis, here are the concrete next steps.

## Summary

- **Files Present:** 96/111 (86.5%)
- **Function parity:** 633/1059 matched (target 1766) — 59.8%
- **Class/type parity:** 159/228 matched (target 317) — 69.7%
- **Combined symbol parity:** 792/1287 matched (target 2083) — 61.5%
- **Average inline-code cosine:** 0.23 (function body across 96 matched files)
- **Average documentation cosine:** 0.53 (doc text across 96 matched files)
- **Cheat-zeroed Files:** 10
- **Critical Issues:** 86 files with <0.60 function similarity
- **Needs Review:** 0 files with 0.60-0.84 function similarity
- **Excellent:** 10 files with >=0.85 function similarity

## Priority 1: Fix Incomplete High-Dependency Files

### 1. flow.Channels
- **Similarity:** 0.23 (needs 62% improvement)
- **Dependencies:** 14
- **Priority Score:** 14001308.0
- **Functions:** 12/12 matched (target 22)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_
- **Action:** Deep review - likely missing major functionality

### 2. flow.Flow
- **Similarity:** 0.00 (needs 85% improvement)
- **Dependencies:** 13
- **Priority Score:** 13010310.0
- **Functions:** 0/1 matched (target 0)
- **Missing functions:** `AbstractFlow::collect`
- **Types:** 2/2 matched (target 3)
- **Missing types:** _none_
- **Symbol Deficit:** 1 (functions: 1, types: 0)
- **Action:** Deep review - likely missing major functionality

## Priority 2: Port Missing High-Value Files

Critical missing files (>10 dependencies):

No missing high-value files detected.

## Detailed Work Items

Every matched file is listed below with function and type symbol parity.

### 1. flow.Channels

- **Target:** `flow.Channels`
- **Similarity:** 0.23
- **Dependents:** 14
- **Priority Score:** 14001308.0
- **Functions:** 12/12 matched (target 22)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 2. flow.Flow

- **Target:** `flow.Flow [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 13
- **Priority Score:** 13010310.0
- **Functions:** 0/1 matched (target 0)
- **Missing functions:** `AbstractFlow::collect`
- **Types:** 2/2 matched (target 3)
- **Missing types:** _none_
- **Lint issues:** 2

### 3. channels.BufferOverflow

- **Target:** `channels.BufferOverflow`
- **Similarity:** 1.00
- **Dependents:** 2
- **Priority Score:** 2000100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 4. CoroutineStart

- **Target:** `CoroutineStart`
- **Similarity:** 0.00
- **Dependents:** 1
- **Priority Score:** 1010210.0
- **Functions:** 0/1 matched (target 4)
- **Missing functions:** `CoroutineStart::invoke`
- **Types:** 1/1 matched (target 3)
- **Missing types:** _none_
- **Lint issues:** 19

### 5. internal.Symbol

- **Target:** `internal.Symbol`
- **Similarity:** 0.11
- **Dependents:** 1
- **Priority Score:** 1000308.9
- **Functions:** 2/2 matched (target 3)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 6. EventLoop.common

- **Target:** `native.EventLoop [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 484810.0
- **Functions:** 0/38 matched (target 0)
- **Missing functions:** `EventLoop::processNextEvent`, `EventLoop::processUnconfinedEvent`, `EventLoop::shouldBeProcessedFromContext`, `EventLoop::dispatchUnconfined`, `EventLoop::delta`, `EventLoop::incrementUseCount`, `EventLoop::decrementUseCount`, `EventLoop::limitedParallelism`, `EventLoop::shutdown`, `ThreadLocalEventLoop::currentOrNull`, `ThreadLocalEventLoop::resetEventLoop`, `ThreadLocalEventLoop::setEventLoop`, `delayToNanos`, `delayNanosToMillis`, `EventLoopImplBase::shutdown`, `EventLoopImplBase::scheduleResumeAfterDelay`, `EventLoopImplBase::scheduleInvokeOnTimeout`, `EventLoopImplBase::processNextEvent`, `EventLoopImplBase::dispatch`, `EventLoopImplBase::enqueue`, `EventLoopImplBase::enqueueImpl`, `EventLoopImplBase::dequeue`, `EventLoopImplBase::enqueueDelayedTasks`, `EventLoopImplBase::closeQueue`, `EventLoopImplBase::schedule`, `EventLoopImplBase::shouldUnpark`, `EventLoopImplBase::scheduleImpl`, `EventLoopImplBase::resetAll`, `EventLoopImplBase::rescheduleAllDelayed`, `EventLoopImplBase::DelayedTask::compareTo`, `EventLoopImplBase::DelayedTask::timeToExecute`, `EventLoopImplBase::DelayedTask::scheduleTask`, `EventLoopImplBase::DelayedTask::dispose`, `EventLoopImplBase::DelayedTask::toString`, `EventLoopImplBase::DelayedResumeTask::run`, `EventLoopImplBase::DelayedResumeTask::toString`, `EventLoopImplBase::DelayedRunnableTask::run`, `EventLoopImplBase::DelayedRunnableTask::toString`
- **Types:** 0/10 matched (target 0)
- **Missing types:** `EventLoop`, `ThreadLocalEventLoop`, `Queue`, `EventLoopImplPlatform`, `EventLoopImplBase`, `DelayedTask`, `DelayedResumeTask`, `DelayedRunnableTask`, `DelayedTaskQueue`, `DefaultExecutor`

### 7. channels.Deprecated

- **Target:** `channels.Deprecated [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 474710.0
- **Functions:** 0/47 matched (target 0)
- **Missing functions:** `BroadcastChannel<E>::consume`, `BroadcastChannel<E>::consumeEach`, `consumesAll`, `ReceiveChannel<E>::elementAt`, `ReceiveChannel<E>::elementAtOrNull`, `ReceiveChannel<E>::first`, `ReceiveChannel<E>::firstOrNull`, `ReceiveChannel<E>::indexOf`, `ReceiveChannel<E>::last`, `ReceiveChannel<E>::lastIndexOf`, `ReceiveChannel<E>::lastOrNull`, `ReceiveChannel<E>::single`, `ReceiveChannel<E>::singleOrNull`, `ReceiveChannel<E>::drop`, `ReceiveChannel<E>::dropWhile`, `ReceiveChannel<E>::filter`, `ReceiveChannel<E>::filterIndexed`, `ReceiveChannel<E>::filterNot`, `ReceiveChannel<E?>::filterNotNull`, `ReceiveChannel<E?>::filterNotNullTo`, `ReceiveChannel<E?>::filterNotNullTo`, `ReceiveChannel<E>::take`, `ReceiveChannel<E>::takeWhile`, `ReceiveChannel<E>::toChannel`, `ReceiveChannel<E>::toCollection`, `ReceiveChannel<Pair<K, V>>::toMap`, `ReceiveChannel<Pair<K, V>>::toMap`, `ReceiveChannel<E>::toMutableList`, `ReceiveChannel<E>::toSet`, `ReceiveChannel<E>::flatMap`, `ReceiveChannel<E>::map`, `ReceiveChannel<E>::mapIndexed`, `ReceiveChannel<E>::mapIndexedNotNull`, `ReceiveChannel<E>::mapNotNull`, `ReceiveChannel<E>::withIndex`, `ReceiveChannel<E>::distinct`, `ReceiveChannel<E>::distinctBy`, `ReceiveChannel<E>::toMutableSet`, `ReceiveChannel<E>::any`, `ReceiveChannel<E>::count`, `ReceiveChannel<E>::maxWith`, `ReceiveChannel<E>::minWith`, `ReceiveChannel<E>::none`, `ReceiveChannel<E?>::requireNoNulls`, `ReceiveChannel<E>::zip`, `ReceiveChannel<E>::zip`, `ReceiveChannel<*>::consumes`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 8. JobSupport

- **Target:** `JobSupport`
- **Similarity:** 0.13
- **Dependents:** 0
- **Priority Score:** 400908.7
- **Functions:** 54/91 matched (target 152)
- **Missing functions:** `JobSupport::loopOnState`, `JobSupport::finalizeFinishingState`, `JobSupport::getFinalRootCause`, `JobSupport::addSuppressedExceptions`, `JobSupport::tryFinalizeSimpleState`, `JobSupport::completeStateFinalization`, `JobSupport::notifyCancelling`, `JobSupport::cancelParent`, `JobSupport::notifyCompletion`, `JobSupport::notifyHandlers`, `JobSupport::startInternal`, `JobSupport::tryPutNodeIntoList`, `JobSupport::promoteEmptyToNodeList`, `JobSupport::promoteSingleToNodeList`, `JobSupport::SelectOnJoinCompletionHandler::invoke`, `JobSupport::removeNode`, `JobSupport::cancel`, `JobSupport::cancelMakeCompleting`, `JobSupport::createCauseException`, `JobSupport::makeCancelling`, `JobSupport::getOrPromoteCancellingList`, `JobSupport::tryMakeCancelling`, `JobSupport::tryMakeCompleting`, `JobSupport::tryMakeCompletingSlowPath`, `JobSupport::tryWaitForChild`, `JobSupport::continueCompleting`, `JobSupport::nextChild`, `JobSupport::Finishing::toString`, `JobSupport::AwaitContinuation::getContinuationCancellationCause`, `JobSupport::AwaitContinuation::nameString`, `JobSupport::SelectOnAwaitCompletionHandler::invoke`, `Empty::toString`, `JobImpl::complete`, `JobImpl::completeExceptionally`, `JobImpl::handlesException`, `InactiveNodeList::toString`, `ResumeAwaitOnCompletion::invoke`
- **Types:** 16/18 matched (target 21)
- **Missing types:** `JobImpl`, `ResumeAwaitOnCompletion`
- **Lint issues:** 24

### 9. flow.Migration

- **Target:** `flow.Migration`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 353610.0
- **Functions:** 1/36 matched (target 1)
- **Missing functions:** `Flow<T>::observeOn`, `Flow<T>::publishOn`, `Flow<T>::subscribeOn`, `Flow<T>::onErrorResume`, `Flow<T>::onErrorResumeNext`, `Flow<T>::subscribe`, `Flow<T>::subscribe`, `Flow<T>::subscribe`, `Flow<T>::flatMap`, `Flow<T>::concatMap`, `Flow<Flow<T>>::merge`, `Flow<Flow<T>>::flatten`, `Flow<T>::compose`, `Flow<T>::skip`, `Flow<T>::forEach`, `Flow<T>::scanFold`, `Flow<T>::onErrorReturn`, `Flow<T>::onErrorReturn`, `Flow<T>::startWith`, `Flow<T>::startWith`, `Flow<T>::concatWith`, `Flow<T>::concatWith`, `Flow<T1>::combineLatest`, `Flow<T1>::combineLatest`, `Flow<T1>::combineLatest`, `Flow<T1>::combineLatest`, `Flow<T>::delayFlow`, `Flow<T>::delayEach`, `Flow<T>::switchMap`, `Flow<T>::scanReduce`, `Flow<T>::publish`, `Flow<T>::publish`, `Flow<T>::replay`, `Flow<T>::replay`, `Flow<T>::cache`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 8

### 10. Job

- **Target:** `Job`
- **Similarity:** 0.10
- **Dependents:** 0
- **Priority Score:** 213109.0
- **Functions:** 5/24 matched (target 18)
- **Missing functions:** `Job::cancel`, `Job::plus`, `Job::invokeOnCompletion`, `Job`, `Job0`, `Job::disposeOnCompletion`, `Job::cancelAndJoin`, `Job::cancelChildren`, `Job::cancelChildren`, `CoroutineContext::cancel`, `CoroutineContext::cancel`, `CoroutineContext::ensureActive`, `Job::cancel`, `CoroutineContext::cancel`, `CoroutineContext::cancelChildren`, `CoroutineContext::cancelChildren`, `CoroutineContext::cancelChildren`, `orCancellation`, `DisposeOnCompletion::invoke`
- **Types:** 5/7 matched (target 9)
- **Missing types:** `DisposableHandle`, `DisposeOnCompletion`
- **Lint issues:** 1

### 11. CancellableContinuationImpl

- **Target:** `CancellableContinuationImpl`
- **Similarity:** 0.23
- **Dependents:** 0
- **Priority Score:** 175707.8
- **Functions:** 37/50 matched (target 129)
- **Missing functions:** `CancellableContinuationImpl::getStackTraceElement`, `CancellableContinuationImpl::callCancelHandlerSafely`, `CancellableContinuationImpl::invokeOnCancellationInternal`, `CancellableContinuationImpl::multipleHandlersError`, `CancellableContinuationImpl::tryResumeImpl`, `CancellableContinuationImpl::alreadyResumedError`, `CancellableContinuationImpl::getExceptionalResult`, `CancellableContinuationImpl::toString`, `CancellableContinuationImpl::nameString`, `Active::toString`, `CancelHandler::UserSupplied::invoke`, `CancelHandler::UserSupplied::toString`, `CompletedContinuation::invokeHandlers`
- **Types:** 3/7 matched (target 11)
- **Missing types:** `NotCompleted`, `Active`, `UserSupplied`, `CompletedContinuation`
- **Lint issues:** 25

### 12. flow.Builders

- **Target:** `flow.FlowBuilders`
- **Similarity:** 0.03
- **Dependents:** 0
- **Priority Score:** 172709.7
- **Functions:** 9/23 matched (target 20)
- **Missing functions:** `SafeFlow::collectSafely`, `Iterable<T>::asFlow`, `Iterator<T>::asFlow`, `Sequence<T>::asFlow`, `Array<T>::asFlow`, `IntArray::asFlow`, `LongArray::asFlow`, `IntRange::asFlow`, `LongRange::asFlow`, `ChannelFlowBuilder::create`, `ChannelFlowBuilder::collectTo`, `ChannelFlowBuilder::toString`, `CallbackFlowBuilder::collectTo`, `CallbackFlowBuilder::create`
- **Types:** 1/4 matched
- **Missing types:** `SafeFlow`, `ChannelFlowBuilder`, `CallbackFlowBuilder`
- **Lint issues:** 7

### 13. sync.Semaphore

- **Target:** `sync.Semaphore`
- **Similarity:** 0.15
- **Dependents:** 0
- **Priority Score:** 142508.5
- **Functions:** 8/21 matched (target 20)
- **Missing functions:** `Semaphore`, `SemaphoreAndMutexImpl::tryAcquire`, `SemaphoreAndMutexImpl::acquire`, `SemaphoreAndMutexImpl::acquireSlowPath`, `SemaphoreAndMutexImpl::acquire`, `SemaphoreAndMutexImpl::acquire`, `SemaphoreAndMutexImpl::onAcquireRegFunction`, `SemaphoreAndMutexImpl::decPermits`, `SemaphoreAndMutexImpl::release`, `SemaphoreAndMutexImpl::coerceAvailablePermitsAtMaximum`, `SemaphoreAndMutexImpl::addAcquireToQueue`, `SemaphoreAndMutexImpl::tryResumeNextFromQueue`, `SemaphoreAndMutexImpl::tryResumeAcquire`
- **Types:** 3/4 matched (target 3)
- **Missing types:** `SemaphoreAndMutexImpl`
- **Lint issues:** 2

### 14. operators.Lint

- **Target:** `flow.Lint [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 131310.0
- **Functions:** 0/13 matched (target 0)
- **Missing functions:** `SharedFlow<T>::cancellable`, `SharedFlow<T>::flowOn`, `StateFlow<T>::conflate`, `StateFlow<T>::distinctUntilChanged`, `FlowCollector<*>::cancel`, `SharedFlow<T>::catch`, `SharedFlow<T>::retry`, `SharedFlow<T>::retryWhen`, `SharedFlow<T>::toList`, `SharedFlow<T>::toList`, `SharedFlow<T>::toSet`, `SharedFlow<T>::toSet`, `SharedFlow<T>::count`
- **Types:** 0/0 matched (target 2)
- **Missing types:** _none_

### 15. channels.BufferedChannel

- **Target:** `channels.BufferedChannel`
- **Similarity:** 0.26
- **Dependents:** 0
- **Priority Score:** 121707.4
- **Functions:** 100/111 matched (target 161)
- **Missing functions:** `BufferedChannel::sendImpl`, `BufferedChannel::receiveImpl`, `BufferedChannel::cancel`, `BufferedChannel::cancel`, `BufferedChannel::invokeCloseHandler`, `BufferedChannel::toStringDebug`, `BufferedChannel::checkSegmentStructureInvariants`, `BufferedChannel::onCancellationChannelResultImplDoNotCall`, `BufferedChannel::onCancellationImplDoNotCall`, `createSegmentFunction`, `CancellableContinuation<T>::tryResume0`
- **Types:** 6/6 matched (target 7)
- **Missing types:** _none_
- **Lint issues:** 33

### 16. Await

- **Target:** `Await`
- **Similarity:** 0.01
- **Dependents:** 0
- **Priority Score:** 111209.9
- **Functions:** 1/9 matched (target 2)
- **Missing functions:** `Collection<Deferred<T>>::awaitAll`, `joinAll`, `Collection<Job>::joinAll`, `AwaitAll::await`, `AwaitAll::DisposeHandlersOnCancel::disposeAll`, `AwaitAll::DisposeHandlersOnCancel::invoke`, `AwaitAll::DisposeHandlersOnCancel::toString`, `AwaitAll::AwaitAllNode::invoke`
- **Types:** 0/3 matched (target 2)
- **Missing types:** `AwaitAll`, `DisposeHandlersOnCancel`, `AwaitAllNode`

### 17. operators.Zip

- **Target:** `flow.Zip`
- **Similarity:** 0.06
- **Dependents:** 0
- **Priority Score:** 91809.4
- **Functions:** 9/18 matched (target 11)
- **Missing functions:** `combine`, `combineTransform`, `combine`, `combineTransform`, `combineUnsafe`, `combineTransformUnsafe`, `nullArrayFactory`, `combine`, `combineTransform`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 5

### 18. Builders.common

- **Target:** `Builders.common`
- **Similarity:** 0.05
- **Dependents:** 0
- **Priority Score:** 82009.5
- **Functions:** 8/14 matched (target 51)
- **Missing functions:** `CoroutineDispatcher::invoke`, `DispatchedCoroutine::trySuspend`, `DispatchedCoroutine::tryResume`, `DispatchedCoroutine::afterCompletion`, `DispatchedCoroutine::afterResume`, `DispatchedCoroutine::getResult`
- **Types:** 4/6 matched (target 8)
- **Missing types:** `UndispatchedCoroutine`, `DispatchedCoroutine`
- **Lint issues:** 6

### 19. sync.Mutex

- **Target:** `sync.Mutex`
- **Similarity:** 0.23
- **Dependents:** 0
- **Priority Score:** 72007.7
- **Functions:** 11/16 matched (target 17)
- **Missing functions:** `Mutex`, `MutexImpl::CancellableContinuationWithOwner::tryResume`, `MutexImpl::CancellableContinuationWithOwner::resume`, `MutexImpl::SelectInstanceWithOwner::trySelect`, `MutexImpl::SelectInstanceWithOwner::selectInRegistrationPhase`
- **Types:** 2/4 matched (target 2)
- **Missing types:** `CancellableContinuationWithOwner`, `SelectInstanceWithOwner`

### 20. channels.Broadcast

- **Target:** `channels.Broadcast`
- **Similarity:** 0.14
- **Dependents:** 0
- **Priority Score:** 71208.6
- **Functions:** 3/10 matched (target 11)
- **Missing functions:** `ReceiveChannel<E>::broadcast`, `CoroutineScope::broadcast`, `BroadcastCoroutine::cancel`, `BroadcastCoroutine::cancel`, `BroadcastCoroutine::cancelInternal`, `LazyBroadcastCoroutine::openSubscription`, `LazyBroadcastCoroutine::onStart`
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Lint issues:** 5

### 21. CancellableContinuation

- **Target:** `CancellableContinuation`
- **Similarity:** 0.02
- **Dependents:** 0
- **Priority Score:** 70909.8
- **Functions:** 1/7 matched (target 8)
- **Missing functions:** `CancellableContinuation<T>::invokeOnCancellation`, `suspendCancellableCoroutine`, `suspendCancellableCoroutineReusable`, `getOrCreateCancellableContinuation`, `DisposeOnCancel::invoke`, `DisposeOnCancel::toString`
- **Types:** 1/2 matched (target 4)
- **Missing types:** `DisposeOnCancel`
- **Lint issues:** 5

### 22. selects.Select

- **Target:** `selects.Select`
- **Similarity:** 0.21
- **Dependents:** 0
- **Priority Score:** 64607.9
- **Functions:** 24/30 matched (target 71)
- **Missing functions:** `SelectBuilder::invoke`, `SelectBuilder::onTimeout`, `SelectImplementation::register`, `SelectImplementation::processResultAndInvokeBlockRecoveringException`, `CancellableContinuation<Unit>::tryResume`, `TrySelectDetailedResult`
- **Types:** 16/16 matched (target 21)
- **Missing types:** _none_
- **Lint issues:** 6

### 23. internal.ChannelFlow

- **Target:** `internal.ChannelFlow`
- **Similarity:** 0.19
- **Dependents:** 0
- **Priority Score:** 62508.1
- **Functions:** 14/19 matched (target 43)
- **Missing functions:** `ChannelFlowOperator::collectWithContextUndispatched`, `ChannelFlowOperator::toString`, `FlowCollector<T>::withUndispatchedContextCollector`, `StackFrameContinuation::resumeWith`, `StackFrameContinuation::getStackTraceElement`
- **Types:** 5/6 matched (target 9)
- **Missing types:** `StackFrameContinuation`
- **Lint issues:** 8

### 24. internal.DispatchedTask

- **Target:** `common.DispatchedTaskDispatch`
- **Similarity:** 0.12
- **Dependents:** 0
- **Priority Score:** 61208.8
- **Functions:** 6/10 matched (target 6)
- **Missing functions:** `DispatchedTask::cancelCompletedResult`, `DispatchedTask::getSuccessfulResult`, `DispatchedTask::getExceptionalResult`, `DispatchedTask<*>::runUnconfinedEventLoop`
- **Types:** 0/2 matched (target 0)
- **Missing types:** `DispatchedTask`, `DispatchException`
- **Lint issues:** 1

### 25. CoroutineScope

- **Target:** `CoroutineScope`
- **Similarity:** 0.03
- **Dependents:** 0
- **Priority Score:** 61009.7
- **Functions:** 2/8 matched (target 12)
- **Missing functions:** `CoroutineScope::plus`, `MainScope`, `coroutineScope`, `CoroutineScope`, `CoroutineScope::cancel`, `currentCoroutineContext`
- **Types:** 2/2 matched (target 3)
- **Missing types:** _none_
- **Lint issues:** 3

### 26. flow.SharedFlow

- **Target:** `flow.SharedFlow`
- **Similarity:** 0.35
- **Dependents:** 0
- **Priority Score:** 53606.5
- **Functions:** 26/31 matched (target 47)
- **Missing functions:** `MutableSharedFlow`, `SharedFlowImpl::fuse`, `Array<Any?>::getBufferAt`, `Array<Any?>::setBufferAt`, `SharedFlow<T>::fuseSharedFlow`
- **Types:** 5/5 matched (target 8)
- **Missing types:** _none_
- **Lint issues:** 5

### 27. operators.Emitters

- **Target:** `flow.Emitters`
- **Similarity:** 0.05
- **Dependents:** 0
- **Priority Score:** 50909.5
- **Functions:** 4/8 matched (target 7)
- **Missing functions:** `Flow<T>::unsafeTransform`, `FlowCollector<*>::ensureActive`, `ThrowingCollector::emit`, `FlowCollector<T>::invokeSafely`
- **Types:** 0/1 matched (target 0)
- **Missing types:** `ThrowingCollector`

### 28. internal.ConcurrentLinkedList

- **Target:** `internal.ConcurrentLinkedList`
- **Similarity:** 0.12
- **Dependents:** 0
- **Priority Score:** 41608.8
- **Functions:** 9/13 matched (target 27)
- **Missing functions:** `AtomicRef<S>::moveForward`, `AtomicRef<S>::findSegmentAndMoveForward`, `N::close`, `AtomicInt::addConditionally`
- **Types:** 3/3 matched (target 5)
- **Missing types:** _none_
- **Lint issues:** 3

### 29. operators.Delay

- **Target:** `flow.Delay`
- **Similarity:** 0.09
- **Dependents:** 0
- **Priority Score:** 41009.1
- **Functions:** 6/10 matched (target 9)
- **Missing functions:** `Flow<T>::debounce`, `Flow<T>::debounce`, `Flow<T>::sample`, `Flow<T>::timeoutInternal`
- **Types:** 0/0 matched (target 1)
- **Missing types:** _none_
- **Lint issues:** 8

### 30. internal.Concurrent.common

- **Target:** `internal.Concurrent.common [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 40410.0
- **Functions:** 0/1 matched (target 0)
- **Missing functions:** `WorkaroundAtomicReference<T>::loop`
- **Types:** 0/3 matched (target 0)
- **Missing types:** `ReentrantLock`, `BenignDataRace`, `WorkaroundAtomicReference`

### 31. flow.SharingStarted

- **Target:** `flow.SharingStarted`
- **Similarity:** 0.32
- **Dependents:** 0
- **Priority Score:** 31506.8
- **Functions:** 7/10 matched (target 20)
- **Missing functions:** `SharingStarted.Companion::WhileSubscribed`, `StartedWhileSubscribed::equals`, `StartedWhileSubscribed::hashCode`
- **Types:** 5/5 matched (target 7)
- **Missing types:** _none_
- **Lint issues:** 7

### 32. CompletionState

- **Target:** `CompletionState`
- **Similarity:** 0.10
- **Dependents:** 0
- **Priority Score:** 30809.0
- **Functions:** 4/6 matched
- **Missing functions:** `CompletedExceptionally::toString`, `CancelledContinuation::makeResumed`
- **Types:** 1/2 matched
- **Missing types:** `CancelledContinuation`

### 33. channels.Produce

- **Target:** `channels.Produce`
- **Similarity:** 0.18
- **Dependents:** 0
- **Priority Score:** 30808.2
- **Functions:** 4/6 matched (target 14)
- **Missing functions:** `CoroutineScope::produce`, `CoroutineScope::produce`
- **Types:** 1/2 matched
- **Missing types:** `ProducerScope`
- **Lint issues:** 1

### 34. CoroutineExceptionHandler

- **Target:** `CoroutineExceptionHandler`
- **Similarity:** 0.03
- **Dependents:** 0
- **Priority Score:** 30509.7
- **Functions:** 1/4 matched (target 2)
- **Missing functions:** `handlerException`, `CoroutineExceptionHandler`, `handleException`
- **Types:** 1/1 matched
- **Missing types:** _none_

### 35. internal.SystemProps.common

- **Target:** `internal.SystemProps.common`
- **Similarity:** 0.02
- **Dependents:** 0
- **Priority Score:** 30409.8
- **Functions:** 1/4 matched (target 5)
- **Missing functions:** `systemProp`, `systemProp`, `systemProp`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 36. internal.LockFreeTaskQueue

- **Target:** `internal.LockFreeTaskQueue`
- **Similarity:** 0.33
- **Dependents:** 0
- **Priority Score:** 22506.7
- **Functions:** 19/21 matched (target 27)
- **Missing functions:** `LockFreeTaskQueueCore::wo`, `LockFreeTaskQueueCore::withState`
- **Types:** 4/4 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 37. flow.StateFlow

- **Target:** `flow.StateFlow`
- **Similarity:** 0.25
- **Dependents:** 0
- **Priority Score:** 22307.5
- **Functions:** 17/19 matched (target 44)
- **Missing functions:** `MutableStateFlow`, `StateFlowImpl::fuse`
- **Types:** 4/4 matched (target 7)
- **Missing types:** _none_
- **Lint issues:** 5

### 38. operators.Share

- **Target:** `flow.Share`
- **Similarity:** 0.18
- **Dependents:** 0
- **Priority Score:** 21808.2
- **Functions:** 12/13 matched (target 49)
- **Missing functions:** `SubscribedFlowCollector::onSubscription`
- **Types:** 4/5 matched (target 10)
- **Missing types:** `SubscribedFlowCollector`
- **Lint issues:** 4

### 39. operators.Context

- **Target:** `flow.Context`
- **Similarity:** 0.09
- **Dependents:** 0
- **Priority Score:** 20909.1
- **Functions:** 6/7 matched
- **Missing functions:** `Flow<T>::buffer`
- **Types:** 1/2 matched (target 1)
- **Missing types:** `CancellableFlow`
- **Lint issues:** 4

### 40. CompletableDeferred

- **Target:** `CompletableDeferred`
- **Similarity:** 0.16
- **Dependents:** 0
- **Priority Score:** 20908.4
- **Functions:** 5/7 matched (target 15)
- **Missing functions:** `CompletableDeferred`, `CompletableDeferred`
- **Types:** 2/2 matched (target 3)
- **Missing types:** _none_

### 41. channels.Channels.common

- **Target:** `channels.Channels.common`
- **Similarity:** 0.05
- **Dependents:** 0
- **Priority Score:** 20609.5
- **Functions:** 4/6 matched (target 11)
- **Missing functions:** `ReceiveChannel<E>::receiveOrNull`, `ReceiveChannel<E>::onReceiveOrNull`
- **Types:** 0/0 matched (target 1)
- **Missing types:** _none_
- **Lint issues:** 1

### 42. operators.Errors

- **Target:** `flow.Errors`
- **Similarity:** 0.07
- **Dependents:** 0
- **Priority Score:** 20609.3
- **Functions:** 4/6 matched (target 13)
- **Missing functions:** `Throwable::isCancellationCause`, `Throwable::isSameExceptionAs`
- **Types:** 0/0 matched (target 1)
- **Missing types:** _none_

### 43. channels.ChannelCoroutine

- **Target:** `channels.ChannelCoroutine`
- **Similarity:** 0.29
- **Dependents:** 0
- **Priority Score:** 20507.1
- **Functions:** 2/4 matched (target 15)
- **Missing functions:** `ChannelCoroutine::cancel`, `ChannelCoroutine::cancel`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 44. internal.NamedDispatcher

- **Target:** `internal.NamedDispatcher`
- **Similarity:** 0.39
- **Dependents:** 0
- **Priority Score:** 20506.1
- **Functions:** 2/4 matched
- **Missing functions:** `NamedDispatcher::isDispatchNeeded`, `NamedDispatcher::dispatchYield`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 45. channels.BroadcastChannel

- **Target:** `channels.BroadcastChannel`
- **Similarity:** 0.15
- **Dependents:** 0
- **Priority Score:** 11608.5
- **Functions:** 10/11 matched (target 35)
- **Missing functions:** `BroadcastChannel`
- **Types:** 5/5 matched (target 6)
- **Missing types:** _none_
- **Lint issues:** 8

### 46. operators.Transform

- **Target:** `flow.Transform`
- **Similarity:** 0.07
- **Dependents:** 0
- **Priority Score:** 11309.3
- **Functions:** 12/13 matched (target 100)
- **Missing functions:** `Flow<*>::filterIsInstance`
- **Types:** 0/0 matched (target 13)
- **Missing types:** _none_
- **Lint issues:** 10

### 47. internal.LimitedDispatcher

- **Target:** `internal.LimitedDispatcher`
- **Similarity:** 0.27
- **Dependents:** 0
- **Priority Score:** 11207.3
- **Functions:** 9/10 matched (target 12)
- **Missing functions:** `CoroutineDispatcher::namedOrThis`
- **Types:** 2/2 matched (target 3)
- **Missing types:** _none_
- **Lint issues:** 13

### 48. NonCancellable

- **Target:** `NonCancellable`
- **Similarity:** 0.34
- **Dependents:** 0
- **Priority Score:** 11006.6
- **Functions:** 8/9 matched (target 18)
- **Missing functions:** `NonCancellable::cancel`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 4

### 49. operators.Merge

- **Target:** `flow.Merge`
- **Similarity:** 0.08
- **Dependents:** 0
- **Priority Score:** 10909.2
- **Functions:** 8/9 matched (target 23)
- **Missing functions:** `Iterable<Flow<T>>::merge`
- **Types:** 0/0 matched (target 3)
- **Missing types:** _none_
- **Lint issues:** 4

### 50. CoroutineDispatcher

- **Target:** `CoroutineDispatcher`
- **Similarity:** 0.39
- **Dependents:** 0
- **Priority Score:** 10906.1
- **Functions:** 7/8 matched (target 9)
- **Missing functions:** `CoroutineDispatcher::limitedParallelism`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 51. Delay

- **Target:** `Delay`
- **Similarity:** 0.25
- **Dependents:** 0
- **Priority Score:** 10807.5
- **Functions:** 5/6 matched (target 13)
- **Missing functions:** `Duration::toDelayMillis`
- **Types:** 2/2 matched
- **Missing types:** _none_

### 52. channels.ConflatedBufferedChannel

- **Target:** `channels.ConflatedBufferedChannel`
- **Similarity:** 0.37
- **Dependents:** 0
- **Priority Score:** 10806.3
- **Functions:** 6/7 matched (target 9)
- **Missing functions:** `ConflatedBufferedChannel::registerSelectForSend`
- **Types:** 1/1 matched
- **Missing types:** _none_

### 53. operators.Distinct

- **Target:** `flow.Distinct`
- **Similarity:** 0.04
- **Dependents:** 0
- **Priority Score:** 10609.6
- **Functions:** 4/5 matched (target 10)
- **Missing functions:** `Flow<T>::distinctUntilChangedBy`
- **Types:** 1/1 matched (target 5)
- **Missing types:** _none_
- **Lint issues:** 2

### 54. selects.OnTimeout

- **Target:** `selects.OnTimeout`
- **Similarity:** 0.03
- **Dependents:** 0
- **Priority Score:** 10409.7
- **Functions:** 2/3 matched (target 6)
- **Missing functions:** `OnTimeout::register`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 6

### 55. internal.Combine

- **Target:** `internal.Combine`
- **Similarity:** 0.02
- **Dependents:** 0
- **Priority Score:** 10309.8
- **Functions:** 1/2 matched (target 23)
- **Missing functions:** `FlowCollector<R>::combineInternal`
- **Types:** 1/1 matched (target 9)
- **Missing types:** _none_
- **Lint issues:** 4

### 56. internal.InlineList

- **Target:** `internal.InlineList`
- **Similarity:** 0.10
- **Dependents:** 0
- **Priority Score:** 10309.0
- **Functions:** 1/2 matched (target 5)
- **Missing functions:** `InlineList::plus`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 3

### 57. internal.LocalAtomics.common

- **Target:** `internal.LocalAtomics.common`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 10100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 0/1 matched (target 0)
- **Missing types:** `LocalAtomicInt`

### 58. internal.ThreadSafeHeap

- **Target:** `internal.ThreadSafeHeap`
- **Similarity:** 0.38
- **Dependents:** 0
- **Priority Score:** 1606.2
- **Functions:** 14/14 matched (target 20)
- **Missing functions:** _none_
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 59. internal.Merge

- **Target:** `internal.Merge`
- **Similarity:** 0.26
- **Dependents:** 0
- **Priority Score:** 1207.4
- **Functions:** 9/9 matched (target 31)
- **Missing functions:** _none_
- **Types:** 3/3 matched (target 9)
- **Missing types:** _none_
- **Lint issues:** 10

### 60. Timeout

- **Target:** `Timeout`
- **Similarity:** 0.13
- **Dependents:** 0
- **Priority Score:** 1108.7
- **Functions:** 9/9 matched (target 13)
- **Missing functions:** _none_
- **Types:** 2/2 matched (target 3)
- **Missing types:** _none_
- **Lint issues:** 3

### 61. terminal.Reduce

- **Target:** `flow.Reduce`
- **Similarity:** 0.03
- **Dependents:** 0
- **Priority Score:** 1009.7
- **Functions:** 10/10 matched (target 64)
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 6)
- **Missing types:** _none_
- **Lint issues:** 9

### 62. selects.SelectOld

- **Target:** `selects.SelectOld`
- **Similarity:** 0.22
- **Dependents:** 0
- **Priority Score:** 1007.8
- **Functions:** 8/8 matched (target 10)
- **Missing functions:** _none_
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Lint issues:** 5

### 63. AbstractCoroutine

- **Target:** `AbstractCoroutine`
- **Similarity:** 0.38
- **Dependents:** 0
- **Priority Score:** 1006.2
- **Functions:** 9/9 matched (target 16)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 4

### 64. operators.Limit

- **Target:** `flow.Limit`
- **Similarity:** 0.05
- **Dependents:** 0
- **Priority Score:** 809.5
- **Functions:** 8/8 matched (target 44)
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 4)
- **Missing types:** _none_
- **Lint issues:** 9

### 65. terminal.Collect

- **Target:** `flow.Collect`
- **Similarity:** 0.13
- **Dependents:** 0
- **Priority Score:** 808.7
- **Functions:** 8/8 matched (target 25)
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 4)
- **Missing types:** _none_
- **Lint issues:** 4

### 66. selects.SelectUnbiased

- **Target:** `selects.SelectUnbiased`
- **Similarity:** 0.16
- **Dependents:** 0
- **Priority Score:** 708.4
- **Functions:** 6/6 matched (target 8)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 67. Supervisor

- **Target:** `Supervisor`
- **Similarity:** 0.24
- **Dependents:** 0
- **Priority Score:** 707.6
- **Functions:** 5/5 matched (target 8)
- **Missing functions:** _none_
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Lint issues:** 3

### 68. Unconfined

- **Target:** `Unconfined`
- **Similarity:** 0.47
- **Dependents:** 0
- **Priority Score:** 605.3
- **Functions:** 4/4 matched (target 6)
- **Missing functions:** _none_
- **Types:** 2/2 matched (target 3)
- **Missing types:** _none_
- **Lint issues:** 7

### 69. intrinsics.Cancellable

- **Target:** `intrinsics.Cancellable`
- **Similarity:** 0.16
- **Dependents:** 0
- **Priority Score:** 508.4
- **Functions:** 5/5 matched (target 19)
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 2)
- **Missing types:** _none_
- **Lint issues:** 3

### 70. Exceptions.common

- **Target:** `Exceptions [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 410.0
- **Functions:** 0/0 matched (target 15)
- **Missing functions:** _none_
- **Types:** 4/4 matched (target 7)
- **Missing types:** _none_
- **Lint issues:** 7

### 71. MainCoroutineDispatcher

- **Target:** `MainCoroutineDispatcher`
- **Similarity:** 0.08
- **Dependents:** 0
- **Priority Score:** 409.2
- **Functions:** 3/3 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 72. internal.FlowCoroutine

- **Target:** `internal.FlowCoroutine`
- **Similarity:** 0.19
- **Dependents:** 0
- **Priority Score:** 408.1
- **Functions:** 3/3 matched (target 6)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_
- **Lint issues:** 1

### 73. internal.OnUndeliveredElement

- **Target:** `internal.OnUndeliveredElement`
- **Similarity:** 0.32
- **Dependents:** 0
- **Priority Score:** 406.8
- **Functions:** 2/2 matched (target 3)
- **Missing functions:** _none_
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 74. terminal.Logic

- **Target:** `flow.Logic`
- **Similarity:** 0.02
- **Dependents:** 0
- **Priority Score:** 309.8
- **Functions:** 3/3 matched (target 23)
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 4)
- **Missing types:** _none_
- **Lint issues:** 5

### 75. terminal.Collection

- **Target:** `flow.Collection`
- **Similarity:** 0.11
- **Dependents:** 0
- **Priority Score:** 308.9
- **Functions:** 3/3 matched (target 51)
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 9)
- **Missing types:** _none_
- **Lint issues:** 1

### 76. terminal.Count

- **Target:** `flow.Count`
- **Similarity:** 0.03
- **Dependents:** 0
- **Priority Score:** 209.7
- **Functions:** 2/2 matched (target 26)
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 2)
- **Missing types:** _none_
- **Lint issues:** 2

### 77. CoroutineName

- **Target:** `CoroutineName`
- **Similarity:** 0.04
- **Dependents:** 0
- **Priority Score:** 209.6
- **Functions:** 1/1 matched (target 6)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 78. internal.MainDispatcherFactory

- **Target:** `internal.MainDispatcherFactory`
- **Similarity:** 0.07
- **Dependents:** 0
- **Priority Score:** 209.3
- **Functions:** 1/1 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 79. internal.NopCollector

- **Target:** `internal.NopCollector`
- **Similarity:** 0.08
- **Dependents:** 0
- **Priority Score:** 209.2
- **Functions:** 1/1 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 80. internal.SendingCollector

- **Target:** `internal.SendingCollector`
- **Similarity:** 0.16
- **Dependents:** 0
- **Priority Score:** 208.4
- **Functions:** 1/1 matched (target 2)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 81. Guidance

- **Target:** `Guidance`
- **Similarity:** 0.17
- **Dependents:** 0
- **Priority Score:** 208.3
- **Functions:** 2/2 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 5)
- **Missing types:** _none_

### 82. internal.Synchronized.common

- **Target:** `internal.SynchronizedObject`
- **Similarity:** 0.24
- **Dependents:** 0
- **Priority Score:** 207.6
- **Functions:** 1/1 matched (target 6)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 83. Runnable.common

- **Target:** `Runnable [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 110.0
- **Functions:** 0/0 matched (target 3)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 84. Waiter

- **Target:** `Waiter [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 110.0
- **Functions:** 0/0 matched (target 1)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 85. selects.WhileSelect

- **Target:** `selects.WhileSelect`
- **Similarity:** 0.04
- **Dependents:** 0
- **Priority Score:** 109.6
- **Functions:** 1/1 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 86. Yield

- **Target:** `Yield`
- **Similarity:** 0.17
- **Dependents:** 0
- **Priority Score:** 108.3
- **Functions:** 1/1 matched (target 3)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 4

### 87. CloseableCoroutineDispatcher

- **Target:** `CloseableCoroutineDispatcher`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 88. Deferred

- **Target:** `Deferred`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 89. Dispatchers.common

- **Target:** `Dispatchers`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 90. Debug.common

- **Target:** `Debug.common`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 91. Annotations

- **Target:** `Annotations`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 7)
- **Missing types:** _none_

### 92. flow.FlowCollector

- **Target:** `flow.FlowCollector`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 93. CompletableJob

- **Target:** `CompletableJob`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 94. CompletionHandler.common

- **Target:** `CompletionHandler`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 95. internal.NullSurrogate

- **Target:** `internal.NullSurrogate [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 96. internal.ProbesSupport.common

- **Target:** `internal.ProbesSupport.common [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 2)
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 1)
- **Missing types:** _none_
- **Lint issues:** 1

## Success Criteria

For each file to be considered "complete":
- **Similarity ≥ 0.85** (Excellent threshold)
- All public APIs ported
- All tests ported
- Documentation ported
- port-lint header present

