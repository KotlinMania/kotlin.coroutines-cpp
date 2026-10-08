// port-lint: source kotlinx-coroutines-core/common/src/Supervisor.kt
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
 * Upstream:
 *   private class SupervisorJobImpl(parent: Job?) : JobImpl(parent) {
 *       override fun childCancelled(cause: Throwable): Boolean = false
 *   }
 */
// Transliterated from: kotlinx-coroutines-core/common/src/Supervisor.kt:60-62
class SupervisorJobImpl : public JobImpl {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/Supervisor.kt:60-60
    explicit SupervisorJobImpl(std::shared_ptr<Job> parent) : JobImpl(std::move(parent)) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/Supervisor.kt:61-61
    bool child_cancelled(std::exception_ptr /*cause*/) override { return false; }
};

        } // namespace

        // Transliterated from: kotlinx-coroutines-core/common/src/Supervisor.kt:27-27
        std::shared_ptr<CompletableJob> SupervisorJob(std::shared_ptr<Job> parent) {
            return std::make_shared<SupervisorJobImpl>(std::move(parent));
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/Supervisor.kt:27-27
        std::shared_ptr<CompletableJob> make_supervisor_job(std::shared_ptr<Job> parent) {
            return SupervisorJob(std::move(parent));
        }
    } // namespace coroutines
} // namespace kotlinx