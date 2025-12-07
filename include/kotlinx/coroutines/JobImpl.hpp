#pragma once
#include "JobSupport.hpp"
#include "../../../kotlinx-coroutines-core/common/src/CompletableJob.hpp"

namespace kotlinx {
namespace coroutines {

/**
 * Concrete implementation of CompletableJob that extends JobSupport.
 * This is the standard implementation returned by Job() and SupervisorJob() factory functions.
 *
 * Based on Kotlin's JobImpl:
 * internal open class JobImpl(parent: Job?) : JobSupport(true), CompletableJob {
 *     init { initParentJob(parent) }
 *     override val onCancelComplete get() = true
 *     override val handlesException: Boolean = handlesException()
 *     override fun complete() = makeCompleting(Unit)
 *     override fun completeExceptionally(exception: Throwable): Boolean =
 *         makeCompleting(CompletedExceptionally(exception))
 * }
 */
class JobImpl : public JobSupport, public CompletableJob {
public:
    /**
     * Creates a new JobImpl with the specified active state.
     */
    explicit JobImpl(bool active = true);

    /**
     * Creates a new JobImpl with the specified parent.
     */
    explicit JobImpl(struct Job* parent);

    virtual ~JobImpl();

    // JobImpl-specific overrides (matching Kotlin)
    bool on_cancel_complete() const override { return true; }
    bool handles_exception() const override { return handles_exception_impl(); }

    // CompletableJob interface implementation
    bool complete() override;
    bool complete_exceptionally(Throwable* exception) override;

private:
    // Helper method for computed handlesException property
    bool handles_exception_impl() const;

    // Factory functions
    friend CompletableJob* createJob(struct Job* parent);
    friend CompletableJob* createSupervisorJob(struct Job* parent);
};

} // namespace coroutines
} // namespace kotlinx