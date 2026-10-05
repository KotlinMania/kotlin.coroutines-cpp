#pragma once
// port-lint: source flow/internal/Merge.kt
/**
 * @file Merge.hpp
 * @brief Internal flow merge operators (ChannelFlowMerge, ChannelLimitedFlowMerge, ChannelFlowTransformLatest).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt
 *
 * The structured-concurrency semantics here ride on the project-wide Continuation ABI;
 * each `collect` site returns `void*` so the eventual Clang-plugin state machines can
 * slot in unchanged. Until the plugin lands, the suspension handling in the bodies
 * routes COROUTINE_SUSPENDED through the outer caller's continuation explicitly.
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
#include <mutex>
#include <atomic>
#include <thread>
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

class NoopContinuation : public Continuation<void*> {
public:
    explicit NoopContinuation(std::shared_ptr<CoroutineContext> context) : context_(std::move(context)) {}

    std::shared_ptr<CoroutineContext> get_context() const override { return context_; }
    void resume_with(Result<void*> result) override {}

private:
    std::shared_ptr<CoroutineContext> context_;
};

// Helper declarations
std::string format_concurrency_props(int concurrency);
void acquire_semaphore_permit(Job* job, kotlinx::coroutines::sync::Semaphore& semaphore);
void release_semaphore_permit(kotlinx::coroutines::sync::Semaphore& semaphore);

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
        int capacity = Channel<R>::OPTIONAL_CHANNEL,
        BufferOverflow on_buffer_overflow = BufferOverflow::SUSPEND
    ) : ChannelFlowOperator<T, R>(std::move(flow), std::move(context), capacity, on_buffer_overflow),
        transform_(std::move(transform)) {}

    ChannelFlow<R>* create(std::shared_ptr<CoroutineContext> context, int capacity, BufferOverflow on_buffer_overflow) override {
        return new ChannelFlowTransformLatest<T, R>(transform_, this->upstream(), std::move(context), capacity, on_buffer_overflow);
    }

protected:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt:19-34
    void* flow_collect(FlowCollector<R>* collector, Continuation<void*>* continuation) override {
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

template <typename T>
class ChannelFlowMerge : public ChannelFlow<T> {
public:
    ChannelFlowMerge(
        std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> flow,
        int concurrency,
        std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
        int capacity = Channel<T>::BUFFERED,
        BufferOverflow on_buffer_overflow = BufferOverflow::SUSPEND
    ) : ChannelFlow<T>(context, capacity, on_buffer_overflow), flow_(flow), concurrency_(concurrency) {}

    ChannelFlow<T>* create(std::shared_ptr<CoroutineContext> context, int capacity, BufferOverflow on_buffer_overflow) override {
        return new ChannelFlowMerge<T>(flow_, concurrency_, context, capacity, on_buffer_overflow);
    }

    std::shared_ptr<ReceiveChannel<T>> produce_impl(CoroutineScope* scope) override {
        // Kotlin:
        // override fun produceImpl(scope: CoroutineScope): ReceiveChannel<T> =
        //     scope.produce(context, capacity, block = collectToFun)
        return channels::produce<T>(
            scope,
            this->context(),
            this->capacity(),
            this->on_buffer_overflow(),
            CoroutineStart::DEFAULT,
            [this](ProducerScope<T>* scope) { collect_to(scope); }
        );
    }

    void collect_to(ProducerScope<T>* scope) override {
        // Kotlin (suspend):
        // val semaphore = Semaphore(concurrency)
        // val collector = SendingCollector(scope)
        // val job: Job? = coroutineContext[Job]
        // flow.collect { inner ->
        //   job?.ensureActive()
        //   semaphore.acquire()
        //   scope.launch {
        //     try { inner.collect(collector) } finally { semaphore.release() }
        //   }
        // }

        auto semaphore = create_semaphore(concurrency_);
        SendingCollector<T> collector(scope);

        std::shared_ptr<Job> job;
        if (auto element = scope->get_coroutine_context()->get(Job::type_key)) {
            job = std::dynamic_pointer_cast<Job>(element);
        }

        std::mutex threads_mutex;
        std::vector<std::thread> threads;

        std::mutex exception_mutex;
        std::exception_ptr first_exception = nullptr;

        class OuterCollector : public FlowCollector<std::shared_ptr<Flow<T>>> {
        public:
            OuterCollector(ProducerScope<T>* scope,
                           SendingCollector<T>* collector,
                           std::shared_ptr<Job> job,
                           std::shared_ptr<Semaphore> semaphore,
                           std::vector<std::thread>* threads,
                           std::mutex* threads_mutex,
                           std::exception_ptr* first_exception,
                           std::mutex* exception_mutex)
                : scope_(scope),
                  collector_(collector),
                  job_(std::move(job)),
                  semaphore_(std::move(semaphore)),
                  threads_(threads),
                  threads_mutex_(threads_mutex),
                  first_exception_(first_exception),
                  exception_mutex_(exception_mutex) {}

            void* emit(std::shared_ptr<Flow<T>> inner, Continuation<void*>* cont) override {
                if (job_) ensure_active(*job_);
                semaphore_->acquire();

                auto ctx = scope_->get_coroutine_context();
                auto collector_ptr = collector_;
                auto semaphore = semaphore_;
                auto first_exception = first_exception_;
                auto exception_mutex = exception_mutex_;

                std::thread t([inner = std::move(inner), collector_ptr, semaphore, ctx, first_exception, exception_mutex]() mutable {
                    try {
                        NoopContinuation noop(ctx);
                        // NoopContinuation drives any suspension to completion on this thread —
                        // the inner collect returns either nullptr (eager finish) or
                        // COROUTINE_SUSPENDED (the noop continuation will run the rest).
                        void* r = inner->collect(collector_ptr, &noop);
                        (void)r;
                    } catch (...) {
                        std::lock_guard<std::mutex> lock(*exception_mutex);
                        if (!*first_exception) *first_exception = std::current_exception();
                    }
                    semaphore->release();
                });

                {
                    std::lock_guard<std::mutex> lock(*threads_mutex_);
                    threads_->push_back(std::move(t));
                }

                return nullptr;
            }

        private:
            ProducerScope<T>* scope_;
            SendingCollector<T>* collector_;
            std::shared_ptr<Job> job_;
            std::shared_ptr<Semaphore> semaphore_;
            std::vector<std::thread>* threads_;
            std::mutex* threads_mutex_;
            std::exception_ptr* first_exception_;
            std::mutex* exception_mutex_;
        };

        OuterCollector outer(scope, &collector, job, semaphore, &threads, &threads_mutex, &first_exception, &exception_mutex);
        NoopContinuation noop(scope->get_coroutine_context());
        // NoopContinuation owns the suspension drive — the outer collect either returns
        // immediately (eager finish) or COROUTINE_SUSPENDED, with the noop continuation
        // running the remainder before the join loop below.
        void* outer_result = flow_->collect(&outer, &noop);
        (void)outer_result;

        for (auto& t : threads) {
            if (t.joinable()) t.join();
        }

        if (first_exception) {
            std::rethrow_exception(first_exception);
        }
    }

    std::string additional_to_string_props() override {
        return format_concurrency_props(concurrency_);
    }

private:
    std::shared_ptr<Flow<std::shared_ptr<Flow<T>>>> flow_;
    int concurrency_;
};



template <typename T>
class ChannelLimitedFlowMerge : public ChannelFlow<T> {
public:
    ChannelLimitedFlowMerge(
        std::vector<std::shared_ptr<Flow<T>>> flows,
        std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance(),
        int capacity = Channel<T>::BUFFERED,
        BufferOverflow on_buffer_overflow = BufferOverflow::SUSPEND
    ) : ChannelFlow<T>(context, capacity, on_buffer_overflow), flows_(flows) {}

    ChannelFlow<T>* create(std::shared_ptr<CoroutineContext> context, int capacity, BufferOverflow on_buffer_overflow) override {
        return new ChannelLimitedFlowMerge<T>(flows_, context, capacity, on_buffer_overflow);
    }

    std::shared_ptr<ReceiveChannel<T>> produce_impl(CoroutineScope* scope) override {
        // Kotlin:
        // override fun produceImpl(scope: CoroutineScope): ReceiveChannel<T> =
        //     scope.produce(context, capacity, block = collectToFun)
        return channels::produce<T>(
            scope,
            this->context(),
            this->capacity(),
            this->on_buffer_overflow(),
            CoroutineStart::DEFAULT,
            [this](ProducerScope<T>* scope) { collect_to(scope); }
        );
    }

    void collect_to(ProducerScope<T>* scope) override {
        // Kotlin (suspend):
        // val collector = SendingCollector(scope)
        // flows.forEach { flow -> scope.launch { flow.collect(collector) } }
        SendingCollector<T> collector(scope);

        std::mutex exception_mutex;
        std::exception_ptr first_exception = nullptr;

        std::vector<std::thread> threads;
        threads.reserve(flows_.size());
        auto ctx = scope->get_coroutine_context();

        for (auto& flow : flows_) {
            threads.emplace_back([flow, &collector, ctx, &first_exception, &exception_mutex]() {
                try {
                    NoopContinuation noop(ctx);
                    // The NoopContinuation drives any suspension to completion on this
                    // worker thread; the join loop below waits for all workers, so a
                    // suspended collect resumes before this scope unwinds.
                    void* r = flow->collect(&collector, &noop);
                    (void)r;
                } catch (...) {
                    std::lock_guard<std::mutex> lock(exception_mutex);
                    if (!first_exception) first_exception = std::current_exception();
                }
            });
        }

        for (auto& t : threads) {
            if (t.joinable()) t.join();
        }

        if (first_exception) {
            std::rethrow_exception(first_exception);
        }
    }

private:
    std::vector<std::shared_ptr<Flow<T>>> flows_;
};

} // namespace internal
} // namespace flow
} // namespace coroutines
} // namespace kotlinx
