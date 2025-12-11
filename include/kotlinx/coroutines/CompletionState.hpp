#pragma once
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include "kotlinx/coroutines/Continuation.hpp"

namespace kotlinx {
namespace coroutines {

// NOTE: T should be Unit for coroutines that don't return a value, NOT void.
template<typename T>
void* to_state(Result<T> result) {
    if (result.is_success()) {
        return new T(result.get_or_throw());
    } else {
        return new CompletedExceptionally(result.exception_or_null());
    }
}

// NOTE: T should be Unit for coroutines that don't return a value, NOT void.
template<typename T>
Result<T> recover_result(void* state, Continuation<T>* u_cont) {
    if (auto* completed_exceptionally = dynamic_cast<CompletedExceptionally*>(static_cast<JobState*>(state))) {
        return Result<T>::failure(completed_exceptionally->cause);
    } else {
        return Result<T>::success(*static_cast<T*>(state));
    }
}

} // namespace coroutines
} // namespace kotlinx
