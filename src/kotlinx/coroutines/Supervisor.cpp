// port-lint: source Supervisor.kt
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
        std::shared_ptr<CompletableJob> make_supervisor_job(std::shared_ptr<Job> parent) {
            return SupervisorJob(std::move(parent));
        }
    } // namespace coroutines
} // namespace kotlinx