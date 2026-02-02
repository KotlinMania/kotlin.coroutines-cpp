#pragma once
// port-lint: source flow/FlowCollector.kt

#include "kotlinx/coroutines/Continuation.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {

template <typename T>
struct FlowCollector {
    virtual ~FlowCollector() = default;
    
    /**
     * Collects the value emitted by the upstream.
     *
     * This method is called by flow implementations to emit values to the collector.
     * In the intended design, this should be a suspending function that provides
     * backpressure by pausing when the collector cannot keep up.
     *
     * @param value The value to emit
     * @param continuation The continuation for suspension support
     * @return Pointer to suspension state/result
     *
     * @note **CURRENT LIMITATION**: While this method signature supports suspension,
     *       most implementations do not actually suspend. This breaks backpressure
     *       guarantees and can lead to unbounded memory usage or dropped values.
     *
     * @note **INTENDED BEHAVIOR**: Should suspend when downstream is not ready,
     *       providing proper flow control and preventing buffer overflow.
     *
     * @throws Exception Any exception thrown by the collector will propagate
     *                   upstream and stop further emissions.
     *
     * ### Thread Safety
     * FlowCollector implementations are not thread-safe by default. Emissions
     * must be serialized unless the collector explicitly documents thread safety.
     */
    virtual void* emit(T value, Continuation<void*>* continuation) = 0;
};

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
