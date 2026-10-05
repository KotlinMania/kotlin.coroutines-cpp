/**
 * Transliterated from: kotlinx-coroutines-core/common/test/flow/sharing/SharingStartedTest.kt:39-100
 * Additional continuation regressions exercise suspended command emission and failure.
 */
#include "kotlinx/coroutines/flow/Share.hpp"
#include "kotlinx/coroutines/flow/SharingStarted.hpp"
#include "kotlinx/coroutines/flow/internal/SafeCollector.hpp"
#include "kotlinx/coroutines/CancellableContinuationImpl.hpp"
#include "kotlinx/coroutines/Delay.hpp"
#include "kotlinx/coroutines/NonCancellable.hpp"
#include "kotlinx/coroutines/flow/FlowBuilders.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include "kotlinx/coroutines/testing/TestBase.hpp"
#include <deque>
#include <limits>
#include <map>
#include <utility>
#include <vector>
#include <iostream>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::flow;
using namespace kotlinx::coroutines::testing;

// Deterministic single-thread fixture implementing the actual Dispatcher/Delay interfaces.
class VirtualDispatcher final : public CoroutineDispatcher, public Delay {
public:
    long long now = 0;

    bool is_dispatch_needed(const CoroutineContext&) const override { return true; }
    void dispatch(const CoroutineContext&, std::shared_ptr<Runnable> block) const override {
        queue_.push_back(std::move(block));
    }
    void schedule_resume_after_delay(long long millis, CancellableContinuation<void>& continuation) override {
        auto retained = dynamic_cast<CancellableContinuationImpl<void>&>(continuation).shared_from_this();
        timers_.emplace(now + millis, [retained] {
            if (retained->is_active()) retained->resume(nullptr);
        });
    }
    std::shared_ptr<DisposableHandle> invoke_on_timeout(
        long long millis, std::shared_ptr<Runnable> block, const CoroutineContext&) override {
        struct Handle final : DisposableHandle {
            bool cancelled = false;
            void dispose() override { cancelled = true; }
        };
        auto handle = std::make_shared<Handle>();
        timers_.emplace(now + millis, [handle, block] {
            if (!handle->cancelled) block->run();
        });
        return handle;
    }
    void run_current() {
        int steps = 0;
        while (!queue_.empty()) {
            assert_true(++steps < 10000);
            auto task = std::move(queue_.front());
            queue_.pop_front();
            task->run();
        }
    }
    void advance_by(long long millis) {
        const auto target = now + millis;
        run_current();
        while (!timers_.empty() && timers_.begin()->first <= target) {
            auto it = timers_.begin();
            now = it->first;
            auto action = std::move(it->second);
            timers_.erase(it);
            action();
            run_current();
        }
        now = target;
        run_current();
    }

private:
    mutable std::deque<std::shared_ptr<Runnable>> queue_;
    std::multimap<long long, std::function<void()>> timers_;
};

class Commands {
public:
    std::shared_ptr<VirtualDispatcher> clock = std::make_shared<VirtualDispatcher>();
    std::shared_ptr<Job> job = make_job();
    std::shared_ptr<MutableStateFlow<int>> count = make_mutable_state_flow(0);
    std::vector<std::pair<long long, SharingCommand>> events;
    int completions = 0;
    std::exception_ptr failure;
    bool pause_stop = false;
    Continuation<void*>* stopped_emit = nullptr;

    Commands(long long stop, long long expiration) : started_(stop, expiration), recorder_(*this) {
        auto context = std::dynamic_pointer_cast<CoroutineContext>(job)->operator+(clock);
        completion_ = std::make_shared<FunctionalContinuation<void*>>(context, [this](Result<void*> result) {
            ++completions;
            failure = result.exception_or_null();
        });
        flow_ = started_.command(count);
        assert_true(intrinsics::is_coroutine_suspended(flow_->collect(&recorder_, completion_.get())));
        clock->run_current();
    }
    ~Commands() { stop(); }
    void subscriptions(int value) { count->set_value(value); clock->run_current(); }
    void stop() { job->cancel(); clock->run_current(); }
    void resume_stop(Result<void*> result = Result<void*>::success(nullptr)) {
        auto* completion = stopped_emit;
        assert_true(completion != nullptr);
        stopped_emit = nullptr;
        completion->resume_with(std::move(result));
        clock->run_current();
    }

private:
    class Recorder final : public FlowCollector<SharingCommand> {
    public:
        explicit Recorder(Commands& owner) : owner_(owner) {}
        void* emit(SharingCommand command, Continuation<void*>* completion) override {
            owner_.events.emplace_back(owner_.clock->now, command);
            if (command == SharingCommand::STOP && owner_.pause_stop) {
                owner_.stopped_emit = completion;
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
            return nullptr;
        }
    private:
        Commands& owner_;
    };
    StartedWhileSubscribed started_;
    Recorder recorder_;
    std::shared_ptr<Flow<SharingCommand>> flow_;
    std::shared_ptr<Continuation<void*>> completion_;
};

// Transliterated from: kotlinx-coroutines-core/common/test/flow/sharing/SharingStartedTest.kt:75-100
void test_stop_and_expiration_with_resubscription() {
    Commands commands(50, 100);
    commands.clock->advance_by(200);
    assert_true(commands.events.empty()); // Suppress STOP/RESET before the first START.
    commands.subscriptions(1);
    commands.subscriptions(0);
    commands.clock->advance_by(49);
    assert_equals(size_t(1), commands.events.size());
    commands.subscriptions(1); // Cancel the pending STOP.
    commands.clock->advance_by(200);
    assert_equals(size_t(1), commands.events.size()); // Duplicate START is suppressed.
    commands.subscriptions(0);
    commands.clock->advance_by(50);
    assert_true(commands.events.back() == std::make_pair(499LL, SharingCommand::STOP));
    commands.clock->advance_by(99);
    assert_equals(size_t(2), commands.events.size());
    commands.subscriptions(1); // Cancel pending cache expiration.
    commands.subscriptions(0);
    commands.clock->advance_by(50);
    commands.clock->advance_by(100);
    assert_true(commands.events.back() == std::make_pair(748LL, SharingCommand::STOP_AND_RESET_REPLAY_CACHE));
    commands.stop();
    assert_equals(1, commands.completions);
}

// Transliterated from: kotlinx-coroutines-core/common/test/flow/sharing/SharingStartedTest.kt:39-45
void test_zero_expiration() {
    Commands commands(50, 0);
    commands.subscriptions(1);
    commands.subscriptions(0);
    commands.clock->advance_by(49);
    assert_equals(size_t(1), commands.events.size());
    commands.clock->advance_by(1);
    assert_true(commands.events.back() == std::make_pair(50LL, SharingCommand::STOP_AND_RESET_REPLAY_CACHE));
    assert_equals(size_t(2), commands.events.size());
    commands.stop();
    assert_equals(1, commands.completions);
}

void test_stop_emission_suspends_before_expiration_delay() {
    Commands commands(50, 100);
    commands.pause_stop = true;
    commands.subscriptions(1);
    commands.subscriptions(0);
    commands.clock->advance_by(50);
    assert_true(commands.stopped_emit != nullptr);
    commands.clock->advance_by(500);
    assert_equals(size_t(2), commands.events.size());
    commands.resume_stop();
    commands.clock->advance_by(99);
    assert_equals(size_t(2), commands.events.size());
    commands.clock->advance_by(1);
    assert_true(commands.events.back() == std::make_pair(650LL, SharingCommand::STOP_AND_RESET_REPLAY_CACHE));
    commands.stop();
    assert_equals(1, commands.completions);
}

void test_resumed_failure_stops_the_sequence() {
    Commands commands(50, 100);
    commands.pause_stop = true;
    commands.subscriptions(1);
    commands.subscriptions(0);
    commands.clock->advance_by(50);
    commands.resume_stop(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("STOP emission failed"))));
    commands.clock->advance_by(1000);
    assert_equals(size_t(2), commands.events.size());
    assert_equals(1, commands.completions);
    assert_true(commands.failure != nullptr);
    try {
        std::rethrow_exception(commands.failure);
    } catch (const std::runtime_error& error) {
        assert_true(std::string(error.what()) == "STOP emission failed");
    }
}

void test_infinite_stop_timeout_is_cancellable() {
    Commands commands(std::numeric_limits<long long>::max(), 0);
    commands.subscriptions(1);
    commands.subscriptions(0);
    commands.clock->advance_by(1000);
    assert_equals(size_t(1), commands.events.size());
    commands.stop();
    assert_equals(1, commands.completions);
}

// Continuation regression: cancel must join the child's suspending finally before launch.
void test_latest_waits_for_suspended_cleanup() {
    auto clock = std::make_shared<VirtualDispatcher>();
    auto job = make_job();
    auto context = std::dynamic_pointer_cast<CoroutineContext>(job)->operator+(clock);
    std::vector<int> events;
    int completions = 0;
    std::exception_ptr failure;
    auto completion = std::make_shared<FunctionalContinuation<void*>>(context, [&](Result<void*> result) {
        ++completions;
        failure = result.exception_or_null();
    });

    class CleanupFrame final : public ContinuationImpl {
    public:
        CleanupFrame(std::vector<int>& events, Continuation<void*>* completion)
            : ContinuationImpl(std::shared_ptr<Continuation<void*>>(completion, [](Continuation<void*>*) {})),
              events_(events), cleanup_context_(get_context()->operator+(non_cancellable())) {}
        void retain() { self_ref_ = shared_from_this(); }
        std::shared_ptr<CoroutineContext> get_context() const override {
            return cleaning_ ? cleanup_context_ : ContinuationImpl::get_context();
        }
        void* invoke_suspend(Result<void*> result) override {
            try {
                if (state_ == 0) {
                    state_ = 1;
                    try {
                        return await_cancellation(shared_from_this());
                    } catch (...) {
                        result = Result<void*>::failure(std::current_exception());
                    }
                }
                if (state_ == 1) {
                    assert_false(result.is_success());
                    cancellation_ = result.exception_or_null();
                    cleaning_ = true;
                    events_.push_back(2);
                    state_ = 2;
                    void* delayed = delay(50, shared_from_this());
                    if (intrinsics::is_coroutine_suspended(delayed)) return delayed;
                } else {
                    (void)result.get_or_throw();
                }
                events_.push_back(3);
                std::rethrow_exception(cancellation_);
            } catch (...) {
                self_ref_.reset();
                throw;
            }
        }
    private:
        int state_ = 0;
        std::vector<int>& events_;
        bool cleaning_ = false;
        std::exception_ptr cancellation_;
        std::shared_ptr<CoroutineContext> cleanup_context_;
        std::shared_ptr<BaseContinuationImpl> self_ref_;
    };

    auto values = flow_of<int>({1, 2});
    auto result = collect_latest<int>(values, [&](int value, Continuation<void*>* completion) -> void* {
        if (value == 2) {
            events.push_back(4);
            return nullptr;
        }
        events.push_back(1);
        auto frame = std::make_shared<CleanupFrame>(events, completion);
        frame->retain();
        return frame->invoke_suspend(Result<void*>::success(nullptr));
    }, completion.get());
    assert_true(intrinsics::is_coroutine_suspended(result));
    clock->run_current();
    assert_true(events == std::vector<int>({1, 2}));
    assert_equals(0, completions);
    clock->advance_by(49);
    assert_true(events == std::vector<int>({1, 2}));
    clock->advance_by(1);
    assert_true(events == std::vector<int>({1, 2, 3, 4}));
    assert_equals(1, completions);
    assert_true(failure == nullptr);
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/SharingStarted.kt:148-158
void test_lazy_command_starts_once_and_releases_on_cancellation() {
    auto clock = std::make_shared<VirtualDispatcher>();
    auto job = make_job();
    auto context = std::dynamic_pointer_cast<CoroutineContext>(job)->operator+(clock);
    auto counts = make_mutable_state_flow(0);
    int starts = 0;
    int completions = 0;
    class Recorder final : public FlowCollector<SharingCommand> {
    public:
        explicit Recorder(int& starts) : starts_(starts) {}
        void* emit(SharingCommand command, Continuation<void*>*) override {
            assert_true(command == SharingCommand::START);
            ++starts_;
            return nullptr;
        }
    private:
        int& starts_;
    } recorder(starts);
    auto completion = std::make_shared<FunctionalContinuation<void*>>(context, [&](Result<void*> result) {
        assert_false(result.is_success());
        ++completions;
    });
    auto commands = SharingStarted::lazily()->command(counts);
    assert_true(intrinsics::is_coroutine_suspended(commands->collect(&recorder, completion.get())));
    assert_equals(0, starts);
    counts->set_value(1);
    clock->run_current();
    counts->set_value(0);
    clock->run_current();
    counts->set_value(1);
    clock->run_current();
    assert_equals(1, starts);
    job->cancel();
    clock->run_current();
    assert_equals(1, completions);
}

// Regression for the deferred sharing launch: delay the first value, update,
// then remain suspended until cancellation or fail after the update.
class DeferredUpstreamFrame final : public ContinuationImpl {
public:
    DeferredUpstreamFrame(FlowCollector<int>* collector, bool empty, bool fail,
                          Continuation<void*>* completion)
        : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
          collector_(collector), empty_(empty), fail_(fail) {}
    void retain() { self_ref_ = shared_from_this(); }
    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)
            coroutine_yield(this, delay(10, shared_from_this()));
            if (empty_) {
                if (fail_) throw std::runtime_error("upstream failed");
            } else {
                coroutine_yield(this, collector_->emit(10, this));
                coroutine_yield(this, delay(10, shared_from_this()));
                coroutine_yield(this, collector_->emit(20, this));
                if (fail_) throw std::runtime_error("upstream failed");
                coroutine_yield(this, await_cancellation(shared_from_this()));
            }
            self_ref_.reset();
            coroutine_end(this)
        } catch (...) {
            self_ref_.reset();
            throw;
        }
    }
private:
    void* _label = nullptr;
    FlowCollector<int>* collector_;
    bool empty_;
    bool fail_;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};

// Transliterated from: kotlinx-coroutines-core/common/src/flow/operators/Share.kt:333-353
void test_deferred_sharing_retains_collection(bool empty, bool fail, bool cancel_before_first = false) {
    auto clock = std::make_shared<VirtualDispatcher>();
    auto parent = make_job();
    auto scope = create_coroutine_scope(
        std::dynamic_pointer_cast<CoroutineContext>(parent)->operator+(clock));
    auto deferred = make_completable_deferred<Result<std::shared_ptr<StateFlow<int>>>>(parent);
    std::weak_ptr<BaseContinuationImpl> upstream_frame;
    std::weak_ptr<BaseContinuationImpl> sharing_frame;
    std::weak_ptr<Job> child;
    auto upstream = flow::flow<int>([&](FlowCollector<int>* collector, Continuation<void*>* cont) {
        sharing_frame = dynamic_cast<BaseContinuationImpl*>(cont)->shared_from_this();
        child = std::dynamic_pointer_cast<Job>(cont->get_context()->get(Job::type_key));
        assert_true(child.lock() != parent);
        auto frame = std::make_shared<DeferredUpstreamFrame>(collector, empty, fail, cont);
        upstream_frame = frame;
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
    launch_sharing_deferred<int>(scope.get(), EmptyCoroutineContext::instance(), upstream, deferred);
    upstream.reset();
    clock->run_current();
    assert_false(deferred->is_completed());
    assert_false(sharing_frame.expired());
    clock->advance_by(9);
    assert_false(deferred->is_completed());
    if (cancel_before_first) {
        auto sharing_child = child.lock();
        assert_true(sharing_child != nullptr);
        parent->cancel();
        clock->advance_by(1);
        assert_true(deferred->is_cancelled());
        assert_true(sharing_child->is_completed());
        assert_true(upstream_frame.expired());
        assert_true(sharing_frame.expired());
        return;
    }
    clock->advance_by(1);
    assert_true(deferred->is_completed());
    if (empty) {
        bool caught = false;
        try {
            (void)deferred->get_completed().get_or_throw();
        } catch (const std::out_of_range& error) {
            assert_false(fail);
            assert_true(std::string(error.what()) == "Flow is empty");
            caught = true;
        } catch (const std::runtime_error& error) {
            assert_true(fail);
            assert_true(std::string(error.what()) == "upstream failed");
            caught = true;
        }
        assert_true(caught);
        assert_true(parent->is_cancelled() == fail);
    } else {
        auto state = deferred->get_completed().get_or_throw();
        assert_equals(10, state->value());
        clock->advance_by(10);
        assert_equals(20, state->value());
        assert_true(deferred->get_completed().get_or_throw() == state);
        assert_true(parent->is_cancelled() == fail);
        if (!fail) {
            auto sharing_child = child.lock();
            assert_true(sharing_child != nullptr);
            assert_false(sharing_child->is_completed());
            sharing_child->cancel();
            clock->run_current();
            assert_true(sharing_child->is_completed());
            assert_true(parent->is_active());
        }
    }
    assert_true(upstream_frame.expired());
    assert_true(sharing_frame.expired());
    parent->cancel();
    clock->run_current();
}

void test_completable_deferred_typed_completion() {
    auto completed = make_completable_deferred<int>(42);
    assert_equals(42, completed->get_completed());
    assert_equals(42, completed->await_blocking());
    assert_true(completed->get_completion_exception_or_null() == nullptr);
    assert_false(completed->complete(99));
    assert_equals(42, completed->get_completed());
    auto parent = make_job();
    auto pending = make_completable_deferred<int>(parent);
    parent->cancel();
    assert_true(pending->is_completed());
    assert_true(pending->get_completion_exception_or_null() != nullptr);
}

// Await completion is dispatched, and cancellation wins over a queued value.
void test_deferred_await_dispatch_and_cancellation(int cancellation) {
    auto clock = std::make_shared<VirtualDispatcher>();
    auto waiter = make_job();
    auto context = std::dynamic_pointer_cast<CoroutineContext>(waiter)->operator+(clock);
    auto deferred = make_completable_deferred<int>();
    std::weak_ptr<CompletableDeferred<int>> weak_deferred = deferred;
    int completions = 0;
    int value = 0;
    std::exception_ptr failure;
    auto completion = std::make_shared<FunctionalContinuation<void*>>(context, [&](Result<void*> result) {
        ++completions;
        failure = result.exception_or_null();
        if (result.is_success()) {
            std::unique_ptr<int> box(static_cast<int*>(result.get_or_throw()));
            value = *box;
        }
    });
    assert_true(intrinsics::is_coroutine_suspended(deferred->await(completion.get())));
    if (cancellation == 1) {
        waiter->cancel();
        clock->run_current();
        assert_equals(1, completions);
        assert_true(failure != nullptr);
        assert_true(deferred->is_active());
        completion.reset(); // A disposed await handler must never call it again.
    }
    deferred->complete(42);
    if (cancellation == 2) waiter->cancel();
    if (cancellation != 1) assert_equals(0, completions);
    clock->run_current();
    assert_equals(1, completions);
    assert_true((failure != nullptr) == (cancellation != 0));
    if (cancellation == 0) assert_equals(42, value);
    deferred.reset();
    assert_true(weak_deferred.expired(), "await must release the deferred after resume/cancellation");
    waiter->cancel();
    clock->run_current();
}

void test_deferred_await_preserves_child_failure() {
    auto clock = std::make_shared<VirtualDispatcher>();
    auto parent = make_job();
    auto context = std::dynamic_pointer_cast<CoroutineContext>(parent)->operator+(clock);
    auto deferred = make_completable_deferred<int>(parent);
    int completions = 0;
    std::exception_ptr failure;
    auto completion = std::make_shared<FunctionalContinuation<void*>>(context, [&](Result<void*> result) {
        ++completions;
        failure = result.exception_or_null();
    });
    assert_true(intrinsics::is_coroutine_suspended(deferred->await(completion.get())));
    deferred->complete_exceptionally(std::make_exception_ptr(std::runtime_error("child failure")));
    assert_equals(0, completions);
    clock->run_current();
    assert_equals(1, completions);
    assert_true(parent->is_cancelled());
    assert_true(failure != nullptr);
    try {
        std::rethrow_exception(failure);
    } catch (const std::runtime_error& error) {
        assert_true(std::string(error.what()) == "child failure");
    }
}

void test_deferred_await_immediate_value_and_failure() {
    auto completed = make_completable_deferred<int>(42);
    auto cancelled = make_job();
    cancelled->cancel();
    int completions = 0;
    auto completion = std::make_shared<FunctionalContinuation<void*>>(
        std::dynamic_pointer_cast<CoroutineContext>(cancelled), [&](Result<void*>) { ++completions; });
    std::unique_ptr<int> value(static_cast<int*>(completed->await(completion.get())));
    assert_equals(42, *value); // Kotlin's completed fast path does not check the caller's job.
    auto failed = make_completable_deferred<int>();
    failed->complete_exceptionally(std::make_exception_ptr(std::runtime_error("already failed")));
    bool caught = false;
    try {
        (void)failed->await(completion.get());
    } catch (const std::runtime_error& error) {
        assert_true(std::string(error.what()) == "already failed");
        caught = true;
    }
    assert_true(caught);
    assert_equals(0, completions);
}

// Exercise state_in through its public erased result ABI, including resumed failure.
void test_state_in_await_unwraps(bool empty, bool fail, bool cancel_waiter = false) {
    auto clock = std::make_shared<VirtualDispatcher>();
    auto sharing_parent = make_job();
    auto waiter = make_job();
    auto scope = create_coroutine_scope(
        std::dynamic_pointer_cast<CoroutineContext>(sharing_parent)->operator+(clock));
    auto context = std::dynamic_pointer_cast<CoroutineContext>(waiter)->operator+(clock);
    std::shared_ptr<StateFlow<int>> state;
    int completions = 0;
    std::exception_ptr failure;
    auto completion = std::make_shared<FunctionalContinuation<void*>>(context, [&](Result<void*> result) {
        ++completions;
        failure = result.exception_or_null();
        if (result.is_success()) {
            std::unique_ptr<std::shared_ptr<StateFlow<int>>> box(
                static_cast<std::shared_ptr<StateFlow<int>>*>(result.get_or_throw()));
            state = *box;
        }
    });
    std::weak_ptr<Continuation<void*>> weak_completion = completion;
    std::weak_ptr<BaseContinuationImpl> upstream_frame;
    auto upstream = flow::flow<int>([&](FlowCollector<int>* collector, Continuation<void*>* cont) {
        auto frame = std::make_shared<DeferredUpstreamFrame>(collector, empty, fail, cont);
        upstream_frame = frame;
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    });
    assert_true(intrinsics::is_coroutine_suspended(state_in<int>(upstream, scope.get(), completion)));
    completion.reset(); // The public shared completion must survive until its single resume.
    upstream.reset();
    clock->run_current();
    assert_equals(0, completions);
    if (cancel_waiter) {
        waiter->cancel();
        clock->run_current();
        assert_equals(1, completions);
        assert_true(failure != nullptr);
        assert_true(weak_completion.expired(), "cancelled state_in must release its completion");
        assert_true(sharing_parent->is_active()); // Await cancellation does not cancel the sharing scope.
    }
    clock->advance_by(10);
    assert_equals(1, completions);
    assert_true(weak_completion.expired(), "completed state_in must release its completion");
    if (!cancel_waiter) {
        assert_true((failure != nullptr) == empty);
        if (empty) {
            try { std::rethrow_exception(failure); }
            catch (const std::out_of_range& error) {
                assert_false(fail);
                assert_true(std::string(error.what()) == "Flow is empty");
            } catch (const std::runtime_error& error) {
                assert_true(fail);
                assert_true(std::string(error.what()) == "upstream failed");
            }
        } else {
            assert_equals(10, state->value());
            clock->advance_by(10);
            assert_equals(20, state->value());
        }
    }
    sharing_parent->cancel();
    clock->advance_by(10);
    assert_true(upstream_frame.expired(), "state_in cancellation must release upstream collection");
    assert_equals(1, completions);
    waiter->cancel();
    clock->run_current();
}

void test_state_in_immediate_result() {
    auto parent = make_job();
    auto unconfined = std::shared_ptr<CoroutineContext>(
        &Dispatchers::get_unconfined(), [](CoroutineContext*) {});
    auto scope = create_coroutine_scope(
        std::dynamic_pointer_cast<CoroutineContext>(parent)->operator+(unconfined));
    int completions = 0;
    auto completion = std::make_shared<FunctionalContinuation<void*>>(
        scope->get_coroutine_context(), [&](Result<void*>) { ++completions; });
    auto result = state_in<int>(flow_of<int>({42}), scope.get(), completion);
    assert_false(intrinsics::is_coroutine_suspended(result));
    std::unique_ptr<std::shared_ptr<StateFlow<int>>> state(
        static_cast<std::shared_ptr<StateFlow<int>>*>(result));
    assert_equals(42, (*state)->value());
    bool caught = false;
    try {
        (void)state_in<int>(flow_of<int>({}), scope.get(), completion);
    } catch (const std::out_of_range& error) {
        assert_true(std::string(error.what()) == "Flow is empty");
        caught = true;
    }
    assert_true(caught);
    assert_equals(0, completions);
    assert_true(parent->is_active());
    parent->cancel();
}

void test_exception_resume_after_waiter_cancellation() {
    auto clock = std::make_shared<VirtualDispatcher>();
    auto waiter = make_job();
    auto context = std::dynamic_pointer_cast<CoroutineContext>(waiter)->operator+(clock);
    std::shared_ptr<CancellableContinuationImpl<int>> suspended;
    int completions = 0;
    std::exception_ptr failure;
    auto completion = std::make_shared<FunctionalContinuation<void*>>(context, [&](Result<void*> result) {
        ++completions;
        failure = result.exception_or_null();
    });
    assert_true(intrinsics::is_coroutine_suspended(suspend_cancellable_coroutine<int>(
        [&](CancellableContinuation<int>& cont) {
            suspended = dynamic_cast<CancellableContinuationImpl<int>&>(cont).shared_from_this();
        }, completion)));
    waiter->cancel();
    suspended->resume_with(Result<int>::failure(std::make_exception_ptr(std::runtime_error("late failure"))));
    clock->run_current();
    assert_equals(1, completions);
    assert_true(failure != nullptr);
    try { std::rethrow_exception(failure); }
    catch (const CancellationException&) {}
}

class SubscriptionActionFrame final : public ContinuationImpl {
public:
    SubscriptionActionFrame(FlowCollector<int>* collector, int value, bool wait, bool fail,
                            std::function<void(int)> after_emit,
                            std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)), collector_(collector), value_(value),
          wait_(wait), fail_(fail), after_emit_(std::move(after_emit)) {}
    void retain() { self_ref_ = shared_from_this(); }
    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)
            if (wait_) coroutine_yield(this, delay(10, shared_from_this()));
            coroutine_yield(this, collector_->emit(value_, this));
            if (fail_) throw std::runtime_error("subscription failed");
            after_emit_(value_);
            self_ref_.reset();
            coroutine_end(this)
        } catch (...) {
            self_ref_.reset();
            throw;
        }
    }
private:
    void* _label = nullptr;
    FlowCollector<int>* collector_;
    int value_;
    bool wait_;
    bool fail_;
    std::function<void(int)> after_emit_;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};

// Transliterated from: kotlinx-coroutines-core/common/test/flow/sharing/SharedFlowTest.kt:562-621
// Suspension regressions also cover StateFlow's subscription-before-current-value order.
void test_subscription_actions_and_collector_lifetime(bool state_flow, bool fail, bool cancel_action = false) {
    auto clock = std::make_shared<VirtualDispatcher>();
    auto job = make_job();
    auto context = std::dynamic_pointer_cast<CoroutineContext>(job)->operator+(clock);
    std::shared_ptr<MutableSharedFlow<int>> shared;
    if (state_flow) shared = make_mutable_state_flow<int>(42);
    else {
        shared = make_mutable_shared_flow<int>(1, 4);
        shared->try_emit(42);
    }
    std::vector<int> received;
    std::vector<int> started;
    std::vector<std::weak_ptr<BaseContinuationImpl>> action_frames;
    std::vector<std::weak_ptr<BaseContinuationImpl>> subscription_frames;
    auto token = std::make_shared<int>(1);
    std::weak_ptr<int> weak_token = token;
    auto action = [&](int value, FlowCollector<int>* collector, std::shared_ptr<Continuation<void*>> cont) {
        assert_true(dynamic_cast<kotlinx::coroutines::flow::internal::SafeCollector<int>*>(collector) != nullptr);
        assert_true(cont != nullptr);
        started.push_back(value);
        subscription_frames.push_back(std::dynamic_pointer_cast<BaseContinuationImpl>(cont));
        auto frame = std::make_shared<SubscriptionActionFrame>(collector, value, value == 1,
            fail && value == 2, [&](int v) { shared->try_emit(v * 100); }, std::move(cont));
        action_frames.push_back(frame);
        frame->retain();
        return frame->start(Result<void*>::success(nullptr));
    };
    auto subscribed = on_subscription<int>(on_subscription<int>(shared,
        [&, token](FlowCollector<int>* collector, std::shared_ptr<Continuation<void*>> cont) {
            assert_equals(1, *token);
            return action(1, collector, std::move(cont));
        }), [&, token](FlowCollector<int>* collector, std::shared_ptr<Continuation<void*>> cont) {
            assert_equals(1, *token);
            return action(2, collector, std::move(cont));
        });
    class Recorder final : public FlowCollector<int> {
    public:
        Recorder(std::vector<int>& received) : received_(received) {}
        void* emit(int value, Continuation<void*>* cont) override {
            received_.push_back(value);
            if (value == 1) return delay(5, kotlinx::coroutines::internal::retain_continuation(cont));
            return nullptr;
        }
    private:
        std::vector<int>& received_;
    } recorder(received);
    int completions = 0;
    std::exception_ptr failure;
    auto completion = std::make_shared<FunctionalContinuation<void*>>(context, [&](Result<void*> result) {
        ++completions;
        failure = result.exception_or_null();
    });
    assert_true(intrinsics::is_coroutine_suspended(subscribed->collect(&recorder, completion.get())));
    subscribed.reset();
    token.reset();
    assert_false(weak_token.expired());
    assert_equals(1, shared->subscription_count()->value());
    assert_true(started == std::vector<int>({1}));
    assert_true(received.empty());
    if (cancel_action) {
        job->cancel();
        clock->run_current();
        assert_true(received.empty());
        assert_true(started == std::vector<int>({1}));
        assert_equals(1, completions);
        assert_equals(0, shared->subscription_count()->value());
        clock->advance_by(10);
    } else {
        clock->advance_by(10);
        assert_true(received == std::vector<int>({1}));
        assert_true(started == std::vector<int>({1}));
        clock->advance_by(4);
        assert_true(started == std::vector<int>({1}));
        clock->advance_by(1);
        assert_true(started == std::vector<int>({1, 2}));
        if (fail) {
            assert_true(received == std::vector<int>({1, 2})); // Action failure suppresses replay/current value.
            assert_equals(1, completions);
            assert_equals(0, shared->subscription_count()->value());
            assert_true(failure != nullptr);
            try { std::rethrow_exception(failure); }
            catch (const std::runtime_error& error) {
                assert_true(std::string(error.what()) == "subscription failed");
            }
        } else {
            assert_equals(0, completions);
            assert_true(received == (state_flow ? std::vector<int>({1, 2, 200})
                                               : std::vector<int>({1, 2, 42, 100, 200})));
            for (auto& frame : action_frames) assert_true(frame.expired());
            for (auto& frame : subscription_frames) assert_true(frame.expired());
            assert_false(weak_token.expired()); // Collectors retain their captured action while waiting.
            job->cancel();
            clock->run_current();
            assert_equals(1, completions);
            assert_equals(0, shared->subscription_count()->value());
        }
    }
    for (auto& frame : action_frames) assert_true(frame.expired());
    for (auto& frame : subscription_frames) assert_true(frame.expired());
    assert_true(weak_token.expired(), "collection completion must release the subscribed collectors");
    assert_true(failure != nullptr);
    job->cancel();
    clock->run_current();
}

void test_subscription_safe_collector_rejects_other_job() {
    auto shared = make_mutable_shared_flow<int>(1, 1);
    shared->try_emit(42);
    auto job = make_job();
    auto alien = make_job();
    auto wrong_context = std::make_shared<FunctionalContinuation<void*>>(
        std::dynamic_pointer_cast<CoroutineContext>(alien), [](Result<void*>) {});
    auto subscribed = on_subscription<int>(shared,
        [&](FlowCollector<int>* collector, std::shared_ptr<Continuation<void*>>) {
            return collector->emit(1, wrong_context.get());
        });
    class Recorder final : public FlowCollector<int> {
    public:
        int emissions = 0;
        void* emit(int, Continuation<void*>*) override { ++emissions; return nullptr; }
    } recorder;
    auto completion = std::make_shared<FunctionalContinuation<void*>>(
        std::dynamic_pointer_cast<CoroutineContext>(job), [](Result<void*>) {});
    bool caught = false;
    try { (void)subscribed->collect(&recorder, completion.get()); }
    catch (const IllegalStateException&) { caught = true; }
    assert_true(caught);
    assert_equals(0, recorder.emissions);
    assert_equals(0, shared->subscription_count()->value());
    job->cancel();
    alien->cancel();
}

// A hot StateFlow must own its suspended continuation even without a parent Job.
void test_state_flow_without_job_retains_waiter_and_reuses_slot() {
    auto clock = std::make_shared<VirtualDispatcher>();
    auto state = make_mutable_state_flow<int>(1);
    assert_true(clock->get(Job::type_key) == nullptr);
    for (int iteration = 0; iteration < 2; ++iteration) {
        state->set_value(iteration * 10 + 1);
        std::vector<int> values;
        std::weak_ptr<BaseContinuationImpl> frame;
        class Recorder final : public FlowCollector<int> {
        public:
            Recorder(std::vector<int>& values, std::weak_ptr<BaseContinuationImpl>& frame)
                : values_(values), frame_(frame) {}
            void* emit(int value, Continuation<void*>* cont) override {
                frame_ = dynamic_cast<BaseContinuationImpl*>(cont)->shared_from_this();
                values_.push_back(value);
                if (values_.size() == 2) throw kotlinx::coroutines::flow::internal::AbortFlowException(this);
                return nullptr;
            }
        private:
            std::vector<int>& values_;
            std::weak_ptr<BaseContinuationImpl>& frame_;
        } recorder(values, frame);
        int completions = 0;
        std::exception_ptr failure;
        auto completion = std::make_shared<FunctionalContinuation<void*>>(clock, [&](Result<void*> result) {
            ++completions;
            failure = result.exception_or_null();
        });
        assert_true(intrinsics::is_coroutine_suspended(state->collect(&recorder, completion.get())));
        assert_equals(1, state->subscription_count()->value());
        assert_false(frame.expired());
        state->set_value(iteration * 10 + 2);
        state->set_value(iteration * 10 + 3);
        assert_true(values == std::vector<int>({iteration * 10 + 1}));
        assert_equals(0, completions);
        clock->run_current();
        assert_true(values == std::vector<int>({iteration * 10 + 1, iteration * 10 + 3}));
        assert_equals(1, completions);
        assert_equals(0, state->subscription_count()->value());
        assert_true(frame.expired());
        assert_true(failure != nullptr);
        try { std::rethrow_exception(failure); }
        catch (const kotlinx::coroutines::flow::internal::AbortFlowException& error) {
            assert_true(error.owner == &recorder);
        }
    }
}

// Controlled pending-before-install and install-before-wake paths use the real slot.
void test_state_flow_slot_pending_order() {
    kotlinx::coroutines::flow::internal::StateFlowSlot slot;
    assert_true(slot.allocate_locked(nullptr));
    assert_false(slot.allocate_locked(nullptr));
    auto clock = std::make_shared<VirtualDispatcher>();
    int completions = 0;
    auto completion = std::make_shared<FunctionalContinuation<void*>>(clock, [&](Result<void*> result) {
        std::unique_ptr<Unit> value(static_cast<Unit*>(result.get_or_throw()));
        ++completions;
    });
    slot.make_pending();
    auto immediate = slot.await_pending(completion.get());
    assert_false(intrinsics::is_coroutine_suspended(immediate));
    delete static_cast<Unit*>(immediate);
    assert_equals(0, completions);
    assert_true(slot.take_pending());
    assert_false(slot.take_pending());
    assert_true(intrinsics::is_coroutine_suspended(slot.await_pending(completion.get())));
    slot.make_pending();
    clock->run_current();
    assert_equals(1, completions);
    assert_false(slot.take_pending());
    assert_true(slot.free_locked(nullptr).empty());
    slot.make_pending(); // A freed slot ignores pending notifications.
    assert_true(slot.allocate_locked(nullptr));
    assert_false(slot.take_pending());
    slot.free_locked(nullptr);
}

// Suspended subscribers and queued emitters retain real continuations without a Job.
void test_shared_flow_without_job_reuses_waiting_slot() {
    auto clock = std::make_shared<VirtualDispatcher>();
    auto shared = make_mutable_shared_flow<int>(0, 2);
    for (int iteration = 0; iteration < 2; ++iteration) {
        class Recorder final : public FlowCollector<int> {
        public:
            std::vector<int> values;
            std::weak_ptr<BaseContinuationImpl> frame;
            void* emit(int value, Continuation<void*>* cont) override {
                frame = dynamic_cast<BaseContinuationImpl*>(cont)->shared_from_this();
                values.push_back(value);
                if (values.size() == 2) throw kotlinx::coroutines::flow::internal::AbortFlowException(this);
                return nullptr;
            }
        } recorder;
        int completions = 0;
        auto completion = std::make_shared<FunctionalContinuation<void*>>(clock, [&](Result<void*> result) {
            assert_true(result.is_failure());
            ++completions;
        });
        assert_true(intrinsics::is_coroutine_suspended(shared->collect(&recorder, completion.get())));
        assert_equals(1, shared->subscription_count()->value());
        assert_true(shared->try_emit(iteration * 10 + 1));
        assert_true(shared->try_emit(iteration * 10 + 2));
        assert_true(recorder.values.empty());
        assert_equals(0, completions);
        clock->run_current();
        assert_true(recorder.values == std::vector<int>({iteration * 10 + 1, iteration * 10 + 2}));
        assert_equals(1, completions);
        assert_equals(0, shared->subscription_count()->value());
        assert_true(recorder.frame.expired());
    }
}

void test_shared_flow_queued_emitter_ownership(int capacity, bool terminate_early) {
    auto clock = std::make_shared<VirtualDispatcher>();
    auto shared = make_mutable_shared_flow<std::shared_ptr<int>>(0, capacity);
    class Recorder final : public FlowCollector<std::shared_ptr<int>> {
    public:
        std::vector<int> values;
        Continuation<void*>* paused = nullptr;
        std::weak_ptr<BaseContinuationImpl> frame;
        bool terminate_early;
        explicit Recorder(bool early) : terminate_early(early) {}
        void* emit(std::shared_ptr<int> value, Continuation<void*>* cont) override {
            frame = dynamic_cast<BaseContinuationImpl*>(cont)->shared_from_this();
            values.push_back(*value);
            if (values.size() == 1) {
                paused = cont;
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
            if (values.size() == 3) throw kotlinx::coroutines::flow::internal::AbortFlowException(this);
            return nullptr;
        }
        void finish_pause() {
            auto* cont = paused;
            paused = nullptr;
            if (terminate_early) cont->resume_with(Result<void*>::failure(std::make_exception_ptr(
                kotlinx::coroutines::flow::internal::AbortFlowException(this))));
            else cont->resume_with(Result<void*>::success(nullptr));
        }
    } recorder(terminate_early);
    int collection_completions = 0;
    auto completion = std::make_shared<FunctionalContinuation<void*>>(clock, [&](Result<void*> result) {
        assert_true(result.is_failure());
        ++collection_completions;
    });
    assert_true(intrinsics::is_coroutine_suspended(shared->collect(&recorder, completion.get())));
    int emissions = 0;
    auto emit_completion = std::make_shared<FunctionalContinuation<void*>>(clock, [&](Result<void*> result) {
        std::unique_ptr<Unit> value(static_cast<Unit*>(result.get_or_throw()));
        ++emissions;
    });
    std::vector<std::weak_ptr<int>> tokens;
    auto emit = [&](int value) {
        auto token = std::make_shared<int>(value);
        tokens.push_back(token);
        auto result = shared->emit(token, emit_completion.get());
        if (!intrinsics::is_coroutine_suspended(result)) {
            delete static_cast<Unit*>(result);
            ++emissions;
        }
    };
    emit(1);
    clock->run_current();
    assert_true(recorder.values == std::vector<int>({1}));
    assert_equals(1, emissions);
    emit(2);
    emit(3);
    assert_equals(capacity == 0 ? 1 : 2, emissions);
    assert_false(tokens[0].expired()); // The collector is still using the first value.
    assert_false(tokens[1].expired());
    assert_false(tokens[2].expired());

    // Cancellation disposes a queued emitter before any collector takes its value.
    auto job = make_job();
    int cancelled_emissions = 0;
    std::exception_ptr cancelled_failure;
    auto cancel_completion = std::make_shared<FunctionalContinuation<void*>>(
        std::dynamic_pointer_cast<CoroutineContext>(job)->operator+(clock), [&](Result<void*> result) {
            ++cancelled_emissions;
            cancelled_failure = result.exception_or_null();
        });
    auto cancelled_value = std::make_shared<int>(4);
    std::weak_ptr<int> cancelled_token = cancelled_value;
    assert_true(intrinsics::is_coroutine_suspended(shared->emit(cancelled_value, cancel_completion.get())));
    cancelled_value.reset();
    assert_false(cancelled_token.expired());
    job->cancel();
    clock->run_current();
    assert_equals(1, cancelled_emissions);
    assert_true(cancelled_failure != nullptr);
    assert_true(cancelled_token.expired());
    assert_equals(capacity == 0 ? 1 : 2, emissions);

    recorder.finish_pause();
    clock->run_current();
    assert_equals(3, emissions); // Also resumes all queued emitters when the last collector leaves.
    assert_equals(1, cancelled_emissions);
    assert_equals(1, collection_completions);
    assert_equals(0, shared->subscription_count()->value());
    assert_true(recorder.frame.expired());
    assert_true(recorder.values == (terminate_early ? std::vector<int>({1}) : std::vector<int>({1, 2, 3})));
    for (const auto& token : tokens) assert_true(token.expired());
}

void test_shared_flow_replay_reference_release() {
    auto shared = make_mutable_shared_flow<std::shared_ptr<int>>(1);
    auto first = std::make_shared<int>(1);
    std::weak_ptr<int> old = first;
    assert_true(shared->try_emit(first));
    first.reset();
    assert_false(old.expired());
    auto second = std::make_shared<int>(2);
    std::weak_ptr<int> current = second;
    assert_true(shared->try_emit(second));
    second.reset();
    assert_true(old.expired());
    assert_false(current.expired());
    {
        auto replay = shared->get_replay_cache();
        assert_equals(1, static_cast<int>(replay.size()));
        assert_equals(2, *replay[0]);
        shared->reset_replay_cache();
        assert_false(current.expired()); // The replay snapshot owns its element.
    }
    assert_true(current.expired());
    auto last = std::make_shared<int>(3);
    std::weak_ptr<int> destroyed = last;
    assert_true(shared->try_emit(last));
    last.reset();
    shared.reset();
    assert_true(destroyed.expired());
}

int main() {
    test_stop_and_expiration_with_resubscription();
    test_zero_expiration();
    test_stop_emission_suspends_before_expiration_delay();
    test_resumed_failure_stops_the_sequence();
    test_infinite_stop_timeout_is_cancellable();
    test_latest_waits_for_suspended_cleanup();
    test_lazy_command_starts_once_and_releases_on_cancellation();
    test_deferred_sharing_retains_collection(false, false);
    test_deferred_sharing_retains_collection(false, true);
    test_deferred_sharing_retains_collection(true, false);
    test_deferred_sharing_retains_collection(true, true);
    test_deferred_sharing_retains_collection(false, false, true);
    test_completable_deferred_typed_completion();
    test_deferred_await_dispatch_and_cancellation(0);
    test_deferred_await_dispatch_and_cancellation(1);
    test_deferred_await_dispatch_and_cancellation(2);
    test_deferred_await_preserves_child_failure();
    test_deferred_await_immediate_value_and_failure();
    test_state_in_await_unwraps(false, false);
    test_state_in_await_unwraps(false, true);
    test_state_in_await_unwraps(true, false);
    test_state_in_await_unwraps(true, true);
    test_state_in_await_unwraps(false, false, true);
    test_state_in_immediate_result();
    test_exception_resume_after_waiter_cancellation();
    test_subscription_actions_and_collector_lifetime(false, false);
    test_subscription_actions_and_collector_lifetime(true, false);
    test_subscription_actions_and_collector_lifetime(false, true);
    test_subscription_actions_and_collector_lifetime(true, true);
    test_subscription_actions_and_collector_lifetime(false, false, true);
    test_subscription_actions_and_collector_lifetime(true, false, true);
    test_subscription_safe_collector_rejects_other_job();
    test_state_flow_without_job_retains_waiter_and_reuses_slot();
    test_state_flow_slot_pending_order();
    test_shared_flow_without_job_reuses_waiting_slot();
    test_shared_flow_queued_emitter_ownership(0, false);
    test_shared_flow_queued_emitter_ownership(1, false);
    test_shared_flow_queued_emitter_ownership(0, true);
    test_shared_flow_queued_emitter_ownership(1, true);
    test_shared_flow_replay_reference_release();
    std::cout << "Sharing suspension and virtual-time tests passed\n";
}
