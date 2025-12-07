#pragma once
#include <string>
#include <memory>
#include <vector>
#include <exception>
#include <functional>
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/context_impl.hpp"

namespace kotlinx {
namespace coroutines {

struct DisposableHandle;

/**
 * A background job.
 * Conceptually, a job is a cancellable thing with a lifecycle that
 * concludes in its completion.
 *
 * Jobs can be arranged into parent-child hierarchies where the cancellation
 * of a parent leads to the immediate cancellation of all its [children] recursively.
 *
 * ### Job states
 *
 * A job has the following states:
 *
 * | **State**    | [isActive] | [isCompleted] | [isCancelled] |
 * | ------------ | ---------- | ------------- | ------------- |
 * | _New_        | `false`    | `false`       | `false`       |
 * | _Active_     | `true`     | `false`       | `false`       |
 * | _Completing_ | `true`     | `false`       | `false`       |
 * | _Cancelling_ | `false`    | `false`       | `true`        |
 * | _Cancelled_  | `false`    | `true`        | `true`        |
 * | _Completed_  | `false`    | `true`        | `false`       |
 */
struct Job : public CoroutineContext::Element {
    // Key for CoroutineContext
    static CoroutineContext::KeyTyped<Job> key_instance;
    static constexpr Key* key = &key_instance;

    virtual ~Job() = default;

    // ------------ state query ------------

    /**
     * Returns the parent of the current job if the parent-child relationship
     * is established or `nullptr` if the job has no parent or was successfully completed.
     */
    virtual std::shared_ptr<Job> get_parent() const = 0;

    /**
     * Returns `true` when this job is active -- it was already started and has not completed nor was cancelled yet.
     */
    virtual bool is_active() const = 0;

    /**
     * Returns `true` when this job has completed for any reason.
     */
    virtual bool is_completed() const = 0;

    /**
     * Returns `true` if this job was cancelled for any reason.
     */
    virtual bool is_cancelled() const = 0;

    /**
     * Returns [CancellationException] that signals the completion of this job.
     */
    virtual std::exception_ptr get_cancellation_exception() = 0;

    // ------------ state update ------------

    /**
     * Starts coroutine related to this job (if any) if it was not started yet.
     */
    virtual bool start() = 0;

    /**
     * Cancels this job with an optional cancellation [cause].
     */
    virtual void cancel(std::exception_ptr cause = nullptr) = 0;

    /**
     * Cancels the job and suspends the invoking coroutine until the cancelled job is complete.
     */
    virtual void cancel_and_join() {
        cancel();
        join();
    }

    /**
     * Ensures that current job is [active].
     * If the job is no longer active, throws [CancellationException].
     */
    virtual void ensure_active() {
        if (!is_active()) {
            throw get_cancellation_exception(); // Re-throw the cancellation cause
        }
    }

    // ------------ parent-child ------------

    /**
     * Returns a sequence of this job's children.
     */
    virtual std::vector<std::shared_ptr<Job>> get_children() const = 0;

    /**
     * Attaches child job so that this job becomes its parent.
     */
    virtual std::shared_ptr<DisposableHandle> attach_child(std::shared_ptr<Job> child) = 0;

    // ------------ state waiting ------------

    /**
     * Suspends the coroutine until this job is complete.
     */
    virtual void join() = 0;

    // ------------ low-level state-notification ------------

    /**
     * Registers handler that is **synchronously** invoked once on completion of this job.
     */
    virtual std::shared_ptr<DisposableHandle> invoke_on_completion(std::function<void(std::exception_ptr)> handler) = 0;

    virtual std::shared_ptr<DisposableHandle> invoke_on_completion(bool on_cancelling, bool invoke_immediately, std::function<void(std::exception_ptr)> handler) = 0;
};

// DisposableHandle
struct DisposableHandle {
    virtual void dispose() = 0;
    virtual ~DisposableHandle() = default;
};

// Singleton implementation for Key
// CoroutineContext::KeyTyped<Job> Job::key_instance;

} // namespace coroutines
} // namespace kotlinx
