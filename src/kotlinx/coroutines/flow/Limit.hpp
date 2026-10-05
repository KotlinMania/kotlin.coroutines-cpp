#pragma once
// port-lint: source flow/operators/Limit.kt
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt
/** @file Limit.hpp Flow operators that discard initial elements or retain a prefix. */

#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/flow/internal/SafeCollector.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace kotlinx::coroutines::flow {

namespace detail {

// Adapt non-suspending callables to the suspend Boolean ABI.
// The receiving frame owns and deletes the Boolean box on either result path.
template<typename Predicate, typename... Args>
void* invoke_limit_predicate(Predicate& predicate, Continuation<void*>* continuation, Args&&... args) {
    if constexpr (std::is_invocable_v<Predicate&, Args..., Continuation<void*>*>) {
        static_assert(std::is_same_v<std::invoke_result_t<Predicate&, Args..., Continuation<void*>*>, void*>);
        return predicate(std::forward<Args>(args)..., continuation);
    } else {
        return new bool(predicate(std::forward<Args>(args)...));
    }
}

} // namespace detail

/**
 * Returns a flow that ignores the first @p count elements of @p upstream.
 * @throws std::invalid_argument if @p count is negative.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:17-26
template<typename T>
std::shared_ptr<Flow<T>> drop(std::shared_ptr<Flow<T>> upstream, int count) {
    if (count < 0) throw std::invalid_argument("Drop count should be non-negative, but had " + std::to_string(count));
    return internal::unsafe_flow<T>([upstream, count](FlowCollector<T>* collector, Continuation<void*>* cont) -> void* {
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:19-25
        class CollectFrame final : public ContinuationImpl, public FlowCollector<T> {
        public:
            CollectFrame(std::shared_ptr<Flow<T>> upstream, FlowCollector<T>* collector, int count,
                         Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  upstream_(std::move(upstream)), collector_(collector), count_(count) {}
            void retain() { self_ref_ = shared_from_this(); }
            void* invoke_suspend(Result<void*> result) override {
                coroutine_begin(this)
                coroutine_yield(this, upstream_->collect(this, this));
                coroutine_end(this)
            }
            void* emit(T value, Continuation<void*>* completion) override {
                if (skipped_ >= count_) return collector_->emit(std::move(value), completion);
                ++skipped_;
                return nullptr;
            }
        protected:
            void release_intercepted() override {
                ContinuationImpl::release_intercepted();
                self_ref_.reset();
            }
        private:
            void* _label = nullptr;
            std::shared_ptr<Flow<T>> upstream_;
            FlowCollector<T>* collector_;
            int count_;
            int skipped_ = 0;
            // Retain the captured collector until collection terminates.
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
        auto frame = std::make_shared<CollectFrame>(upstream, collector, count, cont);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

/**
 * Returns a flow containing all elements except the initial elements satisfying
 * @p predicate. Once the predicate returns false, that element and all remaining
 * elements are emitted without evaluating the predicate again.
 *
 * A suspending callable accepts (value, Continuation<void*>*) and returns either
 * intrinsics::get_COROUTINE_SUSPENDED() or a heap-allocated bool. The receiving
 * frame deletes the bool after reading it. A synchronous bool(value) callable is
 * also accepted.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:30-40
template<typename T, typename Predicate>
std::shared_ptr<Flow<T>> drop_while(std::shared_ptr<Flow<T>> upstream, Predicate predicate) {
    return internal::unsafe_flow<T>([upstream, predicate](FlowCollector<T>* collector, Continuation<void*>* cont) -> void* {
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:30-40
        class CollectFrame final : public ContinuationImpl, public FlowCollector<T> {
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:35-38
            class EmitFrame final : public ContinuationImpl {
            public:
                EmitFrame(CollectFrame* owner, T value, Continuation<void*>* completion)
                    : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                      owner_(owner), value_(std::move(value)) {}
                void retain() { self_ref_ = shared_from_this(); }
                void* invoke_suspend(Result<void*> result) override {
                    coroutine_begin(this)
                    coroutine_yield_value(this, result,
                        detail::invoke_limit_predicate(owner_->predicate_, this, value_), predicate_result_);
                    if (!*std::unique_ptr<bool>(static_cast<bool*>(predicate_result_))) {
                        owner_->matched_ = true;
                        coroutine_yield(this, owner_->collector_->emit(std::move(value_), this));
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
                CollectFrame* owner_;
                T value_;
                void* predicate_result_ = nullptr;
                std::shared_ptr<BaseContinuationImpl> self_ref_;
            };
        public:
            CollectFrame(std::shared_ptr<Flow<T>> upstream, FlowCollector<T>* collector, Predicate predicate,
                         Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  upstream_(std::move(upstream)), collector_(collector), predicate_(std::move(predicate)) {}
            void retain() { self_ref_ = shared_from_this(); }
            void* invoke_suspend(Result<void*> result) override {
                coroutine_begin(this)
                coroutine_yield(this, upstream_->collect(this, this));
                coroutine_end(this)
            }
            void* emit(T value, Continuation<void*>* completion) override {
                if (matched_) return collector_->emit(std::move(value), completion);
                auto frame = std::make_shared<EmitFrame>(this, std::move(value), completion);
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
            FlowCollector<T>* collector_;
            Predicate predicate_;
            bool matched_ = false;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
        auto frame = std::make_shared<CollectFrame>(upstream, collector, predicate, cont);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

namespace detail {
template<typename T>
void* emit_abort(FlowCollector<T>* collector, T value, void* ownership_marker, Continuation<void*>* completion);
} // namespace detail

/**
 * Returns a flow containing the first @p count elements of @p upstream.
 * Upstream collection is cancelled after the final downstream emission completes.
 * @throws std::invalid_argument if @p count is not positive.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:47-68
template<typename T>
std::shared_ptr<Flow<T>> take(std::shared_ptr<Flow<T>> upstream, int count) {
    if (count <= 0) throw std::invalid_argument("Requested element count " + std::to_string(count) + " should be positive");
    return internal::unsafe_flow<T>([upstream, count](FlowCollector<T>* collector, Continuation<void*>* cont) -> void* {
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:49-67
        class CollectFrame final : public ContinuationImpl, public FlowCollector<T> {
        public:
            CollectFrame(std::shared_ptr<Flow<T>> upstream, FlowCollector<T>* collector, int count,
                         Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  upstream_(std::move(upstream)), collector_(collector), count_(count) {}
            void retain() { self_ref_ = shared_from_this(); }
            void* invoke_suspend(Result<void*> result) override {
                try {
                    coroutine_begin(this)
                    coroutine_yield(this, upstream_->collect(this, this));
                    coroutine_end(this)
                } catch (internal::AbortFlowException& error) {
                    error.check_ownership(&ownership_marker_);
                    return nullptr;
                }
            }
            void* emit(T value, Continuation<void*>* completion) override {
                // Check the condition first, then tail-call emit or emit_abort.
                // Only the terminating emission needs its own state machine.
                if (++consumed_ < count_) return collector_->emit(std::move(value), completion);
                return detail::emit_abort(collector_, std::move(value), &ownership_marker_, completion);
            }
        protected:
            void release_intercepted() override {
                ContinuationImpl::release_intercepted();
                self_ref_.reset();
            }
        private:
            void* _label = nullptr;
            std::shared_ptr<Flow<T>> upstream_;
            FlowCollector<T>* collector_;
            int count_;
            int consumed_ = 0;
            char ownership_marker_ = 0;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
        auto frame = std::make_shared<CollectFrame>(upstream, collector, count, cont);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
}

namespace detail {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:70-73
template<typename T>
void* emit_abort(FlowCollector<T>* collector, T value, void* ownership_marker, Continuation<void*>* completion) {
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:70-73
    class EmitAbortFrame final : public ContinuationImpl {
    public:
        EmitAbortFrame(FlowCollector<T>* collector, T value, void* marker, Continuation<void*>* completion)
            : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
              collector_(collector), value_(std::move(value)), ownership_marker_(marker) {}
        void retain() { self_ref_ = shared_from_this(); }
        void* invoke_suspend(Result<void*> result) override {
            coroutine_begin(this)
            coroutine_yield(this, collector_->emit(std::move(value_), this));
            throw internal::AbortFlowException(ownership_marker_);
        }
    protected:
        void release_intercepted() override {
            ContinuationImpl::release_intercepted();
            self_ref_.reset();
        }
    private:
        void* _label = nullptr;
        FlowCollector<T>* collector_;
        T value_;
        void* ownership_marker_;
        std::shared_ptr<BaseContinuationImpl> self_ref_;
    };
    auto frame = std::make_shared<EmitAbortFrame>(collector, std::move(value), ownership_marker, completion);
    frame->retain();
    return frame->start(Result<void*>::success(nullptr));
}

} // namespace detail

template<typename T, typename Predicate>
void* collect_while(std::shared_ptr<Flow<T>> upstream, Predicate predicate, Continuation<void*>* cont = nullptr);

/**
 * Returns a flow containing the initial elements satisfying @p predicate.
 * The element for which the predicate returns false is excluded.
 *
 * A suspending callable accepts (value, Continuation<void*>*) and returns either
 * intrinsics::get_COROUTINE_SUSPENDED() or a heap-allocated bool, consumed and
 * deleted by the receiving frame. A synchronous bool(value) callable is also
 * accepted.
 * @see transform_while for a more flexible operator.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:81-90
template<typename T, typename Predicate>
std::shared_ptr<Flow<T>> take_while(std::shared_ptr<Flow<T>> upstream, Predicate predicate) {
    return internal::unsafe_flow<T>([upstream, predicate](FlowCollector<T>* collector, Continuation<void*>* cont) -> void* {
        return collect_while<T>(upstream, [collector, predicate](T value, Continuation<void*>* completion) mutable -> void* {
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:83-89
            class PredicateFrame final : public ContinuationImpl {
            public:
                PredicateFrame(FlowCollector<T>* collector, Predicate* predicate, T value,
                               Continuation<void*>* completion)
                    : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                      collector_(collector), predicate_(predicate), value_(std::move(value)) {}
                void retain() { self_ref_ = shared_from_this(); }
                void* invoke_suspend(Result<void*> result) override {
                    coroutine_begin(this)
                    coroutine_yield_value(this, result,
                        detail::invoke_limit_predicate(*predicate_, this, value_), predicate_result_);
                    if (*std::unique_ptr<bool>(static_cast<bool*>(predicate_result_))) {
                        coroutine_yield(this, collector_->emit(std::move(value_), this));
                        return new bool(true);
                    }
                    return new bool(false);
                }
            protected:
                void release_intercepted() override {
                    ContinuationImpl::release_intercepted();
                    self_ref_.reset();
                }
            private:
                void* _label = nullptr;
                FlowCollector<T>* collector_;
                Predicate* predicate_;
                T value_;
                void* predicate_result_ = nullptr;
                std::shared_ptr<BaseContinuationImpl> self_ref_;
            };
            auto frame = std::make_shared<PredicateFrame>(collector, &predicate, std::move(value), completion);
            frame->retain();
            return frame->start(Result<void*>::success(nullptr));
        }, cont);
    });
}

/**
 * Applies @p transform_fn to each value of @p upstream while it returns true.
 * The callable receives a FlowCollector<R>* and may transform the value, skip it,
 * or emit it multiple times. Emissions made by the call returning false are kept.
 *
 * A suspending transform accepts (collector, value, Continuation<void*>*) and
 * returns intrinsics::get_COROUTINE_SUSPENDED() or a heap-allocated bool. The
 * receiving frame consumes and deletes that result. A synchronous
 * bool(collector, value) transform is also accepted; any emission that can suspend
 * requires the continuation-based form. The exposed collector enforces the flow
 * context and exception-transparency rules.
 *
 * This operator generalizes take_while and can build other limiting operators.
 * A download-progress transform can emit the final update before stopping. For
 * example, a retained ContinuationImpl frame with collector_ and progress_ fields
 * uses this body; its factory supplies the completion and retains the frame:
 * @code{.cpp}
 * void* invoke_suspend(Result<void*> result) override {
 *     coroutine_begin(this)
 *     coroutine_yield(this, collector_->emit(progress_, this));
 *     return new bool(!progress_.is_done());
 * }
 * @endcode
 * The frame also owns its resume label and releases its retained lifetime when
 * invocation terminates. The final progress value is emitted even when is_done()
 * is true, whereas take_while excludes the first value failing its predicate.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:112-120
template<typename T, typename R, typename Transform>
std::shared_ptr<Flow<R>> transform_while(std::shared_ptr<Flow<T>> upstream, Transform transform_fn) {
    return flow<R>([upstream, transform_fn](FlowCollector<R>* collector, Continuation<void*>* cont) -> void* {
        return collect_while<T>(upstream,
            [collector, transform_fn](T value, Continuation<void*>* completion) mutable -> void* {
                return detail::invoke_limit_predicate(transform_fn, completion, collector, std::move(value));
            }, cont);
    });
}

/**
 * Internal building block for flow-truncating operators with non-tail suspension.
 * Evaluates @p predicate before deciding whether to abort upstream collection.
 * A suspended predicate resumes that decision after producing its bool result.
 * Only this collector's abort is handled; its catch then checks cancellation of
 * the current context. Unrelated aborts and other failures propagate.
 */
// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:123-140
template<typename T, typename Predicate>
void* collect_while(std::shared_ptr<Flow<T>> upstream, Predicate predicate, Continuation<void*>* cont) {
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:124-139
    class CollectFrame final : public ContinuationImpl, public FlowCollector<T> {
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Limit.kt:125-132
        class EmitFrame final : public ContinuationImpl {
        public:
            EmitFrame(CollectFrame* owner, T value, Continuation<void*>* completion)
                : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                  owner_(owner), value_(std::move(value)) {}
            void retain() { self_ref_ = shared_from_this(); }
            void* invoke_suspend(Result<void*> result) override {
                coroutine_begin(this)
                // Evaluate the predicate first, then abort. A suspended predicate
                // must resume the remaining branch, including when it emits values.
                coroutine_yield_value(this, result,
                    detail::invoke_limit_predicate(owner_->predicate_, this, value_), predicate_result_);
                if (!*std::unique_ptr<bool>(static_cast<bool*>(predicate_result_))) {
                    throw internal::AbortFlowException(static_cast<FlowCollector<T>*>(owner_));
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
            CollectFrame* owner_;
            T value_;
            void* predicate_result_ = nullptr;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
    public:
        CollectFrame(std::shared_ptr<Flow<T>> upstream, Predicate predicate, Continuation<void*>* completion)
            : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
              upstream_(std::move(upstream)), predicate_(std::move(predicate)) {}
        void retain() { self_ref_ = shared_from_this(); }
        void* invoke_suspend(Result<void*> result) override {
            try {
                coroutine_begin(this)
                coroutine_yield(this, upstream_->collect(this, this));
                coroutine_end(this)
            } catch (internal::AbortFlowException& error) {
                error.check_ownership(static_cast<FlowCollector<T>*>(this));
                context_ensure_active(*get_context());
                return nullptr;
            }
        }
        void* emit(T value, Continuation<void*>* completion) override {
            auto frame = std::make_shared<EmitFrame>(this, std::move(value), completion);
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
        Predicate predicate_;
        std::shared_ptr<BaseContinuationImpl> self_ref_;
    };
    auto frame = std::make_shared<CollectFrame>(std::move(upstream), std::move(predicate), cont);
    frame->retain();
    return frame->start(Result<void*>::success(nullptr));
}

} // namespace kotlinx::coroutines::flow
