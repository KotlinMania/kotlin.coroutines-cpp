/**
 * @file CoroutineExceptionHandlerImpl.common.cpp
 * @brief Implementation of CoroutineExceptionHandler infrastructure.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/CoroutineExceptionHandler.hpp`.
 */

#include "kotlinx/coroutines/CoroutineExceptionHandler.hpp"
#include <iostream>

namespace kotlinx {
namespace coroutines {

void handle_coroutine_exception(CoroutineContext& context, std::exception_ptr exception) {
    // Basic implementation: print to stderr if no handler found or as a fallback
    try {
         // TODO: Look up CoroutineExceptionHandler in context and invoke it
         // auto* handler = context[CoroutineExceptionHandler::Key];
         // if (handler) { handler->handle_exception(context, exception); return; }
    } catch (...) {
        // Ignore errors during exception handling
    }

    // Fallback: print to stderr
    try {
        if (exception) {
            std::rethrow_exception(exception);
        }
    } catch (const std::exception& e) {
        std::cerr << "Uncaught coroutine exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Uncaught coroutine exception (unknown type)" << std::endl;
    }
}

} // namespace coroutines
} // namespace kotlinx
