#pragma once
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Count.kt
 *
 * Kotlin file header (translated):
 *   @file:JvmMultifileClass
 *   @file:JvmName("FlowKt")
 *   package kotlinx.coroutines.flow
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <atomic>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace kotlinx::coroutines::flow {

namespace detail {

/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Count.kt:11-18
 *
 * Heap-allocated state machine frame for bare `count()`.
 * Implements both FlowCollector<T> (sink collector) and Continuation<void*> (completion interception),
 * anchoring the counter and flow instance across suspension points to eliminate stack-UAF.
 */
template <typename T>
struct CountFrame : public FlowCollector<T>,
                    public Continuation<void*>,
                    public std::enable_shared_from_this<CountFrame<T>> {
    int count_ = 0;
    Continuation<void*>* completion = nullptr;
    std::shared_ptr<Flow<T>> flow_holder;

    std::recursive_mutex mutex;
    std::exception_ptr failure = nullptr;
    std::atomic<bool> completed{false};
    std::shared_ptr<CountFrame<T>> self_ref;

    explicit CountFrame(
        Continuation<void*>* comp,
        std::shared_ptr<Flow<T>> flow = nullptr)
        : completion(comp),
          flow_holder(std::move(flow)) {}

    ~CountFrame() override = default;

    void retain_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (!completed.load()) {
            self_ref = this->shared_from_this();
        }
    }

    void release_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        self_ref = nullptr;
        flow_holder = nullptr;
    }

    bool is_completed() const {
        return completed.load();
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        return completion ? completion->get_context() : nullptr;
    }

    void resume_with(Result<void*> collect_res) override {
        std::shared_ptr<CountFrame<T>> guard;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (completed.exchange(true)) return;
            guard = std::move(self_ref);
            self_ref = nullptr;
            flow_holder = nullptr;
        }

        std::exception_ptr fail = nullptr;
        int final_count = 0;
        if (collect_res.is_success()) {
            {
                std::lock_guard<std::recursive_mutex> lock(mutex);
                fail = failure;
                final_count = count_;
            }
            if (fail) {
                if (completion) {
                    completion->resume_with(Result<void*>::failure(fail));
                }
            } else {
                if (completion) {
                    void* res = static_cast<void*>(new int(final_count));
                    completion->resume_with(Result<void*>::success(res));
                }
            }
        } else {
            if (completion) {
                completion->resume_with(Result<void*>::failure(collect_res.exception_or_null()));
            }
        }
    }

    void* emit(T /*value*/, Continuation<void*>* /*continuation*/) override {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (failure) {
            std::rethrow_exception(failure);
        }
        ++count_;
        return nullptr;
    }
};

/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Count.kt:23-32
 *
 * Heap-allocated state machine frame for `count(predicate)`.
 * Implements both FlowCollector<T> and Continuation<void*>, handling both synchronous
 * and suspending predicates without skipping counter increments, deadlocks, or leaking heap memory.
 */
template <typename T>
struct CountPredicateFrame : public FlowCollector<T>,
                             public Continuation<void*>,
                             public std::enable_shared_from_this<CountPredicateFrame<T>> {
    int count_ = 0;
    std::function<void*(T, Continuation<void*>*)> predicate_;
    Continuation<void*>* completion = nullptr;
    std::shared_ptr<Flow<T>> flow_holder;

    std::recursive_mutex mutex;
    std::exception_ptr failure = nullptr;
    std::atomic<bool> completed{false};
    std::shared_ptr<CountPredicateFrame<T>> self_ref;
    std::shared_ptr<Continuation<void*>> active_op_cont;

    CountPredicateFrame(
        std::function<void*(T, Continuation<void*>*)> predicate,
        Continuation<void*>* comp,
        std::shared_ptr<Flow<T>> flow = nullptr)
        : predicate_(std::move(predicate)),
          completion(comp),
          flow_holder(std::move(flow)) {}

    ~CountPredicateFrame() override = default;

    void retain_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (!completed.load()) {
            self_ref = this->shared_from_this();
        }
    }

    void release_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        self_ref = nullptr;
        flow_holder = nullptr;
        active_op_cont = nullptr;
    }

    bool is_completed() const {
        return completed.load();
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        return completion ? completion->get_context() : nullptr;
    }

    void resume_with(Result<void*> collect_res) override {
        std::shared_ptr<CountPredicateFrame<T>> guard;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (completed.exchange(true)) return;
            guard = std::move(self_ref);
            self_ref = nullptr;
            flow_holder = nullptr;
            active_op_cont = nullptr;
        }

        std::exception_ptr fail = nullptr;
        int final_count = 0;
        if (collect_res.is_success()) {
            {
                std::lock_guard<std::recursive_mutex> lock(mutex);
                fail = failure;
                final_count = count_;
            }
            if (fail) {
                if (completion) {
                    completion->resume_with(Result<void*>::failure(fail));
                }
            } else {
                if (completion) {
                    void* res = static_cast<void*>(new int(final_count));
                    completion->resume_with(Result<void*>::success(res));
                }
            }
        } else {
            if (completion) {
                completion->resume_with(Result<void*>::failure(collect_res.exception_or_null()));
            }
        }
    }

    void* emit(T value, Continuation<void*>* cont) override {
        {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (failure) {
                std::rethrow_exception(failure);
            }
        }

        std::weak_ptr<CountPredicateFrame<T>> self_weak = this->shared_from_this();
        auto op_cont = std::make_shared<FunctionalContinuation<void*>>(
            cont ? cont->get_context() : nullptr,
            [self_weak, cont](Result<void*> op_res) {
                auto self = self_weak.lock();
                if (!self) return;

                std::shared_ptr<Continuation<void*>> op_guard;
                {
                    std::lock_guard<std::recursive_mutex> lock(self->mutex);
                    op_guard = std::move(self->active_op_cont);
                }

                if (op_res.is_success()) {
                    void* raw = op_res.get_or_throw();
                    if (raw) {
                        auto* b_ptr = static_cast<bool*>(raw);
                        if (*b_ptr) {
                            std::lock_guard<std::recursive_mutex> lock(self->mutex);
                            ++self->count_;
                        }
                        delete b_ptr;
                    }
                    if (cont) cont->resume_with(Result<void*>::success(nullptr));
                } else {
                    {
                        std::lock_guard<std::recursive_mutex> lock(self->mutex);
                        self->failure = op_res.exception_or_null();
                    }
                    if (cont) cont->resume_with(op_res);
                }
            }
        );

        {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            active_op_cont = op_cont;
        }

        void* res = nullptr;
        try {
            res = predicate_(std::move(value), op_cont.get());
        } catch (...) {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            active_op_cont = nullptr;
            failure = std::current_exception();
            throw;
        }

        if (intrinsics::is_coroutine_suspended(res)) {
            return intrinsics::get_COROUTINE_SUSPENDED();
        }

        {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            active_op_cont = nullptr;
        }

        if (res) {
            auto* b_ptr = static_cast<bool*>(res);
            if (*b_ptr) {
                std::lock_guard<std::recursive_mutex> lock(mutex);
                ++count_;
            }
            delete b_ptr;
        }
        return nullptr;
    }
};

} // namespace detail

/**
 * Returns the number of elements in this flow.
 *
 * Upstream:
 *   public suspend fun <T> Flow<T>.count(): Int  {
 *       var i = 0
 *       collect { ++i }
 *       return i
 *   }
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Count.kt:11-18
 */
template <typename T>
[[suspend]]
inline void* count(
    Flow<T>* flow,
    Continuation<void*>* completion = nullptr,
    std::shared_ptr<Flow<T>> flow_holder = nullptr) {
    if (!flow) {
        throw std::invalid_argument("flow cannot be null");
    }

    auto frame = std::make_shared<detail::CountFrame<T>>(completion, std::move(flow_holder));
    frame->retain_self();

    void* r = nullptr;
    try {
        r = flow->collect(frame.get(), frame.get());
    } catch (...) {
        frame->release_self();
        throw;
    }

    if (intrinsics::is_coroutine_suspended(r) || frame->is_completed()) {
        return intrinsics::get_COROUTINE_SUSPENDED();
    }

    frame->completed.store(true);
    frame->release_self();

    if (frame->failure) {
        std::rethrow_exception(frame->failure);
    }

    return static_cast<void*>(new int(frame->count_));
}

template <typename T>
[[suspend]]
inline void* count(
    Flow<T>* flow,
    std::shared_ptr<Continuation<void*>> completion) {
    return count<T>(flow, completion.get());
}

template <typename T>
[[suspend]]
inline void* count(
    std::shared_ptr<Flow<T>> flow,
    Continuation<void*>* completion = nullptr) {
    if (!flow) {
        throw std::invalid_argument("flow cannot be null");
    }
    auto* flow_ptr = flow.get();
    return count<T>(flow_ptr, completion, std::move(flow));
}

template <typename T>
[[suspend]]
inline void* count(
    std::shared_ptr<Flow<T>> flow,
    std::shared_ptr<Continuation<void*>> completion) {
    return count<T>(std::move(flow), completion.get());
}

/**
 * Returns the number of elements matching the given predicate.
 *
 * Upstream:
 *   public suspend fun <T> Flow<T>.count(predicate: suspend (T) -> Boolean): Int {
 *       var i = 0
 *       collect { value -> if (predicate(value)) ++i }
 *       return i
 *   }
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Count.kt:23-32
 */
template <typename T>
[[suspend]]
inline void* count(
    Flow<T>* flow,
    std::function<void*(T, Continuation<void*>*)> predicate,
    Continuation<void*>* completion = nullptr,
    std::shared_ptr<Flow<T>> flow_holder = nullptr) {
    if (!flow) {
        throw std::invalid_argument("flow cannot be null");
    }
    if (!predicate) {
        throw std::invalid_argument("predicate cannot be null");
    }

    auto frame = std::make_shared<detail::CountPredicateFrame<T>>(
        std::move(predicate), completion, std::move(flow_holder));
    frame->retain_self();

    void* r = nullptr;
    try {
        r = flow->collect(frame.get(), frame.get());
    } catch (...) {
        frame->release_self();
        throw;
    }

    if (intrinsics::is_coroutine_suspended(r) || frame->is_completed()) {
        return intrinsics::get_COROUTINE_SUSPENDED();
    }

    frame->completed.store(true);
    frame->release_self();

    if (frame->failure) {
        std::rethrow_exception(frame->failure);
    }

    return static_cast<void*>(new int(frame->count_));
}

template <typename T>
[[suspend]]
inline void* count(
    Flow<T>* flow,
    std::function<void*(T, Continuation<void*>*)> predicate,
    std::shared_ptr<Continuation<void*>> completion) {
    return count<T>(flow, std::move(predicate), completion.get());
}

template <typename T>
[[suspend]]
inline void* count(
    std::shared_ptr<Flow<T>> flow,
    std::function<void*(T, Continuation<void*>*)> predicate,
    Continuation<void*>* completion = nullptr) {
    if (!flow) {
        throw std::invalid_argument("flow cannot be null");
    }
    auto* flow_ptr = flow.get();
    return count<T>(flow_ptr, std::move(predicate), completion, std::move(flow));
}

template <typename T>
[[suspend]]
inline void* count(
    std::shared_ptr<Flow<T>> flow,
    std::function<void*(T, Continuation<void*>*)> predicate,
    std::shared_ptr<Continuation<void*>> completion) {
    return count<T>(std::move(flow), std::move(predicate), completion.get());
}

template <typename T, typename Predicate,
          typename = std::enable_if_t<!std::is_same_v<std::decay_t<Predicate>, std::shared_ptr<Continuation<void*>>> &&
                                      !std::is_same_v<std::decay_t<Predicate>, Continuation<void*>*>>>
[[suspend]]
inline void* count(
    Flow<T>* flow,
    Predicate predicate,
    Continuation<void*>* completion = nullptr) {
    if constexpr (std::is_invocable_r_v<void*, Predicate, T, Continuation<void*>*>) {
        return count<T>(
            flow,
            std::function<void*(T, Continuation<void*>*)>(
                [pred = std::move(predicate)](T value, Continuation<void*>* cont) -> void* {
                    return pred(std::move(value), cont);
                }),
            completion);
    } else {
        return count<T>(
            flow,
            std::function<void*(T, Continuation<void*>*)>(
                [pred = std::move(predicate)](T value, Continuation<void*>* cont) -> void* {
                    return static_cast<void*>(new bool(pred(std::move(value))));
                }),
            completion);
    }
}

template <typename T, typename Predicate,
          typename = std::enable_if_t<!std::is_same_v<std::decay_t<Predicate>, std::shared_ptr<Continuation<void*>>> &&
                                      !std::is_same_v<std::decay_t<Predicate>, Continuation<void*>*>>>
[[suspend]]
inline void* count(
    Flow<T>* flow,
    Predicate predicate,
    std::shared_ptr<Continuation<void*>> completion) {
    return count<T, Predicate>(flow, std::move(predicate), completion.get());
}

template <typename T, typename Predicate,
          typename = std::enable_if_t<!std::is_same_v<std::decay_t<Predicate>, std::shared_ptr<Continuation<void*>>> &&
                                      !std::is_same_v<std::decay_t<Predicate>, Continuation<void*>*>>>
[[suspend]]
inline void* count(
    std::shared_ptr<Flow<T>> flow,
    Predicate predicate,
    Continuation<void*>* completion = nullptr) {
    if (!flow) {
        throw std::invalid_argument("flow cannot be null");
    }
    auto* flow_ptr = flow.get();
    if constexpr (std::is_invocable_r_v<void*, Predicate, T, Continuation<void*>*>) {
        return count<T>(
            flow_ptr,
            std::function<void*(T, Continuation<void*>*)>(
                [pred = std::move(predicate)](T value, Continuation<void*>* cont) -> void* {
                    return pred(std::move(value), cont);
                }),
            completion,
            std::move(flow));
    } else {
        return count<T>(
            flow_ptr,
            std::function<void*(T, Continuation<void*>*)>(
                [pred = std::move(predicate)](T value, Continuation<void*>* cont) -> void* {
                    return static_cast<void*>(new bool(pred(std::move(value))));
                }),
            completion,
            std::move(flow));
    }
}

template <typename T, typename Predicate,
          typename = std::enable_if_t<!std::is_same_v<std::decay_t<Predicate>, std::shared_ptr<Continuation<void*>>> &&
                                      !std::is_same_v<std::decay_t<Predicate>, Continuation<void*>*>>>
[[suspend]]
inline void* count(
    std::shared_ptr<Flow<T>> flow,
    Predicate predicate,
    std::shared_ptr<Continuation<void*>> completion) {
    return count<T, Predicate>(std::move(flow), std::move(predicate), completion.get());
}

} // namespace kotlinx::coroutines::flow
