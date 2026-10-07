// port-lint: source kotlinx-coroutines-core/common/src/Delay.kt
/**
 * @file Delay.cpp
 * @brief Implementation of delay functions
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt
 *
 * Provides delay functionality for coroutines.
 */

#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/common/DispatchedTaskDispatch.hpp"
#include <limits>

namespace kotlinx {
    namespace coroutines {
        // Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:155-158
        long long to_delay_millis(kotlin::time::Duration duration) {
            return duration.is_positive()
                ? (duration + kotlin::time::nanoseconds(999999)).in_whole_milliseconds() : 0;
        }
        // Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:149
        Delay& get_delay(const CoroutineContext& context) {
            auto element = context.get(ContinuationInterceptor::type_key);
            if (element) {
                if (auto* delay_impl = dynamic_cast<Delay*>(element.get())) {
                    return *delay_impl;
                }
            }
            return get_default_delay();
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:52-54
        std::shared_ptr<DisposableHandle> Delay::invoke_on_timeout(
            long long time_millis,
            std::shared_ptr<Runnable> block,
            const CoroutineContext& context) {
            return get_default_delay().invoke_on_timeout(time_millis, block, context);
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:25-28
        void* Delay::delay(long long time_millis, Continuation<void*>* continuation) {
            if (time_millis <= 0) return nullptr;
            return suspend_cancellable_coroutine<void>(
                [this, time_millis](CancellableContinuation<void>& cont) {
                    schedule_resume_after_delay(time_millis, cont);
                },
                continuation);
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:25-28
        void* Delay::delay(long long time_millis, std::shared_ptr<Continuation<void*>> continuation) {
            if (time_millis <= 0) return nullptr;
            auto block = [this, time_millis](CancellableContinuation<void>& cont) {
                schedule_resume_after_delay(time_millis, cont);
            };
            return suspend_cancellable_coroutine_void(block, std::move(continuation));
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:121-129
        void *delay(long long time_millis, Continuation<void *> *continuation) {
            if (time_millis <= 0) return nullptr; // Return immediately

            return suspend_cancellable_coroutine<void>([time_millis](CancellableContinuation<void> &cont) {
                if (time_millis < std::numeric_limits<long long>::max()) {
                    get_delay(*cont.get_context()).schedule_resume_after_delay(time_millis, cont);
                }
            }, continuation);
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:146
        void* delay(kotlin::time::Duration duration, Continuation<void*>* continuation) {
            return delay(to_delay_millis(duration), continuation);
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:103
        void *await_cancellation(Continuation<void *> *continuation) {
            // Kotlin: suspendCancellableCoroutine {} - empty lambda, never resumes
            return suspend_cancellable_coroutine<void>([](CancellableContinuation<void>&) {
                // Do nothing. Never resume. Wait until cancelled.
            }, continuation);
        }
    } // namespace coroutines
} // namespace kotlinx

// -----------------------------------------------------------------------------
// Owned continuation ABI entries
// -----------------------------------------------------------------------------

namespace kotlinx {
    namespace coroutines {

        // Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:121-129
        void* delay(long long time_millis, std::shared_ptr<Continuation<void*>> continuation) {
            if (time_millis <= 0) return nullptr;
            auto block = [time_millis](CancellableContinuation<void>& cont) {
                // At std::numeric_limits<long long>::max(), wait like await_cancellation; do not schedule.
                if (time_millis < std::numeric_limits<long long>::max()) {
                    get_delay(*cont.get_context()).schedule_resume_after_delay(time_millis, cont);
                }
            };
            return suspend_cancellable_coroutine_void(block, std::move(continuation));
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:146
        void* delay(kotlin::time::Duration duration, std::shared_ptr<Continuation<void*>> continuation) {
            return delay(to_delay_millis(duration), std::move(continuation));
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Delay.kt:103
        void* await_cancellation(std::shared_ptr<Continuation<void*>> continuation) {
            auto block = [](CancellableContinuation<void>&) {};
            return suspend_cancellable_coroutine_void(block, std::move(continuation));
        }

    } // namespace coroutines
} // namespace kotlinx
