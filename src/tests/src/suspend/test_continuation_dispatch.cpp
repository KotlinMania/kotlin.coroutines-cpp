/**
 * Transliterated from: kotlinx-coroutines-core/common/src/Yield.kt:145-166
 * Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:570-588
 * Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuation.kt:423-435
 * Continuation ABI regressions for dispatch, prompt cancellation, and lifetime.
 */
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include "kotlinx/coroutines/Yield.hpp"
#include "kotlinx/coroutines/Unconfined.hpp"
#include "kotlinx/coroutines/Dispatchers.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/dsl/CancellableReusable.hpp"
#include "kotlinx/coroutines/testing/TestBase.hpp"
#include <deque>
#include <iostream>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::testing;

class QueueDispatcher final : public CoroutineDispatcher, public Delay {
public:
    mutable std::deque<std::shared_ptr<Runnable>> queue;
    mutable int yield_calls = 0;
    bool immediate = false;
    std::shared_ptr<CancellableContinuationImpl<void>> timer;

    bool is_dispatch_needed(const CoroutineContext&) const override { return !immediate; }
    void dispatch(const CoroutineContext&, std::shared_ptr<Runnable> block) const override {
        queue.push_back(std::move(block));
    }
    void dispatch_yield(const CoroutineContext& context, std::shared_ptr<Runnable> block) const override {
        ++yield_calls;
        CoroutineDispatcher::dispatch_yield(context, std::move(block));
    }
    void schedule_resume_after_delay(long long, CancellableContinuation<void>& continuation) override {
        timer = dynamic_cast<CancellableContinuationImpl<void>&>(continuation).shared_from_this();
    }
    void fire_timer() {
        auto task = std::move(timer);
        task->resume(nullptr);
    }
    void drain() {
        int steps = 0;
        while (!queue.empty()) {
            assert_true(++steps < 100);
            auto task = std::move(queue.front());
            queue.pop_front();
            task->run();
        }
    }
};

struct RecordingContinuation final : Continuation<void*> {
    std::shared_ptr<CoroutineContext> context;
    int resumes = 0;
    Result<void*> result = Result<void*>::success(nullptr);
    explicit RecordingContinuation(std::shared_ptr<CoroutineContext> context)
        : context(std::move(context)) {}
    std::shared_ptr<CoroutineContext> get_context() const override { return context; }
    void resume_with(Result<void*> resumed) override {
        ++resumes;
        result = std::move(resumed);
    }
};

class CallFrame final : public ContinuationImpl {
public:
    std::function<void*(Continuation<void*>*)> operation;
    CallFrame(std::shared_ptr<Continuation<void*>> completion,
              std::function<void*(Continuation<void*>*)> operation)
        : ContinuationImpl(std::move(completion)), operation(std::move(operation)) {}
    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)
        coroutine_yield_value(this, result, operation(this), value_);
        return value_;
    }
private:
    void* _label = nullptr;
    void* value_ = nullptr;
};

void test_join_dispatch_retains_frame() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto parent = std::make_shared<RecordingContinuation>(dispatcher);
    auto target = JobImpl::create(nullptr);
    auto frame = std::make_shared<CallFrame>(parent, [target](auto* continuation) {
        return target->join(continuation);
    });
    std::weak_ptr<CallFrame> retained = frame;
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    target->complete();
    assert_equals(0, parent->resumes);
    assert_equals(size_t(1), dispatcher->queue.size());
    frame.reset();
    assert_false(retained.expired());
    dispatcher->drain();
    assert_equals(1, parent->resumes);
    assert_true(parent->result.is_success());
    assert_true(retained.expired());
}

void test_join_prompt_cancellation_after_completion() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto job = JobImpl::create(nullptr);
    auto context = job->operator+(dispatcher);
    auto parent = std::make_shared<RecordingContinuation>(context);
    auto target = JobImpl::create(nullptr);
    auto frame = std::make_shared<CallFrame>(parent, [target](auto* continuation) {
        return target->join(continuation);
    });
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    target->complete();
    job->cancel();
    assert_equals(0, parent->resumes);
    dispatcher->drain();
    assert_equals(1, parent->resumes);
    assert_false(parent->result.is_success());
}

void test_cancelled_join_releases_disposed_handler() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto job = JobImpl::create(nullptr);
    auto parent = std::make_shared<RecordingContinuation>(job->operator+(dispatcher));
    auto target = JobImpl::create(nullptr);
    auto frame = std::make_shared<CallFrame>(parent, [target](auto* continuation) {
        return target->join(continuation);
    });
    std::weak_ptr<CallFrame> retained = frame;
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    job->cancel();
    frame.reset();
    dispatcher->drain();
    assert_equals(1, parent->resumes);
    assert_false(parent->result.is_success());
    assert_true(retained.expired());
    target->complete(); // The removed handler cannot resume a second time.
    dispatcher->drain();
    assert_equals(1, parent->resumes);
}

void test_delay_resumes_through_dispatcher() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto parent = std::make_shared<RecordingContinuation>(dispatcher);
    auto frame = std::make_shared<CallFrame>(parent, [](auto* continuation) {
        return delay(1, continuation);
    });
    std::weak_ptr<CallFrame> retained = frame;
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    dispatcher->fire_timer();
    frame.reset();
    assert_equals(0, parent->resumes);
    assert_false(retained.expired());
    dispatcher->drain();
    assert_equals(1, parent->resumes);
    assert_true(retained.expired());
}

void test_typed_result_prompt_cancellation() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto job = JobImpl::create(nullptr);
    auto parent = std::make_shared<RecordingContinuation>(job->operator+(dispatcher));
    std::shared_ptr<CancellableContinuationImpl<int>> suspended;
    int cancelled_values = 0;
    auto frame = std::make_shared<CallFrame>(parent, [&](auto* continuation) {
        return suspend_cancellable_coroutine<int>([&](CancellableContinuation<int>& cont) {
            suspended = dynamic_cast<CancellableContinuationImpl<int>&>(cont).shared_from_this();
        }, continuation);
    });
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    auto capture = std::make_shared<int>(42);
    std::weak_ptr<int> handler_capture = capture;
    suspended->resume(42, [&, capture](std::exception_ptr) { ++cancelled_values; });
    capture.reset();
    suspended.reset();
    job->cancel();
    dispatcher->drain();
    assert_equals(1, cancelled_values);
    assert_equals(1, parent->resumes);
    assert_false(parent->result.is_success());
    assert_true(handler_capture.expired());
}

void test_typed_result_is_boxed_once_after_dispatch() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto parent = std::make_shared<RecordingContinuation>(dispatcher);
    std::shared_ptr<CancellableContinuationImpl<int>> suspended;
    auto frame = std::make_shared<CallFrame>(parent, [&](auto* continuation) {
        return suspend_cancellable_coroutine<int>([&](CancellableContinuation<int>& cont) {
            suspended = dynamic_cast<CancellableContinuationImpl<int>&>(cont).shared_from_this();
        }, continuation);
    });
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    suspended->resume(42, nullptr);
    suspended.reset();
    assert_equals(0, parent->resumes);
    dispatcher->drain();
    assert_equals(1, parent->resumes);
    std::unique_ptr<int> value(static_cast<int*>(parent->result.get_or_throw()));
    assert_equals(42, *value);
}

void test_yield_prompt_cancellation() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto job = JobImpl::create(nullptr);
    auto parent = std::make_shared<RecordingContinuation>(job->operator+(dispatcher));
    auto frame = std::make_shared<CallFrame>(parent, [](auto* continuation) {
        return yield(internal::retain_continuation(continuation));
    });
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    job->cancel();
    dispatcher->drain();
    assert_equals(1, parent->resumes);
    assert_false(parent->result.is_success());
}

void test_unconfined_yields_to_pending_task() {
    auto context = std::shared_ptr<CoroutineContext>(&Dispatchers::get_unconfined(), [](CoroutineContext*) {});
    auto parent = std::make_shared<RecordingContinuation>(context);
    int other_runs = 0;
    class OtherTask final : public SchedulerTask {
        int& runs_;
    public:
        explicit OtherTask(int& runs) : runs_(runs) {}
        void run() override { ++runs_; }
    };
    auto loop = ThreadLocalEventLoop::get_event_loop();
    loop->increment_use_count(true);
    loop->dispatch_unconfined(std::make_shared<OtherTask>(other_runs));
    auto frame = std::make_shared<CallFrame>(parent, [](auto* continuation) {
        return yield(internal::retain_continuation(continuation));
    });
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    assert_equals(0, other_runs);
    assert_equals(0, parent->resumes);
    assert_true(loop->process_unconfined_event());
    assert_equals(1, other_runs);
    assert_equals(0, parent->resumes);
    assert_true(loop->process_unconfined_event());
    assert_equals(1, parent->resumes);
    assert_false(loop->process_unconfined_event());
    loop->decrement_use_count(true);
}

void test_yield_dispatch_and_immediate_dispatch() {
    for (bool immediate : {false, true}) {
        auto dispatcher = std::make_shared<QueueDispatcher>();
        dispatcher->immediate = immediate;
        auto parent = std::make_shared<RecordingContinuation>(dispatcher);
        auto frame = std::make_shared<CallFrame>(parent, [](auto* continuation) {
            return yield(internal::retain_continuation(continuation));
        });
        assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
        assert_equals(1, dispatcher->yield_calls);
        assert_equals(0, parent->resumes);
        dispatcher->drain();
        assert_equals(1, parent->resumes);
        assert_true(parent->result.is_success());
    }
}

void test_unconfined_and_plain_continuations_do_not_suspend() {
    assert_true(&Dispatchers::get_unconfined() == &Unconfined::instance());
    auto unconfined = std::shared_ptr<CoroutineContext>(&Dispatchers::get_unconfined(), [](CoroutineContext*) {});
    auto parent = std::make_shared<RecordingContinuation>(unconfined);
    auto frame = std::make_shared<CallFrame>(parent, [](auto* continuation) {
        return yield(internal::retain_continuation(continuation));
    });
    std::weak_ptr<CallFrame> retained = frame;
    assert_false(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    frame.reset();
    assert_true(retained.expired());
    assert_equals(0, parent->resumes);
    // Upstream intercepted() leaves non-frame continuations unchanged.
    auto dispatcher = std::make_shared<QueueDispatcher>();
    parent->context = dispatcher;
    assert_false(intrinsics::is_coroutine_suspended(yield(parent)));
    assert_true(dispatcher->queue.empty());
}

class CountingInterceptor final : public AbstractCoroutineContextElement, public ContinuationInterceptor {
public:
    int interceptions = 0;
    int releases = 0;
    CountingInterceptor() : AbstractCoroutineContextElement(ContinuationInterceptor::type_key) {}
    CoroutineContext::Key* key() const override { return ContinuationInterceptor::type_key; }
    std::shared_ptr<Continuation<void*>> intercept_continuation(
        std::shared_ptr<Continuation<void*>> continuation) override {
        ++interceptions;
        return std::make_shared<FunctionalContinuation<void*>>(continuation->get_context(),
            [continuation](Result<void*> result) { continuation->resume_with(std::move(result)); });
    }
    void release_intercepted_continuation(std::shared_ptr<Continuation<void*>>) override {
        ++releases;
    }
};

void test_interception_cached_and_released_on_entry_paths() {
    for (bool fail : {false, true}) {
        auto interceptor = std::make_shared<CountingInterceptor>();
        auto parent = std::make_shared<RecordingContinuation>(interceptor);
        auto frame = std::make_shared<CallFrame>(parent, [fail](auto* continuation) -> void* {
            auto* frame = dynamic_cast<ContinuationImpl*>(continuation);
            auto first = frame->intercepted();
            assert_true(first == frame->intercepted());
            if (fail) throw std::runtime_error("initial entry failed");
            return nullptr;
        });
        std::weak_ptr<CallFrame> retained = frame;
        try {
            frame->start(Result<void*>::success(nullptr));
            assert_false(fail);
        } catch (const std::runtime_error&) {
            assert_true(fail);
        }
        assert_equals(1, interceptor->interceptions);
        assert_equals(1, interceptor->releases);
        frame.reset();
        assert_true(retained.expired());
    }
}

// Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:78-111
void test_async_suspend_value_and_start_modes() {
    for (auto start : {CoroutineStart::DEFAULT, CoroutineStart::LAZY, CoroutineStart::UNDISPATCHED}) {
        auto dispatcher = std::make_shared<QueueDispatcher>();
        auto scope = create_coroutine_scope(dispatcher);
        int entered = 0;
        class ValueFrame final : public ContinuationImpl {
            int& entered_;
            void* _label = nullptr;
        public:
            ValueFrame(int& entered, std::shared_ptr<Continuation<void*>> completion)
                : ContinuationImpl(std::move(completion)), entered_(entered) {}
            void* invoke_suspend(Result<void*> result) override {
                coroutine_begin(this)
                ++entered_;
                coroutine_yield(this, yield(shared_from_this()));
                return new int(42);
            }
        };
        std::weak_ptr<ValueFrame> retained;
        auto deferred = async<int>(scope.get(), EmptyCoroutineContext::instance(), start,
            std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)>(
                [&](CoroutineScope*, std::shared_ptr<Continuation<void*>> completion) {
                    auto frame = std::make_shared<ValueFrame>(entered, std::move(completion));
                    retained = frame;
                    return frame->start(Result<void*>::success(nullptr));
                }));
        assert_equals(start == CoroutineStart::UNDISPATCHED ? 1 : 0, entered);
        if (start == CoroutineStart::LAZY) {
            assert_false(deferred->is_active());
            assert_true(dispatcher->queue.empty());
            assert_true(deferred->start());
        }
        dispatcher->drain();
        assert_equals(1, entered);
        assert_true(deferred->is_completed());
        assert_equals(42, deferred->get_completed());
        assert_true(retained.expired());
        scope->get_job()->cancel();
    }
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto scope = create_coroutine_scope(dispatcher);
    auto deferred = async<int>(scope.get(), EmptyCoroutineContext::instance(), CoroutineStart::UNDISPATCHED,
        std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)>(
            [](CoroutineScope*, std::shared_ptr<Continuation<void*>>) -> void* { return new int(99); }));
    assert_true(deferred->is_completed());
    assert_equals(99, deferred->get_completed());
    scope->get_job()->cancel();

    // Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:32-36
    auto scope_without_dispatcher = create_coroutine_scope(EmptyCoroutineContext::instance());
    bool default_inserted = false;
    auto default_deferred = async<int>(scope_without_dispatcher.get(),
        EmptyCoroutineContext::instance(), CoroutineStart::UNDISPATCHED,
        std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)>(
            [&](CoroutineScope* receiver, std::shared_ptr<Continuation<void*>>) -> void* {
                default_inserted = receiver->get_coroutine_context()->get(
                    ContinuationInterceptor::type_key).get() == &Dispatchers::get_default();
                return new int(101);
            }));
    assert_true(default_inserted);
    assert_equals(101, default_deferred->get_completed());
    scope_without_dispatcher->get_job()->cancel();
}

void test_reusable_cache_owns_claimed_continuation() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto completion = std::make_shared<FunctionalContinuation<int>>(dispatcher, [](Result<int>) {});
    auto delegate = dispatcher->intercept_continuation<int>(completion);
    auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<int>>(delegate);
    auto first = dsl::get_or_create_cancellable_continuation<int>(delegate);
    auto* identity = first.get();
    std::weak_ptr<CancellableContinuationImpl<int>> retained = first;
    first->resume(41, nullptr);
    std::unique_ptr<int> initial(static_cast<int*>(first->get_result()));
    assert_equals(41, *initial);
    first.reset();
    assert_false(retained.expired());
    auto second = dsl::get_or_create_cancellable_continuation<int>(delegate);
    assert_true(second.get() == identity);
    assert_true(second->is_active());
    second->resume(42, nullptr);
    std::unique_ptr<int> repeated(static_cast<int*>(second->get_result()));
    assert_equals(42, *repeated);
    second.reset();
    assert_false(retained.expired());
    dispatched->release();
    assert_true(retained.expired());
    assert_false(dispatched->is_reusable());
}

void test_reusable_postponed_cancellation_preserves_first_cause() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    int completions = 0;
    std::exception_ptr failure;
    auto completion = std::make_shared<FunctionalContinuation<int>>(dispatcher, [&](Result<int> result) {
        ++completions;
        failure = result.exception_or_null();
    });
    auto delegate = dispatcher->intercept_continuation<int>(completion);
    auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<int>>(delegate);
    auto cont = dsl::get_or_create_cancellable_continuation<int>(delegate);
    std::weak_ptr<CancellableContinuationImpl<int>> retained = cont;
    auto first = std::make_exception_ptr(std::runtime_error("first postponed cancellation"));
    auto later = std::make_exception_ptr(std::runtime_error("later cancellation"));
    assert_true(dispatched->postpone_cancellation(first));
    assert_true(dispatched->postpone_cancellation(later));
    dispatched->release(); // Release must leave the postponed cause for get_result to consume.
    assert_true(dispatched->is_reusable());
    assert_true(intrinsics::is_coroutine_suspended(cont->get_result()));
    assert_false(dispatched->is_reusable());
    cont.reset();
    assert_equals(0, completions);
    dispatcher->drain();
    assert_equals(1, completions);
    assert_true(failure == first);
    assert_true(retained.expired());
    dispatched->release();
}

void test_reusable_published_cancellation_releases_cache() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto completion = std::make_shared<FunctionalContinuation<int>>(dispatcher, [](Result<int>) {});
    auto delegate = dispatcher->intercept_continuation<int>(completion);
    auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<int>>(delegate);
    auto cont = dsl::get_or_create_cancellable_continuation<int>(delegate);
    std::weak_ptr<CancellableContinuationImpl<int>> retained = cont;
    cont->resume(42, nullptr);
    std::unique_ptr<int> value(static_cast<int*>(cont->get_result()));
    cont.reset();
    assert_false(retained.expired());
    assert_false(dispatched->postpone_cancellation(std::make_exception_ptr(std::runtime_error("cancel"))));
    assert_true(retained.expired());
    assert_false(dispatched->is_reusable());
    dispatched->release();
}

void test_reusable_idempotent_resume_rejects_reset() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto completion = std::make_shared<FunctionalContinuation<int>>(dispatcher, [](Result<int>) {});
    auto delegate = dispatcher->intercept_continuation<int>(completion);
    auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<int>>(delegate);
    auto first = dsl::get_or_create_cancellable_continuation<int>(delegate);
    std::weak_ptr<CancellableContinuationImpl<int>> rejected = first;
    int idempotent = 0;
    auto token = first->try_resume(42, &idempotent, nullptr);
    assert_true(token != nullptr);
    first->complete_resume(token);
    std::unique_ptr<int> initial(static_cast<int*>(first->get_result()));
    first.reset();
    auto next = dsl::get_or_create_cancellable_continuation<int>(delegate);
    assert_true(rejected.expired());
    next->resume(43, nullptr);
    std::unique_ptr<int> value(static_cast<int*>(next->get_result()));
    assert_equals(43, *value);
    std::weak_ptr<CancellableContinuationImpl<int>> retained = next;
    next.reset();
    dispatched->release();
    assert_true(retained.expired());
}

void test_reusable_reset_releases_completed_references() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto completion = std::make_shared<FunctionalContinuation<std::shared_ptr<int>>>(
        dispatcher, [](Result<std::shared_ptr<int>>) {});
    auto delegate = dispatcher->intercept_continuation<std::shared_ptr<int>>(completion);
    auto dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<std::shared_ptr<int>>>(delegate);
    auto first = dsl::get_or_create_cancellable_continuation<std::shared_ptr<int>>(delegate);
    auto payload = std::make_shared<int>(42);
    std::weak_ptr<int> value_retained = payload;
    first->resume(payload, nullptr);
    {
        std::unique_ptr<std::shared_ptr<int>> result(
            static_cast<std::shared_ptr<int>*>(first->get_result()));
        assert_equals(42, **result);
    }
    payload.reset();
    first.reset();
    assert_false(value_retained.expired());
    auto next = dsl::get_or_create_cancellable_continuation<std::shared_ptr<int>>(delegate);
    assert_true(value_retained.expired());
    next->resume(std::make_shared<int>(43), nullptr);
    std::unique_ptr<std::shared_ptr<int>> result(static_cast<std::shared_ptr<int>*>(next->get_result()));
    assert_equals(43, **result);
    next.reset();
    dispatched->release();

    auto unit_completion = std::make_shared<FunctionalContinuation<void>>(dispatcher, [](Result<void>) {});
    auto unit_delegate = dispatcher->intercept_continuation<void>(unit_completion);
    auto unit_dispatched = std::dynamic_pointer_cast<internal::DispatchedContinuation<void>>(unit_delegate);
    auto unit_first = dsl::get_or_create_cancellable_continuation<void>(unit_delegate);
    auto capture = std::make_shared<int>(1);
    std::weak_ptr<int> handler_retained = capture;
    unit_first->invoke_on_cancellation([capture](std::exception_ptr) {});
    unit_first->resume(nullptr);
    assert_true(unit_first->get_result() == nullptr);
    capture.reset();
    unit_first.reset();
    assert_false(handler_retained.expired());
    auto unit_next = dsl::get_or_create_cancellable_continuation<void>(unit_delegate);
    assert_true(handler_retained.expired());
    unit_next->resume(nullptr);
    assert_true(unit_next->get_result() == nullptr);
    unit_next.reset();
    unit_dispatched->release();
}

void test_reusable_completion_releases_frame_cycle(bool cancel_before_dispatch) {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto job = JobImpl::create(nullptr);
    std::shared_ptr<CoroutineContext> context = cancel_before_dispatch ? job->operator+(dispatcher) : dispatcher;
    auto parent = std::make_shared<RecordingContinuation>(context);
    std::shared_ptr<CancellableContinuationImpl<void*>> suspended;
    auto frame = std::make_shared<CallFrame>(parent, [&](auto* continuation) {
        auto delegate = intrinsics::intercepted(internal::retain_continuation(continuation));
        suspended = dsl::get_or_create_cancellable_continuation<void*>(delegate);
        return suspended->get_result();
    });
    std::weak_ptr<CallFrame> retained_frame = frame;
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    std::weak_ptr<CancellableContinuationImpl<void*>> retained_cont = suspended;
    frame.reset();
    auto* payload = new int(42);
    int cancelled_values = 0;
    suspended->resume(payload, [payload, &cancelled_values](std::exception_ptr) {
        delete payload;
        ++cancelled_values;
    });
    suspended.reset();
    assert_equals(0, parent->resumes);
    assert_false(retained_frame.expired());
    if (cancel_before_dispatch) job->cancel();
    dispatcher->drain();
    assert_equals(1, parent->resumes);
    if (cancel_before_dispatch) {
        assert_false(parent->result.is_success());
        assert_equals(1, cancelled_values);
    } else {
        std::unique_ptr<int> value(static_cast<int*>(parent->result.get_or_throw()));
        assert_equals(42, *value);
        assert_equals(0, cancelled_values);
    }
    assert_true(retained_cont.expired());
    assert_true(retained_frame.expired());
}

void test_reusable_raw_wrapper_reuses_typed_delegate() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto parent = std::make_shared<RecordingContinuation>(dispatcher);
    std::weak_ptr<CancellableContinuationImpl<void>> unit;
    std::weak_ptr<CancellableContinuationImpl<int>> first;
    std::shared_ptr<CancellableContinuationImpl<int>> suspended;
    auto frame = std::make_shared<CallFrame>(parent, [&](auto* continuation) {
        auto unit_result = dsl::suspend_cancellable_coroutine_reusable<void>(continuation, [&](auto* cont) {
            unit = cont->shared_from_this();
            cont->resume(nullptr);
        });
        assert_true(unit_result == nullptr);
        assert_false(unit.expired());
        std::unique_ptr<int> initial(static_cast<int*>(
            dsl::suspend_cancellable_coroutine_reusable<int>(continuation, [&](auto* cont) {
                first = cont->shared_from_this();
                cont->resume(41, nullptr);
            })));
        assert_equals(41, *initial);
        assert_true(unit.expired()); // Changing the result type releases the prior cache.
        assert_false(first.expired());
        return dsl::suspend_cancellable_coroutine_reusable<int>(continuation, [&](auto* cont) {
            assert_true(cont == first.lock().get());
            suspended = cont->shared_from_this();
        });
    });
    std::weak_ptr<CallFrame> retained_frame = frame;
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    frame.reset();
    suspended->resume(42, nullptr);
    suspended.reset();
    assert_equals(0, parent->resumes);
    assert_equals(size_t(1), dispatcher->queue.size());
    dispatcher->drain();
    assert_equals(1, parent->resumes);
    std::unique_ptr<int> result(static_cast<int*>(parent->result.get_or_throw()));
    assert_equals(42, *result);
    assert_true(first.expired());
    assert_true(unit.expired());
    assert_true(retained_frame.expired());
}

void test_reusable_raw_wrapper_void_dispatch_and_failure(bool fail) {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto parent = std::make_shared<RecordingContinuation>(dispatcher);
    std::shared_ptr<CancellableContinuationImpl<void>> suspended;
    auto frame = std::make_shared<CallFrame>(parent, [&](auto* continuation) {
        return dsl::suspend_cancellable_coroutine_reusable_void(continuation, [&](auto* cont) {
            suspended = cont->shared_from_this();
        });
    });
    std::weak_ptr<CallFrame> retained_frame = frame;
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    std::weak_ptr<CancellableContinuationImpl<void>> retained_cont = suspended;
    frame.reset();
    auto failure = std::make_exception_ptr(std::runtime_error("reusable failure"));
    if (fail) suspended->resume_with(Result<void>::failure(failure));
    else suspended->resume(nullptr);
    suspended.reset();
    assert_equals(0, parent->resumes);
    assert_equals(size_t(1), dispatcher->queue.size());
    dispatcher->drain();
    assert_equals(1, parent->resumes);
    if (fail) assert_true(parent->result.exception_or_null() == failure);
    else assert_true(parent->result.get_or_throw() == nullptr);
    assert_true(retained_cont.expired());
    assert_true(retained_frame.expired());
}

void test_reusable_raw_wrapper_prompt_cancellation(bool cancel_in_block) {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto job = JobImpl::create(nullptr);
    auto parent = std::make_shared<RecordingContinuation>(job->operator+(dispatcher));
    std::shared_ptr<CancellableContinuationImpl<std::shared_ptr<int>>> suspended;
    int cancelled_values = 0;
    auto payload = std::make_shared<int>(42);
    std::weak_ptr<int> retained_value = payload;
    auto frame = std::make_shared<CallFrame>(parent, [&](auto* continuation) {
        return dsl::suspend_cancellable_coroutine_reusable<std::shared_ptr<int>>(continuation, [&](auto* cont) {
            suspended = cont->shared_from_this();
            if (cancel_in_block) {
                job->cancel();
                cont->resume(payload, [&](std::exception_ptr) { ++cancelled_values; });
            }
        });
    });
    std::weak_ptr<CallFrame> retained_frame = frame;
    if (cancel_in_block) {
        bool caught = false;
        try { (void)frame->start(Result<void*>::success(nullptr)); }
        catch (const CancellationException&) { caught = true; }
        assert_true(caught);
        assert_equals(0, parent->resumes);
    } else {
        assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
        suspended->resume(payload, [&](std::exception_ptr) { ++cancelled_values; });
        job->cancel();
        assert_equals(0, parent->resumes);
        dispatcher->drain();
        assert_equals(1, parent->resumes);
        assert_false(parent->result.is_success());
    }
    std::weak_ptr<CancellableContinuationImpl<std::shared_ptr<int>>> retained_cont = suspended;
    suspended.reset();
    payload.reset();
    frame.reset();
    assert_equals(1, cancelled_values);
    assert_true(retained_value.expired());
    assert_true(retained_cont.expired());
    assert_true(retained_frame.expired());
}

void test_reusable_raw_wrapper_block_failure_and_plain_completion() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto parent = std::make_shared<RecordingContinuation>(dispatcher);
    std::weak_ptr<CancellableContinuationImpl<int>> retained;
    auto frame = std::make_shared<CallFrame>(parent, [&](auto* continuation) -> void* {
        return dsl::suspend_cancellable_coroutine_reusable<int>(continuation, [&](auto* cont) {
            retained = cont->shared_from_this();
            throw std::runtime_error("block failure");
        });
    });
    std::weak_ptr<CallFrame> retained_frame = frame;
    bool caught = false;
    try { (void)frame->start(Result<void*>::success(nullptr)); }
    catch (const std::runtime_error& error) { caught = std::string(error.what()) == "block failure"; }
    assert_true(caught);
    assert_equals(0, parent->resumes);
    assert_true(retained.expired());
    frame.reset();
    assert_true(retained_frame.expired());
    std::unique_ptr<int> result(static_cast<int*>(dsl::suspend_cancellable_coroutine_reusable<int>(
        parent.get(), [&](auto* cont) {
            retained = cont->shared_from_this();
            assert_equals(MODE_CANCELLABLE, cont->resume_mode);
            cont->resume(42, nullptr);
        })));
    assert_equals(42, *result);
    assert_true(retained.expired());
    assert_equals(0, parent->resumes);
}

int main() {
    std::cerr << "test_reusable_raw_wrapper_reuses_typed_delegate\n";
    test_reusable_raw_wrapper_reuses_typed_delegate();
    std::cerr << "test_reusable_raw_wrapper_void_dispatch_and_failure\n";
    test_reusable_raw_wrapper_void_dispatch_and_failure(false);
    test_reusable_raw_wrapper_void_dispatch_and_failure(true);
    std::cerr << "test_reusable_raw_wrapper_prompt_cancellation\n";
    test_reusable_raw_wrapper_prompt_cancellation(false);
    test_reusable_raw_wrapper_prompt_cancellation(true);
    std::cerr << "test_reusable_raw_wrapper_block_failure_and_plain_completion\n";
    test_reusable_raw_wrapper_block_failure_and_plain_completion();
    std::cerr << "test_reusable_cache_owns_claimed_continuation\n";
    test_reusable_cache_owns_claimed_continuation();
    std::cerr << "test_reusable_postponed_cancellation_preserves_first_cause\n";
    test_reusable_postponed_cancellation_preserves_first_cause();
    std::cerr << "test_reusable_published_cancellation_releases_cache\n";
    test_reusable_published_cancellation_releases_cache();
    std::cerr << "test_reusable_idempotent_resume_rejects_reset\n";
    test_reusable_idempotent_resume_rejects_reset();
    std::cerr << "test_reusable_reset_releases_completed_references\n";
    test_reusable_reset_releases_completed_references();
    std::cerr << "test_reusable_completion_releases_frame_cycle\n";
    test_reusable_completion_releases_frame_cycle(false);
    test_reusable_completion_releases_frame_cycle(true);
    std::cerr << "test_join_dispatch_retains_frame\n";
    test_join_dispatch_retains_frame();
    std::cerr << "test_join_prompt_cancellation_after_completion\n";
    test_join_prompt_cancellation_after_completion();
    std::cerr << "test_delay_resumes_through_dispatcher\n";
    test_cancelled_join_releases_disposed_handler();
    test_delay_resumes_through_dispatcher();
    std::cerr << "test_typed_result_prompt_cancellation\n";
    test_typed_result_prompt_cancellation();
    std::cerr << "test_yield_dispatch_and_immediate_dispatch\n";
    test_typed_result_is_boxed_once_after_dispatch();
    test_yield_prompt_cancellation();
    test_unconfined_yields_to_pending_task();
    test_yield_dispatch_and_immediate_dispatch();
    std::cerr << "test_unconfined_and_plain_continuations_do_not_suspend\n";
    test_unconfined_and_plain_continuations_do_not_suspend();
    std::cerr << "test_interception_cached_and_released_on_entry_paths\n";
    test_interception_cached_and_released_on_entry_paths();
    test_async_suspend_value_and_start_modes();
    std::cout << "Continuation dispatch and lifetime tests passed\n";
}
