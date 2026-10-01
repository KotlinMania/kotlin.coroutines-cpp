/**
 * @file CoroutineDispatcher.cpp
 * @brief Implementation of CoroutineDispatcher and its helpers.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/CoroutineDispatcher.hpp`.
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
        // CoroutineDispatcher implementation

        CoroutineDispatcher::CoroutineDispatcher() : AbstractCoroutineContextElement(ContinuationInterceptor::type_key) {
        }

        bool CoroutineDispatcher::is_dispatch_needed(const CoroutineContext& context) const {
            // Base implementation always returns true - context exists for subclasses
            // that might check if they're already on the correct thread/executor
            return true;
        }

        void CoroutineDispatcher::dispatch_yield(const CoroutineContext &context,
                                                 std::shared_ptr<Runnable> block) const {
            dispatch(context, block);
        }

        // Template method intercept_continuation is in header

        // Explicit instantiation for common types if needed, or keep in header if possible.
        // But we defined it in header as template.

        void CoroutineDispatcher::release_intercepted_continuation(std::shared_ptr<ContinuationBase> continuation) {
            auto dispatched = std::dynamic_pointer_cast<DispatchedContinuationBase>(continuation);
            if (dispatched) {
                dispatched->release();
            }
        }

        std::shared_ptr<CoroutineDispatcher> CoroutineDispatcher::limited_parallelism(
            int parallelism, const std::string &name) {
            internal::check_parallelism(parallelism);
            return std::make_shared<internal::LimitedDispatcher>(
                std::dynamic_pointer_cast<CoroutineDispatcher>(shared_from_this()), parallelism, name);
        }

        std::string CoroutineDispatcher::to_string() const {
            return "CoroutineDispatcher";
        }
    } // namespace coroutines
} // namespace kotlinx
