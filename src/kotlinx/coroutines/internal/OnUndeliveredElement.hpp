#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/OnUndeliveredElement.kt
 *
 * Handler invocation path for undelivered channel elements.
 */

#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/CoroutineExceptionHandler.hpp"
#include <exception>
#include <functional>
#include <string>

namespace kotlinx {
    namespace coroutines {
        namespace internal {

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
                    if (undelivered_element_exception != nullptr &&
                        undelivered_element_exception->cause != &ex) {
                        // Drop duplicate
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
                    if (context) {
                        handle_coroutine_exception(*context, std::make_exception_ptr(*ex));
                    }
                    delete ex;
                }
            }
        } // namespace internal
    } // namespace coroutines
} // namespace kotlinx
