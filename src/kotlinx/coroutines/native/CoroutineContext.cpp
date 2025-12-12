/**
 * @file CoroutineContext.cpp
 * @brief Native platform implementation of coroutine context
 *
 * Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt
 *
 * Platform-specific (native) implementation of DefaultExecutor and coroutine context utilities.
 *
 * TODO:
 * - TODO(port): implement WorkerDispatcher parity for native threading
 * - TODO(semantics): DefaultExecutor scheduling parity vs Kotlin/Native Worker
 * - TODO(perf): replace detached-thread timers with shared event loop
 */

#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/Dispatchers.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include <thread>
#include <chrono>
#include <atomic>

namespace kotlinx {
    namespace coroutines {
        namespace {
            /**
             * Internal DefaultExecutor singleton.
             *
             * Kotlin source: kotlinx-coroutines-core/native/src/CoroutineContext.kt
             * `internal actual object DefaultExecutor : CoroutineDispatcher(), Delay`
             */
            class DefaultExecutor : public CoroutineDispatcher, public Delay {
            public:
                DefaultExecutor() = default;

                void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
                    // Kotlin: delegate.dispatch(context, block)
                    Dispatchers::get_default().dispatch(context, std::move(block));
                }

                void schedule_resume_after_delay(
                    long long time_millis,
                    CancellableContinuation<void>& continuation) override {
                    if (time_millis <= 0) {
                        continuation.resume(nullptr);
                        return;
                    }

                    // Fallback timer until WorkerDispatcher is ported.
                    auto* impl_ptr = dynamic_cast<CancellableContinuationImpl<void>*>(&continuation);
                    if (!impl_ptr) {
                        // TODO(semantics): capture shared continuation handle parity.
                        continuation.resume(nullptr);
                        return;
                    }

                    auto shared_impl = impl_ptr->shared_from_this();
                    std::thread([time_millis, shared_impl]() {
                        std::this_thread::sleep_for(std::chrono::milliseconds(time_millis));
                        shared_impl->resume(nullptr);
                    }).detach();
                }

                std::shared_ptr<DisposableHandle> invoke_on_timeout(
                    long long time_millis,
                    std::shared_ptr<Runnable> block,
                    const CoroutineContext& context) override {
                    if (!block) {
                        return std::shared_ptr<DisposableHandle>(NoOpDisposableHandle::instance(), [](DisposableHandle*){});
                    }

                    if (time_millis <= 0) {
                        block->run();
                        return std::shared_ptr<DisposableHandle>(NoOpDisposableHandle::instance(), [](DisposableHandle*){});
                    }

                    struct TimeoutHandle : public DisposableHandle {
                        std::shared_ptr<std::atomic<bool>> cancelled;
                        explicit TimeoutHandle(std::shared_ptr<std::atomic<bool>> c) : cancelled(std::move(c)) {}
                        void dispose() override { cancelled->store(true, std::memory_order_relaxed); }
                    };

                    auto cancelled = std::make_shared<std::atomic<bool>>(false);
                    std::thread([time_millis, block, cancelled]() {
                        std::this_thread::sleep_for(std::chrono::milliseconds(time_millis));
                        if (!cancelled->load(std::memory_order_relaxed)) {
                            block->run();
                        }
                    }).detach();

                    // TODO(perf): timer wheel/heap instead of detached thread.
                    return std::make_shared<TimeoutHandle>(cancelled);
                }

                void enqueue(std::shared_ptr<Runnable> task) {
                    // Kotlin: delegate.dispatch(EmptyCoroutineContext, task)
                    dispatch(*EmptyCoroutineContext::instance(), std::move(task));
                }
            };

            DefaultExecutor& default_executor() {
                static DefaultExecutor instance;
                return instance;
            }
        } // namespace

        Delay& get_default_delay() {
            // Kotlin: internal actual val DefaultDelay: Delay = DefaultExecutor
            return default_executor();
        }
    } // namespace coroutines
} // namespace kotlinx
