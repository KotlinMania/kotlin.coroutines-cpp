// port-lint: source kotlinx-coroutines-core/common/src/flow/internal/Merge.kt
/** Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt */
#include "kotlinx/coroutines/flow/internal/Merge.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"

namespace kotlinx::coroutines::flow::internal {
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:23-33
[[clang::annotate("suspend")]]
void* transform_latest_emit(std::shared_ptr<Job> previous_flow,
    std::function<void()> launch_next, std::shared_ptr<Continuation<void*>> completion) {
    if (previous_flow) {
        previous_flow->cancel(std::make_exception_ptr(ChildCancelledException()));
        previous_flow->join();
    }
    launch_next();
    return nullptr;
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:55-70
[[clang::annotate("suspend")]]
void* acquire_and_launch_merge_inner(std::shared_ptr<Job> job,
    std::shared_ptr<Semaphore> semaphore, std::function<void()> launch_inner,
    std::shared_ptr<Continuation<void*>> completion) {
    /*
     * We launch a coroutine on each emitted element and the only potential
     * suspension point in this collector is semaphore.acquire that rarely suspends,
     * so we manually check for cancellation to propagate it to the upstream in time.
     */
    if (job) ensure_active(*job);
    dsl::suspend(semaphore->acquire(completion.get()));
    launch_inner();
    return nullptr;
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:64-68
[[clang::annotate("suspend")]]
void* collect_merge_child(std::function<void*(Continuation<void*>*)> collect,
    std::shared_ptr<Semaphore> semaphore, std::shared_ptr<Continuation<void*>> completion) {
    try {
        dsl::suspend(collect(completion.get()));
    } catch (...) {
        semaphore->release(); // Release concurrency permit
        throw;
    }
    semaphore->release(); // Release concurrency permit
    return nullptr;
}
} // namespace kotlinx::coroutines::flow::internal
