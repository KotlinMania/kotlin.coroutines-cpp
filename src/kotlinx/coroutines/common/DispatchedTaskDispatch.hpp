#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt
/**
 * @file DispatchedTaskDispatch.hpp
 *
 * Definitions for DispatchedTask template methods and related helpers.
 * Split out to avoid circular dependencies between DispatchedTask and EventLoop/DispatchedContinuation.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt
 */

#include "kotlinx/coroutines/internal/DispatchedTask.hpp"
#include "kotlinx/coroutines/common/CoroutineContextUtils.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/CoroutineExceptionHandler.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/internal/StackTraceRecovery.hpp"
#include "kotlinx/coroutines/EventLoop.hpp"
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"

#include <functional>
#include <cassert>
#include <type_traits>
#include <string>

namespace kotlinx {
namespace coroutines {

// Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:203-205
template<typename T>
inline void resume_with_stack_trace(Continuation<T>& continuation, std::exception_ptr exception) {
    continuation.resume_with(Result<T>::failure(internal::recover_stack_trace(exception, &continuation)));
}

// Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:129-133
template<typename T>
void DispatchedTask<T>::handle_fatal_exception(std::exception_ptr exception) {
    CoroutinesInternalError reason(
        std::string("Fatal exception in coroutines machinery for DispatchedTask. ") +
            "Please read KDoc to 'handleFatalException' method and report this incident to maintainers",
        exception);
    auto delegate = get_delegate();
    handle_coroutine_exception(*delegate->get_context(), std::make_exception_ptr(reason));
}

// Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:77-109
template<typename T>
void DispatchedTask<T>::run() {
    assert(resume_mode != MODE_UNINITIALIZED); // should have been set before dispatching
    try {
        auto dispatched_delegate = std::dynamic_pointer_cast<internal::DispatchedContinuation<T>>(get_delegate());
        // NOTE(port): Kotlin's checked cast fails; a plain delegate is not a fallback.
        if (!dispatched_delegate) throw std::bad_cast();
        auto continuation = dispatched_delegate->continuation;
        void* count_or_element = dispatched_delegate->count_or_element;

        with_continuation_context<void, T>(
            continuation,
            count_or_element,
            [this, continuation]() {
                auto context = continuation->get_context();
                auto state = take_state(); // NOTE: Must take state in any case, even if cancelled
                auto exception = get_exceptional_result(state);

                /*
                 * Check whether continuation was originally resumed with an exception.
                 * If so, it dominates cancellation, otherwise the original exception
                 * will be silently lost.
                 */
                std::shared_ptr<Job> job = nullptr;
                if (!exception && is_cancellable_mode(resume_mode)) {
                    auto job_element = context->get(Job::type_key);
                    job = std::dynamic_pointer_cast<Job>(job_element);
                    if (job_element && !job) throw std::bad_cast();
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
        handle_coroutine_exception(*get_delegate()->get_context(), e.cause);
    } catch (...) {
        handle_fatal_exception(std::current_exception());
    }
}

// Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:180-200
template<typename T, typename Block>
inline void run_unconfined_event_loop(DispatchedTask<T>* task, EventLoop& event_loop, Block&& block) {
    event_loop.increment_use_count(true);
    // NOTE(port): An outer catch implements Kotlin finally even when fatal reporting throws.
    try {
        try {
            block();
            while (true) {
                // break when all unconfined continuations where executed
                if (!event_loop.process_unconfined_event()) break;
            }
        } catch (...) {
            /*
             * This exception doesn't happen normally, only if we have a bug in implementation.
             * Report it as a fatal exception.
             */
            task->handle_fatal_exception(std::current_exception());
        }
    } catch (...) {
        event_loop.decrement_use_count(true);
        throw;
    }
    event_loop.decrement_use_count(true);
}

// Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:167-178
template<typename T>
static void resume_unconfined(DispatchedTask<T>* task) {
    auto event_loop = ThreadLocalEventLoop::get_event_loop();
    if (event_loop->is_unconfined_loop_active()) {
        // When unconfined loop is active -- dispatch continuation for execution to avoid stack overflow
        // NOTE(port): The event loop retains the same task instance as Kotlin GC.
        event_loop->dispatch_unconfined(task->shared_task());
    } else {
        // Was not active -- run event loop until all unconfined tasks are executed
        run_unconfined_event_loop(task, *event_loop, [task] {
            resume(task, task->get_delegate(), true);
        });
    }
}

// Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:136-154
template<typename T>
void dispatch(DispatchedTask<T>* task, int mode) {
    assert(mode != MODE_UNINITIALIZED);

    auto delegate = task->get_delegate();
    bool undispatched = (mode == MODE_UNDISPATCHED);

    auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<T>>(delegate);
    if (!undispatched && dispatched && is_cancellable_mode(mode) == is_cancellable_mode(task->resume_mode)) {
        auto dispatcher = dispatched->dispatcher;
        auto context = dispatched->get_context();
        if (internal::safe_is_dispatch_needed(*dispatcher, *context)) {
            // dispatch directly using this instance's Runnable implementation
            internal::safe_dispatch(*dispatcher, *context, task->shared_task());
        } else {
            resume_unconfined(task);
        }
    } else {
        // delegate is coming from 3rd-party interceptor implementation (and does not support cancellation)
        // or undispatched mode was requested
        resume(task, delegate, undispatched);
    }
}

// Transliterated from: kotlinx-coroutines-core/common/src/internal/DispatchedTask.kt:156-165
template<typename T>
void resume(DispatchedTask<T>* task, std::shared_ptr<Continuation<T>> delegate, bool undispatched) {
    // This resume is never cancellable. The result is always delivered to delegate continuation.
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
        auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<T>>(delegate);
        if (!dispatched) throw std::bad_cast();
        dispatched->resume_undispatched_with(std::move(result));
        return;
    }

    delegate->resume_with(std::move(result));
}

} // namespace coroutines
} // namespace kotlinx
