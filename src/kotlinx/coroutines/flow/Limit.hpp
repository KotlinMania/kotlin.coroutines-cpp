// port-lint: source kotlinx-coroutines-core/common/src/flow/operators/Limit.kt
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt
#pragma once
/** @file Limit.hpp Flow operators that discard initial elements or retain a prefix. */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace kotlinx::coroutines::flow {
namespace detail {

// NOTE(port): Adapt ordinary/Continuation C++ callables to the source Boolean
// result. The receiving body owns and deletes the erased result box.
template<typename Predicate, typename... Args>
void* invoke_limit_predicate(Predicate& predicate, Continuation<void*>* continuation, Args&&... args) {
    if constexpr (std::is_invocable_v<Predicate&, Args..., Continuation<void*>*>) {
        static_assert(std::is_same_v<std::invoke_result_t<Predicate&, Args..., Continuation<void*>*>, void*>);
        return predicate(std::forward<Args>(args)..., continuation);
    } else if constexpr (std::is_invocable_v<Predicate&, Args..., std::shared_ptr<Continuation<void*>>>) {
        static_assert(std::is_same_v<std::invoke_result_t<Predicate&, Args..., std::shared_ptr<Continuation<void*>>>, void*>);
        return predicate(std::forward<Args>(args)..., kotlinx::coroutines::internal::retain_continuation(continuation));
    } else {
        return new bool(predicate(std::forward<Args>(args)...));
    }
}
} // namespace detail

// NOTE(port): The source-private generic extension needs a header definition
// for arbitrary public element types, in the source flow namespace.
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:70-73
template<typename T>
[[clang::annotate("suspend")]]
void* emit_abort(FlowCollector<T>* collector, T value, void* ownership_marker,
    std::shared_ptr<Continuation<void*>> completion) {
    dsl::suspend(collector->emit(std::move(value), completion.get()));
    throw internal::AbortFlowException(ownership_marker);
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:70-73
template<typename T>
void* emit_abort(FlowCollector<T>* collector, T value, void* ownership_marker, Continuation<void*>* completion) {
    return emit_abort(collector, std::move(value), ownership_marker,
        kotlinx::coroutines::internal::retain_continuation(completion));
}

/**
 * Returns a flow that ignores the first count elements.
 * Throws std::invalid_argument if count is negative.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:17-26
template<typename T>
std::shared_ptr<Flow<T>> drop(std::shared_ptr<Flow<T>> upstream, int count) {
    if (count < 0) throw std::invalid_argument("Drop count should be non-negative, but had " + std::to_string(count));
    return internal::unsafe_flow<T>([upstream = std::move(upstream), count](
        FlowCollector<T>* collector, Continuation<void*>* completion) -> void* {
        // NOTE(port): Typed collector binding for the source skipped capture.
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:19-25
        class DropCollector final : public FlowCollector<T>, public std::enable_shared_from_this<DropCollector> {
        public:
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:19-25
            DropCollector(std::shared_ptr<Flow<T>> upstream, FlowCollector<T>* collector, int count)
                : upstream_(std::move(upstream)), collector_(collector), count_(count) {}
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:21-24
            [[clang::annotate("suspend")]]
            void* collect(std::shared_ptr<Continuation<void*>> completion) {
                auto owner = this->shared_from_this();
                dsl::suspend(upstream_->collect(this, completion.get()));
                return nullptr;
            }
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:22-23
            void* emit(T value, Continuation<void*>* completion) override {
                if (skipped_ >= count_) return collector_->emit(std::move(value), completion);
                ++skipped_;
                return nullptr;
            }
        private:
            const std::shared_ptr<Flow<T>> upstream_;
            FlowCollector<T>* const collector_;
            const int count_;
            int skipped_ = 0;
        };
        auto receiver = std::make_shared<DropCollector>(upstream, collector, count);
        return receiver->collect(kotlinx::coroutines::internal::retain_continuation(completion));
    });
}

/** Returns a flow containing all elements except initial elements satisfying predicate. */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:30-40
template<typename T, typename Predicate>
std::shared_ptr<Flow<T>> drop_while(std::shared_ptr<Flow<T>> upstream, Predicate predicate_fn) {
    auto predicate = std::make_shared<Predicate>(std::move(predicate_fn));
    return internal::unsafe_flow<T>([upstream = std::move(upstream), predicate = std::move(predicate)](
        FlowCollector<T>* collector, Continuation<void*>* completion) -> void* {
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:31-39
        class DropWhileCollector final : public FlowCollector<T>, public std::enable_shared_from_this<DropWhileCollector> {
        public:
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:31-39
            DropWhileCollector(std::shared_ptr<Flow<T>> upstream, FlowCollector<T>* collector, std::shared_ptr<Predicate> predicate)
                : upstream_(std::move(upstream)), collector_(collector), predicate_(std::move(predicate)) {}
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:32-39
            [[clang::annotate("suspend")]]
            void* collect(std::shared_ptr<Continuation<void*>> completion) {
                auto owner = this->shared_from_this();
                dsl::suspend(upstream_->collect(this, completion.get()));
                return nullptr;
            }
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:33-38
            void* emit(T value, Continuation<void*>* completion) override {
                if (matched_) return collector_->emit(std::move(value), completion);
                return emit(std::move(value), kotlinx::coroutines::internal::retain_continuation(completion));
            }
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:35-38
            [[clang::annotate("suspend")]]
            void* emit(T value, std::shared_ptr<Continuation<void*>> completion) {
                auto owner = this->shared_from_this();
                void* raw = dsl::suspend(detail::invoke_limit_predicate(*predicate_, completion.get(), value));
                std::unique_ptr<bool> box(static_cast<bool*>(raw));
                bool matches = *box;
                box.reset();
                if (!matches) {
                    matched_ = true;
                    dsl::suspend(collector_->emit(std::move(value), completion.get()));
                }
                return nullptr;
            }
        private:
            const std::shared_ptr<Flow<T>> upstream_;
            FlowCollector<T>* const collector_;
            const std::shared_ptr<Predicate> predicate_;
            bool matched_ = false;
        };
        auto receiver = std::make_shared<DropWhileCollector>(upstream, collector, predicate);
        return receiver->collect(kotlinx::coroutines::internal::retain_continuation(completion));
    });
}

/**
 * Returns a flow containing the first count elements.
 * Collection is cancelled when count elements are consumed.
 * Throws std::invalid_argument if count is not positive.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:47-68
template<typename T>
std::shared_ptr<Flow<T>> take(std::shared_ptr<Flow<T>> upstream, int count) {
    if (count <= 0) throw std::invalid_argument("Requested element count " + std::to_string(count) + " should be positive");
    return internal::unsafe_flow<T>([upstream = std::move(upstream), count](
        FlowCollector<T>* collector, Continuation<void*>* completion) -> void* {
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:49-67
        class TakeCollector final : public FlowCollector<T>, public std::enable_shared_from_this<TakeCollector> {
        public:
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:49-67
            TakeCollector(std::shared_ptr<Flow<T>> upstream, FlowCollector<T>* collector, int count)
                : upstream_(std::move(upstream)), collector_(collector), count_(count) {}
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:52-67
            [[clang::annotate("suspend")]]
            void* collect(std::shared_ptr<Continuation<void*>> completion) {
                auto owner = this->shared_from_this();
                try {
                    dsl::suspend(upstream_->collect(this, completion.get()));
                } catch (internal::AbortFlowException& error) {
                    error.check_ownership(&ownership_marker_);
                }
                return nullptr;
            }
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:54-62
            void* emit(T value, Continuation<void*>* completion) override {
                // Check the condition first, then tail-call emit or emit_abort.
                // Normal execution needs no state machine; only termination does.
                // See TakeBenchmark for comparison of the source approaches.
                // NOTE(port): Kotlin Int increment wraps; signed C++ overflow is undefined.
                consumed_ = consumed_ == std::numeric_limits<int>::max() ? std::numeric_limits<int>::min() : consumed_ + 1;
                if (consumed_ < count_) return collector_->emit(std::move(value), completion);
                return emit_abort(collector_, std::move(value), &ownership_marker_, completion);
            }
        private:
            const std::shared_ptr<Flow<T>> upstream_;
            FlowCollector<T>* const collector_;
            const int count_;
            int consumed_ = 0;
            // NOTE(port): This stable identity represents the source ownershipMarker.
            char ownership_marker_ = 0;
        };
        auto receiver = std::make_shared<TakeCollector>(upstream, collector, count);
        return receiver->collect(kotlinx::coroutines::internal::retain_continuation(completion));
    });
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:123-140
template<typename T, typename Predicate>
void* collect_while(std::shared_ptr<Flow<T>> upstream, Predicate predicate, Continuation<void*>* completion = nullptr) {
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:124-139
    class CollectWhileCollector final : public FlowCollector<T>, public std::enable_shared_from_this<CollectWhileCollector> {
    public:
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:124-139
        CollectWhileCollector(std::shared_ptr<Flow<T>> upstream, Predicate predicate)
            : upstream_(std::move(upstream)), predicate_(std::move(predicate)) {}
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:133-139
        [[clang::annotate("suspend")]]
        void* collect(std::shared_ptr<Continuation<void*>> completion) {
            auto owner = this->shared_from_this();
            try {
                dsl::suspend(upstream_->collect(this, completion.get()));
            } catch (internal::AbortFlowException& error) {
                error.check_ownership(static_cast<FlowCollector<T>*>(this));
                // Cancellation may precede this collector's AbortFlowException.
                context_ensure_active(*completion->get_context());
            }
            return nullptr;
        }
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:125-132
        void* emit(T value, Continuation<void*>* completion) override {
            return emit(std::move(value), kotlinx::coroutines::internal::retain_continuation(completion));
        }
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:125-132
        [[clang::annotate("suspend")]]
        void* emit(T value, std::shared_ptr<Continuation<void*>> completion) {
            auto owner = this->shared_from_this();
            // Evaluate predicate first, then throw. A suspending predicate makes
            // this source operation non-tail-suspending and requires lowering.
            void* raw = dsl::suspend(detail::invoke_limit_predicate(predicate_, completion.get(), value));
            std::unique_ptr<bool> box(static_cast<bool*>(raw));
            bool keep_collecting = *box;
            box.reset();
            if (!keep_collecting) throw internal::AbortFlowException(static_cast<FlowCollector<T>*>(this));
            return nullptr;
        }
    private:
        const std::shared_ptr<Flow<T>> upstream_;
        Predicate predicate_;
    };
    auto receiver = std::make_shared<CollectWhileCollector>(std::move(upstream), std::move(predicate));
    return receiver->collect(kotlinx::coroutines::internal::retain_continuation(completion));
}

/**
 * Returns a flow containing initial elements satisfying predicate.
 * The element for which predicate returns false is excluded.
 * See transform_while for a more flexible operator.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:81-90
template<typename T, typename Predicate>
std::shared_ptr<Flow<T>> take_while(std::shared_ptr<Flow<T>> upstream, Predicate predicate_fn) {
    auto predicate = std::make_shared<Predicate>(std::move(predicate_fn));
    return internal::unsafe_flow<T>([upstream = std::move(upstream), predicate = std::move(predicate)](
        FlowCollector<T>* collector, Continuation<void*>* completion) -> void* {
        auto block = [collector, predicate](T value, std::shared_ptr<Continuation<void*>> completion)
            __attribute__((annotate("suspend"))) -> void* {
            void* raw = dsl::suspend(detail::invoke_limit_predicate(*predicate, completion.get(), value));
            std::unique_ptr<bool> box(static_cast<bool*>(raw));
            bool matches = *box;
            box.reset();
            if (matches) {
                dsl::suspend(collector->emit(std::move(value), completion.get()));
                return new bool(true);
            }
            return new bool(false);
        };
        // Preserve the source tail return through collect_while (KT-39227).
        return collect_while<T>(upstream, [block = std::move(block)](T value, Continuation<void*>* completion) -> void* {
            return block(std::move(value), kotlinx::coroutines::internal::retain_continuation(completion));
        }, completion);
    });
}

/**
 * Applies transform_fn to each upstream value while it returns true.
 * Its collector receiver may emit multiple elements, skip elements or transform them.
 * Emissions made by the call returning false are kept.
 *
 * This generalizes take_while: a download-progress transform can emit progress
 * before returning !progress.is_done(), retaining the final completed update.
 *
 * ```cpp
 * auto complete_when_done(std::shared_ptr<Flow<DownloadProgress>> updates) {
 *     return transform_while<DownloadProgress, DownloadProgress>(updates,
 *         [](FlowCollector<DownloadProgress>* collector, DownloadProgress progress,
 *            std::shared_ptr<Continuation<void*>> completion)
 *            __attribute__((annotate("suspend"))) -> void* {
 *             dsl::suspend(collector->emit(progress, completion.get()));
 *             return new bool(!progress.is_done());
 *         });
 * }
 * ```
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:112-120
template<typename T, typename R, typename Transform>
std::shared_ptr<Flow<R>> transform_while(std::shared_ptr<Flow<T>> upstream, Transform transform_fn) {
    auto transform = std::make_shared<Transform>(std::move(transform_fn));
    // Safe flow is used because the collector is exposed to the transform.
    return flow<R>([upstream = std::move(upstream), transform = std::move(transform)](
        FlowCollector<R>* collector, Continuation<void*>* completion) -> void* {
        // Preserve the source tail return through collect_while (KT-39227).
        return collect_while<T>(upstream, [collector, transform](T value, Continuation<void*>* completion) -> void* {
            return detail::invoke_limit_predicate(*transform, completion, collector, std::move(value));
        }, completion);
    });
}

} // namespace kotlinx::coroutines::flow
