#pragma once
// port-lint: source flow/terminal/Logic.kt
// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Logic.kt
/** @file Logic.hpp Terminal flow operators for predicate logic: any, all, none. */

#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/Limit.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"

#include <memory>
#include <type_traits>
#include <utility>

namespace kotlinx::coroutines::flow {

/**
 * Terminal operator that returns true and immediately cancels the flow if at least one
 * element matches the given predicate.
 *
 * If the flow terminates without emitting any elements or no element matches the predicate,
 * returns false.
 * The unboxed Boolean result is equivalent to the negation of the unboxed Boolean result
 * of all evaluated with the inverted predicate, and equivalent to the negation of the unboxed
 * Boolean result of none evaluated with the same predicate.
 *
 * In the Continuation ABI, returns a heap-allocated bool* or COROUTINE_SUSPENDED.
 * Supply a live completion continuation whenever upstream or predicate can suspend.
 * The caller or resumed completion continuation owns and deletes the returned Boolean box.
 *
 * Example:
 * ```cpp
 * auto source = as_flow(std::vector<int>{1, 2, 3});
 * void* res = any(source, [](int x) { return x == 2; });
 * // Note: This example uses a non-suspending source and a synchronous predicate,
 * // returning an immediate heap-allocated bool* result. Calls with suspending sources
 * // or predicates may suspend (returning COROUTINE_SUSPENDED) or complete immediately;
 * // if suspension occurs, the result is delivered to the completion continuation.
 * std::unique_ptr<bool> box(static_cast<bool*>(res));
 * bool matches = *box;
 * ```
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Logic.kt:34-42
template <typename T, typename Predicate>
void* any(std::shared_ptr<Flow<T>> upstream, Predicate predicate, Continuation<void*>* completion = nullptr) {
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Logic.kt:34-42
    class AnyFrame final : public ContinuationImpl {
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Logic.kt:36-40
        class PredicateFrame final : public ContinuationImpl {
        public:
            PredicateFrame(AnyFrame* owner, T value, Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  owner_(owner), value_(std::move(value)) {}
            void retain() { self_ref_ = shared_from_this(); }
            void* invoke_suspend(Result<void*> result) override {
                coroutine_begin(this)
                coroutine_yield_value(this, result,
                    detail::invoke_limit_predicate(owner_->predicate_, this, value_), predicate_result_);
                {
                    bool satisfies = *std::unique_ptr<bool>(static_cast<bool*>(predicate_result_));
                    if (satisfies) {
                        owner_->found_ = true;
                    }
                    return new bool(!satisfies);
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
            AnyFrame* owner_;
            T value_;
            void* predicate_result_ = nullptr;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
    public:
        AnyFrame(std::shared_ptr<Flow<T>> upstream, Predicate predicate, Continuation<void*>* completion)
            : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
              upstream_(std::move(upstream)), predicate_(std::move(predicate)) {}
        void retain() { self_ref_ = shared_from_this(); }
        void* invoke_suspend(Result<void*> result) override {
            coroutine_begin(this)
            coroutine_yield(this, collect_while<T>(upstream_,
                [this](T value, Continuation<void*>* completion) -> void* {
                    auto frame = std::make_shared<PredicateFrame>(this, std::move(value), completion);
                    frame->retain();
                    return frame->start(Result<void*>::success(nullptr));
                }, this));
            return new bool(found_);
            coroutine_end(this)
        }
    protected:
        void release_intercepted() override {
            ContinuationImpl::release_intercepted();
            self_ref_.reset();
        }
    private:
        void* _label = nullptr;
        std::shared_ptr<Flow<T>> upstream_;
        Predicate predicate_;
        bool found_ = false;
        std::shared_ptr<BaseContinuationImpl> self_ref_;
    };
    auto frame = std::make_shared<AnyFrame>(std::move(upstream), std::move(predicate), completion);
    frame->retain();
    return frame->start(Result<void*>::success(nullptr));
}

/**
 * Terminal operator that returns true if all elements match the given predicate, or returns
 * false and immediately cancels the flow as soon as the first non-matching element is encountered.
 *
 * If the flow terminates without emitting any elements, returns true (vacuous truth).
 * The unboxed Boolean result is equivalent to the negation of the unboxed Boolean result
 * of any evaluated with the inverted predicate, and equivalent to the unboxed Boolean result
 * of none evaluated with the inverted predicate.
 *
 * In the Continuation ABI, returns a heap-allocated bool* or COROUTINE_SUSPENDED.
 * Supply a live completion continuation whenever upstream or predicate can suspend.
 * The caller or resumed completion continuation owns and deletes the returned Boolean box.
 *
 * Example:
 * ```cpp
 * auto source = as_flow(std::vector<int>{1, 2, 3});
 * void* res = all(source, [](int x) { return x > 0; });
 * // Note: This example uses a non-suspending source and a synchronous predicate,
 * // returning an immediate heap-allocated bool* result. Calls with suspending sources
 * // or predicates may suspend (returning COROUTINE_SUSPENDED) or complete immediately;
 * // if suspension occurs, the result is delivered to the completion continuation.
 * std::unique_ptr<bool> box(static_cast<bool*>(res));
 * bool all_match = *box;
 * ```
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Logic.kt:71-79
template <typename T, typename Predicate>
void* all(std::shared_ptr<Flow<T>> upstream, Predicate predicate, Continuation<void*>* completion = nullptr) {
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Logic.kt:71-79
    class AllFrame final : public ContinuationImpl {
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Logic.kt:73-77
        class PredicateFrame final : public ContinuationImpl {
        public:
            PredicateFrame(AllFrame* owner, T value, Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  owner_(owner), value_(std::move(value)) {}
            void retain() { self_ref_ = shared_from_this(); }
            void* invoke_suspend(Result<void*> result) override {
                coroutine_begin(this)
                coroutine_yield_value(this, result,
                    detail::invoke_limit_predicate(owner_->predicate_, this, value_), predicate_result_);
                {
                    bool satisfies = *std::unique_ptr<bool>(static_cast<bool*>(predicate_result_));
                    if (!satisfies) {
                        owner_->found_counter_example_ = true;
                    }
                    return new bool(satisfies);
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
            AllFrame* owner_;
            T value_;
            void* predicate_result_ = nullptr;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
    public:
        AllFrame(std::shared_ptr<Flow<T>> upstream, Predicate predicate, Continuation<void*>* completion)
            : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
              upstream_(std::move(upstream)), predicate_(std::move(predicate)) {}
        void retain() { self_ref_ = shared_from_this(); }
        void* invoke_suspend(Result<void*> result) override {
            coroutine_begin(this)
            coroutine_yield(this, collect_while<T>(upstream_,
                [this](T value, Continuation<void*>* completion) -> void* {
                    auto frame = std::make_shared<PredicateFrame>(this, std::move(value), completion);
                    frame->retain();
                    return frame->start(Result<void*>::success(nullptr));
                }, this));
            return new bool(!found_counter_example_);
            coroutine_end(this)
        }
    protected:
        void release_intercepted() override {
            ContinuationImpl::release_intercepted();
            self_ref_.reset();
        }
    private:
        void* _label = nullptr;
        std::shared_ptr<Flow<T>> upstream_;
        Predicate predicate_;
        bool found_counter_example_ = false;
        std::shared_ptr<BaseContinuationImpl> self_ref_;
    };
    auto frame = std::make_shared<AllFrame>(std::move(upstream), std::move(predicate), completion);
    frame->retain();
    return frame->start(Result<void*>::success(nullptr));
}

/**
 * Terminal operator that returns true if no elements match the given predicate, or returns
 * false and immediately cancels the flow as soon as the first matching element is encountered.
 *
 * If the flow terminates without emitting any elements, returns true (vacuous truth).
 * The unboxed Boolean result is equivalent to the negation of the unboxed Boolean result
 * of any evaluated with the same predicate, and equivalent to the unboxed Boolean result
 * of all evaluated with the inverted predicate.
 *
 * In the Continuation ABI, returns a heap-allocated bool* or COROUTINE_SUSPENDED.
 * Supply a live completion continuation whenever upstream or predicate can suspend.
 * The caller or resumed completion continuation owns and deletes the returned Boolean box.
 *
 * Example:
 * ```cpp
 * auto source = as_flow(std::vector<int>{1, 2, 3});
 * void* res = none(source, [](int x) { return x > 5; });
 * // Note: This example uses a non-suspending source and a synchronous predicate,
 * // returning an immediate heap-allocated bool* result. Calls with suspending sources
 * // or predicates may suspend (returning COROUTINE_SUSPENDED) or complete immediately;
 * // if suspension occurs, the result is delivered to the completion continuation.
 * std::unique_ptr<bool> box(static_cast<bool*>(res));
 * bool none_match = *box;
 * ```
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Logic.kt:107
template <typename T, typename Predicate>
void* none(std::shared_ptr<Flow<T>> upstream, Predicate predicate, Continuation<void*>* completion = nullptr) {
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/terminal/Logic.kt:107
    class NoneFrame final : public ContinuationImpl {
    public:
        NoneFrame(std::shared_ptr<Flow<T>> upstream, Predicate predicate, Continuation<void*>* completion)
            : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
              upstream_(std::move(upstream)), predicate_(std::move(predicate)) {}
        void retain() { self_ref_ = shared_from_this(); }
        void* invoke_suspend(Result<void*> result) override {
            coroutine_begin(this)
            coroutine_yield_value(this, result,
                any<T>(upstream_, predicate_, this), any_result_);
            {
                bool any_val = *std::unique_ptr<bool>(static_cast<bool*>(any_result_));
                return new bool(!any_val);
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
        std::shared_ptr<Flow<T>> upstream_;
        Predicate predicate_;
        void* any_result_ = nullptr;
        std::shared_ptr<BaseContinuationImpl> self_ref_;
    };
    auto frame = std::make_shared<NoneFrame>(std::move(upstream), std::move(predicate), completion);
    frame->retain();
    return frame->start(Result<void*>::success(nullptr));
}

} // namespace kotlinx::coroutines::flow
