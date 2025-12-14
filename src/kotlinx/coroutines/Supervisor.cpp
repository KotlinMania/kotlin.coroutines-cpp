/**
 * @file Supervisor.cpp
 * @brief Supervisor job implementation
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Supervisor.kt
 *
 * Provides the implementation of SupervisorJob - a job whose children can fail independently.
 */

#include "kotlinx/coroutines/Supervisor.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"

namespace kotlinx {
    namespace coroutines {
        namespace {
            /**
 * Internal implementation of SupervisorJob.
 *
 * Unlike a regular job, a supervisor job does not fail when one of its children fails.
 * The child's failure is isolated and does not propagate to siblings or the supervisor itself.
 */
            class SupervisorJobImpl : public JobImpl {
            public:
                explicit SupervisorJobImpl(std::shared_ptr<struct Job> parent)
                    : JobImpl(parent) {
                }

                /**
     * Override to return false - supervisor jobs do not fail when children fail.
     * This isolates child failures from affecting other children or the supervisor.
     */
                bool child_cancelled(std::exception_ptr cause) override {
                    return false; // Supervisor ignores child failures
                }

                /**
     * Create a SupervisorJobImpl instance.
     */
                static std::shared_ptr<SupervisorJobImpl> create(std::shared_ptr<struct Job> parent) {
                    auto job = std::make_shared<SupervisorJobImpl>(parent);
                    job->init_parent_job(parent);
                    return job;
                }
            };
        } // anonymous namespace

        std::shared_ptr<CompletableJob> make_supervisor_job(std::shared_ptr<struct Job> parent) {
            return SupervisorJobImpl::create(parent);
        }
    } // namespace coroutines
} // namespace kotlinx