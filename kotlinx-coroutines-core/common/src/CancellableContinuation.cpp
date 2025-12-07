/**
 * @file CancellableContinuation.cpp
 * @brief Implementation of CancellableContinuation.
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/CancellableContinuation.hpp`.
 */

#include "kotlinx/coroutines/CancellableContinuation.hpp"

namespace kotlinx {
namespace coroutines {

// Template interface is in the header.

void dispose_on_cancellation(CancellableContinuation<void>& cont, DisposableHandle* handle) {
    cont.invoke_on_cancellation([handle](std::exception_ptr) {
        handle->dispose();
    });
}

} // namespace coroutines
} // namespace kotlinx
