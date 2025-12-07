#pragma once
#include "../../../include/kotlinx/coroutines/Job.hpp"

namespace kotlinx {
namespace coroutines {

// Throwable is defined in core_fwd.hpp

/**
 * A job that can be completed using [complete()] function.
 * It is returned by [Job()][Job] and [SupervisorJob()][SupervisorJob] constructor functions.
 *
 * All functions on this struct are **thread-safe** and can
 * be safely invoked from concurrent coroutines without external synchronization.
 *
 * **The `CompletableJob` struct is not stable for inheritance in 3rd party libraries**,
 * as new methods might be added to this struct in the future, but is stable for use.
 */
class CompletableJob : public virtual Job {
public:
    /**
     * Completes this job. The result is `true` if this job was completed as a result of this invocation and
     * `false` otherwise (if it was already completed).
     *
     * Subsequent invocations of this function have no effect and always produce `false`.
     *
     * This function transitions this job into _completed_ state if it was not completed or cancelled yet.
     * However, that if this job has children, then it transitions into _completing_ state and becomes _complete_
     * once all its children are [complete][isCompleted]. See [Job] for details.
     */
    virtual bool complete() = 0;

    /**
     * Completes this job exceptionally with a given [exception]. The result is `true` if this job was
     * completed as a result of this invocation and `false` otherwise (if it was already completed).
     * [exception] parameter is used as an additional debug information that is not handled by any exception handlers.
     *
     * Subsequent invocations of this function have no effect and always produce `false`.
     *
     * This function transitions this job into the _cancelled_ state if it has not been _completed_ or _cancelled_ yet.
     * However, if this job has children, then it transitions into the _cancelling_ state and becomes _cancelled_
     * once all its children are [complete][isCompleted]. See [Job] for details.
     *
     * It is the responsibility of the caller to properly handle and report the given [exception].
     * All the job's children will receive a [CancellationException] with
     * the [exception] as a cause for the sake of diagnosis.
     */
    virtual bool complete_exceptionally(Throwable* exception) = 0;
    
    virtual ~CompletableJob() = default;
};

} // namespace coroutines
} // namespace kotlinx