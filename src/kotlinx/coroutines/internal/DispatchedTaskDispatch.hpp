#pragma once
/**
 * @file DispatchedTaskDispatch.hpp
 *
 * Definitions for DispatchedTask template methods and related helpers.
 * Split out to avoid circular dependencies between DispatchedTask and EventLoop/DispatchedContinuation.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt
 */

#include "kotlinx/coroutines/internal/DispatchedTask.hpp"
#include "kotlinx/coroutines/internal/CoroutineContextUtils.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/CoroutineExceptionHandler.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/EventLoop.hpp"
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"

#include <functional>
#include <cassert>
#include <type_traits>
#include <string>

namespace kotlinx {
namespace coroutines {

template<typename T>
inline void resume_with_stack_trace(Continuation<T>& continuation, std::exception_ptr exception) {
    continuation.resume_with(Result<T>::failure(exception));
}

template<typename T>
void DispatchedTask<T>::handle_fatal_exception(std::exception_ptr exception) {
    CoroutinesInternalError reason(
        std::string("Fatal exception in coroutines machinery for DispatchedTask. ") +
            "Please read KDoc to 'handleFatalException' method and report this incident to maintainers",
        exception);
    auto delegate = get_delegate();
    if (delegate) {
        auto ctx = delegate->get_context();
        if (ctx) {
            handle_coroutine_exception(*ctx, std::make_exception_ptr(reason));
        }
    }
}

template<typename T>
void DispatchedTask<T>::run() {
    assert(resume_mode != MODE_UNINITIALIZED);
    auto delegate = get_delegate();
    try {
        // Kotlin: val delegate = delegate as DispatchedContinuation<T>
        auto dispatched_delegate = std::dynamic_pointer_cast<internal::DispatchedContinuation<T>>(delegate);
        auto continuation = dispatched_delegate ? dispatched_delegate->continuation : delegate;
        void* count_or_element = dispatched_delegate ? dispatched_delegate->count_or_element : nullptr;

        with_continuation_context<void, T>(
            continuation,
            count_or_element,
            [this, continuation, dispatched_delegate]() {
                auto context = continuation->get_context();
                auto state = take_state(); // must take state even if cancelled
                auto exception = get_exceptional_result(state);

                std::shared_ptr<Job> job = nullptr;
                if (!exception && is_cancellable_mode(resume_mode) && context) {
                    auto job_element = context->get(Job::type_key);
                    job = std::dynamic_pointer_cast<Job>(job_element);
                }

                if (job && !job->is_active()) {
                    auto cause = job->get_cancellation_exception();
                    cancel_completed_result(state, cause);
                    resume_with_stack_trace(*continuation, cause);
                } else {
                    if (exception) {
                        continuation->resume_with(Result<T>::failure(exception));
                    } else {
                        if constexpr (std::is_void_v<T>) {
                            continuation->resume_with(Result<void>::success());
                        } else {
                            continuation->resume_with(Result<T>::success(get_successful_result<T>(state)));
                        }
                    }
                }
                return;
            });
    } catch (const internal::DispatchException& e) {
        if (delegate) {
            auto ctx = delegate->get_context();
            if (ctx) {
                handle_coroutine_exception(*ctx, e.cause);
            }
        }
    } catch (...) {
        handle_fatal_exception(std::current_exception());
    }
}

template<typename T>
static void resume_unconfined(DispatchedTask<T>* task) {
    auto event_loop = ThreadLocalEventLoop::get_event_loop();
    if (!event_loop) {
        resume(task, task->get_delegate(), true);
        return;
    }

    if (event_loop->is_unconfined_loop_active()) {
        event_loop->dispatch_unconfined(std::shared_ptr<SchedulerTask>(task, [](SchedulerTask*){}));
        return;
    }

    // Kotlin: runUnconfinedEventLoop(eventLoop) { resume(delegate, undispatched = true) }
    event_loop->increment_use_count(true);
    try {
        resume(task, task->get_delegate(), true);
        while (true) {
            if (!event_loop->process_unconfined_event()) break;
        }
        event_loop->decrement_use_count(true);
    } catch (...) {
        task->handle_fatal_exception(std::current_exception());
        event_loop->decrement_use_count(true);
    }
}

template<typename T>
void dispatch(DispatchedTask<T>* task, int mode) {
    assert(mode != MODE_UNINITIALIZED);

    auto delegate = task->get_delegate();
    bool undispatched = (mode == MODE_UNDISPATCHED);

    auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<T>>(delegate);
    if (!undispatched && dispatched && is_cancellable_mode(mode) == is_cancellable_mode(task->resume_mode)) {
        auto dispatcher = dispatched->dispatcher;
        auto context = dispatched->get_context();
        if (dispatcher && context && internal::safe_is_dispatch_needed(*dispatcher, *context)) {
            internal::safe_dispatch(*dispatcher, *context, std::shared_ptr<Runnable>(task, [](Runnable*){}));
        } else {
            resume_unconfined(task);
        }
    } else {
        resume(task, delegate, undispatched);
    }
}

template<typename T>
void resume(DispatchedTask<T>* task, std::shared_ptr<Continuation<T>> delegate, bool undispatched) {
    auto state = task->take_state();
    auto exception = task->get_exceptional_result(state);

    Result<T> result;
    if (exception) {
        result = Result<T>::failure(exception);
    } else {
        if constexpr (std::is_void_v<T>) {
            result = Result<void>::success();
        } else {
            result = Result<T>::success(task->template get_successful_result<T>(state));
        }
    }

    if (undispatched) {
        if (auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<T>>(delegate)) {
            with_continuation_context<void, T>(
                dispatched->continuation,
                dispatched->count_or_element,
                [dispatched, result]() mutable {
                    dispatched->continuation->resume_with(result);
                    return;
                });
            return;
        }
    }

    delegate->resume_with(std::move(result));
}

} // namespace coroutines
} // namespace kotlinx
