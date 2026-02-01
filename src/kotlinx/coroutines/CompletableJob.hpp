#pragma once
// port-lint: source CompletableJob.kt
/**
 * @file CompletableJob.hpp
 * @brief CompletableJob interface
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/CompletableJob.kt
 */

#include "kotlinx/coroutines/Job.hpp"
#include <exception>

namespace kotlinx {
namespace coroutines {

/**
 * @brief A job that can be manually completed using complete() functions.
 * 
 * CompletableJob extends the Job interface to add manual completion capabilities.
 * This is useful for creating jobs that represent external asynchronous operations
 * or for coordinating between different parts of code.
 *
 * === Creation and Usage ===
 * CompletableJob instances are created through factory functions:
 * - make_job(): Creates a standard completable job
 * - make_supervisor_job(): Creates a supervisor completable job
 * 
 * ```cpp
 * auto job = make_job();
 * 
 * // Later, when the operation is done:
 * job->complete();  // Normal completion
 * // or
 * job->complete_exceptionally(std::make_exception_ptr(std::runtime_error("Failed")));
 * ```
 *
 * === Completion Semantics ===
 * - complete(): Transitions to completing state, waits for children, then completed
 * - complete_exceptionally(): Transitions to cancelling state, cancels children, then cancelled
 * - Both methods are idempotent - subsequent calls have no effect
 * - Return value indicates whether this invocation actually completed the job
 *
 * === Use Cases ===
 * - Representing external async operations (file I/O, network requests)
 * - Coordinating completion between multiple coroutines
 * - Building custom coroutine builders
 * - Testing and mocking scenarios
 * - Manual job management in complex concurrency scenarios
 *
 * === Thread Safety ===
 * All functions on this interface are **thread-safe** and can
 * be safely invoked from concurrent coroutines without external synchronization.
 *
 * === Inheritance Stability ===
 * **The `CompletableJob` interface is not stable for inheritance in 3rd party libraries**,
 * as new methods might be added to this interface in the future, but is stable for use.
 * 
 * @note This interface provides the foundation for manual job control.
 *       For most use cases, prefer using coroutine builders like launch() or async()
 *       which handle completion automatically.
 */
struct CompletableJob : public virtual Job {
    virtual ~CompletableJob() = default;

    /**
     * @brief Completes this job normally.
     * 
     * Transitions the job to completion state. If the job has children,
     * it first transitions to completing state and waits for all children
     * to complete before reaching the final completed state.
     * 
     * @return true if this invocation completed the job, false if already completed
     * 
     * @note Subsequent invocations have no effect and always return false.
     *       This method is thread-safe and can be called from any thread.
     */
    virtual bool complete() = 0;

    /**
     * @brief Completes this job exceptionally with the given exception.
     * 
     * Transitions the job to cancelled state. If the job has children,
     * it first transitions to cancelling state, cancels all children,
     * waits for them to complete, then reaches the final cancelled state.
     * 
     * The exception is used as the cancellation cause and is propagated
     * to children for debugging purposes. The exception itself is not
     * handled by exception handlers - only the resulting CancellationException
     * is handled.
     * 
     * @param exception The exception that caused the failure
     * @return true if this invocation completed the job, false if already completed
     * 
     * @note The caller is responsible for properly handling and reporting
     *       the exception. This method is thread-safe and can be called from any thread.
     *       Subsequent invocations have no effect and always return false.
     */
    virtual bool complete_exceptionally(std::exception_ptr exception) = 0;
};

} // namespace coroutines
} // namespace kotlinx
