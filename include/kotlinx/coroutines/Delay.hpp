#pragma once
#include "kotlinx/coroutines/CancellableContinuation.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include "kotlinx/coroutines/DisposableHandle.hpp"
#include <memory>

namespace kotlinx {
namespace coroutines {

class Delay {
public:
    virtual ~Delay() = default;

    /**
     * Schedules a resume of the continuation after the specified delay.
     */
    virtual void schedule_resume_after_delay(long long time_millis, CancellableContinuation<void>& continuation) = 0;

    /**
     * Schedules execution of the block after the specified delay.
     */
    virtual std::shared_ptr<DisposableHandle> invoke_on_timeout(long long time_millis, std::shared_ptr<Runnable> block, const CoroutineContext& context) = 0;
};

} // namespace coroutines
} // namespace kotlinx
