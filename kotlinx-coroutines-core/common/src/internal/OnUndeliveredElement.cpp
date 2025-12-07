// Transliterated from Kotlin to C++
// Original: kotlinx-coroutines-core/common/src/internal/OnUndeliveredElement.kt
//
// TODO: This is a mechanical transliteration - semantics not fully implemented
// TODO: CoroutineContext, Throwable need C++ equivalents
// TODO: typealias needs using declaration
// TODO: Extension functions need free function implementations

#include <functional>
#include <exception>
#include <string>

namespace kotlinx {
namespace coroutines {
namespace internal {

// Forward declarations
class CoroutineContext;
class UndeliveredElementException;

// typealias OnUndeliveredElement<E> = (E) -> Unit
template<typename E>
using OnUndeliveredElement = std::function<void(E)>;

template<typename E>
UndeliveredElementException* call_undelivered_element_catching_exception(
    const OnUndeliveredElement<E>& handler,
    E element,
    UndeliveredElementException* undelivered_element_exception = nullptr
) {
    try {
        handler(element);
    } catch (const std::exception& ex) {
        // undeliveredElementException.cause !== ex is an optimization in case the same exception is thrown
        // over and over again by on OnUndeliveredElement
        if (undelivered_element_exception != nullptr /* && undelivered_element_exception->cause != &ex */) {
            // TODO: undelivered_element_exception->add_suppressed(ex);
        } else {
            // TODO: proper string conversion for element
            return new UndeliveredElementException("Exception in undelivered element handler", &ex);
        }
    }
    return undelivered_element_exception;
}

template<typename E>
void call_undelivered_element(
    const OnUndeliveredElement<E>& handler,
    E element,
    CoroutineContext* context
) {
    UndeliveredElementException* ex = call_undelivered_element_catching_exception(handler, element, nullptr);
    if (ex != nullptr) {
        // TODO: handle_coroutine_exception(context, ex);
    }
}

/**
 * Internal exception that is thrown when [OnUndeliveredElement] handler in
 * a [kotlinx.coroutines.channels.Channel] throws an exception.
 */
class UndeliveredElementException : public std::runtime_error {
public:
    const std::exception* cause;

    UndeliveredElementException(const std::string& message, const std::exception* cause)
        : std::runtime_error(message), cause(cause) {}
};

} // namespace internal
} // namespace coroutines
} // namespace kotlinx
