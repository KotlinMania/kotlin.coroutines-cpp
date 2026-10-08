# Code Port - Progress Report

**Generated:** 2026-10-05
**Source:** tmp/kotlinx.coroutines/kotlinx-coroutines-core/common/src
**Target:** src/kotlinx/coroutines

## Executive Summary

| Metric | Count | Percentage |
|--------|-------|------------|
| Function parity | 633/1059 matched (target 1766) | 59.8% |
| Class/type parity | 159/228 matched (target 317) | 69.7% |
| Combined symbol parity | 792/1287 matched (target 2083) | 61.5% |
| Average function body similarity | 0.23 | inline-code cosine |
| Average documentation similarity | 0.53 | doc text cosine |
| Missing source functions | 63 | 0% parity until ported |
| Missing source classes/types | 27 | 0% parity until ported |
| Missing source symbol files | 13 | 90 symbols |
| Cheat/scoring failures | 10 | forced to 0% |
| Total source files | 111 | 100% |
| Target units (paired) | 201 | - |
| Target files (total) | 298 | - |
| Porting progress | 96 | 86.5% (matched) |
| Missing files | 15 | 13.5% |

## Port Quality Analysis

**Average Function Similarity:** 0.23

Similarity in this report is the required function-by-function body/parameter score. Class/type parity and symbol deficits are reported beside it; whole-file shape is diagnostic only.

**Work Distribution:**
- Critical (<0.60): 86 files (89.6% of matched)
- Needs review (0.60-0.84): 0 files (0.0% of matched)
- Excellent (>=0.85): 10 files (10.4% of matched)

## Worst Function Scores First

Every matched file is listed from lowest function body/parameter similarity upward. Missing symbol names are not capped.

| Rank | Source | Target | Function similarity | Functions | Missing functions | Types | Missing types | Tests | Symbol deficit | Priority |
|------|--------|--------|---------------------|-----------|-------------------|-------|---------------|-------|----------------|----------|
| 1 | `flow.Flow` | `flow.Flow [ZERO]` | 0.00 | 0/1 matched (target 0) | `AbstractFlow::collect` | 2/2 matched (target 3) | _none_ | - | 1 | 13010310.0 |
| 2 | `EventLoop.common` | `native.EventLoop [ZERO]` | 0.00 | 0/38 matched (target 0) | `EventLoop::processNextEvent`, `EventLoop::processUnconfinedEvent`, `EventLoop::shouldBeProcessedFromContext`, `EventLoop::dispatchUnconfined`, `EventLoop::delta`, `EventLoop::incrementUseCount`, `EventLoop::decrementUseCount`, `EventLoop::limitedParallelism`, `EventLoop::shutdown`, `ThreadLocalEventLoop::currentOrNull`, `ThreadLocalEventLoop::resetEventLoop`, `ThreadLocalEventLoop::setEventLoop`, `delayToNanos`, `delayNanosToMillis`, `EventLoopImplBase::shutdown`, `EventLoopImplBase::scheduleResumeAfterDelay`, `EventLoopImplBase::scheduleInvokeOnTimeout`, `EventLoopImplBase::processNextEvent`, `EventLoopImplBase::dispatch`, `EventLoopImplBase::enqueue`, `EventLoopImplBase::enqueueImpl`, `EventLoopImplBase::dequeue`, `EventLoopImplBase::enqueueDelayedTasks`, `EventLoopImplBase::closeQueue`, `EventLoopImplBase::schedule`, `EventLoopImplBase::shouldUnpark`, `EventLoopImplBase::scheduleImpl`, `EventLoopImplBase::resetAll`, `EventLoopImplBase::rescheduleAllDelayed`, `EventLoopImplBase::DelayedTask::compareTo`, `EventLoopImplBase::DelayedTask::timeToExecute`, `EventLoopImplBase::DelayedTask::scheduleTask`, `EventLoopImplBase::DelayedTask::dispose`, `EventLoopImplBase::DelayedTask::toString`, `EventLoopImplBase::DelayedResumeTask::run`, `EventLoopImplBase::DelayedResumeTask::toString`, `EventLoopImplBase::DelayedRunnableTask::run`, `EventLoopImplBase::DelayedRunnableTask::toString` | 0/10 matched (target 0) | `EventLoop`, `ThreadLocalEventLoop`, `Queue`, `EventLoopImplPlatform`, `EventLoopImplBase`, `DelayedTask`, `DelayedResumeTask`, `DelayedRunnableTask`, `DelayedTaskQueue`, `DefaultExecutor` | - | 48 | 484810.0 |
| 3 | `channels.Deprecated` | `channels.Deprecated [ZERO]` | 0.00 | 0/47 matched (target 0) | `BroadcastChannel<E>::consume`, `BroadcastChannel<E>::consumeEach`, `consumesAll`, `ReceiveChannel<E>::elementAt`, `ReceiveChannel<E>::elementAtOrNull`, `ReceiveChannel<E>::first`, `ReceiveChannel<E>::firstOrNull`, `ReceiveChannel<E>::indexOf`, `ReceiveChannel<E>::last`, `ReceiveChannel<E>::lastIndexOf`, `ReceiveChannel<E>::lastOrNull`, `ReceiveChannel<E>::single`, `ReceiveChannel<E>::singleOrNull`, `ReceiveChannel<E>::drop`, `ReceiveChannel<E>::dropWhile`, `ReceiveChannel<E>::filter`, `ReceiveChannel<E>::filterIndexed`, `ReceiveChannel<E>::filterNot`, `ReceiveChannel<E?>::filterNotNull`, `ReceiveChannel<E?>::filterNotNullTo`, `ReceiveChannel<E?>::filterNotNullTo`, `ReceiveChannel<E>::take`, `ReceiveChannel<E>::takeWhile`, `ReceiveChannel<E>::toChannel`, `ReceiveChannel<E>::toCollection`, `ReceiveChannel<Pair<K, V>>::toMap`, `ReceiveChannel<Pair<K, V>>::toMap`, `ReceiveChannel<E>::toMutableList`, `ReceiveChannel<E>::toSet`, `ReceiveChannel<E>::flatMap`, `ReceiveChannel<E>::map`, `ReceiveChannel<E>::mapIndexed`, `ReceiveChannel<E>::mapIndexedNotNull`, `ReceiveChannel<E>::mapNotNull`, `ReceiveChannel<E>::withIndex`, `ReceiveChannel<E>::distinct`, `ReceiveChannel<E>::distinctBy`, `ReceiveChannel<E>::toMutableSet`, `ReceiveChannel<E>::any`, `ReceiveChannel<E>::count`, `ReceiveChannel<E>::maxWith`, `ReceiveChannel<E>::minWith`, `ReceiveChannel<E>::none`, `ReceiveChannel<E?>::requireNoNulls`, `ReceiveChannel<E>::zip`, `ReceiveChannel<E>::zip`, `ReceiveChannel<*>::consumes` | 0/0 matched | _none_ | - | 47 | 474710.0 |
| 4 | `operators.Lint` | `flow.Lint [ZERO]` | 0.00 | 0/13 matched (target 0) | `SharedFlow<T>::cancellable`, `SharedFlow<T>::flowOn`, `StateFlow<T>::conflate`, `StateFlow<T>::distinctUntilChanged`, `FlowCollector<*>::cancel`, `SharedFlow<T>::catch`, `SharedFlow<T>::retry`, `SharedFlow<T>::retryWhen`, `SharedFlow<T>::toList`, `SharedFlow<T>::toList`, `SharedFlow<T>::toSet`, `SharedFlow<T>::toSet`, `SharedFlow<T>::count` | 0/0 matched (target 2) | _none_ | - | 13 | 131310.0 |
| 5 | `internal.Concurrent.common` | `internal.Concurrent.common [ZERO]` | 0.00 | 0/1 matched (target 0) | `WorkaroundAtomicReference<T>::loop` | 0/3 matched (target 0) | `ReentrantLock`, `BenignDataRace`, `WorkaroundAtomicReference` | - | 4 | 40410.0 |
| 6 | `Exceptions.common` | `Exceptions [ZERO]` | 0.00 | 0/0 matched (target 15) | _none_ | 4/4 matched (target 7) | _none_ | - | 0 | 410.0 |
| 7 | `Runnable.common` | `Runnable [ZERO]` | 0.00 | 0/0 matched (target 3) | _none_ | 1/1 matched (target 2) | _none_ | - | 0 | 110.0 |
| 8 | `Waiter` | `Waiter [ZERO]` | 0.00 | 0/0 matched (target 1) | _none_ | 1/1 matched (target 2) | _none_ | - | 0 | 110.0 |
| 9 | `internal.NullSurrogate` | `internal.NullSurrogate [ZERO]` | 0.00 | 0/0 matched | _none_ | 0/0 matched | _none_ | - | 0 | 10.0 |
| 10 | `internal.ProbesSupport.common` | `internal.ProbesSupport.common [ZERO]` | 0.00 | 0/0 matched (target 2) | _none_ | 0/0 matched (target 1) | _none_ | - | 0 | 10.0 |
| 11 | `CoroutineStart` | `CoroutineStart` | 0.00 | 0/1 matched (target 4) | `CoroutineStart::invoke` | 1/1 matched (target 3) | _none_ | - | 1 | 1010210.0 |
| 12 | `flow.Migration` | `flow.Migration` | 0.00 | 1/36 matched (target 1) | `Flow<T>::observeOn`, `Flow<T>::publishOn`, `Flow<T>::subscribeOn`, `Flow<T>::onErrorResume`, `Flow<T>::onErrorResumeNext`, `Flow<T>::subscribe`, `Flow<T>::subscribe`, `Flow<T>::subscribe`, `Flow<T>::flatMap`, `Flow<T>::concatMap`, `Flow<Flow<T>>::merge`, `Flow<Flow<T>>::flatten`, `Flow<T>::compose`, `Flow<T>::skip`, `Flow<T>::forEach`, `Flow<T>::scanFold`, `Flow<T>::onErrorReturn`, `Flow<T>::onErrorReturn`, `Flow<T>::startWith`, `Flow<T>::startWith`, `Flow<T>::concatWith`, `Flow<T>::concatWith`, `Flow<T1>::combineLatest`, `Flow<T1>::combineLatest`, `Flow<T1>::combineLatest`, `Flow<T1>::combineLatest`, `Flow<T>::delayFlow`, `Flow<T>::delayEach`, `Flow<T>::switchMap`, `Flow<T>::scanReduce`, `Flow<T>::publish`, `Flow<T>::publish`, `Flow<T>::replay`, `Flow<T>::replay`, `Flow<T>::cache` | 0/0 matched | _none_ | - | 35 | 353610.0 |
| 13 | `Await` | `Await` | 0.01 | 1/9 matched (target 2) | `Collection<Deferred<T>>::awaitAll`, `joinAll`, `Collection<Job>::joinAll`, `AwaitAll::await`, `AwaitAll::DisposeHandlersOnCancel::disposeAll`, `AwaitAll::DisposeHandlersOnCancel::invoke`, `AwaitAll::DisposeHandlersOnCancel::toString`, `AwaitAll::AwaitAllNode::invoke` | 0/3 matched (target 2) | `AwaitAll`, `DisposeHandlersOnCancel`, `AwaitAllNode` | - | 11 | 111209.9 |
| 14 | `terminal.Logic` | `flow.Logic` | 0.02 | 3/3 matched (target 23) | _none_ | 0/0 matched (target 4) | _none_ | - | 0 | 309.8 |
| 15 | `internal.Combine` | `internal.Combine` | 0.02 | 1/2 matched (target 23) | `FlowCollector<R>::combineInternal` | 1/1 matched (target 9) | _none_ | - | 1 | 10309.8 |
| 16 | `CancellableContinuation` | `CancellableContinuation` | 0.02 | 1/7 matched (target 8) | `CancellableContinuation<T>::invokeOnCancellation`, `suspendCancellableCoroutine`, `suspendCancellableCoroutineReusable`, `getOrCreateCancellableContinuation`, `DisposeOnCancel::invoke`, `DisposeOnCancel::toString` | 1/2 matched (target 4) | `DisposeOnCancel` | - | 7 | 70909.8 |
| 17 | `internal.SystemProps.common` | `internal.SystemProps.common` | 0.02 | 1/4 matched (target 5) | `systemProp`, `systemProp`, `systemProp` | 0/0 matched | _none_ | - | 3 | 30409.8 |
| 18 | `terminal.Count` | `flow.Count` | 0.03 | 2/2 matched (target 26) | _none_ | 0/0 matched (target 2) | _none_ | - | 0 | 209.7 |
| 19 | `CoroutineExceptionHandler` | `CoroutineExceptionHandler` | 0.03 | 1/4 matched (target 2) | `handlerException`, `CoroutineExceptionHandler`, `handleException` | 1/1 matched | _none_ | - | 3 | 30509.7 |
| 20 | `terminal.Reduce` | `flow.Reduce` | 0.03 | 10/10 matched (target 64) | _none_ | 0/0 matched (target 6) | _none_ | - | 0 | 1009.7 |
| 21 | `flow.Builders` | `flow.FlowBuilders` | 0.03 | 9/23 matched (target 20) | `SafeFlow::collectSafely`, `Iterable<T>::asFlow`, `Iterator<T>::asFlow`, `Sequence<T>::asFlow`, `Array<T>::asFlow`, `IntArray::asFlow`, `LongArray::asFlow`, `IntRange::asFlow`, `LongRange::asFlow`, `ChannelFlowBuilder::create`, `ChannelFlowBuilder::collectTo`, `ChannelFlowBuilder::toString`, `CallbackFlowBuilder::collectTo`, `CallbackFlowBuilder::create` | 1/4 matched | `SafeFlow`, `ChannelFlowBuilder`, `CallbackFlowBuilder` | - | 17 | 172709.7 |
| 22 | `CoroutineScope` | `CoroutineScope` | 0.03 | 2/8 matched (target 12) | `CoroutineScope::plus`, `MainScope`, `coroutineScope`, `CoroutineScope`, `CoroutineScope::cancel`, `currentCoroutineContext` | 2/2 matched (target 3) | _none_ | - | 6 | 61009.7 |
| 23 | `selects.OnTimeout` | `selects.OnTimeout` | 0.03 | 2/3 matched (target 6) | `OnTimeout::register` | 1/1 matched | _none_ | - | 1 | 10409.7 |
| 24 | `operators.Distinct` | `flow.Distinct` | 0.04 | 4/5 matched (target 10) | `Flow<T>::distinctUntilChangedBy` | 1/1 matched (target 5) | _none_ | - | 1 | 10609.6 |
| 25 | `CoroutineName` | `CoroutineName` | 0.04 | 1/1 matched (target 6) | _none_ | 1/1 matched | _none_ | - | 0 | 209.6 |
| 26 | `selects.WhileSelect` | `selects.WhileSelect` | 0.04 | 1/1 matched | _none_ | 0/0 matched | _none_ | - | 0 | 109.6 |
| 27 | `operators.Limit` | `flow.Limit` | 0.05 | 8/8 matched (target 44) | _none_ | 0/0 matched (target 4) | _none_ | - | 0 | 809.5 |
| 28 | `channels.Channels.common` | `channels.Channels.common` | 0.05 | 4/6 matched (target 11) | `ReceiveChannel<E>::receiveOrNull`, `ReceiveChannel<E>::onReceiveOrNull` | 0/0 matched (target 1) | _none_ | - | 2 | 20609.5 |
| 29 | `Builders.common` | `Builders.common` | 0.05 | 8/14 matched (target 51) | `CoroutineDispatcher::invoke`, `DispatchedCoroutine::trySuspend`, `DispatchedCoroutine::tryResume`, `DispatchedCoroutine::afterCompletion`, `DispatchedCoroutine::afterResume`, `DispatchedCoroutine::getResult` | 4/6 matched (target 8) | `UndispatchedCoroutine`, `DispatchedCoroutine` | - | 8 | 82009.5 |
| 30 | `operators.Emitters` | `flow.Emitters` | 0.05 | 4/8 matched (target 7) | `Flow<T>::unsafeTransform`, `FlowCollector<*>::ensureActive`, `ThrowingCollector::emit`, `FlowCollector<T>::invokeSafely` | 0/1 matched (target 0) | `ThrowingCollector` | - | 5 | 50909.5 |
| 31 | `operators.Zip` | `flow.Zip` | 0.06 | 9/18 matched (target 11) | `combine`, `combineTransform`, `combine`, `combineTransform`, `combineUnsafe`, `combineTransformUnsafe`, `nullArrayFactory`, `combine`, `combineTransform` | 0/0 matched | _none_ | - | 9 | 91809.4 |
| 32 | `internal.MainDispatcherFactory` | `internal.MainDispatcherFactory` | 0.07 | 1/1 matched | _none_ | 1/1 matched (target 2) | _none_ | - | 0 | 209.3 |
| 33 | `operators.Errors` | `flow.Errors` | 0.07 | 4/6 matched (target 13) | `Throwable::isCancellationCause`, `Throwable::isSameExceptionAs` | 0/0 matched (target 1) | _none_ | - | 2 | 20609.3 |
| 34 | `operators.Transform` | `flow.Transform` | 0.07 | 12/13 matched (target 100) | `Flow<*>::filterIsInstance` | 0/0 matched (target 13) | _none_ | - | 1 | 11309.3 |
| 35 | `MainCoroutineDispatcher` | `MainCoroutineDispatcher` | 0.08 | 3/3 matched | _none_ | 1/1 matched | _none_ | - | 0 | 409.2 |
| 36 | `internal.NopCollector` | `internal.NopCollector` | 0.08 | 1/1 matched | _none_ | 1/1 matched | _none_ | - | 0 | 209.2 |
| 37 | `operators.Merge` | `flow.Merge` | 0.08 | 8/9 matched (target 23) | `Iterable<Flow<T>>::merge` | 0/0 matched (target 3) | _none_ | - | 1 | 10909.2 |
| 38 | `operators.Context` | `flow.Context` | 0.09 | 6/7 matched | `Flow<T>::buffer` | 1/2 matched (target 1) | `CancellableFlow` | - | 2 | 20909.1 |
| 39 | `operators.Delay` | `flow.Delay` | 0.09 | 6/10 matched (target 9) | `Flow<T>::debounce`, `Flow<T>::debounce`, `Flow<T>::sample`, `Flow<T>::timeoutInternal` | 0/0 matched (target 1) | _none_ | - | 4 | 41009.1 |
| 40 | `CompletionState` | `CompletionState` | 0.10 | 4/6 matched | `CompletedExceptionally::toString`, `CancelledContinuation::makeResumed` | 1/2 matched | `CancelledContinuation` | - | 3 | 30809.0 |
| 41 | `Job` | `Job` | 0.10 | 5/24 matched (target 18) | `Job::cancel`, `Job::plus`, `Job::invokeOnCompletion`, `Job`, `Job0`, `Job::disposeOnCompletion`, `Job::cancelAndJoin`, `Job::cancelChildren`, `Job::cancelChildren`, `CoroutineContext::cancel`, `CoroutineContext::cancel`, `CoroutineContext::ensureActive`, `Job::cancel`, `CoroutineContext::cancel`, `CoroutineContext::cancelChildren`, `CoroutineContext::cancelChildren`, `CoroutineContext::cancelChildren`, `orCancellation`, `DisposeOnCompletion::invoke` | 5/7 matched (target 9) | `DisposableHandle`, `DisposeOnCompletion` | - | 21 | 213109.0 |
| 42 | `internal.InlineList` | `internal.InlineList` | 0.10 | 1/2 matched (target 5) | `InlineList::plus` | 1/1 matched | _none_ | - | 1 | 10309.0 |
| 43 | `internal.Symbol` | `internal.Symbol` | 0.11 | 2/2 matched (target 3) | _none_ | 1/1 matched | _none_ | - | 0 | 1000308.9 |
| 44 | `terminal.Collection` | `flow.Collection` | 0.11 | 3/3 matched (target 51) | _none_ | 0/0 matched (target 9) | _none_ | - | 0 | 308.9 |
| 45 | `internal.DispatchedTask` | `common.DispatchedTaskDispatch` | 0.12 | 6/10 matched (target 6) | `DispatchedTask::cancelCompletedResult`, `DispatchedTask::getSuccessfulResult`, `DispatchedTask::getExceptionalResult`, `DispatchedTask<*>::runUnconfinedEventLoop` | 0/2 matched (target 0) | `DispatchedTask`, `DispatchException` | - | 6 | 61208.8 |
| 46 | `internal.ConcurrentLinkedList` | `internal.ConcurrentLinkedList` | 0.12 | 9/13 matched (target 27) | `AtomicRef<S>::moveForward`, `AtomicRef<S>::findSegmentAndMoveForward`, `N::close`, `AtomicInt::addConditionally` | 3/3 matched (target 5) | _none_ | - | 4 | 41608.8 |
| 47 | `terminal.Collect` | `flow.Collect` | 0.13 | 8/8 matched (target 25) | _none_ | 0/0 matched (target 4) | _none_ | - | 0 | 808.7 |
| 48 | `JobSupport` | `JobSupport` | 0.13 | 54/91 matched (target 152) | `JobSupport::loopOnState`, `JobSupport::finalizeFinishingState`, `JobSupport::getFinalRootCause`, `JobSupport::addSuppressedExceptions`, `JobSupport::tryFinalizeSimpleState`, `JobSupport::completeStateFinalization`, `JobSupport::notifyCancelling`, `JobSupport::cancelParent`, `JobSupport::notifyCompletion`, `JobSupport::notifyHandlers`, `JobSupport::startInternal`, `JobSupport::tryPutNodeIntoList`, `JobSupport::promoteEmptyToNodeList`, `JobSupport::promoteSingleToNodeList`, `JobSupport::SelectOnJoinCompletionHandler::invoke`, `JobSupport::removeNode`, `JobSupport::cancel`, `JobSupport::cancelMakeCompleting`, `JobSupport::createCauseException`, `JobSupport::makeCancelling`, `JobSupport::getOrPromoteCancellingList`, `JobSupport::tryMakeCancelling`, `JobSupport::tryMakeCompleting`, `JobSupport::tryMakeCompletingSlowPath`, `JobSupport::tryWaitForChild`, `JobSupport::continueCompleting`, `JobSupport::nextChild`, `JobSupport::Finishing::toString`, `JobSupport::AwaitContinuation::getContinuationCancellationCause`, `JobSupport::AwaitContinuation::nameString`, `JobSupport::SelectOnAwaitCompletionHandler::invoke`, `Empty::toString`, `JobImpl::complete`, `JobImpl::completeExceptionally`, `JobImpl::handlesException`, `InactiveNodeList::toString`, `ResumeAwaitOnCompletion::invoke` | 16/18 matched (target 21) | `JobImpl`, `ResumeAwaitOnCompletion` | - | 39 | 400908.7 |
| 49 | `Timeout` | `Timeout` | 0.13 | 9/9 matched (target 13) | _none_ | 2/2 matched (target 3) | _none_ | - | 0 | 1108.7 |
| 50 | `channels.Broadcast` | `channels.Broadcast` | 0.14 | 3/10 matched (target 11) | `ReceiveChannel<E>::broadcast`, `CoroutineScope::broadcast`, `BroadcastCoroutine::cancel`, `BroadcastCoroutine::cancel`, `BroadcastCoroutine::cancelInternal`, `LazyBroadcastCoroutine::openSubscription`, `LazyBroadcastCoroutine::onStart` | 2/2 matched | _none_ | - | 7 | 71208.6 |
| 51 | `channels.BroadcastChannel` | `channels.BroadcastChannel` | 0.15 | 10/11 matched (target 35) | `BroadcastChannel` | 5/5 matched (target 6) | _none_ | - | 1 | 11608.5 |
| 52 | `sync.Semaphore` | `sync.Semaphore` | 0.15 | 8/21 matched (target 20) | `Semaphore`, `SemaphoreAndMutexImpl::tryAcquire`, `SemaphoreAndMutexImpl::acquire`, `SemaphoreAndMutexImpl::acquireSlowPath`, `SemaphoreAndMutexImpl::acquire`, `SemaphoreAndMutexImpl::acquire`, `SemaphoreAndMutexImpl::onAcquireRegFunction`, `SemaphoreAndMutexImpl::decPermits`, `SemaphoreAndMutexImpl::release`, `SemaphoreAndMutexImpl::coerceAvailablePermitsAtMaximum`, `SemaphoreAndMutexImpl::addAcquireToQueue`, `SemaphoreAndMutexImpl::tryResumeNextFromQueue`, `SemaphoreAndMutexImpl::tryResumeAcquire` | 3/4 matched (target 3) | `SemaphoreAndMutexImpl` | - | 14 | 142508.5 |
| 53 | `intrinsics.Cancellable` | `intrinsics.Cancellable` | 0.16 | 5/5 matched (target 19) | _none_ | 0/0 matched (target 2) | _none_ | - | 0 | 508.4 |
| 54 | `CompletableDeferred` | `CompletableDeferred` | 0.16 | 5/7 matched (target 15) | `CompletableDeferred`, `CompletableDeferred` | 2/2 matched (target 3) | _none_ | - | 2 | 20908.4 |
| 55 | `internal.SendingCollector` | `internal.SendingCollector` | 0.16 | 1/1 matched (target 2) | _none_ | 1/1 matched | _none_ | - | 0 | 208.4 |
| 56 | `selects.SelectUnbiased` | `selects.SelectUnbiased` | 0.16 | 6/6 matched (target 8) | _none_ | 1/1 matched (target 2) | _none_ | - | 0 | 708.4 |
| 57 | `Guidance` | `Guidance` | 0.17 | 2/2 matched | _none_ | 0/0 matched (target 5) | _none_ | - | 0 | 208.3 |
| 58 | `Yield` | `Yield` | 0.17 | 1/1 matched (target 3) | _none_ | 0/0 matched | _none_ | - | 0 | 108.3 |
| 59 | `channels.Produce` | `channels.Produce` | 0.18 | 4/6 matched (target 14) | `CoroutineScope::produce`, `CoroutineScope::produce` | 1/2 matched | `ProducerScope` | - | 3 | 30808.2 |
| 60 | `operators.Share` | `flow.Share` | 0.18 | 12/13 matched (target 49) | `SubscribedFlowCollector::onSubscription` | 4/5 matched (target 10) | `SubscribedFlowCollector` | - | 2 | 21808.2 |
| 61 | `internal.FlowCoroutine` | `internal.FlowCoroutine` | 0.19 | 3/3 matched (target 6) | _none_ | 1/1 matched (target 2) | _none_ | - | 0 | 408.1 |
| 62 | `internal.ChannelFlow` | `internal.ChannelFlow` | 0.19 | 14/19 matched (target 43) | `ChannelFlowOperator::collectWithContextUndispatched`, `ChannelFlowOperator::toString`, `FlowCollector<T>::withUndispatchedContextCollector`, `StackFrameContinuation::resumeWith`, `StackFrameContinuation::getStackTraceElement` | 5/6 matched (target 9) | `StackFrameContinuation` | - | 6 | 62508.1 |
| 63 | `selects.Select` | `selects.Select` | 0.21 | 24/30 matched (target 71) | `SelectBuilder::invoke`, `SelectBuilder::onTimeout`, `SelectImplementation::register`, `SelectImplementation::processResultAndInvokeBlockRecoveringException`, `CancellableContinuation<Unit>::tryResume`, `TrySelectDetailedResult` | 16/16 matched (target 21) | _none_ | - | 6 | 64607.9 |
| 64 | `selects.SelectOld` | `selects.SelectOld` | 0.22 | 8/8 matched (target 10) | _none_ | 2/2 matched | _none_ | - | 0 | 1007.8 |
| 65 | `CancellableContinuationImpl` | `CancellableContinuationImpl` | 0.23 | 37/50 matched (target 129) | `CancellableContinuationImpl::getStackTraceElement`, `CancellableContinuationImpl::callCancelHandlerSafely`, `CancellableContinuationImpl::invokeOnCancellationInternal`, `CancellableContinuationImpl::multipleHandlersError`, `CancellableContinuationImpl::tryResumeImpl`, `CancellableContinuationImpl::alreadyResumedError`, `CancellableContinuationImpl::getExceptionalResult`, `CancellableContinuationImpl::toString`, `CancellableContinuationImpl::nameString`, `Active::toString`, `CancelHandler::UserSupplied::invoke`, `CancelHandler::UserSupplied::toString`, `CompletedContinuation::invokeHandlers` | 3/7 matched (target 11) | `NotCompleted`, `Active`, `UserSupplied`, `CompletedContinuation` | - | 17 | 175707.8 |
| 66 | `flow.Channels` | `flow.Channels` | 0.23 | 12/12 matched (target 22) | _none_ | 1/1 matched (target 2) | _none_ | - | 0 | 14001308.0 |
| 67 | `sync.Mutex` | `sync.Mutex` | 0.23 | 11/16 matched (target 17) | `Mutex`, `MutexImpl::CancellableContinuationWithOwner::tryResume`, `MutexImpl::CancellableContinuationWithOwner::resume`, `MutexImpl::SelectInstanceWithOwner::trySelect`, `MutexImpl::SelectInstanceWithOwner::selectInRegistrationPhase` | 2/4 matched (target 2) | `CancellableContinuationWithOwner`, `SelectInstanceWithOwner` | - | 7 | 72007.7 |
| 68 | `Supervisor` | `Supervisor` | 0.24 | 5/5 matched (target 8) | _none_ | 2/2 matched | _none_ | - | 0 | 707.6 |
| 69 | `internal.Synchronized.common` | `internal.SynchronizedObject` | 0.24 | 1/1 matched (target 6) | _none_ | 1/1 matched | _none_ | - | 0 | 207.6 |
| 70 | `flow.StateFlow` | `flow.StateFlow` | 0.25 | 17/19 matched (target 44) | `MutableStateFlow`, `StateFlowImpl::fuse` | 4/4 matched (target 7) | _none_ | - | 2 | 22307.5 |
| 71 | `Delay` | `Delay` | 0.25 | 5/6 matched (target 13) | `Duration::toDelayMillis` | 2/2 matched | _none_ | - | 1 | 10807.5 |
| 72 | `channels.BufferedChannel` | `channels.BufferedChannel` | 0.26 | 100/111 matched (target 161) | `BufferedChannel::sendImpl`, `BufferedChannel::receiveImpl`, `BufferedChannel::cancel`, `BufferedChannel::cancel`, `BufferedChannel::invokeCloseHandler`, `BufferedChannel::toStringDebug`, `BufferedChannel::checkSegmentStructureInvariants`, `BufferedChannel::onCancellationChannelResultImplDoNotCall`, `BufferedChannel::onCancellationImplDoNotCall`, `createSegmentFunction`, `CancellableContinuation<T>::tryResume0` | 6/6 matched (target 7) | _none_ | - | 11 | 121707.4 |
| 73 | `internal.Merge` | `internal.Merge` | 0.26 | 9/9 matched (target 31) | _none_ | 3/3 matched (target 9) | _none_ | - | 0 | 1207.4 |
| 74 | `internal.LimitedDispatcher` | `internal.LimitedDispatcher` | 0.27 | 9/10 matched (target 12) | `CoroutineDispatcher::namedOrThis` | 2/2 matched (target 3) | _none_ | - | 1 | 11207.3 |
| 75 | `channels.ChannelCoroutine` | `channels.ChannelCoroutine` | 0.29 | 2/4 matched (target 15) | `ChannelCoroutine::cancel`, `ChannelCoroutine::cancel` | 1/1 matched | _none_ | - | 2 | 20507.1 |
| 76 | `internal.OnUndeliveredElement` | `internal.OnUndeliveredElement` | 0.32 | 2/2 matched (target 3) | _none_ | 2/2 matched | _none_ | - | 0 | 406.8 |
| 77 | `flow.SharingStarted` | `flow.SharingStarted` | 0.32 | 7/10 matched (target 20) | `SharingStarted.Companion::WhileSubscribed`, `StartedWhileSubscribed::equals`, `StartedWhileSubscribed::hashCode` | 5/5 matched (target 7) | _none_ | - | 3 | 31506.8 |
| 78 | `internal.LockFreeTaskQueue` | `internal.LockFreeTaskQueue` | 0.33 | 19/21 matched (target 27) | `LockFreeTaskQueueCore::wo`, `LockFreeTaskQueueCore::withState` | 4/4 matched | _none_ | - | 2 | 22506.7 |
| 79 | `NonCancellable` | `NonCancellable` | 0.34 | 8/9 matched (target 18) | `NonCancellable::cancel` | 1/1 matched | _none_ | - | 1 | 11006.6 |
| 80 | `flow.SharedFlow` | `flow.SharedFlow` | 0.35 | 26/31 matched (target 47) | `MutableSharedFlow`, `SharedFlowImpl::fuse`, `Array<Any?>::getBufferAt`, `Array<Any?>::setBufferAt`, `SharedFlow<T>::fuseSharedFlow` | 5/5 matched (target 8) | _none_ | - | 5 | 53606.5 |
| 81 | `channels.ConflatedBufferedChannel` | `channels.ConflatedBufferedChannel` | 0.37 | 6/7 matched (target 9) | `ConflatedBufferedChannel::registerSelectForSend` | 1/1 matched | _none_ | - | 1 | 10806.3 |
| 82 | `AbstractCoroutine` | `AbstractCoroutine` | 0.38 | 9/9 matched (target 16) | _none_ | 1/1 matched | _none_ | - | 0 | 1006.2 |
| 83 | `internal.ThreadSafeHeap` | `internal.ThreadSafeHeap` | 0.38 | 14/14 matched (target 20) | _none_ | 2/2 matched | _none_ | - | 0 | 1606.2 |
| 84 | `internal.NamedDispatcher` | `internal.NamedDispatcher` | 0.39 | 2/4 matched | `NamedDispatcher::isDispatchNeeded`, `NamedDispatcher::dispatchYield` | 1/1 matched | _none_ | - | 2 | 20506.1 |
| 85 | `CoroutineDispatcher` | `CoroutineDispatcher` | 0.39 | 7/8 matched (target 9) | `CoroutineDispatcher::limitedParallelism` | 1/1 matched | _none_ | - | 1 | 10906.1 |
| 86 | `Unconfined` | `Unconfined` | 0.47 | 4/4 matched (target 6) | _none_ | 2/2 matched (target 3) | _none_ | - | 0 | 605.3 |
| 87 | `channels.BufferOverflow` | `channels.BufferOverflow` | 1.00 | 0/0 matched | _none_ | 1/1 matched | _none_ | - | 0 | 2000100.0 |
| 88 | `internal.LocalAtomics.common` | `internal.LocalAtomics.common` | 1.00 | 0/0 matched | _none_ | 0/1 matched (target 0) | `LocalAtomicInt` | - | 1 | 10100.0 |
| 89 | `Annotations` | `Annotations` | 1.00 | 0/0 matched | _none_ | 1/1 matched (target 7) | _none_ | - | 0 | 100.0 |
| 90 | `CloseableCoroutineDispatcher` | `CloseableCoroutineDispatcher` | 1.00 | 0/0 matched | _none_ | 1/1 matched | _none_ | - | 0 | 100.0 |
| 91 | `CompletableJob` | `CompletableJob` | 1.00 | 0/0 matched | _none_ | 1/1 matched | _none_ | - | 0 | 100.0 |
| 92 | `CompletionHandler.common` | `CompletionHandler` | 1.00 | 0/0 matched | _none_ | 1/1 matched | _none_ | - | 0 | 100.0 |
| 93 | `Debug.common` | `Debug.common` | 1.00 | 0/0 matched | _none_ | 1/1 matched | _none_ | - | 0 | 100.0 |
| 94 | `Deferred` | `Deferred` | 1.00 | 0/0 matched | _none_ | 1/1 matched | _none_ | - | 0 | 100.0 |
| 95 | `Dispatchers.common` | `Dispatchers` | 1.00 | 0/0 matched | _none_ | 1/1 matched (target 2) | _none_ | - | 0 | 100.0 |
| 96 | `flow.FlowCollector` | `flow.FlowCollector` | 1.00 | 0/0 matched | _none_ | 1/1 matched | _none_ | - | 0 | 100.0 |

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

### Critical Ports (Similarity < 0.60, Worst First)

These files need significant work:

- `flow.Flow` -> `flow.Flow [ZERO]` (0.00, 13 deps)
- `EventLoop.common` -> `native.EventLoop [ZERO]` (0.00)
- `channels.Deprecated` -> `channels.Deprecated [ZERO]` (0.00)
- `operators.Lint` -> `flow.Lint [ZERO]` (0.00)
- `internal.Concurrent.common` -> `internal.Concurrent.common [ZERO]` (0.00)
- `Exceptions.common` -> `Exceptions [ZERO]` (0.00)
- `Runnable.common` -> `Runnable [ZERO]` (0.00)
- `Waiter` -> `Waiter [ZERO]` (0.00)
- `internal.NullSurrogate` -> `internal.NullSurrogate [ZERO]` (0.00)
- `internal.ProbesSupport.common` -> `internal.ProbesSupport.common [ZERO]` (0.00)
- `CoroutineStart` -> `CoroutineStart` (0.00, 1 deps)
- `flow.Migration` -> `flow.Migration` (0.00)
- `Await` -> `Await` (0.01)
- `terminal.Logic` -> `flow.Logic` (0.02)
- `internal.Combine` -> `internal.Combine` (0.02)
- `CancellableContinuation` -> `CancellableContinuation` (0.02)
- `internal.SystemProps.common` -> `internal.SystemProps.common` (0.02)
- `terminal.Count` -> `flow.Count` (0.03)
- `CoroutineExceptionHandler` -> `CoroutineExceptionHandler` (0.03)
- `terminal.Reduce` -> `flow.Reduce` (0.03)
- `flow.Builders` -> `flow.FlowBuilders` (0.03)
- `CoroutineScope` -> `CoroutineScope` (0.03)
- `selects.OnTimeout` -> `selects.OnTimeout` (0.03)
- `operators.Distinct` -> `flow.Distinct` (0.04)
- `CoroutineName` -> `CoroutineName` (0.04)
- `selects.WhileSelect` -> `selects.WhileSelect` (0.04)
- `operators.Limit` -> `flow.Limit` (0.05)
- `channels.Channels.common` -> `channels.Channels.common` (0.05)
- `Builders.common` -> `Builders.common` (0.05)
- `operators.Emitters` -> `flow.Emitters` (0.05)
- `operators.Zip` -> `flow.Zip` (0.06)
- `internal.MainDispatcherFactory` -> `internal.MainDispatcherFactory` (0.07)
- `operators.Errors` -> `flow.Errors` (0.07)
- `operators.Transform` -> `flow.Transform` (0.07)
- `MainCoroutineDispatcher` -> `MainCoroutineDispatcher` (0.08)
- `internal.NopCollector` -> `internal.NopCollector` (0.08)
- `operators.Merge` -> `flow.Merge` (0.08)
- `operators.Context` -> `flow.Context` (0.09)
- `operators.Delay` -> `flow.Delay` (0.09)
- `CompletionState` -> `CompletionState` (0.10)
- `Job` -> `Job` (0.10)
- `internal.InlineList` -> `internal.InlineList` (0.10)
- `internal.Symbol` -> `internal.Symbol` (0.11, 1 deps)
- `terminal.Collection` -> `flow.Collection` (0.11)
- `internal.DispatchedTask` -> `common.DispatchedTaskDispatch` (0.12)
- `internal.ConcurrentLinkedList` -> `internal.ConcurrentLinkedList` (0.12)
- `terminal.Collect` -> `flow.Collect` (0.13)
- `JobSupport` -> `JobSupport` (0.13)
- `Timeout` -> `Timeout` (0.13)
- `channels.Broadcast` -> `channels.Broadcast` (0.14)
- `channels.BroadcastChannel` -> `channels.BroadcastChannel` (0.15)
- `sync.Semaphore` -> `sync.Semaphore` (0.15)
- `intrinsics.Cancellable` -> `intrinsics.Cancellable` (0.16)
- `CompletableDeferred` -> `CompletableDeferred` (0.16)
- `internal.SendingCollector` -> `internal.SendingCollector` (0.16)
- `selects.SelectUnbiased` -> `selects.SelectUnbiased` (0.16)
- `Guidance` -> `Guidance` (0.17)
- `Yield` -> `Yield` (0.17)
- `channels.Produce` -> `channels.Produce` (0.18)
- `operators.Share` -> `flow.Share` (0.18)
- `internal.FlowCoroutine` -> `internal.FlowCoroutine` (0.19)
- `internal.ChannelFlow` -> `internal.ChannelFlow` (0.19)
- `selects.Select` -> `selects.Select` (0.21)
- `selects.SelectOld` -> `selects.SelectOld` (0.22)
- `CancellableContinuationImpl` -> `CancellableContinuationImpl` (0.23)
- `flow.Channels` -> `flow.Channels` (0.23, 14 deps)
- `sync.Mutex` -> `sync.Mutex` (0.23)
- `Supervisor` -> `Supervisor` (0.24)
- `internal.Synchronized.common` -> `internal.SynchronizedObject` (0.24)
- `flow.StateFlow` -> `flow.StateFlow` (0.25)
- `Delay` -> `Delay` (0.25)
- `channels.BufferedChannel` -> `channels.BufferedChannel` (0.26)
- `internal.Merge` -> `internal.Merge` (0.26)
- `internal.LimitedDispatcher` -> `internal.LimitedDispatcher` (0.27)
- `channels.ChannelCoroutine` -> `channels.ChannelCoroutine` (0.29)
- `internal.OnUndeliveredElement` -> `internal.OnUndeliveredElement` (0.32)
- `flow.SharingStarted` -> `flow.SharingStarted` (0.32)
- `internal.LockFreeTaskQueue` -> `internal.LockFreeTaskQueue` (0.33)
- `NonCancellable` -> `NonCancellable` (0.34)
- `flow.SharedFlow` -> `flow.SharedFlow` (0.35)
- `channels.ConflatedBufferedChannel` -> `channels.ConflatedBufferedChannel` (0.37)
- `AbstractCoroutine` -> `AbstractCoroutine` (0.38)
- `internal.ThreadSafeHeap` -> `internal.ThreadSafeHeap` (0.38)
- `internal.NamedDispatcher` -> `internal.NamedDispatcher` (0.39)
- `CoroutineDispatcher` -> `CoroutineDispatcher` (0.39)
- `Unconfined` -> `Unconfined` (0.47)

## Incorrect Ports (Missing Types)

These files are matched (often via `// port-lint`) but appear to be missing one or more type declarations
present in the Rust source file.

| Source | Target | Missing types | Examples |
|--------|--------|---------------|----------|
| `EventLoop.common` | `native.EventLoop [ZERO]` | 10/10 | `EventLoop`, `ThreadLocalEventLoop`, `Queue`, `EventLoopImplPlatform`, `EventLoopImplBase`, `DelayedTask`, `DelayedResumeTask`, `DelayedRunnableTask`, `DelayedTaskQueue`, `DefaultExecutor` |
| `JobSupport` | `JobSupport` | 2/18 | `JobImpl`, `ResumeAwaitOnCompletion` |
| `Job` | `Job` | 2/7 | `DisposableHandle`, `DisposeOnCompletion` |
| `CancellableContinuationImpl` | `CancellableContinuationImpl` | 4/7 | `NotCompleted`, `Active`, `UserSupplied`, `CompletedContinuation` |
| `flow.Builders` | `flow.FlowBuilders` | 3/4 | `SafeFlow`, `ChannelFlowBuilder`, `CallbackFlowBuilder` |
| `sync.Semaphore` | `sync.Semaphore` | 1/4 | `SemaphoreAndMutexImpl` |
| `Await` | `Await` | 3/3 | `AwaitAll`, `DisposeHandlersOnCancel`, `AwaitAllNode` |
| `Builders.common` | `Builders.common` | 2/6 | `UndispatchedCoroutine`, `DispatchedCoroutine` |
| `sync.Mutex` | `sync.Mutex` | 2/4 | `CancellableContinuationWithOwner`, `SelectInstanceWithOwner` |
| `CancellableContinuation` | `CancellableContinuation` | 1/2 | `DisposeOnCancel` |
| `internal.ChannelFlow` | `internal.ChannelFlow` | 1/6 | `StackFrameContinuation` |
| `internal.DispatchedTask` | `common.DispatchedTaskDispatch` | 2/2 | `DispatchedTask`, `DispatchException` |
| `operators.Emitters` | `flow.Emitters` | 1/1 | `ThrowingCollector` |
| `internal.Concurrent.common` | `internal.Concurrent.common [ZERO]` | 3/3 | `ReentrantLock`, `BenignDataRace`, `WorkaroundAtomicReference` |
| `CompletionState` | `CompletionState` | 1/2 | `CancelledContinuation` |
| `channels.Produce` | `channels.Produce` | 1/2 | `ProducerScope` |
| `operators.Share` | `flow.Share` | 1/5 | `SubscribedFlowCollector` |
| `operators.Context` | `flow.Context` | 1/2 | `CancellableFlow` |
| `internal.LocalAtomics.common` | `internal.LocalAtomics.common` | 1/1 | `LocalAtomicInt` |

## High Priority Missing Files

| Rank | Source file | Expected target | Deps | Functions | Classes/types | Symbols | Source path | Expected path |
|------|-------------|-----------------|------|-----------|---------------|---------|-------------|---------------|
| 1 | `channels.Channel` | `channels.Channel` | 0 | 22 | 9 | 31 | `channels/Channel.kt` | `channels/Channel.cpp` |
| 2 | `internal.DispatchedContinuation` | `internal.DispatchedContinuation` | 0 | 19 | 1 | 20 | `internal/DispatchedContinuation.kt` | `internal/DispatchedContinuation.cpp` |
| 3 | `internal.AbstractSharedFlow` | `flow.internal.AbstractSharedFlow` | 0 | 4 | 3 | 7 | `flow/internal/AbstractSharedFlow.kt` | `flow/internal/AbstractSharedFlow.cpp` |
| 4 | `internal.Scopes` | `internal.Scopes` | 0 | 5 | 2 | 7 | `internal/Scopes.kt` | `internal/Scopes.cpp` |
| 5 | `intrinsics.Undispatched` | `intrinsics.Undispatched` | 0 | 6 | 0 | 6 | `intrinsics/Undispatched.kt` | `intrinsics/Undispatched.cpp` |
| 6 | `internal.SafeCollector.common` | `flow.internal.SafeCollector.common` | 0 | 4 | 1 | 5 | `flow/internal/SafeCollector.common.kt` | `flow/internal/SafeCollector.common.cpp` |
| 7 | `internal.FlowExceptions.common` | `flow.internal.FlowExceptions.common` | 0 | 2 | 2 | 4 | `flow/internal/FlowExceptions.common.kt` | `flow/internal/FlowExceptions.common.cpp` |
| 8 | `internal.CoroutineExceptionHandlerImpl.common` | `internal.CoroutineExceptionHandlerImpl.common` | 0 | 1 | 2 | 3 | `internal/CoroutineExceptionHandlerImpl.common.kt` | `internal/CoroutineExceptionHandlerImpl.common.cpp` |
| 9 | `internal.LockFreeLinkedList.common` | `internal.LockFreeLinkedList.common` | 0 | 0 | 2 | 2 | `internal/LockFreeLinkedList.common.kt` | `internal/LockFreeLinkedList.common.cpp` |
| 10 | `internal.StackTraceRecovery.common` | `internal.StackTraceRecovery.common` | 0 | 0 | 2 | 2 | `internal/StackTraceRecovery.common.kt` | `internal/StackTraceRecovery.common.cpp` |
| 11 | `SchedulerTask.common` | `SchedulerTask.common` | 0 | 0 | 1 | 1 | `SchedulerTask.common.kt` | `SchedulerTask.common.cpp` |
| 12 | `internal.InternalAnnotations.common` | `internal.InternalAnnotations.common` | 0 | 0 | 1 | 1 | `internal/InternalAnnotations.common.kt` | `internal/InternalAnnotations.common.cpp` |
| 13 | `internal.ThreadLocal.common` | `internal.ThreadLocal.common` | 0 | 0 | 1 | 1 | `internal/ThreadLocal.common.kt` | `internal/ThreadLocal.common.cpp` |
| 14 | `CoroutineContext.common` | `CoroutineContext.common` | 0 | 0 | 0 | 0 | `CoroutineContext.common.kt` | `CoroutineContext.common.cpp` |
| 15 | `internal.ThreadContext.common` | `internal.ThreadContext.common` | 0 | 0 | 0 | 0 | `internal/ThreadContext.common.kt` | `internal/ThreadContext.common.cpp` |

## Documentation Gaps

**Documentation line amount:** 7925 / 7936 lines (100%)

Documentation gaps (>20%), complete list:

- `operators.Share` - 97% gap (204 → 6 lines)
- `CancellableContinuation` - 41% gap (377 → 223 lines)
- `flow.Migration` - 52% gap (216 → 103 lines)
- `selects.Select` - 28% gap (402 → 291 lines)
- `operators.Delay` - 45% gap (236 → 130 lines)
- `Job` - 22% gap (477 → 372 lines)
- `flow.StateFlow` - 51% gap (196 → 96 lines)
- `flow.SharedFlow` - 40% gap (245 → 148 lines)
- `EventLoop.common` - 93% gap (99 → 7 lines)
- `channels.Produce` - 38% gap (226 → 139 lines)
- `CoroutineScope` - 27% gap (320 → 235 lines)
- `flow.Builders` - 37% gap (201 → 127 lines)
- `operators.Merge` - 46% gap (142 → 77 lines)
- `CoroutineExceptionHandler` - 88% gap (67 → 8 lines)
- `CoroutineDispatcher` - 27% gap (213 → 155 lines)
- `internal.DispatchedTask` - 88% gap (66 → 8 lines)
- `operators.Emitters` - 54% gap (105 → 48 lines)
- `channels.Deprecated` - 100% gap (48 → 0 lines)
- `operators.Zip` - 33% gap (133 → 89 lines)
- `operators.Lint` - 71% gap (55 → 16 lines)
- `operators.Errors` - 30% gap (101 → 71 lines)
- `Deferred` - 25% gap (84 → 63 lines)
- `Builders.common` - 28% gap (75 → 54 lines)
- `flow.Channels` - 26% gap (74 → 55 lines)
- `Dispatchers.common` - 31% gap (62 → 43 lines)
- `CloseableCoroutineDispatcher` - 53% gap (17 → 8 lines)
- `internal.ChannelFlow` - 23% gap (35 → 27 lines)
- `channels.Broadcast` - 100% gap (6 → 0 lines)

