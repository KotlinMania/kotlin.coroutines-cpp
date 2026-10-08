/**
 * Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt
 */

#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/Dispatchers.hpp"
#include "kotlinx/coroutines/native/MultithreadedDispatchers.hpp"
#include "kotlinx/coroutines/context_impl.hpp"

namespace kotlinx {
    namespace coroutines {
        // Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:32-36
        std::shared_ptr<CoroutineContext> new_coroutine_context(
            CoroutineScope* scope, std::shared_ptr<CoroutineContext> context) {
            auto combined = scope->get_coroutine_context()->operator+(std::move(context));
            auto default_dispatcher = std::shared_ptr<CoroutineContext>(
                &Dispatchers::get_default(), [](CoroutineContext*) {});
            return combined != default_dispatcher && !combined->get(ContinuationInterceptor::type_key)
                ? combined->operator+(std::move(default_dispatcher)) : combined;
        }

        // Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:38-40
        std::shared_ptr<CoroutineContext> new_coroutine_context(
            std::shared_ptr<CoroutineContext> base_context,
            std::shared_ptr<CoroutineContext> added_context) {
            return base_context->operator+(std::move(added_context));
        }

        // No debugging facilities on Native.
        // Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:46-46
        std::optional<std::string> coroutine_name(const std::shared_ptr<CoroutineContext>&) {
            return std::nullopt;
        }

        namespace {
            // Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:6-24
            class DefaultExecutor : public CoroutineDispatcher, public Delay {
            public:
                // Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:8
                DefaultExecutor() : delegate_("DefaultExecutor") {}

                // Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:10-12
                void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
                    delegate_.dispatch(context, std::move(block));
                }

                // Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:14-16
                void schedule_resume_after_delay(
                    long long time_millis, CancellableContinuation<void>& continuation) override {
                    delegate_.schedule_resume_after_delay(time_millis, continuation);
                }

                // Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:18-20
                std::shared_ptr<DisposableHandle> invoke_on_timeout(
                    long long time_millis, std::shared_ptr<Runnable> block,
                    const CoroutineContext& context) override {
                    return delegate_.invoke_on_timeout(time_millis, std::move(block), context);
                }

                // Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:22-24
                void enqueue(std::shared_ptr<Runnable> task) {
                    delegate_.dispatch(*EmptyCoroutineContext::instance(), std::move(task));
                }
            private:
                WorkerDispatcher delegate_;
            };

            // Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:6-8
            // NOTE(port): C++ function-local storage implements the source object.
            DefaultExecutor& default_executor() {
                static DefaultExecutor instance;
                return instance;
            }
        } // namespace

        // Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:30
        Delay& get_default_delay() {
            return default_executor();
        }
    } // namespace coroutines
} // namespace kotlinx
