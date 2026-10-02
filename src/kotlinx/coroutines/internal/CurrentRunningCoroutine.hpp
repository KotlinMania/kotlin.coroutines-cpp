#pragma once

#include "kotlinx/coroutines/Continuation.hpp"
#include <memory>

namespace kotlinx::coroutines::internal {

struct CurrentRunningCoroutine {
    static inline thread_local std::shared_ptr<Continuation<void*>> current = nullptr;
    static inline thread_local bool suspended = false;
};

struct CurrentRunningCoroutineGuard {
    std::shared_ptr<Continuation<void*>> prev_cont;
    bool prev_suspended;

    explicit CurrentRunningCoroutineGuard(std::shared_ptr<Continuation<void*>> cont)
        : prev_cont(CurrentRunningCoroutine::current),
          prev_suspended(CurrentRunningCoroutine::suspended) {
        CurrentRunningCoroutine::current = cont;
        CurrentRunningCoroutine::suspended = false;
    }

    ~CurrentRunningCoroutineGuard() {
        CurrentRunningCoroutine::current = prev_cont;
        CurrentRunningCoroutine::suspended = prev_suspended;
    }
};

} // namespace kotlinx::coroutines::internal
