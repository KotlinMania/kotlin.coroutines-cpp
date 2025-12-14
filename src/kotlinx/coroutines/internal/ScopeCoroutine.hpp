#pragma once
#include "kotlinx/coroutines/AbstractCoroutine.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include "kotlinx/coroutines/CompletedValue.hpp"
#include <memory>

namespace kotlinx::coroutines {
    class TimeoutCancellationException;
}

namespace kotlinx {
namespace coroutines {
namespace internal {

/**
 * Kotlin: internal fun <T> recoverResult(state: Any?, uCont: Continuation<T>): Result<T>
 * Converts a JobState to a Result<T> for resumption.
 */
template <typename T>
Result<T> recover_result(JobState* state) {
    if (auto* ex = dynamic_cast<CompletedExceptionally*>(state)) {
        return Result<T>::failure(ex->cause);
    }
    if (auto* completed = dynamic_cast<CompletedValue<T>*>(state)) {
        return Result<T>::success(completed->value);
    }
    // Default case: state is null or unknown type, return default value
    return Result<T>::success(T{});
}

/**
 * This is a coroutine instance that is created by [coroutineScope] builder.
 */
template <typename T>
class ScopeCoroutine : public AbstractCoroutine<T> {
public:
    std::shared_ptr<Continuation<T>> u_cont; // unintercepted continuation

    ScopeCoroutine(std::shared_ptr<CoroutineContext> context, std::shared_ptr<Continuation<T>> uCont)
        : AbstractCoroutine<T>(context, true, true), u_cont(uCont) {}

    bool get_is_scoped_coroutine() const override { return true; }

    void after_completion(JobState* state) override {
        // Resume in a cancellable way by default when resuming from another context
        // Kotlin: uCont.intercepted().resumeCancellableWith(recoverResult(state, uCont))
        if (u_cont) {
            u_cont->resume_with(recover_result<T>(state));
        }
    }

    /**
     * Invoked when a scoped coroutine was completed in an undispatched manner directly
     * at the place of its start because it never suspended.
     */
    virtual void after_completion_undispatched() {
    }

    void after_resume(JobState* state) override {
        // Resume direct because scope is already in the correct context
        // Kotlin: uCont.resumeWith(recoverResult(state, uCont))
        if (u_cont) {
            u_cont->resume_with(recover_result<T>(state));
        }
    }

    // startUndispatchedOrReturnIgnoreTimeout from Undispatched.kt
    void* start_undispatched_or_return_ignore_timeout(
        std::shared_ptr<ScopeCoroutine<T>> receiver,
        std::function<T(CoroutineScope&)> block) {
        // Simplified implementation - TODO: full undispatched logic
        try {
            // Start the coroutine
            auto result = block(*this);
            // If it completes immediately, return the result
            return reinterpret_cast<void*>(&result);
        } catch (const TimeoutCancellationException& e) {
            // Ignore timeout exceptions on fast path
            if (e.coroutine.get() == this) {
                return nullptr; // null result for timeout
            }
            throw;
        }
    }
};

class ContextScope : public CoroutineScope {
    std::shared_ptr<CoroutineContext> context_;
public:
    explicit ContextScope(std::shared_ptr<CoroutineContext> context) : context_(context) {}
    std::shared_ptr<CoroutineContext> get_coroutine_context() const override { return context_; }
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
