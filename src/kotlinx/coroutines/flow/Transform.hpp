// port-lint: source kotlinx-coroutines-core/common/src/flow/operators/Transform.kt
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt
#pragma once
/**
 * @file Transform.hpp
 * @brief Flow transformation operators: filter, filter_not, filter_is_instance, filter_not_null,
 *        map, map_not_null, with_index, on_each, scan, running_fold, running_reduce, chunked.
 */

#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/flow/internal/SafeCollector.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <concepts>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace kotlinx::coroutines::flow {

/**
 * An indexed value holding an integer index and the associated value.
 */
// Transliterated from: kotlin.collections.IndexedValue
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

namespace detail {

// Adapt non-suspending and suspending callables to the Continuation Boolean ABI.
template<typename Predicate, typename... Args>
void* invoke_transform_predicate(Predicate& predicate, Continuation<void*>* continuation, Args&&... args) {
    if constexpr (std::is_invocable_v<Predicate&, Args..., Continuation<void*>*>) {
        static_assert(std::is_same_v<std::invoke_result_t<Predicate&, Args..., Continuation<void*>*>, void*>);
        return predicate(std::forward<Args>(args)..., continuation);
    } else {
        return new bool(predicate(std::forward<Args>(args)...));
    }
}

// Adapt non-suspending and suspending transform functions to the Continuation ABI.
template<typename Transform, typename T>
void* invoke_transform_fn(Transform& transform, Continuation<void*>* continuation, T&& value) {
    if constexpr (std::is_invocable_v<Transform&, T, Continuation<void*>*>) {
        return transform(std::forward<T>(value), continuation);
    } else {
        using Ret = std::invoke_result_t<Transform&, T>;
        return new Ret(transform(std::forward<T>(value)));
    }
}

// Adapt non-suspending and suspending actions to the Continuation Unit ABI.
template<typename Action, typename T>
void* invoke_action_fn(Action& action, Continuation<void*>* continuation, const T& value) {
    if constexpr (std::is_invocable_v<Action&, const T&, Continuation<void*>*>) {
        return action(value, continuation);
    } else if constexpr (std::is_invocable_v<Action&, T, Continuation<void*>*>) {
        return action(value, continuation);
    } else {
        action(value);
        return nullptr;
    }
}

// Adapt non-suspending and suspending fold operations to the Continuation ABI.
template<typename Op, typename Acc, typename Value>
void* invoke_fold_op(Op& op, Continuation<void*>* continuation, const Acc& acc, Value&& val) {
    if constexpr (std::is_invocable_v<Op&, Acc, Value, Continuation<void*>*>) {
        return op(acc, std::forward<Value>(val), continuation);
    } else if constexpr (std::is_invocable_v<Op&, const Acc&, Value, Continuation<void*>*>) {
        return op(acc, std::forward<Value>(val), continuation);
    } else {
        using Ret = std::invoke_result_t<Op&, const Acc&, Value>;
        return new Ret(op(acc, std::forward<Value>(val)));
    }
}

// Adapt non-suspending and suspending reduce operations to the Continuation ABI.
template<typename Op, typename Acc, typename Value>
void* invoke_reduce_op(Op& op, Continuation<void*>* continuation, const Acc& acc, Value&& val) {
    if constexpr (std::is_invocable_v<Op&, Acc, Value, Continuation<void*>*>) {
        return op(acc, std::forward<Value>(val), continuation);
    } else if constexpr (std::is_invocable_v<Op&, const Acc&, Value, Continuation<void*>*>) {
        return op(acc, std::forward<Value>(val), continuation);
    } else {
        using Ret = std::invoke_result_t<Op&, const Acc&, Value>;
        return new Ret(op(acc, std::forward<Value>(val)));
    }
}

} // namespace detail

namespace internal {

/**
 * Transforms upstream elements by forwarding them to the transform function with the downstream collector.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:44-51
template <typename T, typename R, typename TransformBlock>
inline std::shared_ptr<Flow<R>> unsafe_transform(std::shared_ptr<Flow<T>> upstream, TransformBlock transform) {
    return internal::unsafe_flow<R>([upstream = std::move(upstream), transform = std::move(transform)](
        FlowCollector<R>* collector, Continuation<void*>* completion) mutable -> void* {
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:44-51
        // NOTE(port): This typed binding represents the source collector lambda;
        // its collection lifetime is lowered by the compiler, not a manual frame.
        class TransformCollector final : public FlowCollector<T>,
            public std::enable_shared_from_this<TransformCollector> {
        public:
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:44-51
            TransformCollector(std::shared_ptr<Flow<T>> upstream, FlowCollector<R>* downstream,
                TransformBlock transform)
                : upstream_(std::move(upstream)), downstream_(downstream), transform_(std::move(transform)) {}

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:47-51
            [[clang::annotate("suspend")]]
            void* collect(std::shared_ptr<Continuation<void*>> completion) {
                auto owner = this->shared_from_this();
                dsl::suspend(upstream_->collect(this, completion.get()));
                return nullptr;
            }

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:48-50
            void* emit(T value, Continuation<void*>* completion) override {
                // Return the transform result directly so the source tail call is preserved (KT-28938).
                return transform_(downstream_, std::move(value), completion);
            }

        private:
            const std::shared_ptr<Flow<T>> upstream_;
            FlowCollector<R>* const downstream_;
            TransformBlock transform_;
        };

        auto receiver = std::make_shared<TransformCollector>(upstream, collector, transform);
        return receiver->collect(kotlinx::coroutines::internal::retain_continuation(completion));
    });
}

} // namespace internal

/**
 * Returns a flow containing only values of the original flow that match the given predicate.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:17-19
template <typename T, typename Predicate>
inline std::shared_ptr<Flow<T>> filter(std::shared_ptr<Flow<T>> upstream, Predicate predicate_fn) {
    // NOTE(port): Own the supplied C++ callable; the downstream collector remains borrowed.
    auto predicate = std::make_shared<Predicate>(std::move(predicate_fn));
    auto block = [predicate = std::move(predicate)](FlowCollector<T>* collector, T value,
        std::shared_ptr<Continuation<void*>> completion)
        __attribute__((annotate("suspend"))) -> void* {
        void* raw = dsl::suspend(detail::invoke_transform_predicate(*predicate, completion.get(), value));
        // NOTE(port): This receiving side owns the erased Boolean result box.
        std::unique_ptr<bool> box(static_cast<bool*>(raw));
        bool accepted = *box;
        box.reset();
        if (accepted) {
            dsl::suspend(collector->emit(std::move(value), completion.get()));
        }
        return nullptr;
    };
    return internal::unsafe_transform<T, T>(std::move(upstream),
        [block = std::move(block)](FlowCollector<T>* collector, T value,
            Continuation<void*>* completion) -> void* {
            return block(collector, std::move(value), kotlinx::coroutines::internal::retain_continuation(completion));
        });
}

/**
 * Synchronous functional overload of filter.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:17-19
template <typename T>
inline std::shared_ptr<Flow<T>> filter(
    std::shared_ptr<Flow<T>> upstream,
    std::function<bool(const T&)> predicate) {
    return filter<T, std::function<bool(const T&)>>(std::move(upstream), std::move(predicate));
}

/**
 * Suspending functional overload of filter.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:17-19
template <typename T>
inline std::shared_ptr<Flow<T>> filter(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(const T&, Continuation<void*>*)> predicate) {
    return filter<T, std::function<void*(const T&, Continuation<void*>*)>>(std::move(upstream), std::move(predicate));
}

/**
 * Returns a flow containing only values of the original flow that do not match the given predicate.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:24-26
template <typename T, typename Predicate>
inline std::shared_ptr<Flow<T>> filter_not(std::shared_ptr<Flow<T>> upstream, Predicate predicate_fn) {
    // NOTE(port): Own the supplied C++ callable; the downstream collector remains borrowed.
    auto predicate = std::make_shared<Predicate>(std::move(predicate_fn));
    auto block = [predicate = std::move(predicate)](FlowCollector<T>* collector, T value,
        std::shared_ptr<Continuation<void*>> completion)
        __attribute__((annotate("suspend"))) -> void* {
        void* raw = dsl::suspend(detail::invoke_transform_predicate(*predicate, completion.get(), value));
        // NOTE(port): This receiving side owns the erased Boolean result box.
        std::unique_ptr<bool> box(static_cast<bool*>(raw));
        bool accepted = *box;
        box.reset();
        if (!accepted) {
            dsl::suspend(collector->emit(std::move(value), completion.get()));
        }
        return nullptr;
    };
    return internal::unsafe_transform<T, T>(std::move(upstream),
        [block = std::move(block)](FlowCollector<T>* collector, T value,
            Continuation<void*>* completion) -> void* {
            return block(collector, std::move(value), kotlinx::coroutines::internal::retain_continuation(completion));
        });
}

/**
 * Synchronous functional overload of filter_not.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:24-26
template <typename T>
inline std::shared_ptr<Flow<T>> filter_not(
    std::shared_ptr<Flow<T>> upstream,
    std::function<bool(const T&)> predicate) {
    return filter_not<T, std::function<bool(const T&)>>(std::move(upstream), std::move(predicate));
}

/**
 * Suspending functional overload of filter_not.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:24-26
template <typename T>
inline std::shared_ptr<Flow<T>> filter_not(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(const T&, Continuation<void*>*)> predicate) {
    return filter_not<T, std::function<void*(const T&, Continuation<void*>*)>>(std::move(upstream), std::move(predicate));
}

/**
 * Returns a flow containing only values that are instances of specified type R (shared_ptr representation).
 *
 * Downcasts each element via `std::dynamic_pointer_cast<R>`. If the downcast succeeds (non-null pointer),
 * the downcast shared pointer is emitted downstream; otherwise, the element is filtered out.
 *
 * @tparam R The target derived type to retain.
 * @tparam T The source base type.
 * @param upstream The source flow of shared pointers.
 * @return A flow emitting only non-null `std::shared_ptr<R>` instances.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:32-37
template <typename R, typename T>
inline std::shared_ptr<Flow<std::shared_ptr<R>>> filter_is_instance(
    std::shared_ptr<Flow<std::shared_ptr<T>>> upstream) {
    return internal::unsafe_transform<std::shared_ptr<T>, std::shared_ptr<R>>(
        std::move(upstream),
        [](FlowCollector<std::shared_ptr<R>>* collector,
           std::shared_ptr<T> value,
           Continuation<void*>* cont) -> void* {
            auto derived = std::dynamic_pointer_cast<R>(value);
            if (derived != nullptr) {
                return collector->emit(std::move(derived), cont);
            }
            return nullptr;
        });
}

/**
 * Returns a flow containing only values that are instances of specified type R (raw pointer representation).
 *
 * Downcasts each element via `dynamic_cast<R*>`. If the downcast succeeds (non-null pointer),
 * the pointer is emitted downstream; otherwise, the element is filtered out.
 *
 * @tparam R The target derived type to retain.
 * @tparam T The source base type.
 * @param upstream The source flow of base pointers.
 * @return A flow emitting only non-null `R*` instances.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:32-37
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
 * Filters out `nullptr` raw pointers, emitting only non-null pointer values downstream.
 *
 * @tparam T Pointer element type.
 * @param upstream The source flow of raw pointers.
 * @return A flow emitting only non-null `T*` values.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:42-44
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
 * Filters out null `std::shared_ptr<T>` instances, emitting only non-null shared pointers downstream.
 *
 * @tparam T Element type held by the shared pointer.
 * @param upstream The source flow of shared pointers.
 * @return A flow emitting only non-null `std::shared_ptr<T>` instances.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:42-44
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
 * Filters out disengaged `std::optional<T>` values (`std::nullopt`), unwrapping and emitting engaged
 * values `T` downstream.
 *
 * @tparam T The unwrapped element type.
 * @param upstream The source flow of optional values.
 * @return A flow emitting only unwrapped `T` values.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:42-44
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
 * Returns a flow containing the results of applying transform to each upstream value.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:49-51
template <typename T, typename R, typename Transform>
inline std::shared_ptr<Flow<R>> map(std::shared_ptr<Flow<T>> upstream, Transform transform_fn) {
    // NOTE(port): The supplied callable is moved into actual owned storage. The
    // source closure keeps the same transform across collection and emission.
    auto transform = std::make_shared<Transform>(std::move(transform_fn));
    auto block = [transform = std::move(transform)](FlowCollector<R>* collector, T value,
        std::shared_ptr<Continuation<void*>> completion)
        __attribute__((annotate("suspend"))) -> void* {
        void* raw = dsl::suspend(detail::invoke_transform_fn(*transform, completion.get(), std::move(value)));
        // NOTE(port): The receiving side owns the R box for both ordinary and
        // suspending callables. Delete it before the source downstream emit.
        std::unique_ptr<R> box(static_cast<R*>(raw));
        R transformed = std::move(*box);
        box.reset();
        dsl::suspend(collector->emit(std::move(transformed), completion.get()));
        return nullptr;
    };
    return internal::unsafe_transform<T, R>(std::move(upstream),
        [block = std::move(block)](FlowCollector<R>* collector, T value,
            Continuation<void*>* completion) -> void* {
            return block(collector, std::move(value), kotlinx::coroutines::internal::retain_continuation(completion));
        });
}

/**
 * Synchronous functional overload of map.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:49-51
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> map(
    std::shared_ptr<Flow<T>> upstream,
    std::function<R(T)> transform_fn) {
    return map<T, R, std::function<R(T)>>(std::move(upstream), std::move(transform_fn));
}

/**
 * Suspending functional overload of map.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:49-51
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> map(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(T, Continuation<void*>*)> transform_fn) {
    return map<T, R, std::function<void*(T, Continuation<void*>*)>>(std::move(upstream), std::move(transform_fn));
}

/**
 * Returns a flow that contains only non-null results of applying the given transform function.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:56-59
template <typename T, typename R, typename Transform>
    requires (requires(Transform& fn, T value) {
        { fn(std::move(value)) } -> std::same_as<std::optional<R>>;
    } || requires(Transform& fn, T value, Continuation<void*>* continuation) {
        { fn(std::move(value), continuation) } -> std::same_as<void*>;
    })
inline std::shared_ptr<Flow<R>> map_not_null(
    std::shared_ptr<Flow<T>> upstream,
    Transform transform_fn) {
    auto transform = std::make_shared<Transform>(std::move(transform_fn));
    auto block = [transform = std::move(transform)](FlowCollector<R>* collector, T value,
        std::shared_ptr<Continuation<void*>> completion)
        __attribute__((annotate("suspend"))) -> void* {
        void* raw = dsl::suspend(detail::invoke_transform_fn(*transform, completion.get(), std::move(value)));
        // NOTE(port): Nullable R is an owning optional result box, including an
        // absent result. A null erased result is accepted as absent by this C++ ABI.
        std::unique_ptr<std::optional<R>> box(static_cast<std::optional<R>*>(raw));
        std::optional<R> transformed;
        if (box && box->has_value()) transformed.emplace(std::move(**box));
        box.reset();
        if (!transformed.has_value()) return nullptr;
        dsl::suspend(collector->emit(std::move(*transformed), completion.get()));
        return nullptr;
    };
    return internal::unsafe_transform<T, R>(std::move(upstream),
        [block = std::move(block)](FlowCollector<R>* collector, T value,
            Continuation<void*>* completion) -> void* {
            return block(collector, std::move(value), kotlinx::coroutines::internal::retain_continuation(completion));
        });
}

/**
 * Synchronous std::optional overload of map_not_null.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:56-59
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> map_not_null(
    std::shared_ptr<Flow<T>> upstream,
    std::function<std::optional<R>(T)> transform_fn) {
    return map_not_null<T, R, std::function<std::optional<R>(T)>>(std::move(upstream), std::move(transform_fn));
}

/**
 * Suspending std::optional overload of map_not_null.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:56-59
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> map_not_null(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(T, Continuation<void*>*)> transform_fn) {
    return map_not_null<T, R, std::function<void*(T, Continuation<void*>*)>>(std::move(upstream), std::move(transform_fn));
}

/**
 * Synchronous raw pointer overload of map_not_null.
 * Non-null results are borrowed pointers: emits a copy of the pointed-to value
 * without deleting or moving from the caller-owned object.
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
                return collector->emit(*transformed, cont);
            }
            return nullptr;
        });
}

/**
 * Synchronous shared_ptr overload of map_not_null.
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
 * Returns a flow that wraps each element into IndexedValue, containing value and its index starting from zero.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:64-69
template <typename T>
inline std::shared_ptr<Flow<IndexedValue<T>>> with_index(
    std::shared_ptr<Flow<T>> upstream) {
    return internal::unsafe_flow<IndexedValue<T>>([upstream = std::move(upstream)](
        FlowCollector<IndexedValue<T>>* collector, Continuation<void*>* completion) -> void* {
        // NOTE(port): The typed source collector captures the per-collection index.
        // The compiler retains its actual owner through upstream collection.
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:65-68
        class WithIndexCollector final : public FlowCollector<T>,
            public std::enable_shared_from_this<WithIndexCollector> {
        public:
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:65-68
            WithIndexCollector(std::shared_ptr<Flow<T>> upstream, FlowCollector<IndexedValue<T>>* downstream)
                : upstream_(std::move(upstream)), downstream_(downstream) {}

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:66-68
            [[clang::annotate("suspend")]]
            void* collect(std::shared_ptr<Continuation<void*>> completion) {
                auto owner = this->shared_from_this();
                dsl::suspend(upstream_->collect(this, completion.get()));
                return nullptr;
            }

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:67-67
            void* emit(T value, Continuation<void*>* completion) override {
                const int index = index_;
                // NOTE(port): Kotlin Int post-increment wraps; signed C++ overflow is undefined.
                index_ = index_ == std::numeric_limits<int>::max() ? std::numeric_limits<int>::min() : index_ + 1;
                return downstream_->emit(IndexedValue<T>(internal::check_index_overflow(index), std::move(value)), completion);
            }

        private:
            const std::shared_ptr<Flow<T>> upstream_;
            FlowCollector<IndexedValue<T>>* const downstream_;
            int index_ = 0;
        };

        auto receiver = std::make_shared<WithIndexCollector>(upstream, collector);
        return receiver->collect(kotlinx::coroutines::internal::retain_continuation(completion));
    });
}

/**
 * Returns a flow that invokes the given action before each value of the upstream flow is emitted downstream.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:74-77
template <typename T, typename Action>
inline std::shared_ptr<Flow<T>> on_each(
    std::shared_ptr<Flow<T>> upstream,
    Action action_fn) {
    auto action = std::make_shared<Action>(std::move(action_fn));
    auto block = [action = std::move(action)](FlowCollector<T>* collector, T value,
        std::shared_ptr<Continuation<void*>> completion)
        __attribute__((annotate("suspend"))) -> void* {
        dsl::suspend(detail::invoke_action_fn(*action, completion.get(), value));
        dsl::suspend(collector->emit(std::move(value), completion.get()));
        return nullptr;
    };
    return internal::unsafe_transform<T, T>(std::move(upstream),
        [block = std::move(block)](FlowCollector<T>* collector, T value,
            Continuation<void*>* completion) -> void* {
            return block(collector, std::move(value), kotlinx::coroutines::internal::retain_continuation(completion));
        });
}

/**
 * Synchronous functional overload of on_each.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:74-77
template <typename T>
inline std::shared_ptr<Flow<T>> on_each(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void(const T&)> action) {
    return on_each<T, std::function<void(const T&)>>(std::move(upstream), std::move(action));
}

/**
 * Suspending functional overload of on_each.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:74-77
template <typename T>
inline std::shared_ptr<Flow<T>> on_each(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(const T&, Continuation<void*>*)> action) {
    return on_each<T, std::function<void*(const T&, Continuation<void*>*)>>(std::move(upstream), std::move(action));
}

// Declare the folding implementation used by scan.
template <typename T, typename R, typename Operation>
std::shared_ptr<Flow<R>> running_fold(std::shared_ptr<Flow<T>> upstream, R initial, Operation operation);

/**
 * Folds the given flow with operation, emitting every intermediate result, including initial value.
 * This function is an alias to running_fold. The initial value must remain immutable if
 * it shares mutable storage between collectors.
 *
 * ```cpp
 * auto append = [](std::vector<int> acc, int value) {
 *     acc.push_back(value);
 *     return acc;
 * };
 * auto values = scan<int, std::vector<int>>(
 *     as_flow(std::vector<int>{1, 2, 3}), {}, append);
 * // Collects {}, {1}, {1, 2}, {1, 2, 3}.
 * ```
 *
 * @tparam T The element type of the source flow.
 * @tparam R The accumulator and result element type.
 * @tparam Operation Binary callable `(const R&, T)` returning `R` synchronously or via Continuation ABI.
 * @param upstream The source flow.
 * @param initial The initial accumulator value.
 * @param operation The folding function applied sequentially to each element.
 * @return A flow emitting intermediate accumulated results.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:90
template <typename T, typename R, typename Operation>
inline std::shared_ptr<Flow<R>> scan(
    std::shared_ptr<Flow<T>> upstream,
    R initial,
    Operation operation) {
    return running_fold<T, R, Operation>(std::move(upstream), std::move(initial), std::move(operation));
}

/**
 * Synchronous functional overload of scan.
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> scan(
    std::shared_ptr<Flow<T>> upstream,
    R initial,
    std::function<R(R, T)> operation) {
    return scan<T, R, std::function<R(R, T)>>(std::move(upstream), std::move(initial), std::move(operation));
}

/**
 * Suspending functional overload of scan.
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> scan(
    std::shared_ptr<Flow<T>> upstream,
    R initial,
    std::function<void*(R, T, Continuation<void*>*)> operation) {
    return scan<T, R, std::function<void*(R, T, Continuation<void*>*)>>(std::move(upstream), std::move(initial), std::move(operation));
}

/**
 * Folds the given flow with operation, emitting every intermediate result, including initial value.
 *
 * @tparam T The element type of the source flow.
 * @tparam R The accumulator and result element type.
 * @tparam Operation Binary callable `(const R&, T)` returning `R` synchronously or via Continuation ABI.
 * @param upstream The source flow.
 * @param initial The initial accumulator value.
 * @param operation The folding function applied sequentially to each element.
 * @return A flow emitting the intermediate accumulated results.
 *
 * ```cpp
 * auto append = [](std::vector<int> accumulator, int value) {
 *     accumulator.push_back(value);
 *     return accumulator;
 * };
 * auto values = running_fold<int, std::vector<int>>(
 *     as_flow(std::vector<int>{1, 2, 3}), {}, append);
 * // Collects {}, {1}, {1, 2}, {1, 2, 3}.
 * ```
 *
 * A suspending operation accepts an additional `Continuation<void*>*` and returns
 * the suspension sentinel or an owned heap-allocated `R*`. On successful return
 * or resume, the frame consumes and deletes that result box before emitting.
 * A failed resume propagates without assigning a new accumulator.
 *
 * Initial Emission & Accumulator Lifecycle:
 * - Immediate initial emission: The initial accumulator value is emitted immediately upon collection start
 *   before any upstream elements are collected. If downstream emission of the initial value suspends, upstream
 *   collection does not start until the initial emission completes.
 * - Accumulator independence: Each collection maintains its own independent accumulator state copied from `initial`.
 *   However, if `initial` contains mutable shared pointers or references, mutations through those pointers are not
 *   isolated; callers must ensure shared pointees remain immutable.
 * - Empty flow: If the upstream flow produces no elements, exactly the initial value is emitted.
 *
 * Sequential Updates & Fault Tolerance:
 * - Sequential accumulation: For each upstream element, `operation(accumulator, value)` is evaluated while the
 *   existing accumulator remains unmodified in place.
 * - Successful state advancement: The accumulator is updated ONLY after the operation successfully produces a result.
 *   If the operation fails or is cancelled, the accumulator is never updated and no stale or partial emission occurs.
 * - Tail emission: Downstream emission of the updated accumulator is awaited via `coroutine_yield` before completing
 *   the emit frame with Unit (`nullptr`).
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:101-108
template <typename T, typename R, typename Operation>
inline std::shared_ptr<Flow<R>> running_fold(
    std::shared_ptr<Flow<T>> upstream,
    R initial,
    Operation operation) {
    return internal::unsafe_flow<R>([upstream = std::move(upstream),
                                      initial = std::move(initial),
                                      operation = std::move(operation)](
        FlowCollector<R>* collector, Continuation<void*>* completion) mutable -> void* {
        class RunningFoldFrame final : public ContinuationImpl, public FlowCollector<T> {
            class EmitFrame final : public ContinuationImpl {
            public:
                EmitFrame(RunningFoldFrame* owner, T value, Continuation<void*>* completion)
                    : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                      owner_(owner), value_(std::move(value)) {}

                void retain() { self_ref_ = shared_from_this(); }

                void* invoke_suspend(Result<void*> result) override {
                    coroutine_begin(this)
                    coroutine_yield_value(this, result,
                        detail::invoke_fold_op(owner_->operation_, this, owner_->accumulator_, value_), op_result_);
                    {
                        std::unique_ptr<R> box(static_cast<R*>(op_result_));
                        op_result_ = nullptr;
                        owner_->accumulator_ = std::move(*box);
                    }
                    coroutine_yield(this, owner_->downstream_->emit(owner_->accumulator_, this));
                    coroutine_end(this)
                }

            protected:
                void release_intercepted() override {
                    ContinuationImpl::release_intercepted();
                    self_ref_.reset();
                }

            private:
                void* _label = nullptr;
                RunningFoldFrame* owner_;
                T value_;
                void* op_result_ = nullptr;
                std::shared_ptr<BaseContinuationImpl> self_ref_;
            };

        public:
            RunningFoldFrame(std::shared_ptr<Flow<T>> upstream, FlowCollector<R>* downstream, R initial,
                             Operation operation, Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  upstream_(std::move(upstream)), downstream_(downstream),
                  accumulator_(std::move(initial)), operation_(std::move(operation)) {}

            void retain() { self_ref_ = shared_from_this(); }

            void* invoke_suspend(Result<void*> result) override {
                coroutine_begin(this)
                // 1. Emit initial value
                coroutine_yield(this, downstream_->emit(accumulator_, this));
                // 2. Collect upstream flow
                coroutine_yield(this, upstream_->collect(this, this));
                coroutine_end(this)
            }

            void* emit(T value, Continuation<void*>* cont) override {
                auto frame = std::make_shared<EmitFrame>(this, std::move(value), cont);
                frame->retain();
                return frame->start(Result<void*>::success(nullptr));
            }

        protected:
            void release_intercepted() override {
                ContinuationImpl::release_intercepted();
                self_ref_.reset();
            }

        private:
            void* _label = nullptr;
            std::shared_ptr<Flow<T>> upstream_;
            FlowCollector<R>* downstream_;
            R accumulator_;
            Operation operation_;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };

        auto frame = std::make_shared<RunningFoldFrame>(upstream, collector, initial, operation, completion);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

/**
 * Synchronous functional overload of running_fold.
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> running_fold(
    std::shared_ptr<Flow<T>> upstream,
    R initial,
    std::function<R(R, T)> operation) {
    return running_fold<T, R, std::function<R(R, T)>>(std::move(upstream), std::move(initial), std::move(operation));
}

/**
 * Suspending functional overload of running_fold.
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> running_fold(
    std::shared_ptr<Flow<T>> upstream,
    R initial,
    std::function<void*(R, T, Continuation<void*>*)> operation) {
    return running_fold<T, R, std::function<void*(R, T, Continuation<void*>*)>>(std::move(upstream), std::move(initial), std::move(operation));
}

/**
 * Reduces the given flow with operation, emitting every intermediate result, starting from the first element.
 *
 * The first element of the upstream flow is emitted immediately without applying the operation. Every subsequent
 * element is combined with the accumulated value using the operation, and the new result is emitted.
 * The sibling operator `scan` takes an explicit initial value.
 *
 * @tparam T The element type of the source flow.
 * @tparam Operation Binary callable `(const T&, T)` returning `T` synchronously or via Continuation ABI.
 * @param upstream The source flow.
 * @param operation The reducing function applied to elements.
 * @return A flow emitting intermediate reduced results.
 *
 * ```cpp
 * auto values = running_reduce<int>(as_flow(std::vector<int>{1, 2, 3, 4}),
 *     [](int accumulator, int value) { return accumulator + value; });
 * // Collects 1, 3, 6, 10.
 * ```
 *
 * A suspending operation accepts an additional `Continuation<void*>*` and returns
 * the suspension sentinel or an owned heap-allocated `T*`. On successful return
 * or resume, the frame consumes and deletes the result box before emitting.
 *
 * Initial State & Uninitialized Sentinel:
 * - Disengaged sentinel: Accumulator state is managed via `std::optional<T> accumulator_` (initially `std::nullopt`).
 *   No default constructor of `T` is ever called, safely supporting non-default-constructible types.
 * - Nullable elements: When `T` is a nullable type (such as `std::optional<U>`), `accumulator_.has_value()` distinguishes
 *   uninitialized state from an initialized null element (e.g. `std::nullopt` emitted as the first element).
 * - First element: The first element is captured directly via `accumulator_.emplace(std::move(value))` and emitted downstream.
 *   `operation` is NOT called on the first element.
 * - Empty flow: If the upstream flow is empty, `running_reduce` produces exactly 0 emissions.
 *
 * Sequential Updates & Fault Tolerance:
 * - State protection: During operation execution, `*accumulator_` is passed by const reference and remains intact.
 * - Successful assignment: Only upon successful operation return is `accumulator_` updated with the new result.
 *   Failure or cancellation during operation preserves state and aborts without emitting.
 * - Tail emission: Emission is awaited via `coroutine_yield` before completing with Unit (`nullptr`).
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:121-131
template <typename T, typename Operation>
inline std::shared_ptr<Flow<T>> running_reduce(
    std::shared_ptr<Flow<T>> upstream,
    Operation operation) {
    return internal::unsafe_flow<T>([upstream = std::move(upstream),
                                      operation = std::move(operation)](
        FlowCollector<T>* collector, Continuation<void*>* completion) mutable -> void* {
        class RunningReduceFrame final : public ContinuationImpl, public FlowCollector<T> {
            class EmitFrame final : public ContinuationImpl {
            public:
                EmitFrame(RunningReduceFrame* owner, T value, Continuation<void*>* completion)
                    : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                      owner_(owner), value_(std::move(value)) {}

                void retain() { self_ref_ = shared_from_this(); }

                void* invoke_suspend(Result<void*> result) override {
                    coroutine_begin(this)
                    if (!owner_->accumulator_.has_value()) {
                        owner_->accumulator_.emplace(std::move(value_));
                    } else {
                        coroutine_yield_value(this, result,
                            detail::invoke_reduce_op(owner_->operation_, this, *owner_->accumulator_, value_), op_result_);
                        {
                            std::unique_ptr<T> box(static_cast<T*>(op_result_));
                            op_result_ = nullptr;
                            owner_->accumulator_.emplace(std::move(*box));
                        }
                    }
                    coroutine_yield(this, owner_->downstream_->emit(*owner_->accumulator_, this));
                    coroutine_end(this)
                }

            protected:
                void release_intercepted() override {
                    ContinuationImpl::release_intercepted();
                    self_ref_.reset();
                }

            private:
                void* _label = nullptr;
                RunningReduceFrame* owner_;
                T value_;
                void* op_result_ = nullptr;
                std::shared_ptr<BaseContinuationImpl> self_ref_;
            };

        public:
            RunningReduceFrame(std::shared_ptr<Flow<T>> upstream, FlowCollector<T>* downstream,
                               Operation operation, Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  upstream_(std::move(upstream)), downstream_(downstream),
                  operation_(std::move(operation)), accumulator_(std::nullopt) {}

            void retain() { self_ref_ = shared_from_this(); }

            void* invoke_suspend(Result<void*> result) override {
                coroutine_begin(this)
                coroutine_yield(this, upstream_->collect(this, this));
                coroutine_end(this)
            }

            void* emit(T value, Continuation<void*>* cont) override {
                auto frame = std::make_shared<EmitFrame>(this, std::move(value), cont);
                frame->retain();
                return frame->start(Result<void*>::success(nullptr));
            }

        protected:
            void release_intercepted() override {
                ContinuationImpl::release_intercepted();
                self_ref_.reset();
            }

        private:
            void* _label = nullptr;
            std::shared_ptr<Flow<T>> upstream_;
            FlowCollector<T>* downstream_;
            Operation operation_;
            std::optional<T> accumulator_ = std::nullopt;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };

        auto frame = std::make_shared<RunningReduceFrame>(upstream, collector, operation, completion);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

/**
 * Synchronous functional overload of running_reduce.
 */
template <typename T>
inline std::shared_ptr<Flow<T>> running_reduce(
    std::shared_ptr<Flow<T>> upstream,
    std::function<T(T, T)> operation) {
    return running_reduce<T, std::function<T(T, T)>>(std::move(upstream), std::move(operation));
}

/**
 * Suspending functional overload of running_reduce.
 */
template <typename T>
inline std::shared_ptr<Flow<T>> running_reduce(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(T, T, Continuation<void*>*)> operation) {
    return running_reduce<T, std::function<void*(T, T, Continuation<void*>*)>>(std::move(upstream), std::move(operation));
}

/**
 * Splits the given flow into a flow of non-overlapping lists (std::vector<T>), each not exceeding the given size,
 * but never empty.
 *
 * @tparam T The element type of the source flow.
 * @param upstream The source flow.
 * @param size The maximum number of elements per chunk. Must be strictly positive (size >= 1).
 * @return A flow emitting `std::vector<T>` chunks.
 * @throws std::invalid_argument Immediately upon invocation if `size < 1`.
 *
 * The final chunk may have fewer elements than `size`.
 *
 * ```cpp
 * auto chunks = chunked(as_flow(std::vector<std::string>{"a", "b", "c", "d", "e"}), 2);
 * auto values = map<std::vector<std::string>, std::string>(chunks,
 *     [](std::vector<std::string> chunk) {
 *         std::string joined;
 *         for (const auto& value : chunk) joined += value;
 *         return joined;
 *     });
 * struct Printer final : FlowCollector<std::string> {
 *     void* emit(std::string value, Continuation<void*>*) override {
 *         std::cout << value << '\n';
 *         return nullptr;
 *     }
 * } printer;
 * values->collect(&printer, nullptr); // Prints "ab", "cd", "e" on separate lines.
 * ```
 *
 * The printing example requires `<iostream>`. Collection completes synchronously;
 * the local printer remains alive until it returns.
 *
 * Batching & Buffer Lifetime:
 * - Lazy allocation: The chunk buffer `std::optional<std::vector<T>> result_` is allocated on demand on the first element.
 * - Buffer emission by value: When a chunk is emitted downstream (either a full batch or terminal partial batch),
 *   the buffer is passed by value to `FlowCollector::emit`. The owned source buffer in `ChunkedFrame` remains intact
 *   while the downstream emission is pending and is reset only after the emission settles.
 * - Terminal partial chunk: When upstream collection completes normally, any non-empty trailing partial chunk is
 *   emitted downstream before terminating.
 * - Failure / cancellation contract: If upstream collection terminates exceptionally or is cancelled, the partial
 *   chunk is NOT flushed downstream; buffered elements are cleanly destroyed.
 * - Empty flow: If the upstream flow produces no elements, exactly 0 chunks are emitted.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:150-166
template <typename T>
inline std::shared_ptr<Flow<std::vector<T>>> chunked(
    std::shared_ptr<Flow<T>> upstream,
    int size) {
    if (size < 1) {
        throw std::invalid_argument("Expected positive chunk size, but got " + std::to_string(size));
    }
    return internal::unsafe_flow<std::vector<T>>([upstream = std::move(upstream), size](
        FlowCollector<std::vector<T>>* collector, Continuation<void*>* completion) mutable -> void* {
        class ChunkedFrame final : public ContinuationImpl, public FlowCollector<T> {
            class EmitFrame final : public ContinuationImpl {
            public:
                EmitFrame(ChunkedFrame* owner, T value, Continuation<void*>* completion)
                    : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                      owner_(owner), value_(std::move(value)) {}

                void retain() { self_ref_ = shared_from_this(); }

                void* invoke_suspend(Result<void*> result) override {
                    coroutine_begin(this)
                    // Allocate if needed.
                    if (!owner_->result_.has_value()) {
                        owner_->result_.emplace();
                        owner_->result_->reserve(owner_->size_);
                    }
                    owner_->result_->push_back(std::move(value_));
                    if (static_cast<int>(owner_->result_->size()) == owner_->size_) {
                        coroutine_yield(this, owner_->downstream_->emit(*owner_->result_, this));
                        // Cleanup, but don't allocate: this may be the last element.
                        owner_->result_.reset();
                    }
                    coroutine_end(this)
                }

            protected:
                void release_intercepted() override {
                    ContinuationImpl::release_intercepted();
                    self_ref_.reset();
                }

            private:
                void* _label = nullptr;
                ChunkedFrame* owner_;
                T value_;
                std::shared_ptr<BaseContinuationImpl> self_ref_;
            };

        public:
            ChunkedFrame(std::shared_ptr<Flow<T>> upstream, FlowCollector<std::vector<T>>* downstream, int size,
                         Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  upstream_(std::move(upstream)), downstream_(downstream), size_(size) {}

            void retain() { self_ref_ = shared_from_this(); }

            void* invoke_suspend(Result<void*> result) override {
                coroutine_begin(this)
                coroutine_yield(this, upstream_->collect(this, this));
                if (result_.has_value()) {
                    coroutine_yield(this, downstream_->emit(*result_, this));
                    result_.reset();
                }
                coroutine_end(this)
            }

            void* emit(T value, Continuation<void*>* cont) override {
                auto frame = std::make_shared<EmitFrame>(this, std::move(value), cont);
                frame->retain();
                return frame->start(Result<void*>::success(nullptr));
            }

        protected:
            void release_intercepted() override {
                ContinuationImpl::release_intercepted();
                self_ref_.reset();
            }

        private:
            void* _label = nullptr;
            std::shared_ptr<Flow<T>> upstream_;
            FlowCollector<std::vector<T>>* downstream_;
            int size_;
            std::optional<std::vector<T>> result_ = std::nullopt; // Do not preallocate anything.
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };

        auto frame = std::make_shared<ChunkedFrame>(upstream, collector, size, completion);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

} // namespace kotlinx::coroutines::flow
