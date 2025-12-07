// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/AbstractCoroutine.kt
//
// TODO: @file:Suppress - no C++ equivalent
// TODO: Continuation, Job, CoroutineScope interfaces need to be defined
// TODO: suspend functions are not directly supported in C++, need custom implementation
// TODO: Generics with 'in' variance need C++ template equivalents

// TODO: package kotlinx.coroutines -> namespace
namespace kotlinx {
namespace coroutines {

// TODO: import statements removed, use fully qualified names or includes
// TODO: import kotlinx.coroutines.CoroutineStart.*
// TODO: import kotlinx.coroutines.intrinsics.*
// TODO: import kotlin.coroutines.*
// TODO: import kotlinx.coroutines.internal.ScopeCoroutine

/**
 * Abstract base class for implementation of coroutines in coroutine builders.
 *
 * This class implements completion [Continuation], [Job], and [CoroutineScope] interfaces.
 * It stores the result of continuation in the state of the job.
 * This coroutine waits for children coroutines to finish before completing and
 * fails through an intermediate _failing_ state.
 *
 * The following methods are available for override:
 *
 * - [onStart] is invoked when the coroutine was created in non-active state and is being [started][Job.start].
 * - [onCancelling] is invoked as soon as the coroutine starts being cancelled for any reason (or completes).
 * - [onCompleted] is invoked when the coroutine completes with a value.
 * - [onCancelled] in invoked when the coroutine completes with an exception (cancelled).
 *
 * @param parentContext the context of the parent coroutine.
 * @param initParentJob specifies whether the parent-child relationship should be instantiated directly
 *               in `AbstractCoroutine` constructor. If set to `false`, it's the responsibility of the child class
 *               to invoke [initParentJob] manually.
 * @param active when `true` (by default), the coroutine is created in the _active_ state, otherwise it is created in the _new_ state.
 *               See [Job] for details.
 *
 * @suppress **This an internal API and should not be used from general code.**
 */
// TODO: @OptIn(InternalForInheritanceCoroutinesApi::class) - no C++ equivalent
// TODO: @InternalCoroutinesApi - no C++ equivalent
// TODO: Template parameter 'in T' uses variance, map to template<typename T>
template<typename T>
class AbstractCoroutine : public JobSupport, public Job, public Continuation<T>, public CoroutineScope {
    // TODO: Multiple inheritance - ensure proper virtual inheritance if needed

private:
    CoroutineContext parent_context;
    bool init_parent_job_flag;
    bool active_flag;

public:
    // Constructor
    AbstractCoroutine(
        CoroutineContext parentContext,
        bool initParentJob,
        bool active
    ) : JobSupport(active),
        parent_context(parentContext),
        init_parent_job_flag(initParentJob),
        active_flag(active)
    {
        /*
         * Setup parent-child relationship between the parent in the context and the current coroutine.
         * It may cause this coroutine to become _cancelling_ if the parent is already cancelled.
         * It is dangerous to install parent-child relationship here if the coroutine class
         * operates its state from within onCancelled or onCancelling
         * (with exceptions for rx integrations that can't have any parent)
         */
        if (initParentJob) {
            // TODO: parentContext[Job] - need operator[] implementation for CoroutineContext
            initParentJob(parentContext[Job]);
        }
    }

    /**
     * The context of this coroutine that includes this coroutine as a [Job].
     */
    // TODO: @Suppress("LeakingThis") - no C++ equivalent
    // TODO: final override - use 'final' keyword in C++
    CoroutineContext context;
    // TODO: Initialize in constructor: context = parentContext + this;

    /**
     * The context of this scope which is the same as the [context] of this coroutine.
     */
    // TODO: override - use 'override' keyword
    CoroutineContext coroutineContext() const override {
        return context;
    }

    // TODO: override val isActive - property with override
    bool isActive() const override {
        return JobSupport::isActive();
    }

    /**
     * This function is invoked once when the job was completed normally with the specified [value],
     * right before all the waiters for the coroutine's completion are notified.
     */
    // TODO: protected open fun - virtual function
    virtual void onCompleted(T value) {}

    /**
     * This function is invoked once when the job was cancelled with the specified [cause],
     * right before all the waiters for coroutine's completion are notified.
     *
     * **Note:** the state of the coroutine might not be final yet in this function and should not be queried.
     * You can use [completionCause] and [completionCauseHandled] to recover parameters that we passed
     * to this `onCancelled` invocation only when [isCompleted] returns `true`.
     *
     * @param cause The cancellation (failure) cause
     * @param handled `true` if the exception was handled by parent (always `true` when it is a [CancellationException])
     */
    // TODO: protected open fun - virtual function
    virtual void onCancelled(Throwable* cause, bool handled) {}

    // TODO: override fun - override keyword
    std::string cancellationExceptionMessage() override {
        return classSimpleName + " was cancelled";
    }

    // TODO: @Suppress("UNCHECKED_CAST") - no C++ equivalent
    // TODO: protected final override - final override
protected:
    void onCompletionInternal(void* state) final override {
        // TODO: state is CompletedExceptionally - dynamic_cast or type checking
        if (auto* completed_exceptionally = dynamic_cast<CompletedExceptionally*>(state)) {
            onCancelled(completed_exceptionally->cause, completed_exceptionally->handled);
        } else {
            // TODO: state as T - cast
            onCompleted(static_cast<T>(state));
        }
    }

public:
    /**
     * Completes execution of this with coroutine with the specified result.
     */
    // TODO: final override - final override
    void resumeWith(Result<T> result) final override {
        // TODO: result.toState() - need toState method
        void* state = makeCompletingOnce(result.toState());
        if (state == COMPLETING_WAITING_CHILDREN) return;
        afterResume(state);
    }

    /**
     * Invoked when the corresponding `AbstractCoroutine` was **conceptually** resumed, but not mechanically.
     * Currently, this function only invokes `resume` on the underlying continuation for [ScopeCoroutine]
     * or does nothing otherwise.
     *
     * Examples of resumes:
     * - `afterCompletion` calls when the corresponding `Job` changed its state (i.e. got cancelled)
     * - [AbstractCoroutine.resumeWith] was invoked
     */
    // TODO: protected open fun - virtual function
    virtual void afterResume(void* state) {
        afterCompletion(state);
    }

    // TODO: internal final override - need friend or internal visibility mechanism
    void handleOnCompletionException(Throwable* exception) final override {
        handleCoroutineException(context, exception);
    }

    // TODO: internal override - internal visibility
    std::string nameString() override {
        // TODO: context.coroutineName - need extension property
        auto coroutine_name = context.coroutineName;
        if (!coroutine_name) {
            return JobSupport::nameString();
        }
        return "\"" + *coroutine_name + "\":" + JobSupport::nameString();
    }

    /**
     * Starts this coroutine with the given code [block] and [start] strategy.
     * This function shall be invoked at most once on this coroutine.
     *
     * - [DEFAULT] uses [startCoroutineCancellable].
     * - [ATOMIC] uses [startCoroutine].
     * - [UNDISPATCHED] uses [startCoroutineUndispatched].
     * - [LAZY] does nothing.
     */
    // TODO: Template with receiver R
    // TODO: suspend block - need coroutine/continuation representation
    template<typename R>
    void start(CoroutineStart start, R receiver, std::function<T(R)> block) {
        // TODO: start(block, receiver, this) - invoke operator on CoroutineStart
        start(block, receiver, this);
    }
};

} // namespace coroutines
} // namespace kotlinx
