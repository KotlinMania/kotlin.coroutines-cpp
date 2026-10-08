#pragma once
// port-lint: source internal/DispatchedContinuation.kt
/**
 * @file DispatchedContinuation.hpp
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedContinuation.kt
 */

#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/common/CoroutineContextUtils.hpp"
#include "kotlinx/coroutines/internal/CoroutineStackFrame.hpp"
#include "kotlinx/coroutines/internal/DispatchedTask.hpp"
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include "kotlinx/coroutines/internal/ThreadContext.hpp"
#include "kotlinx/coroutines/EventLoop.hpp"

#include <atomic>
#include <cassert>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <typeindex>
#include <utility>

namespace kotlinx {
namespace coroutines {

/**
 * Type-erased base for DispatchedContinuation to support release_intercepted_continuation.
 */
class DispatchedContinuationBase : public ContinuationBase {
public:
    virtual void release() = 0;
};

namespace internal {

// Private Symbol("UNDEFINED")
static const Symbol UNDEFINED("UNDEFINED");

// Kotlin: @JvmField internal val REUSABLE_CLAIMED = Symbol("REUSABLE_CLAIMED")
static const Symbol REUSABLE_CLAIMED("REUSABLE_CLAIMED");

// Forward declarations for safe dispatcher helpers (defined below).
inline void safe_dispatch(
    const CoroutineDispatcher& dispatcher,
    const CoroutineContext& context,
    std::shared_ptr<Runnable> runnable);

inline bool safe_is_dispatch_needed(const CoroutineDispatcher& dispatcher, const CoroutineContext& context);

/**
 * Internal dispatched continuation wrapper.
 */
template<typename T>
class DispatchedContinuation :
    public DispatchedTask<T>,
    public CoroutineStackFrame,
    public Continuation<T>,
    public DispatchedContinuationBase,
    public std::enable_shared_from_this<DispatchedContinuation<T>> {
public:
    std::shared_ptr<CoroutineDispatcher> dispatcher;
    std::shared_ptr<Continuation<T>> continuation;

    // Kotlin: internal val countOrElement = threadContextElements(context)
    void* count_or_element;

    // Kotlin: internal var _state: Any? = UNDEFINED
    std::optional<Result<T>> state_;

    DispatchedContinuation(
        std::shared_ptr<CoroutineDispatcher> dispatcher_,
        std::shared_ptr<Continuation<T>> continuation_
    ) :
        DispatchedTask<T>(MODE_UNINITIALIZED),
        dispatcher(std::move(dispatcher_)),
        continuation(std::move(continuation_)) {
        auto ctx = DispatchedContinuation<T>::get_context();
        count_or_element = ctx ? thread_context_elements(*ctx) : nullptr;
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        return continuation->get_context();
    }

    std::shared_ptr<SchedulerTask> shared_task() override {
        return this->shared_from_this();
    }

    // Kotlin: override val callerFrame: CoroutineStackFrame? get() = continuation as? CoroutineStackFrame
    CoroutineStackFrame* get_caller_frame() const override {
        return dynamic_cast<CoroutineStackFrame*>(continuation.get());
    }

    // Kotlin: override fun getStackTraceElement(): StackTraceElement? = null
    StackTraceElement* get_stack_trace_element() const override {
        return nullptr;
    }

    // Kotlin: override val delegate: Continuation<T> get() = this
    std::shared_ptr<Continuation<T>> get_delegate() override {
        return this->shared_from_this();
    }

    // Kotlin: override fun takeState(): Any?
    Result<T> take_state() override {
        assert(state_.has_value());
        auto taken = *state_;
        state_.reset();
        return taken;
    }

private:
    struct ReusableState {
        enum class Kind { CLAIMED, CONTINUATION, CANCELLATION };
        Kind kind;
        std::shared_ptr<CancellableContinuationImpl<T>> continuation;
        std::exception_ptr cause;

        explicit ReusableState(Kind kind) : kind(kind) {}
        explicit ReusableState(std::shared_ptr<CancellableContinuationImpl<T>> continuation)
            : kind(Kind::CONTINUATION), continuation(std::move(continuation)) {}
        explicit ReusableState(std::exception_ptr cause)
            : kind(Kind::CANCELLATION), cause(std::move(cause)) {}
    };
    std::shared_ptr<ReusableState> reusable_cancellable_continuation_;
    std::mutex typed_delegate_mutex_;
    std::type_index typed_reusable_type_{typeid(void)};
    std::shared_ptr<DispatchedContinuationBase> typed_reusable_delegate_;

    static const std::shared_ptr<ReusableState>& reusable_claimed() {
        static const auto claimed = std::make_shared<ReusableState>(ReusableState::Kind::CLAIMED);
        return claimed;
    }

public:
    /** Reuses a matching typed ABI delegate and releases the previous type when it changes. */
    template<typename U, typename Factory>
    std::shared_ptr<Continuation<U>> typed_reusable_delegate(Factory&& create) {
        std::shared_ptr<DispatchedContinuationBase> previous;
        std::shared_ptr<Continuation<U>> typed;
        {
            std::lock_guard<std::mutex> lock(typed_delegate_mutex_);
            const std::type_index type(typeid(U));
            if (typed_reusable_delegate_ && type == typed_reusable_type_) {
                return std::dynamic_pointer_cast<Continuation<U>>(typed_reusable_delegate_);
            }
            auto next = std::make_shared<DispatchedContinuation<U>>(dispatcher, create());
            previous = std::move(typed_reusable_delegate_);
            typed_reusable_delegate_ = next;
            typed_reusable_type_ = type;
            typed = std::move(next);
        }
        if (previous) previous->release();
        return typed;
    }

    /** Whether reuse has been claimed, published or cancelled while claimed. */
    bool is_reusable() const {
        return std::atomic_load(&reusable_cancellable_continuation_) != nullptr;
    }

    void await_reusability() {
        while (true) {
            auto state = std::atomic_load(&reusable_cancellable_continuation_);
            if (state != reusable_claimed()) return;
        }
    }

    /** Waits for the active claim, then detaches and releases the cached continuation. */
    void release() override {
        await_reusability();
        auto state = std::atomic_load(&reusable_cancellable_continuation_);
        if (state && state->kind == ReusableState::Kind::CONTINUATION) {
            state->continuation->detach_child();
            auto expected = state;
            std::atomic_compare_exchange_strong(
                &reusable_cancellable_continuation_, &expected, std::shared_ptr<ReusableState>{});
        }
        std::shared_ptr<DispatchedContinuationBase> typed;
        {
            std::lock_guard<std::mutex> lock(typed_delegate_mutex_);
            typed = std::move(typed_reusable_delegate_);
        }
        if (typed) typed->release();
    }

    /** Claims null or a published continuation; cancellation remains postponed while claimed. */
    std::shared_ptr<CancellableContinuationImpl<T>> claim_reusable_cancellable_continuation() {
        while (true) {
            auto state = std::atomic_load(&reusable_cancellable_continuation_);
            if (!state) {
                std::atomic_store(&reusable_cancellable_continuation_, reusable_claimed());
                return nullptr;
            }
            if (state == reusable_claimed()) continue;
            if (state->kind == ReusableState::Kind::CANCELLATION) continue;
            auto expected = state;
            if (std::atomic_compare_exchange_strong(
                    &reusable_cancellable_continuation_, &expected, reusable_claimed())) {
                return state->continuation;
            }
        }
    }

    /** Publishes the owned continuation or consumes cancellation postponed during its claim. */
    std::exception_ptr try_release_claimed_continuation(CancellableContinuation<T>* continuation_) {
        while (true) {
            auto state = std::atomic_load(&reusable_cancellable_continuation_);
            if (state == reusable_claimed()) {
                auto* impl = dynamic_cast<CancellableContinuationImpl<T>*>(continuation_);
                auto published = std::make_shared<ReusableState>(impl->shared_from_this());
                auto expected = state;
                if (std::atomic_compare_exchange_strong(
                        &reusable_cancellable_continuation_, &expected, published)) {
                    return nullptr;
                }
            } else if (state && state->kind == ReusableState::Kind::CANCELLATION) {
                auto expected = state;
                if (std::atomic_compare_exchange_strong(
                        &reusable_cancellable_continuation_, &expected, std::shared_ptr<ReusableState>{})) {
                    return state->cause;
                }
            } else {
                assert(false);
            }
        }
    }

    /** Records the first cancellation while claimed, otherwise invalidates reusable state. */
    bool postpone_cancellation(std::exception_ptr cause) {
        auto cancelled = std::make_shared<ReusableState>(std::move(cause));
        while (true) {
            auto state = std::atomic_load(&reusable_cancellable_continuation_);
            if (state == reusable_claimed()) {
                auto expected = state;
                if (std::atomic_compare_exchange_strong(
                        &reusable_cancellable_continuation_, &expected, cancelled)) {
                    return true;
                }
            } else if (state && state->kind == ReusableState::Kind::CANCELLATION) {
                return true;
            } else {
                auto expected = state;
                if (std::atomic_compare_exchange_strong(
                        &reusable_cancellable_continuation_, &expected, std::shared_ptr<ReusableState>{})) {
                    return false;
                }
            }
        }
    }

    // Kotlin: override fun resumeWith(result: Result<T>)
    void resume_with(Result<T> result) override {
        auto state = result;
        auto context = get_context();
        if (context && safe_is_dispatch_needed(*dispatcher, *context)) {
            state_ = state;
            this->resume_mode = MODE_ATOMIC;
            safe_dispatch(*dispatcher, *context, this->shared_from_this());
        } else {
            execute_unconfined(state, MODE_ATOMIC, false, [this, result = std::move(result), context]() mutable {
                with_coroutine_context<void>(context, count_or_element, [this, result = std::move(result)]() mutable {
                    continuation->resume_with(std::move(result));
                    return;
                });
            });
        }
    }

    // Kotlin: internal inline fun resumeCancellableWith(result: Result<T>)
    void resume_cancellable_with(Result<T> result) {
        auto state = result;
        auto context = get_context();
        if (context && safe_is_dispatch_needed(*dispatcher, *context)) {
            state_ = state;
            this->resume_mode = MODE_CANCELLABLE;
            safe_dispatch(*dispatcher, *context, this->shared_from_this());
        } else {
            execute_unconfined(state, MODE_CANCELLABLE, false, [this, result = std::move(result), state]() mutable {
                if (!resume_cancelled(state)) {
                    resume_undispatched_with(std::move(result));
                }
            });
        }
    }

    // Kotlin: internal inline fun resumeCancelled(state: Any?): Boolean
    bool resume_cancelled(const Result<T>& state) {
        auto context = get_context();
        std::shared_ptr<Job> job = nullptr;
        if (context) {
            auto job_element = context->get(Job::type_key);
            job = std::dynamic_pointer_cast<Job>(job_element);
        }
        if (job && !job->is_active()) {
            auto cause = job->get_cancellation_exception();
            this->cancel_completed_result(state, cause);
            this->resume_with(Result<T>::failure(cause));
            return true;
        }
        return false;
    }

    // Kotlin: internal inline fun resumeUndispatchedWith(result: Result<T>)
    void resume_undispatched_with(Result<T> result) {
        with_continuation_context<void, T>(continuation, count_or_element, [this, result = std::move(result)]() mutable {
            continuation->resume_with(std::move(result));
            return;
        });
    }

    // Kotlin: internal fun dispatchYield(context: CoroutineContext, value: T)
    // Kotlin: internal fun dispatchYield(context: CoroutineContext, value: T)
    template<typename U = T>
    std::enable_if_t<!std::is_void_v<U>> dispatch_yield(const CoroutineContext& context, U value) {
        state_ = Result<T>::success(std::move(value));
        this->resume_mode = MODE_CANCELLABLE;
        dispatcher->dispatch_yield(context, this->shared_from_this());
    }

    template<typename U = T>
    std::enable_if_t<std::is_void_v<U>> dispatch_yield(const CoroutineContext& context) {
        state_ = Result<T>::success();
        this->resume_mode = MODE_CANCELLABLE;
        dispatcher->dispatch_yield(context, this->shared_from_this());
    }

    // Kotlin: override fun toString()
    std::string to_string() const {
        return std::string("DispatchedContinuation[") +
            (dispatcher ? dispatcher->to_string() : "<null>") +
            ", " +
            to_debug_string(continuation.get()) +
            "]";
    }

public:
    /**
 * Executes given [block] as part of current event loop, updating current continuation
 * mode and state if continuation is not resumed immediately.
 * [doYield] indicates whether current continuation is yielding (to provide fast-path if event-loop is empty).
 * Returns `true` if execution of continuation was queued (trampolined) or `false` otherwise.
 */
    // Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedContinuation.kt:293-312
    // NOTE(port): Source private generic extension remains in the template header.
    template<typename Block>
    bool execute_unconfined(const Result<T>& cont_state, int mode, bool do_yield, Block&& block) {
        assert(mode != MODE_UNINITIALIZED); // invalid execution mode
        auto event_loop = ThreadLocalEventLoop::get_event_loop();
        // If we are yielding and unconfined queue is empty, we can bail out as part of fast path
        if (do_yield && event_loop->is_unconfined_queue_empty()) return false;
        if (event_loop->is_unconfined_loop_active()) {
            // When unconfined loop is active -- dispatch continuation for execution to avoid stack overflow
            state_ = cont_state;
            this->resume_mode = mode;
            event_loop->dispatch_unconfined(this->shared_task());
            return true; // queued into the active loop
        } else {
            // Was not active -- run event loop until all unconfined tasks are executed
            run_unconfined_event_loop(this, *event_loop, std::forward<Block>(block));
            return false;
        }
    }

};

// Kotlin: internal fun CoroutineDispatcher.safeDispatch(...)
inline void safe_dispatch(
    const CoroutineDispatcher& dispatcher,
    const CoroutineContext& context,
    std::shared_ptr<Runnable> runnable
) {
    try {
        dispatcher.dispatch(context, std::move(runnable));
    } catch (...) {
        throw DispatchException(std::current_exception(), &dispatcher, &context);
    }
}

// Kotlin: internal fun CoroutineDispatcher.safeIsDispatchNeeded(...)
inline bool safe_is_dispatch_needed(const CoroutineDispatcher& dispatcher, const CoroutineContext& context) {
    try {
        return dispatcher.is_dispatch_needed(context);
    } catch (...) {
        throw DispatchException(std::current_exception(), &dispatcher, &context);
    }
}

} // namespace internal

/**
 * Kotlin: public fun <T> Continuation<T>.resumeCancellableWith(...)
 */
template<typename T>
inline void resume_cancellable_with(const std::shared_ptr<Continuation<T>>& continuation, Result<T> result) {
    if (auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<T>>(continuation)) {
        dispatched->resume_cancellable_with(std::move(result));
    } else {
        continuation->resume_with(std::move(result));
    }
}

/**
 * Kotlin: internal fun DispatchedContinuation<Unit>.yieldUndispatched(): Boolean
 */
inline bool yield_undispatched(internal::DispatchedContinuation<void>& continuation) {
    return continuation.execute_unconfined(Result<void>::success(), MODE_CANCELLABLE, true, [&continuation]() {
        continuation.run();
    });
}

// Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedContinuation.kt:282-285
// NOTE(port): Unit is nullptr at the Continuation<void*> ABI boundary.
inline bool yield_undispatched(internal::DispatchedContinuation<void*>& continuation) {
    return continuation.execute_unconfined(Result<void*>::success(nullptr), MODE_CANCELLABLE, true, [&continuation]() {
        continuation.run();
    });
}

// Implement CoroutineDispatcher::intercept_continuation here to avoid circular dependency
template <typename T>
std::shared_ptr<Continuation<T>> CoroutineDispatcher::intercept_continuation(std::shared_ptr<Continuation<T>> continuation) {
    return std::make_shared<internal::DispatchedContinuation<T>>(
        std::dynamic_pointer_cast<CoroutineDispatcher>(shared_from_this()),
        std::move(continuation));
}

} // namespace coroutines
} // namespace kotlinx

// Definitions for DispatchedTask::run/dispatch/resume live here to avoid cycles.
#include "kotlinx/coroutines/common/DispatchedTaskDispatch.hpp"
