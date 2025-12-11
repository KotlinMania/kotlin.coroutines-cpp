#pragma once
/**
 * @file JobSupport.hpp
 * @brief Core job implementation base class
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt
 *
 * This header exposes only the public API. All internal implementation
 * details are in JobSupport.cpp.
 */

#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/DisposableHandle.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include <memory>
#include <string>
#include <functional>
#include <exception>

namespace kotlinx {
namespace coroutines {

// Forward declarations for internal types (defined in .cpp)
namespace internal {
    class JobSupportState;
}

/**
 * Base class for Job implementations.
 *
 * This class provides the core job lifecycle management including:
 * - State transitions (New -> Active -> Completing -> Completed/Cancelled)
 * - Parent-child relationships
 * - Completion handlers
 * - Cancellation propagation
 *
 * Transliterated from: public open class JobSupport
 */
class JobSupport : public ParentJob,
                   public ChildJob {
public:
    /**
     * Creates a new JobSupport.
     * @param active If true, starts in Active state; if false, starts in New state
     */
    explicit JobSupport(bool active);

    virtual ~JobSupport();

    // ===========================================
    // Job interface implementation
    // ===========================================

    std::shared_ptr<Job> get_parent() const override;
    bool is_active() const override;
    bool is_completed() const override;
    bool is_cancelled() const override;
    std::exception_ptr get_cancellation_exception() override;
    bool start() override;
    void cancel(std::exception_ptr cause = nullptr) override;
    void* join(Continuation<void*>* continuation) override;
    void join_blocking() override;
    std::vector<std::shared_ptr<Job>> get_children() const override;
    std::shared_ptr<ChildHandle> attach_child(std::shared_ptr<ChildJob> child) override;

    std::shared_ptr<DisposableHandle> invoke_on_completion(
        std::function<void(std::exception_ptr)> handler) override;

    std::shared_ptr<DisposableHandle> invoke_on_completion(
        bool on_cancelling,
        bool invoke_immediately,
        std::function<void(std::exception_ptr)> handler) override;

    // ===========================================
    // CoroutineContext::Element implementation
    // ===========================================

    CoroutineContext::Key* key() const override;
    std::shared_ptr<CoroutineContext::Element> get(CoroutineContext::Key* k) const override;

    // ===========================================
    // ParentJob interface
    // ===========================================

    std::exception_ptr get_child_job_cancellation_cause() override;

    // ===========================================
    // ChildJob interface
    // ===========================================

    void parent_cancelled(ParentJob* parent) override;

    // ===========================================
    // Protected hooks for subclasses
    // ===========================================

    /**
     * Called when a child job is cancelled.
     * @param cause The cancellation cause
     * @return true if the exception was handled
     *
     * Note: This is public to allow access from internal handler nodes.
     * Transliterated from: public open fun childCancelled(cause: Throwable): Boolean
     */
    virtual bool child_cancelled(std::exception_ptr cause);

    /**
     * Cancels the coroutine with the given cause.
     * Transliterated from: public fun cancelCoroutine(cause: Throwable?): Boolean
     * @return true if cancellation was initiated
     */
    bool cancel_coroutine(std::exception_ptr cause = nullptr);

    /**
     * Returns the completion cause if completed exceptionally, nullptr otherwise.
     * Transliterated from: protected val completionCause: Throwable?
     */
    std::exception_ptr get_completion_cause() const;

    /**
     * Returns whether the completion exception was handled.
     * Transliterated from: protected val completionCauseHandled: Boolean
     */
    bool get_completion_cause_handled() const;

    /**
     * Returns true if this job completed exceptionally.
     * Transliterated from: val isCompletedExceptionally: Boolean
     */
    bool is_completed_exceptionally() const;

    /**
     * Returns the completion exception if completed exceptionally, nullptr if completed normally.
     * Throws if not yet completed.
     * Transliterated from: fun getCompletionExceptionOrNull(): Throwable?
     */
    std::exception_ptr get_completion_exception_or_null() const;

protected:
    /**
     * Called when the job is started (transitions from New to Active).
     * Override to perform initialization.
     */
    virtual void on_start() {}

    /**
     * Called when the job starts cancelling.
     * @param cause The cancellation cause (may be null for normal cancellation)
     */
    virtual void on_cancelling(std::exception_ptr cause) {}

    /**
     * Called when the job completes (either normally or exceptionally).
     * @param state The final state (result value or CompletedExceptionally)
     */
    virtual void on_completion_internal(JobState* state) {}

    /**
     * Called after the job has fully completed and all handlers have been invoked.
     * @param state The final state
     */
    virtual void after_completion(JobState* state) {}

    /**
     * Handles exceptions thrown by completion handlers.
     */
    virtual void handle_on_completion_exception(std::exception_ptr exception);

    /**
     * Returns the message for the default CancellationException.
     */
    virtual std::string cancellation_exception_message() const;

    /**
     * Returns true if completing this job should cancel it.
     * Used by CompletableJob implementations.
     */
    virtual bool get_on_cancel_complete() const { return false; }

    /**
     * Returns true if this is a scoped coroutine (affects exception handling).
     */
    virtual bool get_is_scoped_coroutine() const { return false; }

    /**
     * Returns true if this job handles exceptions from children.
     */
    virtual bool get_handles_exception() const { return true; }

    /**
     * Called to handle a job exception (for CoroutineExceptionHandler).
     * @return true if the exception was handled
     */
    virtual bool handle_job_exception(std::exception_ptr exception) { return false; }

    // ===========================================
    // Protected methods for subclasses
    // ===========================================

    /**
     * Initializes the parent-child relationship.
     * Call this from subclass constructors after shared_from_this() is available.
     */
    void init_parent_job(std::shared_ptr<Job> parent);

    /**
     * Result codes for make_completing_once
     */
    enum class CompletingResult {
        COMPLETING,           // Completion started, waiting for children
        ALREADY_COMPLETING,   // Job already completing
        COMPLETED            // Job completed immediately
    };

    /**
     * Attempts to complete the job with the given state.
     * @param proposed_state The proposed completion state
     * @return true if completion was initiated
     */
    bool make_completing(JobState* proposed_state);

    /**
     * Attempts to complete the job, returning detailed status.
     * Used by AbstractCoroutine to know if it should wait.
     */
    CompletingResult make_completing_once(JobState* proposed_state);

    /**
     * Gets the completed state, throwing if not completed or if completed exceptionally.
     */
    JobState* get_completed_internal() const;

    /**
     * Gets the state for await - returns the raw state (result or CompletedExceptionally).
     * Does not throw - caller must check for CompletedExceptionally.
     * Transliterated from: state access in ResumeAwaitOnCompletion
     */
    JobState* get_state_for_await() const;

    /**
     * Suspend function: await for completion and return result.
     * Transliterated from: protected suspend fun awaitInternal(): Any?
     * @return COROUTINE_SUSPENDED if suspended, or result pointer if complete
     */
    void* await_internal(Continuation<void*>* continuation);

    /**
     * Blocking version of await for non-coroutine contexts.
     */
    JobState* await_internal_blocking();

    // ===========================================
    // Debug support
    // ===========================================

public:
    std::string to_debug_string() const;

protected:
    virtual std::string name_string() const;
    std::string state_string() const;

private:
    // Internal join helpers
    bool join_internal();
    void* join_suspend(Continuation<void*>* continuation);

    // Internal await helpers
    void* await_suspend(Continuation<void*>* continuation);

    // Opaque pointer to implementation state
    // All internal state machine logic is in JobSupport.cpp
    class Impl;
    std::unique_ptr<Impl> impl_;

    // Allow internal node classes to access impl_ for dispose operations
    friend class JobNode;
    friend class ChildCompletion;
    friend class ResumeOnCompletion;
    friend class ResumeAwaitOnCompletion;
};

} // namespace coroutines
} // namespace kotlinx
