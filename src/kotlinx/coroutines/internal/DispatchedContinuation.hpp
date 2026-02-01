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
#include <optional>
#include <string>
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

    /**
     * Possible states of reusability:
     *
     * 1) `null`. Cancellable continuation wasn't yet attempted to be reused or
     *    was used and then invalidated (e.g. because of the cancellation).
     * 2) `CancellableContinuation`. Continuation to be/that is being reused.
     * 3) `REUSABLE_CLAIMED`. CC is currently being reused and its owner executes `suspend` block:
     *    ```
     *    // state == null | CC
     *    suspend_cancellable_coroutine_reusable { cont ->
     *        // state == REUSABLE_CLAIMED
     *        block(cont)
     *    }
     *    // state == CC
     *    ```
     * 4) `Throwable`. Continuation was cancelled with this cause while being in
     *    `suspend_cancellable_coroutine_reusable`, `CancellableContinuationImpl::get_result`
     *    will check for cancellation later.
     *
     * `REUSABLE_CLAIMED` state is required to prevent double-use of the reused continuation.
     * In `get_result`, we have the following code:
     * ```
     * if (try_suspend()) {
     *     // <- at this moment current continuation can be redispatched and claimed again.
     *     attach_child_to_parent();
     *     release_claimed_continuation();
     * }
     * ```
     *
     * C++ encoding:
     * - 0: null
     * - k_reusable_claimed: REUSABLE_CLAIMED
     * - even pointer: CancellableContinuationImpl<T>*
     * - odd pointer (except k_reusable_claimed): postponed cancellation cause (boxed std::exception_ptr)
     */
    std::atomic<std::uintptr_t> reusable_cancellable_continuation_{0};

    bool is_reusable() const {
        return reusable_cancellable_continuation_.load() != 0;
    }

    void await_reusability() {
        while (true) {
            auto state = reusable_cancellable_continuation_.load();
            if (state != k_reusable_claimed) return;
        }
    }

    void release() override {
        await_reusability();
        auto cc = reusable_cancellable_continuation();
        if (cc) cc->detach_child();
    }

    CancellableContinuationImpl<T>* claim_reusable_cancellable_continuation() {
        while (true) {
            auto state = reusable_cancellable_continuation_.load();
            if (state == 0) {
                reusable_cancellable_continuation_.store(k_reusable_claimed);
                return nullptr;
            }
            if (state == k_reusable_claimed) {
                continue;
            }
            if (is_postponed_cancellation(state)) {
                continue;
            }
            auto expected = state;
            if (reusable_cancellable_continuation_.compare_exchange_strong(expected, k_reusable_claimed)) {
                return reinterpret_cast<CancellableContinuationImpl<T>*>(state);
            }
        }
    }

    std::exception_ptr try_release_claimed_continuation(CancellableContinuation<T>* continuation_) {
        while (true) {
            auto state = reusable_cancellable_continuation_.load();
            if (state == k_reusable_claimed) {
                auto* impl = dynamic_cast<CancellableContinuationImpl<T>*>(continuation_);
                auto new_state = reinterpret_cast<std::uintptr_t>(impl);
                auto expected = state;
                if (reusable_cancellable_continuation_.compare_exchange_strong(expected, new_state)) {
                    return nullptr;
                }
            } else if (is_postponed_cancellation(state)) {
                auto* box = postponed_cancellation_box(state);
                auto expected = state;
                if (reusable_cancellable_continuation_.compare_exchange_strong(expected, 0)) {
                    auto cause = *box;
                    delete box;
                    return cause;
                }
            } else {
                assert(false);
            }
        }
    }

    bool postpone_cancellation(std::exception_ptr cause) {
        auto* box = new std::exception_ptr(cause);
        auto boxed = reinterpret_cast<std::uintptr_t>(box) | k_postponed_tag;
        while (true) {
            auto state = reusable_cancellable_continuation_.load();
            if (state == k_reusable_claimed) {
                auto expected = state;
                if (reusable_cancellable_continuation_.compare_exchange_strong(expected, boxed)) {
                    return true;
                }
            } else if (is_postponed_cancellation(state)) {
                delete box;
                return true;
            } else {
                auto expected = state;
                if (reusable_cancellable_continuation_.compare_exchange_strong(expected, 0)) {
                    delete box;
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

    void run() override {
        DispatchedTask<T>::run();
    }

private:
    static constexpr std::uintptr_t k_reusable_claimed = 1;
    static constexpr std::uintptr_t k_postponed_tag = 1;

    bool is_postponed_cancellation(std::uintptr_t state) const {
        return (state & k_postponed_tag) != 0 && state != k_reusable_claimed;
    }

    std::exception_ptr* postponed_cancellation_box(std::uintptr_t state) const {
        return reinterpret_cast<std::exception_ptr*>(state & ~k_postponed_tag);
    }

    CancellableContinuationImpl<T>* reusable_cancellable_continuation() {
        auto state = reusable_cancellable_continuation_.load();
        if (state == 0 || state == k_reusable_claimed || is_postponed_cancellation(state)) return nullptr;
        return reinterpret_cast<CancellableContinuationImpl<T>*>(state);
    }

public:
    bool execute_unconfined(
        const Result<T>& cont_state,
        int mode,
        bool do_yield,
        std::function<void()> block
    ) {
        assert(mode != MODE_UNINITIALIZED);
        auto event_loop = ThreadLocalEventLoop::get_event_loop();
        if (!event_loop) {
            block();
            return false;
        }
        if (do_yield && event_loop->is_unconfined_queue_empty()) return false;
        if (event_loop->is_unconfined_loop_active()) {
            state_ = cont_state;
            this->resume_mode = mode;
            auto self = this->shared_from_this();
            event_loop->dispatch_unconfined(
                std::shared_ptr<SchedulerTask>(self, static_cast<SchedulerTask*>(self.get())));
            return true;
        }

        event_loop->increment_use_count(true);
        try {
            block();
            while (true) {
                if (!event_loop->process_unconfined_event()) break;
            }
            event_loop->decrement_use_count(true);
        } catch (...) {
            this->handle_fatal_exception(std::current_exception());
            event_loop->decrement_use_count(true);
        }
        return false;
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
