#pragma once
#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include <memory>
#include <exception>

namespace kotlinx {
namespace coroutines {

/**
 * @brief Concrete job implementation with full lifecycle management.
 * 
 * JobImpl provides a complete implementation of the Job interface through
 * inheritance from JobSupport and CompletableJob. This class represents
 * a standard job that can be used directly or as a base for more specialized
 * job implementations.
 * 
 * === Features ===
 * - Full job state machine implementation (New -> Active -> Completing/Cancelled -> Completed)
 * - Parent-child relationship management with proper cancellation propagation
 * - Completion handler registration and invocation
 * - Thread-safe state transitions and operations
 * - Automatic memory management via shared_ptr
 * 
 * === Usage ===
 * JobImpl is typically created through factory functions like make_job():
 * ```cpp
 * auto job = JobImpl::create(parent);  // With parent job
 * auto standalone = JobImpl::create(nullptr);  // Standalone job
 * 
 * // Complete the job
 * job->complete();  // Normal completion
 * job->complete_exceptionally(std::make_exception_ptr(std::runtime_error("Failed")));  // Failure
 * ```
 * 
 * === Memory Management ===
 * - Uses shared_ptr for reference counting
 * - Parent-child relationships create circular references that are broken on completion
 * - Completion handlers are automatically cleaned up when job completes
 * - Thread-safe without external synchronization
 * 
 * @note This is the standard job implementation used by most coroutine builders.
 *       For supervisor behavior (where child failures don't cancel parent), 
 *       use SupervisorJobImpl instead.
 */
class JobImpl : public JobSupport, public CompletableJob {
public:
    /**
     * @brief Constructs a JobImpl with optional parent.
     *
     * Creates a new job instance and establishes parent-child relationship
     * if parent is provided. The job starts in active state.
     * Note: Parent cannot be used in constructor due to shared_from_this() requirement.
     * The create() factory handles calling init_parent_job after construction.
     *
     * @param parent Optional parent job for structured concurrency
     */
    explicit JobImpl(std::shared_ptr<Job> parent);

protected:
    /**
     * Returns true - JobImpl completes on cancel.
     * Transliterated from: override val onCancelComplete get() = true (JobSupport.kt:1427)
     */
    bool get_on_cancel_complete() const override { return true; }

    /**
     * Cached result of handlesException() check.
     * Transliterated from: override val handlesException: Boolean = handlesException() (JobSupport.kt:1438)
     *
     * This recursively checks whether the parent job handles exceptions.
     * With this check, an exception in this pattern will be handled once:
     * ```cpp
     * launch {
     *     auto child = JobImpl::create(coroutineContext[Job]);
     *     launch(child, [] { throw ... });
     * }
     * ```
     */
    bool get_handles_exception() const override;

private:
    /**
     * Computes whether this job's parent hierarchy handles exceptions.
     * Transliterated from: private fun handlesException(): Boolean (JobSupport.kt:1444-1449)
     */
    bool compute_handles_exception() const;

    // Cached result - mutable because computed lazily in const getter
    mutable bool handles_exception_cached_ = false;
    mutable bool handles_exception_computed_ = false;

public:
    
    /**
     * @brief Factory method to create JobImpl with proper shared_ptr setup.
     * 
     * This is the recommended way to create JobImpl instances as it ensures
     * proper shared_ptr initialization for parent-child relationships.
     * 
     * @param parent Optional parent job for structured concurrency
     * @return shared_ptr to the created JobImpl
     */
    static std::shared_ptr<JobImpl> create(std::shared_ptr<Job> parent);
    
    /**
     * @brief Initializes parent-child relationship.
     * 
     * @warning This method is internal and should be used with caution.
     *          It's primarily for cases where parent initialization needs
     *          to be deferred or handled specially.
     * 
     * @param parent Parent job to attach to
     */
    void init_parent_job(std::shared_ptr<Job> parent) override;

    /**
     * @brief Completes the job normally.
     * 
     * Transitions the job to completing state, waits for all children
     * to complete, then transitions to completed state.
     * 
     * @return true if this invocation completed the job, false if already completed
     */
    bool complete() override;
    
    /**
     * @brief Completes the job exceptionally with given exception.
     * 
     * Transitions the job to cancelling state, cancels all children,
     * waits for them to complete, then transitions to cancelled state.
     * 
     * @param exception The exception that caused the failure
     * @return true if this invocation completed the job, false if already completed
     */
    bool complete_exceptionally(std::exception_ptr exception) override;
    
    /**
     * @brief Destructor.
     * 
     * Cleans up resources. Job should be completed before destruction
     * for proper cleanup of parent-child relationships.
     */
    ~JobImpl() override = default;
};

} // namespace coroutines
} // namespace kotlinx
