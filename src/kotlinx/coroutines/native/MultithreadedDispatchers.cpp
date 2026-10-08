/**
 * Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt
 */
#include "kotlinx/coroutines/native/MultithreadedDispatchers.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlin/native/concurrent/Worker.hpp"
#include "kotlin/time/TimeSource.hpp"
#include <algorithm>
#include <atomic>

namespace kotlinx::coroutines {
namespace {
// Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:38-54
// Workers cannot cancel queued executeAfter calls. Disposal drops the runnable
// and its reachable objects, leaving only this shell in the worker queue.
class DisposableBlock final : public DisposableHandle {
public:
    // Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:42-43
    explicit DisposableBlock(std::shared_ptr<Runnable> block)
        : disposable_holder_(std::move(block)) {}
    // Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:45-47
    void invoke() {
        if (auto block = disposable_holder_.load()) block->run();
    }
    // Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:49-51
    void dispose() override { disposable_holder_.store(nullptr); }
    // Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:53
    bool is_disposed() const { return disposable_holder_.load() == nullptr; }
private:
    std::atomic<std::shared_ptr<Runnable>> disposable_holder_;
};

// Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:56-65
void run_after_delay(kotlin::native::concurrent::Worker worker,
                     std::shared_ptr<DisposableBlock> block,
                     std::shared_ptr<kotlin::time::TimeMark> target_moment) {
    if (block->is_disposed()) return;
    auto duration_until_target = -target_moment->elapsed_now();
    auto quantum = kotlin::time::milliseconds(100);
    if (duration_until_target.compare_to(quantum) > 0) {
        worker.execute_after(quantum.to_long(kotlin::time::DurationUnit::MICROSECONDS),
            [worker, block, target_moment] { run_after_delay(worker, block, target_moment); });
    } else {
        worker.execute_after(std::max(0LL,
            duration_until_target.to_long(kotlin::time::DurationUnit::MICROSECONDS)),
            [block] { block->invoke(); });
    }
}

// NOTE(port): Existing C++ cancellable implementations carry shared ownership.
// Other raw interface receivers remain borrowed; this binding transfers no
// ownership and does not resume them early when no owner exists.
std::shared_ptr<CancellableContinuation<void>> bind_continuation(
    CancellableContinuation<void>& continuation) {
    if (auto* owned = dynamic_cast<CancellableContinuationImpl<void>*>(&continuation)) {
        if (auto owner = owned->weak_from_this().lock()) return owner;
    }
    return std::shared_ptr<CancellableContinuation<void>>(
        &continuation, [](CancellableContinuation<void>*) {});
}
} // namespace

// Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:20-21
// NOTE(port): The private source field stays concrete in opaque C++ storage.
struct WorkerDispatcher::Impl {
    // Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:21
    explicit Impl(const std::string& name)
        : worker(kotlin::native::concurrent::Worker::start(true, name)) {}
    kotlin::native::concurrent::Worker worker;
};

// Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:20-21
WorkerDispatcher::WorkerDispatcher(const std::string& name)
    : impl_(std::make_unique<Impl>(name)) {}
// NOTE(port): Opaque storage destruction does not replace source close().
WorkerDispatcher::~WorkerDispatcher() = default;

// Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:23-25
void WorkerDispatcher::dispatch(const CoroutineContext&, std::shared_ptr<Runnable> block) const {
    impl_->worker.execute_after(0, [block = std::move(block)] { block->run(); });
}

// Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:27-32
// Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuation.kt:491-496
void WorkerDispatcher::schedule_resume_after_delay(
    long long time_millis, CancellableContinuation<void>& continuation) {
    auto bound = bind_continuation(continuation);
    // NOTE(port): A worker executes the scheduled operation once. Move its
    // continuation lease into that invocation so the completed callback cannot
    // form a shared-owner cycle through the continuation's cancellation handle.
    auto handle = schedule(time_millis, std::shared_ptr<Runnable>(make_runnable(
        [this, bound = std::move(bound)]() mutable {
            auto continuation = std::move(bound);
            continuation->resume_undispatched(this);
        })));
    // NOTE(port): Own the actual handle in the cancellation callback. The
    // existing raw-handle helper cannot express this source owning capture.
    continuation.invoke_on_cancellation([handle](std::exception_ptr) { handle->dispose(); });
}

// Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:34-35
std::shared_ptr<DisposableHandle> WorkerDispatcher::invoke_on_timeout(
    long long time_millis, std::shared_ptr<Runnable> block, const CoroutineContext&) {
    return schedule(time_millis, std::move(block));
}

// Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:37-71
std::shared_ptr<DisposableHandle> WorkerDispatcher::schedule(
    long long time_millis, std::shared_ptr<Runnable> block) {
    auto disposable_block = std::make_shared<DisposableBlock>(std::move(block));
    // Transliterated from: libraries/stdlib/src/kotlin/time/TimeSource.kt:51-52,70-85
    // NOTE(port): Box the source value mark for the TimeMark interface captured
    // by the recursive worker extension.
    auto now = std::unique_ptr<kotlin::time::TimeSource::Monotonic::ValueTimeMark>(
        kotlin::time::TimeSource::Monotonic::instance().mark_now());
    auto target_moment = std::shared_ptr<kotlin::time::TimeMark>(
        now->plus(kotlin::time::milliseconds(time_millis)));
    run_after_delay(impl_->worker, disposable_block, std::move(target_moment));
    return disposable_block;
}

// Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:73-75
void WorkerDispatcher::close() { impl_->worker.request_termination().result(); }
} // namespace kotlinx::coroutines
