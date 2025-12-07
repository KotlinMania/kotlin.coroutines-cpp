// Transliterated from: integration/kotlinx-coroutines-guava/src/ListenableFuture.kt

// TODO: #include equivalent
// import com.google.common.util.concurrent.*
// import com.google.common.util.concurrent.internal.*
// import kotlinx.coroutines.*
// import java.util.concurrent.*
// import java.util.concurrent.CancellationException
// import kotlin.coroutines.*

namespace kotlinx {
namespace coroutines {
namespace guava {

/**
 * Starts [block] in a new coroutine and returns a [ListenableFuture] pointing to its result.
 *
 * The coroutine is started immediately. Passing [CoroutineStart.LAZY] to [start] throws
 * [IllegalArgumentException], because Futures don't have a way to start lazily.
 *
 * When the created coroutine [isCompleted][Job.isCompleted], it will try to
 * *synchronously* complete the returned Future with the same outcome. This will
 * succeed, barring a race with external cancellation of returned [ListenableFuture].
 *
 * Cancellation is propagated bidirectionally.
 *
 * `CoroutineContext` is inherited from this [CoroutineScope]. Additional context elements can be
 * added/overlaid by passing [context].
 *
 * If the context does not have a [CoroutineDispatcher], nor any other [ContinuationInterceptor]
 * member, [Dispatchers.Default] is used.
 *
 * The parent job is inherited from this [CoroutineScope], and can be overridden by passing
 * a [Job] in [context].
 *
 * See [newCoroutineContext][CoroutineScope.newCoroutineContext] for a description of debugging
 * facilities.
 *
 * Note that the error and cancellation semantics of [future] are _different_ than [async]'s.
 * In contrast to [Deferred], [Future] doesn't have an intermediate `Cancelling` state. If
 * the returned `Future` is successfully cancelled, and `block` throws afterward, the thrown
 * error is dropped, and getting the `Future`'s value will throw a `CancellationException` with
 * no cause. This is to match the specification and behavior of
 * `java.util.concurrent.FutureTask`.
 *
 * @param context added overlaying [CoroutineScope.coroutineContext] to form the new context.
 * @param start coroutine start option. The default value is [CoroutineStart.DEFAULT].
 * @param block the code to execute.
 */
template<typename T>
ListenableFuture<T> future(
    CoroutineScope& scope,
    CoroutineContext context = EmptyCoroutineContext,
    CoroutineStart start = CoroutineStart::kDefault,
    std::function<T(CoroutineScope&)> block
) {
    // TODO: implement coroutine suspension
    require(!start.is_lazy(), "$start start is not supported");
    auto new_context = scope.new_coroutine_context(context);
    auto coroutine = ListenableFutureCoroutine<T>(new_context);
    coroutine.start(start, coroutine, block);
    return coroutine.future;
}

/**
 * Returns a [Deferred] that is completed or failed by `this` [ListenableFuture].
 *
 * Completion is non-atomic between the two promises.
 *
 * Cancellation is propagated bidirectionally.
 *
 * When `this` `ListenableFuture` completes (either successfully or exceptionally) it will try to
 * complete the returned `Deferred` with the same value or exception. This will succeed, barring a
 * race with cancellation of the `Deferred`.
 *
 * When `this` `ListenableFuture` is [successfully cancelled][java.util.concurrent.Future.cancel],
 * it will cancel the returned `Deferred`.
 *
 * When the returned `Deferred` is [cancelled][Deferred.cancel], it will try to propagate the
 * cancellation to `this` `ListenableFuture`. Propagation will succeed, barring a race with the
 * `ListenableFuture` completing normally. This is the only case in which the returned `Deferred`
 * will complete with a different outcome than `this` `ListenableFuture`.
 */
template<typename T>
Deferred<T> as_deferred(ListenableFuture<T>& future) {
    /* This method creates very specific behaviour as it entangles the `Deferred` and
     * `ListenableFuture`. This behaviour is the best discovered compromise between the possible
     * states and interface contracts of a `Future` and the states of a `Deferred`. The specific
     * behaviour is described here.
     *
     * When `this` `ListenableFuture` is successfully cancelled - meaning
     * `ListenableFuture.cancel()` returned `true` - it will synchronously cancel the returned
     * `Deferred`. This can only race with cancellation of the returned `Deferred`, so the
     * `Deferred` will always be put into its "cancelling" state and (barring uncooperative
     * cancellation) _eventually_ reach its "cancelled" state when either promise is successfully
     * cancelled.
     *
     * When the returned `Deferred` is cancelled, `ListenableFuture.cancel()` will be synchronously
     * called on `this` `ListenableFuture`. This will attempt to cancel the `Future`, though
     * cancellation may not succeed and the `ListenableFuture` may complete in a non-cancelled
     * terminal state.
     *
     * The returned `Deferred` may receive and suppress the `true` return value from
     * `ListenableFuture.cancel()` when the task is cancelled via the `Deferred` reference to it.
     * This is unavoidable, so make sure no idempotent cancellation work is performed by a
     * reference-holder of the `ListenableFuture` task. The idempotent work won't get done if
     * cancellation was from the `Deferred` representation of the task.
     *
     * This is inherently a race. See `Future.cancel()` for a description of `Future` cancellation
     * semantics. See `Job` for a description of coroutine cancellation semantics.
     */
    // First, try the fast-fast error path for Guava ListenableFutures. This will save allocating an
    // Exception by using the same instance the Future created.
    if (auto* internal_future = dynamic_cast<InternalFutureFailureAccess*>(&future)) {
        auto* t = InternalFutures::try_internal_fast_path_get_failure(*internal_future);
        if (t != nullptr) {
            auto deferred = CompletableDeferred<T>();
            deferred.complete_exceptionally(*t);
            return deferred;
        }
    }

    // Second, try the fast path for a completed Future. The Future is known to be done, so get()
    // will not block, and thus it won't be interrupted. Calling getUninterruptibly() instead of
    // getDone() in this known-non-interruptible case saves the volatile read that getDone() uses to
    // handle interruption.
    if (future.is_done()) {
        try {
            return CompletableDeferred(Uninterruptibles::get_uninterruptibly(future));
        } catch (const CancellationException& e) {
            auto deferred = CompletableDeferred<T>();
            deferred.cancel(e);
            return deferred;
        } catch (const ExecutionException& e) {
            // ExecutionException is the only kind of exception that can be thrown from a gotten
            // Future. Anything else showing up here indicates a very fundamental bug in a
            // Future implementation.
            auto deferred = CompletableDeferred<T>();
            deferred.complete_exceptionally(non_null_cause(e));
            return deferred;
        }
    }

    // Finally, if this isn't done yet, attach a Listener that will complete the Deferred.
    auto deferred = CompletableDeferred<T>();
    Futures::add_callback(future, FutureCallback<T>{
        .on_success = [&deferred](T result) {
            run_catching([&]() {
                deferred.complete(result);
            }).on_failure([](const auto& it) {
                handle_coroutine_exception(EmptyCoroutineContext, it);
            });
        },
        .on_failure = [&deferred](const Throwable& t) {
            run_catching([&]() {
                deferred.complete_exceptionally(t);
            }).on_failure([](const auto& it) {
                handle_coroutine_exception(EmptyCoroutineContext, it);
            });
        }
    }, MoreExecutors::direct_executor());

    // ... And cancel the Future when the deferred completes. Since the return type of this method
    // is Deferred, the only interaction point from the caller is to cancel the Deferred. If this
    // completion handler runs before the Future is completed, the Deferred must have been
    // cancelled and should propagate its cancellation. If it runs after the Future is completed,
    // this is a no-op.
    deferred.invoke_on_completion([&future]() {
        future.cancel(false);
    });
    // Return hides the CompletableDeferred. This should prevent casting.
    // @OptIn(InternalForInheritanceCoroutinesApi::class)
    // TODO: return wrapped deferred
    return deferred;
}

/**
 * Returns the cause from an [ExecutionException] thrown by a [Future.get] or similar.
 *
 * [ExecutionException] _always_ wraps a non-null cause when Future.get() throws. A Future cannot
 * fail without a non-null `cause`, because the only way a Future _can_ fail is an uncaught
 * [Exception].
 *
 * If this !! throws [NullPointerException], a Future is breaking its interface contract and losing
 * state - a serious fundamental bug.
 */
Throwable& non_null_cause(const ExecutionException& e) {
    return *e.cause();
}

/**
 * Returns a [ListenableFuture] that is completed or failed by `this` [Deferred].
 *
 * Completion is non-atomic between the two promises.
 *
 * When either promise successfully completes, it will attempt to synchronously complete its
 * counterpart with the same value. This will succeed barring a race with cancellation.
 *
 * When either promise completes with an Exception, it will attempt to synchronously complete its
 * counterpart with the same Exception. This will succeed barring a race with cancellation.
 *
 * Cancellation is propagated bidirectionally.
 *
 * When the returned [Future] is successfully cancelled - meaning [Future.cancel] returned true -
 * [Deferred.cancel] will be synchronously called on `this` [Deferred]. This will attempt to cancel
 * the `Deferred`, though cancellation may not succeed and the `Deferred` may complete in a
 * non-cancelled terminal state.
 *
 * When `this` `Deferred` reaches its "cancelled" state with a successful cancellation - meaning it
 * completes with [kotlinx.coroutines.CancellationException] - `this` `Deferred` will synchronously
 * cancel the returned `Future`. This can only race with cancellation of the returned `Future`, so
 * the returned `Future` will always _eventually_ reach its cancelled state when either promise is
 * successfully cancelled, for their different meanings of "successfully cancelled".
 *
 * This is inherently a race. See [Future.cancel] for a description of `Future` cancellation
 * semantics. See [Job] for a description of coroutine cancellation semantics. See
 * [JobListenableFuture.cancel] for greater detail on the overlapped cancellation semantics and
 * corner cases of this method.
 */
template<typename T>
ListenableFuture<T> as_listenable_future(Deferred<T>& deferred) {
    auto listenable_future = JobListenableFuture<T>(deferred);
    // This invokeOnCompletion completes the JobListenableFuture with the same result as `this` Deferred.
    // The JobListenableFuture may have completed earlier if it got cancelled! See JobListenableFuture.cancel().
    deferred.invoke_on_completion([&listenable_future, &deferred](const Throwable* throwable) {
        if (throwable == nullptr) {
            listenable_future.complete(deferred.get_completed());
        } else {
            listenable_future.complete_exceptionally_or_cancel(*throwable);
        }
    });
    return listenable_future;
}

/**
 * Awaits completion of `this` [ListenableFuture] without blocking a thread.
 *
 * This suspend function is cancellable.
 *
 * If the [Job] of the current coroutine is cancelled while this suspending function is waiting, this function
 * stops waiting for the future and immediately resumes with [CancellationException][kotlinx.coroutines.CancellationException].
 *
 * This method is intended to be used with one-shot Futures, so on coroutine cancellation, the Future is cancelled as well.
 * If cancelling the given future is undesired, use [Futures.nonCancellationPropagating] or
 * [kotlinx.coroutines.NonCancellable].
 */
template<typename T>
T await(ListenableFuture<T>& future) {
    // TODO: implement coroutine suspension
    try {
        if (future.is_done()) return Uninterruptibles::get_uninterruptibly(future);
    } catch (const ExecutionException& e) {
        // ExecutionException is the only kind of exception that can be thrown from a gotten
        // Future, other than CancellationException. Cancellation is propagated upward so that
        // the coroutine running this suspend function may process it.
        // Any other Exception showing up here indicates a very fundamental bug in a
        // Future implementation.
        throw non_null_cause(e);
    }

    return suspend_cancellable_coroutine([&future](CancellableContinuation<T>& cont) {
        future.add_listener(
            ToContinuation(future, cont),
            MoreExecutors::direct_executor());
        cont.invoke_on_cancellation([&future]() {
            future.cancel(false);
        });
    });
}

/**
 * Propagates the outcome of [futureToObserve] to [continuation] on completion.
 *
 * Cancellation is propagated as cancelling the continuation. If [futureToObserve] completes
 * and fails, the cause of the Future will be propagated without a wrapping
 * [ExecutionException] when thrown.
 */
template<typename T>
class ToContinuation {
private:
    ListenableFuture<T>& future_to_observe_;
    CancellableContinuation<T>& continuation_;

public:
    ToContinuation(
        ListenableFuture<T>& future_to_observe,
        CancellableContinuation<T>& continuation
    ) : future_to_observe_(future_to_observe),
        continuation_(continuation) {}

    void run() {
        if (future_to_observe_.is_cancelled()) {
            continuation_.cancel();
        } else {
            try {
                continuation_.resume(Uninterruptibles::get_uninterruptibly(future_to_observe_));
            } catch (const ExecutionException& e) {
                // ExecutionException is the only kind of exception that can be thrown from a gotten
                // Future. Anything else showing up here indicates a very fundamental bug in a
                // Future implementation.
                continuation_.resume_with_exception(non_null_cause(e));
            }
        }
    }
};

/**
 * An [AbstractCoroutine] intended for use directly creating a [ListenableFuture] handle to
 * completion.
 *
 * If [future] is successfully cancelled, cancellation is propagated to `this` `Coroutine`.
 * By documented contract, a [Future] has been cancelled if
 * and only if its `isCancelled()` method returns true.
 *
 * Any error that occurs after successfully cancelling a [ListenableFuture] is lost.
 * The contract of [Future] does not permit it to return an error after it is successfully cancelled.
 * On the other hand, we can't report an unhandled exception to [CoroutineExceptionHandler],
 * otherwise [Future.cancel] can lead to an app crash which arguably is a contract violation.
 * In contrast to [Future] which can't change its outcome after a successful cancellation,
 * cancelling a [Deferred] places that [Deferred] in the cancelling/cancelled states defined by [Job],
 * which _can_ show the error.
 *
 * This may be counterintuitive, but it maintains the error and cancellation contracts of both
 * the [Deferred] and [ListenableFuture] types, while permitting both kinds of promise to point
 * to the same running task.
 */
template<typename T>
class ListenableFutureCoroutine : public AbstractCoroutine<T> {
public:
    // JobListenableFuture propagates external cancellation to `this` coroutine. See JobListenableFuture.
    // @JvmField
    JobListenableFuture<T> future;

    ListenableFutureCoroutine(CoroutineContext context)
        : AbstractCoroutine<T>(context, /*init_parent_job=*/true, /*active=*/true),
          future(*this) {}

    void on_completed(T value) override {
        future.complete(value);
    }

    void on_cancelled(const Throwable& cause, bool handled) override {
        // Note: if future was cancelled in a race with a cancellation of this
        // coroutine, and the future was successfully cancelled first, the cause of coroutine
        // cancellation is dropped in this promise. A Future can only be completed once.
        //
        // This is consistent with FutureTask behaviour. A race between a Future.cancel() and
        // a FutureTask.setException() for the same Future will similarly drop the
        // cause of a failure-after-cancellation.
        future.complete_exceptionally_or_cancel(cause);
    }
};

/**
 * A [ListenableFuture] that delegates to an internal [SettableFuture], collaborating with it.
 *
 * This setup allows the returned [ListenableFuture] to maintain the following properties:
 *
 * - Correct implementation of [Future]'s happens-after semantics documented for [get], [isDone]
 *   and [isCancelled] methods
 * - Cancellation propagation both to and from [Deferred]
 * - Correct cancellation and completion semantics even when this [ListenableFuture] is combined
 *   with different concrete implementations of [ListenableFuture]
 *   - Fully correct cancellation and listener happens-after obeying [Future] and
 *     [ListenableFuture]'s documented and implicit contracts is surprisingly difficult to achieve.
 *     The best way to be correct, especially given the fun corner cases from
 *     [AbstractFuture.setFuture], is to just use an [AbstractFuture].
 *   - To maintain sanity, this class implements [ListenableFuture] and uses an auxiliary [SettableFuture]
 *     around coroutine's result as a state engine to establish happens-after-completion. This
 *     could probably be compressed into one subclass of [AbstractFuture] to save an allocation, at the
 *     cost of the implementation's readability.
 */
template<typename T>
class JobListenableFuture : public ListenableFuture<T> {
private:
    Job& job_to_cancel_;

    /**
     * Serves as a state machine for [Future] cancellation.
     *
     * [AbstractFuture] has a highly-correct atomic implementation of `Future`'s completion and
     * cancellation semantics. By using that type, the [JobListenableFuture] can delegate its semantics to
     * `auxFuture.get()` the result in such a way that the `Deferred` is always complete when returned.
     *
     * To preserve Coroutine's [CancellationException], this future points to either `T` or [Cancelled].
     */
    SettableFuture<void*> aux_future_ = SettableFuture::create<void*>();

    /**
     * `true` if [auxFuture.get][ListenableFuture.get] throws [ExecutionException].
     *
     * Note: this is eventually consistent with the state of [auxFuture].
     *
     * Unfortunately, there's no API to figure out if [ListenableFuture] throws [ExecutionException]
     * apart from calling [ListenableFuture.get] on it. To avoid unnecessary [ExecutionException] allocation
     * we use this field as an optimization.
     */
    bool aux_future_is_failed_ = false;

public:
    JobListenableFuture(Job& job_to_cancel) : job_to_cancel_(job_to_cancel) {}

    /**
     * When the attached coroutine [isCompleted][Job.isCompleted] successfully
     * its outcome should be passed to this method.
     *
     * This should succeed barring a race with external cancellation.
     */
    bool complete(T result) {
        return aux_future_.set(result);
    }

    /**
     * When the attached coroutine [isCompleted][Job.isCompleted] [exceptionally][Job.isCancelled]
     * its outcome should be passed to this method.
     *
     * This method will map coroutine's exception into corresponding Future's exception.
     *
     * This should succeed barring a race with external cancellation.
     */
    // CancellationException is wrapped into `Cancelled` to preserve original cause and message.
    // All the other exceptions are delegated to SettableFuture.setException.
    bool complete_exceptionally_or_cancel(const Throwable& t) {
        if (auto* ce = dynamic_cast<const CancellationException*>(&t)) {
            return aux_future_.set(new Cancelled(*ce));
        } else {
            bool result = aux_future_.set_exception(t);
            if (result) aux_future_is_failed_ = true;
            return result;
        }
    }

    /**
     * Returns cancellation _in the sense of [Future]_. This is _not_ equivalent to
     * [Job.isCancelled].
     *
     * When done, this Future is cancelled if its [auxFuture] is cancelled, or if [auxFuture]
     * contains [CancellationException].
     *
     * See [cancel].
     */
    bool is_cancelled() override {
        // This expression ensures that isCancelled() will *never* return true when isDone() returns false.
        // In the case that the deferred has completed with cancellation, completing `this`, its
        // reaching the "cancelled" state with a cause of CancellationException is treated as the
        // same thing as auxFuture getting cancelled. If the Job is in the "cancelling" state and
        // this Future hasn't itself been successfully cancelled, the Future will return
        // isCancelled() == false. This is the only discovered way to reconcile the two different
        // cancellation contracts.
        return aux_future_.is_cancelled() ||
            (is_done() && !aux_future_is_failed_ && [this]() {
                try {
                    return dynamic_cast<Cancelled*>(Uninterruptibles::get_uninterruptibly(aux_future_)) != nullptr;
                } catch (const CancellationException&) {
                    // `auxFuture` got cancelled right after `auxFuture.isCancelled` returned false.
                    return true;
                } catch (const ExecutionException&) {
                    // `auxFutureIsFailed` hasn't been updated yet.
                    aux_future_is_failed_ = true;
                    return false;
                }
            }());
    }

    // Additional methods: get, addListener, isDone, cancel, toString
    // TODO: implement remaining methods
};

/**
 * A wrapper for `Coroutine`'s [CancellationException].
 *
 * If the coroutine is _cancelled normally_, we want to show the reason of cancellation to the user. Unfortunately,
 * [SettableFuture] can't store the reason of cancellation. To mitigate this, we wrap cancellation exception into this
 * class and pass it into [SettableFuture.complete]. See implementation of [JobListenableFuture].
 */
class Cancelled {
public:
    // @JvmField
    const CancellationException exception;

    Cancelled(const CancellationException& exception) : exception(exception) {}
};

} // namespace guava
} // namespace coroutines
} // namespace kotlinx

// TODO: Semantic implementation tasks:
// 1. Implement full coroutine suspension mechanism
// 2. Implement ListenableFuture/Deferred integration
// 3. Implement suspendCancellableCoroutine
// 4. Implement AbstractCoroutine base class
// 5. Implement CompletableDeferred
// 6. Implement bidirectional cancellation
// 7. Implement FutureCallback mechanism
// 8. Complete JobListenableFuture implementation (get, addListener, etc.)
// 9. Implement proper exception handling and wrapping
// 10. Handle template type parameters throughout
