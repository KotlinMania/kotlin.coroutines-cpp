/**
 * @file Yield.cpp
 * @brief Implementation of yield function
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Yield.kt
 *
 * yield() suspends the current coroutine and immediately schedules it for
 * further execution on its dispatcher. This allows other coroutines to run.
 */

#include "kotlinx/coroutines/Yield.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"
#include "kotlinx/coroutines/EventLoop.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include <thread>

namespace kotlinx {
namespace coroutines {

// Legacy function - just does OS thread yield
void yield_coroutine() {
    std::this_thread::yield();
}

/**
 * A Runnable that resumes a continuation when run.
 * This bridges the dispatcher's Runnable interface with the continuation mechanism.
 */
class ResumeContinuationRunnable : public Runnable {
private:
    std::shared_ptr<Continuation<void*>> continuation_;

public:
    explicit ResumeContinuationRunnable(std::shared_ptr<Continuation<void*>> cont)
        : continuation_(std::move(cont)) {}

    void run() override {
        if (continuation_) {
            // Resume with Unit result (nullptr = success with Unit)
            continuation_->resume_with(Result<void*>::success(nullptr));
        }
    }
};

/**
 * Proper suspend function implementation of yield().
 *
 * Transliterated from Yield.kt:
 *   public suspend fun yield(): Unit = suspendCoroutineUninterceptedOrReturn sc@ { uCont ->
 *       val context = uCont.context
 *       context.ensureActive()
 *       val cont = uCont.intercepted() as? DispatchedContinuation<Unit> ?: return@sc Unit
 *       if (cont.dispatcher.safeIsDispatchNeeded(context)) {
 *           cont.dispatchYield(context, Unit)
 *       } else {
 *           // ... unconfined handling ...
 *       }
 *       COROUTINE_SUSPENDED
 *   }
 */
void* yield(std::shared_ptr<Continuation<void*>> completion) {
    using namespace intrinsics;

    if (!completion) {
        // No continuation - can't suspend, just do OS yield
        std::this_thread::yield();
        return nullptr;
    }

    // 1. Get the context
    auto context = completion->get_context();
    if (!context) {
        std::this_thread::yield();
        return nullptr;
    }

    // 2. Check for cancellation - context.ensureActive()
    // Get Job from context and check if cancelled
    auto job_element = context->get(Job::type_key);
    if (job_element) {
        auto job = std::dynamic_pointer_cast<Job>(job_element);
        if (job && !job->is_active()) {
            // Job is cancelled - throw CancellationException
            std::rethrow_exception(job->get_cancellation_exception());
        }
    }

    // 3. Get the dispatcher from context
    auto interceptor_element = context->get(ContinuationInterceptor::type_key);
    CoroutineDispatcher* dispatcher = nullptr;
    if (interceptor_element) {
        dispatcher = dynamic_cast<CoroutineDispatcher*>(interceptor_element.get());
    }

    // 4. If no dispatcher, check for thread-local event loop
    if (!dispatcher) {
        auto event_loop = ThreadLocalEventLoop::current_or_null();
        if (event_loop) {
            dispatcher = event_loop.get();
        }
    }

    // 5. If still no dispatcher, yield is a no-op (return immediately)
    if (!dispatcher) {
        std::this_thread::yield();
        return nullptr;
    }

    // 6. Schedule resumption and return COROUTINE_SUSPENDED
    // Create a runnable that will resume the continuation
    auto resumeTask = std::make_shared<ResumeContinuationRunnable>(completion);

    // Dispatch the task to resume later
    dispatcher->dispatch(*context, resumeTask);

    // Return suspended marker - caller will propagate this
    return COROUTINE_SUSPENDED;
}

} // namespace coroutines
} // namespace kotlinx
