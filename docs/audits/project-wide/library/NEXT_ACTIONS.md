# Immediate Actions - High-Value Files

Based on AST analysis, here are the concrete next steps.

## Summary

- **Files Present:** 244/354 (68.9%)
- **Function parity:** 782/2918 matched (target 2863) — 26.8%
- **Class/type parity:** 341/560 matched (target 490) — 60.9%
- **Combined symbol parity:** 1123/3478 matched (target 3353) — 32.3%
- **Average inline-code cosine:** 0.26 (function body across 135 matched files)
- **Average documentation cosine:** 0.37 (doc text across 135 matched files)
- **Cheat-zeroed Files:** 122
- **Critical Issues:** 226 files with <0.60 function similarity
- **Needs Review:** 9 files with 0.60-0.84 function similarity
- **Excellent:** 9 files with >=0.85 function similarity

## Priority 1: Fix Incomplete High-Dependency Files

### 1. flow.Channels
- **Similarity:** 0.22 (needs 63% improvement)
- **Dependencies:** 65
- **Priority Score:** 65001308.0
- **Functions:** 12/12 matched (target 24)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_
- **Action:** Deep review - likely missing major functionality

### 2. flow.Flow
- **Similarity:** 0.04 (needs 81% improvement)
- **Dependencies:** 28
- **Priority Score:** 28000310.0
- **Functions:** 1/1 matched (target 4)
- **Missing functions:** _none_
- **Types:** 2/2 matched (target 5)
- **Missing types:** _none_
- **Action:** Deep review - likely missing major functionality

### 3. internal.Concurrent
- **Similarity:** 0.36 (needs 49% improvement)
- **Dependencies:** 14
- **Priority Score:** 14010906.0
- **Functions:** 6/6 matched (target 11)
- **Missing functions:** _none_
- **Types:** 2/3 matched (target 2)
- **Missing types:** `BenignDataRace`
- **Symbol Deficit:** 1 (functions: 0, types: 1)
- **Action:** Deep review - likely missing major functionality

## Priority 2: Port Missing High-Value Files

Critical missing files (>10 dependencies):

No missing high-value files detected.

## Detailed Work Items

Every matched file is listed below with function and type symbol parity.

### 1. flow.Channels

- **Target:** `flow.Channels`
- **Similarity:** 0.22
- **Dependents:** 65
- **Priority Score:** 65001308.0
- **Functions:** 12/12 matched (target 24)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_
- **Lint issues:** 2

### 2. flow.Flow

- **Target:** `flow.Flow`
- **Similarity:** 0.04
- **Dependents:** 28
- **Priority Score:** 28000310.0
- **Functions:** 1/1 matched (target 4)
- **Missing functions:** _none_
- **Types:** 2/2 matched (target 5)
- **Missing types:** _none_
- **Lint issues:** 2

### 3. internal.Concurrent

- **Target:** `internal.Concurrent`
- **Similarity:** 0.36
- **Dependents:** 14
- **Priority Score:** 14010906.0
- **Functions:** 6/6 matched (target 11)
- **Missing functions:** _none_
- **Types:** 2/3 matched (target 2)
- **Missing types:** `BenignDataRace`
- **Lint issues:** 2

### 4. native.Exceptions

- **Target:** `native.Exceptions`
- **Similarity:** 0.11
- **Dependents:** 6
- **Priority Score:** 6050609.0
- **Functions:** 1/4 matched (target 12)
- **Missing functions:** `JobCancellationException::toString`, `JobCancellationException::equals`, `JobCancellationException::hashCode`
- **Types:** 0/2 matched (target 0)
- **Missing types:** `CancellationException`, `JobCancellationException`
- **Lint issues:** 2

### 5. native.CoroutineContext

- **Target:** `coroutines.UndispatchedCoroutine`
- **Similarity:** 0.22
- **Dependents:** 6
- **Priority Score:** 6011208.0
- **Functions:** 9/10 matched (target 16)
- **Missing functions:** `Continuation<*>::toDebugString`
- **Types:** 2/2 matched (target 3)
- **Missing types:** _none_

### 6. common.CoroutineStart

- **Target:** `coroutines.CoroutineStart`
- **Similarity:** 0.00
- **Dependents:** 2
- **Priority Score:** 2010210.0
- **Functions:** 0/1 matched (target 4)
- **Missing functions:** `CoroutineStart::invoke`
- **Types:** 1/1 matched (target 3)
- **Missing types:** _none_
- **Lint issues:** 19

### 7. channels.BufferOverflow

- **Target:** `channels.BufferOverflow`
- **Similarity:** 1.00
- **Dependents:** 2
- **Priority Score:** 2000100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 8. selects.Select

- **Target:** `selects.Select`
- **Similarity:** 0.21
- **Dependents:** 1
- **Priority Score:** 1064608.0
- **Functions:** 24/30 matched (target 71)
- **Missing functions:** `SelectBuilder::invoke`, `SelectBuilder::onTimeout`, `SelectImplementation::register`, `SelectImplementation::processResultAndInvokeBlockRecoveringException`, `CancellableContinuation<Unit>::tryResume`, `TrySelectDetailedResult`
- **Types:** 16/16 matched (target 22)
- **Missing types:** _none_
- **Lint issues:** 6

### 9. internal.Combine

- **Target:** `internal.Combine`
- **Similarity:** 0.02
- **Dependents:** 1
- **Priority Score:** 1010309.8
- **Functions:** 1/2 matched (target 23)
- **Missing functions:** `FlowCollector<R>::combineInternal`
- **Types:** 1/1 matched (target 9)
- **Missing types:** _none_
- **Lint issues:** 4

### 10. internal.Merge

- **Target:** `internal.Merge`
- **Similarity:** 0.20
- **Dependents:** 1
- **Priority Score:** 1001207.9
- **Functions:** 9/9 matched (target 37)
- **Missing functions:** _none_
- **Types:** 3/3 matched (target 8)
- **Missing types:** _none_
- **Lint issues:** 10

### 11. internal.Symbol

- **Target:** `internal.Symbol`
- **Similarity:** 0.11
- **Dependents:** 1
- **Priority Score:** 1000308.9
- **Functions:** 2/2 matched (target 3)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 12. channels.Deprecated

- **Target:** `channels.Deprecated [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 474710.0
- **Functions:** 0/47 matched (target 0)
- **Missing functions:** `BroadcastChannel<E>::consume`, `BroadcastChannel<E>::consumeEach`, `consumesAll`, `ReceiveChannel<E>::elementAt`, `ReceiveChannel<E>::elementAtOrNull`, `ReceiveChannel<E>::first`, `ReceiveChannel<E>::firstOrNull`, `ReceiveChannel<E>::indexOf`, `ReceiveChannel<E>::last`, `ReceiveChannel<E>::lastIndexOf`, `ReceiveChannel<E>::lastOrNull`, `ReceiveChannel<E>::single`, `ReceiveChannel<E>::singleOrNull`, `ReceiveChannel<E>::drop`, `ReceiveChannel<E>::dropWhile`, `ReceiveChannel<E>::filter`, `ReceiveChannel<E>::filterIndexed`, `ReceiveChannel<E>::filterNot`, `ReceiveChannel<E?>::filterNotNull`, `ReceiveChannel<E?>::filterNotNullTo`, `ReceiveChannel<E?>::filterNotNullTo`, `ReceiveChannel<E>::take`, `ReceiveChannel<E>::takeWhile`, `ReceiveChannel<E>::toChannel`, `ReceiveChannel<E>::toCollection`, `ReceiveChannel<Pair<K, V>>::toMap`, `ReceiveChannel<Pair<K, V>>::toMap`, `ReceiveChannel<E>::toMutableList`, `ReceiveChannel<E>::toSet`, `ReceiveChannel<E>::flatMap`, `ReceiveChannel<E>::map`, `ReceiveChannel<E>::mapIndexed`, `ReceiveChannel<E>::mapIndexedNotNull`, `ReceiveChannel<E>::mapNotNull`, `ReceiveChannel<E>::withIndex`, `ReceiveChannel<E>::distinct`, `ReceiveChannel<E>::distinctBy`, `ReceiveChannel<E>::toMutableSet`, `ReceiveChannel<E>::any`, `ReceiveChannel<E>::count`, `ReceiveChannel<E>::maxWith`, `ReceiveChannel<E>::minWith`, `ReceiveChannel<E>::none`, `ReceiveChannel<E?>::requireNoNulls`, `ReceiveChannel<E>::zip`, `ReceiveChannel<E>::zip`, `ReceiveChannel<*>::consumes`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 13. common.TestBase.common

- **Target:** `testing.TestBase`
- **Similarity:** 0.01
- **Dependents:** 0
- **Priority Score:** 354809.9
- **Functions:** 3/29 matched (target 54)
- **Missing functions:** `assertRunsFast`, `assertRunsFast`, `OrderedExecution::Impl::expect`, `OrderedExecution::Impl::finish`, `OrderedExecution::Impl::expectUnreached`, `OrderedExecution::Impl::checkFinishCall`, `ErrorCatching::Impl::hasError`, `ErrorCatching::Impl::reportError`, `ErrorCatching::Impl::close`, `ErrorCatching::check`, `ErrorCatching::error`, `OrderedExecutionTestBase::checkFinished`, `OrderedExecutionTestBase::reset`, `OrderedExecutionTestBase::expect`, `OrderedExecutionTestBase::finish`, `OrderedExecutionTestBase::expectUnreached`, `OrderedExecutionTestBase::checkFinishCall`, `T::void`, `Flow<Int>::sum`, `Flow<Long>::longSum`, `wrapperDispatcher`, `isDispatchNeeded`, `dispatch`, `wrapperDispatcher`, `BadClass::equals`, `BadClass::hashCode`
- **Types:** 10/19 matched (target 12)
- **Missing types:** `Impl`, `OrderedExecutionTestBase`, `NoJs`, `NoNative`, `NoWasmJs`, `NoWasmWasi`, `TestResult`, `RecoverableTestException`, `RecoverableTestCancellationException`
- **Lint issues:** 6

### 14. common.EventLoop.common

- **Target:** `coroutines.EventLoop.common`
- **Similarity:** 0.14
- **Dependents:** 0
- **Priority Score:** 354808.6
- **Functions:** 11/38 matched (target 24)
- **Missing functions:** `EventLoop::limitedParallelism`, `delayToNanos`, `delayNanosToMillis`, `EventLoopImplBase::shutdown`, `EventLoopImplBase::scheduleResumeAfterDelay`, `EventLoopImplBase::scheduleInvokeOnTimeout`, `EventLoopImplBase::processNextEvent`, `EventLoopImplBase::dispatch`, `EventLoopImplBase::enqueue`, `EventLoopImplBase::enqueueImpl`, `EventLoopImplBase::dequeue`, `EventLoopImplBase::enqueueDelayedTasks`, `EventLoopImplBase::closeQueue`, `EventLoopImplBase::schedule`, `EventLoopImplBase::shouldUnpark`, `EventLoopImplBase::scheduleImpl`, `EventLoopImplBase::resetAll`, `EventLoopImplBase::rescheduleAllDelayed`, `EventLoopImplBase::DelayedTask::compareTo`, `EventLoopImplBase::DelayedTask::timeToExecute`, `EventLoopImplBase::DelayedTask::scheduleTask`, `EventLoopImplBase::DelayedTask::dispose`, `EventLoopImplBase::DelayedTask::toString`, `EventLoopImplBase::DelayedResumeTask::run`, `EventLoopImplBase::DelayedResumeTask::toString`, `EventLoopImplBase::DelayedRunnableTask::run`, `EventLoopImplBase::DelayedRunnableTask::toString`
- **Types:** 2/10 matched (target 3)
- **Missing types:** `Queue`, `EventLoopImplPlatform`, `EventLoopImplBase`, `DelayedTask`, `DelayedResumeTask`, `DelayedRunnableTask`, `DelayedTaskQueue`, `DefaultExecutor`
- **Lint issues:** 4

### 15. flow.Migration

- **Target:** `flow.Migration`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 353610.0
- **Functions:** 1/36 matched (target 1)
- **Missing functions:** `Flow<T>::observeOn`, `Flow<T>::publishOn`, `Flow<T>::subscribeOn`, `Flow<T>::onErrorResume`, `Flow<T>::onErrorResumeNext`, `Flow<T>::subscribe`, `Flow<T>::subscribe`, `Flow<T>::subscribe`, `Flow<T>::flatMap`, `Flow<T>::concatMap`, `Flow<Flow<T>>::merge`, `Flow<Flow<T>>::flatten`, `Flow<T>::compose`, `Flow<T>::skip`, `Flow<T>::forEach`, `Flow<T>::scanFold`, `Flow<T>::onErrorReturn`, `Flow<T>::onErrorReturn`, `Flow<T>::startWith`, `Flow<T>::startWith`, `Flow<T>::concatWith`, `Flow<T>::concatWith`, `Flow<T1>::combineLatest`, `Flow<T1>::combineLatest`, `Flow<T1>::combineLatest`, `Flow<T1>::combineLatest`, `Flow<T>::delayFlow`, `Flow<T>::delayEach`, `Flow<T>::switchMap`, `Flow<T>::scanReduce`, `Flow<T>::publish`, `Flow<T>::publish`, `Flow<T>::replay`, `Flow<T>::replay`, `Flow<T>::cache`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 8

### 16. sharing.SharedFlowTest

- **Target:** `sharing.SharedFlowTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 303510.0
- **Functions:** 2/32 matched (target 7)
- **Missing functions:** `SharedFlowTest::testRendezvousSharedFlowBasic`, `SharedFlowTest::testRendezvousSharedFlowReset`, `SharedFlowTest::testReplay1SharedFlowBasic`, `SharedFlowTest::testReplay1`, `SharedFlowTest::testReplay2Extra1`, `SharedFlowTest::testBufferNoReplayCancelWhileBuffering`, `SharedFlowTest::testRepeatedResetWithReplay`, `SharedFlowTest::testSynchronousSharedFlowEmitterCancel`, `SharedFlowTest::testDifferentBufferedFlowCapacities`, `SharedFlowTest::testBufferedFlow`, `SharedFlowTest::testDropLatest`, `SharedFlowTest::testDropOldest`, `SharedFlowTest::testOnSubscription`, `SharedFlowTest::share`, `SharedFlowTest::onSubscriptionThrows`, `SharedFlowTest::testBigReplayManySubscribers`, `SharedFlowTest::testBigBufferManySubscribers`, `SharedFlowTest::testManySubscribers`, `SharedFlowTest::testStateFlowModel`, `SharedFlowTest::modelLog`, `SharedFlowTest::nextData`, `SharedFlowTest::testOperatorFusion`, `SharedFlowTest::testIllegalArgumentException`, `SharedFlowTest::testReplayCancellability`, `SharedFlowTest::testEmitCancellability`, `SharedFlowTest::testCancellability`, `SharedFlowTest::emitTestData`, `SharedFlowTest::testSubscriptionCount`, `SharedFlowTest::startSubscriber`, `SharedFlowTest::testSubscriptionByFirstSuspensionInSharedFlow`
- **Types:** 3/3 matched
- **Missing types:** _none_
- **Tests:** 0/22 matched
- **TODOs:** 10
- **Lint issues:** 3

### 17. test.RunTestTest

- **Target:** `tests.RunTestTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 272810.0
- **Functions:** 0/27 matched
- **Missing functions:** `RunTestTest::testWithContextDispatching`, `RunTestTest::testJoiningForkedJob`, `RunTestTest::testSuspendCoroutine`, `RunTestTest::testNestedRunTestForbidden`, `RunTestTest::testRunTestWithZeroDispatchTimeoutWithControlledDispatches`, `RunTestTest::testRunTestWithSmallDispatchTimeout`, `RunTestTest::testRunTestWithSmallTimeout`, `RunTestTest::testRunTestWithSmallTimeoutAndManyDispatches`, `RunTestTest::testListingActiveCoroutinesOnTimeout`, `RunTestTest::testFailureWithPendingCoroutine`, `RunTestTest::testRunTestWithLargeDispatchTimeout`, `RunTestTest::testRunTestWithLargeTimeout`, `RunTestTest::testRunTestTimingOutAndThrowing`, `RunTestTest::testRunTestWithIllegalContext`, `RunTestTest::testThrowingInRunTestBody`, `RunTestTest::testThrowingInRunTestPendingTask`, `RunTestTest::reproducer2405`, `RunTestTest::testChildrenCancellationOnTestBodyFailure`, `RunTestTest::testTimeout`, `RunTestTest::testRunTestThrowsRootCause`, `RunTestTest::testCompletesOwnJob`, `RunTestTest::testDoesNotCompleteGivenJob`, `RunTestTest::testSuppressedExceptions`, `RunTestTest::testScopeRunTestExceptionHandler`, `RunTestTest::testCoroutineCompletingWithoutDispatch`, `RunTestTest::testExceptionCaptorCleanedUpOnPreliminaryExit`, `RunTestTest::testCancellingTestScope`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/27 matched
- **TODOs:** 59
- **Lint issues:** 9

### 18. test.AwaitTest

- **Target:** `tests.AwaitTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 242510.0
- **Functions:** 0/24 matched
- **Missing functions:** `AwaitTest::testAwaitAll`, `AwaitTest::testAwaitAllLazy`, `AwaitTest::testAwaitAllTyped`, `AwaitTest::testAwaitAllExceptionally`, `AwaitTest::testAwaitAllMultipleExceptions`, `AwaitTest::testAwaitAllCancellation`, `AwaitTest::testAwaitAllPartiallyCompleted`, `AwaitTest::testAwaitAllPartiallyCompletedExceptionally`, `AwaitTest::testAwaitAllFullyCompleted`, `AwaitTest::testAwaitOnSet`, `AwaitTest::testAwaitAllFullyCompletedExceptionally`, `AwaitTest::testAwaitAllSameJobMultipleTimes`, `AwaitTest::testAwaitAllSameThrowingJobMultipleTimes`, `AwaitTest::testAwaitAllEmpty`, `AwaitTest::testJoinAll`, `AwaitTest::testJoinAllLazy`, `AwaitTest::testJoinAllExceptionally`, `AwaitTest::testJoinAllCancellation`, `AwaitTest::testJoinAllAlreadyCompleted`, `AwaitTest::testJoinAllEmpty`, `AwaitTest::testJoinAllSameJob`, `AwaitTest::testJoinAllSameJobExceptionally`, `AwaitTest::testAwaitAllDelegates`, `AwaitTest::testCancelAwaitAllDelegate`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/24 matched
- **TODOs:** 8
- **Lint issues:** 11

### 19. operators.BufferTest

- **Target:** `operators.BufferTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 232510.0
- **Functions:** 1/24 matched
- **Missing functions:** `BufferTest::testBaseline`, `BufferTest::testBufferDefault`, `BufferTest::testBufferRendezvous`, `BufferTest::testBuffer1`, `BufferTest::testBuffer2`, `BufferTest::testBuffer3`, `BufferTest::testBuffer00Fused`, `BufferTest::testBuffer01Fused`, `BufferTest::testBuffer11Fused`, `BufferTest::testBuffer111Fused`, `BufferTest::testBuffer123Fused`, `BufferTest::testBufferDefaultTwiceFused`, `BufferTest::testBufferDefaultBufferFused`, `BufferTest::testBufferBufferDefaultFused`, `BufferTest::testFlowOnNameNoBuffer`, `BufferTest::testFlowOnDispatcherBufferDefault`, `BufferTest::testFlowOnDispatcherBufferFused`, `BufferTest::testBufferFlowOnDispatcherFused`, `BufferTest::testFlowOnNameBufferFused`, `BufferTest::testBufferFlowOnNameFused`, `BufferTest::testBufferFlowOnMultipleFused`, `BufferTest::testCancellation`, `BufferTest::testFailsOnIllegalArguments`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/23 matched
- **TODOs:** 33
- **Lint issues:** 9

### 20. test.CoroutineScopeTest

- **Target:** `tests.CoroutineScopeTest`
- **Similarity:** 0.15
- **Dependents:** 0
- **Priority Score:** 232508.5
- **Functions:** 1/24 matched (target 5)
- **Missing functions:** `CoroutineScopeTest::testScope`, `CoroutineScopeTest::callJobScoped`, `CoroutineScopeTest::testScopeCancelledFromWithin`, `CoroutineScopeTest::callJobScoped`, `CoroutineScopeTest::testExceptionFromWithin`, `CoroutineScopeTest::testScopeBlockThrows`, `CoroutineScopeTest::callJobScoped`, `CoroutineScopeTest::testOuterJobIsCancelled`, `CoroutineScopeTest::callJobScoped`, `CoroutineScopeTest::testAsyncCancellationFirst`, `CoroutineScopeTest::failedConcurrentSumFirst`, `CoroutineScopeTest::testAsyncCancellationSecond`, `CoroutineScopeTest::failedConcurrentSumSecond`, `CoroutineScopeTest::testDocumentationExample`, `CoroutineScopeTest::loadData`, `CoroutineScopeTest::testCoroutineScopeCancellationVsException`, `CoroutineScopeTest::testLaunchContainsDefaultDispatcher`, `CoroutineScopeTest::testNewCoroutineContextDispatcher`, `CoroutineScopeTest::newContextDispatcher`, `CoroutineScopeTest::testScopePlusContext`, `CoroutineScopeTest::testIncompleteScopeState`, `CoroutineScopeTest::testIsActiveWithoutJob`, `CoroutineScopeTest::testIsActive`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/14 matched
- **TODOs:** 8
- **Lint issues:** 1

### 21. native.MultithreadedDispatchers

- **Target:** `native.MultithreadedDispatchers [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 222210.0
- **Functions:** 0/19 matched (target 0)
- **Missing functions:** `newFixedThreadPoolContext`, `WorkerDispatcher::dispatch`, `WorkerDispatcher::scheduleResumeAfterDelay`, `WorkerDispatcher::invokeOnTimeout`, `WorkerDispatcher::schedule`, `WorkerDispatcher::DisposableBlock::invoke`, `WorkerDispatcher::DisposableBlock::dispose`, `WorkerDispatcher::DisposableBlock::isDisposed`, `WorkerDispatcher::runAfterDelay`, `WorkerDispatcher::close`, `MultiWorkerDispatcher::isClosed`, `MultiWorkerDispatcher::hasTasks`, `MultiWorkerDispatcher::hasWorkers`, `MultiWorkerDispatcher::workerRunLoop`, `MultiWorkerDispatcher::obtainWorker`, `MultiWorkerDispatcher::dispatch`, `MultiWorkerDispatcher::limitedParallelism`, `MultiWorkerDispatcher::close`, `MultiWorkerDispatcher::checkChannelResult`
- **Types:** 0/3 matched (target 0)
- **Missing types:** `WorkerDispatcher`, `DisposableBlock`, `MultiWorkerDispatcher`

### 22. common.Job

- **Target:** `coroutines.Job`
- **Similarity:** 0.10
- **Dependents:** 0
- **Priority Score:** 213109.0
- **Functions:** 5/24 matched (target 19)
- **Missing functions:** `Job::cancel`, `Job::plus`, `Job::invokeOnCompletion`, `Job`, `Job0`, `Job::disposeOnCompletion`, `Job::cancelAndJoin`, `Job::cancelChildren`, `Job::cancelChildren`, `CoroutineContext::cancel`, `CoroutineContext::cancel`, `CoroutineContext::ensureActive`, `Job::cancel`, `CoroutineContext::cancel`, `CoroutineContext::cancelChildren`, `CoroutineContext::cancelChildren`, `CoroutineContext::cancelChildren`, `orCancellation`, `DisposeOnCompletion::invoke`
- **Types:** 5/7 matched (target 9)
- **Missing types:** `DisposableHandle`, `DisposeOnCompletion`
- **Lint issues:** 1

### 23. flow.FlowInvariantsTest

- **Target:** `flow.FlowInvariantsTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 212710.0
- **Functions:** 5/26 matched (target 21)
- **Missing functions:** `FlowInvariantsTest::collectSafely`, `FlowInvariantsTest::testWithContextContract`, `FlowInvariantsTest::testWithDispatcherContractViolated`, `FlowInvariantsTest::testWithNameContractViolated`, `FlowInvariantsTest::testWithContextDoesNotChangeExecution`, `FlowInvariantsTest::testScopedJob`, `FlowInvariantsTest::testScopedJobWithViolation`, `FlowInvariantsTest::testMergeViolation`, `FlowInvariantsTest::merge`, `FlowInvariantsTest::trickyMerge`, `FlowInvariantsTest::testNoMergeViolation`, `FlowInvariantsTest::merge`, `FlowInvariantsTest::trickyMerge`, `FlowInvariantsTest::testScopedCoroutineNoViolation`, `FlowInvariantsTest::buffer`, `FlowInvariantsTest::testEmptyCoroutineContextMap`, `FlowInvariantsTest::testEmptyCoroutineContextTransform`, `FlowInvariantsTest::testEmptyCoroutineContextTransformWhile`, `FlowInvariantsTest::testEmptyCoroutineContextViolationTransform`, `FlowInvariantsTest::testEmptyCoroutineContextViolationTransformWhile`, `FlowInvariantsTest::collector`
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_
- **Tests:** 0/14 matched
- **TODOs:** 14
- **Lint issues:** 7

### 24. operators.CombineParametersTest

- **Target:** `operators.CombineParametersTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 212210.0
- **Functions:** 0/21 matched
- **Missing functions:** `CombineParametersTest::testThreeParameters`, `CombineParametersTest::testThreeParametersTransform`, `CombineParametersTest::testFourParameters`, `CombineParametersTest::testFourParametersTransform`, `CombineParametersTest::testFiveParameters`, `CombineParametersTest::testFiveParametersTransform`, `CombineParametersTest::testNonMatchingTypes`, `CombineParametersTest::testNonMatchingTypesIterable`, `CombineParametersTest::testVararg`, `CombineParametersTest::testVarargTransform`, `CombineParametersTest::testSingleVararg`, `CombineParametersTest::testSingleVarargTransform`, `CombineParametersTest::testReified`, `CombineParametersTest::testReifiedTransform`, `CombineParametersTest::testTransformEmptyIterable`, `CombineParametersTest::testTransformEmptyVararg`, `CombineParametersTest::testEmptyIterable`, `CombineParametersTest::testEmptyVararg`, `CombineParametersTest::testFairnessInVariousConfigurations`, `CombineParametersTest::testEpochOverflow`, `CombineParametersTest::testArrayType`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/21 matched
- **TODOs:** 70
- **Lint issues:** 4

### 25. operators.OnCompletionTest

- **Target:** `operators.OnCompletionTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 202410.0
- **Functions:** 0/20 matched (target 8)
- **Missing functions:** `OnCompletionTest::testOnCompletion`, `OnCompletionTest::testOnCompletionWithException`, `OnCompletionTest::testOnCompletionWithExceptionDownstream`, `OnCompletionTest::testMultipleOnCompletions`, `OnCompletionTest::testExceptionFromOnCompletion`, `OnCompletionTest::testContextPreservation`, `OnCompletionTest::testEmitExample`, `OnCompletionTest::TestData::Done::equals`, `OnCompletionTest::testCrashedEmit`, `OnCompletionTest::testCancelledEmit`, `OnCompletionTest::testFailedEmit`, `OnCompletionTest::testFirst`, `OnCompletionTest::testSingle`, `OnCompletionTest::testEmptySingleInterference`, `OnCompletionTest::testTransparencyViolation`, `OnCompletionTest::testTakeOnCompletion`, `OnCompletionTest::testCancelledEmitAllFlow`, `OnCompletionTest::testCancelledEmitAllChannel`, `OnCompletionTest::testOnCompletionBetweenLimitingOperators`, `OnCompletionTest::testEmittingElementsAfterCancellation`
- **Types:** 4/4 matched
- **Missing types:** _none_
- **Tests:** 0/19 matched
- **TODOs:** 7
- **Lint issues:** 3

### 26. operators.SampleTest

- **Target:** `operators.SampleTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 202110.0
- **Functions:** 0/20 matched (target 3)
- **Missing functions:** `SampleTest::testBasic`, `SampleTest::testDelayedFirst`, `SampleTest::testBasic2`, `SampleTest::testFixedDelay`, `SampleTest::testSingleNull`, `SampleTest::testBasicWithNulls`, `SampleTest::testEmpty`, `SampleTest::testScalar`, `SampleTest::testLongWait`, `SampleTest::testPace`, `SampleTest::testUpstreamError`, `SampleTest::testUpstreamErrorCancellationException`, `SampleTest::testUpstreamError`, `SampleTest::testUpstreamErrorIsolatedContext`, `SampleTest::testUpstreamErrorSampleNotTriggered`, `SampleTest::testUpstreamErrorSampleNotTriggeredInIsolatedContext`, `SampleTest::testDownstreamError`, `SampleTest::testDownstreamErrorIsolatedContext`, `SampleTest::testDurationBasic`, `SampleTest::testFailsWithIllegalArgument`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/19 matched
- **TODOs:** 6

### 27. operators.DebounceTest

- **Target:** `operators.DebounceTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 202110.0
- **Functions:** 0/20 matched (target 4)
- **Missing functions:** `DebounceTest::testBasic`, `DebounceTest::testSingleNull`, `DebounceTest::testBasicWithNulls`, `DebounceTest::testEmpty`, `DebounceTest::testScalar`, `DebounceTest::testPace`, `DebounceTest::testUpstreamError`, `DebounceTest::testUpstreamErrorCancellation`, `DebounceTest::testUpstreamError`, `DebounceTest::testUpstreamErrorIsolatedContext`, `DebounceTest::testUpstreamErrorDebounceNotTriggered`, `DebounceTest::testUpstreamErrorDebounceNotTriggeredInIsolatedContext`, `DebounceTest::testDownstreamError`, `DebounceTest::testDownstreamErrorIsolatedContext`, `DebounceTest::testDurationBasic`, `DebounceTest::testDebounceSelectorBasic`, `DebounceTest::testZeroDebounceTime`, `DebounceTest::testZeroDebounceTimeSelector`, `DebounceTest::testDebounceDurationSelectorBasic`, `DebounceTest::testFailsWithIllegalArgument`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/19 matched
- **TODOs:** 16

### 28. sharing.ShareInConflationTest

- **Target:** `sharing.ShareInConflationTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 192110.0
- **Functions:** 1/20 matched
- **Missing functions:** `ShareInConflationTest::testConflateReplay1`, `ShareInConflationTest::testConflateReplay0`, `ShareInConflationTest::testConflateReplay5`, `ShareInConflationTest::testBufferDropOldestReplay1`, `ShareInConflationTest::testBufferDropOldestReplay0`, `ShareInConflationTest::testBufferDropOldestReplay10`, `ShareInConflationTest::testBuffer20DropOldestReplay0`, `ShareInConflationTest::testBuffer7DropOldestReplay11`, `ShareInConflationTest::testBufferConflateOverride`, `ShareInConflationTest::testBufferDropOldestOverride`, `ShareInConflationTest::testBufferDropLatestReplay0`, `ShareInConflationTest::testBufferDropLatestReplay1`, `ShareInConflationTest::testBufferDropLatestReplay10`, `ShareInConflationTest::testBuffer0DropLatestReplay0`, `ShareInConflationTest::testBuffer0DropLatestReplay1`, `ShareInConflationTest::testBuffer0DropLatestReplay10`, `ShareInConflationTest::testBuffer5DropLatestReplay0`, `ShareInConflationTest::testBuffer5DropLatestReplay10`, `ShareInConflationTest::testBufferDropLatestOverride`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/19 matched
- **TODOs:** 6
- **Lint issues:** 11

### 29. test.CoroutinesTest

- **Target:** `tests.CoroutinesTest`
- **Similarity:** 0.32
- **Dependents:** 0
- **Priority Score:** 192106.8
- **Functions:** 1/20 matched (target 8)
- **Missing functions:** `CoroutinesTest::testSimple`, `CoroutinesTest::testYield`, `CoroutinesTest::testLaunchAndYieldJoin`, `CoroutinesTest::testLaunchUndispatched`, `CoroutinesTest::testNested`, `CoroutinesTest::testWaitChild`, `CoroutinesTest::testCancelChildExplicit`, `CoroutinesTest::testCancelChildWithFinally`, `CoroutinesTest::testWaitNestedChild`, `CoroutinesTest::testExceptionPropagation`, `CoroutinesTest::testCancelParentOnChildException`, `CoroutinesTest::testCancelParentOnNestedException`, `CoroutinesTest::testJoinWithFinally`, `CoroutinesTest::testCancelAndJoin`, `CoroutinesTest::testCancelAndJoinChildCrash`, `CoroutinesTest::testYieldInFinally`, `CoroutinesTest::testCancelAndJoinChildren`, `CoroutinesTest::testParentCrashCancelsChildren`, `CoroutinesTest::testNotCancellableChildWithExceptionCancelled`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/19 matched
- **TODOs:** 6
- **Lint issues:** 1

### 30. operators.FlowOnTest

- **Target:** `operators.FlowOnTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 182310.0
- **Functions:** 2/20 matched (target 22)
- **Missing functions:** `FlowOnTest::testFlowOn`, `FlowOnTest::testFlowOnAndOperators`, `FlowOnTest::testFlowOnThrowingSource`, `FlowOnTest::testFlowOnThrowingOperator`, `FlowOnTest::testFlowOnDownstreamOperator`, `FlowOnTest::testFlowOnThrowingConsumer`, `FlowOnTest::testFlowOnWithJob`, `FlowOnTest::testFlowOnCancellation`, `FlowOnTest::testFlowOnCancellationHappensBefore`, `FlowOnTest::testIndependentOperatorContext`, `FlowOnTest::testMultipleFlowOn`, `FlowOnTest::testTimeoutExceptionUpstream`, `FlowOnTest::testTimeoutExceptionDownstream`, `FlowOnTest::testCancellation`, `FlowOnTest::testAtomicStart`, `FlowOnTest::testException`, `FlowOnTest::testIllegalArgumentException`, `FlowOnTest::testCancelledFlowOn`
- **Types:** 3/3 matched
- **Missing types:** _none_
- **Tests:** 0/18 matched
- **TODOs:** 6
- **Lint issues:** 6

### 31. test.WithContextTest

- **Target:** `tests.WithContextTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 182210.0
- **Functions:** 2/20 matched
- **Missing functions:** `WithContextTest::testThrowException`, `WithContextTest::testThrowExceptionFromWrappedContext`, `WithContextTest::testSameContextNoSuspend`, `WithContextTest::testSameContextWithSuspend`, `WithContextTest::testCancelWithJobNoSuspend`, `WithContextTest::testCancelWithJobWithSuspend`, `WithContextTest::testRunCancellableDefault`, `WithContextTest::testRunCancellationUndispatchedVsException`, `WithContextTest::testRunCancellationDispatchedVsException`, `WithContextTest::testRunSelfCancellationWithException`, `WithContextTest::testRunSelfCancellation`, `WithContextTest::testWithContextScopeFailure`, `WithContextTest::testWithContextChildWaitSameContext`, `WithContextTest::testWithContextChildWaitWrappedContext`, `WithContextTest::testIncompleteWithContextState`, `WithContextTest::testWithContextCancelledJob`, `WithContextTest::testWithContextCancelledThisJob`, `WithContextTest::testSequentialCancellation`
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Tests:** 0/18 matched
- **TODOs:** 43
- **Lint issues:** 13

### 32. test.WithTimeoutOrNullDurationTest

- **Target:** `tests.WithTimeoutOrNullDurationTest`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 182010.0
- **Functions:** 0/18 matched (target 15)
- **Missing functions:** `WithTimeoutOrNullDurationTest::testBasicNoSuspend`, `WithTimeoutOrNullDurationTest::testBasicSuspend`, `WithTimeoutOrNullDurationTest::testDispatch`, `WithTimeoutOrNullDurationTest::testYieldBlockingWithTimeout`, `WithTimeoutOrNullDurationTest::testSmallTimeout`, `WithTimeoutOrNullDurationTest::testThrowException`, `WithTimeoutOrNullDurationTest::testInnerTimeout`, `WithTimeoutOrNullDurationTest::testNestedTimeout`, `WithTimeoutOrNullDurationTest::testOuterTimeout`, `WithTimeoutOrNullDurationTest::testBadClass`, `WithTimeoutOrNullDurationTest::BadClass::equals`, `WithTimeoutOrNullDurationTest::BadClass::hashCode`, `WithTimeoutOrNullDurationTest::BadClass::toString`, `WithTimeoutOrNullDurationTest::testNullOnTimeout`, `WithTimeoutOrNullDurationTest::testSuppressExceptionWithResult`, `WithTimeoutOrNullDurationTest::testSuppressExceptionWithAnotherException`, `WithTimeoutOrNullDurationTest::testNegativeTimeout`, `WithTimeoutOrNullDurationTest::testExceptionFromWithinTimeout`
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Tests:** 0/15 matched
- **TODOs:** 22
- **Lint issues:** 14

### 33. operators.TimeoutTest

- **Target:** `operators.TimeoutTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 172010.0
- **Functions:** 2/19 matched (target 4)
- **Missing functions:** `TimeoutTest::testBasic`, `TimeoutTest::testSingleNull`, `TimeoutTest::testBasicCustomAction`, `TimeoutTest::testDelayedFirst`, `TimeoutTest::testEmpty`, `TimeoutTest::testScalar`, `TimeoutTest::testUpstreamError`, `TimeoutTest::testUpstreamErrorTimeoutException`, `TimeoutTest::testUpstreamErrorCancellationException`, `TimeoutTest::testUpstreamExceptionsTakingPriority`, `TimeoutTest::testDownstreamError`, `TimeoutTest::testUpstreamTimeoutIsolatedContext`, `TimeoutTest::testUpstreamTimeoutActionIsolatedContext`, `TimeoutTest::testSharedFlowTimeout`, `TimeoutTest::testSharedFlowCancelledNoTimeout`, `TimeoutTest::testImmediateTimeout`, `TimeoutTest::testClosing`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/17 matched
- **TODOs:** 8
- **Lint issues:** 2

### 34. operators.BufferConflationTest

- **Target:** `operators.BufferConflationTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 171910.0
- **Functions:** 1/18 matched
- **Missing functions:** `BufferConflationTest::testConflate`, `BufferConflationTest::testBufferConflated`, `BufferConflationTest::testBufferDropOldest`, `BufferConflationTest::testBuffer0DropOldest`, `BufferConflationTest::testBuffer1DropOldest`, `BufferConflationTest::testBuffer10DropOldest`, `BufferConflationTest::testConflateOverridesBuffer`, `BufferConflationTest::testDoubleConflate`, `BufferConflationTest::testConflateBuffer10Combine`, `BufferConflationTest::testBufferDropLatest`, `BufferConflationTest::testBuffer0DropLatest`, `BufferConflationTest::testBuffer1DropLatest`, `BufferConflationTest::testBufferDropLatestOverrideBuffer`, `BufferConflationTest::testBufferDropLatestOverrideConflate`, `BufferConflationTest::testBufferDropLatestBuffer7Combine`, `BufferConflationTest::testConflateOverrideBufferDropLatest`, `BufferConflationTest::testBuffer3DropOldestOverrideBuffer8DropLatest`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/17 matched
- **TODOs:** 27
- **Lint issues:** 17

### 35. channels.ProduceTest

- **Target:** `channels.ProduceTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 171910.0
- **Functions:** 1/18 matched
- **Missing functions:** `ProduceTest::testBasic`, `ProduceTest::testCancelWithoutCause`, `ProduceTest::testCancelWithCause`, `ProduceTest::testCancelOnCompletionUnconfined`, `ProduceTest::testCancelOnCompletion`, `ProduceTest::testCancelWhenTheChannelIsClosed`, `ProduceTest::testAwaitCloseOnlyAllowedOnce`, `ProduceTest::testInvokeOnCloseWithAwaitClose`, `ProduceTest::testAwaitConsumerCancellation`, `ProduceTest::testAwaitProducerCancellation`, `ProduceTest::testAwaitParentCancellation`, `ProduceTest::testAwaitIllegalState`, `ProduceTest::testUncaughtExceptionsInProduce`, `ProduceTest::testCancellingProduceCoroutineButNotChannel`, `ProduceTest::testReceivingValuesAfterFailingTheCoroutine`, `ProduceTest::testSilentKillerInProduce`, `ProduceTest::testProduceWithInvalidCapacity`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/17 matched
- **TODOs:** 43
- **Lint issues:** 1

### 36. test.TestCoroutineSchedulerTest

- **Target:** `tests.TestCoroutineSchedulerTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 161910.0
- **Functions:** 2/18 matched (target 17)
- **Missing functions:** `TestCoroutineSchedulerTest::testContextElement`, `TestCoroutineSchedulerTest::testAdvanceTimeByDoesNotRunCurrent`, `TestCoroutineSchedulerTest::testAdvanceTimeByWithNegativeDelay`, `TestCoroutineSchedulerTest::testAdvanceTimeByEnormousDelays`, `TestCoroutineSchedulerTest::testAdvanceTimeBy`, `TestCoroutineSchedulerTest::testRunCurrent`, `TestCoroutineSchedulerTest::testRunCurrentNotDrainingQueue`, `TestCoroutineSchedulerTest::testNestedAdvanceUntilIdle`, `TestCoroutineSchedulerTest::testYield`, `TestCoroutineSchedulerTest::testDelaysPriority`, `TestCoroutineSchedulerTest::checkTime`, `TestCoroutineSchedulerTest::testSmallTimeouts`, `TestCoroutineSchedulerTest::testLargeTimeouts`, `TestCoroutineSchedulerTest::testSmallAsynchronousTimeouts`, `TestCoroutineSchedulerTest::testLargeAsynchronousTimeouts`, `TestCoroutineSchedulerTest::testAdvanceTimeSource`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/15 matched
- **TODOs:** 30
- **Lint issues:** 6

### 37. operators.ZipTest

- **Target:** `operators.ZipTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 161710.0
- **Functions:** 0/16 matched (target 9)
- **Missing functions:** `ZipTest::testZip`, `ZipTest::testUnevenZip`, `ZipTest::testEmptyFlows`, `ZipTest::testEmpty`, `ZipTest::testEmptyOther`, `ZipTest::testNulls`, `ZipTest::testNullsOther`, `ZipTest::testCancelWhenFlowIsDone`, `ZipTest::testCancelWhenFlowIsDone2`, `ZipTest::testCancelWhenFlowIsDoneReversed`, `ZipTest::testContextIsIsolatedReversed`, `ZipTest::testErrorInDownstreamCancelsUpstream`, `ZipTest::testErrorCancelsSibling`, `ZipTest::testCancellationUpstream`, `ZipTest::testCancellationDownstream`, `ZipTest::testCancellationOfCollector`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/16 matched
- **TODOs:** 6
- **Lint issues:** 31

### 38. common.MainDispatcherTestBase

- **Target:** `tests.MainDispatcherTestBase [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 152510.0
- **Functions:** 8/23 matched
- **Missing functions:** `MainDispatcherTestBase::testMainDispatcherToString`, `MainDispatcherTestBase::testMainDispatcherOrderingInMainThread`, `MainDispatcherTestBase::testMainDispatcherOrderingOutsideMainThread`, `MainDispatcherTestBase::testHandlerDispatcherNotEqualToImmediate`, `MainDispatcherTestBase::testImmediateDispatcherYield`, `MainDispatcherTestBase::testEnteringImmediateFromMain`, `MainDispatcherTestBase::testDispatchRequirements`, `MainDispatcherTestBase::testLaunchInMainScope`, `MainDispatcherTestBase::testFailureInMainScope`, `MainDispatcherTestBase::testCancellationInMainScope`, `MainDispatcherTestBase::WithRealTimeDelay::testDelay`, `MainDispatcherTestBase::WithRealTimeDelay::testWithTimeoutContextDelayNoTimeout`, `MainDispatcherTestBase::WithRealTimeDelay::testWithTimeoutContextDelayTimeout`, `MainDispatcherTestBase::WithRealTimeDelay::testWithContextTimeoutDelayNoTimeout`, `MainDispatcherTestBase::WithRealTimeDelay::testWithContextTimeoutDelayTimeout`
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Tests:** 0/15 matched
- **TODOs:** 50
- **Lint issues:** 25

### 39. common.TestScope

- **Target:** `test.TestScope`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 151710.0
- **Functions:** 1/13 matched (target 2)
- **Missing functions:** `TestScope::advanceUntilIdle`, `TestScope::runCurrent`, `TestScope::advanceTimeBy`, `TestScope::advanceTimeBy`, `CoroutineContext::withDelaySkipping`, `TestScopeImpl::enter`, `TestScopeImpl::leave`, `TestScopeImpl::legacyLeave`, `TestScopeImpl::reportException`, `TestScopeImpl::tryGetCompletionCause`, `TestScopeImpl::toString`, `TestScope::asSpecificImplementation`
- **Types:** 1/4 matched (target 1)
- **Missing types:** `TestScopeImpl`, `UncaughtExceptionsBeforeTest`, `UncompletedCoroutinesError`
- **Lint issues:** 1

### 40. test.WithTimeoutDurationTest

- **Target:** `tests.WithTimeoutDurationTest`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 151710.0
- **Functions:** 0/15 matched (target 12)
- **Missing functions:** `WithTimeoutDurationTest::testBasicNoSuspend`, `WithTimeoutDurationTest::testBasicSuspend`, `WithTimeoutDurationTest::testDispatch`, `WithTimeoutDurationTest::testYieldBlockingWithTimeout`, `WithTimeoutDurationTest::testWithTimeoutChildWait`, `WithTimeoutDurationTest::testBadClass`, `WithTimeoutDurationTest::BadClass::equals`, `WithTimeoutDurationTest::BadClass::hashCode`, `WithTimeoutDurationTest::BadClass::toString`, `WithTimeoutDurationTest::testExceptionOnTimeout`, `WithTimeoutDurationTest::testSuppressExceptionWithResult`, `WithTimeoutDurationTest::testSuppressExceptionWithAnotherException`, `WithTimeoutDurationTest::testNegativeTimeout`, `WithTimeoutDurationTest::testExceptionFromWithinTimeout`, `WithTimeoutDurationTest::testIncompleteWithTimeoutState`
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Tests:** 0/12 matched
- **TODOs:** 19
- **Lint issues:** 11

### 41. test.WithTimeoutOrNullTest

- **Target:** `tests.WithTimeoutOrNullTest`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 151610.0
- **Functions:** 0/15 matched
- **Missing functions:** `WithTimeoutOrNullTest::testBasicNoSuspend`, `WithTimeoutOrNullTest::testBasicSuspend`, `WithTimeoutOrNullTest::testDispatch`, `WithTimeoutOrNullTest::testYieldBlockingWithTimeout`, `WithTimeoutOrNullTest::testSmallTimeout`, `WithTimeoutOrNullTest::testThrowException`, `WithTimeoutOrNullTest::testInnerTimeout`, `WithTimeoutOrNullTest::testNestedTimeout`, `WithTimeoutOrNullTest::testOuterTimeout`, `WithTimeoutOrNullTest::testBadClass`, `WithTimeoutOrNullTest::testNullOnTimeout`, `WithTimeoutOrNullTest::testSuppressExceptionWithResult`, `WithTimeoutOrNullTest::testSuppressExceptionWithAnotherException`, `WithTimeoutOrNullTest::testNegativeTimeout`, `WithTimeoutOrNullTest::testExceptionFromWithinTimeout`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/15 matched
- **TODOs:** 19
- **Lint issues:** 20

### 42. operators.CombineTest

- **Target:** `operators.CombineTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 143110.0
- **Functions:** 8/22 matched (target 12)
- **Missing functions:** `CombineTestBase::testCombineLatest`, `CombineTestBase::testNulls`, `CombineTestBase::testNullsOther`, `CombineTestBase::testEmptyFlow`, `CombineTestBase::testFirstIsEmpty`, `CombineTestBase::testSecondIsEmpty`, `CombineTestBase::testPreservingOrder`, `CombineTestBase::testPreservingOrderReversed`, `CombineTestBase::testContextIsIsolated`, `CombineTestBase::testErrorInDownstreamCancelsUpstream`, `CombineTestBase::testErrorCancelsSibling`, `CombineTestBase::testCancellationExceptionUpstream`, `CombineTestBase::testCancellationExceptionDownstream`, `CombineTestBase::testCancelledCombine`
- **Types:** 9/9 matched
- **Missing types:** _none_
- **Tests:** 0/14 matched
- **TODOs:** 19
- **Lint issues:** 18

### 43. channels.ChannelFlowTest

- **Target:** `channels.ChannelFlowTest`
- **Similarity:** 0.41
- **Dependents:** 0
- **Priority Score:** 141705.9
- **Functions:** 2/16 matched (target 15)
- **Missing functions:** `ChannelFlowTest::testRegular`, `ChannelFlowTest::testBuffer`, `ChannelFlowTest::testConflated`, `ChannelFlowTest::testFailureCancelsChannel`, `ChannelFlowTest::testFailureInSourceCancelsConsumer`, `ChannelFlowTest::testScopedCancellation`, `ChannelFlowTest::testMergeOneCoroutineWithCancellation`, `ChannelFlowTest::testMergeTwoCoroutinesWithCancellation`, `ChannelFlowTest::testBufferWithTimeout`, `ChannelFlowTest::bufferWithTimeout`, `ChannelFlowTest::testChildCancellation`, `ChannelFlowTest::testClosedPrematurely`, `ChannelFlowTest::testNotClosedPrematurely`, `ChannelFlowTest::testCancelledOnCompletion`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/13 matched
- **TODOs:** 14

### 44. test.CompletableDeferredTest

- **Target:** `tests.CompletableDeferredTest`
- **Similarity:** 0.38
- **Dependents:** 0
- **Priority Score:** 131906.2
- **Functions:** 5/18 matched (target 9)
- **Missing functions:** `CompletableDeferredTest::testFresh`, `CompletableDeferredTest::testComplete`, `CompletableDeferredTest::testCompleteWithIncompleteResult`, `CompletableDeferredTest::testCancelWithException`, `CompletableDeferredTest::testCompleteWithResultOK`, `CompletableDeferredTest::testCompleteWithResultException`, `CompletableDeferredTest::testParentCancelsChild`, `CompletableDeferredTest::testParentActiveOnChildCompletion`, `CompletableDeferredTest::testParentCancelledOnChildException`, `CompletableDeferredTest::testParentActiveOnChildCancellation`, `CompletableDeferredTest::testAwait`, `CompletableDeferredTest::testCancelAndAwaitParentWaitChildren`, `CompletableDeferredTest::testCompleteAndAwaitParentWaitChildren`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/13 matched
- **TODOs:** 6
- **Lint issues:** 2

### 45. internal.CopyOnWriteList

- **Target:** `internal.CopyOnWriteList`
- **Similarity:** 0.01
- **Dependents:** 0
- **Priority Score:** 131609.9
- **Functions:** 2/14 matched (target 4)
- **Missing functions:** `CopyOnWriteList::add`, `CopyOnWriteList::removeAt`, `CopyOnWriteList::iterator`, `CopyOnWriteList::listIterator`, `CopyOnWriteList::listIterator`, `CopyOnWriteList::isEmpty`, `CopyOnWriteList::set`, `CopyOnWriteList::get`, `CopyOnWriteList::IteratorImpl::hasNext`, `CopyOnWriteList::IteratorImpl::next`, `CopyOnWriteList::IteratorImpl::remove`, `CopyOnWriteList::rangeCheck`
- **Types:** 1/2 matched (target 1)
- **Missing types:** `IteratorImpl`

### 46. channels.ChannelUndeliveredElementFailureTest

- **Target:** `channels.ChannelUndeliveredElementFailureTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 131510.0
- **Functions:** 1/14 matched
- **Missing functions:** `ChannelUndeliveredElementFailureTest::testSendCancelledFail`, `ChannelUndeliveredElementFailureTest::testSendSelectCancelledFail`, `ChannelUndeliveredElementFailureTest::testReceiveCancelledFail`, `ChannelUndeliveredElementFailureTest::testReceiveSelectCancelledFail`, `ChannelUndeliveredElementFailureTest::testReceiveCatchingCancelledFail`, `ChannelUndeliveredElementFailureTest::testReceiveOrClosedSelectCancelledFail`, `ChannelUndeliveredElementFailureTest::testHasNextCancelledFail`, `ChannelUndeliveredElementFailureTest::testChannelCancelledFail`, `ChannelUndeliveredElementFailureTest::testFailedHandlerInClosedConflatedChannel`, `ChannelUndeliveredElementFailureTest::testFailedHandlerInClosedBufferedChannel`, `ChannelUndeliveredElementFailureTest::testSendDropOldestInvokeHandlerBuffered`, `ChannelUndeliveredElementFailureTest::testSendDropLatestInvokeHandlerBuffered`, `ChannelUndeliveredElementFailureTest::testSendDropOldestInvokeHandlerConflated`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/13 matched
- **TODOs:** 37

### 47. operators.Lint

- **Target:** `flow.Lint [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 131310.0
- **Functions:** 0/13 matched (target 0)
- **Missing functions:** `SharedFlow<T>::cancellable`, `SharedFlow<T>::flowOn`, `StateFlow<T>::conflate`, `StateFlow<T>::distinctUntilChanged`, `FlowCollector<*>::cancel`, `SharedFlow<T>::catch`, `SharedFlow<T>::retry`, `SharedFlow<T>::retryWhen`, `SharedFlow<T>::toList`, `SharedFlow<T>::toList`, `SharedFlow<T>::toSet`, `SharedFlow<T>::toSet`, `SharedFlow<T>::count`
- **Types:** 0/0 matched (target 2)
- **Missing types:** _none_

### 48. channels.BasicOperationsTest

- **Target:** `channels.BasicOperationsTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 121810.0
- **Functions:** 5/17 matched
- **Missing functions:** `BasicOperationsTest::testSimpleSendReceive`, `BasicOperationsTest::testTrySendToFullChannel`, `BasicOperationsTest::testTrySendAfterClose`, `BasicOperationsTest::testSendAfterClose`, `BasicOperationsTest::testReceiveCatching`, `BasicOperationsTest::testInvokeOnClose`, `BasicOperationsTest::testInvokeOnClosed`, `BasicOperationsTest::testMultipleInvokeOnClose`, `BasicOperationsTest::testIterator`, `BasicOperationsTest::testCancelledChannelInvokeOnClose`, `BasicOperationsTest::testCancelledChannelWithCauseInvokeOnClose`, `BasicOperationsTest::testThrowingInvokeOnClose`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/12 matched
- **TODOs:** 80
- **Lint issues:** 3

### 49. channels.BufferedChannel

- **Target:** `channels.BufferedChannel`
- **Similarity:** 0.26
- **Dependents:** 0
- **Priority Score:** 121707.4
- **Functions:** 100/111 matched (target 180)
- **Missing functions:** `BufferedChannel::sendImpl`, `BufferedChannel::receiveImpl`, `BufferedChannel::cancel`, `BufferedChannel::cancel`, `BufferedChannel::invokeCloseHandler`, `BufferedChannel::toStringDebug`, `BufferedChannel::checkSegmentStructureInvariants`, `BufferedChannel::onCancellationChannelResultImplDoNotCall`, `BufferedChannel::onCancellationImplDoNotCall`, `createSegmentFunction`, `CancellableContinuation<T>::tryResume0`
- **Types:** 6/6 matched (target 7)
- **Missing types:** _none_
- **Lint issues:** 33

### 50. channels.ConsumeTest

- **Target:** `channels.ConsumeTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 121310.0
- **Functions:** 0/12 matched (target 10)
- **Missing functions:** `ConsumeTest::testConsumeJsMiscompilation`, `ConsumeTest::testConsumeClosesOnSuccess`, `ConsumeTest::testConsumeClosesOnFailure`, `ConsumeTest::testConsumeClosesOnEarlyReturn`, `ConsumeTest::f`, `ConsumeTest::testConsumeEachClosesOnSuccess`, `ConsumeTest::testConsumeEachClosesOnFailure`, `ConsumeTest::testConsumeEachClosesOnEarlyReturn`, `ConsumeTest::f`, `ConsumeTest::testConsumeEachExitsOnCancellation`, `ConsumeTest::testConsumeEachThrowingOnChannelClosing`, `ConsumeTest::testBroadcastChannelConsumeJsMiscompilation`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/10 matched
- **TODOs:** 29

### 51. test.RunBlockingTest

- **Target:** `concurrent.RunBlockingTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 121310.0
- **Functions:** 0/12 matched
- **Missing functions:** `RunBlockingTest::testWithTimeoutBusyWait`, `RunBlockingTest::testPrivateEventLoop`, `RunBlockingTest::testOuterEventLoop`, `RunBlockingTest::testOtherDispatcher`, `RunBlockingTest::testCancellation`, `RunBlockingTest::testCancelWithDelay`, `RunBlockingTest::testDispatchOnShutdown`, `RunBlockingTest::testDispatchOnShutdown2`, `RunBlockingTest::testNestedRunBlocking`, `RunBlockingTest::testIncompleteState`, `RunBlockingTest::testCancelledParent`, `RunBlockingTest::testReschedulingDelayedTasks`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/12 matched
- **TODOs:** 52
- **Lint issues:** 7

### 52. test.SupervisorTest

- **Target:** `tests.SupervisorTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 121310.0
- **Functions:** 0/12 matched (target 4)
- **Missing functions:** `SupervisorTest::testSupervisorJob`, `SupervisorTest::testSupervisorScope`, `SupervisorTest::testSupervisorScopeIsolation`, `SupervisorTest::testThrowingSupervisorScope`, `SupervisorTest::testSupervisorThrows`, `SupervisorTest::testSupervisorThrowsWithFailingChild`, `SupervisorTest::testSupervisorScopeExternalCancellation`, `SupervisorTest::testAsyncCancellation`, `SupervisorTest::testSupervisorWithParentCancelNormally`, `SupervisorTest::testSupervisorWithParentCancelException`, `SupervisorTest::testSupervisorScopeCancellationVsException`, `SupervisorTest::testSupervisorJobCancellationException`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/12 matched
- **TODOs:** 46

### 53. operators.BooleanTerminationTest

- **Target:** `operators.BooleanTerminationTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 121310.0
- **Functions:** 0/12 matched
- **Missing functions:** `BooleanTerminationTest::testAnyNominal`, `BooleanTerminationTest::testAnyEmpty`, `BooleanTerminationTest::testAnyInfinite`, `BooleanTerminationTest::testAnyShortCircuit`, `BooleanTerminationTest::testAllNominal`, `BooleanTerminationTest::testAllEmpty`, `BooleanTerminationTest::testAllInfinite`, `BooleanTerminationTest::testAllShortCircuit`, `BooleanTerminationTest::testNoneNominal`, `BooleanTerminationTest::testNoneEmpty`, `BooleanTerminationTest::testNoneInfinite`, `BooleanTerminationTest::testNoneShortCircuit`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/12 matched
- **TODOs:** 39

### 54. test.WithTimeoutTest

- **Target:** `tests.WithTimeoutTest`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 121310.0
- **Functions:** 0/12 matched
- **Missing functions:** `WithTimeoutTest::testBasicNoSuspend`, `WithTimeoutTest::testBasicSuspend`, `WithTimeoutTest::testDispatch`, `WithTimeoutTest::testYieldBlockingWithTimeout`, `WithTimeoutTest::testWithTimeoutChildWait`, `WithTimeoutTest::testBadClass`, `WithTimeoutTest::testExceptionOnTimeout`, `WithTimeoutTest::testSuppressExceptionWithResult`, `WithTimeoutTest::testSuppressExceptionWithAnotherException`, `WithTimeoutTest::testNegativeTimeout`, `WithTimeoutTest::testExceptionFromWithinTimeout`, `WithTimeoutTest::testIncompleteWithTimeoutState`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/12 matched
- **TODOs:** 16
- **Lint issues:** 14

### 55. operators.TransformLatestTest

- **Target:** `operators.TransformLatestTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 121310.0
- **Functions:** 0/12 matched
- **Missing functions:** `TransformLatestTest::testTransformLatest`, `TransformLatestTest::testEmission`, `TransformLatestTest::testSwitchIntuitiveBehaviour`, `TransformLatestTest::testSwitchRendezvousBuffer`, `TransformLatestTest::testSwitchBuffer`, `TransformLatestTest::testHangFlows`, `TransformLatestTest::testEmptyFlow`, `TransformLatestTest::testIsolatedContext`, `TransformLatestTest::testFailureInTransform`, `TransformLatestTest::testFailureDownstream`, `TransformLatestTest::testFailureUpstream`, `TransformLatestTest::testTake`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/12 matched
- **TODOs:** 5

### 56. common.CancellableContinuationImpl

- **Target:** `coroutines.ContinuationState`
- **Similarity:** 0.25
- **Dependents:** 0
- **Priority Score:** 115707.5
- **Functions:** 41/50 matched (target 143)
- **Missing functions:** `CancellableContinuationImpl::callCancelHandlerSafely`, `CancellableContinuationImpl::multipleHandlersError`, `CancellableContinuationImpl::alreadyResumedError`, `CancellableContinuationImpl::getExceptionalResult`, `CancellableContinuationImpl::toString`, `CancellableContinuationImpl::nameString`, `CancelHandler::UserSupplied::invoke`, `CancelHandler::UserSupplied::toString`, `CompletedContinuation::invokeHandlers`
- **Types:** 5/7 matched (target 15)
- **Missing types:** `UserSupplied`, `CompletedContinuation`

### 57. channels.ChannelUndeliveredElementTest

- **Target:** `channels.ChannelUndeliveredElementTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 111710.0
- **Functions:** 4/15 matched (target 18)
- **Missing functions:** `ChannelUndeliveredElementTest::testSendSuccessfully`, `ChannelUndeliveredElementTest::testRendezvousSendCancelled`, `ChannelUndeliveredElementTest::testBufferedSendCancelled`, `ChannelUndeliveredElementTest::testUnlimitedChannelCancelled`, `ChannelUndeliveredElementTest::testConflatedResourceCancelled`, `ChannelUndeliveredElementTest::testSendToClosedChannel`, `ChannelUndeliveredElementTest::testHandlerIsNotInvoked`, `ChannelUndeliveredElementTest::testChannelBufferOverflow`, `ChannelUndeliveredElementTest::testTrySendDoesNotInvokeHandlerOnClosedConflatedChannel`, `ChannelUndeliveredElementTest::testTrySendDoesNotInvokeHandlerOnClosedChannel`, `ChannelUndeliveredElementTest::testTrySendDoesNotInvokeHandler`
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Tests:** 0/11 matched
- **TODOs:** 37
- **Lint issues:** 4

### 58. channels.BufferedChannelTest

- **Target:** `channels.BufferedChannelTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 111310.0
- **Functions:** 1/12 matched
- **Missing functions:** `BufferedChannelTest::testMemoryConsumption`, `BufferedChannelTest::testIteratorHasNextIsIdempotent`, `BufferedChannelTest::testSimple`, `BufferedChannelTest::testClosedBufferedReceiveCatching`, `BufferedChannelTest::testClosedExceptions`, `BufferedChannelTest::testTryOp`, `BufferedChannelTest::testConsumeAll`, `BufferedChannelTest::testCancelWithCause`, `BufferedChannelTest::testBufferSize`, `BufferedChannelTest::testBufferSizeFromTheMiddle`, `BufferedChannelTest::testBufferIsNotPreallocated`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/11 matched
- **TODOs:** 84
- **Lint issues:** 6

### 59. test.CancellableResumeTest

- **Target:** `tests.CancellableResumeTest`
- **Similarity:** 0.06
- **Dependents:** 0
- **Priority Score:** 111209.4
- **Functions:** 0/11 matched (target 1)
- **Missing functions:** `CancellableResumeTest::testResumeImmediateNormally`, `CancellableResumeTest::testResumeImmediateAfterCancel`, `CancellableResumeTest::testResumeImmediateAfterCancelWithHandlerFailure`, `CancellableResumeTest::testResumeImmediateAfterIndirectCancel`, `CancellableResumeTest::testResumeImmediateAfterIndirectCancelWithHandlerFailure`, `CancellableResumeTest::testResumeLaterNormally`, `CancellableResumeTest::testResumeLaterAfterCancel`, `CancellableResumeTest::testResumeLaterAfterCancelWithHandlerFailure`, `CancellableResumeTest::testResumeCancelWhileDispatched`, `CancellableResumeTest::testResumeCancelWhileDispatchedWithHandlerFailure`, `CancellableResumeTest::testResumeUnconfined`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/11 matched
- **TODOs:** 7
- **Lint issues:** 1

### 60. test.DurationToMillisTest

- **Target:** `tests.DurationToMillisTest`
- **Similarity:** 0.41
- **Dependents:** 0
- **Priority Score:** 111205.9
- **Functions:** 0/11 matched (target 13)
- **Missing functions:** `DurationToMillisTest::testNegativeDurationCoercedToZeroMillis`, `DurationToMillisTest::testZeroDurationCoercedToZeroMillis`, `DurationToMillisTest::testOneNanosecondCoercedToOneMillisecond`, `DurationToMillisTest::testOneSecondCoercedTo1000Milliseconds`, `DurationToMillisTest::testMixedComponentDurationRoundedUpToNextMillisecond`, `DurationToMillisTest::testOneExtraNanosecondRoundedUpToNextMillisecond`, `DurationToMillisTest::testInfiniteDurationCoercedToLongMaxValue`, `DurationToMillisTest::testNegativeInfiniteDurationCoercedToZero`, `DurationToMillisTest::testNanosecondOffByOneInfinityDoesNotOverflow`, `DurationToMillisTest::testMillisecondOffByOneInfinityDoesNotIncrement`, `DurationToMillisTest::testOutOfBoundsNanosecondsButFiniteDoesNotIncrement`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/11 matched

### 61. test.CancellableContinuationHandlersTest

- **Target:** `tests.CancellableContinuationHandlersTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 101310.0
- **Functions:** 1/11 matched (target 13)
- **Missing functions:** `CancellableContinuationHandlersTest::testDoubleSubscription`, `CancellableContinuationHandlersTest::testDoubleSubscriptionAfterCompletion`, `CancellableContinuationHandlersTest::testDoubleSubscriptionAfterCompletionWithException`, `CancellableContinuationHandlersTest::testDoubleSubscriptionAfterCancellation`, `CancellableContinuationHandlersTest::testSecondSubscriptionAfterCancellation`, `CancellableContinuationHandlersTest::testSecondSubscriptionAfterResumeCancelAndDispatch`, `CancellableContinuationHandlersTest::testDoubleSubscriptionAfterCancellationWithCause`, `CancellableContinuationHandlersTest::testDoubleSubscriptionMixed`, `CancellableContinuationHandlersTest::testExceptionInHandler`, `CancellableContinuationHandlersTest::testSegmentAsHandler`
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Tests:** 0/10 matched
- **TODOs:** 9
- **Lint issues:** 4

### 62. operators.DistinctUntilChangedTest

- **Target:** `operators.DistinctUntilChangedTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 101310.0
- **Functions:** 1/11 matched (target 12)
- **Missing functions:** `DistinctUntilChangedTest::testDistinctUntilChanged`, `DistinctUntilChangedTest::testDistinctUntilChangedKeySelector`, `DistinctUntilChangedTest::testDistinctUntilChangedAreEquivalent`, `DistinctUntilChangedTest::testDistinctUntilChangedAreEquivalentSingleValue`, `DistinctUntilChangedTest::testThrowingKeySelector`, `DistinctUntilChangedTest::testThrowingAreEquivalent`, `DistinctUntilChangedTest::testDistinctUntilChangedNull`, `DistinctUntilChangedTest::testRepeatedDistinctFusionDefault`, `DistinctUntilChangedTest::testRepeatedDistinctFusionAreEquivalent`, `DistinctUntilChangedTest::testRepeatedDistinctFusionByKey`
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Tests:** 0/10 matched
- **TODOs:** 30
- **Lint issues:** 3

### 63. operators.TakeTest

- **Target:** `operators.TakeTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 101110.0
- **Functions:** 0/10 matched
- **Missing functions:** `TakeTest::testTake`, `TakeTest::testIllegalArgument`, `TakeTest::testTakeSuspending`, `TakeTest::testEmptyFlow`, `TakeTest::testNonPositiveValues`, `TakeTest::testCancelUpstream`, `TakeTest::testErrorCancelsUpstream`, `TakeTest::takeWithRetries`, `TakeTest::testNonIdempotentRetry`, `TakeTest::testNestedTake`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/10 matched
- **TODOs:** 5
- **Lint issues:** 3

### 64. operators.CatchTest

- **Target:** `operators.CatchTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 101110.0
- **Functions:** 0/10 matched
- **Missing functions:** `CatchTest::testCatchEmit`, `CatchTest::testCatchEmitExceptionFromDownstream`, `CatchTest::testCatchEmitAll`, `CatchTest::testCatchEmitAllExceptionFromDownstream`, `CatchTest::testWithTimeoutCatch`, `CatchTest::testCancellationFromUpstreamCatch`, `CatchTest::testCatchContext`, `CatchTest::testUpstreamExceptionConcurrentWithDownstream`, `CatchTest::testUpstreamExceptionConcurrentWithDownstreamCancellation`, `CatchTest::testUpstreamCancellationIsIgnoredWhenDownstreamFails`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/10 matched
- **TODOs:** 36
- **Lint issues:** 19

### 65. channels.BufferedBroadcastChannelTest

- **Target:** `channels.BufferedBroadcastChannelTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 101110.0
- **Functions:** 0/10 matched
- **Missing functions:** `BufferedBroadcastChannelTest::testConcurrentModification`, `BufferedBroadcastChannelTest::testBasic`, `BufferedBroadcastChannelTest::testSendSuspend`, `BufferedBroadcastChannelTest::testConcurrentSendCompletion`, `BufferedBroadcastChannelTest::testForgetUnsubscribed`, `BufferedBroadcastChannelTest::testReceiveFullAfterClose`, `BufferedBroadcastChannelTest::testCloseSubDuringIteration`, `BufferedBroadcastChannelTest::testReceiveFromCancelledSub`, `BufferedBroadcastChannelTest::testCancelWithCause`, `BufferedBroadcastChannelTest::testReceiveNoneAfterCancel`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/10 matched
- **TODOs:** 89
- **Lint issues:** 8

### 66. operators.FlatMapLatestTest

- **Target:** `operators.FlatMapLatestTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 101110.0
- **Functions:** 0/10 matched
- **Missing functions:** `FlatMapLatestTest::testFlatMapLatest`, `FlatMapLatestTest::testEmission`, `FlatMapLatestTest::testSwitchIntuitiveBehaviour`, `FlatMapLatestTest::testSwitchRendevouzBuffer`, `FlatMapLatestTest::testHangFlows`, `FlatMapLatestTest::testEmptyFlow`, `FlatMapLatestTest::testFailureInTransform`, `FlatMapLatestTest::testFailureDownstream`, `FlatMapLatestTest::testFailureUpstream`, `FlatMapLatestTest::testTake`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/10 matched
- **TODOs:** 34
- **Lint issues:** 2

### 67. channels.ChannelBuildersFlowTest

- **Target:** `channels.ChannelBuildersFlowTest`
- **Similarity:** 0.39
- **Dependents:** 0
- **Priority Score:** 101106.1
- **Functions:** 0/10 matched
- **Missing functions:** `ChannelBuildersFlowTest::testChannelConsumeAsFlow`, `ChannelBuildersFlowTest::testChannelReceiveAsFlow`, `ChannelBuildersFlowTest::testConsumeAsFlowCancellation`, `ChannelBuildersFlowTest::testReceiveAsFlowCancellation`, `ChannelBuildersFlowTest::testConsumeAsFlowException`, `ChannelBuildersFlowTest::testReceiveAsFlowException`, `ChannelBuildersFlowTest::testConsumeAsFlowProduceFusing`, `ChannelBuildersFlowTest::testReceiveAsFlowProduceFusing`, `ChannelBuildersFlowTest::testConsumeAsFlowProduceBuffered`, `ChannelBuildersFlowTest::testProduceInAtomicity`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/10 matched
- **TODOs:** 16

### 68. operators.Zip

- **Target:** `flow.Zip`
- **Similarity:** 0.06
- **Dependents:** 0
- **Priority Score:** 91809.4
- **Functions:** 9/18 matched (target 11)
- **Missing functions:** `combine`, `combineTransform`, `combine`, `combineTransform`, `combineUnsafe`, `combineTransformUnsafe`, `nullArrayFactory`, `combine`, `combineTransform`
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 5

### 69. operators.FilterTrivialTest

- **Target:** `operators.FilterTrivialTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 91210.0
- **Functions:** 0/9 matched
- **Missing functions:** `FilterTrivialTest::testFilterNotNull`, `FilterTrivialTest::testEmptyFlowNotNull`, `FilterTrivialTest::testFilterIsInstance`, `FilterTrivialTest::testParametrizedFilterIsInstance`, `FilterTrivialTest::testSubtypesFilterIsInstance`, `FilterTrivialTest::testSubtypesParametrizedFilterIsInstance`, `FilterTrivialTest::testFilterIsInstanceNullable`, `FilterTrivialTest::testEmptyFlowIsInstance`, `FilterTrivialTest::testEmptyFlowParametrizedIsInstance`
- **Types:** 3/3 matched
- **Missing types:** _none_
- **Tests:** 0/9 matched
- **TODOs:** 32

### 70. channels.TestChannelKind

- **Target:** `channels.TestChannelKind [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 91010.0
- **Functions:** 0/8 matched (target 0)
- **Missing functions:** `TestChannelKind::create`, `TestChannelKind::toString`, `ChannelViaBroadcast::receive`, `ChannelViaBroadcast::receiveCatching`, `ChannelViaBroadcast::iterator`, `ChannelViaBroadcast::tryReceive`, `ChannelViaBroadcast::cancel`, `ChannelViaBroadcast::cancel`
- **Types:** 1/2 matched (target 1)
- **Missing types:** `ChannelViaBroadcast`
- **TODOs:** 8
- **Lint issues:** 3

### 71. test.AsyncLazyTest

- **Target:** `tests.AsyncLazyTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 91010.0
- **Functions:** 0/9 matched
- **Missing functions:** `AsyncLazyTest::testSimple`, `AsyncLazyTest::testLazyDeferAndYield`, `AsyncLazyTest::testLazyDeferAndYield2`, `AsyncLazyTest::testSimpleException`, `AsyncLazyTest::testLazyDeferAndYieldException`, `AsyncLazyTest::testCatchException`, `AsyncLazyTest::testStart`, `AsyncLazyTest::testCancelBeforeStart`, `AsyncLazyTest::testCancelWhileComputing`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/9 matched
- **TODOs:** 15
- **Lint issues:** 6

### 72. operators.RetryTest

- **Target:** `operators.RetryTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 91010.0
- **Functions:** 0/9 matched
- **Missing functions:** `RetryTest::testRetryWhen`, `RetryTest::testRetry`, `RetryTest::testRetryPredicate`, `RetryTest::testRetryExceptionFromDownstream`, `RetryTest::testWithTimeoutRetried`, `RetryTest::testCancellationFromUpstreamIsNotRetried`, `RetryTest::testUpstreamExceptionConcurrentWithDownstream`, `RetryTest::testUpstreamExceptionConcurrentWithDownstreamCancellation`, `RetryTest::testUpstreamCancellationIsIgnoredWhenDownstreamFails`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/9 matched
- **TODOs:** 5
- **Lint issues:** 6

### 73. flow.Builders

- **Target:** `flow.FlowBuilders`
- **Similarity:** 0.11
- **Dependents:** 0
- **Priority Score:** 82708.9
- **Functions:** 15/23 matched (target 42)
- **Missing functions:** `Iterable<T>::asFlow`, `Iterator<T>::asFlow`, `Sequence<T>::asFlow`, `IntArray::asFlow`, `LongArray::asFlow`, `IntRange::asFlow`, `LongRange::asFlow`, `ChannelFlowBuilder::toString`
- **Types:** 4/4 matched (target 8)
- **Missing types:** _none_
- **Lint issues:** 15

### 74. sharing.ShareInBufferTest

- **Target:** `sharing.ShareInBufferTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 81010.0
- **Functions:** 1/9 matched
- **Missing functions:** `ShareInBufferTest::testReplay0DefaultBuffer`, `ShareInBufferTest::testReplay1DefaultBuffer`, `ShareInBufferTest::testReplay10DefaultBuffer`, `ShareInBufferTest::testReplay100DefaultBuffer`, `ShareInBufferTest::testDefaultBufferKeepsDefault`, `ShareInBufferTest::testOverrideDefaultBuffer0`, `ShareInBufferTest::testOverrideDefaultBuffer10`, `ShareInBufferTest::testBufferReplaySum`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/8 matched
- **TODOs:** 6
- **Lint issues:** 4

### 75. test.UndispatchedResultTest

- **Target:** `tests.UndispatchedResultTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 80910.0
- **Functions:** 0/8 matched (target 5)
- **Missing functions:** `UndispatchedResultTest::testWithContext`, `UndispatchedResultTest::testWithContextFastPath`, `UndispatchedResultTest::testWithTimeout`, `UndispatchedResultTest::testAsync`, `UndispatchedResultTest::testCoroutineScope`, `UndispatchedResultTest::invokeTest`, `UndispatchedResultTest::invokeTest`, `UndispatchedResultTest::block`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/5 matched
- **TODOs:** 14

### 76. test.AtomicCancellationCommonTest

- **Target:** `tests.AtomicCancellationCommonTest`
- **Similarity:** 0.66
- **Dependents:** 0
- **Priority Score:** 80903.4
- **Functions:** 0/8 matched
- **Missing functions:** `AtomicCancellationCommonTest::testCancellableLaunch`, `AtomicCancellationCommonTest::testAtomicLaunch`, `AtomicCancellationCommonTest::testUndispatchedLaunch`, `AtomicCancellationCommonTest::testUndispatchedLaunchWithUnconfinedContext`, `AtomicCancellationCommonTest::testDeferredAwaitCancellable`, `AtomicCancellationCommonTest::testJobJoinCancellable`, `AtomicCancellationCommonTest::testLockCancellable`, `AtomicCancellationCommonTest::testSelectLockCancellable`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/8 matched
- **TODOs:** 16
- **Lint issues:** 7

### 77. sharing.SharedFlowScenarioTest

- **Target:** `sharing.SharedFlowScenarioTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 73310.0
- **Functions:** 18/25 matched (target 30)
- **Missing functions:** `SharedFlowScenarioTest::testReplay1Extra2`, `SharedFlowScenarioTest::testReplay1`, `SharedFlowScenarioTest::testReplay2Extra2DropOldest`, `SharedFlowScenarioTest::testResumeFastSubscriberOnResumedEmitter`, `SharedFlowScenarioTest::testSuspendedConcurrentEmitAndCancelSubscriberReplay1`, `SharedFlowScenarioTest::testSuspendedConcurrentEmitAndCancelSubscriberReplay1ExtraBuffer1`, `SharedFlowScenarioTest::ScenarioDsl::log`
- **Types:** 8/8 matched
- **Missing types:** _none_
- **Tests:** 0/6 matched
- **TODOs:** 22
- **Lint issues:** 7

### 78. sync.Mutex

- **Target:** `sync.Mutex`
- **Similarity:** 0.23
- **Dependents:** 0
- **Priority Score:** 72007.7
- **Functions:** 11/16 matched (target 20)
- **Missing functions:** `Mutex`, `MutexImpl::CancellableContinuationWithOwner::tryResume`, `MutexImpl::CancellableContinuationWithOwner::resume`, `MutexImpl::SelectInstanceWithOwner::trySelect`, `MutexImpl::SelectInstanceWithOwner::selectInRegistrationPhase`
- **Types:** 2/4 matched (target 2)
- **Missing types:** `CancellableContinuationWithOwner`, `SelectInstanceWithOwner`

### 79. operators.MergeTest

- **Target:** `operators.MergeTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 71310.0
- **Functions:** 3/10 matched
- **Missing functions:** `MergeTest::testMerge`, `MergeTest::testSingle`, `MergeTest::testNulls`, `MergeTest::testContext`, `MergeTest::testOneSourceCancelled`, `MergeTest::testOneSourceCancelledNonFused`, `MergeTest::testIsolatedContext`
- **Types:** 3/3 matched
- **Missing types:** _none_
- **Tests:** 0/7 matched
- **TODOs:** 9

### 80. test.UnconfinedTestDispatcherTest

- **Target:** `tests.UnconfinedTestDispatcherTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 71210.0
- **Functions:** 3/10 matched (target 11)
- **Missing functions:** `UnconfinedTestDispatcherTest::reproducer1742`, `UnconfinedTestDispatcherTest::observe`, `UnconfinedTestDispatcherTest::reproducer2082`, `UnconfinedTestDispatcherTest::reproducer2405`, `UnconfinedTestDispatcherTest::testUnconfinedDispatcher`, `UnconfinedTestDispatcherTest::testEagerlyEnteringChildCoroutines`, `UnconfinedTestDispatcherTest::testSchedulerReuse`
- **Types:** 2/2 matched (target 3)
- **Missing types:** _none_
- **Tests:** 0/6 matched
- **TODOs:** 17
- **Lint issues:** 6

### 81. channels.Broadcast

- **Target:** `channels.Broadcast`
- **Similarity:** 0.14
- **Dependents:** 0
- **Priority Score:** 71208.6
- **Functions:** 3/10 matched (target 11)
- **Missing functions:** `ReceiveChannel<E>::broadcast`, `CoroutineScope::broadcast`, `BroadcastCoroutine::cancel`, `BroadcastCoroutine::cancel`, `BroadcastCoroutine::cancelInternal`, `LazyBroadcastCoroutine::openSubscription`, `LazyBroadcastCoroutine::onStart`
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Lint issues:** 5

### 82. operators.ChunkedTest

- **Target:** `operators.ChunkedTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 70910.0
- **Functions:** 1/8 matched
- **Missing functions:** `ChunkedTest::testChunked`, `ChunkedTest::testEmpty`, `ChunkedTest::testChunkedCancelled`, `ChunkedTest::testChunkedCancelledWithSuspension`, `ChunkedTest::testChunkedDoesNotIgnoreCancellation`, `ChunkedTest::testIae`, `ChunkedTest::testSample`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/7 matched
- **TODOs:** 25

### 83. sync.MutexStressTest

- **Target:** `sync.MutexStressTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 70910.0
- **Functions:** 1/8 matched
- **Missing functions:** `MutexStressTest::testDefaultDispatcher`, `MutexStressTest::testSingleThreadContext`, `MutexStressTest::testMultiThreadedContextWithSingleWorker`, `MutexStressTest::testMultiThreadedContext`, `MutexStressTest::stressUnlockCancelRace`, `MutexStressTest::stressUnlockCancelRaceWithSelect`, `MutexStressTest::testShouldBeUnlockedOnCancellation`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/7 matched
- **TODOs:** 32
- **Lint issues:** 6

### 84. native.Dispatchers

- **Target:** `native.Dispatchers [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 70710.0
- **Functions:** 0/5 matched (target 18)
- **Missing functions:** `Dispatchers::injectMain`, `DefaultIoScheduler::limitedParallelism`, `DefaultIoScheduler::dispatch`, `DefaultIoScheduler::dispatchYield`, `DefaultIoScheduler::toString`
- **Types:** 0/2 matched (target 1)
- **Missing types:** `Dispatchers`, `DefaultIoScheduler`
- **Lint issues:** 1

### 85. native.Builders

- **Target:** `native.Builders [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 70710.0
- **Functions:** 0/5 matched (target 0)
- **Missing functions:** `runBlocking`, `ThreadLocalKeepAlive::addCheck`, `ThreadLocalKeepAlive::keepAlive`, `BlockingCoroutine::afterCompletion`, `BlockingCoroutine::joinBlocking`
- **Types:** 0/2 matched (target 0)
- **Missing types:** `ThreadLocalKeepAlive`, `BlockingCoroutine`

### 86. internal.DispatchedTask

- **Target:** `common.DispatchedTaskDispatch`
- **Similarity:** 0.12
- **Dependents:** 0
- **Priority Score:** 61208.8
- **Functions:** 6/10 matched (target 6)
- **Missing functions:** `DispatchedTask::cancelCompletedResult`, `DispatchedTask::getSuccessfulResult`, `DispatchedTask::getExceptionalResult`, `DispatchedTask<*>::runUnconfinedEventLoop`
- **Types:** 0/2 matched (target 0)
- **Missing types:** `DispatchedTask`, `DispatchException`
- **Lint issues:** 1

### 87. operators.ScanTest

- **Target:** `operators.ScanTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 60810.0
- **Functions:** 1/7 matched
- **Missing functions:** `ScanTest::testScan`, `ScanTest::testScanWithInitial`, `ScanTest::testFoldWithInitial`, `ScanTest::testNulls`, `ScanTest::testEmptyFlow`, `ScanTest::testErrorCancelsUpstream`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/6 matched
- **TODOs:** 6

### 88. channels.ConflatedChannelTest

- **Target:** `channels.ConflatedChannelTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 60810.0
- **Functions:** 1/7 matched
- **Missing functions:** `ConflatedChannelTest::testBasicConflationOfferTryReceive`, `ConflatedChannelTest::testConflatedSend`, `ConflatedChannelTest::testConflatedClose`, `ConflatedChannelTest::testConflationSendReceive`, `ConflatedChannelTest::testConsumeAll`, `ConflatedChannelTest::testCancelWithCause`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/6 matched
- **TODOs:** 30

### 89. sync.SemaphoreStressTest

- **Target:** `sync.SemaphoreStressTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 60710.0
- **Functions:** 0/6 matched
- **Missing functions:** `SemaphoreStressTest::testStressTestAsMutex`, `SemaphoreStressTest::testStress`, `SemaphoreStressTest::testStressAsMutex`, `SemaphoreStressTest::testStressCancellation`, `SemaphoreStressTest::testStressReleaseCancelRace`, `SemaphoreStressTest::testShouldBeUnlockedOnCancellation`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/6 matched
- **TODOs:** 29
- **Lint issues:** 8

### 90. channels.TestBroadcastChannelKind

- **Target:** `channels.TestBroadcastChannelKind [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 60710.0
- **Functions:** 0/6 matched (target 0)
- **Missing functions:** `TestBroadcastChannelKind::create`, `TestBroadcastChannelKind::toString`, `TestBroadcastChannelKind::create`, `TestBroadcastChannelKind::toString`, `TestBroadcastChannelKind::create`, `TestBroadcastChannelKind::toString`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **TODOs:** 5

### 91. test.AtomicCancellationTest

- **Target:** `concurrent.AtomicCancellationTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 60710.0
- **Functions:** 0/6 matched
- **Missing functions:** `AtomicCancellationTest::testSendCancellable`, `AtomicCancellationTest::testSelectSendCancellable`, `AtomicCancellationTest::testReceiveCancellable`, `AtomicCancellationTest::testSelectReceiveCancellable`, `AtomicCancellationTest::testSelectDeferredAwaitCancellable`, `AtomicCancellationTest::testSelectJobJoinCancellable`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/6 matched
- **TODOs:** 38
- **Lint issues:** 11

### 92. operators.OnEmptyTest

- **Target:** `operators.OnEmptyTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 60710.0
- **Functions:** 0/6 matched
- **Missing functions:** `OnEmptyTest::testOnEmptyInvoked`, `OnEmptyTest::testOnEmptyNotInvoked`, `OnEmptyTest::testOnEmptyNotInvokedOnError`, `OnEmptyTest::testOnEmptyNotInvokedOnCancellation`, `OnEmptyTest::testOnEmptyCancellation`, `OnEmptyTest::testTransparencyViolation`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/6 matched
- **TODOs:** 5

### 93. channels.BroadcastTest

- **Target:** `channels.BroadcastTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 60710.0
- **Functions:** 0/6 matched
- **Missing functions:** `BroadcastTest::testBroadcastBasic`, `BroadcastTest::testChannelBroadcastLazyCancel`, `BroadcastTest::testChannelBroadcastLazyClose`, `BroadcastTest::testChannelBroadcastEagerCancel`, `BroadcastTest::testChannelBroadcastEagerClose`, `BroadcastTest::testBroadcastCloseWithException`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/6 matched
- **TODOs:** 55
- **Lint issues:** 3

### 94. channels.ChannelReceiveCatchingTest

- **Target:** `channels.ChannelReceiveCatchingTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 60710.0
- **Functions:** 0/6 matched
- **Missing functions:** `ChannelReceiveCatchingTest::testChannelOfThrowables`, `ChannelReceiveCatchingTest::testNullableIntChanel`, `ChannelReceiveCatchingTest::testUIntChannel`, `ChannelReceiveCatchingTest::testCancelChannel`, `ChannelReceiveCatchingTest::testReceiveResultChannel`, `ChannelReceiveCatchingTest::testToString`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/6 matched
- **TODOs:** 22

### 95. operators.FilterTest

- **Target:** `operators.FilterTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 60710.0
- **Functions:** 0/6 matched
- **Missing functions:** `FilterTest::testFilter`, `FilterTest::testEmptyFlow`, `FilterTest::testErrorCancelsUpstream`, `FilterTest::testFilterNot`, `FilterTest::testEmptyFlowFilterNot`, `FilterTest::testErrorCancelsUpstreamwFilterNot`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/6 matched
- **TODOs:** 23
- **Lint issues:** 8

### 96. flow.SharedFlow

- **Target:** `flow.SharedFlow`
- **Similarity:** 0.35
- **Dependents:** 0
- **Priority Score:** 53606.5
- **Functions:** 26/31 matched (target 48)
- **Missing functions:** `MutableSharedFlow`, `SharedFlowImpl::fuse`, `Array<Any?>::getBufferAt`, `Array<Any?>::setBufferAt`, `SharedFlow<T>::fuseSharedFlow`
- **Types:** 5/5 matched (target 8)
- **Missing types:** _none_
- **Lint issues:** 5

### 97. test.CoroutineDispatcherOperatorFunInvokeTest

- **Target:** `tests.CoroutineDispatcherOperatorFunInvokeTest`
- **Similarity:** 0.22
- **Dependents:** 0
- **Priority Score:** 51007.8
- **Functions:** 3/8 matched (target 12)
- **Missing functions:** `CoroutineDispatcherOperatorFunInvokeTest::testThrowException`, `CoroutineDispatcherOperatorFunInvokeTest::testWithContextChildWaitSameContext`, `CoroutineDispatcherOperatorFunInvokeTest::dispatch`, `CoroutineDispatcherOperatorFunInvokeTest::isDispatchNeeded`, `CoroutineDispatcherOperatorFunInvokeTest::dispatchYield`
- **Types:** 2/2 matched (target 3)
- **Missing types:** _none_
- **Tests:** 0/2 matched
- **TODOs:** 8
- **Lint issues:** 1

### 98. test.CancelledParentAttachTest

- **Target:** `tests.CancelledParentAttachTest`
- **Similarity:** 0.48
- **Dependents:** 0
- **Priority Score:** 51005.2
- **Functions:** 4/9 matched
- **Missing functions:** `CancelledParentAttachTest::testAsync`, `CancelledParentAttachTest::testLaunch`, `CancelledParentAttachTest::testProduce`, `CancelledParentAttachTest::testBroadcast`, `CancelledParentAttachTest::testScopes`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/5 matched
- **TODOs:** 8
- **Lint issues:** 2

### 99. channels.ProduceConsumeTest

- **Target:** `channels.ProduceConsumeTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 50810.0
- **Functions:** 2/7 matched
- **Missing functions:** `ProduceConsumeTest::testRendezvous`, `ProduceConsumeTest::testSmallBuffer`, `ProduceConsumeTest::testMediumBuffer`, `ProduceConsumeTest::testLargeMediumBuffer`, `ProduceConsumeTest::testUnlimited`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/5 matched
- **TODOs:** 24
- **Lint issues:** 2

### 100. common.TestDispatcher

- **Target:** `test.TestDispatcher`
- **Similarity:** 0.05
- **Dependents:** 0
- **Priority Score:** 50809.5
- **Functions:** 2/6 matched (target 8)
- **Missing functions:** `TestDispatcher::processEvent`, `TestDispatcher::timeoutMessage`, `CancellableContinuationRunnable::run`, `cancellableRunnableIsCancelled`
- **Types:** 1/2 matched
- **Missing types:** `CancellableContinuationRunnable`
- **Lint issues:** 6

### 101. common.CompletionState

- **Target:** `coroutines.CompletionState`
- **Similarity:** 0.07
- **Dependents:** 0
- **Priority Score:** 50809.3
- **Functions:** 3/6 matched (target 3)
- **Missing functions:** `CompletedExceptionally::makeHandled`, `CompletedExceptionally::toString`, `CancelledContinuation::makeResumed`
- **Types:** 0/2 matched (target 0)
- **Missing types:** `CompletedExceptionally`, `CancelledContinuation`

### 102. operators.FlatMapMergeTest

- **Target:** `operators.FlatMapMergeTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 50710.0
- **Functions:** 1/6 matched
- **Missing functions:** `FlatMapMergeTest::testFlatMapConcurrency`, `FlatMapMergeTest::testAtomicStart`, `FlatMapMergeTest::testCancellationExceptionDownstream`, `FlatMapMergeTest::testCancellationExceptionUpstream`, `FlatMapMergeTest::testCancellation`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/5 matched
- **TODOs:** 19
- **Lint issues:** 2

### 103. internal.StackTraceRecovery

- **Target:** `internal.CoroutineStackFrame [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 50710.0
- **Functions:** 0/5 matched (target 0)
- **Missing functions:** `recoverStackTrace`, `recoverStackTrace`, `unwrap`, `recoverAndThrow`, `Throwable::initCause`
- **Types:** 2/2 matched
- **Missing types:** _none_

### 104. operators.FlowContextOptimizationsTest

- **Target:** `operators.FlowContextOptimizationsTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 50710.0
- **Functions:** 0/5 matched (target 6)
- **Missing functions:** `FlowContextOptimizationsTest::testBaseline`, `FlowContextOptimizationsTest::testFusedSameContext`, `FlowContextOptimizationsTest::testFusedSameContextWithIntermediateOperators`, `FlowContextOptimizationsTest::testFusedSameDispatcher`, `FlowContextOptimizationsTest::testFusedManySameDispatcher`
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Tests:** 0/5 matched
- **TODOs:** 6
- **Lint issues:** 1

### 105. test.JobExtensionsTest

- **Target:** `tests.JobExtensionsTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 50710.0
- **Functions:** 1/6 matched
- **Missing functions:** `JobExtensionsTest::testIsActive`, `JobExtensionsTest::testIsCompleted`, `JobExtensionsTest::testIsCancelled`, `JobExtensionsTest::testEnsureActiveWithEmptyContext`, `JobExtensionsTest::testJobExtension`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/5 matched
- **TODOs:** 46
- **Lint issues:** 3

### 106. test.CompletableJobTest

- **Target:** `tests.CompletableJobTest`
- **Similarity:** 0.70
- **Dependents:** 0
- **Priority Score:** 50703.0
- **Functions:** 1/6 matched
- **Missing functions:** `CompletableJobTest::testComplete`, `CompletableJobTest::testCompleteWithException`, `CompletableJobTest::testCompleteWithChildren`, `CompletableJobTest::testExceptionIsNotReportedToChildren`, `CompletableJobTest::testCompleteExceptionallyDoesntAffectDeferred`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/5 matched
- **TODOs:** 6
- **Lint issues:** 2

### 107. test.UnconfinedTest

- **Target:** `tests.UnconfinedTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 50610.0
- **Functions:** 0/5 matched
- **Missing functions:** `UnconfinedTest::testOrder`, `UnconfinedTest::testBlockThrows`, `UnconfinedTest::testEnterMultipleTimes`, `UnconfinedTest::testYield`, `UnconfinedTest::testCancellationWihYields`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/5 matched
- **TODOs:** 31
- **Lint issues:** 7

### 108. channels.BroadcastChannelFactoryTest

- **Target:** `channels.BroadcastChannelFactoryTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 50610.0
- **Functions:** 0/5 matched
- **Missing functions:** `BroadcastChannelFactoryTest::testRendezvousChannelNotSupported`, `BroadcastChannelFactoryTest::testUnlimitedChannelNotSupported`, `BroadcastChannelFactoryTest::testConflatedBroadcastChannel`, `BroadcastChannelFactoryTest::testBufferedBroadcastChannel`, `BroadcastChannelFactoryTest::testInvalidCapacityNotSupported`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/5 matched
- **TODOs:** 17

### 109. selects.SelectChannelStressTest

- **Target:** `selects.SelectChannelStressTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 50610.0
- **Functions:** 0/5 matched (target 4)
- **Missing functions:** `SelectChannelStressTest::testSelectSendResourceCleanupBufferedChannel`, `SelectChannelStressTest::testSelectReceiveResourceCleanupBufferedChannel`, `SelectChannelStressTest::testSelectSendResourceCleanupRendezvousChannel`, `SelectChannelStressTest::testSelectReceiveResourceRendezvousChannel`, `SelectChannelStressTest::default`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/4 matched
- **TODOs:** 19
- **Lint issues:** 6

### 110. operators.FlatMapBaseTest

- **Target:** `operators.FlatMapBaseTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 50610.0
- **Functions:** 0/5 matched
- **Missing functions:** `FlatMapBaseTest::testFlatMap`, `FlatMapBaseTest::testSingle`, `FlatMapBaseTest::testNulls`, `FlatMapBaseTest::testContext`, `FlatMapBaseTest::testIsolatedContext`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/5 matched
- **TODOs:** 19
- **Lint issues:** 1

### 111. intrinsics.Undispatched

- **Target:** `intrinsics.Undispatched`
- **Similarity:** 0.02
- **Dependents:** 0
- **Priority Score:** 50609.8
- **Functions:** 1/6 matched (target 1)
- **Missing functions:** `ScopeCoroutine<T>::startUndispatchedOrReturn`, `ScopeCoroutine<T>::startUndispatchedOrReturnIgnoreTimeout`, `ScopeCoroutine<T>::startUndspatched`, `ScopeCoroutine<*>::notOwnTimeout`, `ScopeCoroutine<*>::dispatchExceptionAndMakeCompleting`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 112. internal.ConcurrentLinkedList

- **Target:** `internal.ConcurrentLinkedList`
- **Similarity:** 0.12
- **Dependents:** 0
- **Priority Score:** 41608.8
- **Functions:** 9/13 matched (target 28)
- **Missing functions:** `AtomicRef<S>::moveForward`, `AtomicRef<S>::findSegmentAndMoveForward`, `N::close`, `AtomicInt::addConditionally`
- **Types:** 3/3 matched (target 5)
- **Missing types:** _none_
- **Lint issues:** 3

### 113. flow.NamedDispatchers

- **Target:** `testing.NamedDispatchers`
- **Similarity:** 0.07
- **Dependents:** 0
- **Priority Score:** 41109.3
- **Functions:** 5/9 matched (target 15)
- **Missing functions:** `NamedDispatchers::invoke`, `NamedDispatchers::named`, `NamedDispatchers::dispatch`, `ArrayStack::ensureCapacity`
- **Types:** 2/2 matched (target 3)
- **Missing types:** _none_
- **Lint issues:** 2

### 114. operators.Delay

- **Target:** `flow.Delay`
- **Similarity:** 0.09
- **Dependents:** 0
- **Priority Score:** 41009.1
- **Functions:** 6/10 matched (target 9)
- **Missing functions:** `Flow<T>::debounce`, `Flow<T>::debounce`, `Flow<T>::sample`, `Flow<T>::timeoutInternal`
- **Types:** 0/0 matched (target 1)
- **Missing types:** _none_
- **Lint issues:** 8

### 115. test.StandardTestDispatcherTest

- **Target:** `tests.StandardTestDispatcherTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 40710.0
- **Functions:** 2/6 matched
- **Missing functions:** `StandardTestDispatcherTest::testFlowsNotSkippingValues`, `StandardTestDispatcherTest::testLaunchDispatched`, `StandardTestDispatcherTest::testYield`, `StandardTestDispatcherTest::testSchedulerReuse`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/4 matched
- **TODOs:** 16

### 116. operators.FlatMapMergeFastPathTest

- **Target:** `operators.FlatMapMergeFastPathTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 40610.0
- **Functions:** 1/5 matched
- **Missing functions:** `FlatMapMergeFastPathTest::testFlatMapConcurrency`, `FlatMapMergeFastPathTest::testCancellationExceptionDownstream`, `FlatMapMergeFastPathTest::testCancellationExceptionUpstream`, `FlatMapMergeFastPathTest::testCancellation`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/4 matched
- **TODOs:** 17
- **Lint issues:** 2

### 117. test.DelayTest

- **Target:** `tests.DelayTest`
- **Similarity:** 0.41
- **Dependents:** 0
- **Priority Score:** 40605.9
- **Functions:** 1/5 matched
- **Missing functions:** `DelayTest::testCancellation`, `DelayTest::testMaxLongValue`, `DelayTest::testMaxIntValue`, `DelayTest::testRegularDelay`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/4 matched
- **TODOs:** 6

### 118. test.DelayDurationTest

- **Target:** `tests.DelayDurationTest`
- **Similarity:** 0.53
- **Dependents:** 0
- **Priority Score:** 40604.7
- **Functions:** 1/5 matched
- **Missing functions:** `DelayDurationTest::testCancellation`, `DelayDurationTest::testInfinite`, `DelayDurationTest::testRegularDelay`, `DelayDurationTest::testNanoDelay`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/4 matched
- **TODOs:** 9

### 119. operators.TakeWhileTest

- **Target:** `operators.TakeWhileTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 40510.0
- **Functions:** 0/4 matched
- **Missing functions:** `TakeWhileTest::testTakeWhile`, `TakeWhileTest::testEmptyFlow`, `TakeWhileTest::testCancelUpstream`, `TakeWhileTest::testErrorCancelsUpstream`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/4 matched
- **TODOs:** 5
- **Lint issues:** 3

### 120. operators.DropTest

- **Target:** `operators.DropTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 40510.0
- **Functions:** 0/4 matched
- **Missing functions:** `DropTest::testDrop`, `DropTest::testEmptyFlow`, `DropTest::testNegativeCount`, `DropTest::testErrorCancelsUpstream`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/4 matched
- **TODOs:** 14
- **Lint issues:** 2

### 121. test.JobStatesTest

- **Target:** `tests.JobStatesTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 40510.0
- **Functions:** 0/4 matched
- **Missing functions:** `JobStatesTest::testNormalCompletion`, `JobStatesTest::testCompletingFailed`, `JobStatesTest::testFailed`, `JobStatesTest::testCancelling`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/4 matched
- **TODOs:** 97
- **Lint issues:** 7

### 122. operators.IndexedTest

- **Target:** `operators.IndexedTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 40510.0
- **Functions:** 0/4 matched
- **Missing functions:** `IndexedTest::testWithIndex`, `IndexedTest::testWithIndexEmpty`, `IndexedTest::testCollectIndexed`, `IndexedTest::testCollectIndexedEmptyFlow`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/4 matched
- **TODOs:** 5

### 123. test.LimitedParallelismSharedTest

- **Target:** `tests.LimitedParallelismSharedTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 40510.0
- **Functions:** 0/4 matched (target 3)
- **Missing functions:** `LimitedParallelismSharedTest::testLimitedDefault`, `LimitedParallelismSharedTest::testParallelismSpec`, `LimitedParallelismSharedTest::testLimitedParallelismOfOccasionallyFailingDispatcher`, `LimitedParallelismSharedTest::dispatch`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 24
- **Lint issues:** 3

### 124. internal.FlowScopeTest

- **Target:** `internal.FlowScopeTest`
- **Similarity:** 0.57
- **Dependents:** 0
- **Priority Score:** 40504.3
- **Functions:** 0/4 matched
- **Missing functions:** `FlowScopeTest::testCancellation`, `FlowScopeTest::testCancellationWithChildCancelled`, `FlowScopeTest::testCancellationWithSuspensionPoint`, `FlowScopeTest::testNestedScopes`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/4 matched
- **TODOs:** 10

### 125. internal.Concurrent.common

- **Target:** `internal.Concurrent.common [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 40410.0
- **Functions:** 0/1 matched (target 0)
- **Missing functions:** `WorkaroundAtomicReference<T>::loop`
- **Types:** 0/3 matched (target 0)
- **Missing types:** `ReentrantLock`, `BenignDataRace`, `WorkaroundAtomicReference`

### 126. sync.Semaphore

- **Target:** `sync.Semaphore`
- **Similarity:** 0.26
- **Dependents:** 0
- **Priority Score:** 32507.4
- **Functions:** 18/21 matched (target 45)
- **Missing functions:** `Semaphore`, `SemaphoreAndMutexImpl::acquire`, `SemaphoreAndMutexImpl::acquire`
- **Types:** 4/4 matched (target 9)
- **Missing types:** _none_
- **Lint issues:** 7

### 127. flow.SharingStarted

- **Target:** `flow.SharingStarted`
- **Similarity:** 0.32
- **Dependents:** 0
- **Priority Score:** 31506.8
- **Functions:** 7/10 matched (target 20)
- **Missing functions:** `SharingStarted.Companion::WhileSubscribed`, `StartedWhileSubscribed::equals`, `StartedWhileSubscribed::hashCode`
- **Types:** 5/5 matched (target 7)
- **Missing types:** _none_
- **Lint issues:** 7

### 128. test.LimitedParallelismConcurrentTest

- **Target:** `concurrent.LimitedParallelismConcurrentTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30910.0
- **Functions:** 4/7 matched
- **Missing functions:** `LimitedParallelismConcurrentTest::testLimitedExecutor`, `LimitedParallelismConcurrentTest::testTaskFairness`, `LimitedParallelismConcurrentTest::testNotDoingDispatchesWhenNoTasksArePresent`
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 28
- **Lint issues:** 5

### 129. channels.Produce

- **Target:** `channels.Produce`
- **Similarity:** 0.18
- **Dependents:** 0
- **Priority Score:** 30808.2
- **Functions:** 4/6 matched (target 14)
- **Missing functions:** `CoroutineScope::produce`, `CoroutineScope::produce`
- **Types:** 1/2 matched
- **Missing types:** `ProducerScope`
- **Lint issues:** 1

### 130. operators.TransformWhileTest

- **Target:** `operators.TransformWhileTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30710.0
- **Functions:** 2/5 matched (target 6)
- **Missing functions:** `TransformWhileTest::testSimple`, `TransformWhileTest::testCancelUpstream`, `TransformWhileTest::testExample`
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 6

### 131. test.MultithreadedDispatchersTest

- **Target:** `test.MultithreadedDispatchersTest`
- **Similarity:** 0.31
- **Dependents:** 0
- **Priority Score:** 30606.9
- **Functions:** 1/4 matched
- **Missing functions:** `MultithreadedDispatchersTest::testNotAllocatingExtraDispatchers`, `MultithreadedDispatchersTest::spin`, `MultithreadedDispatchersTest::timeoutsNotPreventingClosing`
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Tests:** 0/2 matched
- **Lint issues:** 4

### 132. flow.StateFlowUpdateCommonTest

- **Target:** `flow.StateFlowUpdateCommonTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30510.0
- **Functions:** 1/4 matched
- **Missing functions:** `StateFlowUpdateCommonTest::testUpdate`, `StateFlowUpdateCommonTest::testUpdateAndGet`, `StateFlowUpdateCommonTest::testGetAndUpdate`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 14
- **Lint issues:** 1

### 133. channels.SendReceiveStressTest

- **Target:** `channels.SendReceiveStressTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30510.0
- **Functions:** 1/4 matched
- **Missing functions:** `SendReceiveStressTest::testBufferedChannel`, `SendReceiveStressTest::testUnlimitedChannel`, `SendReceiveStressTest::testRendezvousChannel`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 16

### 134. channels.ConflatedBroadcastChannelTest

- **Target:** `channels.ConflatedBroadcastChannelTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30510.0
- **Functions:** 1/4 matched
- **Missing functions:** `ConflatedBroadcastChannelTest::testConcurrentModification`, `ConflatedBroadcastChannelTest::testBasicScenario`, `ConflatedBroadcastChannelTest::testInitialValueAndReceiveClosed`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 21

### 135. test.JobStructuredJoinStressTest

- **Target:** `concurrent.JobStructuredJoinStressTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30510.0
- **Functions:** 1/4 matched
- **Missing functions:** `JobStructuredJoinStressTest::testStressRegularJoin`, `JobStructuredJoinStressTest::testStressSuspendCancellable`, `JobStructuredJoinStressTest::testStressSuspendCancellableReusable`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 22
- **Lint issues:** 1

### 136. flow.FlowCancellationTest

- **Target:** `flow.FlowCancellationTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30410.0
- **Functions:** 0/3 matched
- **Missing functions:** `FlowCancellationTest::testEmitIsCooperative`, `FlowCancellationTest::testIsActiveOnCurrentContext`, `FlowCancellationTest::testFlowWithEmptyContext`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 21

### 137. sharing.ShareInFusionTest

- **Target:** `sharing.ShareInFusionTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30410.0
- **Functions:** 0/3 matched
- **Missing functions:** `ShareInFusionTest::testOperatorFusion`, `ShareInFusionTest::testFlowOnContextFusion`, `ShareInFusionTest::testChannelFlowBufferShareIn`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 8

### 138. operators.FlatMapMergeBaseTest

- **Target:** `operators.FlatMapMergeBaseTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30410.0
- **Functions:** 0/3 matched
- **Missing functions:** `FlatMapMergeBaseTest::testFailureCancellation`, `FlatMapMergeBaseTest::testConcurrentFailure`, `FlatMapMergeBaseTest::testFailureInMapOperationCancellation`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 16
- **Lint issues:** 2

### 139. operators.OnEachTest

- **Target:** `operators.OnEachTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30410.0
- **Functions:** 0/3 matched
- **Missing functions:** `OnEachTest::testOnEach`, `OnEachTest::testEmptyFlow`, `OnEachTest::testErrorCancelsUpstream`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 5

### 140. channels.UnlimitedChannelTest

- **Target:** `channels.UnlimitedChannelTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30410.0
- **Functions:** 0/3 matched
- **Missing functions:** `UnlimitedChannelTest::testBasic`, `UnlimitedChannelTest::testConsumeAll`, `UnlimitedChannelTest::testCancelWithCause`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 28

### 141. operators.DropWhileTest

- **Target:** `operators.DropWhileTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30410.0
- **Functions:** 0/3 matched
- **Missing functions:** `DropWhileTest::testDropWhile`, `DropWhileTest::testEmptyFlow`, `DropWhileTest::testErrorCancelsUpstream`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 13
- **Lint issues:** 4

### 142. operators.MapTest

- **Target:** `operators.MapTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30410.0
- **Functions:** 0/3 matched
- **Missing functions:** `MapTest::testMap`, `MapTest::testEmptyFlow`, `MapTest::testErrorCancelsUpstream`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 5

### 143. operators.MapNotNullTest

- **Target:** `operators.MapNotNullTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30410.0
- **Functions:** 0/3 matched
- **Missing functions:** `MapNotNullTest::testMap`, `MapNotNullTest::testEmptyFlow`, `MapNotNullTest::testErrorCancelsUpstream`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 7

### 144. test.FailedJobTest

- **Target:** `tests.FailedJobTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30410.0
- **Functions:** 0/3 matched
- **Missing functions:** `FailedJobTest::testCancelledJob`, `FailedJobTest::testFailedJob`, `FailedJobTest::testFailedChildJob`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 32
- **Lint issues:** 2

### 145. test.AbstractDispatcherConcurrencyTest

- **Target:** `concurrent.AbstractDispatcherConcurrencyTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30410.0
- **Functions:** 0/3 matched
- **Missing functions:** `AbstractDispatcherConcurrencyTest::testLaunchAndJoin`, `AbstractDispatcherConcurrencyTest::testDispatcherHasOwnThreads`, `AbstractDispatcherConcurrencyTest::testDelayInDispatcher`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 22

### 146. test.DispatchedContinuationTest

- **Target:** `tests.DispatchedContinuationTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30410.0
- **Functions:** 0/3 matched
- **Missing functions:** `DispatchedContinuationTest::testCancelThenResume`, `DispatchedContinuationTest::testCancelThenResumeUnconfined`, `DispatchedContinuationTest::testResumeThenCancel`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 39
- **Lint issues:** 3

### 147. test.LaunchLazyTest

- **Target:** `tests.LaunchLazyTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30410.0
- **Functions:** 0/3 matched
- **Missing functions:** `LaunchLazyTest::testLaunchAndYieldJoin`, `LaunchLazyTest::testStart`, `LaunchLazyTest::testInvokeOnCompletionAndStart`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 38
- **Lint issues:** 3

### 148. test.CommonThreadLocalTest

- **Target:** `concurrent.CommonThreadLocalTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 30410.0
- **Functions:** 0/3 matched
- **Missing functions:** `CommonThreadLocalTest::testThreadLocalBeingThreadLocal`, `CommonThreadLocalTest::testThreadLocalWithNullableType`, `CommonThreadLocalTest::testThreadLocalsWithDifferentNamesNotInterfering`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **TODOs:** 21
- **Lint issues:** 4

### 149. internal.SystemProps.common

- **Target:** `internal.SystemProps.common`
- **Similarity:** 0.02
- **Dependents:** 0
- **Priority Score:** 30409.8
- **Functions:** 1/4 matched (target 5)
- **Missing functions:** `systemProp`, `systemProp`, `systemProp`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 150. test.WorkerTest

- **Target:** `test.WorkerTest`
- **Similarity:** 0.72
- **Dependents:** 0
- **Priority Score:** 30402.8
- **Functions:** 0/3 matched
- **Missing functions:** `WorkerTest::testLaunchInWorker`, `WorkerTest::testLaunchInWorkerThroughGlobalScope`, `WorkerTest::testRunBlockingInTerminatedWorker`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/3 matched
- **Lint issues:** 4

### 151. internal.LockFreeTaskQueue

- **Target:** `internal.LockFreeTaskQueue`
- **Similarity:** 0.33
- **Dependents:** 0
- **Priority Score:** 22506.7
- **Functions:** 19/21 matched (target 27)
- **Missing functions:** `LockFreeTaskQueueCore::wo`, `LockFreeTaskQueueCore::withState`
- **Types:** 4/4 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 152. flow.StateFlow

- **Target:** `flow.StateFlow`
- **Similarity:** 0.25
- **Dependents:** 0
- **Priority Score:** 22307.5
- **Functions:** 17/19 matched (target 47)
- **Missing functions:** `MutableStateFlow`, `StateFlowImpl::fuse`
- **Types:** 4/4 matched (target 7)
- **Missing types:** _none_
- **Lint issues:** 5

### 153. operators.Share

- **Target:** `flow.Share`
- **Similarity:** 0.18
- **Dependents:** 0
- **Priority Score:** 21808.2
- **Functions:** 12/13 matched (target 49)
- **Missing functions:** `SubscribedFlowCollector::onSubscription`
- **Types:** 4/5 matched (target 10)
- **Missing types:** `SubscribedFlowCollector`
- **Lint issues:** 4

### 154. test.AbstractCoroutineTest

- **Target:** `tests.AbstractCoroutineTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 21110.0
- **Functions:** 8/10 matched
- **Missing functions:** `AbstractCoroutineTest::testNotifications`, `AbstractCoroutineTest::testNotificationsWithException`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/2 matched
- **TODOs:** 9
- **Lint issues:** 4

### 155. operators.Emitters

- **Target:** `flow.Emitters`
- **Similarity:** 0.06
- **Dependents:** 0
- **Priority Score:** 20909.4
- **Functions:** 6/8 matched (target 16)
- **Missing functions:** `Flow<T>::unsafeTransform`, `FlowCollector<T>::invokeSafely`
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_
- **Lint issues:** 2

### 156. operators.Context

- **Target:** `flow.Context`
- **Similarity:** 0.09
- **Dependents:** 0
- **Priority Score:** 20909.1
- **Functions:** 6/7 matched
- **Missing functions:** `Flow<T>::buffer`
- **Types:** 1/2 matched (target 1)
- **Missing types:** `CancellableFlow`
- **Lint issues:** 4

### 157. common.CompletableDeferred

- **Target:** `coroutines.CompletableDeferred`
- **Similarity:** 0.16
- **Dependents:** 0
- **Priority Score:** 20908.4
- **Functions:** 5/7 matched (target 16)
- **Missing functions:** `CompletableDeferred`, `CompletableDeferred`
- **Types:** 2/2 matched (target 3)
- **Missing types:** _none_

### 158. channels.Channels.common

- **Target:** `channels.Channels.common`
- **Similarity:** 0.04
- **Dependents:** 0
- **Priority Score:** 20609.6
- **Functions:** 4/6 matched (target 21)
- **Missing functions:** `ReceiveChannel<E>::receiveOrNull`, `ReceiveChannel<E>::onReceiveOrNull`
- **Types:** 0/0 matched (target 3)
- **Missing types:** _none_
- **Lint issues:** 2

### 159. operators.Errors

- **Target:** `flow.Errors`
- **Similarity:** 0.07
- **Dependents:** 0
- **Priority Score:** 20609.3
- **Functions:** 4/6 matched (target 13)
- **Missing functions:** `Throwable::isCancellationCause`, `Throwable::isSameExceptionAs`
- **Types:** 0/0 matched (target 1)
- **Missing types:** _none_

### 160. channels.ChannelCoroutine

- **Target:** `channels.ChannelCoroutine`
- **Similarity:** 0.29
- **Dependents:** 0
- **Priority Score:** 20507.1
- **Functions:** 2/4 matched (target 18)
- **Missing functions:** `ChannelCoroutine::cancel`, `ChannelCoroutine::cancel`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 161. internal.NamedDispatcher

- **Target:** `internal.NamedDispatcher`
- **Similarity:** 0.39
- **Dependents:** 0
- **Priority Score:** 20506.1
- **Functions:** 2/4 matched
- **Missing functions:** `NamedDispatcher::isDispatchNeeded`, `NamedDispatcher::dispatchYield`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 162. operators.FlattenConcatTest

- **Target:** `operators.FlattenConcatTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 20410.0
- **Functions:** 1/3 matched
- **Missing functions:** `FlattenConcatTest::testFlatMapConcurrency`, `FlattenConcatTest::testCancellation`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/2 matched
- **TODOs:** 10

### 163. operators.FlattenMergeTest

- **Target:** `operators.FlattenMergeTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 20410.0
- **Functions:** 1/3 matched
- **Missing functions:** `FlattenMergeTest::testFlatMapConcurrency`, `FlattenMergeTest::testContextPreservationAcrossFlows`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/2 matched
- **TODOs:** 7

### 164. flow.IdFlowTest

- **Target:** `flow.IdFlowTest`
- **Similarity:** 0.30
- **Dependents:** 0
- **Priority Score:** 20407.0
- **Functions:** 1/3 matched
- **Missing functions:** `IdFlowTest::testCancelInCollect`, `IdFlowTest::testCancelInFlow`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/2 matched
- **TODOs:** 9

### 165. flow.CombineStressTest

- **Target:** `flow.CombineStressTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 20310.0
- **Functions:** 0/2 matched
- **Missing functions:** `CombineStressTest::testCancellation`, `CombineStressTest::testFailure`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/2 matched
- **TODOs:** 18

### 166. operators.CancellableTest

- **Target:** `operators.CancellableTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 20310.0
- **Functions:** 0/2 matched
- **Missing functions:** `CancellableTest::testCancellable`, `CancellableTest::testFastPath`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/2 matched
- **TODOs:** 8

### 167. channels.ChannelBufferOverflowTest

- **Target:** `channels.ChannelBufferOverflowTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 20310.0
- **Functions:** 0/2 matched
- **Missing functions:** `ChannelBufferOverflowTest::testDropLatest`, `ChannelBufferOverflowTest::testDropOldest`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/2 matched
- **TODOs:** 30

### 168. operators.OnStartTest

- **Target:** `operators.OnStartTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 20310.0
- **Functions:** 0/2 matched
- **Missing functions:** `OnStartTest::testEmitExample`, `OnStartTest::testTransparencyViolation`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/2 matched
- **TODOs:** 5

### 169. channels.TrySendBlockingTest

- **Target:** `channels.TrySendBlockingTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 20310.0
- **Functions:** 0/2 matched
- **Missing functions:** `TrySendBlockingTest::testTrySendBlocking`, `TrySendBlockingTest::testTrySendBlockingClosedChannel`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/2 matched
- **TODOs:** 15
- **Lint issues:** 2

### 170. channels.FlowCallbackTest

- **Target:** `channels.FlowCallbackTest`
- **Similarity:** 0.38
- **Dependents:** 0
- **Priority Score:** 20306.2
- **Functions:** 0/2 matched
- **Missing functions:** `FlowCallbackTest::testClosedPrematurely`, `FlowCallbackTest::testNotClosedPrematurely`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/2 matched
- **TODOs:** 11

### 171. test.CoroutineExceptionHandlerTest

- **Target:** `tests.CoroutineExceptionHandlerTest`
- **Similarity:** 0.73
- **Dependents:** 0
- **Priority Score:** 20302.7
- **Functions:** 0/2 matched
- **Missing functions:** `CoroutineExceptionHandlerTest::testJob`, `CoroutineExceptionHandlerTest::testCompletableDeferred`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/2 matched
- **TODOs:** 6
- **Lint issues:** 2

### 172. common.TestCoroutineScheduler

- **Target:** `tests.TestCoroutineScheduler [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 12210.0
- **Functions:** 18/19 matched (target 26)
- **Missing functions:** `TestCoroutineScheduler::read`
- **Types:** 3/3 matched (target 4)
- **Missing types:** _none_
- **TODOs:** 11
- **Lint issues:** 6

### 173. common.Builders.common

- **Target:** `coroutines.Builders.common`
- **Similarity:** 0.19
- **Dependents:** 0
- **Priority Score:** 12008.1
- **Functions:** 14/14 matched (target 55)
- **Missing functions:** _none_
- **Types:** 5/6 matched (target 9)
- **Missing types:** `UndispatchedCoroutine`
- **Lint issues:** 8

### 174. channels.BroadcastChannel

- **Target:** `channels.BroadcastChannel`
- **Similarity:** 0.15
- **Dependents:** 0
- **Priority Score:** 11608.5
- **Functions:** 10/11 matched (target 37)
- **Missing functions:** `BroadcastChannel`
- **Types:** 5/5 matched (target 6)
- **Missing types:** _none_
- **Lint issues:** 8

### 175. operators.Transform

- **Target:** `flow.Transform`
- **Similarity:** 0.07
- **Dependents:** 0
- **Priority Score:** 11309.3
- **Functions:** 12/13 matched (target 100)
- **Missing functions:** `Flow<*>::filterIsInstance`
- **Types:** 0/0 matched (target 13)
- **Missing types:** _none_
- **Lint issues:** 10

### 176. internal.LimitedDispatcher

- **Target:** `internal.LimitedDispatcher`
- **Similarity:** 0.27
- **Dependents:** 0
- **Priority Score:** 11207.3
- **Functions:** 9/10 matched (target 12)
- **Missing functions:** `CoroutineDispatcher::namedOrThis`
- **Types:** 2/2 matched (target 3)
- **Missing types:** _none_
- **Lint issues:** 13

### 177. common.LaunchFlow

- **Target:** `tests.LaunchFlow [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10910.0
- **Functions:** 5/6 matched (target 7)
- **Missing functions:** `LaunchFlowBuilder::catch`
- **Types:** 3/3 matched
- **Missing types:** _none_
- **TODOs:** 17
- **Lint issues:** 1

### 178. operators.Merge

- **Target:** `flow.Merge`
- **Similarity:** 0.08
- **Dependents:** 0
- **Priority Score:** 10909.2
- **Functions:** 8/9 matched (target 23)
- **Missing functions:** `Iterable<Flow<T>>::merge`
- **Types:** 0/0 matched (target 3)
- **Missing types:** _none_
- **Lint issues:** 4

### 179. channels.ConflatedBufferedChannel

- **Target:** `channels.ConflatedBufferedChannel`
- **Similarity:** 0.37
- **Dependents:** 0
- **Priority Score:** 10806.3
- **Functions:** 6/7 matched (target 9)
- **Missing functions:** `ConflatedBufferedChannel::registerSelectForSend`
- **Types:** 1/1 matched
- **Missing types:** _none_

### 180. test.ConcurrentExceptionsStressTest

- **Target:** `concurrent.ConcurrentExceptionsStressTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10610.0
- **Functions:** 3/4 matched (target 5)
- **Missing functions:** `ConcurrentExceptionsStressTest::testStress`
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Tests:** 0/1 matched
- **TODOs:** 19
- **Lint issues:** 5

### 181. operators.Distinct

- **Target:** `flow.Distinct`
- **Similarity:** 0.04
- **Dependents:** 0
- **Priority Score:** 10609.6
- **Functions:** 4/5 matched (target 12)
- **Missing functions:** `Flow<T>::distinctUntilChangedBy`
- **Types:** 1/1 matched (target 5)
- **Missing types:** _none_
- **Lint issues:** 2

### 182. internal.SafeCollector.common

- **Target:** `internal.SafeCollector.common`
- **Similarity:** 0.12
- **Dependents:** 0
- **Priority Score:** 10508.8
- **Functions:** 3/4 matched (target 12)
- **Missing functions:** `SafeCollector<*>::checkContext`
- **Types:** 1/1 matched (target 3)
- **Missing types:** _none_
- **Lint issues:** 1

### 183. channels.ChannelCancelUndeliveredElementStressTest

- **Target:** `channels.ChannelCancelUndeliveredElementStressTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10410.0
- **Functions:** 2/3 matched
- **Missing functions:** `ChannelCancelUndeliveredElementStressTest::testStress`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/1 matched
- **TODOs:** 20
- **Lint issues:** 3

### 184. selects.OnTimeout

- **Target:** `selects.OnTimeout`
- **Similarity:** 0.03
- **Dependents:** 0
- **Priority Score:** 10409.7
- **Functions:** 2/3 matched (target 6)
- **Missing functions:** `OnTimeout::register`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 6

### 185. flow.SafeFlowTest

- **Target:** `flow.SafeFlowTest`
- **Similarity:** 0.28
- **Dependents:** 0
- **Priority Score:** 10407.2
- **Functions:** 2/3 matched
- **Missing functions:** `SafeFlowTest::testEmissionsFromDifferentStateMachine`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/1 matched
- **TODOs:** 8

### 186. test.MainDispatcherTest

- **Target:** `test.MainDispatcherTest`
- **Similarity:** 0.66
- **Dependents:** 0
- **Priority Score:** 10403.4
- **Functions:** 2/3 matched (target 2)
- **Missing functions:** `MainDispatcherTest::scheduleOnMainQueue`
- **Types:** 1/1 matched
- **Missing types:** _none_

### 187. channels.ConflatedBroadcastChannelNotifyStressTest

- **Target:** `channels.ConflatedBroadcastChannelNotifyStressTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10310.0
- **Functions:** 1/2 matched
- **Missing functions:** `ConflatedBroadcastChannelNotifyStressTest::testStressNotify`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/1 matched
- **TODOs:** 18
- **Lint issues:** 1

### 188. operators.FlatMapConcatTest

- **Target:** `operators.FlatMapConcatTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10310.0
- **Functions:** 1/2 matched
- **Missing functions:** `FlatMapConcatTest::testFlatMapConcurrency`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/1 matched
- **TODOs:** 6

### 189. internal.InlineList

- **Target:** `internal.InlineList`
- **Similarity:** 0.10
- **Dependents:** 0
- **Priority Score:** 10309.0
- **Functions:** 1/2 matched (target 5)
- **Missing functions:** `InlineList::plus`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 3

### 190. test.BuilderContractsTest

- **Target:** `tests.BuilderContractsTest`
- **Similarity:** 0.61
- **Dependents:** 0
- **Priority Score:** 10303.9
- **Functions:** 1/2 matched
- **Missing functions:** `BuilderContractsTest::testContracts`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/1 matched
- **TODOs:** 9
- **Lint issues:** 2

### 191. operators.TransformTest

- **Target:** `operators.TransformTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10210.0
- **Functions:** 0/1 matched
- **Missing functions:** `TransformTest::testDoubleEmit`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/1 matched
- **TODOs:** 5

### 192. operators.LintTest

- **Target:** `operators.LintTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10210.0
- **Functions:** 0/1 matched
- **Missing functions:** `LintTest::testSharedFlowToCollection`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/1 matched
- **TODOs:** 5

### 193. channels.BroadcastChannelSubStressTest

- **Target:** `channels.BroadcastChannelSubStressTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10210.0
- **Functions:** 0/1 matched
- **Missing functions:** `BroadcastChannelSubStressTest::testStress`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/1 matched
- **TODOs:** 17
- **Lint issues:** 1

### 194. flow.StateFlowCommonStressTest

- **Target:** `flow.StateFlowCommonStressTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10210.0
- **Functions:** 0/1 matched
- **Missing functions:** `StateFlowCommonStressTest::testSingleEmitterAndCollector`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/1 matched
- **TODOs:** 14
- **Lint issues:** 2

### 195. selects.SelectMutexStressTest

- **Target:** `selects.SelectMutexStressTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10210.0
- **Functions:** 0/1 matched
- **Missing functions:** `SelectMutexStressTest::testSelectCancelledResourceRelease`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/1 matched
- **TODOs:** 12
- **Lint issues:** 1

### 196. operators.ConflateTest

- **Target:** `operators.ConflateTest [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10210.0
- **Functions:** 0/1 matched
- **Missing functions:** `ConflateTest::testExample`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/1 matched
- **TODOs:** 7
- **Lint issues:** 1

### 197. channels.Channels

- **Target:** `channels.Channels`
- **Similarity:** 0.03
- **Dependents:** 0
- **Priority Score:** 10209.7
- **Functions:** 1/2 matched (target 1)
- **Missing functions:** `SendChannel<E>::sendBlocking`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 198. test.MultithreadedDispatcherStressTest

- **Target:** `concurrent.MultithreadedDispatcherStressTest`
- **Similarity:** 0.33
- **Dependents:** 0
- **Priority Score:** 10206.7
- **Functions:** 0/1 matched
- **Missing functions:** `MultithreadedDispatcherStressTest::testClosingNotDroppingTasks`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/1 matched
- **TODOs:** 9
- **Lint issues:** 2

### 199. test.AwaitCancellationTest

- **Target:** `tests.AwaitCancellationTest`
- **Similarity:** 0.53
- **Dependents:** 0
- **Priority Score:** 10204.7
- **Functions:** 0/1 matched
- **Missing functions:** `AwaitCancellationTest::testCancellation`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/1 matched
- **TODOs:** 7

### 200. test.DelayExceptionTest

- **Target:** `test.DelayExceptionTest`
- **Similarity:** 0.70
- **Dependents:** 0
- **Priority Score:** 10203.0
- **Functions:** 0/1 matched
- **Missing functions:** `DelayExceptionTest::testMaxDelay`
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Tests:** 0/1 matched

### 201. native.Debug

- **Target:** `native.Debug`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10110.0
- **Functions:** 0/1 matched (target 3)
- **Missing functions:** `assert`
- **Types:** 0/0 matched
- **Missing types:** _none_

### 202. native.SchedulerTask

- **Target:** `native.SchedulerTask`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 10100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 0/1 matched (target 0)
- **Missing types:** `SchedulerTask`

### 203. native.CloseableCoroutineDispatcher

- **Target:** `native.CloseableCoroutineDispatcher`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 10100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 0/1 matched (target 0)
- **Missing types:** `CloseableCoroutineDispatcher`

### 204. internal.LocalAtomics.common

- **Target:** `internal.LocalAtomics.common`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 10100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 0/1 matched (target 0)
- **Missing types:** `LocalAtomicInt`

### 205. internal.ThreadSafeHeap

- **Target:** `internal.ThreadSafeHeap`
- **Similarity:** 0.38
- **Dependents:** 0
- **Priority Score:** 1606.2
- **Functions:** 14/14 matched (target 20)
- **Missing functions:** _none_
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 206. terminal.Reduce

- **Target:** `flow.Reduce`
- **Similarity:** 0.03
- **Dependents:** 0
- **Priority Score:** 1009.7
- **Functions:** 10/10 matched (target 64)
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 6)
- **Missing types:** _none_
- **Lint issues:** 9

### 207. selects.SelectOld

- **Target:** `selects.SelectOld`
- **Similarity:** 0.22
- **Dependents:** 0
- **Priority Score:** 1007.8
- **Functions:** 8/8 matched (target 10)
- **Missing functions:** _none_
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Lint issues:** 5

### 208. common.AbstractCoroutine

- **Target:** `coroutines.AbstractCoroutine`
- **Similarity:** 0.39
- **Dependents:** 0
- **Priority Score:** 1006.1
- **Functions:** 9/9 matched (target 16)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 4

### 209. flow.VirtualTime

- **Target:** `flow.VirtualTime`
- **Similarity:** 0.51
- **Dependents:** 0
- **Priority Score:** 904.9
- **Functions:** 7/7 matched (target 11)
- **Missing functions:** _none_
- **Types:** 2/2 matched
- **Missing types:** _none_
- **TODOs:** 10
- **Lint issues:** 2

### 210. operators.Limit

- **Target:** `flow.Limit`
- **Similarity:** 0.05
- **Dependents:** 0
- **Priority Score:** 809.5
- **Functions:** 8/8 matched (target 44)
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 4)
- **Missing types:** _none_
- **Lint issues:** 9

### 211. internal.OnDemandAllocatingPool

- **Target:** `internal.OnDemandAllocatingPool`
- **Similarity:** 0.24
- **Dependents:** 0
- **Priority Score:** 807.6
- **Functions:** 7/7 matched (target 8)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 212. common.Delay

- **Target:** `coroutines.Delay`
- **Similarity:** 0.28
- **Dependents:** 0
- **Priority Score:** 807.2
- **Functions:** 6/6 matched (target 11)
- **Missing functions:** _none_
- **Types:** 2/2 matched
- **Missing types:** _none_

### 213. selects.SelectUnbiased

- **Target:** `selects.SelectUnbiased`
- **Similarity:** 0.16
- **Dependents:** 0
- **Priority Score:** 708.4
- **Functions:** 6/6 matched (target 8)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 214. common.Supervisor

- **Target:** `coroutines.Supervisor`
- **Similarity:** 0.25
- **Dependents:** 0
- **Priority Score:** 707.5
- **Functions:** 5/5 matched (target 8)
- **Missing functions:** _none_
- **Types:** 2/2 matched (target 3)
- **Missing types:** _none_
- **Lint issues:** 3

### 215. common.Unconfined

- **Target:** `coroutines.Unconfined`
- **Similarity:** 0.47
- **Dependents:** 0
- **Priority Score:** 605.3
- **Functions:** 4/4 matched (target 7)
- **Missing functions:** _none_
- **Types:** 2/2 matched (target 3)
- **Missing types:** _none_
- **Lint issues:** 7

### 216. intrinsics.Cancellable

- **Target:** `intrinsics.Cancellable`
- **Similarity:** 0.15
- **Dependents:** 0
- **Priority Score:** 508.5
- **Functions:** 5/5 matched (target 19)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 217. common.Exceptions.common

- **Target:** `coroutines.Exceptions [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 410.0
- **Functions:** 0/0 matched (target 11)
- **Missing functions:** _none_
- **Types:** 4/4 matched (target 7)
- **Missing types:** _none_
- **Lint issues:** 6

### 218. common.MainCoroutineDispatcher

- **Target:** `coroutines.MainCoroutineDispatcher`
- **Similarity:** 0.08
- **Dependents:** 0
- **Priority Score:** 409.2
- **Functions:** 3/3 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 219. internal.FlowCoroutine

- **Target:** `internal.FlowCoroutine`
- **Similarity:** 0.19
- **Dependents:** 0
- **Priority Score:** 408.1
- **Functions:** 3/3 matched (target 8)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 220. internal.OnUndeliveredElement

- **Target:** `internal.OnUndeliveredElement`
- **Similarity:** 0.32
- **Dependents:** 0
- **Priority Score:** 406.8
- **Functions:** 2/2 matched (target 3)
- **Missing functions:** _none_
- **Types:** 2/2 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 221. terminal.Logic

- **Target:** `flow.Logic`
- **Similarity:** 0.02
- **Dependents:** 0
- **Priority Score:** 309.8
- **Functions:** 3/3 matched (target 23)
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 4)
- **Missing types:** _none_
- **Lint issues:** 5

### 222. terminal.Collection

- **Target:** `flow.Collection`
- **Similarity:** 0.11
- **Dependents:** 0
- **Priority Score:** 308.9
- **Functions:** 3/3 matched (target 53)
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 9)
- **Missing types:** _none_
- **Lint issues:** 1

### 223. test.DefaultDispatchersConcurrencyTest

- **Target:** `concurrent.DefaultDispatchersConcurrencyTest [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 210.0
- **Functions:** 0/0 matched (target 2)
- **Missing functions:** _none_
- **Types:** 2/2 matched
- **Missing types:** _none_
- **TODOs:** 4

### 224. common.TestDispatchers

- **Target:** `tests.TestDispatchers [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 210.0
- **Functions:** 2/2 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **TODOs:** 7

### 225. test.EmptyContext

- **Target:** `tests.EmptyContext [STUB]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 210.0
- **Functions:** 2/2 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **TODOs:** 12
- **Lint issues:** 1

### 226. terminal.Count

- **Target:** `flow.Count`
- **Similarity:** 0.03
- **Dependents:** 0
- **Priority Score:** 209.7
- **Functions:** 2/2 matched (target 26)
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 2)
- **Missing types:** _none_
- **Lint issues:** 2

### 227. internal.MainDispatcherFactory

- **Target:** `internal.MainDispatcherFactory`
- **Similarity:** 0.07
- **Dependents:** 0
- **Priority Score:** 209.3
- **Functions:** 1/1 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 228. internal.NopCollector

- **Target:** `internal.NopCollector`
- **Similarity:** 0.08
- **Dependents:** 0
- **Priority Score:** 209.2
- **Functions:** 1/1 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 2

### 229. internal.SendingCollector

- **Target:** `internal.SendingCollector`
- **Similarity:** 0.16
- **Dependents:** 0
- **Priority Score:** 208.4
- **Functions:** 1/1 matched (target 2)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_
- **Lint issues:** 1

### 230. internal.Synchronized.common

- **Target:** `internal.SynchronizedObject`
- **Similarity:** 0.24
- **Dependents:** 0
- **Priority Score:** 207.6
- **Functions:** 1/1 matched (target 6)
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 231. test.ConcurrentTestUtilities

- **Target:** `test.ConcurrentTestUtilities`
- **Similarity:** 0.50
- **Dependents:** 0
- **Priority Score:** 205.0
- **Functions:** 2/2 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 232. kotlin.SharedFlowBaseline

- **Target:** `benchmarks.SharedFlowBaseline`
- **Similarity:** 0.66
- **Dependents:** 0
- **Priority Score:** 203.4
- **Functions:** 1/1 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_
- **TODOs:** 12
- **Lint issues:** 1

### 233. common.Runnable.common

- **Target:** `coroutines.Runnable [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 110.0
- **Functions:** 0/0 matched (target 3)
- **Missing functions:** _none_
- **Types:** 1/1 matched (target 2)
- **Missing types:** _none_

### 234. selects.WhileSelect

- **Target:** `selects.WhileSelect`
- **Similarity:** 0.04
- **Dependents:** 0
- **Priority Score:** 109.6
- **Functions:** 1/1 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 235. kotlinx-coroutines-core.nativeDarwin.test.Launcher

- **Target:** `test.Launcher`
- **Similarity:** 0.07
- **Dependents:** 0
- **Priority Score:** 109.3
- **Functions:** 1/1 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 3

### 236. common.Yield

- **Target:** `coroutines.Yield`
- **Similarity:** 0.17
- **Dependents:** 0
- **Priority Score:** 108.3
- **Functions:** 1/1 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_
- **Lint issues:** 4

### 237. concurrent.MultithreadedDispatchers.common

- **Target:** `coroutines.MultithreadedDispatchers`
- **Similarity:** 0.83
- **Dependents:** 0
- **Priority Score:** 101.7
- **Functions:** 1/1 matched (target 8)
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 1)
- **Missing types:** _none_

### 238. common.CompletionHandler.common

- **Target:** `coroutines.CompletionHandler`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 239. flow.FlowCollector

- **Target:** `flow.FlowCollector`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 240. common.Deferred

- **Target:** `coroutines.Deferred`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 100.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 1/1 matched
- **Missing types:** _none_

### 241. internal.ProbesSupport.common

- **Target:** `internal.ProbesSupport.common [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 2)
- **Missing functions:** _none_
- **Types:** 0/0 matched (target 1)
- **Missing types:** _none_
- **Lint issues:** 1

### 242. internal.NullSurrogate

- **Target:** `internal.NullSurrogate [ZERO]`
- **Similarity:** 0.00
- **Dependents:** 0
- **Priority Score:** 10.0
- **Functions:** 0/0 matched (target 3)
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 243. concurrent.Dispatchers

- **Target:** `concurrent.Dispatchers`
- **Similarity:** 1.00
- **Dependents:** 0
- **Priority Score:** 0.0
- **Functions:** 0/0 matched
- **Missing functions:** _none_
- **Types:** 0/0 matched
- **Missing types:** _none_

### 244. concurrent.Builders.concurrent

- **Target:** `concurrent.Builders.concurrent`
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

