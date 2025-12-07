#pragma once
#include "JobSupport.hpp"
#include "../../../kotlinx-coroutines-core/common/src/CompletableJob.hpp"

namespace kotlinx {
namespace coroutines {

/**
 * Concrete implementation of CompletableJob that extends JobSupport.
 * This is the standard implementation returned by Job() and SupervisorJob() factory functions.
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

    // CompletableJob interface implementation
    bool complete() override;
    bool complete_exceptionally(Throwable* exception) override;

    // Factory functions
    friend CompletableJob* createJob(struct Job* parent);
    friend CompletableJob* createSupervisorJob(struct Job* parent);
};

} // namespace coroutines
} // namespace kotlinx