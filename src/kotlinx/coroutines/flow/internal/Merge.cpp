#include "kotlinx/coroutines/flow/internal/Merge.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include <string>

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

// Helper functions that do not depend on template parameters
// moved to .cpp to reduce header bloat and follow project structure.

std::string format_concurrency_props(int concurrency) {
    return "concurrency=" + std::to_string(concurrency);
}

void acquire_semaphore_permit(Job* job, kotlinx::coroutines::sync::Semaphore& semaphore) {
    if (job) kotlinx::coroutines::ensure_active(*job);
    semaphore.acquire();
}

void release_semaphore_permit(kotlinx::coroutines::sync::Semaphore& semaphore) {
    semaphore.release();
}

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx