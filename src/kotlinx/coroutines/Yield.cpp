// port-lint: source Yield.kt
/**
 * @file Yield.cpp
 * @brief Implementation of yield function
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Yield.kt
 *
 * yield() checks cancellation and offers execution to other coroutines through
 * its intercepted dispatcher. The Unconfined empty-queue path returns Unit.
 */

#include "kotlinx/coroutines/Yield.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"
#include "kotlinx/coroutines/Unconfined.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"
#include "kotlinx/coroutines/EventLoop.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/internal/CurrentRunningCoroutine.hpp"
#include <thread>

namespace kotlinx {
namespace coroutines {

// Legacy function - just does OS thread yield or event loop step
void yield_coroutine() {
    auto loop = ThreadLocalEventLoop::current_or_null();
    if (loop && !loop->is_empty()) {
        loop->process_next_event();
        return;
    }
    if (auto cont = internal::CurrentRunningCoroutine::current) {
        internal::CurrentRunningCoroutine::suspended = intrinsics::is_coroutine_suspended(yield(cont));
        return;
    }
    std::this_thread::yield();
}

// Transliterated from: kotlinx-coroutines-core/common/src/Yield.kt:145-166
void* yield(std::shared_ptr<Continuation<void*>> completion) {
    if (!completion) return nullptr;
    auto context = completion->get_context();
    if (context) {
        auto job = std::dynamic_pointer_cast<Job>(context->get(Job::type_key));
        if (job && !job->is_active()) std::rethrow_exception(job->get_cancellation_exception());
    }
    auto cont = std::dynamic_pointer_cast<internal::DispatchedContinuation<void*>>(
        intrinsics::intercepted(std::move(completion)));
    if (!cont) return nullptr;
    if (internal::safe_is_dispatch_needed(*cont->dispatcher, *context)) {
        cont->dispatch_yield(*context, static_cast<void*>(nullptr));
    } else {
        auto yield_context = std::make_shared<YieldContext>();
        cont->dispatch_yield(*context->operator+(yield_context), static_cast<void*>(nullptr));
        if (yield_context->dispatcher_was_unconfined) {
            return yield_undispatched(*cont) ? COROUTINE_SUSPENDED : nullptr;
        }
    }
    return COROUTINE_SUSPENDED;
}

} // namespace coroutines
} // namespace kotlinx
