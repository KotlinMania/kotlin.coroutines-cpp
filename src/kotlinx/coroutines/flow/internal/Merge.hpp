#pragma once
// port-lint: source flow/internal/Merge.kt
/**
 * @file Merge.hpp
 * @brief Internal flow merge operators (ChannelFlowMerge, ChannelLimitedFlowMerge, ChannelFlowTransformLatest).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt
 *
 */

#include "kotlinx/coroutines/flow/internal/ChannelFlow.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/flow/internal/FlowCoroutine.hpp"
#include "kotlinx/coroutines/sync/Semaphore.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/Produce.hpp" // For produce
#include "kotlinx/coroutines/Dispatchers.hpp"
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include "kotlinx/coroutines/EventLoop.hpp"
#include <memory>
#include <functional>
#include <vector>

namespace kotlinx {
namespace coroutines {
namespace flow {
namespace internal {

using kotlinx::coroutines::sync::Semaphore;
using kotlinx::coroutines::sync::create_semaphore;
using kotlinx::coroutines::channels::Channel;
using kotlinx::coroutines::channels::BufferOverflow;
// using kotlinx::coroutines::channels::SendingCollector; // moved to flow/internal

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:19-34
template <typename T, typename R>
class TransformLatestFrame final : public ContinuationImpl, public FlowCollector<T> {
public:
    using TransformType = std::function<void*(FlowCollector<R>*, T, Continuation<void*>*)>;

    TransformLatestFrame(std::shared_ptr<Flow<T>> flow, TransformType transform,
                         FlowCollector<R>* collector, CoroutineScope* scope,
                         std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)), flow_(std::move(flow)),
          transform_(std::move(transform)), collector_(collector), scope_(scope) {}

    void retain() { self_ref_ = shared_from_this(); }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:19-34
    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)
            coroutine_yield(this, flow_->collect(this, this));
            self_ref_.reset();
            coroutine_end(this)
        } catch (...) {
            self_ref_.reset();
            throw;
        }
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:23-33
    void* emit(T value, Continuation<void*>* completion) override {
        class EmitFrame final : public ContinuationImpl {
        public:
            EmitFrame(std::shared_ptr<TransformLatestFrame> owner, T value,
                      Continuation<void*>* completion)
                : ContinuationImpl(std::shared_ptr<Continuation<void*>>(completion, [](Continuation<void*>*) {})),
                  owner_(std::move(owner)), value_(std::move(value)) {}

            void retain() { self_ref_ = shared_from_this(); }

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:19-34
            void* invoke_suspend(Result<void*> result) override {
                try {
                    coroutine_begin(this)
                    if (owner_->previous_flow_) {
                        owner_->previous_flow_->cancel(std::make_exception_ptr(ChildCancelledException()));
                        coroutine_yield(this, owner_->previous_flow_->join(this));
                    }
                    owner_->launch_next(std::move(value_));
                    self_ref_.reset();
                    coroutine_end(this)
                } catch (...) {
                    self_ref_.reset();
                    throw;
                }
            }

        private:
            void* _label = nullptr;
            std::shared_ptr<TransformLatestFrame> owner_;
            T value_;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
        auto owner = std::static_pointer_cast<TransformLatestFrame>(shared_from_this());
        auto frame = std::make_shared<EmitFrame>(std::move(owner), std::move(value), completion);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    }

private:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:30-32
    void launch_next(T value) {
        class ChildFrame final : public ContinuationImpl {
        public:
            ChildFrame(TransformType transform, FlowCollector<R>* collector, T value,
                       std::shared_ptr<Continuation<void*>> completion)
                : ContinuationImpl(std::move(completion)), transform_(std::move(transform)),
                  collector_(collector), value_(std::move(value)) {}

            void retain() { self_ref_ = shared_from_this(); }

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:19-34
            void* invoke_suspend(Result<void*> result) override {
                try {
                    coroutine_begin(this)
                    coroutine_yield(this, transform_(collector_, std::move(value_), this));
                    self_ref_.reset();
                    coroutine_end(this)
                } catch (...) {
                    self_ref_.reset();
                    throw;
                }
            }

        private:
            void* _label = nullptr;
            TransformType transform_;
            FlowCollector<R>* collector_;
            T value_;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
        previous_flow_ = kotlinx::coroutines::launch(
            scope_, EmptyCoroutineContext::instance(), CoroutineStart::UNDISPATCHED,
            std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)>(
                [transform = transform_, collector = collector_, value = std::move(value)](
                    CoroutineScope*, std::shared_ptr<Continuation<void*>> completion) mutable -> void* {
                    auto frame = std::make_shared<ChildFrame>(std::move(transform), collector,
                                                             std::move(value), std::move(completion));
                    frame->retain();
                    return frame->start(Result<void*>::success(nullptr));
                }));
    }

    void* _label = nullptr;
    std::shared_ptr<Flow<T>> flow_;
    TransformType transform_;
    FlowCollector<R>* collector_;
    CoroutineScope* scope_;
    std::shared_ptr<Job> previous_flow_;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:9-35
template <typename T, typename R>
class ChannelFlowTransformLatest : public ChannelFlowOperator<T, R> {
public:
    using TransformType = std::function<void*(FlowCollector<R>*, T, Continuation<void*>*)>;

    ChannelFlowTransformLatest(
        TransformType transform,
        std::shared_ptr<Flow<T>> flow,
        std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
        int capacity = Channel<R>::BUFFERED,
        BufferOverflow on_buffer_overflow = BufferOverflow::SUSPEND
    ) : ChannelFlowOperator<T, R>(std::move(flow), std::move(context), capacity, on_buffer_overflow),
        transform_(std::move(transform)) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:16-17
    ChannelFlow<R>* create(std::shared_ptr<CoroutineContext> context, int capacity, BufferOverflow on_buffer_overflow) override {
        return new ChannelFlowTransformLatest<T, R>(transform_, this->upstream(), std::move(context), capacity, on_buffer_overflow);
    }

protected:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:19-34
    void* flow_collect(FlowCollector<R>* collector, Continuation<void*>* continuation) override {
        assert((dynamic_cast<SendingCollector<R>*>(collector) != nullptr));
        return collect_in_scope([flow = this->upstream(), transform = transform_, collector](
            CoroutineScope* scope, std::shared_ptr<Continuation<void*>> completion) -> void* {
            auto frame = std::make_shared<TransformLatestFrame<T, R>>(
                flow, transform, collector, scope, std::move(completion));
            frame->retain();
            return frame->start(Result<void*>::success(nullptr));
        }, continuation);
    }

private:
    TransformType transform_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:37-74
template <typename T>
class ChannelFlowMerge : public ChannelFlow<T> {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:37-43
    ChannelFlowMerge(
        std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> flow,
        int concurrency,
        std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
        int capacity = Channel<T>::BUFFERED,
        BufferOverflow on_buffer_overflow = BufferOverflow::SUSPEND)
        : ChannelFlow<T>(std::move(context), capacity, on_buffer_overflow),
          flow_(std::move(flow)), concurrency_(concurrency) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:44-45
    ChannelFlow<T>* create(std::shared_ptr<CoroutineContext> context, int capacity,
                           BufferOverflow on_buffer_overflow) override {
        return new ChannelFlowMerge<T>(flow_, concurrency_, std::move(context), capacity, on_buffer_overflow);
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:47-49
    std::shared_ptr<ReceiveChannel<T>> produce_impl(CoroutineScope* scope) override {
        return channels::produce<T>(scope, this->context(), this->capacity(),
                                    this->get_collect_to_fun());
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:51-71
    void* collect_to(ProducerScope<T>* scope,
                     std::shared_ptr<Continuation<void*>> completion) override {
        // NOTE(port): The Kotlin suspend-lambda's captured locals are owned by its continuation.
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:51-71
        class CollectFrame final : public ContinuationImpl,
                                   public FlowCollector<std::shared_ptr<Flow<T>>> {
        public:
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:51-54
            CollectFrame(std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> flow,
                         int concurrency, ProducerScope<T>* scope,
                         std::shared_ptr<Continuation<void*>> completion)
                : ContinuationImpl(std::move(completion)), flow_(std::move(flow)), scope_(scope),
                  semaphore_(create_semaphore(concurrency)),
                  collector_(std::make_shared<SendingCollector<T>>(scope)),
                  job_(std::dynamic_pointer_cast<Job>(get_context()->get(Job::type_key))) {}

            void retain() { self_ref_ = shared_from_this(); }

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:55-70
            void* invoke_suspend(Result<void*> result) override {
                try {
                    coroutine_begin(this)
                    coroutine_yield(this, flow_->collect(this, this));
                    self_ref_.reset();
                    coroutine_end(this)
                } catch (...) {
                    self_ref_.reset();
                    throw;
                }
            }

            // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:55-70
            void* emit(std::shared_ptr<Flow<T>> inner, Continuation<void*>* completion) override {
                // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:55-70
                class EmitFrame final : public ContinuationImpl {
                public:
                    EmitFrame(std::shared_ptr<CollectFrame> owner, std::shared_ptr<Flow<T>> inner,
                              Continuation<void*>* completion)
                        : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
                          owner_(std::move(owner)), inner_(std::move(inner)) {}

                    void retain() { self_ref_ = shared_from_this(); }

                    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:56-69
                    void* invoke_suspend(Result<void*> result) override {
                        try {
                            coroutine_begin(this)
                            /*
                             * We launch a coroutine on each emitted element and the only potential
                             * suspension point in this collector is `semaphore.acquire` that rarely suspends,
                             * so we manually check for cancellation to propagate it to the upstream in time.
                             */
                            if (owner_->job_) ensure_active(*owner_->job_);
                            coroutine_yield(this, owner_->semaphore_->acquire(this));
                            owner_->launch_inner(std::move(inner_));
                            self_ref_.reset();
                            coroutine_end(this)
                        } catch (...) {
                            self_ref_.reset();
                            throw;
                        }
                    }

                private:
                    void* _label = nullptr;
                    std::shared_ptr<CollectFrame> owner_;
                    std::shared_ptr<Flow<T>> inner_;
                    std::shared_ptr<BaseContinuationImpl> self_ref_;
                };
                auto frame = std::make_shared<EmitFrame>(
                    std::static_pointer_cast<CollectFrame>(shared_from_this()), std::move(inner), completion);
                frame->retain();
                return frame->start(Result<void*>::success(nullptr));
            }

        private:
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:63-69
            void launch_inner(std::shared_ptr<Flow<T>> inner) {
                // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:64-68
                class ChildFrame final : public ContinuationImpl {
                public:
                    ChildFrame(std::shared_ptr<Flow<T>> inner,
                               std::shared_ptr<SendingCollector<T>> collector,
                               std::shared_ptr<Semaphore> semaphore,
                               std::shared_ptr<Continuation<void*>> completion)
                        : ContinuationImpl(std::move(completion)), inner_(std::move(inner)),
                          collector_(std::move(collector)), semaphore_(std::move(semaphore)) {}

                    void retain() { self_ref_ = shared_from_this(); }

                    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:64-68
                    void* invoke_suspend(Result<void*> result) override {
                        try {
                            coroutine_begin(this)
                            coroutine_yield(this, inner_->collect(collector_.get(), this));
                        } catch (...) {
                            // Release concurrency permit
                            semaphore_->release();
                            self_ref_.reset();
                            throw;
                        }
                        // Release concurrency permit
                        semaphore_->release();
                        self_ref_.reset();
                        coroutine_end(this)
                    }

                private:
                    void* _label = nullptr;
                    std::shared_ptr<Flow<T>> inner_;
                    std::shared_ptr<SendingCollector<T>> collector_;
                    std::shared_ptr<Semaphore> semaphore_;
                    std::shared_ptr<BaseContinuationImpl> self_ref_;
                };
                kotlinx::coroutines::launch(scope_, EmptyCoroutineContext::instance(), CoroutineStart::DEFAULT,
                    std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)>(
                        [inner = std::move(inner), collector = collector_, semaphore = semaphore_](
                            CoroutineScope*, std::shared_ptr<Continuation<void*>> completion) -> void* {
                            auto frame = std::make_shared<ChildFrame>(inner, collector, semaphore, std::move(completion));
                            frame->retain();
                            return frame->start(Result<void*>::success(nullptr));
                        }));
            }

            void* _label = nullptr;
            std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> flow_;
            ProducerScope<T>* scope_;
            std::shared_ptr<Semaphore> semaphore_;
            std::shared_ptr<SendingCollector<T>> collector_;
            std::shared_ptr<Job> job_;
            std::shared_ptr<BaseContinuationImpl> self_ref_;
        };
        auto frame = std::make_shared<CollectFrame>(flow_, concurrency_, scope, std::move(completion));
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:73-73
    std::optional<std::string> additional_to_string_props() override {
        return "concurrency=" + std::to_string(concurrency_);
    }

private:
    std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> flow_;
    int concurrency_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:76-95
template <typename T>
class ChannelLimitedFlowMerge : public ChannelFlow<T> {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:76-81
    ChannelLimitedFlowMerge(
        std::vector<std::shared_ptr<Flow<T>>> flows,
        std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
        int capacity = Channel<T>::BUFFERED,
        BufferOverflow on_buffer_overflow = BufferOverflow::SUSPEND)
        : ChannelFlow<T>(std::move(context), capacity, on_buffer_overflow), flows_(std::move(flows)) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:82-83
    ChannelFlow<T>* create(std::shared_ptr<CoroutineContext> context, int capacity,
                           BufferOverflow on_buffer_overflow) override {
        return new ChannelLimitedFlowMerge<T>(flows_, std::move(context), capacity, on_buffer_overflow);
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:85-87
    std::shared_ptr<ReceiveChannel<T>> produce_impl(CoroutineScope* scope) override {
        return channels::produce<T>(scope, this->context(), this->capacity(),
                                    this->get_collect_to_fun());
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:89-94
    void* collect_to(ProducerScope<T>* scope,
                     std::shared_ptr<Continuation<void*>>) override {
        auto collector = std::make_shared<SendingCollector<T>>(scope);
        for (const auto& flow : flows_) {
            // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:92-92
            class ChildFrame final : public ContinuationImpl {
            public:
                ChildFrame(std::shared_ptr<Flow<T>> flow, std::shared_ptr<SendingCollector<T>> collector,
                           std::shared_ptr<Continuation<void*>> completion)
                    : ContinuationImpl(std::move(completion)), flow_(std::move(flow)), collector_(std::move(collector)) {}

                void retain() { self_ref_ = shared_from_this(); }

                // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:92-92
                void* invoke_suspend(Result<void*> result) override {
                    try {
                        coroutine_begin(this)
                        coroutine_yield(this, flow_->collect(collector_.get(), this));
                        self_ref_.reset();
                        coroutine_end(this)
                    } catch (...) {
                        self_ref_.reset();
                        throw;
                    }
                }

            private:
                void* _label = nullptr;
                std::shared_ptr<Flow<T>> flow_;
                std::shared_ptr<SendingCollector<T>> collector_;
                std::shared_ptr<BaseContinuationImpl> self_ref_;
            };
            kotlinx::coroutines::launch(scope, EmptyCoroutineContext::instance(), CoroutineStart::DEFAULT,
                std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)>(
                    [flow, collector](CoroutineScope*, std::shared_ptr<Continuation<void*>> completion) -> void* {
                        auto frame = std::make_shared<ChildFrame>(flow, collector, std::move(completion));
                        frame->retain();
                        return frame->start(Result<void*>::success(nullptr));
                    }));
        }
        return nullptr;
    }

private:
    std::vector<std::shared_ptr<Flow<T>>> flows_;
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
