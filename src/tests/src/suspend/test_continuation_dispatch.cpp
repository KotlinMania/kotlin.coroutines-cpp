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
    suspended->resume(42, [&](std::exception_ptr) { ++cancelled_values; });
    suspended.reset();
    job->cancel();
    dispatcher->drain();
    assert_equals(1, cancelled_values);
    assert_equals(1, parent->resumes);
    assert_false(parent->result.is_success());
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

int main() {
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
