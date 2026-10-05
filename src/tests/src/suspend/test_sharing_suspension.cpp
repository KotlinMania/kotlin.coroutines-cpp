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
    std::cout << "Sharing suspension and virtual-time tests passed\n";
}
