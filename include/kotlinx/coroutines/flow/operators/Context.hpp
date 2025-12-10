#pragma once
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {

using channels::BufferOverflow;

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
Flow<T>* flowOn(Flow<T>* flow, CoroutineContext context) {
    // TODO: Implement flowOn (ChannelFlow operator)
    return flow;
}

/**
 * Returns a flow which checks cancellation status on each emission.
 */
template<typename T>
Flow<T>* cancellable(Flow<T>* flow) {
    // TODO: Implement cancellable wrapper
    return flow;
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
