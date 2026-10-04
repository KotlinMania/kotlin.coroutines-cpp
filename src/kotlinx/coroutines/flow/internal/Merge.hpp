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
struct TransformLatestFrame : public FlowCollector<T>,
                              public Continuation<void*>,
                              public std::enable_shared_from_this<TransformLatestFrame<T, R>> {
    using TransformType = std::function<void*(FlowCollector<R>*, T, Continuation<void*>*)>;

    std::shared_ptr<Flow<T>> flow_;
    TransformType transform_;
    FlowCollector<R>* collector_;
    std::shared_ptr<FlowCoroutine<void*>> scope_;
    Continuation<void*>* completion_;
    std::shared_ptr<Job> previous_flow_{nullptr};

    std::recursive_mutex mutex_;
    std::atomic<bool> upstream_completed_{false};
    std::atomic<bool> completed_{false};
    std::exception_ptr failure_{nullptr};
    std::shared_ptr<TransformLatestFrame<T, R>> self_ref_{nullptr};

    TransformLatestFrame(
        std::shared_ptr<Flow<T>> flow,
        TransformType transform,
        FlowCollector<R>* collector,
        std::shared_ptr<FlowCoroutine<void*>> scope,
        Continuation<void*>* completion
    ) : flow_(std::move(flow)),
        transform_(std::move(transform)),
        collector_(collector),
        scope_(std::move(scope)),
        completion_(completion) {}

    void retain_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (!completed_.load()) {
            self_ref_ = this->shared_from_this();
        }
    }

    void release_self() {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        self_ref_ = nullptr;
    }

    bool is_completed() const {
        return completed_.load();
    }

    bool has_failure() const {
        return failure_ != nullptr;
    }

    std::exception_ptr get_failure() const {
        return failure_;
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        if (completion_) return completion_->get_context();
        if (scope_) return scope_->get_coroutine_context();
        return EmptyCoroutineContext::instance();
    }

    void on_child_failed(std::exception_ptr cause) {
        if (!cause) return;
        try {
            std::rethrow_exception(cause);
        } catch (const ChildCancelledException&) {
            return;
        } catch (const CancellationException&) {
            return;
        } catch (...) {
            // Non-cancellation failure in transform or collector
        }

        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (!failure_) {
            failure_ = cause;
        }
        if (scope_) {
            scope_->cancel(cause);
        }
    }

    void* emit(T value, Continuation<void*>* cont) override {
        std::shared_ptr<Job> prev_to_cancel;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            if (failure_) {
                std::rethrow_exception(failure_);
            }
            if (scope_ && !scope_->is_active()) {
                std::rethrow_exception(scope_->get_cancellation_exception());
            }
            prev_to_cancel = previous_flow_;
        }

        auto self = this->shared_from_this();

        auto launch_next = [self, value = std::move(value)]() mutable {
            std::lock_guard<std::recursive_mutex> lock(self->mutex_);
            if (self->failure_ || (self->scope_ && !self->scope_->is_active())) {
                return;
            }

            auto child_ctx = self->scope_->get_coroutine_context();
            auto child_coro = std::make_shared<AbstractCoroutine<void*>>(child_ctx, true, true);
            child_coro->init_parent_job_if_needed();
            self->previous_flow_ = child_coro;

            class ChildContinuation : public Continuation<void*> {
                std::shared_ptr<TransformLatestFrame<T, R>> frame_;
                std::shared_ptr<AbstractCoroutine<void*>> coro_;
            public:
                ChildContinuation(std::shared_ptr<TransformLatestFrame<T, R>> frame,
                                  std::shared_ptr<AbstractCoroutine<void*>> coro)
                    : frame_(std::move(frame)), coro_(std::move(coro)) {}

                std::shared_ptr<CoroutineContext> get_context() const override {
                    return coro_ ? coro_->get_coroutine_context() : EmptyCoroutineContext::instance();
                }

                void resume_with(Result<void*> result) override {
                    if (!result.is_success()) {
                        frame_->on_child_failed(result.exception_or_null());
                    }
                    if (coro_) {
                        coro_->resume_with(result);
                    }
                }
            };

            auto child_cont = std::make_shared<ChildContinuation>(self, child_coro);
            child_coro->invoke_on_completion(true, true, [child_cont](std::exception_ptr) {});

            std::function<void*(CoroutineScope*)> child_block = [self, val = std::move(value), child_cont](CoroutineScope*) -> void* {
                try {
                    void* r = self->transform_(self->collector_, std::move(val), child_cont.get());
                    return r;
                } catch (...) {
                    self->on_child_failed(std::current_exception());
                    return nullptr;
                }
            };

            child_coro->start(CoroutineStart::UNDISPATCHED, static_cast<CoroutineScope*>(child_coro.get()), child_block);
        };

        if (prev_to_cancel && prev_to_cancel->is_active()) {
            prev_to_cancel->cancel(std::make_exception_ptr(ChildCancelledException()));
            if (!cont) {
                auto loop = ThreadLocalEventLoop::current_or_null();
                while (prev_to_cancel->is_active()) {
                    if (loop && !loop->is_empty()) {
                        loop->process_next_event();
                    } else {
                        std::this_thread::yield();
                    }
                }
                launch_next();
                return nullptr;
            }

            auto executed = std::make_shared<std::atomic<bool>>(false);
            auto is_async = std::make_shared<std::atomic<bool>>(false);
            prev_to_cancel->invoke_on_completion(true, true, [launch_next, cont, executed, is_async](std::exception_ptr) mutable {
                if (!executed->exchange(true)) {
                    launch_next();
                }
                if (is_async->load() && cont) {
                    cont->resume_with(Result<void*>::success(nullptr));
                }
            });
            if (!prev_to_cancel->is_active()) {
                if (!executed->exchange(true)) {
                    launch_next();
                }
                return nullptr;
            }
            is_async->store(true);
            return intrinsics::get_COROUTINE_SUSPENDED();
        } else {
            launch_next();
            return nullptr;
        }
    }

    void resume_with(Result<void*> collect_res) override {
        std::shared_ptr<Job> last_job;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            upstream_completed_.store(true);
            if (!collect_res.is_success() && !failure_) {
                failure_ = collect_res.exception_or_null();
            }
            last_job = previous_flow_;
        }

        if (failure_) {
            if (last_job) {
                last_job->cancel(failure_);
            }
            if (scope_) {
                scope_->cancel(failure_);
            }
            complete_terminal(failure_);
            return;
        }

        if (last_job && last_job->is_active()) {
            auto self = this->shared_from_this();
            last_job->invoke_on_completion(true, true, [self](std::exception_ptr child_cause) {
                if (child_cause) {
                    try {
                        std::rethrow_exception(child_cause);
                    } catch (const ChildCancelledException&) {
                        // Expected
                    } catch (const CancellationException&) {
                        // Expected
                    } catch (...) {
                        self->complete_terminal(child_cause);
                        return;
                    }
                }
                self->complete_terminal(nullptr);
            });
        } else {
            complete_terminal(nullptr);
        }
    }

    void complete_terminal(std::exception_ptr error) {
        Continuation<void*>* comp = nullptr;
        std::shared_ptr<TransformLatestFrame<T, R>> guard;
        {
            std::lock_guard<std::recursive_mutex> lock(mutex_);
            if (completed_.exchange(true)) return;
            if (!failure_ && error) {
                failure_ = error;
            }
            comp = completion_;
            completion_ = nullptr;
            guard = std::move(self_ref_);
            self_ref_ = nullptr;
        }

        if (scope_) {
            if (failure_) {
                scope_->resume_with(Result<void*>::failure(failure_));
            } else if (scope_->is_active()) {
                scope_->resume_with(Result<void*>::success(nullptr));
            }
        }

        if (comp) {
            if (failure_) {
                comp->resume_with(Result<void*>::failure(failure_));
            } else {
                comp->resume_with(Result<void*>::success(nullptr));
            }
        }
    }
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
        auto ctx = continuation ? continuation->get_context() : EmptyCoroutineContext::instance();
        auto flow_scope = std::make_shared<FlowCoroutine<void*>>(ctx, nullptr);
        flow_scope->init_parent_job_if_needed();

        auto frame = std::make_shared<TransformLatestFrame<T, R>>(
            this->upstream(),
            transform_,
            collector,
            flow_scope,
            continuation
        );

        if (!continuation) {
            // Synchronous collection mode (e.g. called from collect_to)
            void* result = this->upstream()->collect(frame.get(), nullptr);
            if (result != intrinsics::get_COROUTINE_SUSPENDED()) {
                std::shared_ptr<Job> last_job;
                {
                    std::lock_guard<std::recursive_mutex> lock(frame->mutex_);
                    last_job = frame->previous_flow_;
                }
                if (last_job && last_job->is_active()) {
                    last_job->join_blocking();
                }
                if (frame->has_failure()) {
                    std::rethrow_exception(frame->get_failure());
                }
                return nullptr;
            }
            return result;
        }

        // Asynchronous continuation mode
        frame->retain_self();
        void* result = nullptr;
        try {
            result = this->upstream()->collect(frame.get(), frame.get());
        } catch (...) {
            frame->resume_with(Result<void*>::failure(std::current_exception()));
            if (frame->is_completed() && frame->has_failure()) {
                std::rethrow_exception(frame->get_failure());
            }
            return nullptr;
        }
        if (intrinsics::is_coroutine_suspended(result)) {
            return intrinsics::get_COROUTINE_SUSPENDED();
        }

        frame->resume_with(Result<void*>::success(nullptr));
        if (frame->is_completed()) {
            if (frame->has_failure()) {
                std::rethrow_exception(frame->get_failure());
            }
            return nullptr;
        }

        return intrinsics::get_COROUTINE_SUSPENDED();
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
