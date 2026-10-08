/** Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1423-1451 */
#include "kotlinx/coroutines/JobImpl.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include <utility>

namespace kotlinx::coroutines {

// Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1425-1426
// NOTE(port): create attaches the parent after shared ownership is established.
JobImpl::JobImpl(std::shared_ptr<Job>) : JobSupport(true) {}

// Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1425-1438
std::shared_ptr<JobImpl> JobImpl::create(std::shared_ptr<Job> parent) {
    auto job = std::make_shared<JobImpl>(parent);
    job->init_parent_job(std::move(parent));
    return job;
}

// Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1426-1438
void JobImpl::init_parent_job(std::shared_ptr<Job> parent) {
    JobSupport::init_parent_job(std::move(parent));
    handles_exception_ = handles_exception();
}

// Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1427
bool JobImpl::get_on_cancel_complete() const { return true; }

// Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1438
bool JobImpl::get_handles_exception() const { return handles_exception_; }

// Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1439
bool JobImpl::complete() { return make_completing(nullptr); }

// Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:1440-1441
bool JobImpl::complete_exceptionally(std::exception_ptr exception) {
    return make_completing(new CompletedExceptionally(exception));
}

} // namespace kotlinx::coroutines
