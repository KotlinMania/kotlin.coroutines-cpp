// port-lint: source flow/internal/Merge.kt
/** Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt */
#include "kotlinx/coroutines/flow/internal/Merge.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"

namespace kotlinx::coroutines::flow::internal {
namespace {
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:23-33
class TransformLatestEmitContinuation final : public ContinuationImpl {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:23-33
    TransformLatestEmitContinuation(std::shared_ptr<Job> previous_flow,
        std::function<void()> launch_next, std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)), previous_flow_(std::move(previous_flow)),
          launch_next_(std::move(launch_next)) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:23-33
    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)
        if (previous_flow_) {
            previous_flow_->cancel(std::make_exception_ptr(ChildCancelledException()));
            coroutine_yield(this, previous_flow_->join(this));
        }
        launch_next_();
        coroutine_end(this)
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:23-33
    // NOTE(port): Release terminated source captures, even if a caller retains the frame.
    void release_intercepted() override {
        previous_flow_.reset();
        launch_next_ = {};
        ContinuationImpl::release_intercepted();
    }
private:
    void* _label = nullptr;
    std::shared_ptr<Job> previous_flow_;
    std::function<void()> launch_next_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:55-70
class MergeEmitContinuation final : public ContinuationImpl {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:55-70
    MergeEmitContinuation(std::shared_ptr<Job> job, std::shared_ptr<Semaphore> semaphore,
        std::function<void()> launch_inner, std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)), job_(std::move(job)),
          semaphore_(std::move(semaphore)), launch_inner_(std::move(launch_inner)) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:55-70
    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)
        /*
         * We launch a coroutine on each emitted element and the only potential
         * suspension point in this collector is semaphore.acquire that rarely suspends,
         * so we manually check for cancellation to propagate it to the upstream in time.
         */
        if (job_) ensure_active(*job_);
        coroutine_yield(this, semaphore_->acquire(this));
        launch_inner_();
        coroutine_end(this)
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:55-70
    // NOTE(port): Shared arguments keep their owners; borrowed scopes remain borrowed.
    void release_intercepted() override {
        job_.reset();
        semaphore_.reset();
        launch_inner_ = {};
        ContinuationImpl::release_intercepted();
    }
private:
    void* _label = nullptr;
    std::shared_ptr<Job> job_;
    std::shared_ptr<Semaphore> semaphore_;
    std::function<void()> launch_inner_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:64-68
class MergeChildContinuation final : public ContinuationImpl {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:64-68
    MergeChildContinuation(std::function<void*(Continuation<void*>*)> collect,
        std::shared_ptr<Semaphore> semaphore, std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)), collect_(std::move(collect)),
          semaphore_(std::move(semaphore)) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:64-68
    void retain() { self_ref_ = shared_from_this(); }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:64-68
    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)
            coroutine_yield(this, std::function(collect_)(this));
        } catch (...) {
            semaphore_->release(); // Release concurrency permit
            throw;
        }
        semaphore_->release(); // Release concurrency permit
        coroutine_end(this)
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:64-68
    // NOTE(port): Clear source captures on immediate/resumed success or failure.
    void release_intercepted() override {
        auto self = std::move(self_ref_);
        collect_ = {};
        semaphore_.reset();
        ContinuationImpl::release_intercepted();
    }
private:
    void* _label = nullptr;
    std::function<void*(Continuation<void*>*)> collect_;
    std::shared_ptr<Semaphore> semaphore_;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:23-33
void* transform_latest_emit(std::shared_ptr<Job> previous_flow,
    std::function<void()> launch_next, std::shared_ptr<Continuation<void*>> completion) {
    auto frame = std::make_shared<TransformLatestEmitContinuation>(
        std::move(previous_flow), std::move(launch_next), std::move(completion));
    return frame->start(Result<void*>::success(nullptr));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:55-70
void* acquire_and_launch_merge_inner(std::shared_ptr<Job> job,
    std::shared_ptr<Semaphore> semaphore, std::function<void()> launch_inner,
    std::shared_ptr<Continuation<void*>> completion) {
    auto frame = std::make_shared<MergeEmitContinuation>(
        std::move(job), std::move(semaphore), std::move(launch_inner), std::move(completion));
    return frame->start(Result<void*>::success(nullptr));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:64-68
void* collect_merge_child(std::function<void*(Continuation<void*>*)> collect,
    std::shared_ptr<Semaphore> semaphore, std::shared_ptr<Continuation<void*>> completion) {
    auto frame = std::make_shared<MergeChildContinuation>(
        std::move(collect), std::move(semaphore), std::move(completion));
    frame->retain();
    return frame->start(Result<void*>::success(nullptr));
}
} // namespace kotlinx::coroutines::flow::internal
