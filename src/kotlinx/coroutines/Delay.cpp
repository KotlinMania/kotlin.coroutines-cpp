/**
 * @file Delay.cpp
 * @brief Implementation of delay functions
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt
 *
 * Provides delay functionality for coroutines.
 *
 * NOTE: In a full C++20 coroutine implementation, delay would be a suspend function.
 * This implementation provides a simplified blocking version.
 */

#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/internal/DispatchedTaskDispatch.hpp"
#include <thread>
#include <chrono>
#include <limits>

namespace kotlinx {
    namespace coroutines {
        std::shared_ptr<DisposableHandle> Delay::invoke_on_timeout(
            long long time_millis,
            std::shared_ptr<Runnable> block,
            const CoroutineContext& context) {
            return get_default_delay().invoke_on_timeout(time_millis, block, context);
        }

        void* Delay::delay(long long time_millis, Continuation<void*>* continuation) {
            if (time_millis <= 0) return nullptr;
            return suspend_cancellable_coroutine<void>(
                [this, time_millis](CancellableContinuation<void>& cont) {
                    schedule_resume_after_delay(time_millis, cont);
                },
                continuation);
        }

        void *delay(long long time_millis, Continuation<void *> *continuation) {
            if (time_millis <= 0) return nullptr; // Return immediately

            return suspend_cancellable_coroutine<void>([time_millis](CancellableContinuation<void> &cont) {
                // 1. Get context
                auto context = cont.get_context();
                // 2. Look for Delay interface in context (Dispatcher implements Delay?)
                // In Kotlin: context[ContinuationInterceptor] as? Delay
                // In C++: dynamic_cast
                Delay *delay_impl = nullptr;
                auto element = context->get(ContinuationInterceptor::type_key);
                if (element) {
                    delay_impl = dynamic_cast<Delay *>(element.get());
                }

                if (delay_impl) {
                    if (time_millis < std::numeric_limits<long long>::max()) {
                        delay_impl->schedule_resume_after_delay(time_millis, cont);
                    }
                } else {
                    if (time_millis < std::numeric_limits<long long>::max()) {
                        get_default_delay().schedule_resume_after_delay(time_millis, cont);
                    }
                }
            }, continuation);
        }

        void *delay(std::chrono::nanoseconds duration, Continuation<void *> *continuation) {
            long long millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
            // Round up if > 0 but < 1ms?
            if (millis == 0 && duration.count() > 0) millis = 1;
            return delay(millis, continuation);
        }

        void *delay(std::chrono::milliseconds duration, Continuation<void *> *continuation) {
            return delay(duration.count(), continuation);
        }

        void *await_cancellation(Continuation<void *> *continuation) {
            // Kotlin: suspendCancellableCoroutine {} - empty lambda, never resumes
            return suspend_cancellable_coroutine<void>([](CancellableContinuation<void>&) {
                // Do nothing. Never resume. Wait until cancelled.
            }, continuation);
        }
    } // namespace coroutines
} // namespace kotlinx

// -----------------------------------------------------------------------------
// shared_ptr Overloads (convenience wrappers)
// -----------------------------------------------------------------------------

namespace kotlinx {
    namespace coroutines {

        void* delay(long long time_millis, std::shared_ptr<Continuation<void*>> continuation) {
            return delay(time_millis, continuation.get());
        }

        void* delay(std::chrono::nanoseconds duration, std::shared_ptr<Continuation<void*>> continuation) {
            return delay(duration, continuation.get());
        }

        void* delay(std::chrono::milliseconds duration, std::shared_ptr<Continuation<void*>> continuation) {
            return delay(duration, continuation.get());
        }

        void* await_cancellation(std::shared_ptr<Continuation<void*>> continuation) {
            return await_cancellation(continuation.get());
        }

    } // namespace coroutines
} // namespace kotlinx
