// port-lint: source Timeout.kt
/**
 * @file Timeout.cpp
 * @brief Timeout utilities for coroutines
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Timeout.kt
 */

#include "kotlinx/coroutines/Timeout.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/Job.hpp"

#include <string>

namespace kotlinx {
namespace coroutines {

    TimeoutCancellationException::TimeoutCancellationException(const std::string& message)
        : CancellationException(message) {}

    TimeoutCancellationException::TimeoutCancellationException(const std::string& message, std::shared_ptr<Job> coroutine)
        : CancellationException(message), coroutine(coroutine) {}

    std::exception_ptr TimeoutCancellationException::create_copy() const {
        auto copy = TimeoutCancellationException(what(), coroutine);
        // Note: initCause equivalent not fully established, passing explicit construction
        return std::make_exception_ptr(copy);
    }

    TimeoutCancellationException make_timeout_cancellation_exception(
        long time,
        Delay* delay,
        std::shared_ptr<Job> coroutine
    ) {
        (void)delay;
        // Wrapper for dynamic_cast check if Delay supports diagnostics
        std::string message = "Timed out waiting for " + std::to_string(time) + " ms";
        
        if (auto* diagnostics = dynamic_cast<DelayWithTimeoutDiagnostics*>(delay)) {
             // Convert long milliseconds to nanoseconds for the message
             message = diagnostics->timeout_message(std::chrono::milliseconds(time));
        }
        
        return TimeoutCancellationException(message, coroutine);
    }

} // namespace coroutines
} // namespace kotlinx