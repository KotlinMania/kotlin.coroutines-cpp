/**
 * @file JobImpl.cpp
 * @brief Implementation of JobImpl.
 */

#include "kotlinx/coroutines/JobImpl.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"

namespace kotlinx {
    namespace coroutines {
        JobImpl::JobImpl(std::shared_ptr<Job> parent) : JobSupport(true) {
            // Note: init_parent_job cannot be called here due to shared_from_this() requirement.
        }

        std::shared_ptr<JobImpl> JobImpl::create(std::shared_ptr<Job> parent) {
            auto job = std::make_shared < JobImpl > (parent);
            job->init_parent_job(parent);
            return job;
        }

        void JobImpl::init_parent_job(std::shared_ptr<Job> parent) {
            // Delegate to JobSupport's protected init_parent_job
            JobSupport::init_parent_job(parent);
        }

        bool JobImpl::complete() {
            return make_completing(nullptr);
        }

        bool JobImpl::complete_exceptionally(std::exception_ptr exception) {
            return make_completing(new CompletedExceptionally(exception));
        }
    } // namespace coroutines
} // namespace kotlinx