#pragma once

#include "kotlinx/coroutines/Continuation.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {

template <typename T>
struct FlowCollector {
    virtual ~FlowCollector() = default;
    
    /**
     * Collects the value emitted by the upstream.
     * This method is a suspending function in Kotlin. 
     * In C++, we use the standard suspend signature returning void* (state/result).
     */
    virtual void* emit(T value, Continuation<void*>* continuation) = 0;
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
