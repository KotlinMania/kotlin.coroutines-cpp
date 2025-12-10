#include "kotlinx/coroutines/CoroutineExceptionHandler.hpp"
#include <iostream>

namespace kotlinx {
namespace coroutines {

void handle_coroutine_exception(CoroutineContext& context, std::exception_ptr exception) {
    if (!exception) return;

    try {
        auto element = context.get(CoroutineExceptionHandler::typeKey);
        if (auto handler = std::dynamic_pointer_cast<CoroutineExceptionHandler>(element)) {
            handler->handle_exception(context, exception);
            return;
        }
    } catch (...) {
        // Fallback to global handler if handler throws
        exception = std::current_exception(); // Update exception? Or just log both?
    }

    // fallback global handler
    try {
        std::rethrow_exception(exception);
    } catch (const std::exception& e) {
        std::cerr << "Uncaught coroutine exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Uncaught coroutine exception: unknown" << std::endl;
    }
}

} // namespace coroutines
} // namespace kotlinx