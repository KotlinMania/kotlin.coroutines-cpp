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
#include <optional>
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
 *     return flow<long long>([](FlowCollector<long long>* emit, Continuation<void*>* cont) -> void* {
 *         long long x = 0, y = 1;
 *         while (true) {
 *             emit->emit(x, cont);
 *             std::tie(x, y) = std::make_tuple(y, x + y);
 *         }
 *         return nullptr;
 *     });
 * }
 *
 * fibonacci() | take(100) | collect([](long long v) { std::cout << v << "\n"; });
 * ```
 *
 * Emissions from flow builder are cancellable by default &mdash; each call to emit
 * also calls ensure_active.
 *
 * emit should happen strictly in the dispatchers of the block in order to preserve the flow context.
 * For example, the following code will result in an exception:
 *
 * ```cpp
 * flow<int>([](FlowCollector<int>* emit, Continuation<void*>* cont) -> void* {
 *     emit->emit(1, cont); // Ok
 *     with_context(Dispatchers::IO, [&]() {
 *         emit->emit(2, cont); // Will fail with ISE
 *     });
 *     return nullptr;
 * });
 * ```
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
    return internal::unsafe_flow<T>([function = std::move(function)](FlowCollector<T>* collector, Continuation<void*>* completion) -> void* {
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:78-80
        class Frame final : public ContinuationImpl {
        public:
            Frame(std::function<void*(Continuation<void*>*)> function, FlowCollector<T>* collector,
                  Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  function_(std::move(function)), collector_(collector) {}

            void retain() { self_ref_ = shared_from_this(); }

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:79-79
            void* invoke_suspend(Result<void*> result) override {
                try {
                    coroutine_begin(this)
                    coroutine_yield_value(this, result, function_(this), result_box_);
                    // NOTE(port): The function returns an owning T box through the Continuation ABI.
                    value_.reset(static_cast<T*>(result_box_));
                    coroutine_yield(this, collector_->emit(std::move(*value_), this));
                    value_.reset();
                    self_ref_.reset();
                    coroutine_end(this)
                } catch (...) {
                    value_.reset();
                    self_ref_.reset();
                    throw;
                }
            }

        private:
            void* _label = nullptr;
            std::function<void*(Continuation<void*>*)> function_;
            FlowCollector<T>* collector_;
            void* result_box_ = nullptr;
            std::unique_ptr<T> value_;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
        auto frame = std::make_shared<Frame>(function, collector, completion);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

/**
 * Creates a _cold_ flow that produces values from the given iterable (vector).
 *
 * The vector elements are emitted in order each time the flow is collected.
 * Each collection starts from the beginning of the vector.
 *
 * Transliterated from:
 * public fun <T> Iterable<T>.asFlow(): Flow<T>
 */
namespace detail {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:85-90
template <typename T>
class AsFlowContinuation final : public ContinuationImpl {
public:
    AsFlowContinuation(std::vector<T> iterable, FlowCollector<T>* collector, Continuation<void*>* completion)
        : ContinuationImpl(std::shared_ptr<Continuation<void*>>(completion, [](Continuation<void*>*) {})),
          iterable_(std::move(iterable)), collector_(collector) {}

    void retain() { self_ref_ = shared_from_this(); }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:85-90
    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)
            while (index_ < iterable_.size()) {
                coroutine_yield(this, collector_->emit(iterable_[index_], this));
                ++index_;
            }
            self_ref_.reset();
            coroutine_end(this)
        } catch (...) {
            self_ref_.reset();
            throw;
        }
    }

private:
    void* _label = nullptr;
    std::vector<T> iterable_;
    FlowCollector<T>* collector_;
    size_t index_ = 0;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};

} // namespace detail

template <typename T>
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:85-89
std::shared_ptr<Flow<T>> as_flow(const std::vector<T>& iterable) {
    // Upstream:
    //   public fun <T> Iterable<T>.asFlow(): Flow<T> = flow { forEach { value -> emit(value) } }
    return internal::unsafe_flow<T>([iterable](FlowCollector<T>* collector, Continuation<void*>* cont) -> void* {
        auto sm = std::make_shared<detail::AsFlowContinuation<T>>(iterable, collector, cont);
        sm->retain();
        return sm->start(Result<void*>::success(nullptr));
    });
}


/** Creates a cold flow from the given array; reads its elements for each collection. */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:149-153,160-164,171-175
template <typename T, std::size_t N>
std::shared_ptr<Flow<T>> as_flow(const std::array<T, N>& array) {
    // NOTE(port): This C++ reference borrows the array; the caller keeps it alive during collection.
    return internal::unsafe_flow<T>([&array](FlowCollector<T>* collector, Continuation<void*>* completion) -> void* {
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:150-152,161-163,172-174
        class Frame final : public ContinuationImpl {
        public:
            Frame(const std::array<T, N>& array, FlowCollector<T>* collector, Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  array_(array), collector_(collector) {}

            void retain() { self_ref_ = shared_from_this(); }

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:150-152,161-163,172-174
            void* invoke_suspend(Result<void*> result) override {
                try {
                    coroutine_begin(this)
                    while (index_ < N) {
                        coroutine_yield(this, collector_->emit(array_[index_], this));
                        ++index_;
                    }
                    self_ref_.reset();
                    coroutine_end(this)
                } catch (...) {
                    self_ref_.reset();
                    throw;
                }
            }

        private:
            void* _label = nullptr;
            const std::array<T, N>& array_;
            FlowCollector<T>* collector_;
            std::size_t index_ = 0;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
        auto frame = std::make_shared<Frame>(array, collector, completion);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

/** Creates a cold flow that consumes values from the given iterator. */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:94-98
template <typename Iterator>
auto as_flow(Iterator first, Iterator last)
    -> std::shared_ptr<Flow<typename std::iterator_traits<Iterator>::value_type>> {
    using T = typename std::iterator_traits<Iterator>::value_type;
    // NOTE(port): C++ uses an iterator/sentinel pair; the pair retains its cursor across collections.
    // The caller owns the iterated storage and must retain it during collection.
    auto cursor = std::make_shared<Iterator>(std::move(first));
    return internal::unsafe_flow<T>([cursor, last](FlowCollector<T>* collector, Continuation<void*>* completion) -> void* {
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:94-98
        class Frame final : public ContinuationImpl {
        public:
            Frame(std::shared_ptr<Iterator> cursor, Iterator last, FlowCollector<T>* collector,
                  Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  cursor_(std::move(cursor)), last_(std::move(last)), collector_(collector) {}

            void retain() { self_ref_ = shared_from_this(); }

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:95-97
            void* invoke_suspend(Result<void*> result) override {
                try {
                    coroutine_begin(this)
                    while (*cursor_ != last_) {
                        value_.emplace(**cursor_);
                        ++*cursor_;
                        coroutine_yield(this, collector_->emit(std::move(*value_), this));
                        value_.reset();
                    }
                    self_ref_.reset();
                    coroutine_end(this)
                } catch (...) {
                    value_.reset();
                    self_ref_.reset();
                    throw;
                }
            }

        private:
            void* _label = nullptr;
            std::shared_ptr<Iterator> cursor_;
            Iterator last_;
            FlowCollector<T>* collector_;
            std::optional<T> value_;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
        auto frame = std::make_shared<Frame>(cursor, last, collector, completion);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
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
    std::vector<T> vec = elements;
    return as_flow(vec);
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

namespace detail {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:180-184,189-193
template <typename T>
class RangeFlowContinuation final : public ContinuationImpl {
public:
    RangeFlowContinuation(T first, T last, FlowCollector<T>* collector, Continuation<void*>* completion)
        : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
          current_(first), last_(last), collector_(collector) {}

    void retain() { self_ref_ = shared_from_this(); }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:181-183,190-192
    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)
            if (current_ <= last_) {
                while (true) {
                    coroutine_yield(this, collector_->emit(current_, this));
                    // Kotlin's range iterator stops at its last element before incrementing.
                    if (current_ == last_) break;
                    ++current_;
                }
            }
            self_ref_.reset();
            coroutine_end(this)
        } catch (...) {
            self_ref_.reset();
            throw;
        }
    }

private:
    void* _label = nullptr;
    T current_;
    T last_;
    FlowCollector<T>* collector_;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};

} // namespace detail

/** Creates a cold flow from an inclusive Int range. */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:180-184
inline std::shared_ptr<Flow<int>> as_flow_range(int first, int last) {
    return internal::unsafe_flow<int>([first, last](FlowCollector<int>* collector, Continuation<void*>* completion) -> void* {
        auto frame = std::make_shared<detail::RangeFlowContinuation<int>>(first, last, collector, completion);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

/** Creates a cold flow from an inclusive Long range. */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:189-193
inline std::shared_ptr<Flow<long>> as_flow_range(long first, long last) {
    return internal::unsafe_flow<long>([first, last](FlowCollector<long>* collector, Continuation<void*>* completion) -> void* {
        auto frame = std::make_shared<detail::RangeFlowContinuation<long>>(first, last, collector, completion);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

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

namespace internal {

// ChannelFlow implementation that is the first in the chain of flow operations and introduces (builds) a flow
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:305-320
template <typename T>
class ChannelFlowBuilder : public ChannelFlow<T> {
private:
    const std::function<void*(channels::ProducerScope<T>*, std::shared_ptr<Continuation<void*>>)> block_;

public:
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:305-311
    explicit ChannelFlowBuilder(
        std::function<void*(channels::ProducerScope<T>*, std::shared_ptr<Continuation<void*>>)> block,
        std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
        int capacity = channels::Channel<T>::BUFFERED,
        channels::BufferOverflow on_buffer_overflow = channels::BufferOverflow::SUSPEND
    ) : ChannelFlow<T>(context, capacity, on_buffer_overflow), block_(std::move(block)) {}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:312-313
    ChannelFlow<T>* create(
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
    ChannelFlow<T>* create(std::shared_ptr<CoroutineContext> context, int capacity,
                           channels::BufferOverflow on_buffer_overflow) override {
        return new CallbackFlowBuilder<T>(block_, std::move(context), capacity, on_buffer_overflow);
    }

private:
    const std::function<void*(channels::ProducerScope<T>*, std::shared_ptr<Continuation<void*>>)> block_;
};

} // namespace internal

/**
 * Upstream:
 *   public fun <T> channelFlow(@BuilderInference block: suspend ProducerScope<T>.() -> Unit): Flow<T> =
 *       ChannelFlowBuilder(block)
 *
 * `ChannelFlowBuilder` lives in flow/Builders.kt; the C++ port routes through
 * `internal::ChannelFlowBuilder<T>` which keeps the producer block and replays it once per
 * collector.
 */
template <typename T>
// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:242-243
inline std::shared_ptr<Flow<T>> channel_flow(
    std::function<void*(channels::ProducerScope<T>*, std::shared_ptr<Continuation<void*>>)>
        block) {
    return std::make_shared<internal::ChannelFlowBuilder<T>>(std::move(block));
}


// Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt:302-302
template <typename T>
inline std::shared_ptr<Flow<T>> callback_flow(
    std::function<void*(channels::ProducerScope<T>*, std::shared_ptr<Continuation<void*>>)> block) {
    return std::make_shared<internal::CallbackFlowBuilder<T>>(std::move(block));
}


} // namespace flow
} // namespace coroutines
} // namespace kotlinx
