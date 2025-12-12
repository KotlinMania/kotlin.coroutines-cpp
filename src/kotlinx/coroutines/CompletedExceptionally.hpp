#pragma once
/**
 * @file CompletedExceptionally.hpp
 * @brief Job state representation for exceptional completion
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/CompletedExceptionally.kt
 */

#include <exception>
#include <atomic>

namespace kotlinx {
namespace coroutines {

/**
 * @brief Base class for job state representations.
 * 
 * JobState is the base class for all internal job state objects.
 * Specific state types inherit from this to provide state-specific
 * behavior and data storage.
 */
struct JobState {
    virtual ~JobState() = default;
};

/**
 * @brief Represents a job that completed with an exception.
 * 
 * CompletedExceptionally is used internally by the job state machine
 * to represent jobs that failed or were cancelled. It stores the
 * exception that caused the completion and tracks whether the
 * exception was handled by exception handlers.
 * 
 * === Exception Handling ===
 * The 'handled' flag tracks whether the exception was processed by
 * a CoroutineExceptionHandler. This is important for determining
 * whether the exception should be propagated to parent jobs.
 * 
 * === Thread Safety ===
 * The 'handled' flag is atomic to support concurrent access from
 * multiple threads during completion processing.
 * 
 * === Memory Management ===
 * CompletedExceptionally objects are managed by the job state machine
 * and are automatically cleaned up when the job completes.
 * 
 * @note This is an internal implementation detail. External code should
 *       use the public Job interface methods for completion and error handling.
 */
class CompletedExceptionally : public JobState {
public:
    /**
     * @brief The exception that caused the exceptional completion.
     * 
     * This may be a CancellationException for normal cancellation,
     * or any other exception type for failures.
     */
    std::exception_ptr cause;
    
    /**
     * @brief Atomic flag indicating if the exception was handled.
     * 
     * true if the exception was handled by CoroutineExceptionHandler,
     * false otherwise. This affects parent cancellation behavior.
     */
    std::atomic<bool> handled;

    /**
     * @brief Constructs a CompletedExceptionally with the given cause.
     * 
     * @param cause The exception that caused completion
     * @param handled Whether the exception is initially marked as handled
     */
    CompletedExceptionally(std::exception_ptr cause, bool handled = false) 
        : cause(cause), handled(handled) {}
        
    /**
     * @brief Copy constructor with atomic load of handled flag.
     * 
     * @param other The CompletedExceptionally to copy from
     */
    CompletedExceptionally(const CompletedExceptionally& other) 
        : cause(other.cause), handled(other.handled.load()) {}

    /**
     * @brief Atomically marks the exception as handled.
     * 
     * This method uses compare-and-swap to ensure that only one
     * caller can successfully mark the exception as handled.
     * 
     * @return true if this invocation marked the exception as handled,
     *         false if it was already handled
     */
    bool make_handled() {
        bool expected = false;
        return handled.compare_exchange_strong(expected, true);
    }
};

} // namespace coroutines
} // namespace kotlinx
