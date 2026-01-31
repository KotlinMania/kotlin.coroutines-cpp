#pragma once
/**
 * @file ChannelFlow.hpp
 * @brief Channel-backed Flow operator skeletons and fusion utilities.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt
 *
 * TODO(semantics): Many suspend points are currently implemented as blocking calls.
 * TODO(suspend-plugin): Migrate suspend logic to plugin-generated state machines.
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/Channels.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/ProducerScope.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/channels/Produce.hpp"
#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"
#include "kotlinx/coroutines/flow/internal/SendingCollector.hpp"
#include <cassert>
#include <limits>
#include <sstream>
#include <string>
#include <memory>
#include <typeinfo>
#include <vector>

namespace kotlinx {
namespace coroutines {
namespace flow {

template <typename T>
void* emit_all(FlowCollector<T>* collector, kotlinx::coroutines::channels::ReceiveChannel<T>* channel, kotlinx::coroutines::Continuation<void*>* continuation);

namespace internal {

// Forward declarations and using statements
using kotlinx::coroutines::channels::Channel;
using kotlinx::coroutines::channels::BufferOverflow;
using kotlinx::coroutines::channels::ProducerScope;
using kotlinx::coroutines::channels::ReceiveChannel;
namespace channels = ::kotlinx::coroutines::channels;
using kotlinx::coroutines::Continuation;
using kotlinx::coroutines::CoroutineScope;
using kotlinx::coroutines::CoroutineContext;
using kotlinx::coroutines::EmptyCoroutineContext;

// Forward declaration moved specific to flow namespace

inline const char* buffer_overflow_to_string(BufferOverflow value) {
    switch (value) {
        case BufferOverflow::SUSPEND:
            return "SUSPEND";
        case BufferOverflow::DROP_OLDEST:
            return "DROP_OLDEST";
        case BufferOverflow::DROP_LATEST:
            return "DROP_LATEST";
        default:
            return "UNKNOWN";
    }
}

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
    virtual Flow<T>* fuse(
        std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
        int capacity = channels::Channel<T>::OPTIONAL_CHANNEL,
        BufferOverflow on_overflow = BufferOverflow::SUSPEND
    ) {
        return this;
    }
};

template <typename T>
class ChannelFlow : public FusibleFlow<T> {
public:
    ChannelFlow(std::shared_ptr<CoroutineContext> context, int capacity, BufferOverflow on_overflow)
        : context_(context), capacity_(capacity), on_overflow_(on_overflow) {}

    virtual ~ChannelFlow() = default;

    virtual Flow<T>* drop_channel_operators() { return nullptr; }

    Flow<T>* fuse(std::shared_ptr<CoroutineContext> context, int capacity, BufferOverflow on_overflow) override;

    void* collect(FlowCollector<T>* collector, Continuation<void*>* continuation) override;

    int produce_capacity() const;

    virtual std::string additional_to_string_props();
    virtual std::string to_string();

    virtual std::shared_ptr<ReceiveChannel<T>> produce_impl(CoroutineScope* scope);

protected:
    virtual ChannelFlow<T>* create(std::shared_ptr<CoroutineContext> context, int capacity, BufferOverflow on_overflow) = 0;
    
    virtual void collect_to(ProducerScope<T>* scope) = 0;

    std::shared_ptr<CoroutineContext> context() const { return context_; }
    int capacity() const { return capacity_; }
    BufferOverflow on_buffer_overflow() const { return on_overflow_; }

private:
    std::shared_ptr<CoroutineContext> context_;
    int capacity_;
    BufferOverflow on_overflow_;
};

template <typename T>
inline Flow<T>* ChannelFlow<T>::fuse(std::shared_ptr<CoroutineContext> context, int capacity, BufferOverflow on_overflow) {
    assert(capacity != Channel<T>::CONFLATED);

    if (!context) context = EmptyCoroutineContext::instance();

    // Kotlin: val newContext = context + this.context  (this.context takes precedence)
    auto new_context = context->operator+(context_);

    int new_capacity;
    BufferOverflow new_overflow;
    if (on_overflow != BufferOverflow::SUSPEND) {
        // Kotlin: overwrite preceding buffering configuration
        new_capacity = capacity;
        new_overflow = on_overflow;
    } else {
        // Kotlin: combine capacities, keep previous overflow strategy
        if (capacity_ == Channel<T>::OPTIONAL_CHANNEL) {
            new_capacity = capacity;
        } else if (capacity == Channel<T>::OPTIONAL_CHANNEL) {
            new_capacity = capacity_;
        } else if (capacity_ == Channel<T>::BUFFERED) {
            new_capacity = capacity;
        } else if (capacity == Channel<T>::BUFFERED) {
            new_capacity = capacity_;
        } else {
            assert(capacity_ >= 0);
            assert(capacity >= 0);

            // Kotlin uses Int overflow to detect "unlimited"; avoid signed overflow in C++.
            long long sum = static_cast<long long>(capacity_) + static_cast<long long>(capacity);
            if (sum > std::numeric_limits<int>::max()) {
                new_capacity = Channel<T>::UNLIMITED;
            } else {
                new_capacity = static_cast<int>(sum);
            }
        }
        new_overflow = on_overflow_;
    }

    if (new_context == context_ && new_capacity == capacity_ && new_overflow == on_overflow_) {
        return this;
    }

    // NOTE: create() returns a raw pointer today; callers are expected to manage lifetime.
    // TODO(semantics): move create/fuse to shared_ptr to avoid leaks when fusion is used.
    return create(new_context, new_capacity, new_overflow);
}

template <typename T>
inline int ChannelFlow<T>::produce_capacity() const {
    // Kotlin: if (capacity == OPTIONAL_CHANNEL) BUFFERED else capacity
    if (capacity_ == Channel<T>::OPTIONAL_CHANNEL) return Channel<T>::BUFFERED;
    return capacity_;
}

template <typename T>
inline std::shared_ptr<ReceiveChannel<T>> ChannelFlow<T>::produce_impl(CoroutineScope* scope) {
    // Kotlin: scope.produce(context, produceCapacity, onBufferOverflow, start = ATOMIC, block = collectToFun)
    return channels::produce<T>(
        scope,
        context_,
        produce_capacity(),
        on_overflow_,
        CoroutineStart::ATOMIC,
        [this](ProducerScope<T>* scope) { collect_to(scope); }
    );
}

template <typename T>
inline void* ChannelFlow<T>::collect(FlowCollector<T>* collector, Continuation<void*>* continuation) {
    // Kotlin:
    // coroutineScope { collector.emitAll(produceImpl(this)) }
    //
    // We don't have coroutineScope lowering here yet; create a minimal scope that carries the current context.
    class SimpleScope : public CoroutineScope {
    public:
        explicit SimpleScope(std::shared_ptr<CoroutineContext> ctx) : ctx_(std::move(ctx)) {}
        std::shared_ptr<CoroutineContext> get_coroutine_context() const override { return ctx_; }

    private:
        std::shared_ptr<CoroutineContext> ctx_;
    };

    auto ctx = continuation ? continuation->get_context() : EmptyCoroutineContext::instance();
    SimpleScope scope(ctx);
    auto channel = produce_impl(&scope);
    return kotlinx::coroutines::flow::emit_all(collector, channel.get(), continuation);
}

template <typename T>
inline std::string ChannelFlow<T>::additional_to_string_props() {
    return "";
}

template <typename T>
inline std::string ChannelFlow<T>::to_string() {
    std::vector<std::string> props;
    auto additional = additional_to_string_props();
    if (!additional.empty()) props.push_back(additional);

    if (context_ && context_ != EmptyCoroutineContext::instance()) {
        std::ostringstream oss;
        oss << "context=" << context_.get();
        props.push_back(oss.str());
    }

    if (capacity_ != Channel<T>::OPTIONAL_CHANNEL) {
        props.push_back("capacity=" + std::to_string(capacity_));
    }

    if (on_overflow_ != BufferOverflow::SUSPEND) {
        props.push_back(std::string("onBufferOverflow=") + buffer_overflow_to_string(on_overflow_));
    }

    std::ostringstream out;
    out << typeid(*this).name() << "[";
    for (size_t i = 0; i < props.size(); ++i) {
        if (i) out << ", ";
        out << props[i];
    }
    out << "]";
    return out.str();
}

template <typename S, typename T>
class ChannelFlowOperator : public ChannelFlow<T> {
public:
    ChannelFlowOperator(std::shared_ptr<Flow<S>> flow,
                        std::shared_ptr<CoroutineContext> context,
                        int capacity,
                        BufferOverflow on_overflow)
        : ChannelFlow<T>(context, capacity, on_overflow), flow_(std::move(flow)) {}

    ~ChannelFlowOperator() override = default;

protected:
    virtual void* flow_collect(FlowCollector<T>* collector, Continuation<void*>* continuation) = 0;

    void collect_to(ProducerScope<T>* scope) override {
        // Kotlin: flowCollect(SendingCollector(scope))
        SendingCollector<T> collector(scope);
        // TODO(semantics): flowCollect is suspend in Kotlin; propagate COROUTINE_SUSPENDED once ChannelFlow is suspend-correct.
        (void)flow_collect(&collector, nullptr);
    }

    void* collect(FlowCollector<T>* collector, Continuation<void*>* continuation) override;

    std::shared_ptr<Flow<S>> upstream() const { return flow_; }

private:
    std::shared_ptr<Flow<S>> flow_;
};

template <typename T>
class ChannelFlowOperatorImpl : public ChannelFlowOperator<T, T> {
public:
    ChannelFlowOperatorImpl(std::shared_ptr<Flow<T>> flow,
                            std::shared_ptr<CoroutineContext> context = nullptr,
                            int capacity = Channel<T>::OPTIONAL_CHANNEL,
                            BufferOverflow on_overflow = BufferOverflow::SUSPEND)
        : ChannelFlowOperator<T, T>(std::move(flow),
                                    context ? std::move(context) : EmptyCoroutineContext::instance(),
                                    capacity,
                                    on_overflow),
          flow_(this->upstream()) {}

protected:
    ChannelFlow<T>* create(std::shared_ptr<CoroutineContext> context, int capacity, BufferOverflow on_overflow) override {
        return new ChannelFlowOperatorImpl<T>(flow_, std::move(context), capacity, on_overflow);
    }

    Flow<T>* drop_channel_operators() override { return flow_.get(); }

    void* flow_collect(FlowCollector<T>* collector, Continuation<void*>* continuation) override { return flow_->collect(collector, continuation); }

private:
    std::shared_ptr<Flow<T>> flow_;
};

/**
 * Kotlin: internal fun <T> Flow<T>.asChannelFlow(): ChannelFlow<T>
 */
template <typename T>
inline std::shared_ptr<ChannelFlow<T>> as_channel_flow(std::shared_ptr<Flow<T>> flow) {
    if (!flow) return nullptr;

    if (auto channel_flow = std::dynamic_pointer_cast<ChannelFlow<T>>(flow)) {
        return channel_flow;
    }

    return std::make_shared<ChannelFlowOperatorImpl<T>>(std::move(flow));
}

} // namespace internal

/**
 * Implementation of produce_in after as_channel_flow is available.
 * See Channels.hpp for documentation.
 */
template <typename T>
inline std::shared_ptr<channels::ReceiveChannel<T>> produce_in(
    std::shared_ptr<Flow<T>> flow,
    CoroutineScope* scope
) {
    auto channel_flow = internal::as_channel_flow(std::move(flow));
    return channel_flow->produce_impl(scope);
}

namespace internal {

template <typename S, typename T>
inline void* ChannelFlowOperator<S, T>::collect(FlowCollector<T>* collector, Continuation<void*>* continuation) {
    // Kotlin fast-path optimization is more complex (newCoroutineContext + undispatched collection).
    // Here we preserve the structure and fall back when we can't be precise.
    if (this->capacity() == Channel<T>::OPTIONAL_CHANNEL && continuation) {
        auto collect_context = continuation->get_context();
        auto new_context = collect_context ? collect_context->operator+(this->context()) : this->context();

        // #1: If the resulting context is the same, collect directly.
        if (new_context == collect_context) {
            return flow_collect(collector, continuation);
        }

        // #2: If we don't need to change the dispatcher, we can go without channels.
        auto old_interceptor = collect_context ? collect_context->get(ContinuationInterceptor::type_key) : nullptr;
        auto new_interceptor = new_context ? new_context->get(ContinuationInterceptor::type_key) : nullptr;
        if (old_interceptor == new_interceptor) {
            // TODO(semantics): implement with_context_undispatched + undispatched collector wrapper.
            return flow_collect(collector, continuation);
        }
    }

    // Slow-path: create the actual channel.
    return ChannelFlow<T>::collect(collector, continuation);
}

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
