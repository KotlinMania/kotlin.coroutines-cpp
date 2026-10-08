#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/flow/operators/Merge.kt
/**
 * @file Merge.hpp
 * @brief Flow merge operators: merge, flatten_merge, transform_latest, map_latest
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt
 */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/Transform.hpp"
#include "kotlinx/coroutines/flow/internal/Merge.hpp"
#include "kotlinx/coroutines/internal/SystemProps.hpp"
#include <stdexcept>
#include <string>
#include <limits>

namespace kotlinx {
namespace coroutines {
namespace flow {


using kotlinx::coroutines::channels::Channel;
using kotlinx::coroutines::flow::internal::ChannelFlowMerge;
using kotlinx::coroutines::flow::internal::ChannelLimitedFlowMerge;
using kotlinx::coroutines::flow::internal::ChannelFlowTransformLatest;

// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:110-113
template <typename T>
void* collect(std::shared_ptr<Flow<T>> upstream,
    std::function<void*(T, Continuation<void*>*)> action, Continuation<void*>* completion);
// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:103-106
template <typename T>
void* emit_all(FlowCollector<T>* collector, std::shared_ptr<Flow<T>> upstream,
    Continuation<void*>* completion);

/**
 * Name of the property that defines the value of DEFAULT_CONCURRENCY.
 * This preview API may change incompatibly within one release.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:19
inline constexpr const char* DEFAULT_CONCURRENCY_PROPERTY_NAME = "kotlinx.coroutines.flow.defaultConcurrency";

/**
 * Default concurrency limit used by flatten_merge and flat_map_merge.
 * The default is 16. On JVM, the DEFAULT_CONCURRENCY_PROPERTY_NAME property
 * changes this limit. This preview API may change incompatibly within one release.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:26-30
inline const int DEFAULT_CONCURRENCY = kotlinx::coroutines::internal::system_prop_int(
    DEFAULT_CONCURRENCY_PROPERTY_NAME,
    16,
    1,
    std::numeric_limits<int>::max()
);

/**
 * Flattens the given flow of flows into a single flow in a sequential manner, without interleaving nested flows.
 *
 * Inner flows are collected by this operator *sequentially*.
 *
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:78-80
template <typename T>
std::shared_ptr<Flow<T>> flatten_concat(std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> upstream) {
    return internal::unsafe_flow<T>([upstream = std::move(upstream)](
        FlowCollector<T>* collector, Continuation<void*>* completion) -> void* {
        return collect<std::shared_ptr<Flow<T>>>(upstream,
            std::function<void*(std::shared_ptr<Flow<T>>, Continuation<void*>*)>(
                [collector](std::shared_ptr<Flow<T>> value, Continuation<void*>* continuation) -> void* {
                    return emit_all<T>(collector, std::move(value), continuation);
                }), completion);
    });
}

/**
 * Merges the given flows into a single flow without preserving an order of elements.
 * All flows are merged concurrently, without limit on the number of simultaneously collected flows.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:91-103
template <typename T>
std::shared_ptr<Flow<T>> merge(std::vector<std::shared_ptr<Flow<T>>> flows) {
    return std::make_shared<ChannelLimitedFlowMerge<T>>(flows);
}

/**
 * Merges the given flows into a single flow without preserving an order of elements.
 * All flows are collected concurrently without a concurrency limit.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:114
template <typename T>
std::shared_ptr<Flow<T>> merge(std::initializer_list<std::shared_ptr<Flow<T>>> flows) {
    return merge<T>(std::vector<std::shared_ptr<Flow<T>>>(flows));
}

/**
 * Flattens the given flow of flows into a single flow with a [concurrency] limit.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:135-138
template <typename T>
std::shared_ptr<Flow<T>> flatten_merge(std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> upstream, int concurrency = DEFAULT_CONCURRENCY) {
    if (concurrency <= 0) {
        throw std::invalid_argument("Expected positive concurrency level, but had " + std::to_string(concurrency));
    }

    if (concurrency == 1) {
        return flatten_concat<T>(upstream);
    }

    return std::make_shared<ChannelFlowMerge<T>>(upstream, concurrency);
}

/**
 * Transforms elements emitted by the original flow by applying transform, that returns another flow,
 * and then concatenating and flattening these flows.
 * This is a shortcut for map(transform).flatten_concat(). In regular application
 * flows, a suspending map operation is often sufficient and easier to reason about.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:42-43
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_concat(
    std::shared_ptr<Flow<T>> upstream,
    std::function<std::shared_ptr<Flow<R>>(T)> transform) {
    return flatten_concat<R>(map<T, std::shared_ptr<Flow<R>>>(std::move(upstream), std::move(transform)));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:42-43
// NOTE(port): The transform returns an owned std::shared_ptr<Flow<R>> result box;
// map unboxes and deletes it through its existing Continuation ABI projection.
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_concat(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(T, Continuation<void*>*)> transform) {
    return flatten_concat<R>(map<T, std::shared_ptr<Flow<R>>>(std::move(upstream), std::move(transform)));
}

/**
 * Transforms elements by applying transform sequentially, then merges the returned
 * flows with a concurrency limit. It is a shortcut for map(transform).flatten_merge(concurrency).
 *
 * Applications of flow_on, buffer and produce_in after this operator fuse with
 * its concurrent merging so that one configured channel executes the merge.
 * At most concurrency flows are collected at the same time.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:66-70
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_merge(
    std::shared_ptr<Flow<T>> upstream, int concurrency,
    std::function<std::shared_ptr<Flow<R>>(T)> transform) {
    return flatten_merge<R>(map<T, std::shared_ptr<Flow<R>>>(std::move(upstream), std::move(transform)), concurrency);
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:66-70
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_merge(
    std::shared_ptr<Flow<T>> upstream, int concurrency,
    std::function<void*(T, Continuation<void*>*)> transform) {
    return flatten_merge<R>(map<T, std::shared_ptr<Flow<R>>>(std::move(upstream), std::move(transform)), concurrency);
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:66-70
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_merge(
    std::shared_ptr<Flow<T>> upstream,
    std::function<std::shared_ptr<Flow<R>>(T)> transform) {
    return flat_map_merge<T, R>(std::move(upstream), DEFAULT_CONCURRENCY, std::move(transform));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:66-70
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_merge(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(T, Continuation<void*>*)> transform) {
    return flat_map_merge<T, R>(std::move(upstream), DEFAULT_CONCURRENCY, std::move(transform));
}

/**
 * Returns a flow that produces element by [transform] function every time the original flow emits a value.
 * When the original flow emits a new value, the previous `transform` block is cancelled.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:162-163
template <typename T, typename R>
std::shared_ptr<Flow<R>> transform_latest(std::shared_ptr<Flow<T>> upstream,
    std::function<void*(FlowCollector<R>*, T, Continuation<void*>*)> transform) {
    return std::make_shared<ChannelFlowTransformLatest<T, R>>(std::move(transform), std::move(upstream));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:162-163
// NOTE(port): The owning continuation projection preserves the supplied caller
// for compiler-authored transform lambdas; downstream collectors stay borrowed.
template <typename T, typename R>
std::shared_ptr<Flow<R>> transform_latest(std::shared_ptr<Flow<T>> upstream,
    std::function<void*(FlowCollector<R>*, T, std::shared_ptr<Continuation<void*>>)> transform) {
    return transform_latest<T, R>(std::move(upstream),
        std::function<void*(FlowCollector<R>*, T, Continuation<void*>*)>(
            [transform = std::move(transform)](FlowCollector<R>* collector, T value,
                Continuation<void*>* completion) -> void* {
                return transform(collector, std::move(value),
                    kotlinx::coroutines::internal::retain_continuation(completion));
            }));
}

/**
 * Returns a flow that emits transformed elements from the original flow.
 * A new upstream value cancels the previous transform computation.
 * The operator is buffered by default; buffer changes the output buffer size.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:212-213
// NOTE(port): A suspending transform returns an owned R result box. The receiving
// lambda unboxes and deletes it before downstream emission.
template <typename T, typename R>
std::shared_ptr<Flow<R>> map_latest(std::shared_ptr<Flow<T>> upstream,
    std::function<void*(T, Continuation<void*>*)> transform) {
    return transform_latest<T, R>(std::move(upstream),
        std::function<void*(FlowCollector<R>*, T, std::shared_ptr<Continuation<void*>>)>(
            [transform = std::move(transform)](FlowCollector<R>* collector, T value,
                std::shared_ptr<Continuation<void*>> completion)
                __attribute__((annotate("suspend"))) -> void* {
                void* raw = dsl::suspend(transform(std::move(value), completion.get()));
                std::unique_ptr<R> box(static_cast<R*>(raw));
                R transformed = std::move(*box);
                box.reset();
                dsl::suspend(collector->emit(std::move(transformed), completion.get()));
                return nullptr;
            }));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:212-213
// NOTE(port): Ordinary C++ transforms remain callable through the same source
// operation; the adapter boxes their result for the Continuation ABI receiver.
template <typename T, typename R>
std::shared_ptr<Flow<R>> map_latest(std::shared_ptr<Flow<T>> upstream,
    std::function<R(T)> transform) {
    return map_latest<T, R>(std::move(upstream),
        std::function<void*(T, Continuation<void*>*)>(
            [transform = std::move(transform)](T value, Continuation<void*>*) -> void* {
                return new R(transform(std::move(value)));
            }));
}

/**
 * Switches to the flow returned by transform for each upstream value.
 * A new upstream value cancels the previous transformed flow.
 * The operator is buffered by default; buffer changes the output buffer size.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:188-189
// NOTE(port): The transform returns an owned shared_ptr<Flow<R>> result box.
// emit_all retains the extracted flow while collecting; the box is deleted here.
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_latest(std::shared_ptr<Flow<T>> upstream,
    std::function<void*(T, Continuation<void*>*)> transform) {
    return transform_latest<T, R>(std::move(upstream),
        std::function<void*(FlowCollector<R>*, T, std::shared_ptr<Continuation<void*>>)>(
            [transform = std::move(transform)](FlowCollector<R>* collector, T value,
                std::shared_ptr<Continuation<void*>> completion)
                __attribute__((annotate("suspend"))) -> void* {
                void* raw = dsl::suspend(transform(std::move(value), completion.get()));
                std::unique_ptr<std::shared_ptr<Flow<R>>> box(static_cast<std::shared_ptr<Flow<R>>*>(raw));
                auto inner = std::move(*box);
                box.reset();
                dsl::suspend(emit_all<R>(collector, std::move(inner), completion.get()));
                return nullptr;
            }));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Merge.kt:188-189
template <typename T, typename R>
std::shared_ptr<Flow<R>> flat_map_latest(std::shared_ptr<Flow<T>> upstream,
    std::function<std::shared_ptr<Flow<R>>(T)> transform) {
    return flat_map_latest<T, R>(std::move(upstream),
        std::function<void*(T, Continuation<void*>*)>(
            [transform = std::move(transform)](T value, Continuation<void*>*) -> void* {
                return new std::shared_ptr<Flow<R>>(transform(std::move(value)));
            }));
}


} // namespace flow
} // namespace coroutines
} // namespace kotlinx

// NOTE(port): Definitions for the source collect/emit_all extensions follow the
// merge declarations because Collect.hpp also consumes transform_latest.
#include "kotlinx/coroutines/flow/Collect.hpp"
