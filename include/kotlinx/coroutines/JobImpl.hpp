#pragma once
#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include <memory>
#include <exception>

namespace kotlinx {
namespace coroutines {

/**
 * Concrete job implementation.
 */
class JobImpl : public JobSupport, public CompletableJob {
    std::shared_ptr<DisposableHandle> parent_handle_keeper_;

public:
    explicit JobImpl(std::shared_ptr<Job> parent);
    
    static std::shared_ptr<JobImpl> create(std::shared_ptr<Job> parent);
    
    // Internal init method (public for flexibility if needed, but risky)
    void init_parent_job(std::shared_ptr<Job> parent);

    // CompletableJob implementation
    bool complete() override;
    
    bool complete_exceptionally(std::exception_ptr exception) override;
    
    // Destructor
    ~JobImpl() override = default;
};

} // namespace coroutines
} // namespace kotlinx
