// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/CoroutineExceptionHandlerImpl.common.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: expect declarations need platform-specific implementations
// TODO: CoroutineContext, CoroutineExceptionHandler, Throwable need C++ equivalents
// TODO: Exception handling mechanisms need proper C++ design
// TODO: Collection type needs std::vector or similar

#include <vector>
#include <stdexcept>

namespace kotlinx {
namespace coroutines {
namespace internal {

// Forward declarations
class CoroutineExceptionHandler;
class CoroutineContext;
class DiagnosticCoroutineContextException;

/**
 * The list of globally installed [CoroutineExceptionHandler] instances that will be notified of any exceptions that
 * were not processed in any other manner.
 */
// TODO: expect val - needs platform-specific implementation
extern std::vector<CoroutineExceptionHandler*> platform_exception_handlers;

/**
 * Ensures that the given [callback] is present in the [platformExceptionHandlers] list.
 */
// TODO: expect function - needs platform-specific implementation
void ensure_platform_exception_handler_loaded(CoroutineExceptionHandler* callback);

/**
 * The platform-dependent global exception handler, used so that the exception is logged at least *somewhere*.
 */
// TODO: expect function - needs platform-specific implementation
void propagate_exception_final_resort(std::exception* exception);

/**
 * Deal with exceptions that happened in coroutines and weren't programmatically dealt with.
 *
 * First, it notifies every [CoroutineExceptionHandler] in the [platformExceptionHandlers] list.
 * If one of them throws [ExceptionSuccessfullyProcessed], it means that that handler believes that the exception was
 * dealt with sufficiently well and doesn't need any further processing.
 * Otherwise, the platform-dependent global exception handler is also invoked.
 */
void handle_uncaught_coroutine_exception(CoroutineContext* context, std::exception* exception) {
    // use additional extension handlers
    for (auto handler : platform_exception_handlers) {
        try {
            // TODO: handler.handleException needs proper implementation
            // handler->handle_exception(context, exception);
        } catch (const ExceptionSuccessfullyProcessed&) {
            return;
        } catch (const std::exception& t) {
            // TODO: handlerException function needs implementation
            // propagate_exception_final_resort(handler_exception(exception, t));
        }
    }

    try {
        // TODO: exception.addSuppressed equivalent in C++
        // exception->add_suppressed(new DiagnosticCoroutineContextException(context));
    } catch (const std::exception& e) {
        // addSuppressed is never user-defined and cannot normally throw with the only exception being OOM
        // we do ignore that just in case to definitely deliver the exception
    }
    propagate_exception_final_resort(exception);
}

/**
 * Private exception that is added to suppressed exceptions of the original exception
 * when it is reported to the last-ditch current thread 'uncaughtExceptionHandler'.
 *
 * The purpose of this exception is to add an otherwise inaccessible diagnostic information and to
 * be able to poke the context of the failing coroutine in the debugger.
 */
// TODO: expect class - needs platform-specific implementation
class DiagnosticCoroutineContextException : public std::runtime_error {
public:
    explicit DiagnosticCoroutineContextException(CoroutineContext* context)
        : std::runtime_error("DiagnosticCoroutineContextException") {}
};

/**
 * A dummy exception that signifies that the exception was successfully processed by the handler and no further
 * action is required.
 *
 * Would be nicer if [CoroutineExceptionHandler] could return a boolean, but that would be a breaking change.
 * For now, we will take solace in knowledge that such exceptions are exceedingly rare, even rarer than globally
 * uncaught exceptions in general.
 */
// TODO: object in Kotlin becomes singleton in C++
class ExceptionSuccessfullyProcessed : public std::exception {
private:
    ExceptionSuccessfullyProcessed() = default;
public:
    static ExceptionSuccessfullyProcessed& instance() {
        static ExceptionSuccessfullyProcessed inst;
        return inst;
    }
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
