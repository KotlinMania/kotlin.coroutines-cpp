#include <string>
#include "kotlinx/coroutines/core_fwd.hpp"
// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/DispatchedContinuation.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: atomicfu needs C++ atomic equivalent
// TODO: Continuation<T> struct needs C++ implementation
// TODO: CoroutineDispatcher, CoroutineContext, CoroutineStackFrame need C++ equivalents
// TODO: Symbol class and constants need proper implementation
// TODO: @JvmField annotation - JVM-specific, remove or comment
// TODO: Extension functions need free function implementations
// TODO: MODE constants and inline functions need proper translation

#include <atomic>
#include <functional>

namespace kotlinx {
namespace coroutines {
namespace {

// Forward declarations
class Symbol;
class CoroutineDispatcher;
template<typename T> class Continuation;
template<typename T> class CancellableContinuationImpl;
template<typename T> class DispatchedTask;
class CoroutineStackFrame;
class CoroutineContext;
class Job;
class ThreadLocalEventLoop;

// TODO: Symbol declarations need implementation
extern Symbol* UNDEFINED;
extern Symbol* REUSABLE_CLAIMED;

template<typename T>
class DispatchedContinuation : DispatchedTask<T>, CoroutineStackFrame {
public:
    CoroutineDispatcher* dispatcher;
    Continuation<T>* continuation;

private:
    void* _state;
    std::atomic<void*> _reusable_cancellable_continuation;

public:
    CoroutineStackFrame* caller_frame;
    void* count_or_element; // pre-cached value to avoid ctx.fold on every resumption

    DispatchedContinuation(CoroutineDispatcher* dispatcher, Continuation<T>* continuation)
        : DispatchedTask<T>(MODE_UNINITIALIZED),
          dispatcher(dispatcher),
          continuation(continuation),
          _state(UNDEFINED),
          _reusable_cancellable_continuation(nullptr),
          caller_frame(nullptr), // TODO: dynamic_cast<CoroutineStackFrame*>(continuation)
          count_or_element(nullptr) { // TODO: thread_context_elements(context)
    }

    CoroutineStackFrame* get_caller_frame() override { return caller_frame; }
    void* get_stack_trace_element() override { return nullptr; }

    /**
     * Possible states of reusability:
     *
     * 1) `nullptr`. Cancellable continuation wasn't yet attempted to be reused or
     *     was used and then invalidated (e.g. because of the cancellation).
     * 2) [CancellableContinuation]. Continuation to be/that is being reused.
     * 3) [REUSABLE_CLAIMED]. CC is currently being reused and its owner executes `suspend` block:
     *    ```
     *    // state == nullptr | CC
     *    suspendCancellableCoroutineReusable { cont ->
     *        // state == REUSABLE_CLAIMED
     *        block(cont)
     *    }
     *    // state == CC
     *    ```
     * 4) [Throwable] continuation was cancelled with this cause while being in [suspendCancellableCoroutineReusable],
     *    [CancellableContinuationImpl.getResult] will check for cancellation later.
     *
     * [REUSABLE_CLAIMED] state is required to prevent double-use of the reused continuation.
     * In the `getResult`, we have the following code:
     * ```
     * if (trySuspend()) {
     *     // <- at this moment current continuation can be redispatched and claimed again.
     *     attachChildToParent()
     *     releaseClaimedContinuation()
     * }
     * ```
     */

    CancellableContinuationImpl<T>* reusable_cancellable_continuation() {
        void* value = _reusable_cancellable_continuation.load();
        // TODO: dynamic_cast or type checking
        return static_cast<CancellableContinuationImpl<T>*>(value);
    }

    bool is_reusable() {
        /*
        Invariant: caller.resumeMode.isReusableMode
         * Reusability control:
         * `nullptr` -> no reusability at all, `false`
         * anything else -> reusable.
         */
        return _reusable_cancellable_continuation.load() != nullptr;
    }

    /**
     * Awaits until previous call to `suspendCancellableCoroutineReusable` will
     * stop mutating cached instance
     */
    void await_reusability() {
        while (true) {
            void* it = _reusable_cancellable_continuation.load();
            if (it != REUSABLE_CLAIMED) return;
        }
    }

    void release() {
        /*
         * Called from `releaseInterceptedContinuation`, can be concurrent with
         * the code in `getResult` right after `trySuspend` returned `true`, so we have
         * to wait for a release here.
         */
        await_reusability();
        auto* cc = reusable_cancellable_continuation();
        if (cc) {
            // TODO: cc->detach_child();
        }
    }

    /**
     * Claims the continuation for [suspendCancellableCoroutineReusable] block,
     * so all cancellations will be postponed.
     */
    CancellableContinuationImpl<T>* claim_reusable_cancellable_continuation() {
        /*
         * Transitions:
         * 1) `nullptr` -> claimed, caller will instantiate CC instance
         * 2) `CC` -> claimed, caller will reuse CC instance
         */
        while (true) {
            void* state = _reusable_cancellable_continuation.load();
            if (state == nullptr) {
                /*
                 * nullptr -> CC was not yet published -> we do not compete with cancel
                 * -> can use plain store instead of CAS
                 */
                _reusable_cancellable_continuation.store(REUSABLE_CLAIMED);
                return nullptr;
            } else if (/* state is CancellableContinuationImpl */ true) { // TODO: type check
                void* expected = state;
                if (_reusable_cancellable_continuation.compare_exchange_strong(expected, REUSABLE_CLAIMED)) {
                    return static_cast<CancellableContinuationImpl<T>*>(state);
                }
            } else if (state == REUSABLE_CLAIMED) {
                // Do nothing, wait until reusable instance will be returned from
                // getResult() of a previous `suspendCancellableCoroutineReusable`
            } else if (/* state is Throwable */ true) { // TODO: type check
                // Also do nothing, Throwable can only indicate that the CC
                // is in REUSABLE_CLAIMED state, but with postponed cancellation
            } else {
                // TODO: error("Inconsistent state $state")
            }
        }
    }

    /**
     * Checks whether there were any attempts to cancel reusable CC while it was in [REUSABLE_CLAIMED] state
     * and returns cancellation cause if so, `nullptr` otherwise.
     * If continuation was cancelled, it becomes non-reusable.
     *
     * ```
     * suspendCancellableCoroutineReusable { // <- claimed
     * // Any asynchronous cancellation is "postponed" while this block
     * // is being executed
     * } // postponed cancellation is checked here in `getResult`
     * ```
     *
     * See [CancellableContinuationImpl.getResult].
     */
    void* try_release_claimed_continuation(CancellableContinuation<T>* continuation) {
        while (true) {
            void* state = _reusable_cancellable_continuation.load();
            if (state == REUSABLE_CLAIMED) {
                void* expected = REUSABLE_CLAIMED;
                if (_reusable_cancellable_continuation.compare_exchange_strong(expected, continuation)) {
                    return nullptr;
                }
            } else if (/* state is Throwable */ true) { // TODO: type check
                void* expected = state;
                // TODO: require(_reusable_cancellable_continuation.compare_exchange_strong(expected, nullptr))
                _reusable_cancellable_continuation.compare_exchange_strong(expected, nullptr);
                return state;
            } else {
                // TODO: error("Inconsistent state $state")
            }
        }
    }

    /**
     * Tries to postpone cancellation if reusable CC is currently in [REUSABLE_CLAIMED] state.
     * Returns `true` if cancellation is (or previously was) postponed, `false` otherwise.
     */
    bool postpone_cancellation(void* cause) {
        while (true) {
            void* state = _reusable_cancellable_continuation.load();
            if (state == REUSABLE_CLAIMED) {
                void* expected = REUSABLE_CLAIMED;
                if (_reusable_cancellable_continuation.compare_exchange_strong(expected, cause)) {
                    return true;
                }
            } else if (/* state is Throwable */ true) { // TODO: type check
                return true;
            } else {
                // Invalidate
                void* expected = state;
                if (_reusable_cancellable_continuation.compare_exchange_strong(expected, nullptr)) {
                    return false;
                }
            }
        }
    }

    void* take_state() override {
        void* state = _state;
        // TODO: assert { state != UNDEFINED } // fail-fast if repeatedly invoked
        _state = UNDEFINED;
        return state;
    }

    Continuation<T>* get_delegate() override {
        return this;
    }

    void resume_with(void* result) override {
        // TODO: auto state = result.toState()
        void* state = result; // placeholder
        // TODO: if (dispatcher.safeIsDispatchNeeded(context))
        bool needs_dispatch = true; // placeholder
        if (needs_dispatch) {
            _state = state;
            this->resume_mode = MODE_ATOMIC;
            // TODO: dispatcher.safeDispatch(context, this)
        } else {
            // TODO: executeUnconfined implementation
            // with_coroutine_context(context, count_or_element) {
            //     continuation.resumeWith(result)
            // }
        }
    }

    // We inline it to save an entry on the stack in cases where it shows (unconfined dispatcher)
    // It is used only in Continuation<T>.resumeCancellableWith
    void resume_cancellable_with(void* result) {
        // TODO: auto state = result.toState()
        void* state = result; // placeholder
        // TODO: if (dispatcher.safeIsDispatchNeeded(context))
        bool needs_dispatch = true; // placeholder
        if (needs_dispatch) {
            _state = state;
            this->resume_mode = MODE_CANCELLABLE;
            // TODO: dispatcher.safeDispatch(context, this)
        } else {
            // TODO: executeUnconfined implementation
            // if (!resume_cancelled(state)) {
            //     resume_undispatched_with(result)
            // }
        }
    }

    bool resume_cancelled(void* state) {
        // TODO: auto job = context[Job]
        Job* job = nullptr; // placeholder
        if (job != nullptr /* && !job.isActive */) {
            // TODO: auto cause = job.getCancellationException()
            // cancel_completed_result(state, cause)
            // resume_with_exception(cause)
            return true;
        }
        return false;
    }

    void resume_undispatched_with(void* result) {
        // TODO: with_continuation_context(continuation, count_or_element) {
        //     continuation.resumeWith(result)
        // }
    }

    // used by "yield" implementation
    void dispatch_yield(CoroutineContext* context, T value) {
        _state = &value; // TODO: proper value handling
        this->resume_mode = MODE_CANCELLABLE;
        // TODO: dispatcher.dispatchYield(context, this)
    }

    // TODO: tostd::string implementation
};

// TODO: Extension functions need implementation as free functions

// TODO: MODE constants need definition
constexpr int MODE_ATOMIC = 0;
constexpr int MODE_CANCELLABLE = 1;
constexpr int MODE_CANCELLABLE_REUSABLE = 2;
constexpr int MODE_UNDISPATCHED = 4;
constexpr int MODE_UNINITIALIZED = -1;

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
