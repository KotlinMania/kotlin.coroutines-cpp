// port-lint: source kotlinx-coroutines-core/common/src/CoroutineDispatcher.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/CoroutineDispatcher.kt
 * @file CoroutineDispatcher.cpp
 * @brief Implementation of CoroutineDispatcher and its helpers.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `src/kotlinx/coroutines/CoroutineDispatcher.hpp`.
 *
 * This file contains the implementation of:
 * - `LimitedDispatcher` (internal helper for limited parallelism)
 * - `CoroutineDispatcher::limited_parallelism`
 * - `CoroutineDispatcher::intercept_continuation` logic
 */

#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
// kotlinx.coroutines.internal.* (from Kotlin)
#include "kotlinx/coroutines/internal/DispatchedContinuation.hpp"
#include "kotlinx/coroutines/internal/DispatchedTask.hpp"
#include "kotlinx/coroutines/internal/Symbol.hpp"
#include "kotlinx/coroutines/internal/LimitedDispatcher.hpp"

namespace kotlinx {
    namespace coroutines {
        // Transliterated from: kotlinx-coroutines-core/common/src/CoroutineDispatcher.kt:65-67
        CoroutineDispatcher::Key::Key()
            : AbstractCoroutineContextKey(&ContinuationInterceptor::key_instance,
                [](std::shared_ptr<CoroutineContext::Element> element) {
                    return std::dynamic_pointer_cast<CoroutineDispatcher>(element);
                }) {}
        // Transliterated from: kotlinx-coroutines-core/common/src/CoroutineDispatcher.kt:65-67
        CoroutineDispatcher::Key CoroutineDispatcher::KEY;

        // CoroutineDispatcher implementation

        CoroutineDispatcher::CoroutineDispatcher() : AbstractCoroutineContextElement(ContinuationInterceptor::type_key) {
        }

        bool CoroutineDispatcher::is_dispatch_needed(const CoroutineContext& context) const {
            // Base implementation always returns true - context exists for subclasses
            // that might check if they're already on the correct thread/executor
            return true;
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/CoroutineDispatcher.kt:231-232
        void CoroutineDispatcher::dispatch_yield(const CoroutineContext &context,
                                                 std::shared_ptr<Runnable> block) const {
            internal::safe_dispatch(*this, context, std::move(block));
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/CoroutineDispatcher.kt:240-241
        std::shared_ptr<Continuation<void*>> CoroutineDispatcher::intercept_continuation(
            std::shared_ptr<Continuation<void*>> continuation) {
            return intercept_continuation<void*>(std::move(continuation));
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/CoroutineDispatcher.kt:243-250
        void CoroutineDispatcher::release_intercepted_continuation(std::shared_ptr<Continuation<void*>> continuation) {
            auto dispatched = std::dynamic_pointer_cast<DispatchedContinuationBase>(continuation);
            if (dispatched) {
                dispatched->release();
            }
        }

        std::shared_ptr<CoroutineDispatcher> CoroutineDispatcher::limited_parallelism(
            int parallelism, const std::string &name) {
            internal::check_parallelism(parallelism);
            return std::shared_ptr<CoroutineDispatcher>(new internal::LimitedDispatcher(
                std::dynamic_pointer_cast<CoroutineDispatcher>(shared_from_this()), parallelism, name));
        }

        std::string CoroutineDispatcher::to_string() const {
            return "CoroutineDispatcher";
        }
    } // namespace coroutines
} // namespace kotlinx
