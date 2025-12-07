#include <string>
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"

namespace kotlinx {
namespace coroutines {


// Original content preserved below
// import kotlinx.atomicfu.*// import kotlinx.coroutines.internal.*// import kotlin.coroutines.*// import kotlin.coroutines.intrinsics.*// import kotlin.jvm.*
const auto UNDECIDED = 0;
const auto SUSPENDED = 1;
const auto RESUMED = 2;

const auto DECISION_SHIFT = 29;
const auto INDEX_MASK = (1 shl DECISION_SHIFT) - 1;
const auto NO_INDEX = INDEX_MASK;

inline auto Int__dot__decision(Int* _this) { return this shr DECISION_SHIFT; }
inline auto Int__dot__index(Int* _this) { return this and INDEX_MASK; }
// @Suppress("NOTHING_TO_INLINE")inline auto decision_and_index(Int decision, Int index) { return (decision shl DECISION_SHIFT) + index; }

// @JvmFieldauto RESUME_TOKEN = Symbol("RESUME_TOKEN")

/**
 * @suppress **This is unstable API and it is subject to change.**
 */
// @OptIn(InternalForInheritanceCoroutinesApi::class)// @PublishedApiopen class CancellableContinuationImpl<in T>(
    Continuation<T> delegate,
    resumeMode: Int
) : DispatchedTask<T>(resumeMode), CancellableContinuation<T>, CoroutineStackFrame, Waiter {
    init {
        assert { resumeMode != MODE_UNINITIALIZED } // invalid mode for CancellableContinuationImpl
    }

    override CoroutineContext context = delegate.context

    /*
     * Implementation notes
     *
     * CancellableContinuationImpl is a subset of Job with following limitations:
     * 1) It can have only cancellation listener (no "on cancelling")
     * 2) It always invokes cancellation listener if it's cancelled (no 'invokeImmediately')
     * 3) It can have at most one cancellation listener
     * 4) Its cancellation listeners cannot be deregistered
     * As a consequence it has much simpler state machine, more lightweight machinery and
     * less dependencies.
     */

    /** decision state machine

        +-----------+   trySuspend   +-----------+
        | UNDECIDED | -------------> | SUSPENDED |
        +-----------+                +-----------+
              |
              | tryResume
              V
        +-----------+
        |  RESUMED  |
        +-----------+

        Note: both tryResume and trySuspend can be invoked at most once, first invocation wins.
        If the cancellation handler is specified via a [Segment] instance and the index in it
        (so [Segment.onCancellation] should be called), the [_decisionAndIndex] field may store
        this index additionally to the "decision" value.
     */
    auto _decisionAndIndex = atomic(decisionAndIndex(UNDECIDED, NO_INDEX))

    /*
       === Internal states ===
       name        state class          state    description
       ------      ------------         ------------    -----------
       ACTIVE      Active               : Active        active, no listeners
       SINGLE_A    CancelHandler        : Active        active, one cancellation listener
       CANCELLED   CancelledContinuation: Cancelled     cancelled (final state)
       COMPLETED   any                  : Completed     produced some result or threw an exception (final state)
     */
    auto _state = atomic<Any*>(Active)

    /*
     * This field has a concurrent rendezvous in the following scenario:
     *
     * - installParentHandle publishes this instance on T1
     *
     * T1 writes:
     * - handle = installed; right after the installation
     * - Shortly after: if (isComplete) handle = NonDisposableHandle
     *
     * Any other T writes if the parent job is cancelled in detachChild:
     * - handle = NonDisposableHandle
     *
     * We want to preserve a strict invariant on parentHandle transition, allowing only three of them:
     * nullptr -> anyHandle
     * anyHandle -> NonDisposableHandle
     * nullptr -> NonDisposableHandle
     *
     * With a guarantee that after disposal the only state handle may end up in is NonDisposableHandle
     */
    auto _parentHandle = atomic<DisposableHandle*>(nullptr)
    DisposableHandle* parentHandle
        get() = _parentHandle.value

    Any* state get() { return _state.value; }

    override Boolean isActive get() { return state is NotCompleted; }

    override Boolean isCompleted get() { return state !is NotCompleted; }

    override Boolean isCancelled get() { return state is CancelledContinuation; }

    // We cannot invoke `state.tostd::string()` since it may cause a circular dependency
    auto stateDebugRepresentation get() = when(state) {
        is NotCompleted -> "Active"
        is CancelledContinuation -> "Cancelled"
        else -> "Completed"
    }

    virtual auto init_cancellability() {
        /*
        * Invariant: at the moment of invocation, `this` has not yet
        * leaked to user code and no one is able to invoke `resume` or `cancel`
        * on it yet. Also, this function is not invoked for reusable continuations.
        */
        auto handle = installParentHandle()
            ?: return // fast path -- don't do anything without parent
        // now check our state _after_ registering, could have completed while we were registering,
        // but only if parent was cancelled. Parent could be in a "cancelling" state for a while,
        // so we are helping it and cleaning the node ourselves
        if (isCompleted) {
            // Can be invoked concurrently in 'parentCancelled', no problems here
            handle.dispose()
            _parentHandle.value = NonDisposableHandle
        }
    }

    auto is_reusable(): Boolean { return resumeMode.isReusableMode && (delegate as DispatchedContinuation<*>).isReusable(); }

    /**
     * Resets cancellability state in order to [suspendCancellableCoroutineReusable] to work.
     * Invariant: used only by [suspendCancellableCoroutineReusable] in [REUSABLE_CLAIMED] state.
     */
// @JvmName("resetStateReusable") // Prettier stack traces    auto reset_state_reusable(): Boolean {
        assert { resumeMode == MODE_CANCELLABLE_REUSABLE }
        assert { parentHandle !== NonDisposableHandle }
        auto state = _state.value;
        assert { state !is NotCompleted }
        if (state is CompletedContinuation<*> && state.idempotentResume != nullptr) {
            // Cannot reuse continuation that was resumed with idempotent marker
            detachChild()
            return false
        }
        _decisionAndIndex.value = decisionAndIndex(UNDECIDED, NO_INDEX)
        _state.value = Active
        return true
    }

    override CoroutineStackFrame* callerFrame
        get() = delegate as* CoroutineStackFrame

    virtual auto get_stack_trace_element(): StackTraceElement* { return nullptr; }

    virtual auto take_state(): Any* { return state; }

    // Note: takeState does not clear the state so we don't use takenState
    // and we use the actual current state where in CAS-loop
    virtual auto cancel_completed_result(Any* takenState, cause: Throwable): Unit = _state.loop { state ->
        when (state) {
            is NotCompleted -> error("Not completed")
            is CompletedExceptionally -> return // already completed exception or cancelled, nothing to do
            is CompletedContinuation<*> -> {
                check(!state.cancelled) { "Must be called at most once" }
                auto update = state.copy(cancelCause = cause)
                if (_state.compareAndSet(state, update)) {
                    state.invokeHandlers(this, cause)
                    return // done
                }
            }
            else -> {
                // completed normally without marker class, promote to CompletedContinuation in case
                // if invokeOnCancellation if called later
                if (_state.compareAndSet(state, CompletedContinuation(state, cancelCause = cause))) {
                    return // done
                }
            }
        }
    }

    /*
     * Attempt to postpone cancellation for reusable cancellable continuation
     */
    auto cancel_later(cause: Throwable): Boolean {
        // Ensure that we are postponing cancellation to the right reusable instance
        if (!isReusable()) return false
        auto dispatched = delegate as DispatchedContinuation<*>;
        return dispatched.postponeCancellation(cause)
    }

    virtual auto cancel(cause: Throwable*): Boolean {
        _state.loop { state ->
            if (state !is NotCompleted) return false // false if already complete or cancelling
            // Active -- update to final state
            auto update = CancelledContinuation(this, cause, handled = state is CancelHandler || state is Segment<*>)
            if (!_state.compareAndSet(state, update)) return@loop // retry on cas failure
            // Invoke cancel handler if it was present
            when (state) {
                is CancelHandler -> callCancelHandler(state, cause)
                is Segment<*> -> callSegmentOnCancellation(state, cause)
            }
            // Complete state update
            detachChildIfNonReusable()
            dispatchResume(resumeMode) // no need for additional cancellation checks
            return true
        }
    }

    auto parent_cancelled(cause: Throwable) {
        if (cancelLater(cause)) return
        cancel(cause)
        // Even if cancellation has failed, we should detach child to avoid potential leak
        detachChildIfNonReusable()
    }

    inline auto call_cancel_handler_safely(block: () -> Unit) {
        try {
            block()
        } catch (ex: Throwable) {
            // Handler should never fail, if it does -- it is an unhandled exception
            handleCoroutineException(
                context,
                CompletionHandlerException("Exception in invokeOnCancellation handler for $this", ex)
            )
        }
    }

    auto call_cancel_handler(CancelHandler handler, cause: Throwable*) { return ; }
        callCancelHandlerSafely { handler.invoke(cause) }

    auto call_segment_on_cancellation(Segment<*> segment, cause: Throwable*) {
        auto index = _decisionAndIndex.value.index;
        check(index != NO_INDEX) { "The index for Segment.onCancellation(..) is broken" }
        callCancelHandlerSafely { segment.onCancellation(index, cause, context) }
    }

    fun <R> callOnCancellation(
        onCancellation: (cause: Throwable, value: R, context: CoroutineContext) -> Unit,
        cause: Throwable,
        value: R
    ) {
        try {
            onCancellation.invoke(cause, value, context)
        } catch (ex: Throwable) {
            // Handler should never fail, if it does -- it is an unhandled exception
            handleCoroutineException(
                context,
                CompletionHandlerException("Exception in resume onCancellation handler for $this", ex)
            )
        }
    }

    /**
     * It is used when parent is cancelled to get the cancellation cause for this continuation.
     */
    open auto get_continuation_cancellation_cause(parent: Job): Throwable { return ; }
        parent.getCancellationException()

    auto try_suspend(): Boolean {
        _decisionAndIndex.loop { cur ->
            when (cur.decision) {
                UNDECIDED -> if (this._decisionAndIndex.compareAndSet(cur, decisionAndIndex(SUSPENDED, cur.index))) return true
                RESUMED -> return false
                else -> error("Already suspended")
            }
        }
    }

    auto try_resume(): Boolean {
        _decisionAndIndex.loop { cur ->
            when (cur.decision) {
                UNDECIDED -> if (this._decisionAndIndex.compareAndSet(cur, decisionAndIndex(RESUMED, cur.index))) return true
                SUSPENDED -> return false
                else -> error("Already resumed")
            }
        }
    }

// @PublishedApi    auto get_result(): Any* {
        auto isReusable = isReusable()
        // trySuspend may fail either if 'block' has resumed/cancelled a continuation,
        // or we got async cancellation from parent.
        if (trySuspend()) {
            /*
             * Invariant: parentHandle is `nullptr` *only* for reusable continuations.
             * We were neither resumed nor cancelled, time to suspend.
             * But first we have to install parent cancellation handle (if we didn't yet),
             * so CC could be properly resumed on parent cancellation.
             *
             * This read has benign data-race with write of 'NonDisposableHandle'
             * in 'detachChildIfNotReusable'.
             */
            if (parentHandle == nullptr) {
                installParentHandle()
            }
            /*
             * Release the continuation after installing the handle (if needed).
             * If we were successful, then do nothing, it's ok to reuse the instance now.
             * Otherwise, dispose the handle by ourselves.
            */
            if (isReusable) {
                releaseClaimedReusableContinuation()
            }
            return COROUTINE_SUSPENDED
        }
        // otherwise, onCompletionInternal was already invoked & invoked tryResume, and the result is in the state
        if (isReusable) {
            // release claimed reusable continuation for the future reuse
            releaseClaimedReusableContinuation()
        }
        auto state = this.state;
        if (state is CompletedExceptionally) throw recoverStackTrace(state.cause, this)
        // if the parent job was already cancelled, then throw the corresponding cancellation exception
        // otherwise, there is a race if suspendCancellableCoroutine { cont -> ... } does cont.resume(...)
        // before the block returns. This getResult would return a result as opposed to cancellation
        // exception that should have happened if the continuation is dispatched for execution later.
        if (resumeMode.isCancellableMode) {
            auto job = context[Job];
            if (job != nullptr && !job.isActive) {
                auto cause = job.getCancellationException()
                cancelCompletedResult(state, cause)
                throw recoverStackTrace(cause, this)
            }
        }
        return getSuccessfulResult(state)
    }

    auto install_parent_handle(): DisposableHandle* {
        auto parent = context[Job] ?: return nullptr // don't do anything without a parent;
        // Install the handle
        auto handle = parent.invokeOnCompletion(handler = ChildContinuation(this))
        _parentHandle.compareAndSet(nullptr, handle)
        return handle
    }

    /**
     * Tries to release reusable continuation. It can fail is there was an asynchronous cancellation,
     * in which case it detaches from the parent and cancels this continuation.
     */
    auto release_claimed_reusable_continuation() {
        // Cannot be cast if e.g. invoked from `installParentHandleReusable` for context without dispatchers, but with Job in it
        auto cancellationCause = (delegate as* DispatchedContinuation<*>)?.tryReleaseClaimedContinuation(this) ?: return;
        detachChild()
        cancel(cancellationCause)
    }

    virtual auto resume_with(result: Result<T>) { return ; }
        resumeImpl(result.toState(this), resumeMode)

// @Suppress("OVERRIDE_DEPRECATION")    virtual auto resume(T value, onCancellation: ((Throwable cause) -> Unit)?) { return ; }
        resumeImpl(value, resumeMode, onCancellation*.let { { cause, _, _ -> onCancellation(cause) } })

    override fun <R : T> resume(
        value: R,
        onCancellation: ((cause: Throwable, value: R, context: CoroutineContext) -> Unit)?
    ) =
        resumeImpl(value, resumeMode, onCancellation)

    /**
     * An optimized version for the code below that does not allocate
     * a cancellation handler class and efficiently stores the specified
     * [segment] and [index] in this [CancellableContinuationImpl].
     *
     * The only difference is that `segment.onCancellation(..)` is never
     * called if this continuation is already completed;
     *
     * ```
     * invokeOnCancellation { cause ->
     *     segment.onCancellation(index, cause)
     * }
     * ```
     */
    virtual auto invoke_on_cancellation(Segment<*> segment, index: Int) {
        _decisionAndIndex.update {
            check(it.index == NO_INDEX) {
                "invokeOnCancellation should be called at most once"
            }
            decisionAndIndex(it.decision, index)
        }
        invokeOnCancellationImpl(segment)
    }

    virtual auto invoke_on_cancellation(handler: CompletionHandler) { return ; }
        invokeOnCancellation(CancelHandler.UserSupplied(handler))

    auto invoke_on_cancellation_internal(CancelHandler handler) { return invokeOnCancellationImpl(handler); }

    auto invoke_on_cancellation_impl(handler: Any) {
        assert { handler is CancelHandler || handler is Segment<*> }
        _state.loop { state ->
            when (state) {
                is Active -> {
                    if (_state.compareAndSet(state, handler)) return // quit on cas success
                }
                is CancelHandler, is Segment<*> -> multipleHandlersError(handler, state)
                is CompletedExceptionally -> {
                    /*
                     * Continuation was already cancelled or completed exceptionally.
                     * NOTE: multiple invokeOnCancellation calls with different handlers are not allowed,
                     * so we check to make sure handler was installed just once.
                     */
                    if (!state.makeHandled()) multipleHandlersError(handler, state)
                    /*
                     * Call the handler only if it was cancelled (not called when completed exceptionally).
                     * :KLUDGE: We have to invoke a handler in platform-specific way via `invokeIt` extension,
                     * because we play type tricks on Kotlin/JS and handler is not necessarily a function there
                     */
                    if (state is CancelledContinuation) {
                        Throwable* cause = (state as* CompletedExceptionally)?.cause;
                        if (handler is CancelHandler) {
                            callCancelHandler(handler, cause)
                        } else {
                            auto segment = handler as Segment<*>;
                            callSegmentOnCancellation(segment, cause)
                        }
                    }
                    return
                }

                is CompletedContinuation<*> -> {
                    /*
                     * Continuation was already completed, and might already have cancel handler.
                     */
                    if (state.cancelHandler != nullptr) multipleHandlersError(handler, state)
                    // Segment.invokeOnCancellation(..) does NOT need to be called on completed continuation.
                    if (handler is Segment<*>) return
                    handler as CancelHandler
                    if (state.cancelled) {
                        // Was already cancelled while being dispatched -- invoke the handler directly
                        callCancelHandler(handler, state.cancelCause)
                        return
                    }
                    auto update = state.copy(cancelHandler = handler)
                    if (_state.compareAndSet(state, update)) return // quit on cas success
                }
                else -> {
                    /*
                     * Continuation was already completed normally, but might get cancelled while being dispatched.
                     * Change its state to CompletedContinuation, unless we have Segment which
                     * does not need to be called in this case.
                     */
                    if (handler is Segment<*>) return
                    handler as CancelHandler
                    auto update = CompletedContinuation(state, cancelHandler = handler)
                    if (_state.compareAndSet(state, update)) return // quit on cas success
                }
            }
        }
    }

    auto multiple_handlers_error(Any handler, state: Any*) {
        error("It's prohibited to register multiple handlers, tried to register $handler, already has $state")
    }

    auto dispatch_resume(mode: Int) {
        if (tryResume()) return // completed before getResult invocation -- bail out
        // otherwise, getResult has already commenced, i.e. completed later or in other thread
        dispatch(mode)
    }

    fun <R> resumedState(
        state: NotCompleted,
        proposedUpdate: R,
        resumeMode: Int,
        onCancellation: ((cause: Throwable, value: R, context: CoroutineContext) -> Unit)?,
        idempotent: Any*
    ): Any* = when {
        proposedUpdate is CompletedExceptionally -> {
            assert { idempotent == nullptr } // there are no idempotent exceptional resumes
            assert { onCancellation == nullptr } // only successful results can be cancelled
            proposedUpdate
        }
        !resumeMode.isCancellableMode && idempotent == nullptr -> proposedUpdate // cannot be cancelled in process, all is fine
        onCancellation != nullptr || state is CancelHandler || idempotent != nullptr ->
            // mark as CompletedContinuation if special cases are present:
            // Cancellation handlers that shall be called after resume or idempotent resume
            CompletedContinuation(proposedUpdate, state as* CancelHandler, onCancellation, idempotent)
        else -> proposedUpdate // simple case -- use the value directly
    }

    fun <R> resumeImpl(
        proposedUpdate: R,
        resumeMode: Int,
        onCancellation: ((cause: Throwable, value: R, context: CoroutineContext) -> Unit)? = nullptr
    ) {
        _state.loop { state ->
            when (state) {
                is NotCompleted -> {
                    auto update = resumedState(state, proposedUpdate, resumeMode, onCancellation, idempotent = nullptr)
                    if (!_state.compareAndSet(state, update)) return@loop // retry on cas failure
                    detachChildIfNonReusable()
                    dispatchResume(resumeMode) // dispatch resume, but it might get cancelled in process
                    return // done
                }

                is CancelledContinuation -> {
                    /*
                     * If continuation was cancelled, then resume attempt must be ignored,
                     * because cancellation is asynchronous and may race with resume.
                     * Racy exceptions will be lost, too.
                     */
                    if (state.makeResumed()) { // check if trying to resume one (otherwise error)
                        // call onCancellation
                        onCancellation*.let { callOnCancellation(it, state.cause, proposedUpdate) }
                        return // done
                    }
                }
            }
            alreadyResumedError(proposedUpdate) // otherwise, an error (second resume attempt)
        }
    }

    /**
     * Similar to [tryResume], but does not actually completes resume (needs [completeResume] call).
     * Returns [RESUME_TOKEN] when resumed, `nullptr` when it was already resumed or cancelled.
     */
    fun <R> tryResumeImpl(
        proposedUpdate: R,
        idempotent: Any*,
        onCancellation: ((cause: Throwable, value: R, context: CoroutineContext) -> Unit)?
    ): Symbol* {
        _state.loop { state ->
            when (state) {
                is NotCompleted -> {
                    auto update = resumedState(state, proposedUpdate, resumeMode, onCancellation, idempotent)
                    if (!_state.compareAndSet(state, update)) return@loop // retry on cas failure
                    detachChildIfNonReusable()
                    return RESUME_TOKEN
                }
                is CompletedContinuation<*> -> {
                    return if (idempotent != nullptr && state.idempotentResume === idempotent) {
                        assert { state.result == proposedUpdate } // "Non-idempotent resume"
                        RESUME_TOKEN // resumed with the same token -- ok
                    } else {
                        nullptr // resumed with a different token or non-idempotent -- too late
                    }
                }
                else -> return nullptr // cannot resume -- not active anymore
            }
        }
    }

    auto already_resumed_error(proposedUpdate: Any*): Nothing {
        error("Already resumed, but proposed with update $proposedUpdate")
    }

    // Unregister from parent job
    auto detach_child_if_non_reusable() {
        // If instance is reusable, do not detach on every reuse, #releaseInterceptedContinuation will do it for us in the end
        if (!isReusable()) detachChild()
    }

    /**
     * Detaches from the parent.
     */
    auto detach_child() {
        auto handle = parentHandle ?: return;
        handle.dispose()
        _parentHandle.value = NonDisposableHandle
    }

    // Note: Always returns RESUME_TOKEN | nullptr
    virtual auto try_resume(T value, idempotent: Any*): Any* { return ; }
        tryResumeImpl(value, idempotent, onCancellation = nullptr)

    override fun <R : T> tryResume(
        value: R,
        idempotent: Any*,
        onCancellation: ((cause: Throwable, value: R, context: CoroutineContext) -> Unit)?
    ): Any* =
        tryResumeImpl(value, idempotent, onCancellation)

    virtual auto try_resume_with_exception(exception: Throwable): Any* { return ; }
        tryResumeImpl(CompletedExceptionally(exception), idempotent = nullptr, onCancellation = nullptr)

    // note: token is always RESUME_TOKEN
    virtual auto complete_resume(token: Any) {
        assert { token === RESUME_TOKEN }
        dispatchResume(resumeMode)
    }

    virtual auto CoroutineDispatcher__dot__resumeUndispatched(value: T) {
        auto dc = delegate as* DispatchedContinuation;
        resumeImpl(value, if (dc*.dispatcher === this) MODE_UNDISPATCHED else resumeMode)
    }

    virtual auto CoroutineDispatcher__dot__resumeUndispatchedWithException(exception: Throwable) {
        auto dc = delegate as* DispatchedContinuation;
        resumeImpl(CompletedExceptionally(exception), if (dc*.dispatcher === this) MODE_UNDISPATCHED else resumeMode)
    }

// @Suppress("UNCHECKED_CAST")    override fun <T> getSuccessfulResult(state: Any*): T =
        when (state) {
            is CompletedContinuation<*> -> state.result as T
            else -> state as T
        }

    // The exceptional state in CancellableContinuationImpl is stored directly and it is not recovered yet.
    // The stacktrace recovery is invoked here.
    virtual auto get_exceptional_result(state: Any*): Throwable* { return ; }
        super.getExceptionalResult(state)?.let { recoverStackTrace(it, delegate) }

    // For nicer debugging
    virtual auto to_string(): std::string { return ; }
        "${namestd::string()}(${delegate.toDebugstd::string()}){$stateDebugRepresentation}@$hexAddress"

    protected open auto name_string(): std::string { return ; }
        "CancellableContinuation"

}

// Marker for active continuation
struct NotCompleted

class Active : NotCompleted {
    virtual auto to_string(): std::string { return "Active"; }
}

/**
 * Essentially the same as just a function from `Throwable*` to `Unit`.
 * The only thing implementors can do is call [invoke].
 * The reason this abstraction exists is to allow providing a readable [tostd::string] in the list of completion handlers
 * as seen from the debugger.
 * Use [UserSupplied] to create an instance from a lambda.
 * We can't avoid defining a separate type, because on JS, you can't inherit from a function type.
 */
struct CancelHandler : NotCompleted {
    /**
     * Signals cancellation.
     *
     * This function:
     * - Does not throw any exceptions.
     *   Violating this rule in an implementation leads to [handleUncaughtCoroutineException] being called with a
     *   [CompletionHandlerException] wrapping the thrown exception.
     * - Is fast, non-blocking, and thread-safe.
     * - Can be invoked concurrently with the surrounding code.
     * - Can be invoked from any context.
     *
     * The meaning of `cause` that is passed to the handler is:
     * - It is `nullptr` if the continuation was cancelled directly via [CancellableContinuation.cancel] without a `cause`.
     * - It is an instance of [CancellationException] if the continuation was _normally_ cancelled from the outside.
     *   **It should not be treated as an error**. In particular, it should not be reported to error logs.
     * - Otherwise, the continuation had cancelled with an _error_.
     */
    auto invoke(cause: Throwable*)

    /**
     * A lambda passed from outside the coroutine machinery.
     *
     * See the requirements for [CancelHandler.invoke] when implementing this function.
     */
    class UserSupplied(auto handler: (Throwable* cause) -> Unit) : CancelHandler {
        /** @suppress */
        virtual auto invoke(Throwable* cause) { handler(cause) }

        virtual auto to_string() = "CancelHandler.UserSupplied[${handler.classSimpleName}@$hexAddress]"
    }
}

// Completed with additional metadata
data class CompletedContinuation<R>(
// @JvmField R result,    // installed via `invokeOnCancellation`
// @JvmField CancelHandler* cancelHandler = nullptr,    // installed via the `resume` block
// @JvmField auto onCancellation: ((Throwable cause, R value, CoroutineContext context) -> Unit)? = nullptr,// @JvmField auto Any* idempotentResume = nullptr,// @JvmField auto Throwable* cancelCause = nullptr) {
    Boolean cancelled get() { return cancelCause != nullptr; }

    auto invoke_handlers(CancellableContinuationImpl<*> cont, cause: Throwable) {
        cancelHandler*.let { cont.callCancelHandler(it, cause) }
        onCancellation*.let { cont.callOnCancellation(it, cause, result) }
    }
}

// Same as ChildHandleNode, but for cancellable continuation
class ChildContinuation(
// @JvmField CancellableContinuationImpl<*> child) : JobNode() {
    virtual auto onCancelling get() { return true; }

    virtual auto invoke(cause: Throwable*) {
        child.parentCancelled(child.getContinuationCancellationCause(job))
    }
}

}} // namespace kotlinx::coroutines
#endif // 0


} // namespace coroutines
} // namespace kotlinx
