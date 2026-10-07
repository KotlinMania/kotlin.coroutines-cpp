#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt
/**
 * @file Emitters.hpp
 * @brief Flow operators that emit values: transform, onStart, onCompletion, onEmpty
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt
 */

#include "kotlinx/coroutines/flow/internal/ThrowingCollector.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include <functional>
#include <exception>
#include <memory>

namespace kotlinx {
namespace coroutines {
namespace flow {

// Alias for the flow builder to avoid shadowing issues with parameters named 'flow'
template<typename T>
inline std::shared_ptr<Flow<T>> make_flow(std::function<void(FlowCollector<T>*)> block) {
    return flow<T>(block);
}

/**
 * Applies transform function to each value of the given flow.
 *
 * The receiver of the transform is FlowCollector and thus transform is a
 * flexible function that may transform emitted element, skip it or emit it multiple times.
 */
template <typename T, typename R>
std::shared_ptr<Flow<R>> transform(std::shared_ptr<Flow<T>> upstream, std::function<void(FlowCollector<R>*, T)> transform_fn) {
    return make_flow<R>([upstream, transform_fn](FlowCollector<R>* collector) {
        upstream->collect([&](T value) {
            transform_fn(collector, value);
        });
    });
}

/**
 * Returns a flow that invokes the given action BEFORE this flow starts to be collected.
 *
 * The action is called before the upstream flow is collected. Action may emit values using
 * the collector or just perform some side effect.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:70-81
template <typename T>
std::shared_ptr<Flow<T>> on_start(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(FlowCollector<T>*, Continuation<void*>*)> action) {
    return unsafe_flow<T>([upstream = std::move(upstream), action = std::move(action)](
        FlowCollector<T>* collector, Continuation<void*>* completion) -> void* {
        // NOTE(port): Retained fields represent the Kotlin compiler's spilled
        // locals; mandatory LLVM injection supplies the resume addresses.
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:73-81
        class StartFrame final : public ContinuationImpl {
        public:
            void* _label = nullptr;
            StartFrame(std::shared_ptr<Flow<T>> upstream,
                       std::function<void*(FlowCollector<T>*, Continuation<void*>*)> action,
                       FlowCollector<T>* collector, Continuation<void*>* completion)
                : ContinuationImpl(completion ? std::shared_ptr<Continuation<void*>>(
                      completion, [](Continuation<void*>*) {}) : nullptr),
                  upstream_(std::move(upstream)), action_(std::move(action)), collector_(collector),
                  safe_collector_(std::make_shared<internal::SafeCollector<T>>(
                      collector, completion ? completion->get_context() : EmptyCoroutineContext::instance())) {}

            void retain() { self_ref_ = shared_from_this(); }
            void release() { self_ref_.reset(); }

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:73-81
            void* invoke_suspend(Result<void*> result) override {
                try {
                    coroutine_begin(this)
                    coroutine_yield(this, action_(safe_collector_.get(), this));
                    safe_collector_->release_intercepted();
                    safe_collector_.reset();
                    coroutine_yield(this, upstream_->collect(collector_, this));
                } catch (...) {
                    if (safe_collector_) {
                        safe_collector_->release_intercepted();
                        safe_collector_.reset();
                    }
                    release();
                    throw;
                }
                release();
                coroutine_end(this)
            }

        private:
            std::shared_ptr<Flow<T>> upstream_;
            std::function<void*(FlowCollector<T>*, Continuation<void*>*)> action_;
            FlowCollector<T>* collector_;
            std::shared_ptr<internal::SafeCollector<T>> safe_collector_;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
        auto frame = std::make_shared<StartFrame>(upstream, action, collector, completion);
        frame->retain();
        try {
            return frame->start(Result<void*>::success(nullptr));
        } catch (...) {
            frame->release();
            throw;
        }
    });
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:70-81
template <typename T>
std::shared_ptr<Flow<T>> on_start(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void(FlowCollector<T>*)> action) {
    return on_start<T>(std::move(upstream),
        [action = std::move(action)](FlowCollector<T>* collector, Continuation<void*>*) -> void* {
            action(collector);
            return nullptr;
        });
}

/**
 * Returns a flow that invokes the given action AFTER the flow is completed or cancelled,
 * passing the cancellation exception or null if it completed successfully.
 *
 * Conceptually, onCompletion is similar to wrapping the flow collection into a finally block.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:140-160
 */
template <typename T>
inline std::shared_ptr<Flow<T>> on_completion(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void*(FlowCollector<T>*, std::exception_ptr, Continuation<void*>*)> action) {
    return flow<T>([upstream = std::move(upstream), action = std::move(action)](
        FlowCollector<T>* collector, Continuation<void*>* cont) -> void* {
        try {
            void* res = upstream->collect(collector, cont);
            if (intrinsics::is_coroutine_suspended(res)) {
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
        } catch (...) {
            auto ex = std::current_exception();
            void* a_res = action(collector, ex, cont);
            if (intrinsics::is_coroutine_suspended(a_res)) {
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
            throw;
        }
        void* a_res = action(collector, nullptr, cont);
        if (intrinsics::is_coroutine_suspended(a_res)) {
            return intrinsics::get_COROUTINE_SUSPENDED();
        }
        return nullptr;
    });
}

/**
 * Non-suspending collector overload of [on_completion].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:140-160
 */
template <typename T>
inline std::shared_ptr<Flow<T>> on_completion(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void(FlowCollector<T>*, std::exception_ptr)> action) {
    return on_completion<T>(
        std::move(upstream),
        [action = std::move(action)](
            FlowCollector<T>* collector, std::exception_ptr cause, Continuation<void*>*) -> void* {
            action(collector, cause);
            return nullptr;
        });
}

/**
 * Non-suspending simplified overload of [on_completion] omitting collector.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Emitters.kt:140-160
 */
template <typename T>
inline std::shared_ptr<Flow<T>> on_completion(
    std::shared_ptr<Flow<T>> upstream,
    std::function<void(std::exception_ptr)> action) {
    return on_completion<T>(
        std::move(upstream),
        [action = std::move(action)](FlowCollector<T>*, std::exception_ptr cause) {
            action(cause);
        });
}

/**
 * Invokes the given action when this flow completes without emitting any elements.
 *
 * The receiver of action is FlowCollector so action can emit additional elements.
 */
template <typename T>
std::shared_ptr<Flow<T>> on_empty(std::shared_ptr<Flow<T>> upstream, std::function<void(FlowCollector<T>*)> action) {
    return make_flow<T>([upstream, action](FlowCollector<T>* collector) {
        bool isEmpty = true;
        upstream->collect([&](T value) {
            isEmpty = false;
            collector->emit(value);
        });
        if (isEmpty) {
            action(collector);
        }
    });
}

} // namespace flow
} // namespace coroutines
} // namespace kotlinx
