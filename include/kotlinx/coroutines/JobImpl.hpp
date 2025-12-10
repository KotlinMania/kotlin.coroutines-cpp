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
    explicit JobImpl(std::shared_ptr<Job> parent) : JobSupport(true) {
        // Note: init_parent_job cannot be called here due to shared_from_this() requirement.
        // Use create() factory or manually call init_parent_job().
        // If constructed directly, parent is IGNORED initially until init is called?
        // But we store parent_ in JobSupport::parent_ member?
        // We can't set parent_ safely without attaching?
        // We'll leave it for init_parent_job.
    }
    
    static std::shared_ptr<JobImpl> create(std::shared_ptr<Job> parent) {
        auto job = std::make_shared<JobImpl>(parent);
        job->init_parent_job(parent);
        return job;
    }
    
    // Internal init method (public for flexibility if needed, but risky)
    void init_parent_job(std::shared_ptr<Job> parent) {
        if (!parent) {
             parent_ = nullptr;
             return;
        }
        parent_ = parent;
        parent->start();
        parent_handle_keeper_ = parent->attach_child(shared_from_this());
        parentHandle_.store(parent_handle_keeper_.get());
    }

    // CompletableJob implementation
    bool complete() override {
        return make_completing(nullptr);
    }
    
    bool complete_exceptionally(std::exception_ptr exception) override {
        return make_completing(exception);
    }
    
    // Ensure we expose key() if JobSupport implements it (JobSupport implements key() from Job/Context)
    // JobSupport implements key() -> ContinuationInterceptor? 
    // Wait, JobSupport implements Job. Job implements Element. Element has key().
    // JobSupport::key() -> Job::Key.
    
    // Destructor
    ~JobImpl() override = default;
};

} // namespace coroutines
} // namespace kotlinx
