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
template<typename Op, typename T>
void* invoke_reduce_op(Op& op, Continuation<void*>* continuation, const T& acc, T&& val) {
    if constexpr (std::is_invocable_v<Op&, T, T, Continuation<void*>*>) {
        return op(acc, std::forward<T>(val), continuation);
    } else if constexpr (std::is_invocable_v<Op&, const T&, T, Continuation<void*>*>) {
        return op(acc, std::forward<T>(val), continuation);
    } else {
        using Ret = std::invoke_result_t<Op&, const T&, T>;
        return new Ret(op(acc, std::forward<T>(val)));
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
        FlowCollector<R>* collector, Continuation<void*>* completion) -> void* {
        class CollectFrame final : public ContinuationImpl, public FlowCollector<T> {
        public:
            CollectFrame(std::shared_ptr<Flow<T>> upstream, FlowCollector<R>* downstream, TransformBlock transform,
                         Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  upstream_(std::move(upstream)), downstream_(downstream), transform_(std::move(transform)) {}

            void retain() { self_ref_ = shared_from_this(); }

            void* invoke_suspend(Result<void*> result) override {
                coroutine_begin(this)
                coroutine_yield(this, upstream_->collect(this, this));
                coroutine_end(this)
            }

            void* emit(T value, Continuation<void*>* cont) override {
                return transform_(downstream_, std::move(value), cont);
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
            TransformBlock transform_;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };

        auto frame = std::make_shared<CollectFrame>(upstream, collector, transform, completion);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

} // namespace internal

/**
 * Returns a flow containing only values of the original flow that match the given predicate.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:17-19
template <typename T, typename Predicate>
inline std::shared_ptr<Flow<T>> filter(std::shared_ptr<Flow<T>> upstream, Predicate predicate) {
    return internal::unsafe_transform<T, T>(
        std::move(upstream),
        [predicate = std::move(predicate)](
            FlowCollector<T>* collector, T value, Continuation<void*>* cont) -> void* {
            class FilterFrame final : public ContinuationImpl {
            public:
                FilterFrame(FlowCollector<T>* collector, Predicate& predicate, T value, Continuation<void*>* completion)
                    : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                      collector_(collector), predicate_(predicate), value_(std::move(value)) {}

                void retain() { self_ref_ = shared_from_this(); }

                void* invoke_suspend(Result<void*> result) override {
                    coroutine_begin(this)
                    coroutine_yield_value(this, result,
                        detail::invoke_transform_predicate(predicate_, this, value_), predicate_result_);
                    {
                        std::unique_ptr<bool> box(static_cast<bool*>(predicate_result_));
                        bool matches = *box;
                        if (matches) {
                            coroutine_yield(this, collector_->emit(std::move(value_), this));
                        }
                        return nullptr;
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
                FlowCollector<T>* collector_;
                Predicate& predicate_;
                T value_;
                void* predicate_result_ = nullptr;
                std::shared_ptr<BaseContinuationImpl> self_ref_;
            };

            auto frame = std::make_shared<FilterFrame>(collector, predicate, std::move(value), cont);
            frame->retain();
            return frame->start(Result<void*>::success(nullptr));
        });
}

/**
 * Synchronous functional overload of filter.
 */
template <typename T>
inline std::shared_ptr<Flow<T>> filter(
    std::shared_ptr<Flow<T>> upstream,
    std::function<bool(const T&)> predicate) {
    return filter<T, std::function<bool(const T&)>>(std::move(upstream), std::move(predicate));
}

/**
 * Suspending functional overload of filter.
 */
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
inline std::shared_ptr<Flow<T>> filter_not(std::shared_ptr<Flow<T>> upstream, Predicate predicate) {
    return internal::unsafe_transform<T, T>(
        std::move(upstream),
        [predicate = std::move(predicate)](
            FlowCollector<T>* collector, T value, Continuation<void*>* cont) -> void* {
            class FilterNotFrame final : public ContinuationImpl {
            public:
                FilterNotFrame(FlowCollector<T>* collector, Predicate& predicate, T value, Continuation<void*>* completion)
                    : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                      collector_(collector), predicate_(predicate), value_(std::move(value)) {}

                void retain() { self_ref_ = shared_from_this(); }

                void* invoke_suspend(Result<void*> result) override {
                    coroutine_begin(this)
                    coroutine_yield_value(this, result,
                        detail::invoke_transform_predicate(predicate_, this, value_), predicate_result_);
                    {
                        std::unique_ptr<bool> box(static_cast<bool*>(predicate_result_));
                        bool matches = *box;
                        if (!matches) {
                            coroutine_yield(this, collector_->emit(std::move(value_), this));
                        }
                        return nullptr;
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
                FlowCollector<T>* collector_;
                Predicate& predicate_;
                T value_;
                void* predicate_result_ = nullptr;
                std::shared_ptr<BaseContinuationImpl> self_ref_;
            };

            auto frame = std::make_shared<FilterNotFrame>(collector, predicate, std::move(value), cont);
            frame->retain();
            return frame->start(Result<void*>::success(nullptr));
        });
}

/**
 * Synchronous functional overload of filter_not.
 */
template <typename T>
inline std::shared_ptr<Flow<T>> filter_not(
    std::shared_ptr<Flow<T>> upstream,
    std::function<bool(const T&)> predicate) {
    return filter_not<T, std::function<bool(const T&)>>(std::move(upstream), std::move(predicate));
}

/**
 * Suspending functional overload of filter_not.
 */
template <typename T>
inline std::shared_ptr<Flow<T>> filter_not(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(const T&, Continuation<void*>*)> predicate) {
    return filter_not<T, std::function<void*(const T&, Continuation<void*>*)>>(std::move(upstream), std::move(predicate));
}

/**
 * Returns a flow containing only values that are instances of specified type R (shared_ptr representation).
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
 * Returns a flow containing the results of applying the given transform function to each value of the original flow.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:49-51
template <typename T, typename R, typename Transform>
inline std::shared_ptr<Flow<R>> map(std::shared_ptr<Flow<T>> upstream, Transform transform_fn) {
    return internal::unsafe_transform<T, R>(
        std::move(upstream),
        [transform_fn = std::move(transform_fn)](
            FlowCollector<R>* collector, T value, Continuation<void*>* cont) -> void* {
            class MapFrame final : public ContinuationImpl {
            public:
                MapFrame(FlowCollector<R>* collector, Transform& transform_fn, T value, Continuation<void*>* completion)
                    : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                      collector_(collector), transform_fn_(transform_fn), value_(std::move(value)) {}

                void retain() { self_ref_ = shared_from_this(); }

                void* invoke_suspend(Result<void*> result) override {
                    coroutine_begin(this)
                    coroutine_yield_value(this, result,
                        detail::invoke_transform_fn(transform_fn_, this, std::move(value_)), transform_result_);
                    {
                        std::unique_ptr<R> box(static_cast<R*>(transform_result_));
                        R transformed = std::move(*box);
                        coroutine_yield(this, collector_->emit(std::move(transformed), this));
                        return nullptr;
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
                FlowCollector<R>* collector_;
                Transform& transform_fn_;
                T value_;
                void* transform_result_ = nullptr;
                std::shared_ptr<BaseContinuationImpl> self_ref_;
            };

            auto frame = std::make_shared<MapFrame>(collector, transform_fn, std::move(value), cont);
            frame->retain();
            return frame->start(Result<void*>::success(nullptr));
        });
}

/**
 * Synchronous functional overload of map.
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> map(
    std::shared_ptr<Flow<T>> upstream,
    std::function<R(T)> transform_fn) {
    return map<T, R, std::function<R(T)>>(std::move(upstream), std::move(transform_fn));
}

/**
 * Suspending functional overload of map.
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> map(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(T, Continuation<void*>*)> transform_fn) {
    return map<T, R, std::function<void*(T, Continuation<void*>*)>>(std::move(upstream), std::move(transform_fn));
}

/**
 * Returns a flow that contains only non-null results of applying the given transform function (std::optional overload).
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:56-59
template <typename T, typename R, typename Transform>
inline std::shared_ptr<Flow<R>> map_not_null(
    std::shared_ptr<Flow<T>> upstream,
    Transform transform_fn) {
    return internal::unsafe_transform<T, R>(
        std::move(upstream),
        [transform_fn = std::move(transform_fn)](
            FlowCollector<R>* collector, T value, Continuation<void*>* cont) -> void* {
            class MapNotNullFrame final : public ContinuationImpl {
            public:
                MapNotNullFrame(FlowCollector<R>* collector, Transform& transform_fn, T value, Continuation<void*>* completion)
                    : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                      collector_(collector), transform_fn_(transform_fn), value_(std::move(value)) {}

                void retain() { self_ref_ = shared_from_this(); }

                void* invoke_suspend(Result<void*> result) override {
                    coroutine_begin(this)
                    coroutine_yield_value(this, result,
                        detail::invoke_transform_fn(transform_fn_, this, std::move(value_)), transform_result_);
                    {
                        std::unique_ptr<std::optional<R>> box(static_cast<std::optional<R>*>(transform_result_));
                        if (box && box->has_value()) {
                            R transformed = std::move(**box);
                            coroutine_yield(this, collector_->emit(std::move(transformed), this));
                        }
                        return nullptr;
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
                FlowCollector<R>* collector_;
                Transform& transform_fn_;
                T value_;
                void* transform_result_ = nullptr;
                std::shared_ptr<BaseContinuationImpl> self_ref_;
            };

            auto frame = std::make_shared<MapNotNullFrame>(collector, transform_fn, std::move(value), cont);
            frame->retain();
            return frame->start(Result<void*>::success(nullptr));
        });
}

/**
 * Synchronous std::optional overload of map_not_null.
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> map_not_null(
    std::shared_ptr<Flow<T>> upstream,
    std::function<std::optional<R>(T)> transform_fn) {
    return map_not_null<T, R, std::function<std::optional<R>(T)>>(std::move(upstream), std::move(transform_fn));
}

/**
 * Suspending std::optional overload of map_not_null.
 */
template <typename T, typename R>
inline std::shared_ptr<Flow<R>> map_not_null(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(T, Continuation<void*>*)> transform_fn) {
    return map_not_null<T, R, std::function<void*(T, Continuation<void*>*)>>(std::move(upstream), std::move(transform_fn));
}

/**
 * Synchronous raw pointer overload of map_not_null.
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
        class WithIndexFrame final : public ContinuationImpl, public FlowCollector<T> {
        public:
            WithIndexFrame(std::shared_ptr<Flow<T>> upstream, FlowCollector<IndexedValue<T>>* downstream,
                           Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  upstream_(std::move(upstream)), downstream_(downstream), index_(0) {}

            void retain() { self_ref_ = shared_from_this(); }

            void* invoke_suspend(Result<void*> result) override {
                coroutine_begin(this)
                coroutine_yield(this, upstream_->collect(this, this));
                coroutine_end(this)
            }

            void* emit(T value, Continuation<void*>* cont) override {
                int curr = index_;
                index_ = (index_ == std::numeric_limits<int>::max())
                       ? std::numeric_limits<int>::min()
                       : index_ + 1;
                int checked_idx = internal::check_index_overflow(curr);
                return downstream_->emit(IndexedValue<T>(checked_idx, std::move(value)), cont);
            }

        protected:
            void release_intercepted() override {
                ContinuationImpl::release_intercepted();
                self_ref_.reset();
            }

        private:
            void* _label = nullptr;
            std::shared_ptr<Flow<T>> upstream_;
            FlowCollector<IndexedValue<T>>* downstream_;
            int index_ = 0;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };

        auto frame = std::make_shared<WithIndexFrame>(upstream, collector, completion);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

/**
 * Returns a flow that invokes the given action before each value of the upstream flow is emitted downstream.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:74-77
template <typename T, typename Action>
inline std::shared_ptr<Flow<T>> on_each(
    std::shared_ptr<Flow<T>> upstream,
    Action action) {
    return internal::unsafe_transform<T, T>(
        std::move(upstream),
        [action = std::move(action)](
            FlowCollector<T>* collector, T value, Continuation<void*>* cont) -> void* {
            class OnEachFrame final : public ContinuationImpl {
            public:
                OnEachFrame(FlowCollector<T>* collector, Action& action, T value, Continuation<void*>* completion)
                    : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                      collector_(collector), action_(action), value_(std::move(value)) {}

                void retain() { self_ref_ = shared_from_this(); }

                void* invoke_suspend(Result<void*> result) override {
                    coroutine_begin(this)
                    coroutine_yield(this, detail::invoke_action_fn(action_, this, value_));
                    coroutine_yield(this, collector_->emit(std::move(value_), this));
                    return nullptr;
                    coroutine_end(this)
                }

            protected:
                void release_intercepted() override {
                    ContinuationImpl::release_intercepted();
                    self_ref_.reset();
                }

            private:
                void* _label = nullptr;
                FlowCollector<T>* collector_;
                Action& action_;
                T value_;
                std::shared_ptr<BaseContinuationImpl> self_ref_;
            };

            auto frame = std::make_shared<OnEachFrame>(collector, action, std::move(value), cont);
            frame->retain();
            return frame->start(Result<void*>::success(nullptr));
        });
}

/**
 * Synchronous functional overload of on_each.
 */
template <typename T>
inline std::shared_ptr<Flow<T>> on_each(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void(const T&)> action) {
    return on_each<T, std::function<void(const T&)>>(std::move(upstream), std::move(action));
}

/**
 * Suspending functional overload of on_each.
 */
template <typename T>
inline std::shared_ptr<Flow<T>> on_each(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(const T&, Continuation<void*>*)> action) {
    return on_each<T, std::function<void*(const T&, Continuation<void*>*)>>(std::move(upstream), std::move(action));
}

// Forward declaration of running_fold so scan can precede it in exact Kotlin declaration order.
template <typename T, typename R, typename Operation>
std::shared_ptr<Flow<R>> running_fold(std::shared_ptr<Flow<T>> upstream, R initial, Operation operation);

/**
 * Folds the given flow with operation, emitting every intermediate result, including initial value.
 * This function is an alias to running_fold.
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
        FlowCollector<R>* collector, Continuation<void*>* completion) -> void* {
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
                        owner_->accumulator_ = std::move(*box);
                    }
                    coroutine_yield(this, owner_->downstream_->emit(owner_->accumulator_, this));
                    return nullptr;
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
                return nullptr;
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
 * Reduces the given flow with operation, emitting every intermediate result, including initial value.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Transform.kt:121-131
template <typename T, typename Operation>
inline std::shared_ptr<Flow<T>> running_reduce(
    std::shared_ptr<Flow<T>> upstream,
    Operation operation) {
    return internal::unsafe_flow<T>([upstream = std::move(upstream),
                                      operation = std::move(operation)](
        FlowCollector<T>* collector, Continuation<void*>* completion) -> void* {
        class RunningReduceFrame final : public ContinuationImpl, public FlowCollector<T> {
            class EmitFrame final : public ContinuationImpl {
            public:
                EmitFrame(RunningReduceFrame* owner, T value, Continuation<void*>* completion)
                    : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                      owner_(owner), value_(std::move(value)) {}

                void retain() { self_ref_ = shared_from_this(); }

                void* invoke_suspend(Result<void*> result) override {
                    coroutine_begin(this)
                    if (!owner_->has_accumulator_) {
                        owner_->accumulator_ = std::move(value_);
                        owner_->has_accumulator_ = true;
                    } else {
                        coroutine_yield_value(this, result,
                            detail::invoke_reduce_op(owner_->operation_, this, owner_->accumulator_, value_), op_result_);
                        {
                            std::unique_ptr<T> box(static_cast<T*>(op_result_));
                            owner_->accumulator_ = std::move(*box);
                        }
                    }
                    coroutine_yield(this, owner_->downstream_->emit(owner_->accumulator_, this));
                    return nullptr;
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
                  operation_(std::move(operation)), has_accumulator_(false) {}

            void retain() { self_ref_ = shared_from_this(); }

            void* invoke_suspend(Result<void*> result) override {
                coroutine_begin(this)
                coroutine_yield(this, upstream_->collect(this, this));
                return nullptr;
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
            bool has_accumulator_ = false;
            T accumulator_{};
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
 * Splits the given flow into a flow of non-overlapping lists each not exceeding the given size but never empty.
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
        class ChunkedFrame final : public ContinuationImpl, public FlowCollector<T> {
            class EmitFrame final : public ContinuationImpl {
            public:
                EmitFrame(ChunkedFrame* owner, T value, Continuation<void*>* completion)
                    : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                      owner_(owner), value_(std::move(value)) {}

                void retain() { self_ref_ = shared_from_this(); }

                void* invoke_suspend(Result<void*> result) override {
                    coroutine_begin(this)
                    if (!owner_->result_.has_value()) {
                        owner_->result_.emplace();
                        owner_->result_->reserve(owner_->size_);
                    }
                    owner_->result_->push_back(std::move(value_));
                    if (static_cast<int>(owner_->result_->size()) == owner_->size_) {
                        coroutine_yield(this, owner_->downstream_->emit(*owner_->result_, this));
                        owner_->result_.reset();
                    }
                    return nullptr;
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
                    coroutine_yield(this, downstream_->emit(std::move(*result_), this));
                    result_.reset();
                }
                return nullptr;
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
            std::optional<std::vector<T>> result_ = std::nullopt;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };

        auto frame = std::make_shared<ChunkedFrame>(upstream, collector, size, completion);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

} // namespace kotlinx::coroutines::flow
