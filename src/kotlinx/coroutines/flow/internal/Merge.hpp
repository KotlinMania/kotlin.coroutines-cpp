#pragma once
/**
 * @file Merge.hpp
 * @brief Internal flow merge operators (ChannelFlowMerge, ChannelLimitedFlowMerge, ChannelFlowTransformLatest).
 *
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/Merge.kt
 *
 * TODO(semantics): This file still uses blocking threads where Kotlin would use suspend + structured concurrency.
 * TODO(suspend-plugin): Migrate to plugin-generated state machines for true suspend semantics.
 */

#include "kotlinx/coroutines/flow/internal/ChannelFlow.hpp"
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/flow/internal/FlowExceptions.hpp"
#include "kotlinx/coroutines/sync/Semaphore.hpp"
#include "kotlinx/coroutines/Job.hpp"
#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/CoroutineScope.hpp"
#include "kotlinx/coroutines/channels/Channel.hpp"
#include "kotlinx/coroutines/channels/Produce.hpp" // For produce
#include "kotlinx/coroutines/flow/internal/SendingCollector.hpp" // For SendingCollector
#include "kotlinx/coroutines/intrinsics/Intrinsics.hpp"
#include <memory>
#include <functional>
#include <mutex>
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

    ChannelFlow<R>* create(std::shared_ptr<CoroutineContext> context, int capacity, BufferOverflow on_buffer_overflow) override {
        return new ChannelFlowTransformLatest<T, R>(transform_, this->upstream(), std::move(context), capacity, on_buffer_overflow);
    }

protected:
    void* flow_collect(FlowCollector<R>* collector, Continuation<void*>* continuation) override {
        // Kotlin:
        // coroutineScope {
        //   var previousFlow: Job? = null
        //   flow.collect { value ->
        //     previousFlow?.apply { cancel(ChildCancelledException()); join() }
        //     previousFlow = launch(start = UNDISPATCHED) { collector.transform(value) }
        //   }
        // }

        // We don't model coroutineScope here yet; emulate the structure with a local scope and blocking joins.
        class SimpleScope : public CoroutineScope {
        public:
            explicit SimpleScope(std::shared_ptr<CoroutineContext> ctx) : ctx_(std::move(ctx)) {}
            std::shared_ptr<CoroutineContext> get_coroutine_context() const override { return ctx_; }

        private:
            std::shared_ptr<CoroutineContext> ctx_;
        };

        auto ctx = continuation ? continuation->get_context() : EmptyCoroutineContext::instance();
        SimpleScope scope(ctx);

        std::shared_ptr<Job> previous_flow;

        class ValueCollector : public FlowCollector<T> {
        public:
            ValueCollector(SimpleScope* scope,
                           FlowCollector<R>* collector,
                           TransformType transform,
                           std::shared_ptr<Job>* previous_flow)
                : scope_(scope), collector_(collector), transform_(std::move(transform)), previous_flow_(previous_flow) {}

            void* emit(T value, Continuation<void*>* cont) override {
                if (*previous_flow_) {
                    (*previous_flow_)->cancel(std::make_exception_ptr(ChildCancelledException()));
                    (*previous_flow_)->join_blocking();
                }

                // Do not pay for dispatch here, it's never necessary.
                *previous_flow_ = kotlinx::coroutines::launch(
                    scope_,
                    nullptr,
                    CoroutineStart::UNDISPATCHED,
                    [collector = collector_, transform = transform_, value = std::move(value)](CoroutineScope* scope) mutable {
                        NoopContinuation noop(scope->get_coroutine_context());
                        void* r = transform(collector, std::move(value), &noop);
                        (void)r; // TODO(semantics): handle COROUTINE_SUSPENDED from transform.
                    }
                );

                return nullptr;
            }

        private:
            SimpleScope* scope_;
            FlowCollector<R>* collector_;
            TransformType transform_;
            std::shared_ptr<Job>* previous_flow_;
        };

        ValueCollector value_collector(&scope, collector, transform_, &previous_flow);
        void* result = this->upstream()->collect(&value_collector, continuation);

        // coroutineScope waits for children; approximate by joining the last one when upstream completes synchronously.
        if (result != intrinsics::get_COROUTINE_SUSPENDED() && previous_flow) {
            previous_flow->join_blocking();
        }

        return result;
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

        // Semaphore is an interface; wrap the factory result in a shared_ptr for use across threads.
        auto semaphore = std::shared_ptr<Semaphore>(create_semaphore(concurrency_), [](Semaphore* s) { delete s; });
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
                        void* r = inner->collect(collector_ptr, &noop);
                        (void)r; // TODO(semantics): handle COROUTINE_SUSPENDED from inner.collect.
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
        void* outer_result = flow_->collect(&outer, &noop);
        (void)outer_result; // TODO(semantics): handle COROUTINE_SUSPENDED from outer flow collection.

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
                    void* r = flow->collect(&collector, &noop);
                    (void)r; // TODO(semantics): handle COROUTINE_SUSPENDED from flow.collect.
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
