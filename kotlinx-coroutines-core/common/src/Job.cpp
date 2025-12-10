/**
 * @file Job.cpp
 * @brief Implementation of Job factory and related functions.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/Job.kt
 *
 * NOTE: The detailed API documentation, KDocs, and class definitions are located
 * in the companion header file: `include/kotlinx/coroutines/Job.hpp`.
 */

#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include <stdexcept>

namespace kotlinx {
namespace coroutines {

// -------------------- Factory function implementation --------------------

/**
 * Creates a job object in an active state.
 * See Job.hpp for full documentation.
 */
std::shared_ptr<CompletableJob> make_job(std::shared_ptr<struct Job> parent) {
    return JobImpl::create(parent);
}

} // namespace coroutines
} // namespace kotlinx
