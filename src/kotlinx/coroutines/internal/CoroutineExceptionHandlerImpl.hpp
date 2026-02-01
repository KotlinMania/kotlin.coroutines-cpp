#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/internal/CoroutineExceptionHandlerImpl.common.kt
/**
 * @file CoroutineExceptionHandlerImpl.hpp
 * @brief Internal implementation of coroutine exception handling.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/CoroutineExceptionHandlerImpl.common.kt
 *
 * This header provides the common exception handling infrastructure, including
 * platform exception handlers, global exception propagation, and diagnostic context exceptions.
 */

#include "kotlinx/coroutines/CoroutineExceptionHandler.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include <exception>
#include <vector>
#include <mutex>
#include <iostream>

namespace kotlinx {
namespace coroutines {
namespace internal {

/**
 * A dummy exception that signifies that the exception was successfully processed by the handler
 * and no further action is required.
 *
 * Would be nicer if CoroutineExceptionHandler could return a boolean, but that would be
 * a breaking change. For now, we will take solace in knowledge that such exceptions are
 * exceedingly rare, even rarer than globally uncaught exceptions in general.
 *
 * Transliterated from:
 * internal object ExceptionSuccessfullyProcessed : Exception()
 */
class ExceptionSuccessfullyProcessed : public std::exception {
public:
    const char* what() const noexcept override {
        return "ExceptionSuccessfullyProcessed";
    }
};

/**
 * Private exception that is added to suppressed exceptions of the original exception
 * when it is reported to the last-ditch current thread 'uncaughtExceptionHandler'.
 *
 * The purpose of this exception is to add an otherwise inaccessible diagnostic information
 * and to be able to poke the context of the failing coroutine in the debugger.
 *
 * Transliterated from:
 * internal expect class DiagnosticCoroutineContextException(context: CoroutineContext) : RuntimeException
 */
class DiagnosticCoroutineContextException : public std::runtime_error {
private:
    std::shared_ptr<CoroutineContext> context_;

public:
    explicit DiagnosticCoroutineContextException(std::shared_ptr<CoroutineContext> context)
        : std::runtime_error("Coroutine context at exception"),
          context_(context) {}

    std::shared_ptr<CoroutineContext> get_context() const { return context_; }
};

/**
 * The list of globally installed CoroutineExceptionHandler instances that will be notified
 * of any exceptions that were not processed in any other manner.
 *
 * Transliterated from:
 * internal expect val platformExceptionHandlers: Collection<CoroutineExceptionHandler>
 */
inline std::vector<std::shared_ptr<CoroutineExceptionHandler>>& platform_exception_handlers() {
    static std::vector<std::shared_ptr<CoroutineExceptionHandler>> handlers;
    return handlers;
}

/**
 * Mutex for thread-safe access to platform exception handlers.
 */
inline std::mutex& platform_handlers_mutex() {
    static std::mutex mtx;
    return mtx;
}

/**
 * Ensures that the given callback is present in the platformExceptionHandlers list.
 *
 * Transliterated from:
 * internal expect fun ensurePlatformExceptionHandlerLoaded(callback: CoroutineExceptionHandler)
 */
inline void ensure_platform_exception_handler_loaded(std::shared_ptr<CoroutineExceptionHandler> callback) {
    std::lock_guard<std::mutex> lock(platform_handlers_mutex());
    auto& handlers = platform_exception_handlers();
    // Check if already present
    for (const auto& h : handlers) {
        if (h.get() == callback.get()) return;
    }
    handlers.push_back(callback);
}

/**
 * The platform-dependent global exception handler, used so that the exception is logged
 * at least *somewhere*.
 *
 * Transliterated from:
 * internal expect fun propagateExceptionFinalResort(exception: Throwable)
 */
inline void propagate_exception_final_resort(std::exception_ptr exception) {
    // Native platform implementation: print to stderr
    try {
        if (exception) {
            std::rethrow_exception(exception);
        }
    } catch (const std::exception& e) {
        std::cerr << "Unhandled coroutine exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unhandled coroutine exception: unknown" << std::endl;
    }
}

/**
 * Creates a combined exception when a handler itself throws.
 *
 * Transliterated from: handlerException(exception, t) usage in handleUncaughtCoroutineException
 */
inline std::exception_ptr handler_exception(std::exception_ptr original, std::exception_ptr handler_failure) {
    // In Kotlin, this adds the handler exception as suppressed.
    // In C++, we just return the original for simplicity.
    // TODO: Implement proper exception chaining if needed.
    (void)handler_failure;
    return original;
}

/**
 * Deal with exceptions that happened in coroutines and weren't programmatically dealt with.
 *
 * First, it notifies every CoroutineExceptionHandler in the platformExceptionHandlers list.
 * If one of them throws ExceptionSuccessfullyProcessed, it means that that handler believes
 * that the exception was dealt with sufficiently well and doesn't need any further processing.
 * Otherwise, the platform-dependent global exception handler is also invoked.
 *
 * Transliterated from:
 * internal fun handleUncaughtCoroutineException(context: CoroutineContext, exception: Throwable)
 */
inline void handle_uncaught_coroutine_exception(
    std::shared_ptr<CoroutineContext> context,
    std::exception_ptr exception
) {
    // Use additional extension handlers
    std::lock_guard<std::mutex> lock(platform_handlers_mutex());
    for (const auto& handler : platform_exception_handlers()) {
        try {
            // Note: Need a mutable context reference for handle_exception
            if (context) {
                handler->handle_exception(*context, exception);
            }
        } catch (const ExceptionSuccessfullyProcessed&) {
            return;  // Exception was handled
        } catch (...) {
            propagate_exception_final_resort(
                handler_exception(exception, std::current_exception())
            );
        }
    }

    // Add diagnostic context as "suppressed" exception (logged)
    // In C++, we can't add suppressed exceptions to std::exception,
    // so we just note that context info is available.
    if (context) {
        try {
            std::cerr << "Coroutine context at failure: [CoroutineContext available]" << std::endl;
        } catch (...) {
            // Ignore errors in context stringification
        }
    }

    propagate_exception_final_resort(exception);
}

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
