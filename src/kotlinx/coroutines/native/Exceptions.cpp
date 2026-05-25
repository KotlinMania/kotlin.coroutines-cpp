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
        CancellationException *make_cancellation_exception(const std::string &message,
                                                            std::exception_ptr cause) {
            // Upstream uses Throwable.initCause to attach the cause; K/N already keeps
            // initCause as a no-op (see native/internal/StackTraceRecovery.kt), so the
            // cause parameter is intentionally ignored here. The factory returns a fresh
            // CancellationException carrying just the message.
            (void)cause;
            return new CancellationException(message);
        }

        // JobCancellationException and other classes are defined in the header.
    } // namespace coroutines
} // namespace kotlinx