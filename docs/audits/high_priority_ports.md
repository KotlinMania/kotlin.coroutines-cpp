# High Priority Ports - Action Plan

## Files by Impact

Priority = deps * 1,000,000 + SymDeficit * 10,000 + SrcSymbols * 100 + (1 - function similarity) * 10

Dependency fanout is ranked first so the ladder favors ports that clear downstream compilation failures fastest.

This list is complete and includes function/type detail for every matched file. Function similarity is the required body/parameter comparison; file-level shape does not rescue a port.

| Rank | Source | Target | Function similarity | Deps | Functions | Missing functions | Types | Missing types | SymDeficit | SrcSymbols | Priority |
|------|--------|--------|------------|------|-----------|-------------------|-------|---------------|-----------|------------|----------|
| 1 | `flow.Channels` | `flow.Channels` | 0.23 | 14 | 12/12 matched (target 22) | _none_ | 1/1 matched (target 2) | _none_ | 0 | 13 | 14001308.0 |
| 2 | `flow.Flow` | `flow.Flow [ZERO]` | 0.00 | 13 | 0/1 matched (target 0) | `AbstractFlow::collect` | 2/2 matched (target 3) | _none_ | 1 | 3 | 13010310.0 |
| 3 | `channels.BufferOverflow` | `channels.BufferOverflow` | 1.00 | 2 | 0/0 matched | _none_ | 1/1 matched | _none_ | 0 | 1 | 2000100.0 |
| 4 | `CoroutineStart` | `CoroutineStart` | 0.00 | 1 | 0/1 matched (target 4) | `CoroutineStart::invoke` | 1/1 matched (target 3) | _none_ | 1 | 2 | 1010210.0 |
| 5 | `internal.Symbol` | `internal.Symbol` | 0.11 | 1 | 2/2 matched (target 3) | _none_ | 1/1 matched | _none_ | 0 | 3 | 1000308.9 |
| 6 | `EventLoop.common` | `native.EventLoop [ZERO]` | 0.00 | 0 | 0/38 matched (target 0) | `EventLoop::processNextEvent`, `EventLoop::processUnconfinedEvent`, `EventLoop::shouldBeProcessedFromContext`, `EventLoop::dispatchUnconfined`, `EventLoop::delta`, `EventLoop::incrementUseCount`, `EventLoop::decrementUseCount`, `EventLoop::limitedParallelism`, `EventLoop::shutdown`, `ThreadLocalEventLoop::currentOrNull`, `ThreadLocalEventLoop::resetEventLoop`, `ThreadLocalEventLoop::setEventLoop`, `delayToNanos`, `delayNanosToMillis`, `EventLoopImplBase::shutdown`, `EventLoopImplBase::scheduleResumeAfterDelay`, `EventLoopImplBase::scheduleInvokeOnTimeout`, `EventLoopImplBase::processNextEvent`, `EventLoopImplBase::dispatch`, `EventLoopImplBase::enqueue`, `EventLoopImplBase::enqueueImpl`, `EventLoopImplBase::dequeue`, `EventLoopImplBase::enqueueDelayedTasks`, `EventLoopImplBase::closeQueue`, `EventLoopImplBase::schedule`, `EventLoopImplBase::shouldUnpark`, `EventLoopImplBase::scheduleImpl`, `EventLoopImplBase::resetAll`, `EventLoopImplBase::rescheduleAllDelayed`, `EventLoopImplBase::DelayedTask::compareTo`, `EventLoopImplBase::DelayedTask::timeToExecute`, `EventLoopImplBase::DelayedTask::scheduleTask`, `EventLoopImplBase::DelayedTask::dispose`, `EventLoopImplBase::DelayedTask::toString`, `EventLoopImplBase::DelayedResumeTask::run`, `EventLoopImplBase::DelayedResumeTask::toString`, `EventLoopImplBase::DelayedRunnableTask::run`, `EventLoopImplBase::DelayedRunnableTask::toString` | 0/10 matched (target 0) | `EventLoop`, `ThreadLocalEventLoop`, `Queue`, `EventLoopImplPlatform`, `EventLoopImplBase`, `DelayedTask`, `DelayedResumeTask`, `DelayedRunnableTask`, `DelayedTaskQueue`, `DefaultExecutor` | 48 | 48 | 484810.0 |
| 7 | `channels.Deprecated` | `channels.Deprecated [ZERO]` | 0.00 | 0 | 0/47 matched (target 0) | `BroadcastChannel<E>::consume`, `BroadcastChannel<E>::consumeEach`, `consumesAll`, `ReceiveChannel<E>::elementAt`, `ReceiveChannel<E>::elementAtOrNull`, `ReceiveChannel<E>::first`, `ReceiveChannel<E>::firstOrNull`, `ReceiveChannel<E>::indexOf`, `ReceiveChannel<E>::last`, `ReceiveChannel<E>::lastIndexOf`, `ReceiveChannel<E>::lastOrNull`, `ReceiveChannel<E>::single`, `ReceiveChannel<E>::singleOrNull`, `ReceiveChannel<E>::drop`, `ReceiveChannel<E>::dropWhile`, `ReceiveChannel<E>::filter`, `ReceiveChannel<E>::filterIndexed`, `ReceiveChannel<E>::filterNot`, `ReceiveChannel<E?>::filterNotNull`, `ReceiveChannel<E?>::filterNotNullTo`, `ReceiveChannel<E?>::filterNotNullTo`, `ReceiveChannel<E>::take`, `ReceiveChannel<E>::takeWhile`, `ReceiveChannel<E>::toChannel`, `ReceiveChannel<E>::toCollection`, `ReceiveChannel<Pair<K, V>>::toMap`, `ReceiveChannel<Pair<K, V>>::toMap`, `ReceiveChannel<E>::toMutableList`, `ReceiveChannel<E>::toSet`, `ReceiveChannel<E>::flatMap`, `ReceiveChannel<E>::map`, `ReceiveChannel<E>::mapIndexed`, `ReceiveChannel<E>::mapIndexedNotNull`, `ReceiveChannel<E>::mapNotNull`, `ReceiveChannel<E>::withIndex`, `ReceiveChannel<E>::distinct`, `ReceiveChannel<E>::distinctBy`, `ReceiveChannel<E>::toMutableSet`, `ReceiveChannel<E>::any`, `ReceiveChannel<E>::count`, `ReceiveChannel<E>::maxWith`, `ReceiveChannel<E>::minWith`, `ReceiveChannel<E>::none`, `ReceiveChannel<E?>::requireNoNulls`, `ReceiveChannel<E>::zip`, `ReceiveChannel<E>::zip`, `ReceiveChannel<*>::consumes` | 0/0 matched | _none_ | 47 | 47 | 474710.0 |
| 8 | `JobSupport` | `JobSupport` | 0.13 | 0 | 54/91 matched (target 152) | `JobSupport::loopOnState`, `JobSupport::finalizeFinishingState`, `JobSupport::getFinalRootCause`, `JobSupport::addSuppressedExceptions`, `JobSupport::tryFinalizeSimpleState`, `JobSupport::completeStateFinalization`, `JobSupport::notifyCancelling`, `JobSupport::cancelParent`, `JobSupport::notifyCompletion`, `JobSupport::notifyHandlers`, `JobSupport::startInternal`, `JobSupport::tryPutNodeIntoList`, `JobSupport::promoteEmptyToNodeList`, `JobSupport::promoteSingleToNodeList`, `JobSupport::SelectOnJoinCompletionHandler::invoke`, `JobSupport::removeNode`, `JobSupport::cancel`, `JobSupport::cancelMakeCompleting`, `JobSupport::createCauseException`, `JobSupport::makeCancelling`, `JobSupport::getOrPromoteCancellingList`, `JobSupport::tryMakeCancelling`, `JobSupport::tryMakeCompleting`, `JobSupport::tryMakeCompletingSlowPath`, `JobSupport::tryWaitForChild`, `JobSupport::continueCompleting`, `JobSupport::nextChild`, `JobSupport::Finishing::toString`, `JobSupport::AwaitContinuation::getContinuationCancellationCause`, `JobSupport::AwaitContinuation::nameString`, `JobSupport::SelectOnAwaitCompletionHandler::invoke`, `Empty::toString`, `JobImpl::complete`, `JobImpl::completeExceptionally`, `JobImpl::handlesException`, `InactiveNodeList::toString`, `ResumeAwaitOnCompletion::invoke` | 16/18 matched (target 21) | `JobImpl`, `ResumeAwaitOnCompletion` | 39 | 109 | 400908.7 |
| 9 | `flow.Migration` | `flow.Migration` | 0.00 | 0 | 1/36 matched (target 1) | `Flow<T>::observeOn`, `Flow<T>::publishOn`, `Flow<T>::subscribeOn`, `Flow<T>::onErrorResume`, `Flow<T>::onErrorResumeNext`, `Flow<T>::subscribe`, `Flow<T>::subscribe`, `Flow<T>::subscribe`, `Flow<T>::flatMap`, `Flow<T>::concatMap`, `Flow<Flow<T>>::merge`, `Flow<Flow<T>>::flatten`, `Flow<T>::compose`, `Flow<T>::skip`, `Flow<T>::forEach`, `Flow<T>::scanFold`, `Flow<T>::onErrorReturn`, `Flow<T>::onErrorReturn`, `Flow<T>::startWith`, `Flow<T>::startWith`, `Flow<T>::concatWith`, `Flow<T>::concatWith`, `Flow<T1>::combineLatest`, `Flow<T1>::combineLatest`, `Flow<T1>::combineLatest`, `Flow<T1>::combineLatest`, `Flow<T>::delayFlow`, `Flow<T>::delayEach`, `Flow<T>::switchMap`, `Flow<T>::scanReduce`, `Flow<T>::publish`, `Flow<T>::publish`, `Flow<T>::replay`, `Flow<T>::replay`, `Flow<T>::cache` | 0/0 matched | _none_ | 35 | 36 | 353610.0 |
| 10 | `Job` | `Job` | 0.10 | 0 | 5/24 matched (target 18) | `Job::cancel`, `Job::plus`, `Job::invokeOnCompletion`, `Job`, `Job0`, `Job::disposeOnCompletion`, `Job::cancelAndJoin`, `Job::cancelChildren`, `Job::cancelChildren`, `CoroutineContext::cancel`, `CoroutineContext::cancel`, `CoroutineContext::ensureActive`, `Job::cancel`, `CoroutineContext::cancel`, `CoroutineContext::cancelChildren`, `CoroutineContext::cancelChildren`, `CoroutineContext::cancelChildren`, `orCancellation`, `DisposeOnCompletion::invoke` | 5/7 matched (target 9) | `DisposableHandle`, `DisposeOnCompletion` | 21 | 31 | 213109.0 |
| 11 | `CancellableContinuationImpl` | `CancellableContinuationImpl` | 0.23 | 0 | 37/50 matched (target 129) | `CancellableContinuationImpl::getStackTraceElement`, `CancellableContinuationImpl::callCancelHandlerSafely`, `CancellableContinuationImpl::invokeOnCancellationInternal`, `CancellableContinuationImpl::multipleHandlersError`, `CancellableContinuationImpl::tryResumeImpl`, `CancellableContinuationImpl::alreadyResumedError`, `CancellableContinuationImpl::getExceptionalResult`, `CancellableContinuationImpl::toString`, `CancellableContinuationImpl::nameString`, `Active::toString`, `CancelHandler::UserSupplied::invoke`, `CancelHandler::UserSupplied::toString`, `CompletedContinuation::invokeHandlers` | 3/7 matched (target 11) | `NotCompleted`, `Active`, `UserSupplied`, `CompletedContinuation` | 17 | 57 | 175707.8 |
| 12 | `flow.Builders` | `flow.FlowBuilders` | 0.03 | 0 | 9/23 matched (target 20) | `SafeFlow::collectSafely`, `Iterable<T>::asFlow`, `Iterator<T>::asFlow`, `Sequence<T>::asFlow`, `Array<T>::asFlow`, `IntArray::asFlow`, `LongArray::asFlow`, `IntRange::asFlow`, `LongRange::asFlow`, `ChannelFlowBuilder::create`, `ChannelFlowBuilder::collectTo`, `ChannelFlowBuilder::toString`, `CallbackFlowBuilder::collectTo`, `CallbackFlowBuilder::create` | 1/4 matched | `SafeFlow`, `ChannelFlowBuilder`, `CallbackFlowBuilder` | 17 | 27 | 172709.7 |
| 13 | `sync.Semaphore` | `sync.Semaphore` | 0.15 | 0 | 8/21 matched (target 20) | `Semaphore`, `SemaphoreAndMutexImpl::tryAcquire`, `SemaphoreAndMutexImpl::acquire`, `SemaphoreAndMutexImpl::acquireSlowPath`, `SemaphoreAndMutexImpl::acquire`, `SemaphoreAndMutexImpl::acquire`, `SemaphoreAndMutexImpl::onAcquireRegFunction`, `SemaphoreAndMutexImpl::decPermits`, `SemaphoreAndMutexImpl::release`, `SemaphoreAndMutexImpl::coerceAvailablePermitsAtMaximum`, `SemaphoreAndMutexImpl::addAcquireToQueue`, `SemaphoreAndMutexImpl::tryResumeNextFromQueue`, `SemaphoreAndMutexImpl::tryResumeAcquire` | 3/4 matched (target 3) | `SemaphoreAndMutexImpl` | 14 | 25 | 142508.5 |
| 14 | `operators.Lint` | `flow.Lint [ZERO]` | 0.00 | 0 | 0/13 matched (target 0) | `SharedFlow<T>::cancellable`, `SharedFlow<T>::flowOn`, `StateFlow<T>::conflate`, `StateFlow<T>::distinctUntilChanged`, `FlowCollector<*>::cancel`, `SharedFlow<T>::catch`, `SharedFlow<T>::retry`, `SharedFlow<T>::retryWhen`, `SharedFlow<T>::toList`, `SharedFlow<T>::toList`, `SharedFlow<T>::toSet`, `SharedFlow<T>::toSet`, `SharedFlow<T>::count` | 0/0 matched (target 2) | _none_ | 13 | 13 | 131310.0 |
| 15 | `channels.BufferedChannel` | `channels.BufferedChannel` | 0.26 | 0 | 100/111 matched (target 161) | `BufferedChannel::sendImpl`, `BufferedChannel::receiveImpl`, `BufferedChannel::cancel`, `BufferedChannel::cancel`, `BufferedChannel::invokeCloseHandler`, `BufferedChannel::toStringDebug`, `BufferedChannel::checkSegmentStructureInvariants`, `BufferedChannel::onCancellationChannelResultImplDoNotCall`, `BufferedChannel::onCancellationImplDoNotCall`, `createSegmentFunction`, `CancellableContinuation<T>::tryResume0` | 6/6 matched (target 7) | _none_ | 11 | 117 | 121707.4 |
| 16 | `Await` | `Await` | 0.01 | 0 | 1/9 matched (target 2) | `Collection<Deferred<T>>::awaitAll`, `joinAll`, `Collection<Job>::joinAll`, `AwaitAll::await`, `AwaitAll::DisposeHandlersOnCancel::disposeAll`, `AwaitAll::DisposeHandlersOnCancel::invoke`, `AwaitAll::DisposeHandlersOnCancel::toString`, `AwaitAll::AwaitAllNode::invoke` | 0/3 matched (target 2) | `AwaitAll`, `DisposeHandlersOnCancel`, `AwaitAllNode` | 11 | 12 | 111209.9 |
| 17 | `operators.Zip` | `flow.Zip` | 0.06 | 0 | 9/18 matched (target 11) | `combine`, `combineTransform`, `combine`, `combineTransform`, `combineUnsafe`, `combineTransformUnsafe`, `nullArrayFactory`, `combine`, `combineTransform` | 0/0 matched | _none_ | 9 | 18 | 91809.4 |
| 18 | `Builders.common` | `Builders.common` | 0.05 | 0 | 8/14 matched (target 51) | `CoroutineDispatcher::invoke`, `DispatchedCoroutine::trySuspend`, `DispatchedCoroutine::tryResume`, `DispatchedCoroutine::afterCompletion`, `DispatchedCoroutine::afterResume`, `DispatchedCoroutine::getResult` | 4/6 matched (target 8) | `UndispatchedCoroutine`, `DispatchedCoroutine` | 8 | 20 | 82009.5 |
| 19 | `sync.Mutex` | `sync.Mutex` | 0.23 | 0 | 11/16 matched (target 17) | `Mutex`, `MutexImpl::CancellableContinuationWithOwner::tryResume`, `MutexImpl::CancellableContinuationWithOwner::resume`, `MutexImpl::SelectInstanceWithOwner::trySelect`, `MutexImpl::SelectInstanceWithOwner::selectInRegistrationPhase` | 2/4 matched (target 2) | `CancellableContinuationWithOwner`, `SelectInstanceWithOwner` | 7 | 20 | 72007.7 |
| 20 | `channels.Broadcast` | `channels.Broadcast` | 0.14 | 0 | 3/10 matched (target 11) | `ReceiveChannel<E>::broadcast`, `CoroutineScope::broadcast`, `BroadcastCoroutine::cancel`, `BroadcastCoroutine::cancel`, `BroadcastCoroutine::cancelInternal`, `LazyBroadcastCoroutine::openSubscription`, `LazyBroadcastCoroutine::onStart` | 2/2 matched | _none_ | 7 | 12 | 71208.6 |
| 21 | `CancellableContinuation` | `CancellableContinuation` | 0.02 | 0 | 1/7 matched (target 8) | `CancellableContinuation<T>::invokeOnCancellation`, `suspendCancellableCoroutine`, `suspendCancellableCoroutineReusable`, `getOrCreateCancellableContinuation`, `DisposeOnCancel::invoke`, `DisposeOnCancel::toString` | 1/2 matched (target 4) | `DisposeOnCancel` | 7 | 9 | 70909.8 |
| 22 | `selects.Select` | `selects.Select` | 0.21 | 0 | 24/30 matched (target 71) | `SelectBuilder::invoke`, `SelectBuilder::onTimeout`, `SelectImplementation::register`, `SelectImplementation::processResultAndInvokeBlockRecoveringException`, `CancellableContinuation<Unit>::tryResume`, `TrySelectDetailedResult` | 16/16 matched (target 21) | _none_ | 6 | 46 | 64607.9 |
| 23 | `internal.ChannelFlow` | `internal.ChannelFlow` | 0.19 | 0 | 14/19 matched (target 43) | `ChannelFlowOperator::collectWithContextUndispatched`, `ChannelFlowOperator::toString`, `FlowCollector<T>::withUndispatchedContextCollector`, `StackFrameContinuation::resumeWith`, `StackFrameContinuation::getStackTraceElement` | 5/6 matched (target 9) | `StackFrameContinuation` | 6 | 25 | 62508.1 |
| 24 | `internal.DispatchedTask` | `common.DispatchedTaskDispatch` | 0.12 | 0 | 6/10 matched (target 6) | `DispatchedTask::cancelCompletedResult`, `DispatchedTask::getSuccessfulResult`, `DispatchedTask::getExceptionalResult`, `DispatchedTask<*>::runUnconfinedEventLoop` | 0/2 matched (target 0) | `DispatchedTask`, `DispatchException` | 6 | 12 | 61208.8 |
| 25 | `CoroutineScope` | `CoroutineScope` | 0.03 | 0 | 2/8 matched (target 12) | `CoroutineScope::plus`, `MainScope`, `coroutineScope`, `CoroutineScope`, `CoroutineScope::cancel`, `currentCoroutineContext` | 2/2 matched (target 3) | _none_ | 6 | 10 | 61009.7 |
| 26 | `flow.SharedFlow` | `flow.SharedFlow` | 0.35 | 0 | 26/31 matched (target 47) | `MutableSharedFlow`, `SharedFlowImpl::fuse`, `Array<Any?>::getBufferAt`, `Array<Any?>::setBufferAt`, `SharedFlow<T>::fuseSharedFlow` | 5/5 matched (target 8) | _none_ | 5 | 36 | 53606.5 |
| 27 | `operators.Emitters` | `flow.Emitters` | 0.05 | 0 | 4/8 matched (target 7) | `Flow<T>::unsafeTransform`, `FlowCollector<*>::ensureActive`, `ThrowingCollector::emit`, `FlowCollector<T>::invokeSafely` | 0/1 matched (target 0) | `ThrowingCollector` | 5 | 9 | 50909.5 |
| 28 | `internal.ConcurrentLinkedList` | `internal.ConcurrentLinkedList` | 0.12 | 0 | 9/13 matched (target 27) | `AtomicRef<S>::moveForward`, `AtomicRef<S>::findSegmentAndMoveForward`, `N::close`, `AtomicInt::addConditionally` | 3/3 matched (target 5) | _none_ | 4 | 16 | 41608.8 |
| 29 | `operators.Delay` | `flow.Delay` | 0.09 | 0 | 6/10 matched (target 9) | `Flow<T>::debounce`, `Flow<T>::debounce`, `Flow<T>::sample`, `Flow<T>::timeoutInternal` | 0/0 matched (target 1) | _none_ | 4 | 10 | 41009.1 |
| 30 | `internal.Concurrent.common` | `internal.Concurrent.common [ZERO]` | 0.00 | 0 | 0/1 matched (target 0) | `WorkaroundAtomicReference<T>::loop` | 0/3 matched (target 0) | `ReentrantLock`, `BenignDataRace`, `WorkaroundAtomicReference` | 4 | 4 | 40410.0 |
| 31 | `flow.SharingStarted` | `flow.SharingStarted` | 0.32 | 0 | 7/10 matched (target 20) | `SharingStarted.Companion::WhileSubscribed`, `StartedWhileSubscribed::equals`, `StartedWhileSubscribed::hashCode` | 5/5 matched (target 7) | _none_ | 3 | 15 | 31506.8 |
| 32 | `CompletionState` | `CompletionState` | 0.10 | 0 | 4/6 matched | `CompletedExceptionally::toString`, `CancelledContinuation::makeResumed` | 1/2 matched | `CancelledContinuation` | 3 | 8 | 30809.0 |
| 33 | `channels.Produce` | `channels.Produce` | 0.18 | 0 | 4/6 matched (target 14) | `CoroutineScope::produce`, `CoroutineScope::produce` | 1/2 matched | `ProducerScope` | 3 | 8 | 30808.2 |
| 34 | `CoroutineExceptionHandler` | `CoroutineExceptionHandler` | 0.03 | 0 | 1/4 matched (target 2) | `handlerException`, `CoroutineExceptionHandler`, `handleException` | 1/1 matched | _none_ | 3 | 5 | 30509.7 |
| 35 | `internal.SystemProps.common` | `internal.SystemProps.common` | 0.02 | 0 | 1/4 matched (target 5) | `systemProp`, `systemProp`, `systemProp` | 0/0 matched | _none_ | 3 | 4 | 30409.8 |
| 36 | `internal.LockFreeTaskQueue` | `internal.LockFreeTaskQueue` | 0.33 | 0 | 19/21 matched (target 27) | `LockFreeTaskQueueCore::wo`, `LockFreeTaskQueueCore::withState` | 4/4 matched | _none_ | 2 | 25 | 22506.7 |
| 37 | `flow.StateFlow` | `flow.StateFlow` | 0.25 | 0 | 17/19 matched (target 44) | `MutableStateFlow`, `StateFlowImpl::fuse` | 4/4 matched (target 7) | _none_ | 2 | 23 | 22307.5 |
| 38 | `operators.Share` | `flow.Share` | 0.18 | 0 | 12/13 matched (target 49) | `SubscribedFlowCollector::onSubscription` | 4/5 matched (target 10) | `SubscribedFlowCollector` | 2 | 18 | 21808.2 |
| 39 | `operators.Context` | `flow.Context` | 0.09 | 0 | 6/7 matched | `Flow<T>::buffer` | 1/2 matched (target 1) | `CancellableFlow` | 2 | 9 | 20909.1 |
| 40 | `CompletableDeferred` | `CompletableDeferred` | 0.16 | 0 | 5/7 matched (target 15) | `CompletableDeferred`, `CompletableDeferred` | 2/2 matched (target 3) | _none_ | 2 | 9 | 20908.4 |
| 41 | `channels.Channels.common` | `channels.Channels.common` | 0.05 | 0 | 4/6 matched (target 11) | `ReceiveChannel<E>::receiveOrNull`, `ReceiveChannel<E>::onReceiveOrNull` | 0/0 matched (target 1) | _none_ | 2 | 6 | 20609.5 |
| 42 | `operators.Errors` | `flow.Errors` | 0.07 | 0 | 4/6 matched (target 13) | `Throwable::isCancellationCause`, `Throwable::isSameExceptionAs` | 0/0 matched (target 1) | _none_ | 2 | 6 | 20609.3 |
| 43 | `channels.ChannelCoroutine` | `channels.ChannelCoroutine` | 0.29 | 0 | 2/4 matched (target 15) | `ChannelCoroutine::cancel`, `ChannelCoroutine::cancel` | 1/1 matched | _none_ | 2 | 5 | 20507.1 |
| 44 | `internal.NamedDispatcher` | `internal.NamedDispatcher` | 0.39 | 0 | 2/4 matched | `NamedDispatcher::isDispatchNeeded`, `NamedDispatcher::dispatchYield` | 1/1 matched | _none_ | 2 | 5 | 20506.1 |
| 45 | `channels.BroadcastChannel` | `channels.BroadcastChannel` | 0.15 | 0 | 10/11 matched (target 35) | `BroadcastChannel` | 5/5 matched (target 6) | _none_ | 1 | 16 | 11608.5 |
| 46 | `operators.Transform` | `flow.Transform` | 0.07 | 0 | 12/13 matched (target 100) | `Flow<*>::filterIsInstance` | 0/0 matched (target 13) | _none_ | 1 | 13 | 11309.3 |
| 47 | `internal.LimitedDispatcher` | `internal.LimitedDispatcher` | 0.27 | 0 | 9/10 matched (target 12) | `CoroutineDispatcher::namedOrThis` | 2/2 matched (target 3) | _none_ | 1 | 12 | 11207.3 |
| 48 | `NonCancellable` | `NonCancellable` | 0.34 | 0 | 8/9 matched (target 18) | `NonCancellable::cancel` | 1/1 matched | _none_ | 1 | 10 | 11006.6 |
| 49 | `operators.Merge` | `flow.Merge` | 0.08 | 0 | 8/9 matched (target 23) | `Iterable<Flow<T>>::merge` | 0/0 matched (target 3) | _none_ | 1 | 9 | 10909.2 |
| 50 | `CoroutineDispatcher` | `CoroutineDispatcher` | 0.39 | 0 | 7/8 matched (target 9) | `CoroutineDispatcher::limitedParallelism` | 1/1 matched | _none_ | 1 | 9 | 10906.1 |
| 51 | `Delay` | `Delay` | 0.25 | 0 | 5/6 matched (target 13) | `Duration::toDelayMillis` | 2/2 matched | _none_ | 1 | 8 | 10807.5 |
| 52 | `channels.ConflatedBufferedChannel` | `channels.ConflatedBufferedChannel` | 0.37 | 0 | 6/7 matched (target 9) | `ConflatedBufferedChannel::registerSelectForSend` | 1/1 matched | _none_ | 1 | 8 | 10806.3 |
| 53 | `operators.Distinct` | `flow.Distinct` | 0.04 | 0 | 4/5 matched (target 10) | `Flow<T>::distinctUntilChangedBy` | 1/1 matched (target 5) | _none_ | 1 | 6 | 10609.6 |
| 54 | `selects.OnTimeout` | `selects.OnTimeout` | 0.03 | 0 | 2/3 matched (target 6) | `OnTimeout::register` | 1/1 matched | _none_ | 1 | 4 | 10409.7 |
| 55 | `internal.Combine` | `internal.Combine` | 0.02 | 0 | 1/2 matched (target 23) | `FlowCollector<R>::combineInternal` | 1/1 matched (target 9) | _none_ | 1 | 3 | 10309.8 |
| 56 | `internal.InlineList` | `internal.InlineList` | 0.10 | 0 | 1/2 matched (target 5) | `InlineList::plus` | 1/1 matched | _none_ | 1 | 3 | 10309.0 |
| 57 | `internal.LocalAtomics.common` | `internal.LocalAtomics.common` | 1.00 | 0 | 0/0 matched | _none_ | 0/1 matched (target 0) | `LocalAtomicInt` | 1 | 1 | 10100.0 |
| 58 | `internal.ThreadSafeHeap` | `internal.ThreadSafeHeap` | 0.38 | 0 | 14/14 matched (target 20) | _none_ | 2/2 matched | _none_ | 0 | 16 | 1606.2 |
| 59 | `internal.Merge` | `internal.Merge` | 0.26 | 0 | 9/9 matched (target 31) | _none_ | 3/3 matched (target 9) | _none_ | 0 | 12 | 1207.4 |
| 60 | `Timeout` | `Timeout` | 0.13 | 0 | 9/9 matched (target 13) | _none_ | 2/2 matched (target 3) | _none_ | 0 | 11 | 1108.7 |
| 61 | `terminal.Reduce` | `flow.Reduce` | 0.03 | 0 | 10/10 matched (target 64) | _none_ | 0/0 matched (target 6) | _none_ | 0 | 10 | 1009.7 |
| 62 | `selects.SelectOld` | `selects.SelectOld` | 0.22 | 0 | 8/8 matched (target 10) | _none_ | 2/2 matched | _none_ | 0 | 10 | 1007.8 |
| 63 | `AbstractCoroutine` | `AbstractCoroutine` | 0.38 | 0 | 9/9 matched (target 16) | _none_ | 1/1 matched | _none_ | 0 | 10 | 1006.2 |
| 64 | `operators.Limit` | `flow.Limit` | 0.05 | 0 | 8/8 matched (target 44) | _none_ | 0/0 matched (target 4) | _none_ | 0 | 8 | 809.5 |
| 65 | `terminal.Collect` | `flow.Collect` | 0.13 | 0 | 8/8 matched (target 25) | _none_ | 0/0 matched (target 4) | _none_ | 0 | 8 | 808.7 |
| 66 | `selects.SelectUnbiased` | `selects.SelectUnbiased` | 0.16 | 0 | 6/6 matched (target 8) | _none_ | 1/1 matched (target 2) | _none_ | 0 | 7 | 708.4 |
| 67 | `Supervisor` | `Supervisor` | 0.24 | 0 | 5/5 matched (target 8) | _none_ | 2/2 matched | _none_ | 0 | 7 | 707.6 |
| 68 | `Unconfined` | `Unconfined` | 0.47 | 0 | 4/4 matched (target 6) | _none_ | 2/2 matched (target 3) | _none_ | 0 | 6 | 605.3 |
| 69 | `intrinsics.Cancellable` | `intrinsics.Cancellable` | 0.16 | 0 | 5/5 matched (target 19) | _none_ | 0/0 matched (target 2) | _none_ | 0 | 5 | 508.4 |
| 70 | `Exceptions.common` | `Exceptions [ZERO]` | 0.00 | 0 | 0/0 matched (target 15) | _none_ | 4/4 matched (target 7) | _none_ | 0 | 4 | 410.0 |
| 71 | `MainCoroutineDispatcher` | `MainCoroutineDispatcher` | 0.08 | 0 | 3/3 matched | _none_ | 1/1 matched | _none_ | 0 | 4 | 409.2 |
| 72 | `internal.FlowCoroutine` | `internal.FlowCoroutine` | 0.19 | 0 | 3/3 matched (target 6) | _none_ | 1/1 matched (target 2) | _none_ | 0 | 4 | 408.1 |
| 73 | `internal.OnUndeliveredElement` | `internal.OnUndeliveredElement` | 0.32 | 0 | 2/2 matched (target 3) | _none_ | 2/2 matched | _none_ | 0 | 4 | 406.8 |
| 74 | `terminal.Logic` | `flow.Logic` | 0.02 | 0 | 3/3 matched (target 23) | _none_ | 0/0 matched (target 4) | _none_ | 0 | 3 | 309.8 |
| 75 | `terminal.Collection` | `flow.Collection` | 0.11 | 0 | 3/3 matched (target 51) | _none_ | 0/0 matched (target 9) | _none_ | 0 | 3 | 308.9 |
| 76 | `terminal.Count` | `flow.Count` | 0.03 | 0 | 2/2 matched (target 26) | _none_ | 0/0 matched (target 2) | _none_ | 0 | 2 | 209.7 |
| 77 | `CoroutineName` | `CoroutineName` | 0.04 | 0 | 1/1 matched (target 6) | _none_ | 1/1 matched | _none_ | 0 | 2 | 209.6 |
| 78 | `internal.MainDispatcherFactory` | `internal.MainDispatcherFactory` | 0.07 | 0 | 1/1 matched | _none_ | 1/1 matched (target 2) | _none_ | 0 | 2 | 209.3 |
| 79 | `internal.NopCollector` | `internal.NopCollector` | 0.08 | 0 | 1/1 matched | _none_ | 1/1 matched | _none_ | 0 | 2 | 209.2 |
| 80 | `internal.SendingCollector` | `internal.SendingCollector` | 0.16 | 0 | 1/1 matched (target 2) | _none_ | 1/1 matched | _none_ | 0 | 2 | 208.4 |
| 81 | `Guidance` | `Guidance` | 0.17 | 0 | 2/2 matched | _none_ | 0/0 matched (target 5) | _none_ | 0 | 2 | 208.3 |
| 82 | `internal.Synchronized.common` | `internal.SynchronizedObject` | 0.24 | 0 | 1/1 matched (target 6) | _none_ | 1/1 matched | _none_ | 0 | 2 | 207.6 |
| 83 | `Runnable.common` | `Runnable [ZERO]` | 0.00 | 0 | 0/0 matched (target 3) | _none_ | 1/1 matched (target 2) | _none_ | 0 | 1 | 110.0 |
| 84 | `Waiter` | `Waiter [ZERO]` | 0.00 | 0 | 0/0 matched (target 1) | _none_ | 1/1 matched (target 2) | _none_ | 0 | 1 | 110.0 |
| 85 | `selects.WhileSelect` | `selects.WhileSelect` | 0.04 | 0 | 1/1 matched | _none_ | 0/0 matched | _none_ | 0 | 1 | 109.6 |
| 86 | `Yield` | `Yield` | 0.17 | 0 | 1/1 matched (target 3) | _none_ | 0/0 matched | _none_ | 0 | 1 | 108.3 |
| 87 | `CloseableCoroutineDispatcher` | `CloseableCoroutineDispatcher` | 1.00 | 0 | 0/0 matched | _none_ | 1/1 matched | _none_ | 0 | 1 | 100.0 |
| 88 | `Deferred` | `Deferred` | 1.00 | 0 | 0/0 matched | _none_ | 1/1 matched | _none_ | 0 | 1 | 100.0 |
| 89 | `Dispatchers.common` | `Dispatchers` | 1.00 | 0 | 0/0 matched | _none_ | 1/1 matched (target 2) | _none_ | 0 | 1 | 100.0 |
| 90 | `Debug.common` | `Debug.common` | 1.00 | 0 | 0/0 matched | _none_ | 1/1 matched | _none_ | 0 | 1 | 100.0 |
| 91 | `Annotations` | `Annotations` | 1.00 | 0 | 0/0 matched | _none_ | 1/1 matched (target 7) | _none_ | 0 | 1 | 100.0 |
| 92 | `flow.FlowCollector` | `flow.FlowCollector` | 1.00 | 0 | 0/0 matched | _none_ | 1/1 matched | _none_ | 0 | 1 | 100.0 |
| 93 | `CompletableJob` | `CompletableJob` | 1.00 | 0 | 0/0 matched | _none_ | 1/1 matched | _none_ | 0 | 1 | 100.0 |
| 94 | `CompletionHandler.common` | `CompletionHandler` | 1.00 | 0 | 0/0 matched | _none_ | 1/1 matched | _none_ | 0 | 1 | 100.0 |
| 95 | `internal.NullSurrogate` | `internal.NullSurrogate [ZERO]` | 0.00 | 0 | 0/0 matched | _none_ | 0/0 matched | _none_ | 0 | 0 | 10.0 |
| 96 | `internal.ProbesSupport.common` | `internal.ProbesSupport.common [ZERO]` | 0.00 | 0 | 0/0 matched (target 2) | _none_ | 0/0 matched (target 1) | _none_ | 0 | 0 | 10.0 |

## Cheat Detection / Scoring Failures

- `flow.Flow` -> `flow.Flow [ZERO]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `EventLoop.common` -> `native.EventLoop [ZERO]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `channels.Deprecated` -> `channels.Deprecated [ZERO]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `operators.Lint` -> `flow.Lint [ZERO]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `internal.Concurrent.common` -> `internal.Concurrent.common [ZERO]`: function-by-function score forced to 0. no target functions found; report scoring is function-by-function only
- `Exceptions.common` -> `Exceptions [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Runnable.common` -> `Runnable [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `Waiter` -> `Waiter [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.NullSurrogate` -> `internal.NullSurrogate [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only
- `internal.ProbesSupport.common` -> `internal.ProbesSupport.common [ZERO]`: function-by-function score forced to 0. no source functions found; target defines functions; report scoring is function-by-function only

## Critical Issues (Function Similarity < 0.60 with Dependencies)

These files need immediate attention:

- **flow.Channels** → `flow.Channels`
  - Function similarity: 0.23
  - Dependencies: 14
  - Functions: 12/12 matched (target 22)
  - Missing functions: _none_
  - Types: 1/1 matched (target 2)
  - Missing types: _none_

- **flow.Flow** → `flow.Flow [ZERO]`
  - Function similarity: 0.00
  - Dependencies: 13
  - Functions: 0/1 matched (target 0)
  - Missing functions: `AbstractFlow::collect`
  - Types: 2/2 matched (target 3)
  - Missing types: _none_
  - Scoring failure: no target functions found; report scoring is function-by-function only
  - Lint issues: 2

- **CoroutineStart** → `CoroutineStart`
  - Function similarity: 0.00
  - Dependencies: 1
  - Functions: 0/1 matched (target 4)
  - Missing functions: `CoroutineStart::invoke`
  - Types: 1/1 matched (target 3)
  - Missing types: _none_
  - Lint issues: 19

- **internal.Symbol** → `internal.Symbol`
  - Function similarity: 0.11
  - Dependencies: 1
  - Functions: 2/2 matched (target 3)
  - Missing functions: _none_
  - Types: 1/1 matched
  - Missing types: _none_

## Missing Files (by Dependents)

| Rank | Source file | Expected target | Deps | Functions | Classes/types | Symbols | Source path | Expected path |
|------|-------------|-----------------|------|-----------|---------------|---------|-------------|---------------|
| 1 | `CoroutineContext.common` | `CoroutineContext.common` | 0 | 0 | 0 | 0 | `CoroutineContext.common.kt` | `CoroutineContext.common.cpp` |
| 2 | `SchedulerTask.common` | `SchedulerTask.common` | 0 | 0 | 1 | 1 | `SchedulerTask.common.kt` | `SchedulerTask.common.cpp` |
| 3 | `channels.Channel` | `channels.Channel` | 0 | 22 | 9 | 31 | `channels/Channel.kt` | `channels/Channel.cpp` |
| 4 | `internal.AbstractSharedFlow` | `flow.internal.AbstractSharedFlow` | 0 | 4 | 3 | 7 | `flow/internal/AbstractSharedFlow.kt` | `flow/internal/AbstractSharedFlow.cpp` |
| 5 | `internal.FlowExceptions.common` | `flow.internal.FlowExceptions.common` | 0 | 2 | 2 | 4 | `flow/internal/FlowExceptions.common.kt` | `flow/internal/FlowExceptions.common.cpp` |
| 6 | `internal.SafeCollector.common` | `flow.internal.SafeCollector.common` | 0 | 4 | 1 | 5 | `flow/internal/SafeCollector.common.kt` | `flow/internal/SafeCollector.common.cpp` |
| 7 | `internal.CoroutineExceptionHandlerImpl.common` | `internal.CoroutineExceptionHandlerImpl.common` | 0 | 1 | 2 | 3 | `internal/CoroutineExceptionHandlerImpl.common.kt` | `internal/CoroutineExceptionHandlerImpl.common.cpp` |
| 8 | `internal.DispatchedContinuation` | `internal.DispatchedContinuation` | 0 | 19 | 1 | 20 | `internal/DispatchedContinuation.kt` | `internal/DispatchedContinuation.cpp` |
| 9 | `internal.InternalAnnotations.common` | `internal.InternalAnnotations.common` | 0 | 0 | 1 | 1 | `internal/InternalAnnotations.common.kt` | `internal/InternalAnnotations.common.cpp` |
| 10 | `internal.LockFreeLinkedList.common` | `internal.LockFreeLinkedList.common` | 0 | 0 | 2 | 2 | `internal/LockFreeLinkedList.common.kt` | `internal/LockFreeLinkedList.common.cpp` |
| 11 | `internal.Scopes` | `internal.Scopes` | 0 | 5 | 2 | 7 | `internal/Scopes.kt` | `internal/Scopes.cpp` |
| 12 | `internal.StackTraceRecovery.common` | `internal.StackTraceRecovery.common` | 0 | 0 | 2 | 2 | `internal/StackTraceRecovery.common.kt` | `internal/StackTraceRecovery.common.cpp` |
| 13 | `internal.ThreadContext.common` | `internal.ThreadContext.common` | 0 | 0 | 0 | 0 | `internal/ThreadContext.common.kt` | `internal/ThreadContext.common.cpp` |
| 14 | `internal.ThreadLocal.common` | `internal.ThreadLocal.common` | 0 | 0 | 1 | 1 | `internal/ThreadLocal.common.kt` | `internal/ThreadLocal.common.cpp` |
| 15 | `intrinsics.Undispatched` | `intrinsics.Undispatched` | 0 | 6 | 0 | 6 | `intrinsics/Undispatched.kt` | `intrinsics/Undispatched.cpp` |

