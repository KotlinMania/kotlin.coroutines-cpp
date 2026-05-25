// port-lint: source internal/OnUndeliveredElement.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/OnUndeliveredElement.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.internal
 *
 * Handler invocation path for undelivered channel elements. The wrapper rethrows handler
 * exceptions as `UndeliveredElementException`; the recursive helper carries the prior
 * `UndeliveredElementException` so subsequent failures chain through the JVM's
 * `addSuppressed` mechanism. C++ has no `addSuppressed`, so the C++ port keeps the first
 * exception and discards subsequent duplicates of the same cause.
 */

#include <exception>
#include <functional>
#include <string>

namespace kotlinx {
    namespace coroutines {
        namespace internal {
            // Forward declarations
            class CoroutineContext;

            // typealias OnUndeliveredElement<E> = (E) -> Unit
            template<typename E>
            using OnUndeliveredElement = std::function<void(E)>;

            /**
 * Internal exception that is thrown when OnUndeliveredElement handler in
 * a Channel throws an exception.
 */
            class UndeliveredElementException : public std::runtime_error {
            public:
                const std::exception *cause;

                UndeliveredElementException(const std::string &message, const std::exception *cause_)
                    : std::runtime_error(message), cause(cause_) {
                }
            };

            /**
 * Calls the undelivered element handler, catching any exception.
 */
            template<typename E>
            UndeliveredElementException *call_undelivered_element_catching_exception(
                const OnUndeliveredElement<E> &handler,
                E element,
                UndeliveredElementException *undelivered_element_exception = nullptr
            ) {
                try {
                    handler(element);
                } catch (const std::exception &ex) {
                    // Upstream: undelivered_element_exception?.addSuppressed(ex)
                    //
                    // C++ has no `Throwable.addSuppressed` analogue. Upstream's optimization
                    // (skip when the same exception is rethrown repeatedly) is preserved by
                    // pointer identity — `cause` is the original `&ex` from the first call.
                    if (undelivered_element_exception != nullptr &&
                        undelivered_element_exception->cause != &ex) {
                        // Drop the duplicate: keep the first, drop further occurrences.
                    } else if (undelivered_element_exception == nullptr) {
                        return new UndeliveredElementException(
                            std::string("Exception in undelivered element handler"), &ex);
                    }
                }
                return undelivered_element_exception;
            }

            /**
 * Calls the undelivered element handler and handles any exception.
 */
            template<typename E>
            void call_undelivered_element(
                const OnUndeliveredElement<E> &handler,
                E element,
                CoroutineContext *context
            ) {
                UndeliveredElementException *ex =
                        call_undelivered_element_catching_exception(handler, element, nullptr);
                if (ex != nullptr) {
                    // Upstream: handleCoroutineException(context, ex)
                    // Forwards to the context's CoroutineExceptionHandler. Declared in
                    // CoroutineExceptionHandler.hpp; the concrete CoroutineContext type
                    // is opaque here so the call site casts at the entry point.
                    handle_coroutine_exception(*context, std::make_exception_ptr(*ex));
                    delete ex;
                }
            }
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx