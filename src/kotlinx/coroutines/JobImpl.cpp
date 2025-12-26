/**
 * @file JobImpl.cpp
 * @brief Implementation of JobImpl.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt (lines 1423-1451)
 */

#include "kotlinx/coroutines/JobImpl.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"

namespace kotlinx {
    namespace coroutines {
        JobImpl::JobImpl(std::shared_ptr<Job> parent) : JobSupport(true) {
            // Kotlin: init { initParentJob(parent) }
            // C++ can't call init_parent_job here - shared_from_this() not yet available.
            // Use create() factory which calls init_parent_job after construction.
        }

        std::shared_ptr<JobImpl> JobImpl::create(std::shared_ptr<Job> parent) {
            auto job = std::make_shared<JobImpl>(parent);
            job->init_parent_job(parent);
            return job;
        }

        void JobImpl::init_parent_job(std::shared_ptr<Job> parent) {
            // Delegate to JobSupport's protected init_parent_job
            JobSupport::init_parent_job(parent);
        }

        bool JobImpl::complete() {
            // Transliterated from: override fun complete() = makeCompleting(Unit)
            return make_completing(nullptr);
        }

        bool JobImpl::complete_exceptionally(std::exception_ptr exception) {
            // Transliterated from: override fun completeExceptionally(exception: Throwable): Boolean =
            //     makeCompleting(CompletedExceptionally(exception))
            return make_completing(new CompletedExceptionally(exception));
        }

        bool JobImpl::get_handles_exception() const {
            // Lazy computation with caching
            // Transliterated from: override val handlesException: Boolean = handlesException()
            if (!handles_exception_computed_) {
                handles_exception_cached_ = compute_handles_exception();
                handles_exception_computed_ = true;
            }
            return handles_exception_cached_;
        }

        bool JobImpl::compute_handles_exception() const {
            // Transliterated from: private fun handlesException(): Boolean (JobSupport.kt:1444-1449)
            //
            // Check whether parent is able to handle exceptions as well.
            // With this check, an exception in that pattern will be handled once:
            // ```
            // launch {
            //     val child = Job(coroutineContext[Job])
            //     launch(child) { throw ... }
            // }
            // ```
            auto parent = get_parent();
            if (!parent) return false;

            // Walk up the parent chain to find if any parent handles exceptions
            std::shared_ptr<Job> current = parent;
            while (current) {
                // If the current job handles exceptions, return true
                // NOTE: We need to check if it's a JobSupport to access get_handles_exception
                // TODO(port): This cast chain mirrors Kotlin's (parentHandle as? ChildHandleNode)?.job
                if (auto* job_support = dynamic_cast<JobSupport*>(current.get())) {
                    if (job_support->get_handles_exception()) {
                        return true;
                    }
                    // Move to parent's parent
                    current = job_support->get_parent();
                } else {
                    // Not a JobSupport, can't continue chain
                    break;
                }
            }
            return false;
        }
    } // namespace coroutines
} // namespace kotlinx