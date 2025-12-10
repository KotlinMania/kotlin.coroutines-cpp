#pragma once
#include "kotlinx/coroutines/Result.hpp"
#include "kotlinx/coroutines/CompletedExceptionally.hpp"
#include "kotlinx/coroutines/Continuation.hpp"

namespace kotlinx {
namespace coroutines {

template<typename T>
void* to_state(Result<T> result) {
    if (result.is_success()) {
        if constexpr (std::is_void_v<T>) return nullptr;
        else return new T(result.get_or_throw());
    } else {
        return new CompletedExceptionally(result.exception_or_null());
    }
}

template<typename T>
Result<T> recover_result(void* state, Continuation<T>* u_cont) {
    if (auto* completed_exceptionally = dynamic_cast<CompletedExceptionally*>(static_cast<JobState*>(state))) { // cast void* to JobState*? 
        // JobSupport uses void* for state. 
        // If state represents CompletedExceptionally, it is subclass of JobState (if used).
        // But void* in JobSupport might be anything.
        // Assuming state is valid object pointer.
        return Result<T>::failure(completed_exceptionally->cause);
    } else {
        if constexpr (std::is_void_v<T>) return Result<T>::success();
        else return Result<T>::success(*static_cast<T*>(state));
    }
}

} // namespace coroutines
} // namespace kotlinx
