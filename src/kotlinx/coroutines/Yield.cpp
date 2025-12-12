/**
 * @file Yield.cpp
 * @brief Implementation of yield function
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Yield.kt
 *
 * See the header for full documentation.
 */

#include "kotlinx/coroutines/Yield.hpp"
#include "kotlinx/coroutines/Yield.hpp"
#include "kotlinx/coroutines/internal/DispatchedTaskDispatch.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include <thread>

namespace kotlinx {
    namespace coroutines {
        /*
 * TODO: STUB - yield_coroutine() yields thread instead of suspending coroutine
 *
 * Kotlin source: yield() in Yield.kt
 *
 * What's missing:
 * - Should be a suspend function: suspend fun yield()
 * - Should check for cancellation first: context.ensureActive()
 * - Should get dispatcher from context and call dispatch()
 * - Suspend coroutine and re-enqueue it to dispatcher's queue
 * - This allows other coroutines on same dispatcher to run
 *
 * Current behavior: Calls std::this_thread::yield()
 *   - Only yields to OS scheduler, not to other coroutines
 *   - Other coroutines on same thread don't get a chance to run
 *   - No cancellation check
 * Correct behavior: Suspend current coroutine, dispatch it back to queue,
 *   allowing other coroutines to run first (cooperative scheduling)
 *
 * Dependencies:
 * - Kotlin-style suspension (Continuation<void*>* parameter)
 * - CoroutineDispatcher.dispatch() integration
 * - ensureActive() for cancellation check
 *
 * Impact: Medium - cooperative yielding doesn't work, but OS yields still help
 *
 * Workaround: Use delay(1) for a minimal suspension point
 */
        void yield_coroutine() {
             std::this_thread::yield();
        }

        struct LambdaRunnable : public Runnable {
            std::function<void()> block;
            explicit LambdaRunnable(std::function<void()> b) : block(std::move(b)) {}
            void run() override { block(); }
        };

        [[suspend]]
        void* yield(std::shared_ptr<Continuation<void*>> completion) {
            using namespace kotlinx::coroutines::dsl;
            
            suspend(suspend_cancellable_coroutine<void>([](CancellableContinuation<void>& cont) {
                auto context = cont.get_context();
                context_ensure_active(*context);

                auto element = context->get(ContinuationInterceptor::type_key);
                auto* dispatcher = element ? dynamic_cast<CoroutineDispatcher*>(element.get()) : nullptr;

                if (dispatcher) {
                     // We need to dispatch resumption.
                     // Wrap resume in a Runnable.
                     auto runnable = std::make_shared<LambdaRunnable>([&cont]() {
                         cont.resume();
                     });
                     dispatcher->dispatch(*context, runnable);
                } else {
                     // No dispatcher, just resume (no-op yield or thread yield?)
                     // If we are Unconfined, we just run.
                     // But yield is supposed to check cancellation/give up time.
                     // We checked cancellation.
                     // Since we have no dispatcher, we can't really "yield to others" except OS.
                     std::this_thread::yield();
                     cont.resume();
                }
            }, completion.get()));
            
            return nullptr;
        }

        [[suspend]] void yield() {}
    } // namespace coroutines
} // namespace kotlinx