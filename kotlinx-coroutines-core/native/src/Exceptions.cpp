/**
 * @file Exceptions.cpp
 * @brief Implementation of Exceptions.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/Exceptions.hpp`.
 */

#include "kotlinx/coroutines/Exceptions.hpp"

namespace kotlinx {
namespace coroutines {

CancellationException* make_cancellation_exception(const std::string& message, std::exception_ptr cause) {
     // TODO: Implement proper exception creation or factory
     return new CancellationException(message);
}

// JobCancellationException and other classes are defined in the header.

} // namespace coroutines
} // namespace kotlinx
