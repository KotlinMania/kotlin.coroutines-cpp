/**
 * Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt
 */
#ifndef KOTLINX_COROUTINES_NATIVE_MULTITHREADED_DISPATCHERS_HPP_
#define KOTLINX_COROUTINES_NATIVE_MULTITHREADED_DISPATCHERS_HPP_
#include "kotlinx/coroutines/CloseableCoroutineDispatcher.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include <memory>
#include <string>

namespace kotlinx::coroutines {
// Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:20-76
class WorkerDispatcher : public CloseableCoroutineDispatcher, public Delay {
public:
    // Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:20-21
    explicit WorkerDispatcher(const std::string& name);
    // NOTE(port): Opaque C++ field destruction is out of line; close still
    // performs the source's explicit termination request and blocking join.
    ~WorkerDispatcher() override;
    WorkerDispatcher(const WorkerDispatcher&) = delete;
    WorkerDispatcher& operator=(const WorkerDispatcher&) = delete;
    // Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:23-25
    void dispatch(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override;
    // Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:27-32
    void schedule_resume_after_delay(long long time_millis,
                                     CancellableContinuation<void>& continuation) override;
    // Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:34-35
    std::shared_ptr<DisposableHandle> invoke_on_timeout(
        long long time_millis, std::shared_ptr<Runnable> block,
        const CoroutineContext& context) override;
    // Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:73-75
    void close() override;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    // Transliterated from: kotlinx-coroutines-core/native/src/MultithreadedDispatchers.kt:37-71
    std::shared_ptr<DisposableHandle> schedule(long long time_millis,
                                              std::shared_ptr<Runnable> block);
};
} // namespace kotlinx::coroutines
#endif
