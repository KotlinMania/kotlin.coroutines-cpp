// port-lint: source kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt
#pragma once
/**
 * @file Collect.hpp
 * @brief Terminal flow operators: collect, launch_in, collect_indexed, collect_latest, emit_all
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt
 */

#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Unit.hpp"
#include "kotlinx/coroutines/flow/Context.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/Merge.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/flow/internal/NopCollector.hpp"

#include <functional>
#include <memory>
#include <utility>

namespace kotlinx::coroutines::flow {

/**
 * Terminal flow operator that collects the given flow but ignores all emitted values.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:26
 */
template <typename T>
inline void* collect(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    internal::NopCollector<T> nop;
    return flow->collect(&nop, continuation);
}

/**
 * Terminal flow operator that collects the given flow with a provided [action].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:110-113
 */
template <typename T>
inline void* collect(
    std::shared_ptr<Flow<T>> flow,
    std::function<void*(T, Continuation<void*>*)> action,
    Continuation<void*>* continuation) {
    class ActionCollector : public FlowCollector<T> {
        std::function<void*(T, Continuation<void*>*)> action_;
    public:
        ActionCollector(std::function<void*(T, Continuation<void*>*)> a) : action_(std::move(a)) {}
        void* emit(T value, Continuation<void*>* cont) override {
            return action_(std::move(value), cont);
        }
    };
    ActionCollector collector(std::move(action));
    return flow->collect(&collector, continuation);
}

/**
 * Non-suspending action overload of [collect].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:110-113
 */
template <typename T>
inline void* collect(
    std::shared_ptr<Flow<T>> flow,
    std::function<void(T)> action,
    Continuation<void*>* continuation) {
    return collect<T>(
        std::move(flow),
        [action = std::move(action)](T val, Continuation<void*>*) -> void* {
            action(std::move(val));
            return nullptr;
        },
        continuation);
}

/**
 * Terminal flow operator that [launches][launch] the [collection][collect] of the given flow in the [scope].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:45-47
 */
template <typename T>
inline std::shared_ptr<Job> launch_in(std::shared_ptr<Flow<T>> flow, CoroutineScope* scope) {
    return kotlinx::coroutines::launch(scope, [flow = std::move(flow)](CoroutineScope*) {
        internal::NopCollector<T> nop;
        flow->collect(&nop, nullptr);
    });
}

/**
 * Terminal flow operator that collects the given flow with a provided [action] that takes the index
 * of an element (zero-based) and the element.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:55-59
 */
template <typename T>
inline void* collect_indexed(
    std::shared_ptr<Flow<T>> flow,
    std::function<void*(int, T, Continuation<void*>*)> action,
    Continuation<void*>* continuation) {
    class IndexedActionCollector : public FlowCollector<T> {
        std::function<void*(int, T, Continuation<void*>*)> action_;
        int index_ = 0;
    public:
        IndexedActionCollector(std::function<void*(int, T, Continuation<void*>*)> a)
            : action_(std::move(a)) {}
        void* emit(T value, Continuation<void*>* cont) override {
            int idx = internal::check_index_overflow(index_++);
            return action_(idx, std::move(value), cont);
        }
    };
    IndexedActionCollector collector(std::move(action));
    return flow->collect(&collector, continuation);
}

/**
 * Non-suspending action overload of [collect_indexed].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:55-59
 */
template <typename T>
inline void* collect_indexed(
    std::shared_ptr<Flow<T>> flow,
    std::function<void(int, T)> action,
    Continuation<void*>* continuation) {
    return collect_indexed<T>(
        std::move(flow),
        [action = std::move(action)](int idx, T val, Continuation<void*>*) -> void* {
            action(idx, std::move(val));
            return nullptr;
        },
        continuation);
}

/**
 * Terminal flow operator that collects the given flow with a provided [action].
 * When the original flow emits a new value, the action for the previous value is cancelled.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:82-97
 */
template <typename T>
inline void* collect_latest(
    std::shared_ptr<Flow<T>> flow,
    std::function<void*(T, Continuation<void*>*)> action,
    Continuation<void*>* continuation) {
    auto mapped = transform_latest<T, Unit>(
        std::move(flow),
        [action = std::move(action)](FlowCollector<Unit>*, T val, Continuation<void*>* cont) -> void* {
            return action(std::move(val), cont);
        });
    auto buffered = buffer<Unit>(std::move(mapped), 0);
    internal::NopCollector<Unit> nop;
    return buffered->collect(&nop, continuation);
}

/**
 * Non-suspending action overload of [collect_latest].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:82-97
 */
template <typename T>
inline void* collect_latest(
    std::shared_ptr<Flow<T>> flow,
    std::function<void(T)> action,
    Continuation<void*>* continuation) {
    return collect_latest<T>(
        std::move(flow),
        [action = std::move(action)](T val, Continuation<void*>*) -> void* {
            action(std::move(val));
            return nullptr;
        },
        continuation);
}

/**
 * Collects all the values from the given [flow] and emits them to the collector.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:103-106
 */
template <typename T>
inline void* emit_all(FlowCollector<T>* collector, Flow<T>* flow, Continuation<void*>* continuation) {
    if (continuation && continuation->get_context()) {
        context_ensure_active(*continuation->get_context());
    }
    return flow->collect(collector, continuation);
}

/**
 * Shared pointer overload of [emit_all].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Collect.kt:103-106
 */
template <typename T>
inline void* emit_all(FlowCollector<T>* collector, std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    return emit_all(collector, flow.get(), continuation);
}

} // namespace kotlinx::coroutines::flow
