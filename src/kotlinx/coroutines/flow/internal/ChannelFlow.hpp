#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt
 *
 * Kotlin file header (translated):
 *   package kotlinx.coroutines.flow.internal
 *
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/internal/Scopes.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
// Note: Channels.hpp includes ChannelFlow.hpp, so we use forward declarations instead
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/ProducerScope.hpp"
#include "kotlinx/coroutines/channels/BufferOverflow.hpp"
#include "kotlinx/coroutines/channels/Produce.hpp"
#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"
#include "kotlinx/coroutines/flow/internal/SendingCollector.hpp"
#include "kotlinx/coroutines/flow/internal/NopCollector.hpp"
#include "kotlinx/coroutines/internal/ThreadContext.hpp"
#include <cassert>
#include <limits>
#include <sstream>
#include <string>
#include <memory>
#include <optional>
#include <typeinfo>
#include <vector>

namespace kotlinx::coroutines {
// Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:38-40
std::shared_ptr<CoroutineContext> new_coroutine_context(
    std::shared_ptr<CoroutineContext> base_context, std::shared_ptr<CoroutineContext> added_context);
}

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:117-120
// and kotlinx-coroutines-core/common/src/CoroutineScope.kt:280-288
void* collect_in_scope(
    std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block,
    Continuation<void*>* completion);

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:54-56,118-121,144-148,151-152
// NOTE(port): Typed bindings carry the source arguments into the owning
// suspend entry. The Clang frontend generates its frame and lifetime cleanup.
void* collect_channel_flow(
    std::function<void*(Continuation<void*>*)> collect,
    std::shared_ptr<Continuation<void*>> completion);

// Forward declarations and using statements
using kotlinx::coroutines::channels::Channel;
using kotlinx::coroutines::channels::BufferOverflow;
using kotlinx::coroutines::channels::ProducerScope;
using kotlinx::coroutines::channels::ReceiveChannel;
namespace channels = ::kotlinx::coroutines::channels;
using kotlin::coroutines::Continuation;
using kotlinx::coroutines::CoroutineScope;
using kotlin::coroutines::CoroutineContext;
using kotlin::coroutines::EmptyCoroutineContext;

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
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:20-31
struct FusibleFlow : public Flow<T> {
    virtual ~FusibleFlow() = default;

    /**
     * This function is called by [flowOn] (with context) and [buffer] (with capacity) operators
     * that are applied to this flow. Should not be used with [capacity] of [Channel.CONFLATED]
     * (it shall be desugared to `capacity = 0, onBufferOverflow = DROP_OLDEST`).
     */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:26-30
    virtual Flow<T>* fuse(
        std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
        int capacity = channels::Channel<T>::OPTIONAL_CHANNEL,
        BufferOverflow on_overflow = BufferOverflow::SUSPEND
    ) = 0;
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
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:42-133
class ChannelFlow : public FusibleFlow<T> {
public:
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:42-52
    ChannelFlow(std::shared_ptr<CoroutineContext> context, int capacity, BufferOverflow on_overflow)
        : context_(std::move(context)), capacity_(capacity), on_overflow_(on_overflow) {
        // CONFLATED must be desugared to 0, DROP_OLDEST by callers.
        assert(capacity != Channel<T>::CONFLATED);
    }

    virtual ~ChannelFlow() = default;

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:67-67
    /**
     * When this ChannelFlow implementation can work without a channel (supports
     * channels::Channel<T>::OPTIONAL_CHANNEL), return a non-null value so that a
     * caller can use it without additional flow_on and buffer operators, by
     * incorporating its context, capacity and on_buffer_overflow into its own
     * implementation.
     */
    virtual Flow<T>* drop_channel_operators() { return nullptr; }

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:69-100
    // NOTE(port): C++ overrides must repeat the defaults inherited in Kotlin.
    Flow<T>* fuse(
        std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
        int capacity = Channel<T>::OPTIONAL_CHANNEL,
        BufferOverflow on_overflow = BufferOverflow::SUSPEND) override;

    // Shared code to create a suspend lambda from collect_to in one place.
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:54-56
    std::function<void*(ProducerScope<T>*, std::shared_ptr<Continuation<void*>>)>
    get_collect_to_fun();

    void* collect(FlowCollector<T>* collector, Continuation<void*>* continuation) override;

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:58-59
    int produce_capacity() const;

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:122-122
    virtual std::optional<std::string> additional_to_string_props();
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:125-132
    virtual std::string to_string();

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:114-115
    /**
     * Here we use ATOMIC start for a reason (#1825).
     * NB: produce_impl is used for flow_on.
     * For non-atomic start it is possible to observe the situation where the
     * pipeline after flow_on successfully executes its handlers (mostly
     * on_completion), while the pipeline before does not, because it was
     * cancelled during its dispatch. Thus on_completion and finally blocks
     * would not execute, which may lead to memory leaks.
     */
    virtual std::shared_ptr<ReceiveChannel<T>> produce_impl(CoroutineScope* scope);

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:44-44
    std::shared_ptr<CoroutineContext> context() const { return context_; }
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:46-46
    int capacity() const { return capacity_; }
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:48-48
    BufferOverflow on_buffer_overflow() const { return on_overflow_; }

protected:
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:102-102
    virtual ChannelFlow<T>* create(std::shared_ptr<CoroutineContext> context, int capacity, BufferOverflow on_overflow) = 0;
    
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:104-104
    virtual void* collect_to(ProducerScope<T>* scope,
                             std::shared_ptr<Continuation<void*>> completion) = 0;

private:
    // upstream context
    const std::shared_ptr<CoroutineContext> context_;
    // buffer capacity between upstream and downstream context
    const int capacity_;
    // buffer overflow strategy
    const BufferOverflow on_overflow_;
};

template <typename T>
std::shared_ptr<ChannelFlow<T>> as_channel_flow(std::shared_ptr<Flow<T>> flow);

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx

#include "kotlinx/coroutines/flow/Channels.hpp"

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

template <typename T>
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:69-100
inline Flow<T>* ChannelFlow<T>::fuse(std::shared_ptr<CoroutineContext> context, int capacity, BufferOverflow on_overflow) {
    assert(capacity != Channel<T>::CONFLATED);

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

    if (new_context->equals(context_.get()) && new_capacity == capacity_ && new_overflow == on_overflow_) {
        return this;
    }

    // create() returns a raw pointer matching the upstream `fun create(...): ChannelFlow<T>`
    // signature. Callers in this port wrap the result in shared_ptr at the operator entry
    // sites (see flow_on/buffer in flow/Context.hpp).
    return create(new_context, new_capacity, new_overflow);
}

template <typename T>
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:58-59
inline int ChannelFlow<T>::produce_capacity() const {
    // Kotlin: if (capacity == OPTIONAL_CHANNEL) BUFFERED else capacity
    if (capacity_ == Channel<T>::OPTIONAL_CHANNEL) return Channel<T>::BUFFERED;
    return capacity_;
}

template <typename T>
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:54-56
inline std::function<void*(ProducerScope<T>*, std::shared_ptr<Continuation<void*>>)>
ChannelFlow<T>::get_collect_to_fun() {
    // NOTE(port): The source lambda captures its actual receiver. Preserve an
    // existing owner through queued start and suspension; raw receivers stay borrowed.
    auto owner = this->weak_from_this().lock();
    return [this, owner = std::move(owner)](ProducerScope<T>* scope,
                                          std::shared_ptr<Continuation<void*>> completion) -> void* {
        return collect_channel_flow(
            [this, owner, scope](Continuation<void*>* frame) {
                return collect_to(scope, kotlinx::coroutines::internal::retain_continuation(frame));
            }, std::move(completion));
    };
}

template <typename T>
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:114-115
inline std::shared_ptr<ReceiveChannel<T>> ChannelFlow<T>::produce_impl(CoroutineScope* scope) {
    return channels::produce<T>(
        scope, context_, produce_capacity(), on_overflow_, CoroutineStart::ATOMIC,
        get_collect_to_fun());
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:117-120
template <typename T>
inline void* ChannelFlow<T>::collect(FlowCollector<T>* collector, Continuation<void*>* continuation) {
    return collect_in_scope([this, collector, owner = this->weak_from_this().lock()](
        CoroutineScope* scope, std::shared_ptr<Continuation<void*>> completion) -> void* {
        auto channel = produce_impl(scope);
        return collect_channel_flow(
            [collector, channel = std::move(channel), owner](Continuation<void*>* frame) {
                return kotlinx::coroutines::flow::emit_all(collector, channel.get(), frame);
            }, std::move(completion));
    }, continuation);
}

template <typename T>
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:122-122
inline std::optional<std::string> ChannelFlow<T>::additional_to_string_props() {
    return std::nullopt;
}

template <typename T>
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:125-132
inline std::string ChannelFlow<T>::to_string() {
    std::vector<std::string> props;
    auto additional = additional_to_string_props();
    if (additional) props.push_back(*additional);

    if (context_ != EmptyCoroutineContext::instance()) {
        props.push_back("context=" + context_->to_string());
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
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:136-174
class ChannelFlowOperator : public ChannelFlow<T> {
public:
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:136-141
    ChannelFlowOperator(std::shared_ptr<Flow<S>> flow,
                        std::shared_ptr<CoroutineContext> context,
                        int capacity,
                        BufferOverflow on_overflow)
        : ChannelFlow<T>(context, capacity, on_overflow), flow_(std::move(flow)) {}

    ~ChannelFlowOperator() override = default;

protected:
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:142-142
    virtual void* flow_collect(FlowCollector<T>* collector, Continuation<void*>* continuation) = 0;

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:151-152
    void* collect_to(ProducerScope<T>* scope,
                     std::shared_ptr<Continuation<void*>> completion) override {
        auto collector = std::make_shared<SendingCollector<T>>(scope);
        return collect_channel_flow(
            [this, owner = this->weak_from_this().lock(), collector](Continuation<void*>* frame) {
                return flow_collect(collector.get(), frame);
            }, std::move(completion));
    }

public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:155-170
    void* collect(FlowCollector<T>* collector, Continuation<void*>* continuation) override;

protected:
    std::shared_ptr<Flow<S>> upstream() const { return flow_; }

private:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:144-148
    void* collect_with_context_undispatched(FlowCollector<T>* collector,
        std::shared_ptr<CoroutineContext> new_context, Continuation<void*>* completion);

    const std::shared_ptr<Flow<S>> flow_;
};

template <typename T>
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:179-192
class ChannelFlowOperatorImpl final : public ChannelFlowOperator<T, T> {
public:
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:179-184
    ChannelFlowOperatorImpl(std::shared_ptr<Flow<T>> flow,
                            std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
                            int capacity = Channel<T>::OPTIONAL_CHANNEL,
                            BufferOverflow on_overflow = BufferOverflow::SUSPEND)
        : ChannelFlowOperator<T, T>(std::move(flow),
                                    std::move(context),
                                    capacity,
                                    on_overflow) {}

protected:
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:185-186
    ChannelFlow<T>* create(std::shared_ptr<CoroutineContext> context, int capacity, BufferOverflow on_overflow) override {
        return new ChannelFlowOperatorImpl<T>(this->upstream(), std::move(context), capacity, on_overflow);
    }

public:
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:188-188
    Flow<T>* drop_channel_operators() override { return this->upstream().get(); }

protected:
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:190-191
    void* flow_collect(FlowCollector<T>* collector, Continuation<void*>* continuation) override { return this->upstream()->collect(collector, continuation); }
};

/**
 * Kotlin: internal fun <T> Flow<T>.asChannelFlow(): ChannelFlow<T>
 */
template <typename T>
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:11-12
inline std::shared_ptr<ChannelFlow<T>> as_channel_flow(std::shared_ptr<Flow<T>> flow) {
    if (auto channel_flow = std::dynamic_pointer_cast<ChannelFlow<T>>(flow)) {
        return channel_flow;
    }

    return std::make_shared<ChannelFlowOperatorImpl<T>>(std::move(flow));
}

} // namespace internal


namespace internal {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:215-225
void* call_with_context_undispatched(
    std::shared_ptr<CoroutineContext> new_context, void* count_or_element,
    std::function<void*(Continuation<void*>*)> block, Continuation<void*>* completion);

// Efficiently computes block(value) in the new_context.
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:215-225
template <typename R, typename V, typename Block>
inline R with_context_undispatched(
    std::shared_ptr<CoroutineContext> new_context, V value, void* count_or_element,
    Block&& block, Continuation<void*>* completion) {
    return call_with_context_undispatched(std::move(new_context), count_or_element,
        [value = std::move(value), block = std::forward<Block>(block)](Continuation<void*>* frame) mutable {
            return block(std::move(value), frame);
        }, completion);
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:215-225
template <typename R, typename V, typename Block>
inline R with_context_undispatched(
    std::shared_ptr<CoroutineContext> new_context, V value, Block&& block,
    Continuation<void*>* completion) {
    auto count_or_element = kotlinx::coroutines::internal::thread_context_elements(*new_context);
    return with_context_undispatched<R>(std::move(new_context), std::move(value), count_or_element,
                                       std::forward<Block>(block), completion);
}

// Now if the underlying collector was accepting concurrent emits, then this one is too.
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:203-212
template <typename T>
class UndispatchedContextCollector final : public FlowCollector<T> {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:203-208
    UndispatchedContextCollector(FlowCollector<T>* downstream, std::shared_ptr<CoroutineContext> emit_context)
        : emit_context_(std::move(emit_context)),
          count_or_element_(kotlinx::coroutines::internal::thread_context_elements(*emit_context_)),
          emit_ref_([downstream](T value, Continuation<void*>* completion) {
              return downstream->emit(std::move(value), completion);
          }) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:210-211
    void* emit(T value, Continuation<void*>* completion) override {
        return with_context_undispatched<void*>(emit_context_, std::move(value), count_or_element_, emit_ref_, completion);
    }

private:
    const std::shared_ptr<CoroutineContext> emit_context_;
    void* const count_or_element_; // precompute for fast with_context_undispatched
    // allocate suspend function ref once on creation
    const std::function<void*(T, Continuation<void*>*)> emit_ref_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:196-201
template <typename T>
std::shared_ptr<FlowCollector<T>> with_undispatched_context_collector(
    FlowCollector<T>* collector, std::shared_ptr<CoroutineContext> emit_context) {
    // SendingCollector & NopCollector do not care about the context at all and can be used as is.
    if (dynamic_cast<SendingCollector<T>*>(collector) || dynamic_cast<NopCollector<T>*>(collector)) {
        // NOTE(port): An identity-preserving borrowed handle does not own the original collector.
        return std::shared_ptr<FlowCollector<T>>(collector, [](FlowCollector<T>*) {});
    }
    // Otherwise just wrap into UndispatchedContextCollector interface implementation.
    return std::make_shared<UndispatchedContextCollector<T>>(collector, std::move(emit_context));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:155-170
template <typename S, typename T>
inline void* ChannelFlowOperator<S, T>::collect(FlowCollector<T>* collector, Continuation<void*>* completion) {
    // Fast-path: When channel creation is optional (flowOn/flowWith operators without buffer).
    if (this->capacity() == Channel<T>::OPTIONAL_CHANNEL) {
        auto collect_context = completion->get_context();
        auto new_context = kotlinx::coroutines::new_coroutine_context(collect_context, this->context());
        // If the resulting context happens to be the same as it was, collect directly.
        if (new_context->equals(collect_context.get())) return flow_collect(collector, completion);
        // If we don't need to change the dispatcher we can go without channels.
        auto new_interceptor = new_context->get(ContinuationInterceptor::type_key);
        auto collect_interceptor = collect_context->get(ContinuationInterceptor::type_key);
        if (new_interceptor ? new_interceptor->equals(collect_interceptor.get()) :
                              collect_interceptor == nullptr) {
            return collect_with_context_undispatched(collector, std::move(new_context), completion);
        }
    }
    // Slow-path: create the actual channel.
    return ChannelFlow<T>::collect(collector, completion);
}

// Changes collecting context upstream, while collecting in the original context.
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:144-148
template <typename S, typename T>
inline void* ChannelFlowOperator<S, T>::collect_with_context_undispatched(
    FlowCollector<T>* collector, std::shared_ptr<CoroutineContext> new_context,
    Continuation<void*>* completion) {
    auto original_context_collector = with_undispatched_context_collector(collector, completion->get_context());
    return collect_channel_flow(
        [this, owner = this->weak_from_this().lock(), original_context_collector,
         new_context = std::move(new_context)](Continuation<void*>* frame) {
            return with_context_undispatched<void*>(new_context, original_context_collector.get(),
                [this](FlowCollector<T>* sink, Continuation<void*>* continuation) {
                    return flow_collect(sink, continuation);
                }, frame);
        }, kotlinx::coroutines::internal::retain_continuation(completion));
}

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
