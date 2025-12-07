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

// Forward declarations
using kotlinx::coroutines::channels::Channel;
using kotlinx::coroutines::channels::BufferOverflow;

template <typename T>
struct FusibleFlow : public Flow<T> {
    virtual ~FusibleFlow() = default;
    virtual Flow<T>* fuse(const CoroutineContext& context, int capacity, BufferOverflow on_overflow) {
        return this;
    }
};

template <typename T>
class ChannelFlow : public FusibleFlow<T> {
public:
    ChannelFlow(const CoroutineContext& context, int capacity, BufferOverflow on_overflow)
        : context_(context), capacity_(capacity), on_overflow_(on_overflow) {}

    virtual ~ChannelFlow() = default;

    virtual Flow<T>* drop_channel_operators() { return nullptr; }

    Flow<T>* fuse(const CoroutineContext& context, int capacity, BufferOverflow on_overflow) override;

    void collect(FlowCollector<T>* collector) override;

    int produce_capacity() const;

    virtual std::string additional_to_string_props();
    virtual std::string to_string();

protected:
    virtual ChannelFlow<T>* create(const CoroutineContext& context, int capacity, BufferOverflow on_overflow) = 0;
    virtual void collect_to(ProducerScope<T>* scope) = 0;
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
                            BufferOverflow on_overflow = BufferOverflow::kSuspend)
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
