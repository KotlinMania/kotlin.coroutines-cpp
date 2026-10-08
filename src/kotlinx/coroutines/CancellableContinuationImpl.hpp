/**
 * Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt
 */
#pragma once
// port-lint: source CancellableContinuationImpl.kt

#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/Waiter.hpp"
#include "kotlinx/coroutines/DisposableHandle.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/CompletionHandler.hpp"
#include "kotlinx/coroutines/CoroutineExceptionHandler.hpp"
#include "kotlinx/coroutines/ContinuationState.hpp"
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include "kotlinx/coroutines/internal/CoroutineStackFrame.hpp"
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

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:23-23
inline internal::Symbol RESUME_TOKEN("RESUME_TOKEN");

// State, NotCompleted, Active are defined in ContinuationState.hpp

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:634-670
struct CancelHandler : public virtual NotCompleted {
    virtual void invoke(std::exception_ptr cause) = 0;
};

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:665-669
struct UserSuppliedCancelHandler : public CancelHandler {
    std::function<void(std::exception_ptr)> handler;
    
    explicit UserSuppliedCancelHandler(std::function<void(std::exception_ptr)> h) 
        : handler(std::move(h)) {}
        
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:667-667
    void invoke(std::exception_ptr cause) override { handler(cause); }
    std::string to_string() const override { return "CancelHandler.UserSupplied"; }
};

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:673-689
// NOTE(port): The private Kotlin CompletedContinuation name collides with the
// Kotlin stdlib CompletedContinuation already placed in this C++ namespace.
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

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:683-683
    bool is_cancelled() const { return cancel_cause != nullptr; }
    
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:685-688
    void invoke_handlers(CancellableContinuationImpl<T>& continuation, std::exception_ptr cause) {
        if (cancel_handler) continuation.call_cancel_handler(cancel_handler, cause);
        if (on_cancellation) continuation.call_on_cancellation(on_cancellation, cause, result);
    }

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

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:683-683
    bool is_cancelled() const { return cancel_cause != nullptr; }
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:685-688
    void invoke_handlers(CancellableContinuationImpl<void>& continuation, std::exception_ptr cause);
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

// Transliterated from: kotlinx-coroutines-core/common/src/CompletionState.kt:45-54
struct CancelledContinuation : public CompletedExceptionally, public State {
    CancelledContinuation(const void* continuation, std::exception_ptr cause, bool handled);

    // Transliterated from: kotlinx-coroutines-core/common/src/CompletionState.kt:53-54
    bool make_resumed() {
        bool expected = false;
        return resumed_.compare_exchange_strong(expected, true);
    }

    std::string to_string() const override { return "CancelledContinuation"; }
private:
    std::atomic<bool> resumed_{false};
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
                                    public internal::CoroutineStackFrame,
                                    public Waiter,
                                    public std::enable_shared_from_this<CancellableContinuationImpl<T>> {
private:
    static_assert(!std::is_void_v<T>, "Use void specialization");
    std::shared_ptr<Continuation<T>> delegate;

    // _decisionAndIndex - Kotlin line 69
    std::atomic<int> decision_and_index_;

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:72-80
    // NOTE(port): Atomic shared ownership replaces Kotlin's GC-owned AtomicRef<Any?>.
    // A load retains the same state object through the compare-and-set and callbacks.
    std::shared_ptr<State> state_;

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:91-103
    // NOTE(port): Atomic shared-pointer operations retain the Kotlin GC-owned handle.
    std::shared_ptr<DisposableHandle> parent_handle_;

    // Context cache - Kotlin line 38: context = delegate.context
    std::shared_ptr<CoroutineContext> context_;

public:
    CancellableContinuationImpl(std::shared_ptr<Continuation<T>> delegate_, int resume_mode_)
        : DispatchedTask<T>(resume_mode_), delegate(delegate_) {
        assert(resume_mode_ != MODE_UNINITIALIZED);
        context_ = delegate->get_context();
        std::atomic_store(&state_, std::shared_ptr<State>(&Active::instance, [](State*) {}));
        decision_and_index_.store(decision_and_index(UNDECIDED, NO_INDEX), std::memory_order_relaxed);
    }

    ~CancellableContinuationImpl() override = default;
    using CancellableContinuation<T>::resume;

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:365-383
    void resume(T value,
        std::function<void(std::exception_ptr, T, std::shared_ptr<CoroutineContext>)> on_cancellation) override {
        resume_impl(std::move(value), this->resume_mode, std::move(on_cancellation));
    }


    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:160-161
    internal::CoroutineStackFrame* get_caller_frame() const override {
        return dynamic_cast<internal::CoroutineStackFrame*>(delegate.get());
    }
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:163-163
    internal::StackTraceElement* get_stack_trace_element() const override { return nullptr; }


    std::shared_ptr<SchedulerTask> shared_task() override { return this->shared_from_this(); }
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:38-38
    std::shared_ptr<CoroutineContext> get_context() const override { return context_; }
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:30-31
    std::shared_ptr<Continuation<T>> get_delegate() override { return delegate; }

    /**
     * Kotlin line 138:
     * private fun isReusable(): Boolean = resumeMode.isReusableMode && (delegate as DispatchedContinuation<*>).isReusable()
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:138-138
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
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:145-158
    bool reset_state_reusable() {
        // assert { resumeMode == MODE_CANCELLABLE_REUSABLE }
        assert(this->resume_mode == MODE_CANCELLABLE_REUSABLE);
        // assert { parentHandle !== NonDisposableHandle }
        assert(std::atomic_load(&parent_handle_) != non_disposable_handle());

        // val state = _state.value
        auto state_owner = std::atomic_load(&state_);
        State* state = state_owner.get();
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
        std::atomic_store(&state_, std::shared_ptr<State>(&Active::instance, [](State*) {}));

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
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:351-356
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
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:194-199
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

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:107-107
    bool is_active() const override {
        return dynamic_cast<NotCompleted*>(std::atomic_load(&state_).get()) != nullptr;
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:109-109
    bool is_completed() const override {
        return !is_active();
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:111-111
    bool is_cancelled() const override {
        return dynamic_cast<CancelledContinuation*>(std::atomic_load(&state_).get()) != nullptr;
    }
    
    // initCancellability implementation
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:120-136
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
            std::atomic_store(&parent_handle_, non_disposable_handle());
        }
    }
    
    // State machine helpers
    // private inline fun decisionAndIndex(decision: Int, index: Int)
    
    // takeState - Kotlin line 165: just returns state
    // Combined with getSuccessfulResult (lines 605-609) and getExceptionalResult (lines 613-614)
    // C++ returns Result<T> directly for simplicity
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:165-165
    Result<T> take_state() override {
        auto state_owner = std::atomic_load(&state_);
        State* s = state_owner.get();

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
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:169-189
    void cancel_completed_result(Result<T> taken_state, std::exception_ptr cause) override {
        while (true) {
            auto state_owner = std::atomic_load(&state_);
            State* state = state_owner.get();
            if (dynamic_cast<NotCompleted*>(state)) {
                throw std::runtime_error("Not completed");
            }
            if (dynamic_cast<CompletedExceptionally*>(state)) return; // already exceptional

            // Handle CompletedWithValue - promote to CompletedCancellableContinuationState with cancelCause
            // Kotlin lines 181-187: else branch for raw values
            if (auto* cwv = dynamic_cast<CompletedWithValue<T>*>(state)) {
                std::shared_ptr<State> update(new CompletedCancellableContinuationState<T>(
                    cwv->result, nullptr, nullptr, nullptr, cause));
                if (std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
                    // No handlers to invoke for CompletedWithValue
                    return;
                }

                continue;
            }

            if (auto* cc = dynamic_cast<CompletedCancellableContinuationState<T>*>(state)) {
                if (cc->is_cancelled()) throw std::runtime_error("Must be called at most once");
                std::shared_ptr<State> update(new CompletedCancellableContinuationState<T>(
                    cc->result, cc->cancel_handler, cc->on_cancellation, cc->idempotent_resume, cause));
                if (std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
                    cc->invoke_handlers(*this, cause);
                    return;
                }

                continue;
            }
            return;
        }
    }
    
    // cancel - Kotlin lines 201-217
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:201-217
    bool cancel(std::exception_ptr cause = nullptr) override {
        // NOTE(port): Retain the GC-owned receiver while detaching parent handlers.
        auto self_guard = this->weak_from_this().lock();
        while (true) {
            auto state_owner = std::atomic_load(&state_);
            State* state = state_owner.get();
            // line 203: if (state !is NotCompleted) return false
            if (!dynamic_cast<NotCompleted*>(state)) return false;

            // line 205: val update = CancelledContinuation(this, cause, handled = state is CancelHandler || state is Segment<*>)
            bool is_cancel_handler = dynamic_cast<CancelHandler*>(state) != nullptr;
            bool is_segment = dynamic_cast<internal::SegmentBase*>(state) != nullptr;
            bool handled = is_cancel_handler || is_segment;
            std::shared_ptr<State> update(new CancelledContinuation(this, cause, handled));

            // Retain the same source state across compare-and-set and handler invocation.
            std::shared_ptr<CancelHandler> handler_to_call;
            if (is_cancel_handler) {
                handler_to_call = std::dynamic_pointer_cast<CancelHandler>(state_owner);
            }

            // line 206: if (!_state.compareAndSet(state, update)) return@loop
            if (!std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
                continue;
            }

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
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:219-224
    void parent_cancelled(std::exception_ptr cause) {
        if (cancel_later(cause)) return;
        cancel(cause);
        // Even if cancellation has failed, we should detach child to avoid potential leak
        detach_child_if_non_reusable();
    }

    // callCancelHandlerSafely
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:226-239
    void call_cancel_handler(std::shared_ptr<CancelHandler> handler, std::exception_ptr cause) {
        try {
            handler->invoke(cause);
        } catch (...) {
            if (context_) {
                handle_coroutine_exception(*context_,
                    std::make_exception_ptr(CompletionHandlerException("Exception in invokeOnCancellation handler", std::current_exception())));
            }
        }
    }

    template<typename U=T, typename std::enable_if<!std::is_void_v<U>, int>::type = 0>
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:247-264
    void call_on_cancellation(
        std::function<void(std::exception_ptr, U, std::shared_ptr<CoroutineContext>)> on_cancellation,
        std::exception_ptr cause,
        U value) {
        try {
            on_cancellation(cause, value, context_);
        } catch (...) {
            if (context_) {
                handle_coroutine_exception(*context_,
                    std::make_exception_ptr(CompletionHandlerException("Exception in resume onCancellation handler", std::current_exception())));
            }
        }
    }
    
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:266-267
    virtual std::exception_ptr get_continuation_cancellation_cause(Job& parent) {
        return parent.get_cancellation_exception();
    }
    
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:269-277
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
    
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:279-287,575-583
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
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:290-337
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
            if (std::atomic_load(&parent_handle_) == nullptr) {
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
        auto state_owner = std::atomic_load(&state_);
        State* state = state_owner.get();
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
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:605-609
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
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:339-345
    std::shared_ptr<DisposableHandle> install_parent_handle() {
        if (!context_) return nullptr;
        auto job_element = context_->get(Job::type_key);
        auto parent = std::dynamic_pointer_cast<Job>(job_element);
        if (!parent) return nullptr;  // don't do anything without a parent

        // Create ChildContinuation and set its job pointer
        auto child_handler = std::make_shared<ChildContinuation<T>>(this);
        child_handler->job = dynamic_cast<JobSupport*>(parent.get());

        auto self = this->shared_from_this();
        auto handle = parent->invoke_on_completion(
            true,  // onCancelling = true
            true,  // invokeImmediately = true
            [child_handler, self](std::exception_ptr cause) {
                child_handler->invoke(cause);
            }
        );

        // _parentHandle.compareAndSet(null, handle)
        std::shared_ptr<DisposableHandle> expected;
        std::atomic_compare_exchange_strong(&parent_handle_, &expected, handle);
        return handle;
    }
    
    // Kotlin lines 560-563
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:560-563
    void detach_child_if_non_reusable() {
        // If instance is reusable, do not detach on every reuse
        if (!is_reusable()) detach_child();
    }

    // Kotlin lines 568-572
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:568-572
    void detach_child() {
        auto handle = std::atomic_load(&parent_handle_);
        if (!handle) return;
        handle->dispose();
        std::atomic_store(&parent_handle_, non_disposable_handle());
    }
    
    // tryResume + completeResume (Kotlin parity)
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:279-287,575-583
    void* try_resume(T value, void* idempotent = nullptr) override {
        return try_resume(value, idempotent, nullptr);
    }

    // tryResumeImpl - Kotlin lines 529-553
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:279-287,575-583
    void* try_resume(
        T value,
        void* idempotent,
        std::function<void(std::exception_ptr, T, std::shared_ptr<CoroutineContext>)> on_cancellation
    ) override {
        while (true) {
            auto state_owner = std::atomic_load(&state_);
            State* state = state_owner.get();

            if (auto* nc = dynamic_cast<NotCompleted*>(state)) {
                std::shared_ptr<State> update(resumed_state(nc, state_owner, value, this->resume_mode, on_cancellation, idempotent));
                if (std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
                    detach_child_if_non_reusable();
                    return &RESUME_TOKEN;
                }

                continue;
            }

            // Kotlin lines 542-549: idempotent resume check for CompletedCancellableContinuationState
            if (auto* cc = dynamic_cast<CompletedCancellableContinuationState<T>*>(state)) {
                if (idempotent != nullptr && cc->idempotent_resume == idempotent) {
                    // assert state.result == value
                    return &RESUME_TOKEN;
                }
                return nullptr; // different token or non-idempotent
            }

            // CancelledContinuation or other completed state
            return nullptr;
        }
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:585-586,529-553
    void* try_resume_with_exception(std::exception_ptr exception) override {
        auto self_guard = this->weak_from_this().lock();
        while (true) {
            auto state_owner = std::atomic_load(&state_);
            State* state = state_owner.get();
            if (dynamic_cast<NotCompleted*>(state)) {
                std::shared_ptr<State> update(new CompletedExceptionState(exception, false));
                if (std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
                    detach_child_if_non_reusable();
                    return &RESUME_TOKEN;
                }

                continue;
            }
            if (dynamic_cast<CancelledContinuation*>(state)) return nullptr;
            return nullptr;
        }
    }

    // completeResume - Kotlin lines 589-592
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:589-592
    void complete_resume(void* token) override {
        auto self_guard = this->weak_from_this().lock();
        assert(token == &RESUME_TOKEN);
        // Note: detachChildIfNonReusable is called in tryResumeImpl, not here
        dispatch_resume(this->resume_mode);
    }

    // resumedState - determines what to store as the new state
    // Kotlin lines 473-491
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:473-491
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
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:493-523
    void resume_impl(T proposed_update, int resume_mode,
                     std::function<void(std::exception_ptr, T, std::shared_ptr<CoroutineContext>)> on_cancellation = nullptr) {
        // NOTE(port): A slot may resume through a raw pointer; detach can drop its last owner.
        auto self_guard = this->weak_from_this().lock();
        while (true) {
            auto state_owner = std::atomic_load(&state_);
            State* state = state_owner.get();

            if (auto* nc = dynamic_cast<NotCompleted*>(state)) {
                std::shared_ptr<State> update(resumed_state(nc, state_owner, proposed_update, resume_mode, on_cancellation, nullptr));
                if (!std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
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
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:385-398
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
        // Segment is stored directly, matching Kotlin's NotCompleted state union.
        invoke_on_cancellation_impl_segment(segment);
    }

    // C++ lifetime management: return shared_ptr to self for segment storage
    std::shared_ptr<Waiter> shared_from_this_waiter() override {
        return std::static_pointer_cast<Waiter>(this->shared_from_this());
    }

    // Segment-based cancellation - Kotlin lines 400-427 (Segment branch)
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:399-458
    void invoke_on_cancellation_impl_segment(internal::SegmentBase* segment) {
        while (true) {
            auto state_owner = std::atomic_load(&state_);
            State* state = state_owner.get();

            // Active -> store segment marker (we use the segment pointer as state indicator)
            if (dynamic_cast<Active*>(state)) {
                // SegmentBase implements NotCompleted, matching Kotlin's state union.
                if (std::atomic_compare_exchange_strong(&state_, &state_owner,
                        std::shared_ptr<State>(segment, [](State*) {}))) {
                    return;
                }
                continue;
            }

            if (dynamic_cast<CancelHandler*>(state) || dynamic_cast<internal::SegmentBase*>(state)) {
                throw std::runtime_error("Multiple handlers prohibited");
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

            // A normally completed continuation does not need a segment handler.
            return;
        }
    }

    // callSegmentOnCancellation - Kotlin lines 241-245
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:241-245
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
                    std::make_exception_ptr(CompletionHandlerException("Exception in invokeOnCancellation handler", std::current_exception())));
            }
        }
    }
    
    // CancellableContinuation overrides
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:362-383
    void resume(T value, std::function<void(std::exception_ptr)> on_cancellation) override {
        // Adapter for simple handler
        resume_impl(value, this->resume_mode, 
             on_cancellation ? [on_cancellation](std::exception_ptr e, T, std::shared_ptr<CoroutineContext>) { on_cancellation(e); } 
                             : (std::function<void(std::exception_ptr, T, std::shared_ptr<CoroutineContext>)>)nullptr);
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:385-398
    void invoke_on_cancellation(std::function<void(std::exception_ptr)> handler) override {
         invoke_on_cancellation_impl(std::make_shared<UserSuppliedCancelHandler>(handler));
    }
    
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:398-398
    void invoke_on_cancellation_internal(std::shared_ptr<CancelHandler> handler) {
        invoke_on_cancellation_impl(std::move(handler));
    }

    // invokeOnCancellationImpl - Kotlin lines 400-461
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:400-461
    void invoke_on_cancellation_impl(std::shared_ptr<CancelHandler> handler) {
        while (true) {
            auto state_owner = std::atomic_load(&state_);
            State* state = state_owner.get();

            // Active -> store handler as the new state
            if (dynamic_cast<Active*>(state)) {
                if (std::atomic_compare_exchange_strong(&state_, &state_owner, std::static_pointer_cast<State>(handler))) {
                    return;
                }
                continue; // retry on CAS failure
            }

            // Already has a handler -> error
            if (dynamic_cast<CancelHandler*>(state) || dynamic_cast<internal::SegmentBase*>(state)) {
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
                std::shared_ptr<State> update(new CompletedCancellableContinuationState<T>(
                    cc->result, handler, cc->on_cancellation, cc->idempotent_resume, cc->cancel_cause));
                if (std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
                    return;
                }

                continue;
            }

            // Kotlin lines 448-458: else branch - raw completed value
            // CompletedWithValue -> wrap in CompletedCancellableContinuationState with handler
            if (auto* cwv = dynamic_cast<CompletedWithValue<T>*>(state)) {
                std::shared_ptr<State> update(new CompletedCancellableContinuationState<T>(
                    cwv->result, handler, nullptr, nullptr, nullptr));
                if (std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
                    return;
                }

                continue;
            }

            // Unknown state - should not happen
            return;
        }
    }
    
    // Dispatch
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:467-470
    void dispatch_resume(int mode) {
        if (try_resume()) return; // completed before getResult invocation
        dispatch(this, mode);
    }
    
    // resumeWith
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:358-360
    void resume_with(Result<T> result) override {
        if (result.is_success()) 
             resume_impl(result.get_or_throw(), this->resume_mode);
        else 
             resume_impl_exception(result.exception_or_null(), this->resume_mode);
    }
    
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:493-523
    void resume_impl_exception(std::exception_ptr exception, int mode) {
        auto self_guard = this->weak_from_this().lock();
        while (true) {
            auto state_owner = std::atomic_load(&state_);
            State* state = state_owner.get();
            if (dynamic_cast<NotCompleted*>(state)) {
                std::shared_ptr<State> update(new CompletedExceptionState(exception, false));
                if (!std::atomic_compare_exchange_strong(&state_, &state_owner, update)) {
                    continue;
                }

                detach_child_if_non_reusable();
                dispatch_resume(mode);
                return;
            }
            if (auto* cancelled = dynamic_cast<CancelledContinuation*>(state)) {
                if (cancelled->make_resumed()) return;
            }
            throw std::logic_error("Already resumed");
        }
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:594-597
    void resume_undispatched(CoroutineDispatcher* dispatcher, T value) override {
        auto dc = std::dynamic_pointer_cast<internal::DispatchedContinuation<T>>(delegate);
        resume_impl(value, dc && dc->dispatcher.get() == dispatcher ? MODE_UNDISPATCHED : this->resume_mode);
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:599-602
    void resume_undispatched_with_exception(CoroutineDispatcher* dispatcher, std::exception_ptr exception) override {
        auto dc = std::dynamic_pointer_cast<internal::DispatchedContinuation<T>>(delegate);
        resume_impl_exception(exception, dc && dc->dispatcher.get() == dispatcher ? MODE_UNDISPATCHED : this->resume_mode);
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
// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:30-622
template <>
class CancellableContinuationImpl<void> : public DispatchedTask<void>,
                                           public CancellableContinuation<void>,
                                           public internal::CoroutineStackFrame,
                                           public Waiter,
                                           public std::enable_shared_from_this<CancellableContinuationImpl<void>> {
private:
    friend struct CompletedCancellableContinuationState<void>;
    std::shared_ptr<Continuation<void>> delegate;
    std::atomic<int> decision_and_index_;
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:72-80
    // NOTE(port): State publication and GC-equivalent reachability are one atomic operation.
    std::shared_ptr<State> state_;
    std::shared_ptr<DisposableHandle> parent_handle_;
    std::shared_ptr<CoroutineContext> context_;

public:
    CancellableContinuationImpl(std::shared_ptr<Continuation<void>> delegate_, int resume_mode_)
        : DispatchedTask<void>(resume_mode_), delegate(delegate_) {
        assert(resume_mode_ != MODE_UNINITIALIZED);
        context_ = delegate->get_context();
        std::atomic_store(&state_, std::shared_ptr<State>(&Active::instance, [](State*) {}));
        decision_and_index_.store(decision_and_index(UNDECIDED, NO_INDEX), std::memory_order_relaxed);
    }

    std::shared_ptr<SchedulerTask> shared_task() override { return this->shared_from_this(); }
    std::shared_ptr<CoroutineContext> get_context() const override { return context_; }
    std::shared_ptr<Continuation<void>> get_delegate() override { return delegate; }

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:160-161
    internal::CoroutineStackFrame* get_caller_frame() const override;
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:163-163
    internal::StackTraceElement* get_stack_trace_element() const override;

    using CancellableContinuation<void>::resume;
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:365-383
    void resume(Unit value,
        std::function<void(std::exception_ptr, Unit, std::shared_ptr<CoroutineContext>)> on_cancellation) override;

    // getContinuationCancellationCause - Kotlin lines 266-267
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:266-267
    virtual std::exception_ptr get_continuation_cancellation_cause(Job& parent);

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
        assert(std::atomic_load(&parent_handle_) != non_disposable_handle());

        auto state_owner = std::atomic_load(&state_);

        State* state = state_owner.get();
        assert(dynamic_cast<NotCompleted*>(state) == nullptr);

        if (auto* cc = dynamic_cast<CompletedCancellableContinuationState<void>*>(state)) {
            if (cc->idempotent_resume != nullptr) {
                detach_child();
                return false;
            }
        }

        decision_and_index_.store(decision_and_index(UNDECIDED, NO_INDEX), std::memory_order_release);
        std::atomic_store(&state_, std::shared_ptr<State>(&Active::instance, [](State*) {}));

        return true;
    }

    // ---- Decision machine ----
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:269-277
    bool try_suspend();

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:279-287
    bool try_resume_decision();

    // ---- Cancellation ----
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:169-189
    void cancel_completed_result(Result<void> taken_state, std::exception_ptr cause) override;

    // Kotlin lines 201-217
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:201-217
    bool cancel(std::exception_ptr cause = nullptr) override;

    // Kotlin lines 219-224
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:219-224
    void parent_cancelled(std::exception_ptr cause);

    // Kotlin lines 226-239: callCancelHandlerSafely + callCancelHandler
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:226-239
    void call_cancel_handler(std::shared_ptr<CancelHandler> handler, std::exception_ptr cause);

    // Kotlin lines 241-245
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:241-245
    void call_segment_on_cancellation(internal::SegmentBase* segment, std::exception_ptr cause);

    // ---- Parent handle ----
    // installParentHandle - Kotlin lines 339-345
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:339-345
    std::shared_ptr<DisposableHandle> install_parent_handle();

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:120-136
    void init_cancellability() override;

    // Kotlin lines 560-563
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:560-563
    void detach_child_if_non_reusable();

    // Kotlin lines 568-572
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:568-572
    void detach_child();

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:575-576
    void* try_resume(void* idempotent = nullptr) override;
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:578-583
    void* try_resume(void* idempotent,
        std::function<void(std::exception_ptr, void*, std::shared_ptr<CoroutineContext>)> on_cancellation) override;
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:585-586
    void* try_resume_with_exception(std::exception_ptr exception) override;
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:589-592
    void complete_resume(void* token) override;
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:362-370
    void resume(std::function<void(std::exception_ptr)> on_cancellation) override;
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:594-597
    void resume_undispatched(CoroutineDispatcher* dispatcher) override;
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:599-602
    void resume_undispatched_with_exception(CoroutineDispatcher* dispatcher, std::exception_ptr exception) override;
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:358-360
    void resume_with(Result<void> result) override;

private:
    // NOTE(port): Kotlin Unit has no payload in the existing C++ void specialization.
    using OnCancellation = std::function<void(std::exception_ptr, std::shared_ptr<CoroutineContext>)>;
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:473-491
    State* resumed_state(NotCompleted* state, const std::shared_ptr<State>& state_owner, int mode, OnCancellation on_cancellation, void* idempotent);
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:493-523
    void resume_impl(int mode, OnCancellation on_cancellation = nullptr);
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:493-523
    void resume_impl_exception(std::exception_ptr exception, int mode);
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:529-553
    void* try_resume_impl(void* idempotent, OnCancellation on_cancellation);
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:247-264
    void call_on_cancellation(const OnCancellation& on_cancellation, std::exception_ptr cause);
public:

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
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:399-458
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:400-461
    void invoke_on_cancellation_impl_segment(internal::SegmentBase* segment);

    // C++ lifetime management: return shared_ptr to self for segment storage
    std::shared_ptr<Waiter> shared_from_this_waiter() override {
        return std::static_pointer_cast<Waiter>(this->shared_from_this());
    }

    // ---- invoke_on_cancellation ----
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:385-398
    void invoke_on_cancellation(std::function<void(std::exception_ptr)> handler) override;

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:398-398
    void invoke_on_cancellation_internal(std::shared_ptr<CancelHandler> handler);

    // Kotlin lines 400-461: invokeOnCancellationImpl
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:400-461
    void invoke_on_cancellation_impl(std::shared_ptr<CancelHandler> handler);

    // ---- state helpers ----
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:107-107
    bool is_active() const override;
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:109-109
    bool is_completed() const override;
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:111-111
    bool is_cancelled() const override;

    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:165-165
    Result<void> take_state() override;

    /**
     * getResult implementation - Kotlin lines 290-337
     */
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:290-337
    void* get_result();

    // ---- dispatch ----
    // Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuationImpl.kt:467-470
    void dispatch_resume(int mode);
};

// suspend_cancellable_coroutine implementation
// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuation.kt:423-436
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
        std::shared_ptr<Continuation<void*>> outer_shared_;
        Continuation<void*>* outer_raw_;
        Continuation<void*>* get_outer() const {
            return outer_shared_ ? outer_shared_.get() : outer_raw_;
        }
    public:
        explicit ContinuationAdapter(Continuation<void*>* outer) : outer_shared_(nullptr), outer_raw_(outer) {}
        explicit ContinuationAdapter(std::shared_ptr<Continuation<void*>> outer) : outer_shared_(outer), outer_raw_(outer.get()) {}

        std::shared_ptr<CoroutineContext> get_context() const override {
            return get_outer() ? get_outer()->get_context() : nullptr;
        }

        void resume_with(Result<T> result) override {
            auto* outer = get_outer();
            if (!outer) return;
            if (result.is_success()) {
                // Allocate result on heap to comply with void* return convention
                T* value_ptr = new T(result.get_or_throw());
                outer->resume_with(Result<void*>::success(static_cast<void*>(value_ptr)));
            } else {
                outer->resume_with(Result<void*>::failure(result.exception_or_null()));
            }
        }
    };

    auto intercepted = internal::intercepted_delegate(internal::retain_continuation(continuation));
    std::shared_ptr<Continuation<T>> adapter = std::make_shared<ContinuationAdapter>(intercepted.continuation);
    if (intercepted.dispatcher) adapter = intercepted.dispatcher->template intercept_continuation<T>(adapter);
    auto impl = std::make_shared<CancellableContinuationImpl<T>>(adapter, MODE_CANCELLABLE);
    impl->init_cancellability();

    // Execute user block with the cancellable continuation
    block(*impl);

    // get_result() handles the suspend vs resume fast-path
    return impl->get_result();
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuation.kt:423-436
template <typename T>
inline void* suspend_cancellable_coroutine(
    std::function<void(CancellableContinuation<T>&)> block,
    std::shared_ptr<Continuation<void*>> continuation
) {
    class ContinuationAdapter : public Continuation<T> {
        std::shared_ptr<Continuation<void*>> outer_;
    public:
        explicit ContinuationAdapter(std::shared_ptr<Continuation<void*>> outer) : outer_(outer) {}

        std::shared_ptr<CoroutineContext> get_context() const override {
            return outer_ ? outer_->get_context() : nullptr;
        }

        void resume_with(Result<T> result) override {
            if (!outer_) return;
            if (result.is_success()) {
                T* value_ptr = new T(result.get_or_throw());
                outer_->resume_with(Result<void*>::success(static_cast<void*>(value_ptr)));
            } else {
                outer_->resume_with(Result<void*>::failure(result.exception_or_null()));
            }
        }
    };

    auto intercepted = internal::intercepted_delegate(continuation);
    std::shared_ptr<Continuation<T>> adapter = std::make_shared<ContinuationAdapter>(intercepted.continuation);
    if (intercepted.dispatcher) adapter = intercepted.dispatcher->template intercept_continuation<T>(adapter);
    auto impl = std::make_shared<CancellableContinuationImpl<T>>(adapter, MODE_CANCELLABLE);
    impl->init_cancellability();

    block(*impl);

    return impl->get_result();
}

// Void Specialization for suspend_cancellable_coroutine
// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuation.kt:423-436
template <>
inline void* suspend_cancellable_coroutine<void>(
    std::function<void(CancellableContinuation<void>&)> block,
    Continuation<void*>* continuation
) {
    // Adapter wraps generic Continuation<void*> as Continuation<void>
    class ContinuationAdapter : public Continuation<void> {
        std::shared_ptr<Continuation<void*>> outer_shared_;
        Continuation<void*>* outer_raw_;
        Continuation<void*>* get_outer() const {
            return outer_shared_ ? outer_shared_.get() : outer_raw_;
        }
    public:
        explicit ContinuationAdapter(Continuation<void*>* outer) : outer_shared_(nullptr), outer_raw_(outer) {}
        explicit ContinuationAdapter(std::shared_ptr<Continuation<void*>> outer) : outer_shared_(outer), outer_raw_(outer.get()) {}
        std::shared_ptr<CoroutineContext> get_context() const override { return get_outer() ? get_outer()->get_context() : nullptr; }
        void resume_with(Result<void> result) override {
            auto* outer = get_outer();
            if (!outer) return;
            if (result.is_success()) {
                // Return nullptr as void "value"
                outer->resume_with(Result<void*>::success(nullptr));
            } else {
                outer->resume_with(Result<void*>::failure(result.exception_or_null()));
            }
        }
    };

    auto intercepted = internal::intercepted_delegate(internal::retain_continuation(continuation));
    std::shared_ptr<Continuation<void>> adapter = std::make_shared<ContinuationAdapter>(intercepted.continuation);
    if (intercepted.dispatcher) adapter = intercepted.dispatcher->template intercept_continuation<void>(adapter);
    auto impl = std::make_shared<CancellableContinuationImpl<void>>(adapter, MODE_CANCELLABLE);
    impl->init_cancellability();

    block(*impl);

    return impl->get_result();
}

// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuation.kt:423-436
inline void* suspend_cancellable_coroutine_void(
    std::function<void(CancellableContinuation<void>&)> block,
    std::shared_ptr<Continuation<void*>> continuation
) {
    class ContinuationAdapter : public Continuation<void> {
        std::shared_ptr<Continuation<void*>> outer_shared_;
    public:
        explicit ContinuationAdapter(std::shared_ptr<Continuation<void*>> outer) : outer_shared_(outer) {}
        std::shared_ptr<CoroutineContext> get_context() const override { return outer_shared_ ? outer_shared_->get_context() : nullptr; }
        void resume_with(Result<void> result) override {
            if (!outer_shared_) return;
            if (result.is_success()) {
                outer_shared_->resume_with(Result<void*>::success(nullptr));
            } else {
                outer_shared_->resume_with(Result<void*>::failure(result.exception_or_null()));
            }
        }
    };

    auto intercepted = internal::intercepted_delegate(continuation);
    std::shared_ptr<Continuation<void>> adapter = std::make_shared<ContinuationAdapter>(intercepted.continuation);
    if (intercepted.dispatcher) adapter = intercepted.dispatcher->template intercept_continuation<void>(adapter);
    auto impl = std::make_shared<CancellableContinuationImpl<void>>(adapter, MODE_CANCELLABLE);
    impl->init_cancellability();

    block(*impl);

    return impl->get_result();
}

template <>
inline void* suspend_cancellable_coroutine<void>(
    std::function<void(CancellableContinuation<void>&)> block,
    std::shared_ptr<Continuation<void*>> continuation
) {
    return suspend_cancellable_coroutine_void(block, continuation);
}

} // namespace coroutines
} // namespace kotlinx

// Include DispatchedContinuation.hpp after class definitions to provide full definition
// for template instantiation. This pattern avoids circular includes: CancellableContinuationImpl.hpp
// forward-declares DispatchedContinuation, then includes the full definition at the end.
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"
