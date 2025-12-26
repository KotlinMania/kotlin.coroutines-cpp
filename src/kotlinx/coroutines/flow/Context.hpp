#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/Job.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {

using channels::BufferOverflow;

// CancellableFlow is defined in Flow.hpp

/**
 * Implementation of cancellable() operator.
 *
 * Wraps a flow to check cancellation status on each emission.
 */
template<typename T>
class CancellableFlowImpl : public CancellableFlow<T> {
public:
    explicit CancellableFlowImpl(Flow<T>* upstream) : upstream_(upstream) {}

    void* collect(FlowCollector<T>* collector, Continuation<void*>* continuation) override {
        // TODO(suspend-plugin): Properly implement with ensureActive() check
        // For now, delegate to upstream - cancellation check not yet implemented
        return upstream_->collect(collector, continuation);
    }

private:
    Flow<T>* upstream_;
};

/**
 * Buffers flow emissions via channel of a specified capacity.
 */
template<typename T>
Flow<T>* buffer(Flow<T>* flow, int capacity = -1, BufferOverflow onBufferOverflow = BufferOverflow::SUSPEND) {
    // TODO: Implement Fusion or ChannelFlow wrapper
    return flow; 
}

/**
 * Conflates flow emissions. Shortcut for buffer(CONTFLATED).
 */
template<typename T>
Flow<T>* conflate(Flow<T>* flow) {
    // TODO: Implement conflate
    return buffer(flow, 0, BufferOverflow::DROP_OLDEST);
}

/**
 * Changes the context where this flow is executed.
 */
template<typename T>
Flow<T>* flowOn(Flow<T>* flow, std::shared_ptr<CoroutineContext> context) {
    // TODO: Implement flowOn (ChannelFlow operator)
    return flow;
}

/**
 * Returns a flow which checks cancellation status on each emission.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Context.kt
 *
 * If the flow already implements CancellableFlow (like AbstractFlow-based flows),
 * this is a no-op. Otherwise, wraps in CancellableFlowImpl.
 */
template<typename T>
Flow<T>* cancellable(Flow<T>* flow) {
    // Fast-path: already cancellable
    if (dynamic_cast<CancellableFlow<T>*>(flow) != nullptr) {
        return flow;
    }
    // Wrap in CancellableFlowImpl
    return new CancellableFlowImpl<T>(flow);
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
