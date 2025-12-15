#pragma once
#include "kotlinx/coroutines/AbstractCoroutine.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include "kotlinx/coroutines/CompletedValue.hpp"
#include <memory>
#include <typeinfo>

namespace kotlinx {
namespace coroutines {

// Forward declaration - defined in Timeout.hpp
class TimeoutCancellationException;

/**
 * Checks if the current exception is a TimeoutCancellationException from the given coroutine.
 * Returns true if it IS our own timeout (should NOT rethrow), false otherwise.
 *
 * Kotlin: private fun ScopeCoroutine<*>.notOwnTimeout(cause: Throwable): Boolean =
 *     cause !is TimeoutCancellationException || cause.coroutine !== this
 *
 * This is the inverse: returns true if it IS own timeout.
 * Implemented in Scopes.cpp to avoid circular dependency with Timeout.hpp.
 */
bool is_own_timeout_exception(std::exception_ptr ex, const void* coroutine_ptr);

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

    /**
     * Kotlin: internal fun <T, R> ScopeCoroutine<T>.startUndispatchedOrReturnIgnoreTimeout(...)
     * From Undispatched.kt - starts an undispatched coroutine, ignoring timeout exceptions
     * that originated from this coroutine.
     *
     * @param receiver unused in C++ (Kotlin extension function receiver)
     * @param block the coroutine block to execute
     * @return result pointer, COROUTINE_SUSPENDED, or nullptr on own timeout
     */
    void* start_undispatched_or_return_ignore_timeout(
        std::shared_ptr<ScopeCoroutine<T>> receiver,
        std::function<T(CoroutineScope&)> block) {
        // TODO(port): full undispatched logic with make_completing_once and child waiting
        // Currently simplified: just execute block and handle timeout exceptions
        try {
            auto result = block(*this);
            // Block completed synchronously - return heap-allocated result
            return reinterpret_cast<void*>(new T(std::move(result)));
        } catch (...) {
            auto ex = std::current_exception();
            // Check if this is our own timeout exception (should return null, not rethrow)
            // Kotlin: notOwnTimeout returns true if should rethrow, false if own timeout
            if (is_own_timeout_exception(ex, this)) {
                // Own timeout - return nullptr (null result for with_timeout_or_null)
                return nullptr;
            }
            // Not our timeout or different exception - rethrow
            std::rethrow_exception(ex);
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
