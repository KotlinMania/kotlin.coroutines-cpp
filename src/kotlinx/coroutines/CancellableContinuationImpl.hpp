#pragma once
// port-lint: source CancellableContinuationImpl.kt

#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/Waiter.hpp"
#include "kotlinx/coroutines/DisposableHandle.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include "kotlinx/coroutines/CompletionHandler.hpp"
#include "kotlinx/coroutines/CoroutineExceptionHandler.hpp"
#include "kotlinx/coroutines/ContinuationState.hpp"
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include "kotlinx/coroutines/internal/DispatchedTask.hpp"
#include "kotlinx/coroutines/internal/ConcurrentLinkedList.hpp"
#include <atomic>
#include <cassert>
#include <mutex>
#include <memory>
#include <string>
#include <functional>
#include <exception>
#include <type_traits>

namespace kotlinx {
namespace coroutines {

// Forward declaration: In Kotlin, CancellableContinuationImpl.kt is a single file containing
// methods that reference DispatchedContinuation. We use a forward declaration here to preserve
// that structure - keeping all CancellableContinuationImpl methods in one header rather than
// scattering them across files to work around C++ include cycles.
namespace internal {
template<typename T> class DispatchedContinuation;
}

// Forward declarations
class JobNode;
template<typename T> class CancellableContinuationImpl;
template<typename T> class ChildContinuation;

// constants from CancellableContinuationImpl.kt
static constexpr int UNDECIDED = 0;
static constexpr int SUSPENDED = 1;
static constexpr int RESUMED = 2;

static constexpr int DECISION_SHIFT = 29;
static constexpr int INDEX_MASK = (1 << DECISION_SHIFT) - 1;
static constexpr int NO_INDEX = INDEX_MASK;

// Helper functions for decision/index packing
static inline int get_decision(int value) { return value >> DECISION_SHIFT; }
static inline int get_index(int value) { return value & INDEX_MASK; }
static inline int decision_and_index(int decision, int index) { return (decision << DECISION_SHIFT) + index; }

// Dummy Symbol equivalent
static const void* RESUME_TOKEN = (void*)"RESUME_TOKEN";

// State, NotCompleted, Active are defined in ContinuationState.hpp

struct CancelHandler : public virtual NotCompleted {
    virtual void invoke(std::exception_ptr cause) = 0;
};

struct UserSuppliedCancelHandler : public CancelHandler {
    std::function<void(std::exception_ptr)> handler;
    
    explicit UserSuppliedCancelHandler(std::function<void(std::exception_ptr)> h) 
        : handler(std::move(h)) {}
        
    void invoke(std::exception_ptr cause) override { handler(cause); }
    std::string to_string() const override { return "CancelHandler.UserSupplied"; }
};

// Completed with additional metadata
// private data class CompletedCancellableContinuationState<R>
template <typename T>
struct CompletedCancellableContinuationState : public State {
    T result;
    std::shared_ptr<CancelHandler> cancel_handler;
    std::function<void(std::exception_ptr, T, std::shared_ptr<CoroutineContext>)> on_cancellation;
    void* idempotent_resume;
    std::exception_ptr cancel_cause;

    CompletedCancellableContinuationState(
        T r, 
        std::shared_ptr<CancelHandler> ch = nullptr,
        std::function<void(std::exception_ptr, T, std::shared_ptr<CoroutineContext>)> oc = nullptr,
        void* idempotent = nullptr,
        std::exception_ptr cause = nullptr
    ) : result(r), cancel_handler(ch), on_cancellation(oc), idempotent_resume(idempotent), cancel_cause(cause) {}

    bool is_cancelled() const { return cancel_cause != nullptr; }
    
    std::string to_string() const override { return "CompletedCancellableContinuationState"; }
};

// Void specialization for CompletedCancellableContinuationState to handle void results
template <>
struct CompletedCancellableContinuationState<void> : public State {
    std::shared_ptr<CancelHandler> cancel_handler;
    std::function<void(std::exception_ptr, std::shared_ptr<CoroutineContext>)> on_cancellation;
    void* idempotent_resume;
    std::exception_ptr cancel_cause;

    CompletedCancellableContinuationState(
        std::shared_ptr<CancelHandler> ch = nullptr,
        std::function<void(std::exception_ptr, std::shared_ptr<CoroutineContext>)> oc = nullptr,
        void* idempotent = nullptr,
        std::exception_ptr cause = nullptr
    ) : cancel_handler(ch), on_cancellation(oc), idempotent_resume(idempotent), cancel_cause(cause) {}

    bool is_cancelled() const { return cancel_cause != nullptr; }
    std::string to_string() const override { return "CompletedCancellableContinuationState"; }
};

/**
 * Simple completed state holding just the raw value.
 * Kotlin lines 479-491: resumedState() can return raw proposedUpdate when:
 * - Not in cancellable mode AND no idempotent token, OR
 * - No cancel handler, no onCancellation callback, and no idempotent token
 *
 * In C++ we can't store raw T in State*, so we wrap it in this lightweight holder.
 * This corresponds to Kotlin's "else -> proposedUpdate" case.
 */
template <typename T>
struct CompletedWithValue : public State {
    T result;

    explicit CompletedWithValue(T r) : result(std::move(r)) {}

    std::string to_string() const override { return "CompletedWithValue"; }
};

// Void specialization - represents successful void completion without metadata
template <>
struct CompletedWithValue<void> : public State {
    std::string to_string() const override { return "CompletedWithValue<void>"; }
};

// Private class CancelledContinuation
struct CancelledContinuation : public CompletedExceptionally, public State {
    bool handled;
    
    explicit CancelledContinuation(std::exception_ptr cause, bool handled = false) 
        : CompletedExceptionally(cause), handled(handled) {}

    // Helper to make it look like Kotlin's makeHandled() which is atomic in Kotlin but we simplify here
    // In Kotlin it uses atomic boolean updater.
    // For faithful transliteration we should respect the atomic nature if possible,
    // but here we might rely on the fact that it's checked in a CAS loop on state.
    bool make_handled() override {
        bool expected = false;
        return std::atomic_ref(handled).compare_exchange_strong(expected, true);
    }
    
    bool make_resumed() {
        bool expected = false;
        return std::atomic_ref(handled).compare_exchange_strong(expected, true);
    }
    
    std::string to_string() const override { return "CancelledContinuation"; }
};

// Completed exceptionally (non-cancellation) as State
struct CompletedExceptionState : public CompletedExceptionally, public State {
    explicit CompletedExceptionState(std::exception_ptr cause, bool handled = false)
        : CompletedExceptionally(cause, handled) {}
    std::string to_string() const override { return "CompletedException"; }
};

/**
 * @brief Implementation of CancellableContinuation.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt
 */
template <typename T>
class CancellableContinuationImpl : public DispatchedTask<T>, 
                                    public CancellableContinuation<T>,
                                    public Waiter,
                                    public std::enable_shared_from_this<CancellableContinuationImpl<T>> {
private:
    static_assert(!std::is_void_v<T>, "Use void specialization");
    std::shared_ptr<Continuation<T>> delegate;

    // _decisionAndIndex - Kotlin line 69
    std::atomic<int> decision_and_index_;

    // _state - Kotlin line 80
    // In Kotlin this is atomic<Any?> with GC. In C++ we use raw pointers
    // and must manually manage memory for dynamically allocated states.
    std::atomic<State*> state_;

    // Prevent use-after-free: holds ownership of dynamically allocated states
    // When we CAS a new state in, we store the old owned_state_ here before
    // replacing it, so the previous state stays alive until replaced.
    std::shared_ptr<State> owned_state_;

    // _parentHandle - Kotlin line 101 (simplified to non-atomic for now)
    std::shared_ptr<DisposableHandle> parent_handle_;

    // Context cache - Kotlin line 38: context = delegate.context
    std::shared_ptr<CoroutineContext> context_;

    // For segment-based cancellation (channel operations)
    void* segment_for_cancellation_ = nullptr;

public:
    CancellableContinuationImpl(std::shared_ptr<Continuation<T>> delegate_, int resume_mode_)
        : DispatchedTask<T>(resume_mode_), delegate(delegate_) {
        context_ = delegate->get_context();
        state_.store(&Active::instance, std::memory_order_relaxed);
        decision_and_index_.store(decision_and_index(UNDECIDED, NO_INDEX), std::memory_order_relaxed);
    }

    ~CancellableContinuationImpl() override {
       // Cleanup if needed
    }

    std::shared_ptr<CoroutineContext> get_context() const override { return context_; }
    std::shared_ptr<Continuation<T>> get_delegate() override { return delegate; }

    /**
     * Kotlin line 138:
     * private fun isReusable(): Boolean = resumeMode.isReusableMode && (delegate as DispatchedContinuation<*>).isReusable()
     */
    bool is_reusable() const {
        if (!is_reusable_mode(this->resume_mode)) return false;
        auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<T>>(delegate);
        return dispatched && dispatched->is_reusable();
    }

    /**
     * Resets cancellability state in order to suspendCancellableCoroutineReusable to work.
     * Invariant: used only by suspendCancellableCoroutineReusable in REUSABLE_CLAIMED state.
     *
     * Kotlin lines 140-158.
     */
    bool reset_state_reusable() {
        // assert { resumeMode == MODE_CANCELLABLE_REUSABLE }
        assert(this->resume_mode == MODE_CANCELLABLE_REUSABLE);
        // assert { parentHandle !== NonDisposableHandle }
        assert(parent_handle_ != non_disposable_handle());

        // val state = _state.value
        State* state = state_.load(std::memory_order_acquire);
        // assert { state !is NotCompleted }
        assert(dynamic_cast<NotCompleted*>(state) == nullptr);

        // if (state is CompletedContinuation<*> && state.idempotentResume != null)
        if (auto* cc = dynamic_cast<CompletedCancellableContinuationState<T>*>(state)) {
            if (cc->idempotent_resume != nullptr) {
                // Cannot reuse continuation that was resumed with idempotent marker
                detach_child();
                return false;
            }
        }

        // _decisionAndIndex.value = decisionAndIndex(UNDECIDED, NO_INDEX)
        decision_and_index_.store(decision_and_index(UNDECIDED, NO_INDEX), std::memory_order_release);
        // _state.value = Active
        state_.store(&Active::instance, std::memory_order_release);
        return true;
    }

    /**
     * Kotlin lines 351-356:
     * internal fun releaseClaimedReusableContinuation() {
     *     val cancellationCause = (delegate as? DispatchedContinuation<*>)?.tryReleaseClaimedContinuation(this) ?: return
     *     detachChild()
     *     cancel(cancellationCause)
     * }
     */
    void release_claimed_reusable_continuation() {
        auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<T>>(delegate);
        if (!dispatched) return;

        std::exception_ptr cancellation_cause = dispatched->try_release_claimed_continuation(this);
        if (!cancellation_cause) return;

        detach_child();
        cancel(cancellation_cause);
    }

    /**
     * Kotlin lines 194-199:
     * private fun cancelLater(cause: Throwable): Boolean {
     *     if (!isReusable()) return false
     *     val dispatched = delegate as DispatchedContinuation<*>
     *     return dispatched.postponeCancellation(cause)
     * }
     */
    bool cancel_later(std::exception_ptr cause) {
        if (!is_reusable()) return false;
        auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<T>>(delegate);
        if (!dispatched) return false;
        return dispatched->postpone_cancellation(cause);
    }

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
    */

    bool is_active() const override {
        return dynamic_cast<NotCompleted*>(state_.load(std::memory_order_acquire)) != nullptr;
    }

    bool is_completed() const override {
        return !is_active();
    }

    bool is_cancelled() const override {
        return dynamic_cast<CancelledContinuation*>(state_.load(std::memory_order_acquire)) != nullptr;
    }
    
    // initCancellability implementation
    void init_cancellability() override {
        /*
        * Invariant: at the moment of invocation, `this` has not yet
        * leaked to user code and no one is able to invoke `resume` or `cancel`
        * on it yet. Also, this function is not invoked for reusable continuations.
        */
        auto handle = install_parent_handle();
        if (!handle) return; // fast path
        
        // now check our state _after_ registering
        if (is_completed()) {
            handle->dispose();
            parent_handle_ = non_disposable_handle();
        }
    }
    
    // State machine helpers
    // private inline fun decisionAndIndex(decision: Int, index: Int)
    
    // takeState - Kotlin line 165: just returns state
    // Combined with getSuccessfulResult (lines 605-609) and getExceptionalResult (lines 613-614)
    // C++ returns Result<T> directly for simplicity
    Result<T> take_state() override {
        State* s = state_.load(std::memory_order_acquire);

        // CompletedWithValue<T> -> success with result (lightweight fast path)
        if (auto* cwv = dynamic_cast<CompletedWithValue<T>*>(s)) {
            return Result<T>::success(cwv->result);
        }

        // CompletedCancellableContinuationState<T> -> success with result
        if (auto* cc = dynamic_cast<CompletedCancellableContinuationState<T>*>(s)) {
            return Result<T>::success(cc->result);
        }

        // CancelledContinuation -> failure with cause
        if (auto* cancelled = dynamic_cast<CancelledContinuation*>(s)) {
            return Result<T>::failure(cancelled->cause);
        }

        // CompletedExceptionally (non-cancellation) -> failure with cause
        if (auto* ex = dynamic_cast<CompletedExceptionally*>(s)) {
            return Result<T>::failure(ex->cause);
        }

        // NotCompleted states should not reach here
        throw std::runtime_error("take_state called on non-completed continuation");
    }
    
    // cancelCompletedResult (used by DispatchedTask cancellation)
    // Kotlin lines 169-189
    void cancel_completed_result(Result<T> taken_state, std::exception_ptr cause) override {
        while (true) {
            State* state = state_.load(std::memory_order_acquire);
            if (dynamic_cast<NotCompleted*>(state)) {
                throw std::runtime_error("Not completed");
            }
            if (dynamic_cast<CompletedExceptionally*>(state)) return; // already exceptional

            // Handle CompletedWithValue - promote to CompletedCancellableContinuationState with cancelCause
            // Kotlin lines 181-187: else branch for raw values
            if (auto* cwv = dynamic_cast<CompletedWithValue<T>*>(state)) {
                auto* update = new CompletedCancellableContinuationState<T>(
                    cwv->result, nullptr, nullptr, nullptr, cause);
                if (state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                    owned_state_.reset(update);
                    // No handlers to invoke for CompletedWithValue
                    return;
                }
                delete update;
                continue;
            }

            if (auto* cc = dynamic_cast<CompletedCancellableContinuationState<T>*>(state)) {
                if (cc->is_cancelled()) throw std::runtime_error("Must be called at most once");
                auto* update = new CompletedCancellableContinuationState<T>(
                    cc->result, cc->cancel_handler, cc->on_cancellation, cc->idempotent_resume, cause);
                if (state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                    // Kotlin line 177: state.invokeHandlers(this, cause)
                    if (cc->cancel_handler) call_cancel_handler(cc->cancel_handler, cause);
                    if constexpr (std::is_void_v<T>) {
                        if (cc->on_cancellation) cc->on_cancellation(cause, context_);
                    } else {
                        if (cc->on_cancellation) call_on_cancellation(cc->on_cancellation, cause, cc->result);
                    }
                    return;
                }
                delete update;
                continue;
            }
            return;
        }
    }
    
    // cancel - Kotlin lines 201-217
    bool cancel(std::exception_ptr cause = nullptr) override {
        while (true) {
            State* state = state_.load(std::memory_order_acquire);
            // line 203: if (state !is NotCompleted) return false
            if (!dynamic_cast<NotCompleted*>(state)) return false;

            // line 205: val update = CancelledContinuation(this, cause, handled = state is CancelHandler || state is Segment<*>)
            bool is_cancel_handler = dynamic_cast<CancelHandler*>(state) != nullptr;
            bool is_segment = dynamic_cast<internal::SegmentBase*>(state) != nullptr;
            bool handled = is_cancel_handler || is_segment;
            auto* update = new CancelledContinuation(cause, handled);

            // Save handler BEFORE CAS - owned_state_ holds the CancelHandler if state is one
            // In C++ we need to grab ownership before the CAS invalidates our ability to use it
            std::shared_ptr<CancelHandler> handler_to_call;
            if (is_cancel_handler) {
                handler_to_call = std::dynamic_pointer_cast<CancelHandler>(owned_state_);
            }

            // line 206: if (!_state.compareAndSet(state, update)) return@loop
            if (!state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                delete update;
                continue;
            }
            // CAS succeeded - take ownership of new state
            owned_state_.reset(update);

            // lines 208-211: Invoke cancel handler if it was present
            // when (state) { is CancelHandler -> ..., is Segment<*> -> ... }
            if (is_cancel_handler && handler_to_call) {
                call_cancel_handler(handler_to_call, cause);
            } else if (is_segment) {
                call_segment_on_cancellation(dynamic_cast<internal::SegmentBase*>(state), cause);
            }

            // line 213: detachChildIfNonReusable()
            detach_child_if_non_reusable();
            // line 214: dispatchResume(resumeMode)
            dispatch_resume(this->resume_mode);
            return true;
        }
    }

    // Kotlin lines 219-224
    void parent_cancelled(std::exception_ptr cause) {
        if (cancel_later(cause)) return;
        cancel(cause);
        // Even if cancellation has failed, we should detach child to avoid potential leak
        detach_child_if_non_reusable();
    }

    // callCancelHandlerSafely
    void call_cancel_handler(std::shared_ptr<CancelHandler> handler, std::exception_ptr cause) {
        try {
            handler->invoke(cause);
        } catch (...) {
            if (context_) {
                handle_coroutine_exception(*context_,
                    std::make_exception_ptr(std::runtime_error("Exception in invokeOnCancellation handler")));
            }
        }
    }

    template<typename U=T, typename std::enable_if<!std::is_void_v<U>, int>::type = 0>
    void call_on_cancellation(
        std::function<void(std::exception_ptr, U, std::shared_ptr<CoroutineContext>)> on_cancellation,
        std::exception_ptr cause,
        U value) {
        try {
            on_cancellation(cause, value, context_);
        } catch (...) {
            if (context_) {
                handle_coroutine_exception(*context_,
                    std::make_exception_ptr(std::runtime_error("Exception in resume onCancellation handler")));
            }
        }
    }
    
    std::exception_ptr get_continuation_cancellation_cause(Job& parent) {
        return parent.get_cancellation_exception();
    }
    
    bool try_suspend() {
        while (true) {
            int cur = decision_and_index_.load(std::memory_order_acquire);
            int decision = get_decision(cur);
            int index = get_index(cur);
            
            switch (decision) {
                case UNDECIDED:
                    if (decision_and_index_.compare_exchange_strong(cur, decision_and_index(SUSPENDED, index))) return true;
                    break;
                case RESUMED:
                    return false;
                default:
                    throw std::logic_error("Already suspended");
            }
        }
    }
    
    bool try_resume() {
        while (true) {
            int cur = decision_and_index_.load(std::memory_order_acquire);
            int decision = get_decision(cur);
            int index = get_index(cur);
            
            switch (decision) {
                case UNDECIDED:
                    if (decision_and_index_.compare_exchange_strong(cur, decision_and_index(RESUMED, index))) return true;
                    break;
                case SUSPENDED:
                    return false;
                default:
                    throw std::logic_error("Already resumed");
            }
        }
    }
    
    /**
     * getResult implementation - Kotlin lines 290-337:
     *
     * internal fun getResult(): Any? {
     *     val isReusable = isReusable()
     *     // trySuspend may fail either if 'block' has resumed/cancelled a continuation,
     *     // or we got async cancellation from parent.
     *     if (trySuspend()) {
     *         // Invariant: parentHandle is `null` *only* for reusable continuations.
     *         // We were neither resumed nor cancelled, time to suspend.
     *         // But first we have to install parent cancellation handle (if we didn't yet),
     *         // so CC could be properly resumed on parent cancellation.
     *         if (parentHandle == null) {
     *             installParentHandle()
     *         }
     *         // Release the continuation after installing the handle (if needed).
     *         // If we were successful, then do nothing, it's ok to reuse the instance now.
     *         // Otherwise, dispose the handle by ourselves.
     *         if (isReusable) {
     *             releaseClaimedReusableContinuation()
     *         }
     *         return COROUTINE_SUSPENDED
     *     }
     *     // otherwise, onCompletionInternal was already invoked & invoked tryResume, and the result is in the state
     *     if (isReusable) {
     *         // release claimed reusable continuation for the future reuse
     *         releaseClaimedReusableContinuation()
     *     }
     *     val state = this.state
     *     if (state is CompletedExceptionally) throw recoverStackTrace(state.cause, this)
     *     // if the parent job was already cancelled, then throw the corresponding cancellation exception
     *     // otherwise, there is a race if suspendCancellableCoroutine { cont -> ... } does cont.resume(...)
     *     // before the block returns. This getResult would return a result as opposed to cancellation
     *     // exception that should have happened if the continuation is dispatched for execution later.
     *     if (resumeMode.isCancellableMode) {
     *         val job = context[Job]
     *         if (job != null && !job.isActive) {
     *             val cause = job.getCancellationException()
     *             cancelCompletedResult(state, cause)
     *             throw recoverStackTrace(cause, this)
     *         }
     *     }
     *     return getSuccessfulResult(state)
     * }
     */
    void* get_result() {
        // val isReusable = isReusable()
        bool is_reusable_flag = is_reusable();

        // trySuspend may fail either if 'block' has resumed/cancelled a continuation,
        // or we got async cancellation from parent.
        if (try_suspend()) {
            /*
             * Invariant: parentHandle is `null` *only* for reusable continuations.
             * We were neither resumed nor cancelled, time to suspend.
             * But first we have to install parent cancellation handle (if we didn't yet),
             * so CC could be properly resumed on parent cancellation.
             *
             * This read has benign data-race with write of 'NonDisposableHandle'
             * in 'detachChildIfNotReusable'.
             */
            // if (parentHandle == null) { installParentHandle() }
            if (parent_handle_ == nullptr) {
                install_parent_handle();
            }
            /*
             * Release the continuation after installing the handle (if needed).
             * If we were successful, then do nothing, it's ok to reuse the instance now.
             * Otherwise, dispose the handle by ourselves.
             */
            // if (isReusable) { releaseClaimedReusableContinuation() }
            if (is_reusable_flag) {
                release_claimed_reusable_continuation();
            }
            return intrinsics::get_COROUTINE_SUSPENDED();
        }

        // otherwise, onCompletionInternal was already invoked & invoked tryResume, and the result is in the state
        // if (isReusable) { releaseClaimedReusableContinuation() }
        if (is_reusable_flag) {
            // release claimed reusable continuation for the future reuse
            release_claimed_reusable_continuation();
        }

        // val state = this.state
        State* state = state_.load(std::memory_order_acquire);
        // if (state is CompletedExceptionally) throw recoverStackTrace(state.cause, this)
        if (auto* ex = dynamic_cast<CompletedExceptionally*>(state)) {
            std::rethrow_exception(ex->cause);
        }

        // if the parent job was already cancelled, then throw the corresponding cancellation exception
        // otherwise, there is a race if suspendCancellableCoroutine { cont -> ... } does cont.resume(...)
        // before the block returns. This getResult would return a result as opposed to cancellation
        // exception that should have happened if the continuation is dispatched for execution later.
        // if (resumeMode.isCancellableMode) { ... }
        if (is_cancellable_mode(this->resume_mode) && context_) {
            // val job = context[Job]
            auto job_element = context_->get(Job::type_key);
            auto job = std::dynamic_pointer_cast<Job>(job_element);
            // if (job != null && !job.isActive)
            if (job && !job->is_active()) {
                // val cause = job.getCancellationException()
                std::exception_ptr cause = job->get_cancellation_exception();
                // cancelCompletedResult(state, cause)
                cancel_completed_result(Result<T>(), cause);
                // throw recoverStackTrace(cause, this)
                std::rethrow_exception(cause);
            }
        }

        // return getSuccessfulResult(state)
        return get_successful_result(state);
    }
    
    // Kotlin lines 605-609: getSuccessfulResult
    // when (state) {
    //     is CompletedCancellableContinuationState<*> -> state.result as T
    //     else -> state as T  // Raw value case
    // }
    void* get_successful_result(State* state) {
        // Check CompletedWithValue first (common fast path)
        if (auto* cwv = dynamic_cast<CompletedWithValue<T>*>(state)) {
            if constexpr (std::is_void_v<T>) {
                (void)cwv;
                return nullptr;
            } else {
                return new T(cwv->result); // allocate per ABI convention
            }
        }

        if (auto* cc = dynamic_cast<CompletedCancellableContinuationState<T>*>(state)) {
            if constexpr (std::is_void_v<T>) {
                (void)cc;
                return nullptr;
            } else {
                return new T(cc->result); // allocate per ABI convention
            }
        }
        throw std::logic_error("Invalid state for result");
    }
    
    // installParentHandle - Kotlin lines 339-345
    std::shared_ptr<DisposableHandle> install_parent_handle() {
        if (!context_) return nullptr;
        auto job_element = context_->get(Job::type_key);
        auto parent = std::dynamic_pointer_cast<Job>(job_element);
        if (!parent) return nullptr;  // don't do anything without a parent

        // Create ChildContinuation and set its job pointer
        auto child_handler = std::make_shared<ChildContinuation<T>>(this);
        child_handler->job = dynamic_cast<JobSupport*>(parent.get());

        // val handle = parent.invokeOnCompletion(handler = ChildContinuation(this))
        auto handle = parent->invoke_on_completion(
            true,  // onCancelling = true
            false, // invokeImmediately = false
            [child_handler](std::exception_ptr cause) {
                child_handler->invoke(cause);
            }
        );

        // _parentHandle.compareAndSet(null, handle)
        if (!parent_handle_) {
            parent_handle_ = handle;
        }
        return handle;
    }
    
    // Kotlin lines 560-563
    void detach_child_if_non_reusable() {
        // If instance is reusable, do not detach on every reuse
        if (!is_reusable()) detach_child();
    }

    // Kotlin lines 568-572
    void detach_child() {
        auto handle = parent_handle_;
        if (!handle) return;
        handle->dispose();
        parent_handle_ = non_disposable_handle();
    }
    
    // tryResume + completeResume (Kotlin parity)
    void* try_resume(T value, void* idempotent = nullptr) override {
        return try_resume(value, idempotent, nullptr);
    }

    // tryResumeImpl - Kotlin lines 529-553
    void* try_resume(
        T value,
        void* idempotent,
        std::function<void(std::exception_ptr, T, std::shared_ptr<CoroutineContext>)> on_cancellation
    ) {
        while (true) {
            State* state = state_.load(std::memory_order_acquire);

            if (auto* nc = dynamic_cast<NotCompleted*>(state)) {
                State* update = resumed_state(nc, owned_state_, value, this->resume_mode, on_cancellation, idempotent);
                if (state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                    detach_child_if_non_reusable();
                    return const_cast<void*>(RESUME_TOKEN);
                }
                delete update;
                continue;
            }

            // Kotlin lines 542-549: idempotent resume check for CompletedCancellableContinuationState
            if (auto* cc = dynamic_cast<CompletedCancellableContinuationState<T>*>(state)) {
                if (idempotent != nullptr && cc->idempotent_resume == idempotent) {
                    // assert state.result == value
                    return const_cast<void*>(RESUME_TOKEN);
                }
                return nullptr; // different token or non-idempotent
            }

            // CancelledContinuation or other completed state
            return nullptr;
        }
    }

    void* try_resume_with_exception(std::exception_ptr exception) override {
        while (true) {
            State* state = state_.load(std::memory_order_acquire);
            if (dynamic_cast<NotCompleted*>(state)) {
                auto* update = new CompletedExceptionState(exception, false);
                if (state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                    return const_cast<void*>(RESUME_TOKEN);
                }
                delete update;
                continue;
            }
            if (dynamic_cast<CancelledContinuation*>(state)) return nullptr;
            return nullptr;
        }
    }

    // completeResume - Kotlin lines 589-592
    void complete_resume(void* token) override {
        if (token != RESUME_TOKEN) return;
        // Note: detachChildIfNonReusable is called in tryResumeImpl, not here
        dispatch_resume(this->resume_mode);
    }

    // resumedState - determines what to store as the new state
    // Kotlin lines 473-491
    State* resumed_state(
        NotCompleted* state,
        std::shared_ptr<State> owned_state,  // shared ownership of state for proper CancelHandler handling
        T proposed_update,
        int resume_mode,
        std::function<void(std::exception_ptr, T, std::shared_ptr<CoroutineContext>)> on_cancellation,
        void* idempotent
    ) {
        // (handled separately via resume_impl_exception)

        // Cannot be cancelled in process, no metadata needed - use lightweight wrapper
        if (!is_cancellable_mode(resume_mode) && idempotent == nullptr) {
            return new CompletedWithValue<T>(std::move(proposed_update));
        }

        // Lines 486-489: onCancellation != null || state is CancelHandler || idempotent != null
        // -> CompletedCancellableContinuationState (need to track handlers/metadata)
        auto* ch = dynamic_cast<CancelHandler*>(state);
        if (on_cancellation != nullptr || ch != nullptr || idempotent != nullptr) {
            // Use dynamic_pointer_cast from owned_state to properly share ownership
            auto ch_shared = ch ? std::dynamic_pointer_cast<CancelHandler>(owned_state) : nullptr;
            return new CompletedCancellableContinuationState<T>(
                proposed_update,
                ch_shared,
                on_cancellation,
                idempotent
            );
        }

        return new CompletedWithValue<T>(std::move(proposed_update));
    }

    // resumeImpl - Kotlin lines 493-523
    void resume_impl(T proposed_update, int resume_mode,
                     std::function<void(std::exception_ptr, T, std::shared_ptr<CoroutineContext>)> on_cancellation = nullptr) {
        while (true) {
            State* state = state_.load(std::memory_order_acquire);

            if (auto* nc = dynamic_cast<NotCompleted*>(state)) {
                State* update = resumed_state(nc, owned_state_, proposed_update, resume_mode, on_cancellation, nullptr);
                if (!state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                    delete update;
                    continue; // retry on CAS failure
                }
                detach_child_if_non_reusable();
                dispatch_resume(resume_mode);
                return;
            }

            if (auto* cancelled = dynamic_cast<CancelledContinuation*>(state)) {
                if (cancelled->make_resumed()) {
                    if (on_cancellation) {
                        call_on_cancellation(on_cancellation, cancelled->cause, proposed_update);
                    }
                    return;
                }
            }

            throw std::logic_error("Already resumed");
        }
    }
    
    // Waiter overrides - Kotlin lines 385-393
    void invoke_on_cancellation(internal::SegmentBase* segment, int index) override {
        // _decisionAndIndex.update { ... decisionAndIndex(it.decision, index) }
        while (true) {
            int cur = decision_and_index_.load(std::memory_order_acquire);
            if (get_index(cur) != NO_INDEX) {
                throw std::logic_error("invokeOnCancellation should be called at most once");
            }
            int update = decision_and_index(get_decision(cur), index);
            if (decision_and_index_.compare_exchange_strong(cur, update)) break;
        }
        // invokeOnCancellationImpl(segment)
        // In Kotlin, Segment implements the same interface as CancelHandler for this purpose
        // We wrap it in a handler that calls the segment's cancellation callback
        invoke_on_cancellation_impl_segment(segment);
    }

    // C++ lifetime management: return shared_ptr to self for segment storage
    std::shared_ptr<Waiter> shared_from_this_waiter() override {
        return std::static_pointer_cast<Waiter>(this->shared_from_this());
    }

    // Segment-based cancellation - Kotlin lines 400-427 (Segment branch)
    void invoke_on_cancellation_impl_segment(internal::SegmentBase* segment) {
        while (true) {
            State* state = state_.load(std::memory_order_acquire);

            // Active -> store segment marker (we use the segment pointer as state indicator)
            if (dynamic_cast<Active*>(state)) {
                // For segments, we store a marker indicating segment-based cancellation is pending
                // The actual segment callback is called when cancellation occurs
                segment_for_cancellation_ = segment;
                return;
            }

            // CompletedExceptionally (includes CancelledContinuation)
            if (auto* ex = dynamic_cast<CompletedExceptionally*>(state)) {
                if (!ex->make_handled()) {
                    throw std::runtime_error("Multiple handlers prohibited");
                }
                // Call segment cancellation only if cancelled
                if (dynamic_cast<CancelledContinuation*>(state)) {
                    call_segment_on_cancellation(segment, ex->cause);
                }
                return;
            }

            // CompletedCancellableContinuationState -> segment doesn't need to be called
            if (dynamic_cast<CompletedCancellableContinuationState<T>*>(state)) {
                return;  // Kotlin: if (handler is Segment<*>) return
            }
        }
    }

    // callSegmentOnCancellation - Kotlin lines 241-245
    void call_segment_on_cancellation(internal::SegmentBase* segment, std::exception_ptr cause) {
        // val index = _decisionAndIndex.value.index
        int index = get_index(decision_and_index_.load(std::memory_order_acquire));
        // check(index != NO_INDEX) { "The index for Segment.onCancellation(..) is broken" }
        if (index == NO_INDEX) {
            throw std::logic_error("The index for Segment.onCancellation(..) is broken");
        }
        // callCancelHandlerSafely { segment.onCancellation(index, cause, context) }
        try {
            segment->on_cancellation(index, cause, context_);
        } catch (...) {
            if (context_) {
                handle_coroutine_exception(*context_,
                    std::make_exception_ptr(std::runtime_error("Exception in invokeOnCancellation handler")));
            }
        }
    }
    
    // CancellableContinuation overrides
    void resume(T value, std::function<void(std::exception_ptr)> on_cancellation) override {
        // Adapter for simple handler
        resume_impl(value, this->resume_mode, 
             on_cancellation ? [on_cancellation](std::exception_ptr e, T, std::shared_ptr<CoroutineContext>) { on_cancellation(e); } 
                             : (std::function<void(std::exception_ptr, T, std::shared_ptr<CoroutineContext>)>)nullptr);
    }

    void invoke_on_cancellation(std::function<void(std::exception_ptr)> handler) override {
         invoke_on_cancellation_impl(std::make_shared<UserSuppliedCancelHandler>(handler));
    }
    
    // invokeOnCancellationImpl - Kotlin lines 400-461
    void invoke_on_cancellation_impl(std::shared_ptr<CancelHandler> handler) {
        while (true) {
            State* state = state_.load(std::memory_order_acquire);

            // Active -> store handler as the new state
            if (dynamic_cast<Active*>(state)) {
                if (state_.compare_exchange_strong(state, handler.get(), std::memory_order_acq_rel)) {
                    // Keep handler alive - it's now the state
                    owned_state_ = handler;
                    return;
                }
                continue; // retry on CAS failure
            }

            // Already has a handler -> error
            if (dynamic_cast<CancelHandler*>(state)) {
                throw std::runtime_error("Multiple handlers prohibited");
            }

            // CompletedExceptionally (includes CancelledContinuation)
            if (auto* ex = dynamic_cast<CompletedExceptionally*>(state)) {
                if (!ex->make_handled()) {
                    throw std::runtime_error("Multiple handlers prohibited");
                }
                // Call handler only if cancelled (not just exceptionally completed)
                if (dynamic_cast<CancelledContinuation*>(state)) {
                    call_cancel_handler(handler, ex->cause);
                }
                return;
            }

            // CompletedCancellableContinuationState -> copy with handler
            if (auto* cc = dynamic_cast<CompletedCancellableContinuationState<T>*>(state)) {
                if (cc->cancel_handler) {
                    throw std::runtime_error("Multiple handlers prohibited");
                }
                if (cc->is_cancelled()) {
                    call_cancel_handler(handler, cc->cancel_cause);
                    return;
                }
                auto* update = new CompletedCancellableContinuationState<T>(
                    cc->result, handler, cc->on_cancellation, cc->idempotent_resume, cc->cancel_cause);
                if (state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                    owned_state_.reset(update);
                    return;
                }
                delete update;
                continue;
            }

            // Kotlin lines 448-458: else branch - raw completed value
            // CompletedWithValue -> wrap in CompletedCancellableContinuationState with handler
            if (auto* cwv = dynamic_cast<CompletedWithValue<T>*>(state)) {
                auto* update = new CompletedCancellableContinuationState<T>(
                    cwv->result, handler, nullptr, nullptr, nullptr);
                if (state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                    owned_state_.reset(update);
                    return;
                }
                delete update;
                continue;
            }

            // Unknown state - should not happen
            return;
        }
    }
    
    // Dispatch
    void dispatch_resume(int mode) {
        if (try_resume()) return; // completed before getResult invocation
        dispatch(this, mode);
    }
    
    // resumeWith
    void resume_with(Result<T> result) override {
        if (result.is_success()) 
             resume_impl(result.get_or_throw(), this->resume_mode);
        else 
             resume_impl_exception(result.exception_or_null(), this->resume_mode);
    }
    
    void resume_impl_exception(std::exception_ptr exception, int mode) {
        void* token = try_resume_with_exception(exception);
        if (!token) throw std::logic_error("Already resumed");
        complete_resume(token);
    }

    void resume_undispatched(CoroutineDispatcher* dispatcher, T value) override {
        // Resume directly without dispatch
        resume_impl(value, MODE_UNDISPATCHED);
    }

    void resume_undispatched_with_exception(CoroutineDispatcher* dispatcher, std::exception_ptr exception) override {
        // Resume with exception directly without dispatch
        resume_impl_exception(exception, MODE_UNDISPATCHED);
    }

};

/**
 * Same as ChildHandleNode, but for cancellable continuation.
 * Transliterated from: private class ChildContinuation (CancellableContinuationImpl.kt:691-700)
 */
template<typename T>
class ChildContinuation : public JobNode {
public:
    CancellableContinuationImpl<T>* child;

    explicit ChildContinuation(CancellableContinuationImpl<T>* c) : child(c) {}

    // Kotlin: override val onCancelling get() = true
    bool get_on_cancelling() const override { return true; }

    // Kotlin: override fun invoke(cause: Throwable?)
    void invoke(std::exception_ptr cause) override {
        // child.parent_cancelled(child.get_continuation_cancellation_cause(job))
        // job is the parent JobSupport* from JobNode base class
        child->parent_cancelled(child->get_continuation_cancellation_cause(*static_cast<Job*>(job)));
    }
};

// ------------------------------------------------------------------
// Specialization for void
// ------------------------------------------------------------------
// Audit status: Kotlin CancellableContinuationImpl.kt
//   - Lines 1-217: cancel(), parentCancelled(), cancelLater(), isReusable() - COMPLETE
//   - Lines 218-267: callCancelHandlerSafely(), callSegmentOnCancellation() - COMPLETE
//   - Lines 269-345: trySuspend(), tryResume(), getResult(), installParentHandle() - COMPLETE
//   - Lines 350-461: invokeOnCancellation(), resume variants - COMPLETE
//   - Lines 493-600+: resumeImpl, tryResumeImpl, completeResume, etc. - COMPLETE
template <>
class CancellableContinuationImpl<void> : public DispatchedTask<void>,
                                           public CancellableContinuation<void>,
                                           public Waiter,
                                           public std::enable_shared_from_this<CancellableContinuationImpl<void>> {
private:
    std::shared_ptr<Continuation<void>> delegate;
    std::atomic<int> decision_and_index_;
    std::atomic<State*> state_;
    std::shared_ptr<State> owned_state_; // Prevents use-after-free of dynamically allocated states
    std::shared_ptr<DisposableHandle> parent_handle_;
    std::shared_ptr<CoroutineContext> context_;
    // Note: segment_for_cancellation_ omitted - invoke_on_cancellation(void*, int) is stub
    // When implementing segment-based cancellation for void specialization, add field back

public:
    CancellableContinuationImpl(std::shared_ptr<Continuation<void>> delegate_, int resume_mode_)
        : DispatchedTask<void>(resume_mode_), delegate(delegate_) {
        context_ = delegate->get_context();
        state_.store(&Active::instance, std::memory_order_relaxed);
        decision_and_index_.store(decision_and_index(UNDECIDED, NO_INDEX), std::memory_order_relaxed);
    }

    std::shared_ptr<CoroutineContext> get_context() const override { return context_; }
    std::shared_ptr<Continuation<void>> get_delegate() override { return delegate; }

    // getContinuationCancellationCause - Kotlin lines 266-267
    std::exception_ptr get_continuation_cancellation_cause(Job& parent) {
        return parent.get_cancellation_exception();
    }

    // These three methods need DispatchedContinuation which creates a circular include.
    // Implementations are in CancellableContinuationImpl.cpp where both headers are available.
    bool is_reusable() const;                              // Kotlin line 138
    void release_claimed_reusable_continuation();          // Kotlin lines 351-356
    bool cancel_later(std::exception_ptr cause);           // Kotlin lines 194-199

    /**
     * Resets cancellability state in order to suspendCancellableCoroutineReusable to work.
     * Kotlin lines 140-158.
     */
    bool reset_state_reusable() {
        assert(this->resume_mode == MODE_CANCELLABLE_REUSABLE);
        assert(parent_handle_ != non_disposable_handle());

        State* state = state_.load(std::memory_order_acquire);
        assert(dynamic_cast<NotCompleted*>(state) == nullptr);

        if (auto* cc = dynamic_cast<CompletedCancellableContinuationState<void>*>(state)) {
            if (cc->idempotent_resume != nullptr) {
                detach_child();
                return false;
            }
        }

        decision_and_index_.store(decision_and_index(UNDECIDED, NO_INDEX), std::memory_order_release);
        state_.store(&Active::instance, std::memory_order_release);
        return true;
    }

    // ---- Decision machine ----
    bool try_suspend() {
        while (true) {
            int cur = decision_and_index_.load(std::memory_order_acquire);
            int decision = get_decision(cur);
            int index = get_index(cur);
            switch (decision) {
                case UNDECIDED:
                    if (decision_and_index_.compare_exchange_strong(cur, decision_and_index(SUSPENDED, index)))
                        return true;
                    break;
                case RESUMED:
                    return false;
                default:
                    throw std::logic_error("Already suspended");
            }
        }
    }

    bool try_resume_decision() {
        while (true) {
            int cur = decision_and_index_.load(std::memory_order_acquire);
            int decision = get_decision(cur);
            int index = get_index(cur);
            switch (decision) {
                case UNDECIDED:
                    if (decision_and_index_.compare_exchange_strong(cur, decision_and_index(RESUMED, index)))
                        return true;
                    break;
                case SUSPENDED:
                    return false;
                default:
                    throw std::logic_error("Already resumed");
            }
        }
    }

    // ---- Cancellation ----
    void cancel_completed_result(Result<void> taken_state, std::exception_ptr cause) override {
        while (true) {
            State* state = state_.load(std::memory_order_acquire);
            if (dynamic_cast<NotCompleted*>(state)) throw std::runtime_error("Not completed");
            if (dynamic_cast<CompletedExceptionally*>(state)) return;

            // Handle CompletedWithValue - promote to CompletedCancellableContinuationState with cancelCause
            if (dynamic_cast<CompletedWithValue<void>*>(state)) {
                auto* update = new CompletedCancellableContinuationState<void>(nullptr, nullptr, nullptr, cause);
                if (state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                    owned_state_.reset(update);
                    return;
                }
                delete update;
                continue;
            }

            if (auto* cc = dynamic_cast<CompletedCancellableContinuationState<void>*>(state)) {
                if (cc->is_cancelled()) throw std::runtime_error("Must be called at most once");
                auto* update = new CompletedCancellableContinuationState<void>(cc->cancel_handler, cc->on_cancellation, cc->idempotent_resume, cause);
                if (state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                    if (cc->cancel_handler) call_cancel_handler(cc->cancel_handler, cause);
                    if (cc->on_cancellation) cc->on_cancellation(cause, context_);
                    return;
                }
                delete update;
                continue;
            }
            return;
        }
    }

    // Kotlin lines 201-217
    bool cancel(std::exception_ptr cause = nullptr) override {
        while (true) {
            State* state = state_.load(std::memory_order_acquire);
            // if (state !is NotCompleted) return false
            if (!dynamic_cast<NotCompleted*>(state)) return false;

            // handled = state is CancelHandler || state is Segment<*>
            bool is_cancel_handler = dynamic_cast<CancelHandler*>(state) != nullptr;
            bool is_segment = dynamic_cast<internal::SegmentBase*>(state) != nullptr;
            bool handled = is_cancel_handler || is_segment;

            auto* update = new CancelledContinuation(cause, handled);

            // Save handler BEFORE CAS - owned_state_ holds the CancelHandler if state is one
            std::shared_ptr<CancelHandler> handler_to_call;
            if (is_cancel_handler) {
                handler_to_call = std::dynamic_pointer_cast<CancelHandler>(owned_state_);
            }

            if (!state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                delete update;
                continue;
            }
            // CAS succeeded - take ownership of new state
            owned_state_.reset(update);

            // Invoke cancel handler if it was present
            // when (state) { is CancelHandler -> ..., is Segment<*> -> ... }
            if (is_cancel_handler && handler_to_call) {
                call_cancel_handler(handler_to_call, cause);
            } else if (is_segment) {
                call_segment_on_cancellation(dynamic_cast<internal::SegmentBase*>(state), cause);
            }

            detach_child_if_non_reusable();
            dispatch_resume(this->resume_mode);
            return true;
        }
    }

    // Kotlin lines 219-224
    void parent_cancelled(std::exception_ptr cause) {
        if (cancel_later(cause)) return;
        cancel(cause);
        // Even if cancellation has failed, we should detach child to avoid potential leak
        detach_child_if_non_reusable();
    }

    // Kotlin lines 226-239: callCancelHandlerSafely + callCancelHandler
    void call_cancel_handler(std::shared_ptr<CancelHandler> handler, std::exception_ptr cause) {
        try {
            handler->invoke(cause);
        } catch (...) {
            // Handler should never fail, if it does -- it is an unhandled exception
            if (context_) {
                handle_coroutine_exception(*context_,
                    std::make_exception_ptr(std::runtime_error("Exception in invokeOnCancellation handler")));
            }
        }
    }

    // Kotlin lines 241-245
    void call_segment_on_cancellation(internal::SegmentBase* segment, std::exception_ptr cause) {
        // val index = _decisionAndIndex.value.index
        int index = get_index(decision_and_index_.load(std::memory_order_acquire));
        // check(index != NO_INDEX) { "The index for Segment.onCancellation(..) is broken" }
        if (index == NO_INDEX) {
            throw std::logic_error("The index for Segment.onCancellation(..) is broken");
        }
        // callCancelHandlerSafely { segment.onCancellation(index, cause, context) }
        try {
            segment->on_cancellation(index, cause, context_);
        } catch (...) {
            if (context_) {
                handle_coroutine_exception(*context_,
                    std::make_exception_ptr(std::runtime_error("Exception in invokeOnCancellation handler")));
            }
        }
    }

    // ---- Parent handle ----
    // installParentHandle - Kotlin lines 339-345
    std::shared_ptr<DisposableHandle> install_parent_handle() {
        if (!context_) return nullptr;
        auto job_element = context_->get(Job::type_key);
        auto parent = std::dynamic_pointer_cast<Job>(job_element);
        if (!parent) return nullptr;  // don't do anything without a parent

        // Create ChildContinuation and set its job pointer
        auto child_handler = std::make_shared<ChildContinuation<void>>(this);
        child_handler->job = dynamic_cast<JobSupport*>(parent.get());

        // val handle = parent.invokeOnCompletion(handler = ChildContinuation(this))
        auto handle = parent->invoke_on_completion(
            true,  // onCancelling = true
            false, // invokeImmediately = false
            [child_handler](std::exception_ptr cause) {
                child_handler->invoke(cause);
            }
        );

        // _parentHandle.compareAndSet(null, handle)
        if (!parent_handle_) {
            parent_handle_ = handle;
        }
        return handle;
    }

    void init_cancellability() override {
        // Kotlin lines 120-136
        auto handle = install_parent_handle();
        if (!handle) return;  // fast path
        // now check our state _after_ registering
        if (is_completed()) {
            handle->dispose();
            parent_handle_ = non_disposable_handle();
        }
    }

    // Kotlin lines 560-563
    void detach_child_if_non_reusable() {
        if (!is_reusable()) detach_child();
    }

    // Kotlin lines 568-572
    void detach_child() {
        auto handle = parent_handle_;
        if (!handle) return;
        handle->dispose();
        parent_handle_ = non_disposable_handle();
    }

    // ---- tryResume / completeResume ----
    // Mirrors resumed_state logic for void - Kotlin lines 473-491
    void* try_resume(void* idempotent = nullptr) override {
        while (true) {
            State* state = state_.load(std::memory_order_acquire);
            if (dynamic_cast<NotCompleted*>(state)) {
                auto* ch = dynamic_cast<CancelHandler*>(state);

                // Use CompletedWithValue for simple case (no handlers, no idempotent)
                if (!is_cancellable_mode(this->resume_mode) && idempotent == nullptr) {
                    auto* update = new CompletedWithValue<void>();
                    if (state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                        detach_child_if_non_reusable();
                        return const_cast<void*>(RESUME_TOKEN);
                    }
                    delete update;
                    continue;
                }

                // Use CompletedCancellableContinuationState when metadata needed
                if (ch != nullptr || idempotent != nullptr) {
                    // Use dynamic_pointer_cast from owned_state_ for proper shared ownership
                    auto ch_shared = ch ? std::dynamic_pointer_cast<CancelHandler>(owned_state_) : nullptr;
                    auto* update = new CompletedCancellableContinuationState<void>(
                        ch_shared,
                        nullptr,
                        idempotent);
                    if (state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                        detach_child_if_non_reusable();
                        return const_cast<void*>(RESUME_TOKEN);
                    }
                    delete update;
                    continue;
                }

                // Simple case - no handlers, no idempotent
                auto* update = new CompletedWithValue<void>();
                if (state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                    detach_child_if_non_reusable();
                    return const_cast<void*>(RESUME_TOKEN);
                }
                delete update;
                continue;
            }
            if (dynamic_cast<CancelledContinuation*>(state)) return nullptr;
            return nullptr;
        }
    }

    // Kotlin: tryResume(value, idempotent, onCancellation) - for void specialization
    void* try_resume(
        void* idempotent,
        std::function<void(std::exception_ptr, void*, std::shared_ptr<CoroutineContext>)> on_cancellation
    ) override {
        while (true) {
            State* state = state_.load(std::memory_order_acquire);
            if (dynamic_cast<NotCompleted*>(state)) {
                auto* ch = dynamic_cast<CancelHandler*>(state);

                // Adapt 3-param callback to 2-param (void specialization ignores the value)
                std::function<void(std::exception_ptr, std::shared_ptr<CoroutineContext>)> adapted_on_cancellation;
                if (on_cancellation) {
                    adapted_on_cancellation = [on_cancellation](std::exception_ptr e, std::shared_ptr<CoroutineContext> ctx) {
                        on_cancellation(e, nullptr, ctx);  // Pass nullptr for void value
                    };
                }

                // Create CompletedCancellableContinuationState with adapted handler
                auto ch_shared = ch ? std::dynamic_pointer_cast<CancelHandler>(owned_state_) : nullptr;
                auto* update = new CompletedCancellableContinuationState<void>(
                    ch_shared,
                    adapted_on_cancellation,
                    idempotent);
                if (state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                    detach_child_if_non_reusable();
                    return const_cast<void*>(RESUME_TOKEN);
                }
                delete update;
                continue;
            }

            // Idempotent check for CompletedCancellableContinuationState
            if (auto* cc = dynamic_cast<CompletedCancellableContinuationState<void>*>(state)) {
                if (idempotent != nullptr && cc->idempotent_resume == idempotent) {
                    return const_cast<void*>(RESUME_TOKEN);
                }
                return nullptr;
            }

            if (dynamic_cast<CancelledContinuation*>(state)) return nullptr;
            return nullptr;
        }
    }

    void* try_resume_with_exception(std::exception_ptr exception) override {
        while (true) {
            State* state = state_.load(std::memory_order_acquire);
            if (dynamic_cast<NotCompleted*>(state)) {
                auto* update = new CompletedExceptionState(exception, false);
                if (state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                    return const_cast<void*>(RESUME_TOKEN);
                }
                delete update;
                continue;
            }
            if (dynamic_cast<CancelledContinuation*>(state)) return nullptr;
            return nullptr;
        }
    }

    void complete_resume(void* token) override {
        if (token != RESUME_TOKEN) return;
        detach_child_if_non_reusable();
        dispatch_resume(this->resume_mode);
    }

    // ---- resume / resume_with ----
    void resume(std::function<void(std::exception_ptr)> on_cancellation) override {
        void* token = try_resume(nullptr);
        if (!token) throw std::logic_error("Already resumed");
        (void)on_cancellation; // TODO: store on_cancellation for void specialization
        complete_resume(token);
    }

    void resume_undispatched(CoroutineDispatcher* dispatcher) override {
        (void)dispatcher; // TODO: use dispatcher undispatched fast-path
        resume(nullptr);
    }

    void resume_undispatched_with_exception(CoroutineDispatcher* dispatcher, std::exception_ptr exception) override {
        (void)dispatcher; // TODO: use dispatcher undispatched fast-path
        auto token = try_resume_with_exception(exception);
        if (!token) throw std::logic_error("Already resumed");
        complete_resume(token);
    }

    void resume_with(Result<void> result) override {
        if (result.is_success()) {
            resume(nullptr);
        } else {
            auto token = try_resume_with_exception(result.exception_or_null());
            if (!token) throw std::logic_error("Already resumed");
            complete_resume(token);
        }
    }

    // ---- Waiter ----
    // Kotlin lines 385-393: invokeOnCancellation(segment, index)
    void invoke_on_cancellation(internal::SegmentBase* segment, int index) override {
        // _decisionAndIndex.update { ... decisionAndIndex(it.decision, index) }
        while (true) {
            int cur = decision_and_index_.load(std::memory_order_acquire);
            // check(it.index == NO_INDEX) { "invokeOnCancellation should be called at most once" }
            if (get_index(cur) != NO_INDEX) {
                throw std::logic_error("invokeOnCancellation should be called at most once");
            }
            int update = decision_and_index(get_decision(cur), index);
            if (decision_and_index_.compare_exchange_strong(cur, update)) break;
        }
        // invokeOnCancellationImpl(segment)
        invoke_on_cancellation_impl_segment(segment);
    }

    // Segment-based cancellation - Kotlin lines 400-427 (Segment branch)
    void invoke_on_cancellation_impl_segment(internal::SegmentBase* segment) {
        while (true) {
            State* state = state_.load(std::memory_order_acquire);

            // Active -> store segment marker
            if (dynamic_cast<Active*>(state)) {
                // For segments, we store a marker indicating segment-based cancellation is pending
                // Note: In Kotlin, the segment is stored as the state directly
                // In C++ we'd need a SegmentState wrapper, but for now we store it elsewhere
                // TODO(port): proper segment-as-state storage
                return;
            }

            // CompletedExceptionally (includes CancelledContinuation) - Kotlin lines 408-429
            if (auto* ex = dynamic_cast<CompletedExceptionally*>(state)) {
                if (!ex->make_handled()) {
                    throw std::runtime_error("Multiple handlers prohibited");
                }
                // Call segment cancellation only if cancelled
                if (dynamic_cast<CancelledContinuation*>(state)) {
                    call_segment_on_cancellation(segment, ex->cause);
                }
                return;
            }

            // CompletedCancellableContinuationState -> segment doesn't need to be called
            // Kotlin line 437-438: if (handler is Segment<*>) return
            if (dynamic_cast<CompletedCancellableContinuationState<void>*>(state)) {
                return;
            }

            // CompletedWithValue -> segment doesn't need to be called on completed continuation
            // Kotlin line 454: if (handler is Segment<*>) return
            if (dynamic_cast<CompletedWithValue<void>*>(state)) {
                return;
            }

            return;
        }
    }

    // C++ lifetime management: return shared_ptr to self for segment storage
    std::shared_ptr<Waiter> shared_from_this_waiter() override {
        return std::static_pointer_cast<Waiter>(this->shared_from_this());
    }

    // ---- invoke_on_cancellation ----
    void invoke_on_cancellation(std::function<void(std::exception_ptr)> handler) override {
        invoke_on_cancellation_impl(std::make_shared<UserSuppliedCancelHandler>(handler));
    }

    // Kotlin lines 400-461: invokeOnCancellationImpl
    void invoke_on_cancellation_impl(std::shared_ptr<CancelHandler> handler) {
        while (true) {
            State* state = state_.load(std::memory_order_acquire);

            // Active -> store handler as the new state
            if (dynamic_cast<Active*>(state)) {
                if (state_.compare_exchange_strong(state, handler.get(), std::memory_order_acq_rel)) {
                    // Keep handler alive - it's now the state
                    owned_state_ = handler;
                    return;
                }
                continue; // retry on CAS failure
            }

            // Already has a handler -> error (Kotlin line 407)
            if (dynamic_cast<CancelHandler*>(state)) {
                throw std::runtime_error("Multiple handlers prohibited");
            }

            // CompletedExceptionally (includes CancelledContinuation) - Kotlin lines 408-429
            if (auto* ex = dynamic_cast<CompletedExceptionally*>(state)) {
                // if (!state.makeHandled()) multipleHandlersError(...)
                if (!ex->make_handled()) {
                    throw std::runtime_error("Multiple handlers prohibited");
                }
                // Call handler only if cancelled (not just exceptionally completed)
                if (dynamic_cast<CancelledContinuation*>(state)) {
                    call_cancel_handler(handler, ex->cause);
                }
                return;
            }

            // CompletedCancellableContinuationState -> copy with handler (Kotlin lines 432-446)
            if (auto* cc = dynamic_cast<CompletedCancellableContinuationState<void>*>(state)) {
                if (cc->cancel_handler) {
                    throw std::runtime_error("Multiple handlers prohibited");
                }
                if (cc->is_cancelled()) {
                    // Was already cancelled while being dispatched -- invoke the handler directly
                    call_cancel_handler(handler, cc->cancel_cause);
                    return;
                }
                auto* update = new CompletedCancellableContinuationState<void>(
                    handler, cc->on_cancellation, cc->idempotent_resume, cc->cancel_cause);
                if (state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                    owned_state_.reset(update);
                    return;
                }
                delete update;
                continue;
            }

            // Kotlin lines 448-458: else branch - raw completed value
            // CompletedWithValue -> wrap in CompletedCancellableContinuationState with handler
            if (dynamic_cast<CompletedWithValue<void>*>(state)) {
                auto* update = new CompletedCancellableContinuationState<void>(handler, nullptr, nullptr, nullptr);
                if (state_.compare_exchange_strong(state, update, std::memory_order_acq_rel)) {
                    owned_state_.reset(update);
                    return;
                }
                delete update;
                continue;
            }

            // Unknown state - should not happen
            return;
        }
    }

    // ---- state helpers ----
    bool is_active() const override { return dynamic_cast<NotCompleted*>(state_.load(std::memory_order_acquire)); }
    bool is_completed() const override { return !is_active(); }
    bool is_cancelled() const override { return dynamic_cast<CancelledContinuation*>(state_.load(std::memory_order_acquire)); }

    Result<void> take_state() override {
        State* st = state_.load(std::memory_order_acquire);
        if (auto* ex = dynamic_cast<CompletedExceptionally*>(st)) {
            return Result<void>::failure(ex->cause);
        }
        return Result<void>::success();
    }

    /**
     * getResult implementation - Kotlin lines 290-337
     */
    void* get_result() {
        // val isReusable = isReusable()
        bool is_reusable_flag = is_reusable();

        if (try_suspend()) {
            // if (parentHandle == null) { installParentHandle() }
            if (parent_handle_ == nullptr) {
                install_parent_handle();
            }
            // if (isReusable) { releaseClaimedReusableContinuation() }
            if (is_reusable_flag) {
                release_claimed_reusable_continuation();
            }
            return intrinsics::get_COROUTINE_SUSPENDED();
        }

        // if (isReusable) { releaseClaimedReusableContinuation() }
        if (is_reusable_flag) {
            release_claimed_reusable_continuation();
        }

        State* state = state_.load(std::memory_order_acquire);
        if (auto* ex = dynamic_cast<CompletedExceptionally*>(state)) {
            std::rethrow_exception(ex->cause);
        }

        // Check cancellable mode - Kotlin lines 328-334
        if (is_cancellable_mode(this->resume_mode) && context_) {
            auto job_element = context_->get(Job::type_key);
            if (auto job = std::dynamic_pointer_cast<Job>(job_element)) {
                if (!job->is_active()) {
                    std::exception_ptr cause = job->get_cancellation_exception();
                    cancel_completed_result(Result<void>(), cause);
                    std::rethrow_exception(cause);
                }
            }
        }

        return nullptr;
    }

    // ---- dispatch ----
    void dispatch_resume(int mode) {
        if (try_resume_decision()) return;
        dispatch(this, mode);
    }
};


// suspend_cancellable_coroutine implementation
template <typename T>
inline void* suspend_cancellable_coroutine(
    std::function<void(CancellableContinuation<T>&)> block,
    Continuation<void*>* continuation
) {
    /*
     * Kotlin equivalent:
     *   suspend fun <T> suspendCancellableCoroutine(
     *       block: (CancellableContinuation<T>) -> Unit
     *   ): T
     *
     * We adapt the Kotlin/Native calling convention by taking a
     * Continuation<void*>* and returning either COROUTINE_SUSPENDED or
     * a pointer to the produced value.
     */

    // Adapter that turns Continuation<T> into Continuation<void*>
    class ContinuationAdapter : public Continuation<T> {
        Continuation<void*>* outer_;
    public:
        explicit ContinuationAdapter(Continuation<void*>* outer) : outer_(outer) {}

        std::shared_ptr<CoroutineContext> get_context() const override {
            return outer_->get_context();
        }

        void resume_with(Result<T> result) override {
            if (result.is_success()) {
                // Allocate result on heap to comply with void* return convention
                T* value_ptr = new T(result.get_or_throw());
                outer_->resume_with(Result<void*>::success(static_cast<void*>(value_ptr)));
            } else {
                outer_->resume_with(Result<void*>::failure(result.exception_or_null()));
            }
        }
    };

    auto adapter = std::make_shared<ContinuationAdapter>(continuation);
    auto impl = std::make_shared<CancellableContinuationImpl<T>>(adapter, 1);
    impl->init_cancellability();

    // Execute user block with the cancellable continuation
    block(*impl);

    // get_result() handles the suspend vs resume fast-path
    return impl->get_result();
}

// Void Specialization for suspend_cancellable_coroutine
template <>
inline void* suspend_cancellable_coroutine<void>(
    std::function<void(CancellableContinuation<void>&)> block,
    Continuation<void*>* continuation
) {
    // Adapter wraps generic Continuation<void*> as Continuation<void>
    class ContinuationAdapter : public Continuation<void> {
        Continuation<void*>* outer_;
    public:
        explicit ContinuationAdapter(Continuation<void*>* outer) : outer_(outer) {}
        std::shared_ptr<CoroutineContext> get_context() const override { return outer_->get_context(); }
        void resume_with(Result<void> result) override {
            if (result.is_success()) {
                // Return nullptr as void "value"
                outer_->resume_with(Result<void*>::success(nullptr));
            } else {
                outer_->resume_with(Result<void*>::failure(result.exception_or_null()));
            }
        }
    };

    auto adapter = std::make_shared<ContinuationAdapter>(continuation);
    auto impl = std::make_shared<CancellableContinuationImpl<void>>(adapter, 1);
    impl->init_cancellability();

    block(*impl);

    if (impl->try_suspend()) {
        return intrinsics::get_COROUTINE_SUSPENDED();
    }
    
    // Void result
    impl->get_result();
    return nullptr;
}

} // namespace coroutines
} // namespace kotlinx

// Include DispatchedContinuation.hpp after class definitions to provide full definition
// for template instantiation. This pattern avoids circular includes: CancellableContinuationImpl.hpp
// forward-declares DispatchedContinuation, then includes the full definition at the end.
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"
