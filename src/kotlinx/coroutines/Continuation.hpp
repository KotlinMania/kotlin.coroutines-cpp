#pragma once
/**
 * @file Continuation.hpp
 * @brief Continuation interface for kotlinx.coroutines
 *
 * This file provides the Continuation<T> interface that matches Kotlin's
 * kotlin.coroutines.Continuation interface.
 */

#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Result.hpp"
#include <memory>
#include <functional>

namespace kotlinx {
namespace coroutines {

// Legacy compatibility - ContinuationBase for older code
class ContinuationBase {
public:
    virtual ~ContinuationBase() = default;
};

/**
 * @brief Interface representing a continuation after a suspension point.
 *
 * This interface mirrors Kotlin's kotlin.coroutines.Continuation<in T> and provides
 * the fundamental abstraction for coroutine resumption. A continuation represents
 * the rest of the computation after a suspension point and carries the coroutine
 * context along with the resumption capability.
 *
 * The continuation is the core primitive that enables structured concurrency and
 * proper coroutine lifecycle management. It provides type-safe resumption with
 * either successful results or failures.
 *
 * ## Implementation Notes
 *
 * - Continuations are typically created by the coroutine compiler infrastructure
 * - The context provides access to cancellation, dispatchers, and other coroutine elements
 * - Resume operations should be thread-safe and handle concurrent calls appropriately
 * - The Result<T> type encapsulates both success and failure cases
 *
 * ## Memory Management
 *
 * Continuations are managed through std::shared_ptr to ensure proper lifetime
 * across asynchronous operations. The reference counting model allows safe
 * sharing between threads and coroutine components.
 *
 * @tparam T The type of value that this continuation expects to receive upon resumption.
 *           Use void for continuations that don't return a value.
 *
 * @note This is a low-level interface typically used by coroutine infrastructure
 *       and suspend function implementations. Application code usually uses
 *       higher-level abstractions like CancellableContinuation.
 */
template<typename T>
class Continuation {
public:
    virtual ~Continuation() = default;

    /**
     * @brief Returns the coroutine context associated with this continuation.
     *
     * The context provides access to the coroutine's execution environment,
     * including cancellation information, dispatchers, and other contextual
     * elements. The context is immutable for the lifetime of the continuation.
     *
     * @return std::shared_ptr<CoroutineContext> The coroutine context
     *
     * @note The context is guaranteed to be non-null and contains at minimum
     *       the Job that represents this coroutine's lifecycle.
     */
    virtual std::shared_ptr<CoroutineContext> get_context() const = 0;

    /**
     * @brief Resumes the execution of the corresponding coroutine with a result.
     *
     * This method resumes the coroutine's execution after a suspension point,
     * passing either a successful result or an exception. The resume operation
     * is typically dispatched according to the coroutine's dispatcher.
     *
     * @param result The Result<T> containing either the successful value or
     *               an exception to be thrown in the coroutine
     *
     * @thread_safe This method must be thread-safe as it can be called from
     *              different threads concurrently. Implementations should handle
     *              multiple resume attempts appropriately.
     *
     * @note Multiple calls to resume_with may result in undefined behavior.
     *       Implementations should ensure exactly-once resumption semantics.
     */
    virtual void resume_with(Result<T> result) = 0;
};

/**
 * Resumes the execution of the corresponding coroutine passing [value] as the return value.
 */
template<typename T>
inline void resume(Continuation<T>& continuation, T value) {
    continuation.resume_with(Result<T>::success(std::move(value)));
}

/**
 * Specialization for void - resume with Unit.
 */
inline void resume(Continuation<void>& continuation) {
    continuation.resume_with(Result<void>::success());
}

/**
 * Resumes with an exception.
 */
template<typename T>
inline void resume_with_exception(Continuation<T>& continuation, std::exception_ptr exception) {
    continuation.resume_with(Result<T>::failure(exception));
}

/**
 * @brief Functional implementation of Continuation interface.
 *
 * This class provides a concrete implementation of Continuation<T> that delegates
 * resumption to a std::function. It's useful for testing, interop with callback-based
 * APIs, and situations where a lambda-based continuation is more convenient than
 * a full class implementation.
 *
 * ## Usage Example
 *
 * ```cpp
 * auto continuation = make_continuation<int>(context, [](Result<int> result) {
 *     if (result.is_success()) {
 *         std::cout << "Got value: " << result.get_or_throw() << std::endl;
 *     } else {
 *         std::cout << "Got error: " << result.exception_or_null() << std::endl;
 *     }
 * });
 * ```
 *
 * @tparam T The result type of the continuation
 */
template<typename T>
class FunctionalContinuation : public Continuation<T> {
public:
    /**
     * @brief Constructs a functional continuation with the given context and resume function.
     *
     * @param context The coroutine context to return from get_context()
     * @param resume_withFn The function to call when resume_with() is invoked
     */
    FunctionalContinuation(
        std::shared_ptr<CoroutineContext> context,
        std::function<void(Result<T>)> resume_withFn
    ) : context_(std::move(context)), resume_with_fn_(std::move(resume_withFn)) {}

    /**
     * @brief Returns the stored coroutine context.
     */
    std::shared_ptr<CoroutineContext> get_context() const override {
        return context_;
    }

    /**
     * @brief Invokes the stored resume function with the given result.
     *
     * This method simply forwards the call to the resume_with_fn_ that was
     * provided during construction.
     *
     * @param result The result to pass to the resume function
     */
    void resume_with(Result<T> result) override {
        resume_with_fn_(std::move(result));
    }

private:
    std::shared_ptr<CoroutineContext> context_;
    std::function<void(Result<T>)> resume_with_fn_;
};

/**
 * @brief Factory function for creating functional continuations.
 *
 * This convenience function creates a FunctionalContinuation with the given
 * context and resume function, returning it as a shared_ptr to the base
 * Continuation interface.
 *
 * @tparam T The result type of the continuation
 * @param context The coroutine context to associate with the continuation
 * @param resume_withFn The function to invoke when the continuation is resumed
 *
 * @return std::shared_ptr<Continuation<T>> A shared pointer to the created continuation
 *
 * @example
 * ```cpp
 * auto continuation = make_continuation<std::string>(context, [](Result<std::string> result) {
 *     // Handle the result
 * });
 * ```
 */
template<typename T>
std::shared_ptr<Continuation<T>> make_continuation(
    std::shared_ptr<CoroutineContext> context,
    std::function<void(Result<T>)> resume_withFn
) {
    return std::make_shared<FunctionalContinuation<T>>(
        std::move(context),
        std::move(resume_withFn)
    );
}

/**
 * Empty coroutine context singleton.
 */
// EmptyCoroutineContext moved to context_impl.hpp

} // namespace coroutines
} // namespace kotlinx
