#pragma once
// port-lint: source internal/OnUndeliveredElement.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/internal/OnUndeliveredElement.kt
 */
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/CoroutineExceptionHandler.hpp"
#include <exception>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace kotlinx::coroutines::internal {

// Transliterated from: kotlinx-coroutines-core/common/src/internal/OnUndeliveredElement.kt:6-6
template<typename E>
using OnUndeliveredElement = std::function<void(E)>;

/** Internal exception thrown when an undelivered-element handler throws. */
// Transliterated from: kotlinx-coroutines-core/common/src/internal/OnUndeliveredElement.kt:32-36
class UndeliveredElementException : public std::runtime_error {
public:
    UndeliveredElementException(const std::string& message, std::exception_ptr cause);
    std::exception_ptr cause() const noexcept;
    void add_suppressed(std::exception_ptr exception);
    const std::vector<std::exception_ptr>& suppressed_exceptions() const noexcept;
private:
    // NOTE(port): std::exception_ptr retains the actual Throwable carrier and its identity.
    std::exception_ptr cause_;
    std::vector<std::exception_ptr> suppressed_exceptions_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/internal/OnUndeliveredElement.kt:8-24
template<typename E>
UndeliveredElementException* call_undelivered_element_catching_exception(
    const OnUndeliveredElement<E>& handler,
    E element,
    UndeliveredElementException* undelivered_element_exception = nullptr
) {
    try {
        handler(element);
    } catch (...) {
        auto exception = std::current_exception();
        if (undelivered_element_exception && undelivered_element_exception->cause() != exception) {
            undelivered_element_exception->add_suppressed(exception);
        } else {
            // NOTE(port): Standalone value carriers supply their C++ string representation.
            std::ostringstream message;
            message << std::boolalpha << "Exception in undelivered element handler for ";
            if constexpr (requires { element.to_string(); }) message << element.to_string();
            else message << element;
            return new UndeliveredElementException(message.str(), exception);
        }
    }
    return undelivered_element_exception;
}

// Transliterated from: kotlinx-coroutines-core/common/src/internal/OnUndeliveredElement.kt:26-30
template<typename E>
void call_undelivered_element(
    const OnUndeliveredElement<E>& handler,
    E element,
    CoroutineContext& context
) {
    std::unique_ptr<UndeliveredElementException> exception(
        call_undelivered_element_catching_exception(handler, std::move(element)));
    if (exception) handle_coroutine_exception(context, std::make_exception_ptr(*exception));
}

} // namespace kotlinx::coroutines::internal
