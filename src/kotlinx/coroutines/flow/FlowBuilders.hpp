#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Builders.kt
 */

#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/internal/FlowImpl.hpp"
#include "kotlinx/coroutines/flow/internal/SafeCollector.hpp"
#include "kotlinx/coroutines/flow/internal/ChannelFlow.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/ProducerScope.hpp"
#include <functional>
#include <memory>
#include <vector>

namespace kotlinx {
namespace coroutines {
namespace flow {

// Temporarily disable Flow builder stubs with complex types until full C++ port is ready.
/**
 * Creates a flow from the given suspendable block.
 */
template <typename T>
std::shared_ptr<Flow<T>> flow(std::function<void*(FlowCollector<T>*, Continuation<void*>*)> block) {
    class CallbackFlow : public AbstractFlow<T> {
        std::function<void*(FlowCollector<T>*, Continuation<void*>*)> block_;
    public:
        CallbackFlow(std::function<void*(FlowCollector<T>*, Continuation<void*>*)> b) : block_(b) {}
        void* collect_safely(FlowCollector<T>* collector, Continuation<void*>* continuation) override {
             return block_(collector, continuation);
        }
    };
    return std::make_shared<CallbackFlow>(block);
}

/**
 * Creates a flow that produces a single value from the given functional type.
 */
template <typename T>
std::shared_ptr<Flow<T>> as_flow(std::function<T()> func) {
    return flow<T>([func](FlowCollector<T>* collector, Continuation<void*>* cont) -> void* {
        return collector->emit(func(), cont);
    });
}

/**
 * Creates a flow that produces values from the given iterable.
 */
template <typename T>
std::shared_ptr<Flow<T>> as_flow(const std::vector<T>& iterable) {
    return flow<T>([iterable](FlowCollector<T>* collector, Continuation<void*>* cont) -> void* {
        // TODO: Implement state machine for suspending loop
        for (const auto& value : iterable) {
            collector->emit(value, cont);
        }
        return nullptr;
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
std::shared_ptr<Flow<T>> flow_of(T value) {
    return flow<T>([value](FlowCollector<T>* collector, Continuation<void*>* cont) -> void* {
        return collector->emit(value, cont);
    });
}

/**
 * Returns an empty flow.
 *
 * Transliterated from: public fun <T> emptyFlow(): Flow<T> = EmptyFlow
 */
template <typename T>
std::shared_ptr<Flow<T>> empty_flow() {
    class EmptyFlow : public Flow<T> {
    public:
        void* collect(FlowCollector<T>*, Continuation<void*>*) override {
            return nullptr;  // Unit - nothing to emit
        }
    };
    static auto instance = std::make_shared<EmptyFlow>();
    return instance;
}

/**
 * Creates a flow that produces values from the given range [start, end).
 *
 * Transliterated from: public fun IntRange.asFlow(): Flow<Int>
 */
inline std::shared_ptr<Flow<int>> as_flow_range(int start, int end) {
    return flow<int>([start, end](FlowCollector<int>* collector, Continuation<void*>* cont) -> void* {
        for (int i = start; i < end; ++i) {
            collector->emit(i, cont);
        }
        return nullptr;
    });
}

/**
 * Creates a flow that produces values from the given range [start, end).
 */
inline std::shared_ptr<Flow<long>> as_flow_range(long start, long end) {
    return flow<long>([start, end](FlowCollector<long>* collector, Continuation<void*>* cont) -> void* {
        for (long i = start; i < end; ++i) {
            collector->emit(i, cont);
        }
        return nullptr;
    });
}

/**
 * flowScope builder
 */
// Simple concrete CoroutineScope for flow builders
class FlowCoroutineScope : public CoroutineScope {
    std::shared_ptr<CoroutineContext> context_;
public:
    explicit FlowCoroutineScope(std::shared_ptr<CoroutineContext> ctx) : context_(ctx) {}
    std::shared_ptr<CoroutineContext> get_coroutine_context() const override { return context_; }
};

template <typename R>
R flow_scope(std::function<R(CoroutineScope&)> block) {
    // Implementation stub: just execute block
    // flowScope in Kotlin creates a scope that cancels when flow collector fails.
    // For now, direct execution.
    // TODO: Proper scope isolation with job cancellation
    auto job = std::make_shared<JobSupport>(true);
    auto ctx = std::make_shared<CombinedContext>(std::make_shared<CoroutineContext>(), job);
    FlowCoroutineScope scope(ctx);
    return block(scope);
}

/**
 * scopedFlow builder
 */
template <typename R>
std::shared_ptr<Flow<R>> scoped_flow(std::function<void(CoroutineScope&, FlowCollector<R>*)> block) {
     return flow<R>([block](FlowCollector<R>* collector, Continuation<void*>* cont) -> void* {
         flow_scope<void>([&](CoroutineScope& scope){
             block(scope, collector);
         });
         return nullptr;
     });
}

// channelFlow stub
template<typename T>
std::shared_ptr<Flow<T>> channel_flow(std::function<void(void*)> block) { // void* as placeholder for ProducerScope
    // TODO: Implement ChannelFlow logic with produce
    return nullptr;
}


} // namespace flow
} // namespace coroutines
} // namespace kotlinx
