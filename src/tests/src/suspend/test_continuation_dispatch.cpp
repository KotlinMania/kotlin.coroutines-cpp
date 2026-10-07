/**
 * Transliterated from: kotlinx-coroutines-core/common/src/Yield.kt:145-166
 * Transliterated from: kotlinx-coroutines-core/common/src/JobSupport.kt:570-588
 * Transliterated from: kotlinx-coroutines-core/common/src/CancellableContinuation.kt:423-435
 * Continuation ABI regressions for dispatch, prompt cancellation, and lifetime.
 */
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include "kotlinx/coroutines/Builders.hpp"
#include "kotlinx/coroutines/CompletableDeferred.hpp"
#include "kotlinx/coroutines/Yield.hpp"
#include "kotlinx/coroutines/Unconfined.hpp"
#include "kotlinx/coroutines/Dispatchers.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/dsl/CancellableReusable.hpp"
#include "kotlinx/coroutines/channels/BufferedChannel.hpp"
#include "kotlinx/coroutines/testing/TestBase.hpp"
#include <deque>
#include <iostream>
#include <thread>

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

class RestrictedResultFrame final : public RestrictedContinuationImpl {
public:
    explicit RestrictedResultFrame(std::shared_ptr<Continuation<void*>> completion)
        : RestrictedContinuationImpl(std::move(completion)) {}
    void* invoke_suspend(Result<void*> result) override { return result.get_or_throw(); }
};

class ContextResultFrame final : public ContinuationImpl {
public:
    using ContinuationImpl::ContinuationImpl;
    int invocations = 0;
    void* invoke_suspend(Result<void*> result) override {
        ++invocations;
        return result.get_or_throw();
    }
};

void test_continuation_required_values() {
    auto parent_context = std::make_shared<QueueDispatcher>();
    auto parent = std::make_shared<RecordingContinuation>(parent_context);
    auto inherited = std::make_shared<ContextResultFrame>(parent);
    assert_true(inherited->get_context() == parent_context);
    auto explicit_context = EmptyCoroutineContext::instance();
    auto explicit_frame = std::make_shared<ContextResultFrame>(parent, explicit_context);
    assert_true(explicit_frame->get_context() == explicit_context);
    for (auto frame : {std::make_shared<ContextResultFrame>(nullptr),
                       std::make_shared<ContextResultFrame>(parent, nullptr)}) {
        bool rejected = false;
        try { frame->get_context(); }
        catch (const std::logic_error&) { rejected = true; }
        assert_true(rejected);
    }
    auto no_completion = std::make_shared<ContextResultFrame>(nullptr, explicit_context);
    bool rejected = false;
    try { no_completion->resume_with(Result<void*>::success(nullptr)); }
    catch (const std::logic_error&) { rejected = true; }
    assert_true(rejected);
    assert_equals(0, no_completion->invocations);
    auto restricted = std::make_shared<RestrictedResultFrame>(nullptr);
    rejected = false;
    try { restricted->resume_with(Result<void*>::success(nullptr)); }
    catch (const std::logic_error&) { rejected = true; }
    assert_true(rejected);
    inherited->resume_with(Result<void*>::success(nullptr));
    assert_equals(1, inherited->invocations);
    assert_equals(1, parent->resumes);
}

void test_restricted_continuation_context() {
    auto parent = std::make_shared<RecordingContinuation>(EmptyCoroutineContext::instance());
    for (auto completion : {std::shared_ptr<Continuation<void*>>{}, std::static_pointer_cast<Continuation<void*>>(parent)}) {
        auto frame = std::make_shared<RestrictedResultFrame>(completion);
        assert_true(frame->get_context() == EmptyCoroutineContext::instance());
        std::unique_ptr<int> value(static_cast<int*>(frame->start(Result<void*>::success(new int(42)))));
        assert_equals(42, *value);
    }
    assert_equals(0, parent->resumes);
    auto nonempty = std::make_shared<RecordingContinuation>(std::make_shared<QueueDispatcher>());
    bool rejected = false;
    try { auto frame = std::make_shared<RestrictedResultFrame>(nonempty); }
    catch (const std::invalid_argument& error) {
        rejected = true;
        assert_equals(std::string("Coroutines with restricted suspension must have EmptyCoroutineContext"), std::string(error.what()));
    }
    assert_true(rejected);
}

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
        auto parent = std::make_shared<RecordingContinuation>(dispatcher);
        auto await_frame = std::make_shared<CallFrame>(parent, [deferred](auto* continuation) {
            return deferred->await(continuation);
        });
        std::unique_ptr<int> awaited(static_cast<int*>(await_frame->start(Result<void*>::success(nullptr))));
        assert_equals(42, *awaited);
        assert_equals(0, parent->resumes);
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

void test_channel_sender_dispatch_and_cancellation(int capacity, int cancel_mode) {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto job = JobImpl::create(nullptr);
    std::shared_ptr<CoroutineContext> context = cancel_mode ? job->operator+(dispatcher) : dispatcher;
    auto parent = std::make_shared<RecordingContinuation>(context);
    std::vector<int> undelivered;
    auto channel = std::make_shared<channels::BufferedChannel<std::shared_ptr<int>>>(
        capacity, [&](auto value) { undelivered.push_back(*value); });
    if (capacity) assert_true(channel->try_send(std::make_shared<int>(0)).is_success());
    auto value = std::make_shared<int>(42);
    std::weak_ptr<int> retained_value = value;
    auto frame = std::make_shared<CallFrame>(parent, [channel, value](auto* continuation) {
        return channel->send(value, continuation);
    });
    std::weak_ptr<CallFrame> retained_frame = frame;
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    frame.reset();
    value.reset();
    assert_false(retained_frame.expired());
    if (cancel_mode == 1) job->cancel();
    if (capacity) {
        auto seed = channel->try_receive();
        assert_true(seed.is_success());
        assert_equals(0, *seed.get_or_throw());
    }
    {
        auto received = channel->try_receive();
        if (cancel_mode == 1) assert_true(received.is_failure());
        else {
            assert_true(received.is_success());
            assert_equals(42, *received.get_or_throw());
        }
    }
    if (cancel_mode == 2) job->cancel();
    assert_equals(0, parent->resumes);
    assert_equals(size_t(1), dispatcher->queue.size());
    dispatcher->drain();
    assert_equals(1, parent->resumes);
    if (cancel_mode) assert_false(parent->result.is_success());
    else assert_true(parent->result.get_or_throw() == nullptr);
    assert_true(undelivered == (cancel_mode == 1 ? std::vector<int>({42}) : std::vector<int>{}));
    assert_true(retained_frame.expired(), "completed channel sender frame retained");
    assert_true(retained_value.expired(), "completed channel sender value retained");
}

void test_channel_receiver_dispatch_and_cancellation(bool catching, int capacity, int cancel_mode) {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto job = JobImpl::create(nullptr);
    std::shared_ptr<CoroutineContext> context = cancel_mode ? job->operator+(dispatcher) : dispatcher;
    auto parent = std::make_shared<RecordingContinuation>(context);
    std::vector<int> undelivered;
    auto channel = std::make_shared<channels::BufferedChannel<std::shared_ptr<int>>>(
        capacity, [&](auto value) { undelivered.push_back(*value); });
    auto frame = std::make_shared<CallFrame>(parent, [channel, catching](auto* continuation) {
        return catching ? channel->receive_catching(continuation) : channel->receive(continuation);
    });
    std::weak_ptr<CallFrame> retained_frame = frame;
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    frame.reset();
    assert_false(retained_frame.expired());
    if (cancel_mode == 1) {
        job->cancel();
        dispatcher->drain();
        assert_equals(1, parent->resumes);
        assert_false(parent->result.is_success());
        assert_true(retained_frame.expired(), "receiver frame retained");
    }
    auto value = std::make_shared<int>(42);
    std::weak_ptr<int> retained_value = value;
    auto sent = channel->try_send(value);
    value.reset();
    if (cancel_mode == 1) {
        if (capacity) {
            assert_true(sent.is_success());
            auto received = channel->try_receive();
            assert_equals(42, *received.get_or_throw());
        } else assert_true(sent.is_failure());
        assert_true(undelivered.empty());
        if (capacity) assert_true(retained_value.expired(), "received buffered value retained");
        return;
    }
    assert_true(sent.is_success());
    assert_equals(0, parent->resumes);
    assert_equals(size_t(1), dispatcher->queue.size());
    if (cancel_mode == 2) job->cancel();
    dispatcher->drain();
    assert_equals(1, parent->resumes);
    if (cancel_mode == 2) assert_false(parent->result.is_success());
    else if (catching) {
        std::unique_ptr<channels::ChannelResult<std::shared_ptr<int>>> result(
            static_cast<channels::ChannelResult<std::shared_ptr<int>>*>(parent->result.get_or_throw()));
        assert_true(result->is_success());
        assert_equals(42, *result->get_or_throw());
    } else {
        std::unique_ptr<std::shared_ptr<int>> result(
            static_cast<std::shared_ptr<int>*>(parent->result.get_or_throw()));
        assert_equals(42, **result);
    }
    assert_true(undelivered == (cancel_mode == 2 ? std::vector<int>({42}) : std::vector<int>{}));
    assert_true(retained_value.expired(), "receiver value retained");
    assert_true(retained_frame.expired(), "receiver frame retained");
}

void test_channel_suspended_receive_close(bool catching, bool exceptional) {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto parent = std::make_shared<RecordingContinuation>(dispatcher);
    auto channel = std::make_shared<channels::BufferedChannel<int>>(0);
    auto frame = std::make_shared<CallFrame>(parent, [channel, catching](auto* continuation) {
        return catching ? channel->receive_catching(continuation) : channel->receive(continuation);
    });
    std::weak_ptr<CallFrame> retained_frame = frame;
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    frame.reset();
    std::exception_ptr cause = exceptional ? std::make_exception_ptr(std::runtime_error("channel close")) : nullptr;
    assert_true(channel->close(cause));
    assert_equals(0, parent->resumes);
    assert_equals(size_t(1), dispatcher->queue.size());
    dispatcher->drain();
    assert_equals(1, parent->resumes);
    if (catching) {
        std::unique_ptr<channels::ChannelResult<int>> result(
            static_cast<channels::ChannelResult<int>*>(parent->result.get_or_throw()));
        assert_true(result->is_closed());
        assert_true(result->exception_or_null() == cause);
    } else {
        assert_false(parent->result.is_success());
        if (exceptional) assert_true(parent->result.exception_or_null() == cause);
        else {
            bool closed = false;
            try { std::rethrow_exception(parent->result.exception_or_null()); }
            catch (const channels::ClosedReceiveChannelException&) { closed = true; }
            assert_true(closed);
        }
    }
    assert_true(retained_frame.expired());
}

void test_channel_cancel_retains_queued_senders(int capacity) {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    std::vector<int> undelivered;
    auto channel = std::make_shared<channels::BufferedChannel<std::shared_ptr<int>>>(
        capacity, [&](auto value) { undelivered.push_back(*value); });
    if (capacity) assert_true(channel->try_send(std::make_shared<int>(0)).is_success());
    std::vector<std::shared_ptr<RecordingContinuation>> parents;
    std::vector<std::weak_ptr<CallFrame>> frames;
    std::vector<std::weak_ptr<int>> values;
    for (int index = 0; index < 2; ++index) {
        auto parent = std::make_shared<RecordingContinuation>(dispatcher);
        parents.push_back(parent);
        auto value = std::make_shared<int>(42 + index);
        values.push_back(value);
        auto frame = std::make_shared<CallFrame>(parent, [channel, value](auto* cont) {
            return channel->send(value, cont);
        });
        frames.push_back(frame);
        assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    }
    auto cause = std::make_exception_ptr(std::runtime_error("channel cancelled"));
    channel->cancel(cause);
    assert_true(undelivered == (capacity ? std::vector<int>({43, 42, 0}) : std::vector<int>({43, 42})));
    assert_equals(size_t(2), dispatcher->queue.size());
    {
        auto first = std::move(dispatcher->queue.front());
        dispatcher->queue.pop_front();
        first->run();
        assert_equals(1, parents[0]->resumes);
        assert_equals(0, parents[1]->resumes);
    }
    dispatcher->drain();
    for (size_t index = 0; index < parents.size(); ++index) {
        assert_equals(1, parents[index]->resumes);
        assert_true(parents[index]->result.exception_or_null() == cause);
        assert_true(frames[index].expired());
        assert_true(values[index].expired());
    }
}

void test_channel_broadcast_dispatch_and_cancellation(int capacity, int cancel_mode) {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto job = JobImpl::create(nullptr);
    std::shared_ptr<CoroutineContext> context = (cancel_mode == 1 || cancel_mode == 2)
        ? job->operator+(dispatcher) : dispatcher;
    auto parent = std::make_shared<RecordingContinuation>(context);
    auto channel = std::make_shared<channels::BufferedChannel<std::shared_ptr<int>>>(capacity);
    if (capacity) assert_true(channel->try_send(std::make_shared<int>(0)).is_success());
    auto value = std::make_shared<int>(42);
    std::weak_ptr<int> retained_value = value;
    auto frame = std::make_shared<CallFrame>(parent, [channel, value](auto* continuation) {
        return channel->send_broadcast(value, continuation);
    });
    std::weak_ptr<CallFrame> retained_frame = frame;
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    frame.reset();
    value.reset();
    assert_false(retained_frame.expired());
    if (cancel_mode == 3) {
        channel->cancel(std::make_exception_ptr(std::runtime_error("broadcast channel cancelled")));
    } else {
        if (cancel_mode == 1) job->cancel();
        if (capacity) {
            auto seed = channel->try_receive();
            assert_true(seed.is_success());
            assert_equals(0, *seed.get_or_throw());
        }
        {
            auto received = channel->try_receive();
            if (cancel_mode == 1) assert_true(received.is_failure());
            else {
                assert_true(received.is_success());
                assert_equals(42, *received.get_or_throw());
            }
        }
        if (cancel_mode == 2) job->cancel();
    }
    assert_equals(0, parent->resumes);
    assert_equals(size_t(1), dispatcher->queue.size());
    dispatcher->drain();
    assert_equals(1, parent->resumes);
    if (cancel_mode == 1 || cancel_mode == 2) assert_false(parent->result.is_success());
    else {
        std::unique_ptr<bool> result(static_cast<bool*>(parent->result.get_or_throw()));
        assert_equals(cancel_mode != 3, *result);
    }
    assert_true(retained_frame.expired(), "completed broadcast sender frame retained");
    assert_true(retained_value.expired(), "completed broadcast sender value retained");
}

void test_channel_broadcast_rendezvous_and_retry(bool retry) {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto job = JobImpl::create(nullptr);
    auto receiver = std::make_shared<RecordingContinuation>(job->operator+(dispatcher));
    auto sender = std::make_shared<RecordingContinuation>(dispatcher);
    auto channel = std::make_shared<channels::BufferedChannel<int>>(0);
    auto receive_frame = std::make_shared<CallFrame>(receiver, [channel](auto* continuation) {
        return channel->receive(continuation);
    });
    std::weak_ptr<CallFrame> retained_receiver = receive_frame;
    assert_true(intrinsics::is_coroutine_suspended(receive_frame->start(Result<void*>::success(nullptr))));
    receive_frame.reset();
    if (retry) {
        job->cancel();
        dispatcher->drain();
        assert_false(receiver->result.is_success());
        assert_true(retained_receiver.expired());
    }
    auto frame = std::make_shared<CallFrame>(sender, [channel](auto* continuation) {
        return channel->send_broadcast(42, continuation);
    });
    std::weak_ptr<CallFrame> retained_sender = frame;
    void* result = frame->start(Result<void*>::success(nullptr));
    if (retry) {
        assert_true(intrinsics::is_coroutine_suspended(result));
        frame.reset();
        assert_equals(42, channel->try_receive().get_or_throw());
        assert_equals(0, sender->resumes);
        assert_equals(size_t(1), dispatcher->queue.size());
        dispatcher->drain();
        assert_equals(1, sender->resumes);
        std::unique_ptr<bool> sent(static_cast<bool*>(sender->result.get_or_throw()));
        assert_true(*sent);
        assert_true(retained_sender.expired());
    } else {
        std::unique_ptr<bool> sent(static_cast<bool*>(result));
        assert_true(*sent);
        assert_equals(0, sender->resumes);
        assert_equals(0, receiver->resumes);
        assert_equals(size_t(1), dispatcher->queue.size());
        dispatcher->drain();
        assert_equals(1, receiver->resumes);
        std::unique_ptr<int> received(static_cast<int*>(receiver->result.get_or_throw()));
        assert_equals(42, *received);
        assert_true(retained_receiver.expired());
    }
}

void test_channel_broadcast_immediate_and_closed() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto parent = std::make_shared<RecordingContinuation>(dispatcher);
    auto channel = std::make_shared<channels::BufferedChannel<int>>(1);
    {
        auto frame = std::make_shared<CallFrame>(parent, [channel](auto* continuation) {
            return channel->send_broadcast(42, continuation);
        });
        std::unique_ptr<bool> result(static_cast<bool*>(frame->start(Result<void*>::success(nullptr))));
        assert_true(*result);
        assert_equals(42, channel->try_receive().get_or_throw());
        assert_equals(0, parent->resumes);
        assert_true(dispatcher->queue.empty());
    }
    for (bool exceptional : {false, true}) {
        auto closed = std::make_shared<channels::BufferedChannel<int>>(0);
        closed->close(exceptional ? std::make_exception_ptr(std::runtime_error("closed broadcast")) : nullptr);
        auto frame = std::make_shared<CallFrame>(parent, [closed](auto* continuation) {
            return closed->send_broadcast(42, continuation);
        });
        std::unique_ptr<bool> result(static_cast<bool*>(frame->start(Result<void*>::success(nullptr))));
        assert_false(*result);
        assert_equals(0, parent->resumes);
        assert_true(dispatcher->queue.empty());
    }
    int undelivered = 0;
    auto unsupported = std::make_shared<channels::BufferedChannel<int>>(0, [&](int) { ++undelivered; });
    auto frame = std::make_shared<CallFrame>(parent, [unsupported](auto* continuation) {
        return unsupported->send_broadcast(42, continuation);
    });
    bool rejected = false;
    try { frame->start(Result<void*>::success(nullptr)); }
    catch (const std::logic_error&) { rejected = true; }
    assert_true(rejected);
    assert_equals(0, undelivered);
    assert_true(unsupported->try_receive().is_failure());
    assert_equals(0, parent->resumes);
    assert_true(dispatcher->queue.empty());
}

void test_channel_iterator_dispatch_and_lifetime(int mode, bool drop_handle) {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto job = JobImpl::create(nullptr);
    std::shared_ptr<CoroutineContext> context = (mode == 1 || mode == 2)
        ? job->operator+(dispatcher) : dispatcher;
    auto parent = std::make_shared<RecordingContinuation>(context);
    std::vector<int> undelivered;
    auto channel = std::make_shared<channels::BufferedChannel<std::shared_ptr<int>>>(
        0, [&](auto value) { undelivered.push_back(*value); });
    auto holder = std::make_shared<std::unique_ptr<channels::ChannelIterator<std::shared_ptr<int>>>>(
        channel->iterator());
    auto frame = std::make_shared<CallFrame>(parent, [holder, channel](auto* continuation) {
        return (*holder)->has_next(continuation);
    });
    std::weak_ptr<CallFrame> retained_frame = frame;
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    frame.reset();
    if (drop_handle) holder->reset();
    std::weak_ptr<int> retained_value;
    std::exception_ptr cause;
    if (mode == 1) job->cancel();
    else if (mode == 3 || mode == 4) {
        if (mode == 4) cause = std::make_exception_ptr(std::runtime_error("iterator close"));
        channel->close(cause);
    } else {
        auto value = std::make_shared<int>(42);
        retained_value = value;
        assert_true(channel->try_send(value).is_success());
        value.reset();
        if (mode == 2) job->cancel();
    }
    assert_equals(0, parent->resumes);
    assert_equals(size_t(1), dispatcher->queue.size());
    dispatcher->drain();
    assert_equals(1, parent->resumes);
    if (mode == 1 || mode == 2 || mode == 4) {
        assert_false(parent->result.is_success());
        if (mode == 4) assert_true(parent->result.exception_or_null() == cause);
    } else {
        std::unique_ptr<bool> result(static_cast<bool*>(parent->result.get_or_throw()));
        assert_equals(mode == 0, *result);
        if (!drop_handle && mode == 0) {
            std::unique_ptr<bool> repeated(static_cast<bool*>((*holder)->has_next(parent.get())));
            assert_true(*repeated);
            {
                auto value = (*holder)->next();
                assert_equals(42, *value);
            }
            assert_equals(1, parent->resumes);
            assert_true(dispatcher->queue.empty());
        }
        if (!drop_handle && mode == 3) {
            bool closed = false;
            try { (*holder)->next(); }
            catch (const channels::ClosedReceiveChannelException&) { closed = true; }
            assert_true(closed);
        }
    }
    assert_true(undelivered == (mode == 2 ? std::vector<int>({42}) : std::vector<int>{}));
    if (mode != 1) assert_true(retained_frame.expired(), "resumed iterator retained completed frame");
    holder->reset();
    assert_true(retained_frame.expired(), "iterator retained completed frame after handle release");
    assert_true(retained_value.expired(), "iterator retained element after handle release");
}

void test_channel_iterator_immediate_result_ownership() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto job = JobImpl::create(nullptr);
    job->cancel();
    auto parent = std::make_shared<RecordingContinuation>(job->operator+(dispatcher));
    auto channel = std::make_shared<channels::BufferedChannel<std::shared_ptr<int>>>(1);
    auto iterator = channel->iterator();
    bool missing = false;
    try { iterator->next(); }
    catch (const std::logic_error&) { missing = true; }
    assert_true(missing);
    auto value = std::make_shared<int>(42);
    std::weak_ptr<int> retained = value;
    assert_true(channel->try_send(value).is_success());
    value.reset();
    {
        std::unique_ptr<bool> result(static_cast<bool*>(iterator->has_next(parent.get())));
        assert_true(*result);
        std::unique_ptr<bool> repeated(static_cast<bool*>(iterator->has_next(parent.get())));
        assert_true(*repeated);
        auto element = iterator->next();
        assert_equals(42, *element);
    }
    assert_true(retained.expired());
    value = std::make_shared<int>(84);
    retained = value;
    assert_true(channel->try_send(value).is_success());
    value.reset();
    {
        std::unique_ptr<bool> result(static_cast<bool*>(iterator->has_next(parent.get())));
        assert_true(*result);
    }
    assert_false(retained.expired());
    iterator.reset();
    assert_true(retained.expired(), "unconsumed iterator result retained");
    assert_equals(0, parent->resumes);
    assert_true(dispatcher->queue.empty());
}

class RegistrationRacingChannel final : public channels::BufferedChannel<int> {
public:
    std::atomic<bool> published{false};
    std::atomic<bool> proceed{false};
    RegistrationRacingChannel() : BufferedChannel(0) {}
    void on_receive_enqueued() override {
        published.store(true, std::memory_order_release);
        while (!proceed.load(std::memory_order_acquire)) std::this_thread::yield();
    }
};

void test_iterator_registration_races_completion(bool close) {
    for (int iteration = 0; iteration < 100; ++iteration) {
        auto dispatcher = std::make_shared<QueueDispatcher>();
        auto parent = std::make_shared<RecordingContinuation>(dispatcher);
        auto channel = std::make_shared<RegistrationRacingChannel>();
        auto holder = std::make_shared<std::unique_ptr<channels::ChannelIterator<int>>>(channel->iterator());
        auto frame = std::make_shared<CallFrame>(parent, [holder, channel](auto* continuation) {
            return (*holder)->has_next(continuation);
        });
        std::weak_ptr<CallFrame> retained = frame;
        void* start_result = nullptr;
        std::exception_ptr failure;
        std::atomic<bool> registration_done{false};
        std::thread registration([&] {
            try { start_result = frame->start(Result<void*>::success(nullptr)); }
            catch (...) { failure = std::current_exception(); }
            registration_done.store(true, std::memory_order_release);
        });
        while (!channel->published.load(std::memory_order_acquire) &&
               !registration_done.load(std::memory_order_acquire)) std::this_thread::yield();
        channel->proceed.store(true, std::memory_order_release);
        bool completed = close ? channel->close() : channel->try_send(42).is_success();
        registration.join();
        if (failure) std::rethrow_exception(failure);
        assert_true(channel->published.load(std::memory_order_acquire));
        assert_true(completed);
        frame.reset();
        if (intrinsics::is_coroutine_suspended(start_result)) {
            assert_equals(0, parent->resumes);
            assert_equals(size_t(1), dispatcher->queue.size());
            dispatcher->drain();
            assert_equals(1, parent->resumes);
            start_result = parent->result.get_or_throw();
        } else {
            assert_equals(0, parent->resumes);
            assert_true(dispatcher->queue.empty());
        }
        std::unique_ptr<bool> result(static_cast<bool*>(start_result));
        assert_equals(!close, *result);
        if (!close) assert_equals(42, (*holder)->next());
        assert_true(retained.expired(), "registration race retained completed frame");
        holder->reset();
    }
}

void test_deferred_coroutine_await_typed_result(int mode) {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto job = JobImpl::create(nullptr);
    std::shared_ptr<CoroutineContext> context = (mode == 2 || mode == 3)
        ? job->operator+(dispatcher) : dispatcher;
    auto parent = std::make_shared<RecordingContinuation>(context);
    auto deferred = std::make_shared<DeferredCoroutine<std::shared_ptr<int>>>(EmptyCoroutineContext::instance(), true);
    std::weak_ptr<DeferredCoroutine<std::shared_ptr<int>>> retained_deferred = deferred;
    auto frame = std::make_shared<CallFrame>(parent, [deferred](auto* continuation) {
        return deferred->await(continuation);
    });
    std::weak_ptr<CallFrame> retained_frame = frame;
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    frame.reset();
    if (mode == 2) job->cancel();
    auto value = std::make_shared<int>(42);
    auto cause = std::make_exception_ptr(std::runtime_error("deferred failed"));
    if (mode == 1) deferred->resume_with(Result<std::shared_ptr<int>>::failure(cause));
    else deferred->resume_with(Result<std::shared_ptr<int>>::success(value));
    if (mode == 3) job->cancel();
    deferred.reset();
    assert_equals(0, parent->resumes);
    assert_equals(size_t(1), dispatcher->queue.size());
    dispatcher->drain();
    assert_equals(1, parent->resumes);
    if (mode) {
        assert_false(parent->result.is_success());
        if (mode == 1) assert_true(parent->result.exception_or_null() == cause);
    } else {
        std::unique_ptr<std::shared_ptr<int>> result(
            static_cast<std::shared_ptr<int>*>(parent->result.get_or_throw()));
        assert_true(*result == value);
        assert_equals(42, **result);
    }
    assert_true(retained_frame.expired(), "typed deferred await frame retained");
    assert_true(retained_deferred.expired(), "typed deferred await object retained");
    assert_equals(1L, value.use_count());
}

void test_deferred_coroutine_await_immediate() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto job = JobImpl::create(nullptr);
    job->cancel();
    auto parent = std::make_shared<RecordingContinuation>(job->operator+(dispatcher));
    auto deferred = std::make_shared<DeferredCoroutine<std::shared_ptr<int>>>(EmptyCoroutineContext::instance(), true);
    auto value = std::make_shared<int>(42);
    deferred->resume_with(Result<std::shared_ptr<int>>::success(value));
    auto completed_references = value.use_count();
    for (int iteration = 0; iteration < 2; ++iteration) {
        auto frame = std::make_shared<CallFrame>(parent, [deferred](auto* continuation) {
            return deferred->await(continuation);
        });
        std::weak_ptr<CallFrame> retained = frame;
        {
            std::unique_ptr<std::shared_ptr<int>> result(
                static_cast<std::shared_ptr<int>*>(frame->start(Result<void*>::success(nullptr))));
            assert_true(*result == value);
        }
        frame.reset();
        assert_true(retained.expired());
        assert_equals(completed_references, value.use_count());
    }
    auto failed = std::make_shared<DeferredCoroutine<int>>(EmptyCoroutineContext::instance(), true);
    auto cause = std::make_exception_ptr(std::runtime_error("immediate await failure"));
    failed->resume_with(Result<int>::failure(cause));
    auto frame = std::make_shared<CallFrame>(parent, [failed](auto* continuation) {
        return failed->await(continuation);
    });
    bool caught = false;
    try { frame->start(Result<void*>::success(nullptr)); }
    catch (...) { caught = std::current_exception() == cause; }
    assert_true(caught);
    assert_equals(0, parent->resumes);
    assert_true(dispatcher->queue.empty());
}

void test_completed_typed_continuation_ignores_segment_registration() {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto parent = std::make_shared<RecordingContinuation>(dispatcher);
    auto channel = std::make_shared<channels::BufferedChannel<int>>(0);
    auto segment = std::make_shared<channels::ChannelSegment<int>>(0, nullptr, channel.get(), 0);
    auto frame = std::make_shared<CallFrame>(parent, [segment](auto* continuation) {
        return dsl::suspend_cancellable_coroutine_reusable<bool>(continuation, [&](auto* cont) {
            cont->resume(true, nullptr);
            cont->invoke_on_cancellation(segment.get(), 0);
        });
    });
    std::weak_ptr<CallFrame> retained = frame;
    std::unique_ptr<bool> result(static_cast<bool*>(frame->start(Result<void*>::success(nullptr))));
    assert_true(*result);
    assert_equals(0, parent->resumes);
    assert_true(dispatcher->queue.empty());
    frame.reset();
    assert_true(retained.expired());
}

template<typename DeferredImplementation>
class CompletionInspectingDeferred : public DeferredImplementation {
public:
    using DeferredImplementation::DeferredImplementation;
    using DeferredImplementation::get_state_for_await;
};

void test_deferred_select_completion_processing() {
    auto value_exception = std::make_exception_ptr(std::runtime_error("successful exception payload"));
    auto value = std::make_shared<CompletionInspectingDeferred<CompletableDeferredImpl<std::exception_ptr>>>();
    assert_true(value->complete(value_exception));
    auto* state = value->get_state_for_await();
    auto process = value->on_await().get_process_res_func();
    std::unique_ptr<std::exception_ptr> exception_box(static_cast<std::exception_ptr*>(process(value.get(), nullptr, state)));
    assert_true(*exception_box == value_exception);
    assert_true(value->get_completed() == value_exception);

    auto integer = std::make_shared<CompletionInspectingDeferred<DeferredCoroutine<int>>>(EmptyCoroutineContext::instance(), true);
    integer->resume_with(Result<int>::success(42));
    state = integer->get_state_for_await();
    process = integer->on_await().get_process_res_func();
    std::unique_ptr<int> integer_box(static_cast<int*>(process(integer.get(), nullptr, state)));
    assert_equals(42, *integer_box);
    assert_equals(42, integer->get_completed());

    auto reference = std::make_shared<CompletionInspectingDeferred<CompletableDeferredImpl<std::shared_ptr<int>>>>();
    auto payload = std::make_shared<int>(84);
    assert_true(reference->complete(payload));
    state = reference->get_state_for_await();
    process = reference->on_await().get_process_res_func();
    std::unique_ptr<std::shared_ptr<int>> reference_box(static_cast<std::shared_ptr<int>*>(process(reference.get(), nullptr, state)));
    assert_true(*reference_box == payload);
    assert_true(reference->get_completed() == payload);

    for (bool cancelled : {false, true}) {
        auto failure = cancelled ? std::make_exception_ptr(CancellationException("selected deferred cancelled"))
                                 : std::make_exception_ptr(std::runtime_error("selected deferred failed"));
        auto failed = std::make_shared<CompletionInspectingDeferred<DeferredCoroutine<int>>>(EmptyCoroutineContext::instance(), true);
        failed->resume_with(Result<int>::failure(failure));
        process = failed->on_await().get_process_res_func();
        bool caught = false;
        try { process(failed.get(), nullptr, failed->get_state_for_await()); }
        catch (...) { caught = std::current_exception() == failure; }
        assert_true(caught);

        auto completable = std::make_shared<CompletionInspectingDeferred<CompletableDeferredImpl<int>>>();
        assert_true(completable->complete_exceptionally(failure));
        process = completable->on_await().get_process_res_func();
        caught = false;
        try { process(completable.get(), nullptr, completable->get_state_for_await()); }
        catch (...) { caught = std::current_exception() == failure; }
        assert_true(caught);
    }
    assert_true(JobSupport::on_await_internal_process_res_func(nullptr, nullptr, nullptr) == nullptr);
}

void test_selected_deferred_invokes_typed_block() {
    auto parent = std::make_shared<RecordingContinuation>(EmptyCoroutineContext::instance());
    auto deferred = std::make_shared<CompletableDeferredImpl<int>>();
    assert_true(deferred->complete(42));
    auto second = std::make_shared<DeferredCoroutine<int>>(EmptyCoroutineContext::instance(), true);
    second->resume_with(Result<int>::success(99));
    int calls = 0;
    auto* result = selects::select<void*>([&](auto& builder) {
        builder.template invoke<int>(deferred->on_await(),
            std::function<void*(int, Continuation<void*>*)>([&](int value, auto*) {
                ++calls;
                assert_equals(42, value);
                return new int(value + 1);
            }));
        builder.template invoke<int>(second->on_await(),
            std::function<void*(int, Continuation<void*>*)>([](int, auto*) -> void* {
                throw std::logic_error("A later clause must not replace an already selected clause");
            }));
    }, parent.get());
    std::unique_ptr<int> value(static_cast<int*>(result));
    assert_equals(43, *value);
    assert_equals(1, calls);
    assert_equals(0, parent->resumes);
}

template<typename T>
void check_selected_deferred_value(T expected) {
    auto parent = std::make_shared<RecordingContinuation>(EmptyCoroutineContext::instance());
    auto deferred = std::make_shared<CompletableDeferredImpl<T>>();
    assert_true(deferred->complete(expected));
    int calls = 0;
    std::unique_ptr<T> result(static_cast<T*>(selects::select<void*>([&](auto& builder) {
        builder.template invoke<T>(deferred->on_await(),
            std::function<void*(T, Continuation<void*>*)>([&](T value, auto*) {
                ++calls;
                assert_true(value == expected);
                return new T(value);
            }));
    }, parent.get())));
    assert_true(*result == expected);
    assert_equals(1, calls);
    assert_equals(0, parent->resumes);
}

class SelectedValueFrame final : public ContinuationImpl {
public:
    std::shared_ptr<int> value;
    void* _label = nullptr;
    SelectedValueFrame(std::shared_ptr<Continuation<void*>> completion, std::shared_ptr<int> value)
        : ContinuationImpl(std::move(completion)), value(std::move(value)) {}
    void* invoke_suspend(Result<void*> result) override {
        coroutine_begin(this)
        coroutine_yield(this, delay(1, this));
        return new std::shared_ptr<int>(value);
    }
};

void test_selected_deferred_block_suspends(bool cancel) {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto job = JobImpl::create(nullptr);
    auto parent = std::make_shared<RecordingContinuation>(job->operator+(dispatcher));
    auto deferred = std::make_shared<CompletableDeferredImpl<std::shared_ptr<int>>>();
    auto payload = std::make_shared<int>(84);
    assert_true(deferred->complete(payload));
    std::weak_ptr<SelectedValueFrame> block_lifetime;
    int calls = 0;
    auto frame = std::make_shared<CallFrame>(parent, [&, deferred](auto* continuation) {
        return selects::select<void*>([&](auto& builder) {
            builder.template invoke<std::shared_ptr<int>>(deferred->on_await(),
                std::function<void*(std::shared_ptr<int>, Continuation<void*>*)>([&](auto value, auto* completion) {
                    ++calls;
                    auto owner = dynamic_cast<BaseContinuationImpl*>(completion)->shared_from_this();
                    auto block = std::make_shared<SelectedValueFrame>(std::move(owner), std::move(value));
                    block_lifetime = block;
                    return block->start(Result<void*>::success(nullptr));
                }));
        }, continuation);
    });
    std::weak_ptr<CallFrame> caller_lifetime = frame;
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    assert_equals(1, calls);
    assert_equals(0, parent->resumes);
    assert_false(block_lifetime.expired());
    frame.reset();
    if (cancel) job->cancel();
    else dispatcher->fire_timer();
    assert_equals(0, parent->resumes);
    dispatcher->drain();
    assert_equals(1, parent->resumes);
    assert_equals(1, calls);
    if (cancel) {
        assert_true(parent->result.exception_or_null() == job->get_cancellation_exception());
        dispatcher->timer.reset();
    } else {
        std::unique_ptr<std::shared_ptr<int>> result(static_cast<std::shared_ptr<int>*>(parent->result.get_or_throw()));
        assert_true(*result == payload);
    }
    assert_true(block_lifetime.expired());
    assert_true(caller_lifetime.expired());
}

void test_selected_deferred_failure_skips_block() {
    for (bool cancelled : {false, true}) {
        auto cause = cancelled ? std::make_exception_ptr(CancellationException("selected failure"))
                               : std::make_exception_ptr(std::runtime_error("selected failure"));
        auto deferred = std::make_shared<CompletableDeferredImpl<int>>();
        assert_true(deferred->complete_exceptionally(cause));
        auto parent = std::make_shared<RecordingContinuation>(EmptyCoroutineContext::instance());
        int calls = 0;
        bool caught = false;
        try {
            selects::select<void*>([&](auto& builder) {
                builder.template invoke<int>(deferred->on_await(),
                    std::function<void*(int, Continuation<void*>*)>([&](auto, auto*) { ++calls; return nullptr; }));
            }, parent.get());
        } catch (...) { caught = std::current_exception() == cause; }
        assert_true(caught);
        assert_equals(0, calls);
        assert_equals(0, parent->resumes);
    }
}

void test_deferred_select_wait_resumes_block(int mode) {
    auto dispatcher = std::make_shared<QueueDispatcher>();
    auto job = mode == 0 ? nullptr : JobImpl::create(nullptr);
    auto context = job ? job->operator+(dispatcher) : std::static_pointer_cast<CoroutineContext>(dispatcher);
    auto parent = std::make_shared<RecordingContinuation>(context);
    auto deferred = std::make_shared<CompletableDeferredImpl<int>>();
    std::weak_ptr<CompletableDeferredImpl<int>> target_lifetime = deferred;
    std::weak_ptr<selects::SelectImplementation<void*>> select_lifetime;
    int calls = 0;
    auto frame = std::make_shared<CallFrame>(parent, [&, target = deferred.get()](auto* continuation) {
        return selects::select<void*>([&](auto& builder) {
            select_lifetime = builder.shared_from_this();
            builder.template invoke<int>(target->on_await(),
                std::function<void*(int, Continuation<void*>*)>([&](int value, auto* completion) -> void* {
                    ++calls;
                    assert_equals(42, value);
                    if (mode == 4 || mode == 5) {
                        auto block = std::make_shared<SelectedValueFrame>(internal::retain_continuation(completion), std::make_shared<int>(43));
                        return block->start(Result<void*>::success(nullptr));
                    }
                    return new int(value + 1);
                }));
        }, continuation);
    });
    std::weak_ptr<CallFrame> caller_lifetime = frame;
    assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
    frame.reset();
    deferred.reset();
    assert_false(caller_lifetime.expired());
    assert_false(target_lifetime.expired());
    assert_false(select_lifetime.expired());
    auto target = target_lifetime.lock();
    auto failure = std::make_exception_ptr(std::runtime_error("selected asynchronous failure"));
    if (mode == 1) job->cancel();
    else if (mode == 3) assert_true(target->complete_exceptionally(failure));
    else assert_true(target->complete(42));
    if (mode == 2) job->cancel();
    target.reset();
    assert_equals(0, parent->resumes);
    dispatcher->drain();
    if (mode == 4 || mode == 5) {
        assert_equals(1, calls);
        assert_equals(0, parent->resumes);
        if (mode == 5) job->cancel();
        else dispatcher->fire_timer();
        dispatcher->drain();
    }
    assert_equals(1, parent->resumes);
    assert_equals((mode == 1 || mode == 2 || mode == 3) ? 0 : 1, calls);
    if (mode == 1 || mode == 2 || mode == 5) {
        assert_true(parent->result.exception_or_null() == job->get_cancellation_exception());
        dispatcher->timer.reset();
    } else if (mode == 3) {
        assert_true(parent->result.exception_or_null() == failure);
    } else if (mode == 4) {
        std::unique_ptr<std::shared_ptr<int>> result(static_cast<std::shared_ptr<int>*>(parent->result.get_or_throw()));
        assert_equals(43, **result);
    } else {
        std::unique_ptr<int> result(static_cast<int*>(parent->result.get_or_throw()));
        assert_equals(43, *result);
    }
    assert_true(caller_lifetime.expired());
    assert_true(select_lifetime.expired());
    assert_true(target_lifetime.expired());
}

void test_deferred_select_reregisters_completed_clause() {
    auto parent = std::make_shared<RecordingContinuation>(EmptyCoroutineContext::instance());
    auto deferred = std::make_shared<CompletableDeferredImpl<int>>();
    std::weak_ptr<CompletableDeferredImpl<int>> target_lifetime = deferred;
    int calls = 0;
    auto* raw = selects::select<void*>([&](auto& builder) {
        builder.template invoke<int>(deferred->on_await(),
            std::function<void*(int, Continuation<void*>*)>([&](int value, auto*) {
                ++calls;
                assert_equals(42, value);
                return new int(value);
            }));
        assert_true(deferred->complete(42));
        deferred.reset();
        assert_false(target_lifetime.expired());
    }, parent.get());
    assert_false(intrinsics::is_coroutine_suspended(raw));
    std::unique_ptr<int> result(static_cast<int*>(raw));
    assert_equals(42, *result);
    assert_equals(1, calls);
    assert_equals(0, parent->resumes);
    assert_true(target_lifetime.expired());
}

void test_select_disposes_unselected_and_cancelled_job_handlers() {
    for (bool cancel : {false, true}) {
        auto dispatcher = std::make_shared<QueueDispatcher>();
        auto job = JobImpl::create(nullptr);
        auto parent = std::make_shared<RecordingContinuation>(job->operator+(dispatcher));
        auto deferred = std::make_shared<CompletableDeferredImpl<int>>();
        auto other = JobImpl::create(nullptr);
        selects::SelectClause0Impl join_clause(other.get(), [](void* object, void* select, void* param) {
            static_cast<JobImpl*>(object)->register_select_for_on_join(select, param);
        });
        int calls = 0;
        auto frame = std::make_shared<CallFrame>(parent, [&](auto* completion) {
            return selects::select<void*>([&](auto& builder) {
                builder.template invoke<int>(deferred->on_await(),
                    std::function<void*(int, Continuation<void*>*)>([&](int value, auto*) { ++calls; return new int(value); }));
                builder.invoke(join_clause, std::function<void*(Continuation<void*>*)>([](auto*) -> void* {
                    throw std::logic_error("Disposed losing join handler selected");
                }));
            }, completion);
        });
        std::weak_ptr<CallFrame> lifetime = frame;
        assert_true(intrinsics::is_coroutine_suspended(frame->start(Result<void*>::success(nullptr))));
        frame.reset();
        if (cancel) job->cancel();
        else assert_true(deferred->complete(42));
        dispatcher->drain();
        assert_equals(1, parent->resumes);
        assert_equals(cancel ? 0 : 1, calls);
        if (!cancel) {
            std::unique_ptr<int> result(static_cast<int*>(parent->result.get_or_throw()));
            assert_equals(42, *result);
        }
        assert_true(lifetime.expired());
        other->complete();
        if (cancel) assert_true(deferred->complete(42));
        dispatcher->drain();
        assert_equals(1, parent->resumes);
        assert_equals(cancel ? 0 : 1, calls);
    }
}

int main() {
    test_continuation_required_values();
    test_restricted_continuation_context();
    std::cerr << "test_deferred_select_wait_resumes_block\n";
    for (int mode : {0, 1, 2, 3, 4, 5}) test_deferred_select_wait_resumes_block(mode);
    test_deferred_select_reregisters_completed_clause();
    test_select_disposes_unselected_and_cancelled_job_handlers();
    std::cerr << "test_selected_deferred_invokes_typed_block\n";
    test_selected_deferred_invokes_typed_block();
    check_selected_deferred_value(std::make_shared<int>(84));
    check_selected_deferred_value(std::string("selected value"));
    check_selected_deferred_value(std::make_exception_ptr(std::runtime_error("successful exception payload")));
    test_selected_deferred_block_suspends(false);
    test_selected_deferred_block_suspends(true);
    test_selected_deferred_failure_skips_block();
    std::cerr << "test_deferred_select_completion_processing\n";
    test_deferred_select_completion_processing();
    std::cerr << "test_completed_typed_continuation_ignores_segment_registration\n";
    test_completed_typed_continuation_ignores_segment_registration();
    std::cerr << "test_deferred_coroutine_await_typed_result\n";
    for (int mode : {0, 1, 2, 3}) test_deferred_coroutine_await_typed_result(mode);
    test_deferred_coroutine_await_immediate();
    std::cerr << "test_iterator_registration_races_completion\n";
    test_iterator_registration_races_completion(false);
    test_iterator_registration_races_completion(true);
    std::cerr << "test_channel_iterator_dispatch_and_lifetime\n";
    for (int mode : {0, 1, 2, 3, 4}) for (bool drop : {false, true}) {
        std::cerr << "  iterator mode=" << mode << " drop=" << drop << '\n';
        test_channel_iterator_dispatch_and_lifetime(mode, drop);
    }
    test_channel_iterator_immediate_result_ownership();
    std::cerr << "test_channel_broadcast_dispatch_and_cancellation\n";
    for (int capacity : {0, 1}) for (int mode : {0, 1, 2, 3}) {
        std::cerr << "  broadcast capacity=" << capacity << " cancel=" << mode << '\n';
        test_channel_broadcast_dispatch_and_cancellation(capacity, mode);
    }
    test_channel_broadcast_immediate_and_closed();
    test_channel_broadcast_rendezvous_and_retry(false);
    test_channel_broadcast_rendezvous_and_retry(true);
    std::cerr << "test_channel_sender_dispatch_and_cancellation\n";
    for (int capacity : {0, 1}) for (int mode : {0, 1, 2}) {
        std::cerr << "  sender capacity=" << capacity << " cancel=" << mode << '\n';
        test_channel_sender_dispatch_and_cancellation(capacity, mode);
    }
    std::cerr << "test_channel_receiver_dispatch_and_cancellation\n";
    for (bool catching : {false, true}) for (int capacity : {0, 1}) for (int mode : {0, 1, 2}) {
        std::cerr << "  receiver catching=" << catching << " capacity=" << capacity << " cancel=" << mode << '\n';
        test_channel_receiver_dispatch_and_cancellation(catching, capacity, mode);
    }
    std::cerr << "test_channel_cancel_retains_queued_senders\n";
    test_channel_cancel_retains_queued_senders(0);
    test_channel_cancel_retains_queued_senders(1);
    std::cerr << "test_channel_suspended_receive_close\n";
    for (bool catching : {false, true}) for (bool exceptional : {false, true})
        test_channel_suspended_receive_close(catching, exceptional);
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
