#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/flow/internal/Merge.kt
/**
 * @file Merge.hpp
 * @brief Internal flow merge operators (ChannelFlowMerge, ChannelLimitedFlowMerge, ChannelFlowTransformLatest).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt
 *
 */

#include "kotlinx/coroutines/flow/internal/ChannelFlow.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/sync/Semaphore.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/Produce.hpp" // For produce
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
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

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:23-33
void* transform_latest_emit(std::shared_ptr<Job> previous_flow,
    std::function<void()> launch_next, std::shared_ptr<Continuation<void*>> completion);
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:55-70
void* acquire_and_launch_merge_inner(std::shared_ptr<Job> job,
    std::shared_ptr<Semaphore> semaphore, std::function<void()> launch_inner,
    std::shared_ptr<Continuation<void*>> completion);
// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:64-68
void* collect_merge_child(std::function<void*(Continuation<void*>*)> collect,
    std::shared_ptr<Semaphore> semaphore, std::shared_ptr<Continuation<void*>> completion);

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:21-33
// NOTE(port): The source collector lambda implements the typed FlowCollector ABI;
// its generic definition must remain available to each element instantiation.
template <typename T, typename R>
class TransformLatestCollector final : public FlowCollector<T>,
    public std::enable_shared_from_this<TransformLatestCollector<T, R>> {
public:
    using TransformType = std::function<void*(FlowCollector<R>*, T, Continuation<void*>*)>;

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:21-23
    TransformLatestCollector(std::shared_ptr<Flow<T>> flow, TransformType transform,
                             FlowCollector<R>* collector, CoroutineScope* scope)
        : flow_(std::move(flow)), transform_(std::move(transform)),
          collector_(collector), scope_(scope) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:23-33
    [[clang::annotate("suspend")]]
    void* collect(std::shared_ptr<Continuation<void*>> completion) {
        auto owner = this->shared_from_this();
        dsl::suspend(flow_->collect(this, completion.get()));
        return nullptr;
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:23-33
    void* emit(T value, Continuation<void*>* completion) override {
        return emit(std::move(value), kotlinx::coroutines::internal::retain_continuation(completion));
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:23-33
    [[clang::annotate("suspend")]]
    void* emit(T value, std::shared_ptr<Continuation<void*>> completion) {
        auto owner = this->shared_from_this();
        if (previous_flow_) {
            previous_flow_->cancel(std::make_exception_ptr(ChildCancelledException()));
            dsl::suspend(previous_flow_->join(completion.get()));
        }
        // NOTE(port): std::function copies its closure; this owner retains the
        // actual moved element without requiring that the element be copyable.
        auto argument = std::make_shared<T>(std::move(value));
        // Do not pay for dispatch here, it's never necessary.
        previous_flow_ = kotlinx::coroutines::launch(
            scope_, EmptyCoroutineContext::instance(), CoroutineStart::UNDISPATCHED,
            std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)>(
                [transform = transform_, collector = collector_, argument](
                    CoroutineScope*, std::shared_ptr<Continuation<void*>> completion) -> void* {
                    return transform(collector, std::move(*argument), completion.get());
                }));
        return nullptr;
    }

private:
    const std::shared_ptr<Flow<T>> flow_;
    const TransformType transform_;
    FlowCollector<R>* const collector_;
    CoroutineScope* const scope_;
    std::shared_ptr<Job> previous_flow_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:9-35
template <typename T, typename R>
class ChannelFlowTransformLatest final : public ChannelFlowOperator<T, R> {
public:
    using TransformType = std::function<void*(FlowCollector<R>*, T, Continuation<void*>*)>;

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:9-15
    ChannelFlowTransformLatest(
        TransformType transform,
        std::shared_ptr<Flow<T>> flow,
        std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
        int capacity = Channel<R>::BUFFERED,
        BufferOverflow on_buffer_overflow = BufferOverflow::SUSPEND
    ) : ChannelFlowOperator<T, R>(std::move(flow), std::move(context), capacity, on_buffer_overflow),
        transform_(std::move(transform)) {}

protected:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:16-17
    ChannelFlow<R>* create(std::shared_ptr<CoroutineContext> context, int capacity, BufferOverflow on_buffer_overflow) override {
        return new ChannelFlowTransformLatest<T, R>(transform_, this->upstream(), std::move(context), capacity, on_buffer_overflow);
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:19-34
    void* flow_collect(FlowCollector<R>* collector, Continuation<void*>* continuation) override {
        return flow_collect(collector, kotlinx::coroutines::internal::retain_continuation(continuation));
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:19-34
    [[clang::annotate("suspend")]]
    void* flow_collect(FlowCollector<R>* collector,
        std::shared_ptr<Continuation<void*>> completion) {
        // So cancellation behaviour is not leaking into the downstream.
        assert((dynamic_cast<SendingCollector<R>*>(collector) != nullptr));
        auto owner = this->weak_from_this().lock();
        dsl::suspend(collect_in_scope([flow = this->upstream(), transform = transform_, collector](
            CoroutineScope* scope, std::shared_ptr<Continuation<void*>> continuation)
            __attribute__((annotate("suspend"))) -> void* {
            auto receiver = std::make_shared<TransformLatestCollector<T, R>>(flow, transform, collector, scope);
            return receiver->collect(std::move(continuation));
        }, completion.get()));
        return nullptr;
    }

private:
    const TransformType transform_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:37-74
template <typename T>
class ChannelFlowMerge final : public ChannelFlow<T> {
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

protected:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:44-45
    ChannelFlow<T>* create(std::shared_ptr<CoroutineContext> context, int capacity,
                           BufferOverflow on_buffer_overflow) override {
        return new ChannelFlowMerge<T>(flow_, concurrency_, std::move(context), capacity, on_buffer_overflow);
    }

public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:47-49
    std::shared_ptr<ReceiveChannel<T>> produce_impl(CoroutineScope* scope) override {
        return channels::produce<T>(scope, this->context(), this->capacity(),
                                    this->get_collect_to_fun());
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:51-71
    [[clang::annotate("suspend")]]
    void* collect_to(ProducerScope<T>* scope,
                     std::shared_ptr<Continuation<void*>> completion) override {
        auto owner = this->weak_from_this().lock();
        auto receiver = std::make_shared<MergeCollector>(flow_, concurrency_, scope, completion);
        dsl::suspend(receiver->collect(completion.get()));
        return nullptr;
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:73-73
    std::optional<std::string> additional_to_string_props() override {
        return "concurrency=" + std::to_string(concurrency_);
    }

private:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:51-71
    // NOTE(port): The generic FlowCollector binding is lifted out of the
    // suspend body, as Native lowering requires local declaration lifting.
    class MergeCollector final : public FlowCollector<std::shared_ptr<Flow<T>>>,
                                 public std::enable_shared_from_this<MergeCollector> {
    public:
        // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:51-54
        MergeCollector(std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> flow,
                       int concurrency, ProducerScope<T>* scope,
                       const std::shared_ptr<Continuation<void*>>& completion)
            : flow_(std::move(flow)), scope_(scope), semaphore_(create_semaphore(concurrency)),
              collector_(std::make_shared<SendingCollector<T>>(scope)),
              job_(std::dynamic_pointer_cast<Job>(completion->get_context()->get(Job::type_key))) {}

        // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:55-70
        void* collect(Continuation<void*>* continuation) {
            return flow_->collect(this, continuation);
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:55-70
        void* emit(std::shared_ptr<Flow<T>> inner, Continuation<void*>* completion) override {
            return emit(std::move(inner), kotlinx::coroutines::internal::retain_continuation(completion));
        }

        // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:55-70
        [[clang::annotate("suspend")]]
        void* emit(std::shared_ptr<Flow<T>> inner, std::shared_ptr<Continuation<void*>> completion) {
            auto owner = this->shared_from_this();
            /*
             * We launch a coroutine on each emitted element and the only potential
             * suspension point in this collector is semaphore.acquire that rarely suspends,
             * so we manually check for cancellation to propagate it to the upstream in time.
             */
            if (job_) ensure_active(*job_);
            dsl::suspend(semaphore_->acquire(completion.get()));
            kotlinx::coroutines::launch(scope_, EmptyCoroutineContext::instance(), CoroutineStart::DEFAULT,
                std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)>(
                    [inner = std::move(inner), collector = collector_, semaphore = semaphore_](
                        CoroutineScope*, std::shared_ptr<Continuation<void*>> completion)
                        __attribute__((annotate("suspend"))) -> void* {
                        try {
                            dsl::suspend(inner->collect(collector.get(), completion.get()));
                        } catch (...) {
                            semaphore->release(); // Release concurrency permit
                            throw;
                        }
                        semaphore->release(); // Release concurrency permit
                        return nullptr;
                    }));
            return nullptr;
        }

    private:
        const std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> flow_;
        ProducerScope<T>* const scope_;
        const std::shared_ptr<Semaphore> semaphore_;
        const std::shared_ptr<SendingCollector<T>> collector_;
        const std::shared_ptr<Job> job_;
    };

    const std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> flow_;
    const int concurrency_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:76-95
template <typename T>
class ChannelLimitedFlowMerge final : public ChannelFlow<T> {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:76-81
    ChannelLimitedFlowMerge(
        std::vector<std::shared_ptr<Flow<T>>> flows,
        std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
        int capacity = Channel<T>::BUFFERED,
        BufferOverflow on_buffer_overflow = BufferOverflow::SUSPEND)
        : ChannelFlow<T>(std::move(context), capacity, on_buffer_overflow), flows_(std::move(flows)) {}

protected:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:82-83
    ChannelFlow<T>* create(std::shared_ptr<CoroutineContext> context, int capacity,
                           BufferOverflow on_buffer_overflow) override {
        return new ChannelLimitedFlowMerge<T>(flows_, std::move(context), capacity, on_buffer_overflow);
    }

public:
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
            kotlinx::coroutines::launch(scope, EmptyCoroutineContext::instance(), CoroutineStart::DEFAULT,
                std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)>(
                    [flow, collector](CoroutineScope*, std::shared_ptr<Continuation<void*>> completion) -> void* {
                        return flow->collect(collector.get(), completion.get());
                    }));
        }
        return nullptr;
    }

private:
    const std::vector<std::shared_ptr<Flow<T>>> flows_;
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
