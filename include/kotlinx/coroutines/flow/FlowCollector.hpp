#pragma once

namespace kotlinx {
namespace coroutines {
namespace flow {

template <typename T>
struct FlowCollector {
    virtual ~FlowCollector() = default;
    
    /**
     * Collects the value emitted by the upstream.
     * This method is a suspending function in Kotlin. 
     * In C++, implementations must handle suspension if using C++20 coroutines or a custom runtime.
     */
    virtual void emit(T value) = 0;
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
