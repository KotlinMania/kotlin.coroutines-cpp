// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/Builders.common.kt
//
// TODO: @file:JvmMultifileClass, @file:JvmName - JVM-specific, no C++ equivalent
// TODO: @file:OptIn(ExperimentalContracts::class) - no C++ equivalent
// TODO: @file:Suppress - no C++ equivalent
// TODO: suspend functions and coroutine builders not directly supported
// TODO: Contracts API not available in C++

namespace kotlinx {
namespace coroutines {

// TODO: import statements removed
// TODO: import kotlinx.atomicfu.* - use std::atomic
// TODO: import kotlinx.coroutines.internal.*
// TODO: import kotlinx.coroutines.intrinsics.*
// TODO: import kotlinx.coroutines.selects.*
// TODO: import kotlin.contracts.*
// TODO: import kotlin.coroutines.*
// TODO: import kotlin.coroutines.intrinsics.*
// TODO: import kotlin.jvm.*

// --------------- launch ---------------

/**
 * Launches a new coroutine without blocking the current thread and returns a reference to the coroutine as a [Job].
 * The coroutine is cancelled when the resulting job is [cancelled][Job.cancel].
 *
 * The coroutine context is inherited from a [CoroutineScope]. Additional context elements can be specified with [context] argument.
 * If the context does not have any dispatcher nor any other [ContinuationInterceptor], then [Dispatchers.Default] is used.
 * The parent job is inherited from a [CoroutineScope] as well, but it can also be overridden
 * with a corresponding [context] element.
 *
 * By default, the coroutine is immediately scheduled for execution.
 * Other start options can be specified via `start` parameter. See [CoroutineStart] for details.
 * An optional [start] parameter can be set to [CoroutineStart.LAZY] to start coroutine _lazily_. In this case,
 * the coroutine [Job] is created in _new_ state. It can be explicitly started with [start][Job.start] function
 * and will be started implicitly on the first invocation of [join][Job.join].
 *
 * Uncaught exceptions in this coroutine cancel the parent job in the context by default
 * (unless [CoroutineExceptionHandler] is explicitly specified), which means that when `launch` is used with
 * the context of another coroutine, then any uncaught exception leads to the cancellation of the parent coroutine.
 *
 * See [newCoroutineContext] for a description of debugging facilities that are available for a newly created coroutine.
 *
 * @param context additional to [CoroutineScope.coroutineContext] context of the coroutine.
 * @param start coroutine start option. The default value is [CoroutineStart.DEFAULT].
 * @param block the coroutine code which will be invoked in the context of the provided scope.
 **/
// TODO: Extension function on CoroutineScope - free function or method
// TODO: suspend block - coroutine lambda
Job* launch(
    CoroutineScope* scope,
    CoroutineContext context = EmptyCoroutineContext,
    CoroutineStart start = CoroutineStart::DEFAULT,
    std::function<void(CoroutineScope*)> block
) {
    auto new_context = scope->newCoroutineContext(context);
    StandaloneCoroutine* coroutine;
    if (start.isLazy) {
        coroutine = new LazyStandaloneCoroutine(new_context, block);
    } else {
        coroutine = new StandaloneCoroutine(new_context, true);
    }
    coroutine->start(start, coroutine, block);
    return coroutine;
}

// --------------- async ---------------

/**
 * Creates a coroutine and returns its future result as an implementation of [Deferred].
 * The running coroutine is cancelled when the resulting deferred is [cancelled][Job.cancel].
 * The resulting coroutine has a key difference compared with similar primitives in other languages
 * and frameworks: it cancels the parent job (or outer scope) on failure to enforce *structured concurrency* paradigm.
 * To change that behaviour, supervising parent ([SupervisorJob] or [supervisorScope]) can be used.
 *
 * Coroutine context is inherited from a [CoroutineScope], additional context elements can be specified with [context] argument.
 * If the context does not have any dispatcher nor any other [ContinuationInterceptor], then [Dispatchers.Default] is used.
 * The parent job is inherited from a [CoroutineScope] as well, but it can also be overridden
 * with corresponding [context] element.
 *
 * By default, the coroutine is immediately scheduled for execution.
 * Other options can be specified via `start` parameter. See [CoroutineStart] for details.
 * An optional [start] parameter can be set to [CoroutineStart.LAZY] to start coroutine _lazily_. In this case,
 * the resulting [Deferred] is created in _new_ state. It can be explicitly started with [start][Job.start]
 * function and will be started implicitly on the first invocation of [join][Job.join], [await][Deferred.await] or [awaitAll].
 *
 * @param block the coroutine code.
 */
// TODO: Template parameter T for return type
// TODO: suspend block
template<typename T>
Deferred<T>* async(
    CoroutineScope* scope,
    CoroutineContext context = EmptyCoroutineContext,
    CoroutineStart start = CoroutineStart::DEFAULT,
    std::function<T(CoroutineScope*)> block
) {
    auto new_context = scope->newCoroutineContext(context);
    DeferredCoroutine<T>* coroutine;
    if (start.isLazy) {
        coroutine = new LazyDeferredCoroutine<T>(new_context, block);
    } else {
        coroutine = new DeferredCoroutine<T>(new_context, true);
    }
    coroutine->start(start, coroutine, block);
    return coroutine;
}

// TODO: @OptIn(InternalForInheritanceCoroutinesApi::class) - no C++ equivalent
// TODO: @Suppress("UNCHECKED_CAST") - no C++ equivalent
// TODO: private open class - access control
template<typename T>
class DeferredCoroutine : public AbstractCoroutine<T>, public Deferred<T> {
private:
    CoroutineContext parent_context;
    bool active_flag;

public:
    DeferredCoroutine(CoroutineContext parentContext, bool active)
        : AbstractCoroutine<T>(parentContext, true, active),
          parent_context(parentContext),
          active_flag(active) {}

    T getCompleted() override {
        // TODO: getCompletedInternal() as T - cast
        return static_cast<T>(this->getCompletedInternal());
    }

    // TODO: suspend fun
    T await() override {
        // TODO: awaitInternal() as T - cast
        return static_cast<T>(this->awaitInternal());
    }

    // TODO: val onAwait property
    SelectClause1<T>* get_onAwait() override {
        // TODO: onAwaitInternal as SelectClause1<T> - cast
        return static_cast<SelectClause1<T>*>(this->onAwaitInternal);
    }
};

// TODO: private class
template<typename T>
class LazyDeferredCoroutine : public DeferredCoroutine<T> {
private:
    std::function<T(CoroutineScope*)> block;
    // TODO: Continuation<T> continuation
    void* continuation;

public:
    LazyDeferredCoroutine(
        CoroutineContext parentContext,
        std::function<T(CoroutineScope*)> block_param
    ) : DeferredCoroutine<T>(parentContext, false),
        block(block_param),
        continuation(nullptr)
    {
        // TODO: block.createCoroutineUnintercepted(this, this)
        // continuation = block.createCoroutineUnintercepted(this, this);
    }

    void onStart() override {
        // TODO: continuation.startCoroutineCancellable(this)
        // continuation->startCoroutineCancellable(this);
    }
};

// --------------- withContext ---------------

/**
 * Calls the specified suspending block with a given coroutine context, suspends until it completes, and returns
 * the result.
 *
 * The resulting context for the [block] is derived by merging the current [coroutineContext] with the
 * specified [context] using `coroutineContext + context` (see [CoroutineContext.plus]).
 * This suspending function is cancellable. It immediately checks for cancellation of
 * the resulting context and throws [CancellationException] if it is not [active][CoroutineContext.isActive].
 *
 * Calls to [withContext] whose [context] argument provides a [CoroutineDispatcher] that is
 * different from the current one, by necessity, perform additional dispatches: the [block]
 * can not be executed immediately and needs to be dispatched for execution on
 * the passed [CoroutineDispatcher], and then when the [block] completes, the execution
 * has to shift back to the original dispatcher.
 *
 * Note that the result of `withContext` invocation is dispatched into the original context in a cancellable way
 * with a **prompt cancellation guarantee**, which means that if the original [coroutineContext]
 * in which `withContext` was invoked is cancelled by the time its dispatcher starts to execute the code,
 * it discards the result of `withContext` and throws [CancellationException].
 *
 * The cancellation behaviour described above is enabled if and only if the dispatcher is being changed.
 * For example, when using `withContext(NonCancellable) { ... }` there is no change in dispatcher and
 * this call will not be cancelled neither on entry to the block inside `withContext` nor on exit from it.
 */
// TODO: suspend fun
// TODO: contract { callsInPlace(...) } - no C++ equivalent
template<typename T>
T withContext(
    CoroutineContext context,
    std::function<T(CoroutineScope*)> block
) {
    // TODO: contract - no C++ equivalent
    // TODO: suspendCoroutineUninterceptedOrReturn - coroutine intrinsics
    // TODO: Implementation requires full coroutine support
    // Placeholder implementation
    return T();
}

/**
 * Calls the specified suspending block with the given [CoroutineDispatcher], suspends until it
 * completes, and returns the result.
 *
 * This inline function calls [withContext].
 */
// TODO: suspend inline operator fun - operator overload
// TODO: Extension on CoroutineDispatcher
template<typename T>
T invoke(CoroutineDispatcher* dispatcher, std::function<T(CoroutineScope*)> block) {
    return withContext<T>(dispatcher, block);
}

// --------------- implementation ---------------

// TODO: private open class
class StandaloneCoroutine : public AbstractCoroutine<void> {
private:
    CoroutineContext parent_context;
    bool active_flag;

public:
    StandaloneCoroutine(CoroutineContext parentContext, bool active)
        : AbstractCoroutine<void>(parentContext, true, active),
          parent_context(parentContext),
          active_flag(active) {}

    bool handleJobException(Throwable* exception) override {
        handleCoroutineException(this->context, exception);
        return true;
    }
};

// TODO: private class
class LazyStandaloneCoroutine : public StandaloneCoroutine {
private:
    std::function<void(CoroutineScope*)> block;
    void* continuation;

public:
    LazyStandaloneCoroutine(
        CoroutineContext parentContext,
        std::function<void(CoroutineScope*)> block_param
    ) : StandaloneCoroutine(parentContext, false),
        block(block_param),
        continuation(nullptr)
    {
        // TODO: block.createCoroutineUnintercepted(this, this)
        // continuation = block.createCoroutineUnintercepted(this, this);
    }

    void onStart() override {
        // TODO: continuation.startCoroutineCancellable(this)
        // continuation->startCoroutineCancellable(this);
    }
};

// Used by withContext when context changes, but dispatcher stays the same
// TODO: internal expect class - platform-specific, use virtual/abstract
template<typename T>
class UndispatchedCoroutine; // Forward declaration
// TODO: Expect declaration - platform-specific implementation needed

// TODO: private const val - constexpr
constexpr int UNDECIDED = 0;
constexpr int SUSPENDED = 1;
constexpr int RESUMED = 2;

// Used by withContext when context dispatcher changes
// TODO: internal class
template<typename T>
class DispatchedCoroutine : public ScopeCoroutine<T> {
private:
    CoroutineContext context_val;
    Continuation<T>* u_cont;
    std::atomic<int> _decision;

public:
    DispatchedCoroutine(CoroutineContext context, Continuation<T>* uCont)
        : ScopeCoroutine<T>(context, uCont),
          context_val(context),
          u_cont(uCont),
          _decision(UNDECIDED) {}

private:
    bool trySuspend() {
        while (true) {
            int decision = _decision.load();
            if (decision == UNDECIDED) {
                int expected = UNDECIDED;
                if (_decision.compare_exchange_strong(expected, SUSPENDED)) {
                    return true;
                }
            } else if (decision == RESUMED) {
                return false;
            } else {
                // TODO: error("Already suspended")
                throw std::runtime_error("Already suspended");
            }
        }
    }

    bool tryResume() {
        while (true) {
            int decision = _decision.load();
            if (decision == UNDECIDED) {
                int expected = UNDECIDED;
                if (_decision.compare_exchange_strong(expected, RESUMED)) {
                    return true;
                }
            } else if (decision == SUSPENDED) {
                return false;
            } else {
                // TODO: error("Already resumed")
                throw std::runtime_error("Already resumed");
            }
        }
    }

public:
    void afterCompletion(void* state) override {
        // Call afterResume from afterCompletion and not vice-versa, because stack-size is more
        // important for afterResume implementation
        afterResume(state);
    }

    void afterResume(void* state) override {
        if (tryResume()) return; // completed before getResult invocation -- bail out
        // Resume in a cancellable way because we have to switch back to the original dispatcher
        // TODO: uCont.intercepted().resumeCancellableWith(recoverResult(state, uCont))
        // u_cont->intercepted()->resumeCancellableWith(recoverResult(state, u_cont));
    }

    // TODO: internal fun
    void* getResult() {
        if (trySuspend()) {
            // TODO: return COROUTINE_SUSPENDED
            return COROUTINE_SUSPENDED;
        }
        // otherwise, onCompletionInternal was already invoked & invoked tryResume, and the result is in the state
        auto* state = this->state.unboxState();
        if (auto* completed_exceptionally = dynamic_cast<CompletedExceptionally*>(state)) {
            throw completed_exceptionally->cause;
        }
        // TODO: @Suppress("UNCHECKED_CAST")
        return static_cast<T>(state);
    }
};

} // namespace coroutines
} // namespace kotlinx
