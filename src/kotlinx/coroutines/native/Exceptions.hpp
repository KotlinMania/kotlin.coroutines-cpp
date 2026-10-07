// port-lint: source kotlinx-coroutines-core/native/src/Exceptions.kt
/**
 * Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt
 */
#pragma once
#include "../Exceptions.hpp"

namespace kotlinx::coroutines {

struct Job;

/**
 * Thrown by cancellable suspending functions if the Job of the coroutine is
 * cancelled or completed without cause, or with a cause or exception that is
 * not CancellationException. See Job.getCancellationException.
 */
// Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:16-32
class JobCancellationException final : public CancellationException {
public:
    // Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:21-25
    JobCancellationException(const std::string& message, std::exception_ptr cause, Job* job);

    // Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:24-24
    Job* get_job() const;

    // Transliterated from: kotlinx-coroutines-core/native/src/Exceptions.kt:27-29
    bool equals(const std::exception* other) const override;

private:
    // NOTE(port): The existing raw Job reference remains borrowed.
    Job* job_;
};

} // namespace kotlinx::coroutines
