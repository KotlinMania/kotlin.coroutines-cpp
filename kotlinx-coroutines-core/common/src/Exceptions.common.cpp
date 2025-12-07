/**
 * @file Exceptions.common.cpp
 * @brief Implementation of Exceptions.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/Exceptions.hpp`.
 */

#include "kotlinx/coroutines/Exceptions.hpp"

namespace kotlinx {
namespace coroutines {

// Implementation of make_cancellation_exception stub
CancellationException* make_cancellation_exception(const std::string& message, std::exception_ptr cause) {
    return new CancellationException(message);
}

// Global variable definition stub
const bool RECOVER_STACK_TRACES = false;

} // namespace coroutines
} // namespace kotlinx
