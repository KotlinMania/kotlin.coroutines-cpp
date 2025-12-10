#pragma once
#include "kotlinx/coroutines/core_fwd.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/ProducerScope.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include <string>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

// Forward declarations and using statements
using kotlinx::coroutines::channels::Channel;
using kotlinx::coroutines::channels::BufferOverflow;
using kotlinx::coroutines::channels::ProducerScope;
using kotlinx::coroutines::channels::ReceiveChannel;

/**
 * Operators that can fuse with **downstream** [buffer] and [flowOn] operators implement this interface.
 *
 * @suppress **This an internal API and should not be used from general code.**
 */
template <typename T>
struct FusibleFlow : public Flow<T> {
    virtual ~FusibleFlow() = default;

    /**
     * This function is called by [flowOn] (with context) and [buffer] (with capacity) operators
     * that are applied to this flow. Should not be used with [capacity] of [Channel.CONFLATED]
     * (it shall be desugared to `capacity = 0, onBufferOverflow = DROP_OLDEST`).
     */
    virtual Flow<T>* fuse(const CoroutineContext& context, int capacity, BufferOverflow on_overflow) {
        return this;
    }
};

/**
 * Operators that use channels as their "output" extend this `ChannelFlow` and are always fused with each other.
 * This class servers as a skeleton implementation of [FusibleFlow] and provides other cross-cutting
 * methods like ability to [produceIn] the corresponding flow, thus making it
 * possible to directly use the backing channel if it exists (hence the `ChannelFlow` name).
 *
 * @suppress **This an internal API and should not be used from general code.**
 */
template <typename T>
class ChannelFlow : public FusibleFlow<T> {
public:
    ChannelFlow(const CoroutineContext& context, int capacity, BufferOverflow on_overflow)
        : context_(context), capacity_(capacity), on_overflow_(on_overflow) {}

    virtual ~ChannelFlow() = default;

    /**
     * When this [ChannelFlow] implementation can work without a channel (supports [Channel.OPTIONAL_CHANNEL]),
     * then it should return a non-null value from this function, so that a caller can use it without the effect of
     * additional [flowOn] and [buffer] operators, by incorporating its
     * [context], [capacity], and [onBufferOverflow] into its own implementation.
     */
    virtual Flow<T>* drop_channel_operators() { return nullptr; }

    Flow<T>* fuse(const CoroutineContext& context, int capacity, BufferOverflow on_overflow) override;

    void collect(FlowCollector<T>* collector) override;

    int produce_capacity() const;

    virtual std::string additional_to_string_props();
    virtual std::string to_string();

protected:
    virtual ChannelFlow<T>* create(const CoroutineContext& context, int capacity, BufferOverflow on_overflow) = 0;
    
    virtual void collect_to(ProducerScope<T>* scope) = 0;

    /**
     * Here we use ATOMIC start for a reason (#1825).
     * NB: [produceImpl] is used for [flowOn].
     * For non-atomic start it is possible to observe the situation,
     * where the pipeline after the [flowOn] call successfully executes (mostly, its `onCompletion`)
     * handlers, while the pipeline before does not, because it was cancelled during its dispatch.
     * Thus `onCompletion` and `finally` blocks won't be executed and it may lead to a different kinds of memory leaks.
     */
    virtual ReceiveChannel<T>* produce_impl(CoroutineScope* scope);

    const CoroutineContext& context() const { return context_; }
    int capacity() const { return capacity_; }
    BufferOverflow on_buffer_overflow() const { return on_overflow_; }

private:
    CoroutineContext context_;
    int capacity_;
    BufferOverflow on_overflow_;
};

template <typename S, typename T>
class ChannelFlowOperator : public ChannelFlow<T> {
public:
    ChannelFlowOperator(Flow<S>* flow, const CoroutineContext& context, int capacity, BufferOverflow on_overflow)
        : ChannelFlow<T>(context, capacity, on_overflow), flow_(flow) {}

    ~ChannelFlowOperator() override = default;

protected:
    virtual void flow_collect(FlowCollector<T>* collector) {}

    void collect_to(ProducerScope<T>* scope) override {}

    void collect(FlowCollector<T>* collector) override;

    Flow<S>* upstream() const { return flow_; }

private:
    Flow<S>* flow_;
};

template <typename T>
class ChannelFlowOperatorImpl : public ChannelFlowOperator<T, T> {
public:
    ChannelFlowOperatorImpl(Flow<T>* flow,
                            const CoroutineContext& context = CoroutineContext{},
                            int capacity = Channel<int>::OPTIONAL_CHANNEL,
                            BufferOverflow on_overflow = BufferOverflow::SUSPEND)
        : ChannelFlowOperator<T, T>(flow, context, capacity, on_overflow), flow_(flow) {}

protected:
    ChannelFlow<T>* create(const CoroutineContext& context, int capacity, BufferOverflow on_overflow) override {
        return new ChannelFlowOperatorImpl<T>(flow_, context, capacity, on_overflow);
    }

    Flow<T>* drop_channel_operators() override { return flow_; }

    void flow_collect(FlowCollector<T>* collector) override {
        if (flow_) flow_->collect(collector);
    }

private:
    Flow<T>* flow_;
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
