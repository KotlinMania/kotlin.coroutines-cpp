// port-lint: source kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt
#pragma once
/**
 * @file Reduce.hpp
 * @brief Terminal flow operators for reduction: reduce, fold, first, last, single
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt
 */

#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowCollector.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/flow/internal/NullSurrogate.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>

namespace kotlinx::coroutines::flow {

namespace internal {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:15-30
template <typename T, typename S>
struct ReduceFrame : public FlowCollector<T>, public Continuation<void*>, public std::enable_shared_from_this<ReduceFrame<T, S>> {
    void* accumulator = nullptr;
    bool has_value = false;
    std::function<S(S, T)> sync_operation;
    std::function<void*(S, T, Continuation<void*>*)> susp_operation;
    bool is_suspending_op = false;
    Continuation<void*>* completion = nullptr;
    std::recursive_mutex mutex;
    std::exception_ptr failure = nullptr;
    std::atomic<bool> completed{false};
    std::shared_ptr<ReduceFrame<T, S>> self_ref;

    ReduceFrame(std::function<S(S, T)> op, Continuation<void*>* comp)
        : sync_operation(std::move(op)),
          is_suspending_op(false),
          completion(comp) {}

    ReduceFrame(std::function<void*(S, T, Continuation<void*>*)> op, Continuation<void*>* comp)
        : susp_operation(std::move(op)),
          is_suspending_op(true),
          completion(comp) {}

    ~ReduceFrame() override {
        if (has_value && accumulator != nullptr) {
            delete static_cast<S*>(accumulator);
            accumulator = nullptr;
        }
    }

    void retain_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (!completed.load()) {
            self_ref = this->shared_from_this();
        }
    }

    void release_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        self_ref = nullptr;
    }

    bool is_completed() const {
        return completed.load();
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        return completion ? completion->get_context() : nullptr;
    }

    void resume_with(Result<void*> collect_res) override {
        std::shared_ptr<ReduceFrame<T, S>> guard;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (completed.exchange(true)) return;
            guard = std::move(self_ref);
            self_ref = nullptr;
        }

        if (collect_res.is_success()) {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (failure) {
                if (completion) {
                    completion->resume_with(Result<void*>::failure(failure));
                }
            } else if (!has_value) {
                if (completion) {
                    completion->resume_with(Result<void*>::failure(
                        std::make_exception_ptr(NoSuchElementException("Empty flow can't be reduced"))));
                }
            } else {
                void* res = accumulator;
                accumulator = nullptr;
                has_value = false;
                if (completion) {
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
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (failure) {
            std::rethrow_exception(failure);
        }
        if (!is_suspending_op) {
            try {
                if (!has_value) {
                    accumulator = new S(std::move(value));
                    has_value = true;
                } else {
                    S* acc = static_cast<S*>(accumulator);
                    *acc = sync_operation(std::move(*acc), std::move(value));
                }
            } catch (...) {
                failure = std::current_exception();
                throw;
            }
            return nullptr;
        } else {
            if (!has_value) {
                accumulator = new S(std::move(value));
                has_value = true;
                return nullptr;
            }
            S* acc = static_cast<S*>(accumulator);
            auto op_cont = std::make_shared<FunctionalContinuation<void*>>(
                cont ? cont->get_context() : nullptr,
                [this, cont](Result<void*> op_res) {
                    if (op_res.is_success()) {
                        void* raw = op_res.get_or_throw();
                        if (raw) {
                            auto* s_ptr = static_cast<S*>(raw);
                            {
                                std::lock_guard<std::recursive_mutex> lock(this->mutex);
                                if (this->has_value && this->accumulator != nullptr) {
                                    *static_cast<S*>(this->accumulator) = std::move(*s_ptr);
                                }
                            }
                            delete s_ptr;
                        }
                        if (cont) cont->resume_with(Result<void*>::success(nullptr));
                    } else {
                        {
                            std::lock_guard<std::recursive_mutex> lock(this->mutex);
                            this->failure = op_res.exception_or_null();
                        }
                        if (cont) cont->resume_with(op_res);
                    }
                }
            );

            void* res = susp_operation(*acc, std::move(value), op_cont.get());
            if (intrinsics::is_coroutine_suspended(res)) {
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
            if (res) {
                auto* s_ptr = static_cast<S*>(res);
                *acc = std::move(*s_ptr);
                delete s_ptr;
            }
            return nullptr;
        }
    }
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:35-44
template <typename T, typename R>
struct FoldFrame : public FlowCollector<T>, public Continuation<void*>, public std::enable_shared_from_this<FoldFrame<T, R>> {
    R accumulator;
    std::function<R(R, T)> sync_operation;
    std::function<void*(R, T, Continuation<void*>*)> susp_operation;
    bool is_suspending_op = false;
    Continuation<void*>* completion = nullptr;
    std::recursive_mutex mutex;
    std::exception_ptr failure = nullptr;
    std::atomic<bool> completed{false};
    std::shared_ptr<FoldFrame<T, R>> self_ref;

    FoldFrame(R init, std::function<R(R, T)> op, Continuation<void*>* comp)
        : accumulator(std::move(init)),
          sync_operation(std::move(op)),
          is_suspending_op(false),
          completion(comp) {}

    FoldFrame(R init, std::function<void*(R, T, Continuation<void*>*)> op, Continuation<void*>* comp)
        : accumulator(std::move(init)),
          susp_operation(std::move(op)),
          is_suspending_op(true),
          completion(comp) {}

    void retain_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (!completed.load()) {
            self_ref = this->shared_from_this();
        }
    }

    void release_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        self_ref = nullptr;
    }

    bool is_completed() const {
        return completed.load();
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        return completion ? completion->get_context() : nullptr;
    }

    void resume_with(Result<void*> collect_res) override {
        std::shared_ptr<FoldFrame<T, R>> guard;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (completed.exchange(true)) return;
            guard = std::move(self_ref);
            self_ref = nullptr;
        }

        if (collect_res.is_success()) {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (failure) {
                if (completion) {
                    completion->resume_with(Result<void*>::failure(failure));
                }
            } else {
                auto* final_res = new R(std::move(accumulator));
                if (completion) {
                    completion->resume_with(Result<void*>::success(final_res));
                }
            }
        } else {
            if (completion) {
                completion->resume_with(Result<void*>::failure(collect_res.exception_or_null()));
            }
        }
    }

    void* emit(T value, Continuation<void*>* cont) override {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (failure) {
            std::rethrow_exception(failure);
        }
        if (!is_suspending_op) {
            try {
                accumulator = sync_operation(std::move(accumulator), std::move(value));
            } catch (...) {
                failure = std::current_exception();
                throw;
            }
            return nullptr;
        } else {
            auto op_cont = std::make_shared<FunctionalContinuation<void*>>(
                cont ? cont->get_context() : nullptr,
                [this, cont](Result<void*> op_res) {
                    if (op_res.is_success()) {
                        void* raw = op_res.get_or_throw();
                        if (raw) {
                            auto* r_ptr = static_cast<R*>(raw);
                            {
                                std::lock_guard<std::recursive_mutex> lock(this->mutex);
                                this->accumulator = std::move(*r_ptr);
                            }
                            delete r_ptr;
                        }
                        if (cont) cont->resume_with(Result<void*>::success(nullptr));
                    } else {
                        {
                            std::lock_guard<std::recursive_mutex> lock(this->mutex);
                            this->failure = op_res.exception_or_null();
                        }
                        if (cont) cont->resume_with(op_res);
                    }
                }
            );

            void* res = susp_operation(accumulator, std::move(value), op_cont.get());
            if (intrinsics::is_coroutine_suspended(res)) {
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
            if (res) {
                auto* r_ptr = static_cast<R*>(res);
                accumulator = std::move(*r_ptr);
                delete r_ptr;
            }
            return nullptr;
        }
    }
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:51-60
template <typename T>
struct SingleFrame : public FlowCollector<T>, public Continuation<void*>, public std::enable_shared_from_this<SingleFrame<T>> {
    void* result = nullptr;
    bool has_value = false;
    bool has_multiple = false;
    Continuation<void*>* completion = nullptr;
    std::recursive_mutex mutex;
    std::exception_ptr failure = nullptr;
    std::atomic<bool> completed{false};
    std::shared_ptr<SingleFrame<T>> self_ref;

    explicit SingleFrame(Continuation<void*>* comp) : completion(comp) {}

    ~SingleFrame() override {
        if (has_value && result != nullptr) {
            delete static_cast<T*>(result);
            result = nullptr;
        }
    }

    void retain_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (!completed.load()) {
            self_ref = this->shared_from_this();
        }
    }

    void release_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        self_ref = nullptr;
    }

    bool is_completed() const {
        return completed.load();
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        return completion ? completion->get_context() : nullptr;
    }

    void resume_with(Result<void*> collect_res) override {
        std::shared_ptr<SingleFrame<T>> guard;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (completed.exchange(true)) return;
            guard = std::move(self_ref);
            self_ref = nullptr;
        }

        if (collect_res.is_success()) {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (failure) {
                if (completion) {
                    completion->resume_with(Result<void*>::failure(failure));
                }
            } else if (!has_value) {
                if (completion) {
                    completion->resume_with(Result<void*>::failure(
                        std::make_exception_ptr(NoSuchElementException("Flow is empty"))));
                }
            } else {
                void* res = result;
                result = nullptr;
                has_value = false;
                if (completion) {
                    completion->resume_with(Result<void*>::success(res));
                }
            }
        } else {
            if (completion) {
                completion->resume_with(Result<void*>::failure(collect_res.exception_or_null()));
            }
        }
    }

    void* emit(T value, Continuation<void*>*) override {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (failure) {
            std::rethrow_exception(failure);
        }
        if (has_value) {
            has_multiple = true;
            failure = std::make_exception_ptr(std::invalid_argument("Flow has more than one element"));
            throw std::invalid_argument("Flow has more than one element");
        }
        result = new T(std::move(value));
        has_value = true;
        return nullptr;
    }
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:66-80
template <typename T>
struct SingleOrNullFrame : public FlowCollector<T>, public Continuation<void*>, public std::enable_shared_from_this<SingleOrNullFrame<T>> {
    void* result = nullptr;
    bool has_value = false;
    bool has_multiple = false;
    Continuation<void*>* completion = nullptr;
    std::recursive_mutex mutex;
    std::exception_ptr failure = nullptr;
    std::atomic<bool> completed{false};
    std::shared_ptr<SingleOrNullFrame<T>> self_ref;

    explicit SingleOrNullFrame(Continuation<void*>* comp) : completion(comp) {}

    ~SingleOrNullFrame() override {
        if (has_value && result != nullptr) {
            delete static_cast<T*>(result);
            result = nullptr;
        }
    }

    void retain_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (!completed.load()) {
            self_ref = this->shared_from_this();
        }
    }

    void release_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        self_ref = nullptr;
    }

    bool is_completed() const {
        return completed.load();
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        return completion ? completion->get_context() : nullptr;
    }

    void resume_with(Result<void*> collect_res) override {
        std::shared_ptr<SingleOrNullFrame<T>> guard;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (completed.exchange(true)) return;
            guard = std::move(self_ref);
            self_ref = nullptr;
        }

        std::exception_ptr ex = nullptr;
        if (collect_res.is_failure()) {
            try {
                std::rethrow_exception(collect_res.exception_or_null());
            } catch (internal::AbortFlowException& e) {
                if (e.owner == this) {
                    // Expected short circuit - returns null
                } else {
                    ex = std::current_exception();
                }
            } catch (...) {
                ex = std::current_exception();
            }
        }
        if (ex) {
            if (completion) {
                completion->resume_with(Result<void*>::failure(ex));
            }
        } else {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (has_multiple || !has_value) {
                if (completion) {
                    completion->resume_with(Result<void*>::success(nullptr));
                }
            } else {
                void* res = result;
                result = nullptr;
                has_value = false;
                if (completion) {
                    completion->resume_with(Result<void*>::success(res));
                }
            }
        }
    }

    void* emit(T value, Continuation<void*>*) override {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (failure) {
            std::rethrow_exception(failure);
        }
        if (!has_value && !has_multiple) {
            result = new T(std::move(value));
            has_value = true;
        } else {
            has_multiple = true;
            if (has_value && result != nullptr) {
                delete static_cast<T*>(result);
                result = nullptr;
            }
            has_value = false;
            throw internal::AbortFlowException(this);
        }
        return nullptr;
    }
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:86-142
template <typename T>
struct FirstFrame : public FlowCollector<T>, public Continuation<void*>, public std::enable_shared_from_this<FirstFrame<T>> {
    void* result = nullptr;
    bool has_value = false;
    std::function<bool(const T&)> predicate;
    bool has_predicate = false;
    bool is_or_null = false;
    Continuation<void*>* completion = nullptr;
    std::recursive_mutex mutex;
    std::exception_ptr failure = nullptr;
    std::atomic<bool> completed{false};
    std::shared_ptr<FirstFrame<T>> self_ref;

    FirstFrame(bool or_null, Continuation<void*>* comp)
        : is_or_null(or_null), completion(comp) {}

    FirstFrame(std::function<bool(const T&)> pred, bool or_null, Continuation<void*>* comp)
        : predicate(std::move(pred)), has_predicate(true), is_or_null(or_null), completion(comp) {}

    ~FirstFrame() override {
        if (has_value && result != nullptr) {
            delete static_cast<T*>(result);
            result = nullptr;
        }
    }

    void retain_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (!completed.load()) {
            self_ref = this->shared_from_this();
        }
    }

    void release_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        self_ref = nullptr;
    }

    bool is_completed() const {
        return completed.load();
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        return completion ? completion->get_context() : nullptr;
    }

    void resume_with(Result<void*> collect_res) override {
        std::shared_ptr<FirstFrame<T>> guard;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (completed.exchange(true)) return;
            guard = std::move(self_ref);
            self_ref = nullptr;
        }

        std::exception_ptr ex = nullptr;
        if (collect_res.is_failure()) {
            try {
                std::rethrow_exception(collect_res.exception_or_null());
            } catch (internal::AbortFlowException& e) {
                if (e.owner == this) {
                    // Expected short circuit
                } else {
                    ex = std::current_exception();
                }
            } catch (...) {
                ex = std::current_exception();
            }
        }
        if (ex) {
            if (completion) {
                completion->resume_with(Result<void*>::failure(ex));
            }
        } else {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (!has_value) {
                if (is_or_null) {
                    if (completion) {
                        completion->resume_with(Result<void*>::success(nullptr));
                    }
                } else {
                    if (completion) {
                        const char* msg = has_predicate
                            ? "Expected at least one element matching the predicate"
                            : "Expected at least one element";
                        completion->resume_with(Result<void*>::failure(
                            std::make_exception_ptr(NoSuchElementException(msg))));
                    }
                }
            } else {
                void* res = result;
                result = nullptr;
                has_value = false;
                if (completion) {
                    completion->resume_with(Result<void*>::success(res));
                }
            }
        }
    }

    void* emit(T value, Continuation<void*>*) override {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (failure) {
            std::rethrow_exception(failure);
        }
        if (!has_predicate || predicate(value)) {
            result = new T(std::move(value));
            has_value = true;
            throw internal::AbortFlowException(this);
        }
        return nullptr;
    }
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:149-167
template <typename T>
struct LastFrame : public FlowCollector<T>, public Continuation<void*>, public std::enable_shared_from_this<LastFrame<T>> {
    void* result = nullptr;
    bool has_value = false;
    bool is_or_null = false;
    Continuation<void*>* completion = nullptr;
    std::recursive_mutex mutex;
    std::exception_ptr failure = nullptr;
    std::atomic<bool> completed{false};
    std::shared_ptr<LastFrame<T>> self_ref;

    LastFrame(bool or_null, Continuation<void*>* comp)
        : is_or_null(or_null), completion(comp) {}

    ~LastFrame() override {
        if (has_value && result != nullptr) {
            delete static_cast<T*>(result);
            result = nullptr;
        }
    }

    void retain_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (!completed.load()) {
            self_ref = this->shared_from_this();
        }
    }

    void release_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        self_ref = nullptr;
    }

    bool is_completed() const {
        return completed.load();
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        return completion ? completion->get_context() : nullptr;
    }

    void resume_with(Result<void*> collect_res) override {
        std::shared_ptr<LastFrame<T>> guard;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (completed.exchange(true)) return;
            guard = std::move(self_ref);
            self_ref = nullptr;
        }

        if (collect_res.is_success()) {
            std::lock_guard<std::recursive_mutex> lock(mutex);
            if (failure) {
                if (completion) {
                    completion->resume_with(Result<void*>::failure(failure));
                }
            } else if (!has_value) {
                if (is_or_null) {
                    if (completion) {
                        completion->resume_with(Result<void*>::success(nullptr));
                    }
                } else {
                    if (completion) {
                        completion->resume_with(Result<void*>::failure(
                            std::make_exception_ptr(NoSuchElementException("Expected at least one element"))));
                    }
                }
            } else {
                void* res = result;
                result = nullptr;
                has_value = false;
                if (completion) {
                    completion->resume_with(Result<void*>::success(res));
                }
            }
        } else {
            if (completion) {
                completion->resume_with(Result<void*>::failure(collect_res.exception_or_null()));
            }
        }
    }

    void* emit(T value, Continuation<void*>*) override {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (failure) {
            std::rethrow_exception(failure);
        }
        if (has_value && result != nullptr) {
            delete static_cast<T*>(result);
        }
        result = new T(std::move(value));
        has_value = true;
        return nullptr;
    }
};

} // namespace internal

/**
 * Accumulates value starting with the first element and applying [operation] to current accumulator value and each element.
 * Throws NoSuchElementException if flow was empty.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:15-30
 */
template<typename T, typename S = T>
inline void* reduce(
    std::shared_ptr<Flow<T>> flow,
    std::function<S(S, T)> operation,
    Continuation<void*>* continuation) {
    auto frame = std::make_shared<internal::ReduceFrame<T, S>>(std::move(operation), continuation);
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

    if (!frame->has_value) {
        throw NoSuchElementException("Empty flow can't be reduced");
    }

    void* res = frame->accumulator;
    frame->accumulator = nullptr;
    frame->has_value = false;
    return res;
}

/**
 * Suspending overload of [reduce].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:15-30
 */
template<typename T, typename S = T>
inline void* reduce(
    std::shared_ptr<Flow<T>> flow,
    std::function<void*(S, T, Continuation<void*>*)> operation,
    Continuation<void*>* continuation) {
    auto frame = std::make_shared<internal::ReduceFrame<T, S>>(std::move(operation), continuation);
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

    if (!frame->has_value) {
        throw NoSuchElementException("Empty flow can't be reduced");
    }

    void* res = frame->accumulator;
    frame->accumulator = nullptr;
    frame->has_value = false;
    return res;
}

/**
 * Single-type overload of [reduce] where accumulator type equals element type.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:15-30
 */
template<typename T>
inline void* reduce(
    std::shared_ptr<Flow<T>> flow,
    std::function<T(T, T)> operation,
    Continuation<void*>* continuation) {
    return reduce<T, T>(std::move(flow), std::move(operation), continuation);
}

/**
 * Single-type suspending overload of [reduce] where accumulator type equals element type.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:15-30
 */
template<typename T>
inline void* reduce(
    std::shared_ptr<Flow<T>> flow,
    std::function<void*(T, T, Continuation<void*>*)> operation,
    Continuation<void*>* continuation) {
    return reduce<T, T>(std::move(flow), std::move(operation), continuation);
}

/**
 * Accumulates value starting with [initial] value and applying [operation] to current accumulator value and each element.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:35-44
 */
template<typename T, typename R>
inline void* fold(
    std::shared_ptr<Flow<T>> flow,
    R initial,
    std::function<R(R, T)> operation,
    Continuation<void*>* continuation) {
    auto frame = std::make_shared<internal::FoldFrame<T, R>>(std::move(initial), std::move(operation), continuation);
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

    return new R(std::move(frame->accumulator));
}

/**
 * Suspending overload of [fold].
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:35-44
 */
template<typename T, typename R>
inline void* fold(
    std::shared_ptr<Flow<T>> flow,
    R initial,
    std::function<void*(R, T, Continuation<void*>*)> operation,
    Continuation<void*>* continuation) {
    auto frame = std::make_shared<internal::FoldFrame<T, R>>(std::move(initial), std::move(operation), continuation);
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

    return new R(std::move(frame->accumulator));
}

/**
 * The terminal operator that awaits for one and only one value to be emitted.
 * Throws NoSuchElementException for empty flow and std::invalid_argument for flow
 * that contains more than one element.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:51-60
 */
template<typename T>
inline void* single(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    auto frame = std::make_shared<internal::SingleFrame<T>>(continuation);
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

    if (!frame->has_value) {
        throw NoSuchElementException("Flow is empty");
    }

    void* res = frame->result;
    frame->result = nullptr;
    frame->has_value = false;
    return res;
}

/**
 * The terminal operator that awaits for one and only one value to be emitted.
 * Returns the single value or nullptr, if the flow was empty or emitted more than one value.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:66-80
 */
template<typename T>
inline void* single_or_null(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    auto frame = std::make_shared<internal::SingleOrNullFrame<T>>(continuation);
    frame->retain_self();

    void* r = nullptr;
    try {
        r = flow->collect(frame.get(), frame.get());
    } catch (internal::AbortFlowException& e) {
        e.check_ownership(frame.get());
    } catch (...) {
        frame->release_self();
        throw;
    }

    if (intrinsics::is_coroutine_suspended(r) || frame->is_completed()) {
        return intrinsics::get_COROUTINE_SUSPENDED();
    }

    frame->completed.store(true);
    frame->release_self();

    if (frame->has_multiple || !frame->has_value) {
        return nullptr;
    }

    void* res = frame->result;
    frame->result = nullptr;
    frame->has_value = false;
    return res;
}

/**
 * The terminal operator that returns the first element emitted by the flow and then cancels flow's collection.
 * Throws NoSuchElementException if the flow was empty.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:86-94
 */
template<typename T>
inline void* first(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    auto frame = std::make_shared<internal::FirstFrame<T>>(false, continuation);
    frame->retain_self();

    void* r = nullptr;
    try {
        r = flow->collect(frame.get(), frame.get());
    } catch (internal::AbortFlowException& e) {
        e.check_ownership(frame.get());
    } catch (...) {
        frame->release_self();
        throw;
    }

    if (intrinsics::is_coroutine_suspended(r) || frame->is_completed()) {
        return intrinsics::get_COROUTINE_SUSPENDED();
    }

    frame->completed.store(true);
    frame->release_self();

    if (!frame->has_value) {
        throw NoSuchElementException("Expected at least one element");
    }

    void* res = frame->result;
    frame->result = nullptr;
    frame->has_value = false;
    return res;
}

/**
 * The terminal operator that returns the first element emitted by the flow matching the given [predicate]
 * and then cancels flow's collection. Throws NoSuchElementException if the flow has not contained matching elements.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:100-112
 */
template<typename T>
inline void* first(
    std::shared_ptr<Flow<T>> flow,
    std::function<bool(const T&)> predicate,
    Continuation<void*>* continuation) {
    auto frame = std::make_shared<internal::FirstFrame<T>>(std::move(predicate), false, continuation);
    frame->retain_self();

    void* r = nullptr;
    try {
        r = flow->collect(frame.get(), frame.get());
    } catch (internal::AbortFlowException& e) {
        e.check_ownership(frame.get());
    } catch (...) {
        frame->release_self();
        throw;
    }

    if (intrinsics::is_coroutine_suspended(r) || frame->is_completed()) {
        return intrinsics::get_COROUTINE_SUSPENDED();
    }

    frame->completed.store(true);
    frame->release_self();

    if (!frame->has_value) {
        throw NoSuchElementException("Expected at least one element matching the predicate");
    }

    void* res = frame->result;
    frame->result = nullptr;
    frame->has_value = false;
    return res;
}

/**
 * The terminal operator that returns the first element emitted by the flow and then cancels flow's collection.
 * Returns nullptr if the flow was empty.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:118-125
 */
template<typename T>
inline void* first_or_null(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    auto frame = std::make_shared<internal::FirstFrame<T>>(true, continuation);
    frame->retain_self();

    void* r = nullptr;
    try {
        r = flow->collect(frame.get(), frame.get());
    } catch (internal::AbortFlowException& e) {
        e.check_ownership(frame.get());
    } catch (...) {
        frame->release_self();
        throw;
    }

    if (intrinsics::is_coroutine_suspended(r) || frame->is_completed()) {
        return intrinsics::get_COROUTINE_SUSPENDED();
    }

    frame->completed.store(true);
    frame->release_self();

    if (!frame->has_value) {
        return nullptr;
    }

    void* res = frame->result;
    frame->result = nullptr;
    frame->has_value = false;
    return res;
}

/**
 * The terminal operator that returns the first element emitted by the flow matching the given [predicate]
 * and then cancels flow's collection. Returns nullptr if the flow did not contain a matching element.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:131-142
 */
template<typename T>
inline void* first_or_null(
    std::shared_ptr<Flow<T>> flow,
    std::function<bool(const T&)> predicate,
    Continuation<void*>* continuation) {
    auto frame = std::make_shared<internal::FirstFrame<T>>(std::move(predicate), true, continuation);
    frame->retain_self();

    void* r = nullptr;
    try {
        r = flow->collect(frame.get(), frame.get());
    } catch (internal::AbortFlowException& e) {
        e.check_ownership(frame.get());
    } catch (...) {
        frame->release_self();
        throw;
    }

    if (intrinsics::is_coroutine_suspended(r) || frame->is_completed()) {
        return intrinsics::get_COROUTINE_SUSPENDED();
    }

    frame->completed.store(true);
    frame->release_self();

    if (!frame->has_value) {
        return nullptr;
    }

    void* res = frame->result;
    frame->result = nullptr;
    frame->has_value = false;
    return res;
}

/**
 * The terminal operator that returns the last element emitted by the flow.
 * Throws NoSuchElementException if the flow was empty.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:149-156
 */
template<typename T>
inline void* last(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    auto frame = std::make_shared<internal::LastFrame<T>>(false, continuation);
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

    if (!frame->has_value) {
        throw NoSuchElementException("Expected at least one element");
    }

    void* res = frame->result;
    frame->result = nullptr;
    frame->has_value = false;
    return res;
}

/**
 * The terminal operator that returns the last element emitted by the flow or nullptr if the flow was empty.
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Reduce.kt:161-167
 */
template<typename T>
inline void* last_or_null(std::shared_ptr<Flow<T>> flow, Continuation<void*>* continuation) {
    auto frame = std::make_shared<internal::LastFrame<T>>(true, continuation);
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

    if (!frame->has_value) {
        return nullptr;
    }

    void* res = frame->result;
    frame->result = nullptr;
    frame->has_value = false;
    return res;
}

} // namespace kotlinx::coroutines::flow
