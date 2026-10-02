// port-lint: source kotlinx-coroutines-core/common/src/flow/operators/Transform.kt
#pragma once
/**
 * @file Transform.hpp
 * @brief Flow transformation operators: filter, filter_not, filter_is_instance, filter_not_null,
 *        map, map_not_null, with_index, on_each, scan, running_fold, running_reduce, chunked.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace kotlinx::coroutines::flow {

/**
 * An indexed value holding an integer index and the value.
 *
 * Transliterated from: kotlin.collections.IndexedValue
 */
template <typename T>
struct IndexedValue {
    int index;
    T value;

    IndexedValue() : index(0), value() {}
    IndexedValue(int idx, T val) : index(idx), value(std::move(val)) {}

    bool operator==(const IndexedValue& other) const {
        return index == other.index && value == other.value;
    }
    bool operator!=(const IndexedValue& other) const {
        return !(*this == other);
    }
};

namespace internal {

/**
 * Transforms upstream elements by forwarding them to transform function with downstream collector.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:44-51
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> unsafe_transform(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(FlowCollector<R>*, T, Continuation<void*>*)> transform_fn) {
    class UnsafeTransformFlow : public Flow<R> {
        std::shared_ptr<Flow<T>> upstream_;
        std::function<void*(FlowCollector<R>*, T, Continuation<void*>*)> transform_fn_;
    public:
        UnsafeTransformFlow(
            std::shared_ptr<Flow<T>> upstream,
            std::function<void*(FlowCollector<R>*, T, Continuation<void*>*)> fn)
            : upstream_(std::move(upstream)), transform_fn_(std::move(fn)) {
            (void)fn;
        }

        void* collect(FlowCollector<R>* collector, Continuation<void*>* continuation) override {
            class TransformCollector : public FlowCollector<T> {
                FlowCollector<R>* downstream_;
                const std::function<void*(FlowCollector<R>*, T, Continuation<void*>*)>& transform_fn_;
            public:
                TransformCollector(
                    FlowCollector<R>* d,
                    const std::function<void*(FlowCollector<R>*, T, Continuation<void*>*)>& fn)
                    : downstream_(d), transform_fn_(fn) {
                    (void)fn;
                }

                void* emit(T value, Continuation<void*>* cont) override {
                    return transform_fn_(downstream_, std::move(value), cont);
                }
            };
            TransformCollector tc(collector, transform_fn_);
            return upstream_->collect(&tc, continuation);
        }
    };
    return std::make_shared<UnsafeTransformFlow>(std::move(upstream), std::move(transform_fn));
}

} // namespace internal

/**
 * Returns a flow containing only values of the original flow that match the given [predicate].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:17-19
 */
template <typename T>
inline std::shared_ptr<Flow<T>> filter(
    std::shared_ptr<Flow<T>> upstream,
    std::function<bool(const T&)> predicate) {
    return internal::unsafe_transform<T, T>(
        std::move(upstream),
        [predicate = std::move(predicate)](
            FlowCollector<T>* collector, T value, Continuation<void*>* cont) -> void* {
            if (predicate(value)) {
                return collector->emit(std::move(value), cont);
            }
            return nullptr;
        });
}

/**
 * Suspending overload of [filter].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:17-19
 */
template <typename T>
inline std::shared_ptr<Flow<T>> filter(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(const T&, Continuation<void*>*)> predicate) {
    return internal::unsafe_transform<T, T>(
        std::move(upstream),
        [predicate = std::move(predicate)](
            FlowCollector<T>* collector, T value, Continuation<void*>* cont) -> void* {
            void* res = predicate(value, cont);
            if (intrinsics::is_coroutine_suspended(res)) {
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
            bool matches = false;
            if (res) {
                auto* b = static_cast<bool*>(res);
                matches = *b;
                delete b;
            }
            if (matches) {
                return collector->emit(std::move(value), cont);
            }
            return nullptr;
        });
}

/**
 * Returns a flow containing only values of the original flow that do not match the given [predicate].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:24-26
 */
template <typename T>
inline std::shared_ptr<Flow<T>> filter_not(
    std::shared_ptr<Flow<T>> upstream,
    std::function<bool(const T&)> predicate) {
    return internal::unsafe_transform<T, T>(
        std::move(upstream),
        [predicate = std::move(predicate)](
            FlowCollector<T>* collector, T value, Continuation<void*>* cont) -> void* {
            if (!predicate(value)) {
                return collector->emit(std::move(value), cont);
            }
            return nullptr;
        });
}

/**
 * Suspending overload of [filter_not].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:24-26
 */
template <typename T>
inline std::shared_ptr<Flow<T>> filter_not(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(const T&, Continuation<void*>*)> predicate) {
    return internal::unsafe_transform<T, T>(
        std::move(upstream),
        [predicate = std::move(predicate)](
            FlowCollector<T>* collector, T value, Continuation<void*>* cont) -> void* {
            void* res = predicate(value, cont);
            if (intrinsics::is_coroutine_suspended(res)) {
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
            bool matches = false;
            if (res) {
                auto* b = static_cast<bool*>(res);
                matches = *b;
                delete b;
            }
            if (!matches) {
                return collector->emit(std::move(value), cont);
            }
            return nullptr;
        });
}

/**
 * Returns a flow containing only values that are instances of specified type [R] (shared_ptr).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:32-37
 */
template <typename R, typename T>
inline std::shared_ptr<Flow<std::shared_ptr<R>>> filter_is_instance(
    std::shared_ptr<Flow<std::shared_ptr<T>>> upstream) {
    return internal::unsafe_transform<std::shared_ptr<T>, std::shared_ptr<R>>(
        std::move(upstream),
        [](FlowCollector<std::shared_ptr<R>>* collector,
           std::shared_ptr<T> value,
           Continuation<void*>* cont) -> void* {
            auto derived = std::dynamic_pointer_cast<R>(value);
            if (derived) {
                return collector->emit(std::move(derived), cont);
            }
            return nullptr;
        });
}

/**
 * Returns a flow containing only values that are instances of specified type [R] (raw pointers).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:32-37
 */
template <typename R, typename T>
inline std::shared_ptr<Flow<R*>> filter_is_instance(
    std::shared_ptr<Flow<T*>> upstream) {
    return internal::unsafe_transform<T*, R*>(
        std::move(upstream),
        [](FlowCollector<R*>* collector,
           T* value,
           Continuation<void*>* cont) -> void* {
            if (auto* derived = dynamic_cast<R*>(value)) {
                return collector->emit(derived, cont);
            }
            return nullptr;
        });
}

/**
 * Returns a flow containing only values of the original flow that are not null (raw pointer overload).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:42-44
 */
template <typename T>
inline std::shared_ptr<Flow<T*>> filter_not_null(
    std::shared_ptr<Flow<T*>> upstream) {
    return internal::unsafe_transform<T*, T*>(
        std::move(upstream),
        [](FlowCollector<T*>* collector, T* value, Continuation<void*>* cont) -> void* {
            if (value != nullptr) {
                return collector->emit(value, cont);
            }
            return nullptr;
        });
}

/**
 * Returns a flow containing only values of the original flow that are not null (shared_ptr overload).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:42-44
 */
template <typename T>
inline std::shared_ptr<Flow<std::shared_ptr<T>>> filter_not_null(
    std::shared_ptr<Flow<std::shared_ptr<T>>> upstream) {
    return internal::unsafe_transform<std::shared_ptr<T>, std::shared_ptr<T>>(
        std::move(upstream),
        [](FlowCollector<std::shared_ptr<T>>* collector,
           std::shared_ptr<T> value,
           Continuation<void*>* cont) -> void* {
            if (value != nullptr) {
                return collector->emit(std::move(value), cont);
            }
            return nullptr;
        });
}

/**
 * Returns a flow containing only values of the original flow that are not null (std::optional overload).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:42-44
 */
template <typename T>
inline std::shared_ptr<Flow<T>> filter_not_null(
    std::shared_ptr<Flow<std::optional<T>>> upstream) {
    return internal::unsafe_transform<std::optional<T>, T>(
        std::move(upstream),
        [](FlowCollector<T>* collector, std::optional<T> value, Continuation<void*>* cont) -> void* {
            if (value.has_value()) {
                return collector->emit(std::move(*value), cont);
            }
            return nullptr;
        });
}

/**
 * Returns a flow containing the results of applying the given [transform] function to each value of the original flow.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:49-51
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> map(
    std::shared_ptr<Flow<T>> upstream,
    std::function<R(T)> transform_fn) {
    return internal::unsafe_transform<T, R>(
        std::move(upstream),
        [transform_fn = std::move(transform_fn)](
            FlowCollector<R>* collector, T value, Continuation<void*>* cont) -> void* {
            return collector->emit(transform_fn(std::move(value)), cont);
        });
}

/**
 * Suspending overload of [map].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:49-51
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> map(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(T, Continuation<void*>*)> transform_fn) {
    return internal::unsafe_transform<T, R>(
        std::move(upstream),
        [transform_fn = std::move(transform_fn)](
            FlowCollector<R>* collector, T value, Continuation<void*>* cont) -> void* {
            void* res = transform_fn(std::move(value), cont);
            if (intrinsics::is_coroutine_suspended(res)) {
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
            if (res) {
                auto* val = static_cast<R*>(res);
                R r = std::move(*val);
                delete val;
                return collector->emit(std::move(r), cont);
            }
            return nullptr;
        });
}

/**
 * Returns a flow that contains only non-null results of applying the given [transform] function (std::optional overload).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:56-59
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> map_not_null(
    std::shared_ptr<Flow<T>> upstream,
    std::function<std::optional<R>(T)> transform_fn) {
    return internal::unsafe_transform<T, R>(
        std::move(upstream),
        [transform_fn = std::move(transform_fn)](
            FlowCollector<R>* collector, T value, Continuation<void*>* cont) -> void* {
            auto transformed = transform_fn(std::move(value));
            if (transformed.has_value()) {
                return collector->emit(std::move(*transformed), cont);
            }
            return nullptr;
        });
}

/**
 * Returns a flow that contains only non-null results of applying the given [transform] function (raw pointer overload).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:56-59
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> map_not_null(
    std::shared_ptr<Flow<T>> upstream,
    std::function<R*(T)> transform_fn) {
    return internal::unsafe_transform<T, R>(
        std::move(upstream),
        [transform_fn = std::move(transform_fn)](
            FlowCollector<R>* collector, T value, Continuation<void*>* cont) -> void* {
            auto* transformed = transform_fn(std::move(value));
            if (transformed != nullptr) {
                return collector->emit(std::move(*transformed), cont);
            }
            return nullptr;
        });
}

/**
 * Returns a flow that contains only non-null results of applying the given [transform] function (shared_ptr overload).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:56-59
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<std::shared_ptr<R>>> map_not_null(
    std::shared_ptr<Flow<T>> upstream,
    std::function<std::shared_ptr<R>(T)> transform_fn) {
    return internal::unsafe_transform<T, std::shared_ptr<R>>(
        std::move(upstream),
        [transform_fn = std::move(transform_fn)](
            FlowCollector<std::shared_ptr<R>>* collector, T value, Continuation<void*>* cont) -> void* {
            auto transformed = transform_fn(std::move(value));
            if (transformed != nullptr) {
                return collector->emit(std::move(transformed), cont);
            }
            return nullptr;
        });
}

/**
 * Returns a flow that wraps each element into [IndexedValue], containing value and its index (starting from zero).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:64-69
 */
template <typename T>
inline std::shared_ptr<Flow<IndexedValue<T>>> with_index(
    std::shared_ptr<Flow<T>> upstream) {
    return flow<IndexedValue<T>>(
        [upstream = std::move(upstream)](
            FlowCollector<IndexedValue<T>>* collector,
            Continuation<void*>* completion) -> void* {
            auto index = std::make_shared<int>(0);
            class WithIndexCollector : public FlowCollector<T> {
                FlowCollector<IndexedValue<T>>* downstream_;
                std::shared_ptr<int> index_;
            public:
                WithIndexCollector(FlowCollector<IndexedValue<T>>* d, std::shared_ptr<int> idx)
                    : downstream_(d), index_(std::move(idx)) {}

                void* emit(T value, Continuation<void*>* cont) override {
                    int curr = internal::check_index_overflow((*index_)++);
                    return downstream_->emit(IndexedValue<T>(curr, std::move(value)), cont);
                }
            };
            WithIndexCollector wic(collector, index);
            return upstream->collect(&wic, completion);
        });
}

/**
 * Returns a flow that invokes the given [action] **before** each value of the upstream flow is emitted downstream.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:74-77
 */
template <typename T>
inline std::shared_ptr<Flow<T>> on_each(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void(const T&)> action) {
    return internal::unsafe_transform<T, T>(
        std::move(upstream),
        [action = std::move(action)](
            FlowCollector<T>* collector, T value, Continuation<void*>* cont) -> void* {
            action(value);
            return collector->emit(std::move(value), cont);
        });
}

/**
 * Suspending overload of [on_each].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:74-77
 */
template <typename T>
inline std::shared_ptr<Flow<T>> on_each(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(const T&, Continuation<void*>*)> action) {
    return internal::unsafe_transform<T, T>(
        std::move(upstream),
        [action = std::move(action)](
            FlowCollector<T>* collector, T value, Continuation<void*>* cont) -> void* {
            void* res = action(value, cont);
            if (intrinsics::is_coroutine_suspended(res)) {
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
            return collector->emit(std::move(value), cont);
        });
}

/**
 * Folds the given flow with [operation], emitting every intermediate result, including [initial] value.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:101-108
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> running_fold(
    std::shared_ptr<Flow<T>> upstream,
    R initial,
    std::function<R(R, T)> operation) {
    return flow<R>(
        [upstream = std::move(upstream),
         initial = std::move(initial),
         operation = std::move(operation)](
            FlowCollector<R>* collector,
            Continuation<void*>* completion) -> void* {
            auto acc = std::make_shared<R>(initial);
            void* emit_init = collector->emit(*acc, completion);
            if (intrinsics::is_coroutine_suspended(emit_init)) {
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
            class RunningFoldCollector : public FlowCollector<T> {
                FlowCollector<R>* downstream_;
                std::shared_ptr<R> acc_;
                const std::function<R(R, T)>& op_;
            public:
                RunningFoldCollector(FlowCollector<R>* d, std::shared_ptr<R> a, const std::function<R(R, T)>& op)
                    : downstream_(d), acc_(std::move(a)), op_(op) {
                    (void)op;
                }

                void* emit(T value, Continuation<void*>* cont) override {
                    *acc_ = op_(*acc_, std::move(value));
                    return downstream_->emit(*acc_, cont);
                }
            };
            RunningFoldCollector rfc(collector, acc, operation);
            return upstream->collect(&rfc, completion);
        });
}

/**
 * Suspending overload of [running_fold].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:101-108
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> running_fold(
    std::shared_ptr<Flow<T>> upstream,
    R initial,
    std::function<void*(R, T, Continuation<void*>*)> operation) {
    return flow<R>(
        [upstream = std::move(upstream),
         initial = std::move(initial),
         operation = std::move(operation)](
            FlowCollector<R>* collector,
            Continuation<void*>* completion) -> void* {
            auto acc = std::make_shared<R>(initial);
            void* emit_init = collector->emit(*acc, completion);
            if (intrinsics::is_coroutine_suspended(emit_init)) {
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
            class SuspendingRunningFoldCollector : public FlowCollector<T> {
                FlowCollector<R>* downstream_;
                std::shared_ptr<R> acc_;
                const std::function<void*(R, T, Continuation<void*>*)>& op_;
            public:
                SuspendingRunningFoldCollector(
                    FlowCollector<R>* d,
                    std::shared_ptr<R> a,
                    const std::function<void*(R, T, Continuation<void*>*)>& op)
                    : downstream_(d), acc_(std::move(a)), op_(op) {
                    (void)op;
                }

                void* emit(T value, Continuation<void*>* cont) override {
                    void* res = op_(*acc_, std::move(value), cont);
                    if (intrinsics::is_coroutine_suspended(res)) {
                        return intrinsics::get_COROUTINE_SUSPENDED();
                    }
                    if (res) {
                        auto* val = static_cast<R*>(res);
                        *acc_ = std::move(*val);
                        delete val;
                    }
                    return downstream_->emit(*acc_, cont);
                }
            };
            SuspendingRunningFoldCollector rfc(collector, acc, operation);
            return upstream->collect(&rfc, completion);
        });
}

/**
 * Folds the given flow with [operation], emitting every intermediate result, including [initial] value.
 * Alias to [running_fold].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:90
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> scan(
    std::shared_ptr<Flow<T>> upstream,
    R initial,
    std::function<R(R, T)> operation) {
    return running_fold<T, R>(std::move(upstream), std::move(initial), std::move(operation));
}

/**
 * Suspending overload of [scan].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:90
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> scan(
    std::shared_ptr<Flow<T>> upstream,
    R initial,
    std::function<void*(R, T, Continuation<void*>*)> operation) {
    return running_fold<T, R>(std::move(upstream), std::move(initial), std::move(operation));
}

/**
 * Reduces the given flow with [operation], emitting every intermediate result, including initial value.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:121-131
 */
template <typename T>
inline std::shared_ptr<Flow<T>> running_reduce(
    std::shared_ptr<Flow<T>> upstream,
    std::function<T(T, T)> operation) {
    return flow<T>(
        [upstream = std::move(upstream),
         operation = std::move(operation)](
            FlowCollector<T>* collector,
            Continuation<void*>* completion) -> void* {
            auto acc = std::make_shared<std::optional<T>>();
            class RunningReduceCollector : public FlowCollector<T> {
                FlowCollector<T>* downstream_;
                std::shared_ptr<std::optional<T>> acc_;
                const std::function<T(T, T)>& op_;
            public:
                RunningReduceCollector(
                    FlowCollector<T>* d,
                    std::shared_ptr<std::optional<T>> a,
                    const std::function<T(T, T)>& op)
                    : downstream_(d), acc_(std::move(a)), op_(op) {
                    (void)op;
                }

                void* emit(T value, Continuation<void*>* cont) override {
                    if (!acc_->has_value()) {
                        acc_->emplace(std::move(value));
                    } else {
                        *acc_ = op_(std::move(**acc_), std::move(value));
                    }
                    return downstream_->emit(**acc_, cont);
                }
            };
            RunningReduceCollector rrc(collector, acc, operation);
            return upstream->collect(&rrc, completion);
        });
}

/**
 * Suspending overload of [running_reduce].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:121-131
 */
template <typename T>
inline std::shared_ptr<Flow<T>> running_reduce(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(T, T, Continuation<void*>*)> operation) {
    return flow<T>(
        [upstream = std::move(upstream),
         operation = std::move(operation)](
            FlowCollector<T>* collector,
            Continuation<void*>* completion) -> void* {
            auto acc = std::make_shared<std::optional<T>>();
            class SuspendingRunningReduceCollector : public FlowCollector<T> {
                FlowCollector<T>* downstream_;
                std::shared_ptr<std::optional<T>> acc_;
                const std::function<void*(T, T, Continuation<void*>*)>& op_;
            public:
                SuspendingRunningReduceCollector(
                    FlowCollector<T>* d,
                    std::shared_ptr<std::optional<T>> a,
                    const std::function<void*(T, T, Continuation<void*>*)>& op)
                    : downstream_(d), acc_(std::move(a)), op_(op) {
                    (void)op;
                }

                void* emit(T value, Continuation<void*>* cont) override {
                    if (!acc_->has_value()) {
                        acc_->emplace(std::move(value));
                    } else {
                        void* res = op_(std::move(**acc_), std::move(value), cont);
                        if (intrinsics::is_coroutine_suspended(res)) {
                            return intrinsics::get_COROUTINE_SUSPENDED();
                        }
                        if (res) {
                            auto* val = static_cast<T*>(res);
                            *acc_ = std::move(*val);
                            delete val;
                        }
                    }
                    return downstream_->emit(**acc_, cont);
                }
            };
            SuspendingRunningReduceCollector rrc(collector, acc, operation);
            return upstream->collect(&rrc, completion);
        });
}

/**
 * Splits the given flow into a flow of non-overlapping lists each not exceeding the given [size] but never empty.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:150-166
 */
template <typename T>
inline std::shared_ptr<Flow<std::vector<T>>> chunked(
    std::shared_ptr<Flow<T>> upstream,
    int size) {
    if (size < 1) {
        throw std::invalid_argument("Expected positive chunk size, but got " + std::to_string(size));
    }
    return flow<std::vector<T>>(
        [upstream = std::move(upstream), size](
            FlowCollector<std::vector<T>>* collector,
            Continuation<void*>* completion) -> void* {
            auto chunk = std::make_shared<std::vector<T>>();
            chunk->reserve(size);
            class ChunkedCollector : public FlowCollector<T> {
                FlowCollector<std::vector<T>>* downstream_;
                std::shared_ptr<std::vector<T>> chunk_;
                int size_;
            public:
                ChunkedCollector(
                    FlowCollector<std::vector<T>>* d,
                    std::shared_ptr<std::vector<T>> c,
                    int s)
                    : downstream_(d), chunk_(std::move(c)), size_(s) {
                    (void)s;
                }

                void* emit(T value, Continuation<void*>* cont) override {
                    chunk_->push_back(std::move(value));
                    if (static_cast<int>(chunk_->size()) == size_) {
                        std::vector<T> batch = std::move(*chunk_);
                        chunk_->clear();
                        chunk_->reserve(size_);
                        return downstream_->emit(std::move(batch), cont);
                    }
                    return nullptr;
                }
            };
            ChunkedCollector cc(collector, chunk, size);
            void* res = upstream->collect(&cc, completion);
            if (intrinsics::is_coroutine_suspended(res)) {
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
            if (!chunk->empty()) {
                std::vector<T> remaining = std::move(*chunk);
                chunk->clear();
                return collector->emit(std::move(remaining), completion);
            }
            return nullptr;
        });
}

} // namespace kotlinx::coroutines::flow
