/**
 * @file FlowBuilders.hpp
 * @brief Flow creation functions: flow, flow_of, as_flow, empty_flow, channel_flow
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt
 *
 * This file provides functions for creating Flow instances:
 * - flow(): The primary builder for creating cold flows
 * - flow_of(): Creates flows from values or initializer lists
 * - as_flow(): Converts iterables/vectors to flows
 * - empty_flow(): Returns an empty flow
 * - channel_flow(): Creates a cold flow backed by a channel
 */
#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/flow/Builders.kt

#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/internal/FlowCoroutine.hpp"
#include "kotlinx/coroutines/flow/internal/SafeCollector.hpp"
#include "kotlinx/coroutines/flow/internal/ChannelFlow.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/ProducerScope.hpp"
#include <functional>
#include <array>
#include <iterator>
#include <memory>
#include <vector>

namespace kotlinx {
namespace coroutines {
namespace flow {

/**
 * Creates a _cold_ flow from the given suspendable block.
 * The flow being _cold_ means that the block is called every time a terminal operator is applied to the resulting flow.
 *
 * Example of usage:
 *
 * ```cpp
 * auto fibonacci() {
 *     return flow<long long>([](FlowCollector<long long>* emit,
 *                               std::shared_ptr<Continuation<void*>> completion)
 *         __attribute__((annotate("suspend"))) -> void* {
 *         long long x = 0, y = 1;
 *         while (true) {
 *             dsl::suspend(emit->emit(x, completion.get()));
 *             auto next = x + y;
 *             x = y;
 *             y = next;
 *         }
 *         return nullptr;
 *     });
 * }
 *
 * auto first_twenty = take(fibonacci(), 20);
 * // NOTE(port): Bound the example to the C++ fixed-width integer representation.
 * ```
 *
 * Emissions from flow builder are cancellable by default &mdash; each call to emit
 * also calls ensure_active.
 *
 * emit should happen strictly in the dispatchers of the block in order to preserve the flow context.
 * Emitting after changing context inside the block violates this constraint
 * and throws IllegalStateException, as in the upstream withContext example.
 *
 * If you want to switch the context of execution of a flow, use the flow_on operator.
 *
 * Transliterated from:
 * public fun <T> flow(block: suspend FlowCollector<T>.() -> Unit): Flow<T> = SafeFlow(block)
 */
template <typename T>
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:52-59
std::shared_ptr<Flow<T>> flow(std::function<void*(FlowCollector<T>*, Continuation<void*>*)> block) {
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:55-59
    // Named anonymous object
    class SafeFlow final : public AbstractFlow<T> {
        const std::function<void*(FlowCollector<T>*, Continuation<void*>*)> block_;
    public:
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:55-55
        SafeFlow(std::function<void*(FlowCollector<T>*, Continuation<void*>*)> b) : block_(std::move(b)) {}
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:56-58
        void* collect_safely(FlowCollector<T>* collector, Continuation<void*>* continuation) override {
             return block_(collector, continuation);
        }
    };
    return std::make_shared<SafeFlow>(std::move(block));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:52-59
// NOTE(port): Owning Continuation authoring projection of the same source block;
// AbstractFlow collection retains the actual SafeFlow and its callable owner.
template <typename T>
std::shared_ptr<Flow<T>> flow(
    std::function<void*(FlowCollector<T>*, std::shared_ptr<Continuation<void*>>)> block) {
    return flow<T>([block = std::move(block)](FlowCollector<T>* collector, Continuation<void*>* completion) -> void* {
        return block(collector, kotlinx::coroutines::internal::retain_continuation(completion));
    });
}

/**
 * Non-suspending overload of [flow].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:52-59
 */
template <typename T>
inline std::shared_ptr<Flow<T>> flow(std::function<void(FlowCollector<T>*)> block) {
    return flow<T>([block = std::move(block)](FlowCollector<T>* collector, Continuation<void*>*) -> void* {
        block(collector);
        return nullptr;
    });
}

/**
 * Creates a _cold_ flow that produces a single value from the given functional type.
 */
template <typename T>
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:64-66
std::shared_ptr<Flow<T>> as_flow(std::function<T()> func) {
    return internal::unsafe_flow<T>([func](FlowCollector<T>* collector, Continuation<void*>* cont) -> void* {
        return collector->emit(func(), cont);
    });
}

namespace detail {

// NOTE(port): These generic collection bodies must remain in the header for
// arbitrary public element/iterator types. Captured values are actual owned
// arguments; collectors and array/storage references remain borrowed.
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:78-80
template <typename T>
[[clang::annotate("suspend")]]
void* collect_function(std::function<void*(Continuation<void*>*)> function,
                       FlowCollector<T>* collector, std::shared_ptr<Continuation<void*>> completion) {
    void* result = dsl::suspend(function(completion.get()));
    // NOTE(port): The ABI returns an owning T box. Transfer its value and free
    // the box before the next suspension; the compiler retains the actual value.
    std::unique_ptr<T> box(static_cast<T*>(result));
    T value = std::move(*box);
    box.reset();
    dsl::suspend(collector->emit(std::move(value), completion.get()));
    return nullptr;
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:85-90,118-122
template <typename T>
[[clang::annotate("suspend")]]
void* collect_iterable(std::vector<T> iterable, FlowCollector<T>* collector,
                       std::shared_ptr<Continuation<void*>> completion) {
    for (const auto& value : iterable) {
        dsl::suspend(collector->emit(value, completion.get()));
    }
    return nullptr;
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:149-153,160-164,171-175
template <typename T, std::size_t N>
[[clang::annotate("suspend")]]
void* collect_array(const std::array<T, N>& array, FlowCollector<T>* collector,
                    std::shared_ptr<Continuation<void*>> completion) {
    for (const auto& value : array) {
        dsl::suspend(collector->emit(value, completion.get()));
    }
    return nullptr;
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:94-98
template <typename Iterator>
[[clang::annotate("suspend")]]
void* collect_iterator(std::shared_ptr<Iterator> cursor, Iterator last,
                       FlowCollector<typename std::iterator_traits<Iterator>::value_type>* collector,
                       std::shared_ptr<Continuation<void*>> completion) {
    while (*cursor != last) {
        typename std::iterator_traits<Iterator>::value_type value = **cursor;
        // NOTE(port): Kotlin next() consumes the element before invoking emit.
        ++*cursor;
        dsl::suspend(collector->emit(std::move(value), completion.get()));
    }
    return nullptr;
}

} // namespace detail

/**
 * Creates a _cold_ flow that produces a single value from the given functional type.
 *
 * Example of usage:
 *
 * ```cpp
 * void* remote_call(Continuation<void*>* completion);
 * auto remote_call_flow() { return as_flow<R>(remote_call); }
 * ```
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:78-80
template <typename T>
std::shared_ptr<Flow<T>> as_flow(std::function<void*(Continuation<void*>*)> function) {
    return internal::unsafe_flow<T>([function = std::move(function)](FlowCollector<T>* collector,
                                                                   Continuation<void*>* completion) -> void* {
        return detail::collect_function<T>(function, collector,
            kotlinx::coroutines::internal::retain_continuation(completion));
    });
}

/** Creates a cold flow from an iterable; each collection starts at its beginning. */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:85-90
template <typename T>
std::shared_ptr<Flow<T>> as_flow(const std::vector<T>& iterable) {
    // NOTE(port): Preserve the existing C++ vector snapshot per collection.
    return internal::unsafe_flow<T>([iterable](FlowCollector<T>* collector, Continuation<void*>* completion) -> void* {
        return detail::collect_iterable<T>(iterable, collector,
            kotlinx::coroutines::internal::retain_continuation(completion));
    });
}

/** Creates a cold flow from the given array; reads its elements for each collection. */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:149-153,160-164,171-175
template <typename T, std::size_t N>
std::shared_ptr<Flow<T>> as_flow(const std::array<T, N>& array) {
    // NOTE(port): This reference borrows the array; the caller keeps it alive during collection.
    return internal::unsafe_flow<T>([&array](FlowCollector<T>* collector, Continuation<void*>* completion) -> void* {
        return detail::collect_array<T, N>(array, collector,
            kotlinx::coroutines::internal::retain_continuation(completion));
    });
}

/** Creates a cold flow that consumes values from the given iterator. */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:94-98
template <typename Iterator>
auto as_flow(Iterator first, Iterator last)
    -> std::shared_ptr<Flow<typename std::iterator_traits<Iterator>::value_type>> {
    using T = typename std::iterator_traits<Iterator>::value_type;
    // NOTE(port): C++ uses an iterator/sentinel pair, retaining the consumed
    // cursor across collections. The caller owns the iterated storage.
    auto cursor = std::make_shared<Iterator>(std::move(first));
    return internal::unsafe_flow<T>([cursor, last](FlowCollector<T>* collector, Continuation<void*>* completion) -> void* {
        return detail::collect_iterator<Iterator>(cursor, last, collector,
            kotlinx::coroutines::internal::retain_continuation(completion));
    });
}


/**
 * Creates a flow from elements.
 *
 * Example of usage:
 * ```cpp
 * flow_of({1, 2, 3})
 * ```
 */
template <typename T>
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:118-122
std::shared_ptr<Flow<T>> flow_of(std::initializer_list<T> elements) {
    // NOTE(port): An initializer_list borrows storage, so keep the existing
    // owning vector representation for the captured vararg values.
    std::vector<T> values(elements);
    return internal::unsafe_flow<T>([values = std::move(values)](FlowCollector<T>* collector,
                                                               Continuation<void*>* completion) -> void* {
        return detail::collect_iterable<T>(values, collector,
            kotlinx::coroutines::internal::retain_continuation(completion));
    });
}

/**
 * Creates a flow that produces the given value.
 *
 * Optimized overload for single-value flows which significantly reduces
 * the footprint compared to initializer_list version.
 */
template <typename T>
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:127-133
std::shared_ptr<Flow<T>> flow_of(T value) {
    return internal::unsafe_flow<T>([value](FlowCollector<T>* collector, Continuation<void*>* cont) -> void* {
        /*
         * Implementation note: this is just an "optimized" overload of flow_of(vararg)
         * which significantly reduces the footprint of widespread single-value flows.
         */
        return collector->emit(value, cont);
    });
}

/**
 * Returns an empty flow.
 *
 * Transliterated from: public fun <T> emptyFlow(): Flow<T> = EmptyFlow
 */
template <typename T>
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:138-142
std::shared_ptr<Flow<T>> empty_flow() {
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:140-142
    class EmptyFlow final : public Flow<T> {
    public:
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:141-141
        void* collect(FlowCollector<T>*, Continuation<void*>*) override {
            return nullptr;  // Unit - nothing to emit
        }
    };
    static auto instance = std::make_shared<EmptyFlow>();
    return instance;
}

/** Creates a cold flow from an inclusive Int range. */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:180-184
std::shared_ptr<Flow<int>> as_flow_range(int first, int last);

/** Creates a cold flow from an inclusive Long range. */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:189-193
std::shared_ptr<Flow<long>> as_flow_range(long first, long last);

// NOTE(port): Ordinary synchronous C++ block adapter; the actual flow scope
// supplies cancellation and waits for every child before collection completes.
template <typename R>
std::shared_ptr<Flow<R>> scoped_flow(std::function<void(CoroutineScope&, FlowCollector<R>*)> block) {
    return internal::scoped_flow<R>(
        [block = std::move(block)](CoroutineScope* scope, FlowCollector<R>* collector,
                                   Continuation<void*>*) -> void* {
            block(*scope, collector);
            return nullptr;
        });
}

// ChannelFlow implementation that is the first in the chain of flow operations and introduces (builds) a flow
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:305-320
// NOTE(port): The source-private generic type requires a header definition for
// arbitrary C++ element types; its namespace follows the Kotlin declaration.
template <typename T>
class ChannelFlowBuilder : public internal::ChannelFlow<T> {
private:
    const std::function<void*(channels::ProducerScope<T>*, std::shared_ptr<Continuation<void*>>)> block_;

public:
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:305-311
    explicit ChannelFlowBuilder(
        std::function<void*(channels::ProducerScope<T>*, std::shared_ptr<Continuation<void*>>)> block,
        std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
        int capacity = channels::Channel<T>::BUFFERED,
        channels::BufferOverflow on_buffer_overflow = channels::BufferOverflow::SUSPEND
    ) : internal::ChannelFlow<T>(context, capacity, on_buffer_overflow), block_(std::move(block)) {}

protected:
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:312-313
    internal::ChannelFlow<T>* create(
        std::shared_ptr<CoroutineContext> context,
        int capacity,
        channels::BufferOverflow on_buffer_overflow) override {
        return new ChannelFlowBuilder<T>(block_, context, capacity, on_buffer_overflow);
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:315-316
    void* collect_to(channels::ProducerScope<T>* scope,
                     std::shared_ptr<Continuation<void*>> completion) override {
        return block_(scope, std::move(completion));
    }

};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:322-349
template <typename T>
class CallbackFlowBuilder final : public ChannelFlowBuilder<T> {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:322-327
    explicit CallbackFlowBuilder(
        std::function<void*(channels::ProducerScope<T>*, std::shared_ptr<Continuation<void*>>)> block,
        std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
        int capacity = channels::Channel<T>::BUFFERED,
        channels::BufferOverflow on_buffer_overflow = channels::BufferOverflow::SUSPEND)
        : ChannelFlowBuilder<T>(block, std::move(context), capacity, on_buffer_overflow), block_(std::move(block)) {}

protected:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:329-345
    [[clang::annotate("suspend")]]
    void* collect_to(channels::ProducerScope<T>* scope,
                     std::shared_ptr<Continuation<void*>> completion) override {
        dsl::suspend(ChannelFlowBuilder<T>::collect_to(scope, completion));
        /*
         * We expect user either call `await_close` from within a block (then the channel is closed at this moment)
         * or being closed/cancelled externally/manually. Otherwise "user forgot to call
         * await_close and receives unhelpful ClosedSendChannelException exceptions" situation is detected.
         */
        if (!scope->is_closed_for_send()) {
            throw IllegalStateException(
                "'awaitClose { yourCallbackOrListener.cancel() }' should be used in the end of callbackFlow block.\n"
                "Otherwise, a callback/listener may leak in case of external cancellation.\n"
                "See callbackFlow API documentation for the details.");
        }
        return nullptr;
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:347-348
    internal::ChannelFlow<T>* create(std::shared_ptr<CoroutineContext> context, int capacity,
                           channels::BufferOverflow on_buffer_overflow) override {
        return new CallbackFlowBuilder<T>(block_, std::move(context), capacity, on_buffer_overflow);
    }

private:
    const std::function<void*(channels::ProducerScope<T>*, std::shared_ptr<Continuation<void*>>)> block_;
};

/**
 * Upstream:
 *   public fun <T> channelFlow(@BuilderInference block: suspend ProducerScope<T>.() -> Unit): Flow<T> =
 *       ChannelFlowBuilder(block)
 *
 * `ChannelFlowBuilder` lives in flow/Builders.kt; the C++ port routes through
 * `ChannelFlowBuilder<T>` which keeps the producer block and replays it once per
 * collector.
 */
template <typename T>
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:242-243
inline std::shared_ptr<Flow<T>> channel_flow(
    std::function<void*(channels::ProducerScope<T>*, std::shared_ptr<Continuation<void*>>)>
        block) {
    return std::make_shared<ChannelFlowBuilder<T>>(std::move(block));
}


// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:302-302
template <typename T>
inline std::shared_ptr<Flow<T>> callback_flow(
    std::function<void*(channels::ProducerScope<T>*, std::shared_ptr<Continuation<void*>>)> block) {
    return std::make_shared<CallbackFlowBuilder<T>>(std::move(block));
}


} // namespace flow
} // namespace coroutines
} // namespace kotlinx
