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
 * The initial value should be immutable, as it is shared between different collectors.
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
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:101-108
template <typename T, typename R, typename Operation>
inline std::shared_ptr<Flow<R>> running_fold(
    std::shared_ptr<Flow<T>> upstream,
    R initial,
    Operation operation_fn) {
    auto operation = std::make_shared<Operation>(std::move(operation_fn));
    return internal::unsafe_flow<R>([upstream = std::move(upstream),
        initial = std::move(initial), operation = std::move(operation)](
        FlowCollector<R>* collector, Continuation<void*>* completion) -> void* {
        // NOTE(port): This typed source collector captures the collection's accumulator.
        // Source operations are lowered by the compiler, without handwritten frames.
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:102-107
        class RunningFoldCollector final : public FlowCollector<T>,
            public std::enable_shared_from_this<RunningFoldCollector> {
        public:
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:102-107
            RunningFoldCollector(std::shared_ptr<Flow<T>> upstream, FlowCollector<R>* downstream,
                R initial, std::shared_ptr<Operation> operation)
                : upstream_(std::move(upstream)), downstream_(downstream),
                  accumulator_(std::move(initial)), operation_(std::move(operation)) {}

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:103-107
            [[clang::annotate("suspend")]]
            void* collect(std::shared_ptr<Continuation<void*>> completion) {
                auto owner = this->shared_from_this();
                dsl::suspend(downstream_->emit(accumulator_, completion.get()));
                dsl::suspend(upstream_->collect(this, completion.get()));
                return nullptr;
            }

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:104-107
            void* emit(T value, Continuation<void*>* completion) override {
                return emit(std::move(value), kotlinx::coroutines::internal::retain_continuation(completion));
            }

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:105-106
            [[clang::annotate("suspend")]]
            void* emit(T value, std::shared_ptr<Continuation<void*>> completion) {
                auto owner = this->shared_from_this();
                void* raw = dsl::suspend(detail::invoke_fold_op(*operation_, completion.get(), accumulator_, std::move(value)));
                // NOTE(port): The receiving side owns and deletes the erased R box.
                std::unique_ptr<R> box(static_cast<R*>(raw));
                accumulator_ = std::move(*box);
                box.reset();
                dsl::suspend(downstream_->emit(accumulator_, completion.get()));
                return nullptr;
            }

        private:
            const std::shared_ptr<Flow<T>> upstream_;
            FlowCollector<R>* const downstream_;
            R accumulator_;
            const std::shared_ptr<Operation> operation_;
        };

        auto receiver = std::make_shared<RunningFoldCollector>(upstream, collector, initial, operation);
        return receiver->collect(kotlinx::coroutines::internal::retain_continuation(completion));
    });
}

/**
 * Synchronous functional overload of running_fold.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:101-108
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
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:101-108
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> running_fold(
    std::shared_ptr<Flow<T>> upstream,
    R initial,
    std::function<void*(R, T, Continuation<void*>*)> operation) {
    return running_fold<T, R, std::function<void*(R, T, Continuation<void*>*)>>(std::move(upstream), std::move(initial), std::move(operation));
}

/**
 * Reduces the given flow with operation, emitting every intermediate result, including the initial value.
 * The first element supplies the initial accumulator. The sibling operator scan takes an explicit initial value.
 *
 * ```cpp
 * auto values = running_reduce<int>(as_flow(std::vector<int>{1, 2, 3, 4}),
 *     [](int accumulator, int value) { return accumulator + value; });
 * // Collects 1, 3, 6, 10.
 * ```
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:121-131
template <typename T, typename Operation>
inline std::shared_ptr<Flow<T>> running_reduce(
    std::shared_ptr<Flow<T>> upstream,
    Operation operation_fn) {
    auto operation = std::make_shared<Operation>(std::move(operation_fn));
    return internal::unsafe_flow<T>([upstream = std::move(upstream), operation = std::move(operation)](
        FlowCollector<T>* collector, Continuation<void*>* completion) -> void* {
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:122-130
        class RunningReduceCollector final : public FlowCollector<T>,
            public std::enable_shared_from_this<RunningReduceCollector> {
        public:
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:122-130
            RunningReduceCollector(std::shared_ptr<Flow<T>> upstream, FlowCollector<T>* downstream,
                std::shared_ptr<Operation> operation)
                : upstream_(std::move(upstream)), downstream_(downstream), operation_(std::move(operation)) {}

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:123-130
            [[clang::annotate("suspend")]]
            void* collect(std::shared_ptr<Continuation<void*>> completion) {
                auto owner = this->shared_from_this();
                dsl::suspend(upstream_->collect(this, completion.get()));
                return nullptr;
            }

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:123-130
            void* emit(T value, Continuation<void*>* completion) override {
                return emit(std::move(value), kotlinx::coroutines::internal::retain_continuation(completion));
            }

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:124-129
            [[clang::annotate("suspend")]]
            void* emit(T value, std::shared_ptr<Continuation<void*>> completion) {
                auto owner = this->shared_from_this();
                if (!accumulator_.has_value()) {
                    accumulator_.emplace(std::move(value));
                } else {
                    void* raw = dsl::suspend(detail::invoke_reduce_op(*operation_, completion.get(), *accumulator_, std::move(value)));
                    // NOTE(port): The receiving side owns and deletes the erased T box.
                    std::unique_ptr<T> box(static_cast<T*>(raw));
                    accumulator_.emplace(std::move(*box));
                    box.reset();
                }
                dsl::suspend(downstream_->emit(*accumulator_, completion.get()));
                return nullptr;
            }

        private:
            const std::shared_ptr<Flow<T>> upstream_;
            FlowCollector<T>* const downstream_;
            const std::shared_ptr<Operation> operation_;
            // NOTE(port): Disengagement represents the source NULL sentinel,
            // independently of any nullable value stored inside T.
            std::optional<T> accumulator_;
        };

        auto receiver = std::make_shared<RunningReduceCollector>(upstream, collector, operation);
        return receiver->collect(kotlinx::coroutines::internal::retain_continuation(completion));
    });
}

/**
 * Synchronous functional overload of running_reduce.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:121-131
template <typename T>
inline std::shared_ptr<Flow<T>> running_reduce(
    std::shared_ptr<Flow<T>> upstream,
    std::function<T(T, T)> operation) {
    return running_reduce<T, std::function<T(T, T)>>(std::move(upstream), std::move(operation));
}

/**
 * Suspending functional overload of running_reduce.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:121-131
template <typename T>
inline std::shared_ptr<Flow<T>> running_reduce(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(T, T, Continuation<void*>*)> operation) {
    return running_reduce<T, std::function<void*(T, T, Continuation<void*>*)>>(std::move(upstream), std::move(operation));
}

/**
 * Splits the given flow into non-overlapping lists, each not exceeding size and never empty.
 * The final list may have fewer elements than size.
 *
 * ```cpp
 * auto chunks = chunked(as_flow(std::vector<std::string>{"a", "b", "c", "d", "e"}), 2);
 * // Collects {"a", "b"}, {"c", "d"}, {"e"}.
 * ```
 *
 * Throws std::invalid_argument when size is not positive.
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
        FlowCollector<std::vector<T>>* collector, Continuation<void*>* completion) -> void* {
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:153-165
        class ChunkedCollector final : public FlowCollector<T>,
            public std::enable_shared_from_this<ChunkedCollector> {
        public:
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:153-165
            ChunkedCollector(std::shared_ptr<Flow<T>> upstream, FlowCollector<std::vector<T>>* downstream, int size)
                : upstream_(std::move(upstream)), downstream_(downstream), size_(size) {}

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:154-165
            [[clang::annotate("suspend")]]
            void* collect(std::shared_ptr<Continuation<void*>> completion) {
                auto owner = this->shared_from_this();
                dsl::suspend(upstream_->collect(this, completion.get()));
                if (result_.has_value()) dsl::suspend(downstream_->emit(*result_, completion.get()));
                return nullptr;
            }

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:154-163
            void* emit(T value, Continuation<void*>* completion) override {
                return emit(std::move(value), kotlinx::coroutines::internal::retain_continuation(completion));
            }

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:155-162
            [[clang::annotate("suspend")]]
            void* emit(T value, std::shared_ptr<Continuation<void*>> completion) {
                auto owner = this->shared_from_this();
                // Allocate if needed.
                if (!result_.has_value()) {
                    result_.emplace();
                    result_->reserve(size_);
                }
                auto& acc = *result_;
                acc.push_back(std::move(value));
                if (static_cast<int>(acc.size()) == size_) {
                    dsl::suspend(downstream_->emit(acc, completion.get()));
                    // Cleanup, but don't allocate: this may be the last element.
                    result_.reset();
                }
                return nullptr;
            }

        private:
            const std::shared_ptr<Flow<T>> upstream_;
            FlowCollector<std::vector<T>>* const downstream_;
            const int size_;
            std::optional<std::vector<T>> result_; // Do not preallocate anything.
        };

        auto receiver = std::make_shared<ChunkedCollector>(upstream, collector, size);
        return receiver->collect(kotlinx::coroutines::internal::retain_continuation(completion));
    });
}

} // namespace kotlinx::coroutines::flow
