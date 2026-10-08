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
#include <type_traits>
#include <utility>
#include <vector>

namespace kotlinx::coroutines::internal {

// NOTE(port): The C++ callable binding retains the element-text operation only
// when a handler is installed. A null Kotlin callback needs no such operation.
template<typename E>
class UndeliveredElementHandlerBinding {
public:
    UndeliveredElementHandlerBinding() noexcept = default;
    UndeliveredElementHandlerBinding(std::nullptr_t) noexcept {}

    template<typename F>
        requires (!std::is_same_v<std::remove_cvref_t<F>, UndeliveredElementHandlerBinding> &&
                  std::is_invocable_r_v<void, F&, E>)
    UndeliveredElementHandlerBinding(F&& handler)
        : handler_(std::forward<F>(handler)), element_text_(&format_element) {}

    explicit operator bool() const noexcept { return static_cast<bool>(handler_); }
    void operator()(E element) const { handler_(std::move(element)); }
    std::string element_to_string(const E& element) const {
        if (!handler_) throw std::bad_function_call();
        return element_text_(element);
    }

private:
    static std::string format_element(const E& element) {
        std::ostringstream text;
        text << std::boolalpha;
        if constexpr (requires { text << element.to_string(); }) text << element.to_string();
        else if constexpr (requires { text << to_string(element); }) text << to_string(element);
        else if constexpr (requires { text << element; }) text << element;
        else static_assert(sizeof(E) == 0,
            "An undelivered-element handler requires an element string operation");
        return text.str();
    }

    std::function<void(E)> handler_;
    std::string (*element_text_)(const E&) = nullptr;
};

// Transliterated from: kotlinx-coroutines-core/common/src/internal/OnUndeliveredElement.kt:6-6
template<typename E>
using OnUndeliveredElement = UndeliveredElementHandlerBinding<E>;

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
            return new UndeliveredElementException(
                "Exception in undelivered element handler for " + handler.element_to_string(element), exception);
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
