#pragma once
/**
 * @file Job.hpp
 * @brief Core job interfaces for kotlinx.coroutines
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Job.kt
 */

#include <string>
#include <memory>
#include <vector>
#include <exception>
#include <functional>
#include <stdexcept>
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/DisposableHandle.hpp"

namespace kotlinx {
namespace coroutines {

// Forward declarations
struct Job;
struct ChildJob;
struct ParentJob;
struct ChildHandle;
struct CompletableJob;
class JobSupport;
class JobNode;

// --------------- core job interfaces ---------------

/**
 * A background job.
 * Conceptually, a job is a cancellable thing with a lifecycle that
 * concludes in its completion.
 *
 * Jobs can be arranged into parent-child hierarchies where the cancellation
 * of a parent leads to the immediate cancellation of all its children() recursively.
 * Failure of a child with an exception other than CancellationException immediately cancels its parent and,
 * consequently, all its other children.
 * This behavior can be customized using supervisor_job().
 *
 * The most basic instances of the `Job` interface are created like this:
 *
 * - A **coroutine job** is created with the launch() coroutine builder.
 *   It runs a specified block of code and completes upon completion of this block.
 * - **CompletableJob** is created with a Job() factory function.
 *   It is completed by calling CompletableJob::complete().
 *
 * Conceptually, an execution of a job does not produce a result value.
 * Jobs are launched solely for their side effects.
 * See the Deferred interface for a job that produces a result.
 *
 * ### Job states
 *
 * A job has the following states:
 *
 * | **State**                        | is_active | is_completed | is_cancelled |
 * | -------------------------------- | --------- | ------------ | ------------ |
 * | _New_ (optional initial state)   | false     | false        | false        |
 * | _Active_ (default initial state) | true      | false        | false        |
 * | _Completing_ (transient state)   | true      | false        | false        |
 * | _Cancelling_ (transient state)   | false     | false        | true         |
 * | _Cancelled_ (final state)        | false     | true         | true         |
 * | _Completed_ (final state)        | false     | true         | false        |
 *
 * Usually, a job is created in the _active_ state (it is created and started).
 * However, coroutine builders that provide an optional `start` parameter create a coroutine
 * in the _new_ state when this parameter is set to CoroutineStart::LAZY.
 * Such a job can be made _active_ by invoking start() or join().
 *
 * A job is in the _active_ state while the coroutine is working or until the CompletableJob completes,
 * fails, or is cancelled.
 *
 * Failure of an _active_ job with an exception transitions the state to the _cancelling_ state.
 * A job can be cancelled at any time with the cancel() function that forces it to transition to
 * the _cancelling_ state immediately.
 * The job becomes _cancelled_ when it finishes executing its work and all its children complete.
 *
 * Completion of an _active_ coroutine's body or a call to CompletableJob::complete() transitions
 * the job to the _completing_ state. It waits in the _completing_ state for all its children to
 * complete before transitioning to the _completed_ state.
 *
 * ```
 *                                       wait children
 * +-----+ start  +--------+ complete   +-------------+  finish  +-----------+
 * | New | -----> | Active | ---------> | Completing  | -------> | Completed |
 * +-----+        +--------+            +-------------+          +-----------+
 *                  |  cancel / fail       |
 *                  |     +----------------+
 *                  |     |
 *                  V     V
 *              +------------+                           finish  +-----------+
 *              | Cancelling | --------------------------------> | Cancelled |
 *              +------------+                                   +-----------+
 * ```
 *
 * ### Cancellation cause
 *
 * A coroutine job is said to _complete exceptionally_ when its body throws an exception;
 * a CompletableJob is completed exceptionally by calling CompletableJob::complete_exceptionally().
 * An exceptionally completed job is cancelled, and the corresponding exception becomes the
 * _cancellation cause_ of the job.
 *
 * Normal cancellation of a job is distinguished from its failure by the exception
 * that caused its cancellation. A coroutine that throws a CancellationException is considered
 * to be _cancelled_ normally. If a different exception causes the cancellation, then the job
 * has _failed_. When a job has _failed_, its parent gets cancelled with the same type of exception,
 * thus ensuring transparency in delegating parts of the job to its children.
 *
 * ### Concurrency and synchronization
 *
 * All functions on this interface and on all interfaces derived from it are **thread-safe** and can
 * be safely invoked from concurrent coroutines without external synchronization.
 */
struct Job : public virtual CoroutineContext::Element {
    /**
     * Key for Job instance in the coroutine context.
     */
    inline static CoroutineContext::KeyTyped<Job> key_instance;
    static constexpr Key* type_key = &key_instance;

    virtual ~Job() = default;

    // ------------ state query ------------

    /**
     * Returns the parent of the current job if the parent-child relationship
     * is established or nullptr if the job has no parent or was successfully completed.
     *
     * Accesses to this property are not idempotent, the property becomes nullptr as soon
     * as the job is transitioned to its final state, whether it is cancelled or completed,
     * and all job children are completed.
     */
    virtual std::shared_ptr<Job> get_parent() const = 0;

    /**
     * Returns true when this job is active -- it was already started and has not completed
     * nor was cancelled yet. The job that is waiting for its children to complete is still
     * considered to be active if it was not cancelled nor failed.
     *
     * See Job documentation for more details on job states.
     */
    virtual bool is_active() const = 0;

    /**
     * Returns true when this job has completed for any reason. A job that was cancelled or failed
     * and has finished its execution is also considered complete. Job becomes complete only after
     * all its children complete.
     *
     * See Job documentation for more details on job states.
     */
    virtual bool is_completed() const = 0;

    /**
     * Returns true if this job was cancelled for any reason, either by explicit invocation of cancel() or
     * because it had failed or its child or parent was cancelled.
     * In the general case, it does not imply that the job has already completed (is_completed),
     * because it may still be finishing whatever it was doing and waiting for its children to complete.
     *
     * See Job documentation for more details on cancellation and failures.
     */
    virtual bool is_cancelled() const = 0;

    /**
     * Returns CancellationException that signals the completion of this job. This function is
     * used by cancellable suspending functions. They throw exception returned by this function
     * when they suspend in the context of this job and this job becomes _complete_.
     *
     * This function returns the original cancel() cause of this job if that cause was an instance of
     * CancellationException. Otherwise (if this job was cancelled with a cause of a different type, or
     * was cancelled without a cause, or had completed normally), an instance of CancellationException is
     * returned. The cause of the resulting CancellationException references the original cancellation
     * cause that was passed to cancel() function.
     *
     * This function throws std::logic_error when invoked on a job that is still active.
     *
     * @internal This is an internal API and should not be used from general code.
     */
    virtual std::exception_ptr get_cancellation_exception() = 0;

    // ------------ state update ------------

    /**
     * Starts coroutine related to this job (if any) if it was not started yet.
     * The result is true if this invocation actually started coroutine or false
     * if it was already started or completed.
     */
    virtual bool start() = 0;

    /**
     * Cancels this job with an optional cancellation cause.
     * A cause can be used to specify an error message or to provide other details on
     * the cancellation reason for debugging purposes.
     * See Job documentation for full explanation of cancellation machinery.
     */
    virtual void cancel(std::exception_ptr cause = nullptr) = 0;

    // ------------ parent-child ------------

    /**
     * Returns a vector of this job's children.
     *
     * A job becomes a child of this job when it is constructed with this job in its
     * CoroutineContext or using an explicit `parent` parameter.
     *
     * A parent-child relation has the following effect:
     *
     * - Cancellation of parent with cancel() or its exceptional completion (failure)
     *   immediately cancels all its children.
     * - Parent cannot complete until all its children are complete. Parent waits for all its children to
     *   complete in _completing_ or _cancelling_ state.
     * - Uncaught exception in a child, by default, cancels parent. This applies even to
     *   children created with async() and other future-like coroutine builders, even though
     *   their exceptions are caught and are encapsulated in their result.
     *   This default behavior can be overridden with supervisor_job().
     */
    virtual std::vector<std::shared_ptr<Job>> get_children() const = 0;

    /**
     * Attaches child job so that this job becomes its parent and
     * returns a handle that should be used to detach it.
     *
     * A parent-child relation has the following effect:
     * - Cancellation of parent with cancel() or its exceptional completion (failure)
     *   immediately cancels all its children.
     * - Parent cannot complete until all its children are complete. Parent waits for all its children to
     *   complete in _completing_ or _cancelling_ states.
     *
     * **A child must store the resulting ChildHandle and dispose() the attachment
     * to its parent on its own completion.**
     *
     * @internal This is an internal API. This method is too error prone for public API.
     */
    virtual std::shared_ptr<ChildHandle> attach_child(std::shared_ptr<ChildJob> child) = 0;

    // ------------ state waiting ------------

    /**
     * Suspends the coroutine until this job is complete. This invocation resumes normally (without exception)
     * when the job is complete for any reason and the Job of the invoking coroutine is still active.
     * This function also starts the corresponding coroutine if the Job was still in _new_ state.
     *
     * Note that the job becomes complete only when all its children are complete.
     *
     * This suspending function is cancellable and **always** checks for a cancellation of the invoking
     * coroutine's Job. If the Job of the invoking coroutine is cancelled or completed when this
     * suspending function is invoked or while it is suspended, this function throws CancellationException.
     *
     * Use is_completed() to check for a completion of this job without waiting.
     *
     * There is cancel_and_join() function that combines an invocation of cancel() and join().
     */
    virtual void join() = 0;

    // ------------ low-level state-notification ------------

    /**
     * Registers handler that is **synchronously** invoked once on completion of this job.
     * When the job is already complete, then the handler is immediately invoked
     * with the job's exception or cancellation cause or nullptr. Otherwise, the handler will be
     * invoked once when this job is complete.
     *
     * The meaning of `cause` that is passed to the handler:
     * - Cause is nullptr when the job has completed normally.
     * - Cause is an instance of CancellationException when the job was cancelled _normally_.
     *   **It should not be treated as an error**. In particular, it should not be reported to error logs.
     * - Otherwise, the job had _failed_.
     *
     * The resulting DisposableHandle can be used to dispose() the
     * registration of this handler and release its memory if its invocation is no longer needed.
     * There is no need to dispose the handler after completion of this job. The references to
     * all the handlers are released when this job completes.
     *
     * Installed handler should not throw any exceptions. If it does, they will get caught,
     * wrapped into CompletionHandlerException, and rethrown, potentially causing crash of unrelated code.
     *
     * **Note**: Implementation of handler must be fast, non-blocking, and thread-safe.
     * This handler can be invoked concurrently with the surrounding code.
     * There is no guarantee on the execution context in which the handler is invoked.
     */
    virtual std::shared_ptr<DisposableHandle> invoke_on_completion(
        std::function<void(std::exception_ptr)> handler) = 0;

    /**
     * @internal Kept for preserving compatibility.
     */
    virtual std::shared_ptr<DisposableHandle> invoke_on_completion(
        bool on_cancelling,
        bool invoke_immediately,
        std::function<void(std::exception_ptr)> handler) = 0;

    // TODO: MISSING API - kotlinx.coroutines.Job
    // public abstract fun getOnJoin(): SelectClause0
    // Returns a select clause that selects when this job is complete. This clause never fails,
    // even if the job completes exceptionally.
    // Translation: virtual SelectClause0 get_on_join() const = 0;
    
    // TODO: MISSING API - kotlinx.coroutines.Job  
    // public abstract fun plus(Job): Job
    // Returns a job that is a combination of this job and the specified job. The resulting job
    // completes when both jobs complete or when any of them fails.
    // Translation: virtual std::shared_ptr<Job> plus(std::shared_ptr<Job> other) = 0;

    // Key override
    CoroutineContext::Key* key() const override { return type_key; }
};

// -------------------- Parent-child communication --------------------

/**
 * A reference that parent receives from its child so that it can report its cancellation.
 *
 * @internal This is unstable API and it is subject to change.
 */
struct ChildJob : public virtual Job {
    /**
     * Parent is cancelling its child by invoking this method.
     * Child finds the cancellation cause using ParentJob::get_child_job_cancellation_cause().
     * This method does nothing if the child is already being cancelled.
     *
     * @internal This is unstable API and it is subject to change.
     */
    virtual void parent_cancelled(ParentJob* parent_job) = 0;
};

/**
 * A reference that child receives from its parent when it is being cancelled by the parent.
 *
 * @internal This is unstable API and it is subject to change.
 */
struct ParentJob : public virtual Job {
    /**
     * Child job is using this method to learn its cancellation cause when the parent cancels it
     * with ChildJob::parent_cancelled(). This method is invoked only if the child was not already
     * being cancelled.
     *
     * Note that CancellationException is the method's return type: if child is cancelled by its parent,
     * then the original exception is **already** handled by either the parent or the original source of failure.
     *
     * @internal This is unstable API and it is subject to change.
     */
    virtual std::exception_ptr get_child_job_cancellation_cause() = 0;
};

/**
 * A handle that child keeps onto its parent so that it is able to report its cancellation.
 *
 * @internal This is unstable API and it is subject to change.
 */
struct ChildHandle : public DisposableHandle {
    /**
     * Returns the parent of the current parent-child relationship.
     * @internal This is unstable API and it is subject to change.
     */
    virtual std::shared_ptr<Job> get_parent() const = 0;

    /**
     * Child is cancelling its parent by invoking this method.
     * This method is invoked by the child twice. The first time child reports its root cause as soon
     * as possible, so that all its siblings and the parent can start cancelling their work asap.
     * The second time child invokes this method when it had aggregated and determined its final
     * cancellation cause.
     *
     * @internal This is unstable API and it is subject to change.
     */
    virtual bool child_cancelled(std::exception_ptr cause) = 0;
};

/**
 * No-op implementation of DisposableHandle.
 * @internal This is an internal API and should not be used from general code.
 */
class NonDisposableHandle : public ChildHandle {
public:
    static NonDisposableHandle& instance() {
        static NonDisposableHandle inst;
        return inst;
    }

    std::shared_ptr<Job> get_parent() const override { return nullptr; }
    void dispose() override {}
    bool child_cancelled(std::exception_ptr cause) override { return false; }
    std::string to_string() const { return "NonDisposableHandle"; }

private:
    NonDisposableHandle() = default;
};

// Convenience shared_ptr version
inline std::shared_ptr<DisposableHandle> non_disposable_handle() {
    // Return a shared_ptr that doesn't own (prevent double-delete of singleton)
    return std::shared_ptr<DisposableHandle>(&NonDisposableHandle::instance(), [](DisposableHandle*){});
}

// -------------------- Job extensions --------------------

/**
 * Cancels the job and suspends the invoking coroutine until the cancelled job is complete.
 *
 * This suspending function is cancellable and **always** checks for a cancellation of the invoking
 * coroutine's Job. If the Job of the invoking coroutine is cancelled or completed when this
 * suspending function is invoked or while it is suspended, this function throws CancellationException.
 *
 * This is a shortcut for the invocation of cancel() followed by join().
 */
inline void cancel_and_join(Job& job) {
    job.cancel();
    job.join();
}

/**
 * Cancels all children jobs of this coroutine using Job::cancel() for all of them
 * with an optional cancellation cause.
 * Unlike Job::cancel() on this job as a whole, the state of this job itself is not affected.
 */
inline void cancel_children(Job& job, std::exception_ptr cause = nullptr) {
    for (auto& child : job.get_children()) {
        child->cancel(cause);
    }
}

/**
 * Ensures that current job is active.
 * If the job is no longer active, throws CancellationException.
 * If the job was cancelled, thrown exception contains the original cancellation cause.
 */
inline void ensure_active(Job& job) {
    if (!job.is_active()) {
        std::exception_ptr ex = job.get_cancellation_exception();
        if (ex) {
            std::rethrow_exception(ex);
        }
        throw std::runtime_error("Job is not active");
    }
}

// -------------------- CoroutineContext extensions --------------------

/**
 * Returns true when the Job of the coroutine in this context is still active
 * (has not completed and was not cancelled yet) or the context does not have a Job in it.
 *
 * Check this property in long-running computation loops to support cancellation:
 *
 * ```cpp
 * while (context_is_active(coroutine_context)) {
 *     // do some computation
 * }
 * ```
 *
 * The context_is_active(ctx) expression is a shortcut for `ctx.get(Job::type_key)->is_active()`.
 * See Job::is_active().
 */
inline bool context_is_active(const CoroutineContext& ctx) {
    auto job_element = ctx.get(Job::type_key);
    if (auto job = std::dynamic_pointer_cast<Job>(job_element)) {
        return job->is_active();
    }
    return true; // No job in context means "active" by default
}

/**
 * Cancels Job of this context with an optional cancellation cause.
 * See Job::cancel() for details.
 */
inline void context_cancel(const CoroutineContext& ctx, std::exception_ptr cause = nullptr) {
    auto job_element = ctx.get(Job::type_key);
    if (auto job = std::dynamic_pointer_cast<Job>(job_element)) {
        job->cancel(cause);
    }
}

/**
 * Retrieves the current Job instance from the given CoroutineContext or
 * throws std::logic_error if no job is present in the context.
 *
 * This method is a short-cut for ctx.get(Job::type_key) and should be used only when it is
 * known in advance that the context does have instance of the job in it.
 */
inline std::shared_ptr<Job> context_job(const CoroutineContext& ctx) {
    auto job_element = ctx.get(Job::type_key);
    if (auto job = std::dynamic_pointer_cast<Job>(job_element)) {
        return job;
    }
    throw std::logic_error("Current context doesn't contain Job in it");
}

/**
 * Ensures that job in the context is active.
 *
 * If the job is no longer active, throws CancellationException.
 * If the job was cancelled, thrown exception contains the original cancellation cause.
 * This function does not do anything if there is no Job in the context, since such a coroutine
 * cannot be cancelled.
 */
inline void context_ensure_active(const CoroutineContext& ctx) {
    auto job_element = ctx.get(Job::type_key);
    if (auto job = std::dynamic_pointer_cast<Job>(job_element)) {
        ensure_active(*job);
    }
}

/**
 * Cancels all children of the Job in this context, without touching the state of this job itself
 * with an optional cancellation cause. See Job::cancel().
 * It does not do anything if there is no job in the context or it has no children.
 */
inline void context_cancel_children(const CoroutineContext& ctx, std::exception_ptr cause = nullptr) {
    auto job_element = ctx.get(Job::type_key);
    if (auto job = std::dynamic_pointer_cast<Job>(job_element)) {
        cancel_children(*job, cause);
    }
}

// -------------------- Factory function --------------------

/**
 * Creates a job object in an active state.
 * A failure of any child of this job immediately causes this job to fail, too, and cancels the rest
 * of its children.
 *
 * To handle children failure independently of each other use supervisor_job().
 *
 * If parent job is specified, then this job becomes a child job of its parent and
 * is cancelled when its parent fails or is cancelled. All this job's children are cancelled in this case, too.
 *
 * Conceptually, the resulting job works in the same way as the job created by the `launch { body }` invocation
 * (see launch()), but without any code in the body. It is active until cancelled or completed. Invocation of
 * CompletableJob::complete() or CompletableJob::complete_exceptionally() corresponds to the successful or
 * failed completion of the body of the coroutine.
 *
 * @param parent an optional parent job.
 */
std::shared_ptr<CompletableJob> make_job(std::shared_ptr<struct Job> parent = nullptr);

// Alias for Kotlin-style Job() factory
inline std::shared_ptr<CompletableJob> Job(std::shared_ptr<struct Job> parent = nullptr) {
    return make_job(parent);
}

} // namespace coroutines
} // namespace kotlinx
