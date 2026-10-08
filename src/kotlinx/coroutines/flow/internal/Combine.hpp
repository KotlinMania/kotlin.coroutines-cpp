#pragma once
// port-lint: source flow/internal/Combine.kt
/**
 * @file Combine.hpp
 * @brief Internal primitives for combine and zip operators.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/flow/internal/SendingCollector.hpp"
#include "kotlinx/coroutines/flow/internal/SafeCollector.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include <any>
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace kotlinx::coroutines::flow::internal {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:11-80
// NOTE(port): Concrete algorithms and lowered frames live in Combine.cpp.
void* combine_internal_erased(
    std::vector<std::shared_ptr<Flow<std::any>>> flows,
    std::function<std::vector<std::any>*()> array_factory,
    std::function<void*(const std::vector<std::any>&, Continuation<void*>*)> transform,
    Continuation<void*>* completion);

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:13-13,29-32
// NOTE(port): Retains the typed collector binding through the actual collect call.
void* collect_combine_adapter(std::function<void*(Continuation<void*>*)> collect,
                             Continuation<void*>* completion);

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:273-273
template <typename T>
inline auto null_array_factory() {
    return []() -> std::vector<T>* { return nullptr; };
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:13-13
// NOTE(port): Type-erased flow adapter for Array<out Flow<T>>.
template <typename T>
inline std::shared_ptr<Flow<std::any>> as_any_flow(std::shared_ptr<Flow<T>> flow) {
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:13-13
    // NOTE(port): Typed erasure binding; the collect frame retains its real upstream and collector.
    class AnyFlow : public Flow<std::any> {
        std::shared_ptr<Flow<T>> upstream_;
    public:
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:13-13
        explicit AnyFlow(std::shared_ptr<Flow<T>> u) : upstream_(std::move(u)) {}
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:13-13
        void* collect(FlowCollector<std::any>* collector, Continuation<void*>* cont) override {
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:13-13
            class AnyCollector : public FlowCollector<T> {
                FlowCollector<std::any>* down_;
            public:
                // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:13-13
                explicit AnyCollector(FlowCollector<std::any>* d) : down_(d) {}
                // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:13-13
                void* emit(T value, Continuation<void*>* c) override {
                    return down_->emit(std::any(std::move(value)), c);
                }
            };
            auto adapter = std::make_shared<AnyCollector>(collector);
            return collect_combine_adapter(
                [upstream = upstream_, adapter](Continuation<void*>* frame) {
                    return upstream->collect(adapter.get(), frame);
                }, cont);
        }
    };
    return std::make_shared<AnyFlow>(std::move(flow));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:11-80
// NOTE(port): The array factory transfers its returned vector to the lowered frame.
template <typename R>
inline void* combine_internal(
    FlowCollector<R>* collector,
    const std::vector<std::shared_ptr<Flow<std::any>>>& flows,
    std::function<std::vector<std::any>*()> array_factory,
    std::function<void*(FlowCollector<R>*, const std::vector<std::any>&, Continuation<void*>*)> transform,
    Continuation<void*>* completion) {
    return combine_internal_erased(flows, std::move(array_factory),
        [collector, transform = std::move(transform)](
            const std::vector<std::any>& values, Continuation<void*>* frame) {
            return transform(collector, values, frame);
        }, completion);
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:11-80
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:254-270
// NOTE(port): Compatibility binding for existing FunctionN callers using the null factory.
template <typename R>
inline void* combine_internal(
    FlowCollector<R>* collector,
    const std::vector<std::shared_ptr<Flow<std::any>>>& flows,
    std::function<void*(FlowCollector<R>*, const std::vector<std::any>&, Continuation<void*>*)> transform,
    Continuation<void*>* completion) {
    return combine_internal<R>(collector, flows, null_array_factory<std::any>(),
                               std::move(transform), completion);
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:265-270
template <typename R>
inline std::shared_ptr<Flow<R>> combine_transform_unsafe(
    std::vector<std::shared_ptr<Flow<std::any>>> flows,
    std::function<void*(FlowCollector<R>*, const std::vector<std::any>&, Continuation<void*>*)> transform) {
    return flow<R>([flows = std::move(flows), transform = std::move(transform)](
        FlowCollector<R>* collector, Continuation<void*>* cont) -> void* {
        return combine_internal<R>(
            collector, flows,
            [transform](FlowCollector<R>* sink, const std::vector<std::any>& vals, Continuation<void*>* c) -> void* {
                return transform(sink, vals, c);
            },
            cont);
    });
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:28-30,254-260
// NOTE(port): Concrete lowering of emit(transform(values)); emit takes ownership of the result box.
void* emit_combine_result(
    std::function<void*(Continuation<void*>*)> transform,
    std::function<void*(void*, Continuation<void*>*)> emit,
    std::function<void(void*)> delete_result, Continuation<void*>* completion);

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:28-30,254-260
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:131-131
// NOTE(port): Unit's erased ABI result is null; other results transfer an owning R box.
template <typename R>
inline void* emit_transformed_result(FlowCollector<R>* collector, void* box,
                                    Continuation<void*>* completion) {
    std::unique_ptr<R> value(static_cast<R*>(box));
    if constexpr (std::is_same_v<R, Unit>) return collector->emit(Unit{}, completion);
    else return collector->emit(std::move(*value), completion);
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:254-260
template <typename R>
inline std::shared_ptr<Flow<R>> combine_unsafe(
    std::vector<std::shared_ptr<Flow<std::any>>> flows,
    std::function<void*(const std::vector<std::any>&, Continuation<void*>*)> transform) {
    return unsafe_flow<R>([flows = std::move(flows), transform = std::move(transform)](
        FlowCollector<R>* collector, Continuation<void*>* completion) {
        return combine_internal<R>(collector, flows, null_array_factory<std::any>(),
            [transform](FlowCollector<R>* sink, const std::vector<std::any>& values,
                        Continuation<void*>* frame) {
                return emit_combine_result(
                    [transform, &values](Continuation<void*>* continuation) {
                        return transform(values, continuation);
                    },
                    [sink](void* box, Continuation<void*>* continuation) {
                        return emit_transformed_result(sink, box, continuation);
                    },
                    [](void* box) { delete static_cast<R*>(box); }, frame);
            }, completion);
    });
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Zip.kt:254-260
// NOTE(port): Ordinary C++ transforms enter the same lowered suspend implementation.
template <typename R>
inline std::shared_ptr<Flow<R>> combine_unsafe(
    std::vector<std::shared_ptr<Flow<std::any>>> flows,
    std::function<R(const std::vector<std::any>&)> transform) {
    return combine_unsafe<R>(std::move(flows),
        std::function<void*(const std::vector<std::any>&, Continuation<void*>*)>(
            [transform = std::move(transform)](const std::vector<std::any>& values,
                                             Continuation<void*>*) -> void* {
                if constexpr (std::is_same_v<R, Unit>) { transform(values); return nullptr; }
                else return new R(transform(values));
            }));
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:82-139
void* zip_internal_erased(
    std::shared_ptr<Flow<std::any>> first, std::shared_ptr<Flow<std::any>> second,
    std::function<void*(std::any, std::any, Continuation<void*>*)> transform,
    std::function<void*(void*, Continuation<void*>*)> emit,
    std::function<void(void*)> delete_result, Continuation<void*>* completion);

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:82-139
// NOTE(port): Suspend transform returns an owning R box; this adapter unboxes and deletes it.
template <typename T1, typename T2, typename R>
inline std::shared_ptr<Flow<R>> zip_impl(
    std::shared_ptr<Flow<T1>> first, std::shared_ptr<Flow<T2>> second,
    std::function<void*(T1, T2, Continuation<void*>*)> transform) {
    return unsafe_flow<R>([first = as_any_flow(std::move(first)), second = as_any_flow(std::move(second)),
                           transform = std::move(transform)](
        FlowCollector<R>* collector, Continuation<void*>* completion) {
        return zip_internal_erased(first, second,
            [transform](std::any left, std::any right, Continuation<void*>* frame) {
                return transform(std::any_cast<T1>(std::move(left)), std::any_cast<T2>(std::move(right)), frame);
            },
            [collector](void* box, Continuation<void*>* frame) {
                return emit_transformed_result(collector, box, frame);
            },
            [](void* box) { delete static_cast<R*>(box); }, completion);
    });
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Combine.kt:82-139
// NOTE(port): Ordinary C++ transform binds to the same suspend ABI algorithm.
template <typename T1, typename T2, typename R>
inline std::shared_ptr<Flow<R>> zip_impl(
    std::shared_ptr<Flow<T1>> first, std::shared_ptr<Flow<T2>> second,
    std::function<R(T1, T2)> transform) {
    return zip_impl<T1, T2, R>(std::move(first), std::move(second),
        std::function<void*(T1, T2, Continuation<void*>*)>(
            [transform = std::move(transform)](T1 left, T2 right, Continuation<void*>*) -> void* {
                if constexpr (std::is_same_v<R, Unit>) {
                    transform(std::move(left), std::move(right));
                    return nullptr;
                } else return new R(transform(std::move(left), std::move(right)));
            }));
}

} // namespace kotlinx::coroutines::flow::internal
