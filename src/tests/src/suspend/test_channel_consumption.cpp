// Source contracts: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:90-103,159-162,191-202;
// kotlinx-coroutines-core/common/src/flow/Channels.kt:104-108,119-134.
#include "kotlinx/coroutines/channels/Channels.hpp"
#include "kotlinx/coroutines/flow/Channels.hpp"
#include "kotlinx/coroutines/flow/internal/Combine.hpp"
#include "kotlinx/coroutines/native/Exceptions.hpp"
#include "kotlinx/coroutines/JobSupport.hpp"
#include "kotlinx/coroutines/CompletableJob.hpp"
#include <deque>
#include <iostream>
#include <memory>
#include <vector>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::channels;

namespace {
void require(bool condition, int line) {
    if (!condition) throw std::runtime_error("channel consumption check at " + std::to_string(line));
}
#define CHECK(condition) require((condition), __LINE__)

class RecordingChannel final : public BufferedChannel<int> {
public:
    RecordingChannel() : BufferedChannel<int>(8) {}
    int cancellations = 0;
    std::exception_ptr cancellation_cause;
    std::exception_ptr cancellation_failure;

    void cancel(std::exception_ptr cause = nullptr) override {
        ++cancellations;
        cancellation_cause = cause;
        if (cancellation_failure) std::rethrow_exception(cancellation_failure);
        BufferedChannel<int>::cancel(cause);
    }
};

class Completion final : public Continuation<void*> {
public:
    int resumes = 0;
    void* value = nullptr;
    std::exception_ptr failure;
    std::shared_ptr<CoroutineContext> get_context() const override {
        return EmptyCoroutineContext::instance();
    }
    void resume_with(Result<void*> result) override {
        ++resumes;
        failure = result.exception_or_null();
        if (!failure) value = result.get_or_throw();
    }
};

void check_wrapped(std::exception_ptr actual, std::exception_ptr original) {
    CHECK(actual && actual != original);
    try {
        std::rethrow_exception(actual);
    } catch (const CancellationException& exception) {
        CHECK(exception.get_cause() == original);
        CHECK(exception.get_message() == "Channel was consumed, consumer had failed");
    }
}

// Native Exceptions.kt:27-29: identity, type, message, Job and cause equality
// execute in source order, including virtual equality on the other operands.
void job_cancellation_equality_contract() {
    auto job = std::make_shared<JobSupport>(true);
    auto other_job = std::make_shared<JobSupport>(true);
    auto cause = std::make_exception_ptr(std::runtime_error("cause"));
    JobCancellationException first("cancelled", cause, job.get());
    JobCancellationException same("cancelled", cause, job.get());
    JobCancellationException message("different", cause, job.get());
    JobCancellationException owner("cancelled", cause, other_job.get());
    JobCancellationException distinct_cause("cancelled", std::make_exception_ptr(std::runtime_error("cause")), job.get());
    JobCancellationException no_cause("cancelled", nullptr, job.get());
    CancellationException base("cancelled", cause);
    std::runtime_error unrelated("cancelled");
    CHECK(first.equals(&first) && first.equals(&same) && same.equals(&first));
    CHECK(!first.equals(nullptr) && !first.equals(&base) && !first.equals(&unrelated));
    CHECK(!first.equals(&message) && !first.equals(&owner) && !first.equals(&distinct_cause) && !first.equals(&no_cause));
    CHECK(base.equals(&base) && !base.equals(&first) && !base.equals(nullptr));
    JobCancellationException second_no_cause("cancelled", nullptr, job.get());
    CHECK(no_cause.equals(&second_no_cause));
    CHECK(first.get_job() == job.get() && first.get_cause() == cause);

    class EqualCause final : public CancellationException {
    public:
        explicit EqualCause(int key, std::shared_ptr<int> calls, std::exception_ptr failure = nullptr)
            : CancellationException("cause"), key_(key), calls_(std::move(calls)), failure_(failure) {}
        bool equals(const std::exception* other) const override {
            ++*calls_;
            if (failure_) std::rethrow_exception(failure_);
            auto* value = dynamic_cast<const EqualCause*>(other);
            return value && value->key_ == key_;
        }
    private:
        int key_;
        std::shared_ptr<int> calls_;
        std::exception_ptr failure_;
    };
    auto left_calls = std::make_shared<int>(0);
    auto right_calls = std::make_shared<int>(0);
    auto left_cause = std::make_exception_ptr(EqualCause(8, left_calls));
    auto right_cause = std::make_exception_ptr(EqualCause(8, right_calls));
    JobCancellationException left("equal", left_cause, job.get());
    JobCancellationException right("equal", right_cause, job.get());
    CHECK(left.equals(&right) && *right_calls == 1 && *left_calls == 0);
    JobCancellationException same_cause("equal", right_cause, job.get());
    CHECK(right.equals(&same_cause) && *right_calls == 2);
    CHECK(right.equals(&right) && *right_calls == 2);
    auto equality_failure = std::make_exception_ptr(std::runtime_error("equality failure"));
    auto throwing_cause = std::make_exception_ptr(EqualCause(8, right_calls, equality_failure));
    JobCancellationException throwing("equal", throwing_cause, job.get());
    try {
        left.equals(&throwing);
        CHECK(false);
    } catch (...) { CHECK(std::current_exception() == equality_failure); }

    class EqualJob final : public JobSupport {
    public:
        explicit EqualJob(int key) : JobSupport(true), key_(key) {}
        mutable int calls = 0;
        bool equals(const CoroutineContext* other) const override {
            ++calls;
            auto* job = dynamic_cast<const EqualJob*>(other);
            return job && job->key_ == key_;
        }
    private:
        int key_;
    };
    auto left_job = std::make_shared<EqualJob>(5);
    auto right_job = std::make_shared<EqualJob>(5);
    JobCancellationException left_owner("equal", nullptr, left_job.get());
    JobCancellationException right_owner("equal", nullptr, right_job.get());
    CHECK(left_owner.equals(&right_owner) && right_job->calls == 1 && left_job->calls == 0);
    JobCancellationException same_owner("equal", nullptr, right_job.get());
    CHECK(right_owner.equals(&same_owner) && right_job->calls == 2);
    JobCancellationException wrong_message("unequal", nullptr, right_job.get());
    CHECK(!left_owner.equals(&wrong_message) && right_job->calls == 2);
    CHECK(left_job.use_count() == 1 && right_job.use_count() == 1);

    // Distinct JobCancellationException causes recursively use source equality.
    auto nested_left = std::make_exception_ptr(JobCancellationException("nested", nullptr, job.get()));
    auto nested_right = std::make_exception_ptr(JobCancellationException("nested", nullptr, job.get()));
    JobCancellationException outer_left("outer", nested_left, job.get());
    JobCancellationException outer_right("outer", nested_right, job.get());
    CHECK(outer_left.equals(&outer_right));
}

// ChannelFlow.kt:118-121 uses CoroutineScope.kt:279-288 directly: the
// scoped continuation's caller frame is the actual caller, with no intermediary.
void channel_scope_contract() {
    class FrameCompletion final : public Continuation<void*>, public kotlinx::coroutines::internal::CoroutineStackFrame {
    public:
        int resumes = 0;
        std::exception_ptr failure;
        std::shared_ptr<CoroutineContext> get_context() const override { return EmptyCoroutineContext::instance(); }
        void resume_with(Result<void*> result) override {
            ++resumes;
            failure = result.exception_or_null();
            if (!failure) CHECK(result.get_or_throw() == nullptr);
        }
        kotlinx::coroutines::internal::CoroutineStackFrame* get_caller_frame() const override { return nullptr; }
        kotlinx::coroutines::internal::StackTraceElement* get_stack_trace_element() const override { return nullptr; }
    };
    for (bool suspended : {false, true}) for (bool fails : {false, true}) {
        FrameCompletion caller;
        std::shared_ptr<Continuation<void*>> paused;
        auto failure = std::make_exception_ptr(std::runtime_error("scope source failure"));
        int starts = 0;
        std::exception_ptr observed;
        void* outcome = nullptr;
        try {
            outcome = flow::internal::collect_in_scope(
                [&](CoroutineScope* receiver, std::shared_ptr<Continuation<void*>> continuation) -> void* {
                    ++starts;
                    auto* scope = dynamic_cast<kotlinx::coroutines::internal::ScopeCoroutine<void*>*>(receiver);
                    CHECK(scope && scope == dynamic_cast<kotlinx::coroutines::internal::ScopeCoroutine<void*>*>(continuation.get()));
                    CHECK(scope->get_caller_frame() == &caller);
                    CHECK(scope->u_cont.get() == &caller);
                    CHECK(scope->get_coroutine_context()->get(Job::type_key).get() ==
                          static_cast<CoroutineContext::Element*>(scope));
                    if (suspended) {
                        paused = std::move(continuation);
                        return intrinsics::get_COROUTINE_SUSPENDED();
                    }
                    if (fails) std::rethrow_exception(failure);
                    return nullptr;
                }, &caller);
        } catch (...) { observed = std::current_exception(); }
        CHECK(starts == 1 && caller.resumes == 0);
        if (suspended) {
            CHECK(!observed && intrinsics::is_coroutine_suspended(outcome) && paused);
            paused->resume_with(fails ? Result<void*>::failure(failure) : Result<void*>::success(nullptr));
            CHECK(caller.resumes == 1 && caller.failure == (fails ? failure : nullptr));
        } else CHECK(observed == (fails ? failure : nullptr) && outcome == nullptr);
    }

    // A successful body still waits for its actual attached child before
    // returning to the original caller. Child failure becomes scope failure.
    for (bool fails : {false, true}) {
        FrameCompletion caller;
        std::shared_ptr<CompletableJob> child;
        auto failure = std::make_exception_ptr(std::runtime_error("scope child failure"));
        auto outcome = flow::internal::collect_in_scope(
            [&](CoroutineScope* receiver, std::shared_ptr<Continuation<void*>>) -> void* {
                child = make_job(receiver->get_job());
                return nullptr;
            }, &caller);
        CHECK(intrinsics::is_coroutine_suspended(outcome) && caller.resumes == 0 && child->is_active());
        if (fails) child->complete_exceptionally(failure);
        else child->complete();
        CHECK(caller.resumes == 1 && caller.failure == (fails ? failure : nullptr));
    }
    // Child completion must use the actual compiler caller's interceptor.
    // Wrapping that caller in a non-frame completion silently bypasses it.
    class QueueDispatcher final : public CoroutineDispatcher {
    public:
        mutable std::deque<std::shared_ptr<Runnable>> queue;
        void dispatch(const CoroutineContext&, std::shared_ptr<Runnable> task) const override {
            queue.push_back(std::move(task));
        }
        void drain() {
            while (!queue.empty()) {
                auto task = std::move(queue.front());
                queue.pop_front();
                task->run();
            }
        }
    };
    class CallerFrame final : public ContinuationImpl {
    public:
        CallerFrame(std::shared_ptr<Continuation<void*>> completion, std::shared_ptr<CoroutineContext> context)
            : ContinuationImpl(std::move(completion), std::move(context)) {}
        int resumes = 0;
        void* invoke_suspend(Result<void*> result) override {
            ++resumes;
            return result.get_or_throw();
        }
    };
    for (bool fails : {false, true}) {
        auto dispatcher = std::make_shared<QueueDispatcher>();
        auto outer = std::make_shared<Completion>();
        auto caller = std::make_shared<CallerFrame>(outer, dispatcher);
        std::shared_ptr<CompletableJob> child;
        auto failure = std::make_exception_ptr(std::runtime_error("intercepted child failure"));
        auto outcome = flow::internal::collect_in_scope(
            [&](CoroutineScope* receiver, std::shared_ptr<Continuation<void*>>) -> void* {
                child = make_job(receiver->get_job());
                return nullptr;
            }, caller.get());
        CHECK(intrinsics::is_coroutine_suspended(outcome) && caller->resumes == 0 && outer->resumes == 0);
        if (fails) child->complete_exceptionally(failure);
        else child->complete();
        CHECK(caller->resumes == 0 && outer->resumes == 0 && dispatcher->queue.size() == 1);
        dispatcher->drain();
        CHECK(caller->resumes == 1 && outer->resumes == 1 && outer->failure == (fails ? failure : nullptr));
    }
}

// SendingCollector.kt:12-15 delegates directly to the actual channel send.
void sending_collector_contract() {
    for (bool cancelled : {false, true}) {
        auto channel = create_channel<int>(0);
        auto failure = cancelled
            ? std::make_exception_ptr(CancellationException("original send cancellation"))
            : std::make_exception_ptr(std::runtime_error("original send failure"));
        if (cancelled) channel->cancel(failure);
        else channel->close(failure);
        flow::internal::SendingCollector<int> collector(channel.get());
        Completion completion;
        std::exception_ptr observed;
        try { collector.emit(17, &completion); }
        catch (...) { observed = std::current_exception(); }
        CHECK((observed == failure && completion.resumes == 0) ||
              (!observed && completion.resumes == 1 && completion.failure == failure));
    }

    auto channel = create_channel<int>(0);
    flow::internal::SendingCollector<int> collector(channel.get());
    for (int value : {19, 23}) {
        Completion completion;
        CHECK(intrinsics::is_coroutine_suspended(collector.emit(value, &completion)));
        CHECK(completion.resumes == 0);
        auto received = channel->try_receive();
        CHECK(received.is_success() && received.get_or_throw() == value);
        CHECK(completion.resumes == 1 && !completion.failure && !completion.value);
    }
    Completion cancelled;
    CHECK(intrinsics::is_coroutine_suspended(collector.emit(29, &cancelled)));
    auto failure = std::make_exception_ptr(CancellationException("waiting send cancelled"));
    channel->cancel(failure);
    CHECK(cancelled.resumes == 1 && cancelled.failure == failure);
    CHECK(channel->try_receive().is_closed());

    // The collector borrows the channel and forwards the actual value object.
    BufferedChannel<std::shared_ptr<int>> borrowed(1);
    flow::internal::SendingCollector<std::shared_ptr<int>> forwarding(&borrowed);
    auto resource = std::make_shared<int>(31);
    auto identity = resource.get();
    std::weak_ptr<int> lifetime = resource;
    Completion done;
    CHECK(forwarding.emit(std::move(resource), &done) == nullptr);
    CHECK(!resource && !lifetime.expired() && done.resumes == 0);
    {
        auto received = borrowed.try_receive();
        CHECK(received.is_success() && received.get_or_throw().get() == identity);
    }
    CHECK(lifetime.expired());
}

// Combine.kt:17-80: real child collection, suspending receive/transform and batching.
void combine_contract() {
    class QueueDispatcher final : public CoroutineDispatcher {
    public:
        mutable std::deque<std::shared_ptr<Runnable>> queue;
        void dispatch(const CoroutineContext&, std::shared_ptr<Runnable> task) const override {
            queue.push_back(std::move(task));
        }
        void drain() {
            while (!queue.empty()) {
                auto task = std::move(queue.front());
                queue.pop_front();
                task->run();
            }
        }
    };
    class ContextCompletion final : public Continuation<void*> {
    public:
        std::shared_ptr<CoroutineContext> context;
        int resumes = 0;
        std::exception_ptr failure;
        std::shared_ptr<CoroutineContext> get_context() const override { return context; }
        void resume_with(Result<void*> result) override {
            ++resumes;
            failure = result.exception_or_null();
            if (!failure) CHECK(result.get_or_throw() == nullptr);
        }
    };
    for (bool copy_array : {false, true}) for (bool fails : {false, true}) {
        auto dispatcher = std::make_shared<QueueDispatcher>();
        ContextCompletion completion;
        completion.context = dispatcher;
        auto left = create_channel<std::any>(8);
        auto right = create_channel<std::any>(8);
        std::vector<std::shared_ptr<flow::Flow<std::any>>> sources{
            flow::receive_as_flow<std::any>(left), flow::receive_as_flow<std::any>(right)};
        std::vector<std::vector<int>> batches;
        std::shared_ptr<Continuation<void*>> paused;
        const std::vector<std::any>* suspended_values = nullptr;
        auto resource = std::make_shared<int>(43);
        std::weak_ptr<int> lifetime = resource;
        int factories = 0;
        auto outcome = flow::internal::combine_internal<int>(nullptr, sources,
            [&]() -> std::vector<std::any>* {
                ++factories;
                return copy_array ? new std::vector<std::any>(2) : nullptr;
            },
            [&, resource](flow::FlowCollector<int>*, const std::vector<std::any>& values,
                          Continuation<void*>* frame) -> void* {
                CHECK(*resource == 43);
                batches.push_back({std::any_cast<int>(values[0]), std::any_cast<int>(values[1])});
                if (batches.size() == 1) {
                    paused = kotlinx::coroutines::internal::retain_continuation(frame);
                    suspended_values = &values;
                    return intrinsics::get_COROUTINE_SUSPENDED();
                }
                return nullptr;
            }, &completion);
        resource.reset();
        CHECK(intrinsics::is_coroutine_suspended(outcome));
        dispatcher->drain();
        CHECK(batches.empty() && completion.resumes == 0);
        CHECK(left->try_send(1).is_success());
        dispatcher->drain();
        CHECK(batches.empty());
        CHECK(right->try_send(10).is_success());
        dispatcher->drain();
        CHECK(batches == std::vector<std::vector<int>>({{1, 10}}) && paused && factories == 1);
        CHECK(left->try_send(2).is_success() && right->try_send(20).is_success());
        dispatcher->drain();
        CHECK(std::any_cast<int>((*suspended_values)[0]) == 1);
        CHECK(std::any_cast<int>((*suspended_values)[1]) == 10);
        CHECK(!lifetime.expired() && completion.resumes == 0);
        auto failure = std::make_exception_ptr(std::runtime_error("resumed combine transform failure"));
        paused->resume_with(fails ? Result<void*>::failure(failure) : Result<void*>::success(nullptr));
        dispatcher->drain();
        if (!fails) {
            CHECK(batches == std::vector<std::vector<int>>({{1, 10}, {2, 20}}) && factories == 2);
            // Exercise Byte epoch wrap while both source coroutines remain active.
            for (int epoch = 0; epoch < 260; ++epoch) {
                CHECK(left->try_send(100 + epoch).is_success());
                CHECK(right->try_send(1000 + epoch).is_success());
                dispatcher->drain();
                CHECK(batches.back() == std::vector<int>({100 + epoch, 1000 + epoch}));
                CHECK(completion.resumes == 0);
            }
            left->close();
            right->close();
            dispatcher->drain();
        }
        CHECK(completion.resumes == 1 && completion.failure == (fails ? failure : nullptr));
        CHECK(lifetime.expired()); // Even while the completed frame is independently retained.
    }
}

// Combine.kt:82-139: second completion cancels first collection, but not downstream.
void zip_contract() {
    class QueueDispatcher final : public CoroutineDispatcher {
    public:
        mutable std::deque<std::shared_ptr<Runnable>> queue;
        void dispatch(const CoroutineContext&, std::shared_ptr<Runnable> task) const override {
            queue.push_back(std::move(task));
        }
        void drain() {
            while (!queue.empty()) {
                auto task = std::move(queue.front());
                queue.pop_front();
                task->run();
            }
        }
    };
    class ContextCompletion final : public Continuation<void*> {
    public:
        std::shared_ptr<CoroutineContext> context;
        int resumes = 0;
        std::exception_ptr failure;
        std::shared_ptr<CoroutineContext> get_context() const override { return context; }
        void resume_with(Result<void*> result) override {
            ++resumes;
            failure = result.exception_or_null();
        }
    };
    class PausedCollector final : public flow::FlowCollector<std::shared_ptr<int>> {
    public:
        std::shared_ptr<Continuation<void*>> paused;
        std::shared_ptr<Job> emission_job;
        std::shared_ptr<int> value;
        void* emit(std::shared_ptr<int> received, Continuation<void*>* frame) override {
            value = std::move(received);
            emission_job = std::dynamic_pointer_cast<Job>(frame->get_context()->get(Job::type_key));
            paused = kotlinx::coroutines::internal::retain_continuation(frame);
            return intrinsics::get_COROUTINE_SUSPENDED();
        }
    };
    for (int failure_point : {0, 1, 2}) {
        auto dispatcher = std::make_shared<QueueDispatcher>();
        ContextCompletion completion;
        completion.context = dispatcher;
        auto left = create_channel<int>(8);
        auto right = create_channel<int>(8);
        std::shared_ptr<Continuation<void*>> transform_frame;
        std::shared_ptr<Job> transform_job;
        auto resource = std::make_shared<int>(47);
        auto identity = resource.get();
        std::weak_ptr<int> lifetime = resource;
        auto zipped = flow::internal::zip_impl<int, int, std::shared_ptr<int>>(
            flow::receive_as_flow<int>(left), flow::receive_as_flow<int>(right),
            std::function<void*(int, int, Continuation<void*>*)>(
                [&, resource](int first, int second, Continuation<void*>* frame) -> void* {
                    CHECK(first == 1 && second == 10 && resource.get() == identity);
                    transform_job = std::dynamic_pointer_cast<Job>(frame->get_context()->get(Job::type_key));
                    transform_frame = kotlinx::coroutines::internal::retain_continuation(frame);
                    return intrinsics::get_COROUTINE_SUSPENDED();
                }));
        PausedCollector collector;
        CHECK(intrinsics::is_coroutine_suspended(zipped->collect(&collector, &completion)));
        resource.reset();
        zipped.reset();
        dispatcher->drain();
        CHECK(left->try_send(1).is_success() && right->try_send(10).is_success());
        right->close();
        dispatcher->drain();
        CHECK(transform_frame && transform_job->is_active() && completion.resumes == 0);
        CHECK(!lifetime.expired());
        auto failure = std::make_exception_ptr(std::runtime_error("zip transform failure"));
        if (failure_point == 1) transform_frame->resume_with(Result<void*>::failure(failure));
        else transform_frame->resume_with(Result<void*>::success(new std::shared_ptr<int>(lifetime.lock())));
        dispatcher->drain();
        if (failure_point != 1) {
            CHECK(collector.paused && collector.value.get() == identity);
            CHECK(collector.emission_job == transform_job && transform_job->is_active());
            CHECK(completion.resumes == 0);
            collector.paused->resume_with(failure_point == 2
                ? Result<void*>::failure(failure) : Result<void*>::success(nullptr));
            dispatcher->drain();
            collector.value.reset();
        }
        CHECK(completion.resumes == 1 && completion.failure == (failure_point ? failure : nullptr));
        CHECK(lifetime.expired());
    }
}

void cancellation_contract() {
    for (int kind : {0, 1, 2}) {
        RecordingChannel channel;
        std::exception_ptr cause;
        if (kind == 1) cause = std::make_exception_ptr(std::runtime_error("consumer failure"));
        if (kind == 2) cause = std::make_exception_ptr(CancellationException("already cancelled"));
        cancel_consumed(&channel, cause);
        CHECK(channel.cancellations == 1);
        if (kind == 1) check_wrapped(channel.cancellation_cause, cause);
        else CHECK(channel.cancellation_cause == cause);
    }

    RecordingChannel channel;
    int value = 71;
    int& reference = consume<int, int&>(&channel, [&value](ReceiveChannel<int>*) -> int& { return value; });
    CHECK(&reference == &value && channel.cancellations == 1);
    RecordingChannel move_channel;
    auto owned = consume<int, std::unique_ptr<int>>(&move_channel,
        [](ReceiveChannel<int>*) { return std::make_unique<int>(93); });
    CHECK(*owned == 93 && move_channel.cancellations == 1);

    for (bool returns_value : {false, true}) for (bool block_fails : {false, true}) {
        RecordingChannel failing;
        auto block_failure = std::make_exception_ptr(std::runtime_error("block failure"));
        auto finally_failure = std::make_exception_ptr(std::runtime_error("finally failure"));
        failing.cancellation_failure = finally_failure;
        std::exception_ptr observed;
        try {
            if (returns_value) {
                (void)consume<int, int>(&failing, [&](ReceiveChannel<int>*) {
                    if (block_fails) std::rethrow_exception(block_failure);
                    return 7;
                });
            } else {
                consume<int>(&failing, std::function<void(ReceiveChannel<int>*)>([&](ReceiveChannel<int>*) {
                    if (block_fails) std::rethrow_exception(block_failure);
                }));
            }
        } catch (...) { observed = std::current_exception(); }
        CHECK(observed == finally_failure && failing.cancellations == 1);
        if (block_fails) check_wrapped(failing.cancellation_cause, block_failure);
        else CHECK(!failing.cancellation_cause);
    }
}

void iteration_contract() {
    auto channel = std::make_shared<RecordingChannel>();
    auto completion = std::make_shared<Completion>();
    std::vector<int> values;
    auto capture = std::make_shared<int>(41);
    auto identity = capture.get();
    std::weak_ptr<int> lifetime = capture;
    auto result = consume_each<int>(channel.get(), [capture, identity, &values](int value) {
        CHECK(capture.get() == identity && *capture == 41);
        values.push_back(value);
    }, completion.get());
    capture.reset();
    CHECK(intrinsics::is_coroutine_suspended(result));
    CHECK(!completion->resumes && !channel->cancellations && !lifetime.expired());
    CHECK(channel->try_send(1).is_success());
    CHECK(values == std::vector<int>{1} && !completion->resumes);
    CHECK(channel->try_send(2).is_success());
    CHECK(values == std::vector<int>({1, 2}) && !completion->resumes);
    channel->close(nullptr);
    CHECK(completion->resumes == 1 && !completion->failure && !completion->value);
    CHECK(channel->cancellations == 1 && !channel->cancellation_cause && lifetime.expired());

    for (bool action_fails : {false, true}) {
        auto failed = std::make_shared<RecordingChannel>();
        auto done = std::make_shared<Completion>();
        auto failure = std::make_exception_ptr(std::runtime_error("iteration failure"));
        CHECK(intrinsics::is_coroutine_suspended(consume_each<int>(failed.get(),
            [failure, action_fails](int) { if (action_fails) std::rethrow_exception(failure); }, done.get())));
        if (action_fails) CHECK(failed->try_send(5).is_success());
        else failed->close(failure);
        CHECK(done->resumes == 1 && done->failure == failure);
        CHECK(failed->cancellations == 1);
        check_wrapped(failed->cancellation_cause, failure);
    }
}

void list_contract() {
    for (bool suspended : {false, true}) {
        auto channel = std::make_shared<RecordingChannel>();
        auto completion = std::make_shared<Completion>();
        channel->try_send(3);
        if (!suspended) { channel->try_send(4); channel->close(nullptr); }
        auto result = to_list<int>(channel.get(), completion.get());
        if (suspended) {
            CHECK(intrinsics::is_coroutine_suspended(result));
            CHECK(!completion->resumes && !channel->cancellations);
            CHECK(channel->try_send(4).is_success());
            CHECK(!completion->resumes);
            channel->close(nullptr);
            CHECK(completion->resumes == 1 && !completion->failure);
            result = completion->value;
        } else CHECK(!completion->resumes);
        std::unique_ptr<std::vector<int>> values(static_cast<std::vector<int>*>(result));
        CHECK(*values == std::vector<int>({3, 4}));
        CHECK(channel->cancellations == 1);
    }
    auto channel = std::make_shared<RecordingChannel>();
    auto completion = std::make_shared<Completion>();
    CHECK(intrinsics::is_coroutine_suspended(to_list<int>(channel.get(), completion.get())));
    auto failure = std::make_exception_ptr(std::runtime_error("closed list failure"));
    channel->close(failure);
    CHECK(completion->resumes == 1 && completion->failure == failure && !completion->value);
    check_wrapped(channel->cancellation_cause, failure);

    auto source = flow::consume_as_flow<int>(channel);
    auto scope = create_coroutine_scope(EmptyCoroutineContext::instance());
    auto first = flow::produce_in<int>(source, scope.get());
    CHECK(first.get() == channel.get());
    bool caught = false;
    try { (void)flow::produce_in<int>(source, scope.get()); }
    catch (const IllegalStateException& exception) {
        caught = std::string(exception.what()) == "ReceiveChannel.consumeAsFlow can be collected just once";
    }
    CHECK(caught);
}

void suspended_action_contract() {
    for (bool fails : {false, true}) {
        auto channel = std::make_shared<RecordingChannel>();
        auto completion = std::make_shared<Completion>();
        channel->try_send(6);
        channel->try_send(7);
        channel->close(nullptr);
        Continuation<void*>* paused = nullptr;
        std::vector<int> values;
        auto capture = std::make_shared<int>(19);
        std::weak_ptr<int> lifetime = capture;
        auto result = consume_each<int>(channel.get(),
            [capture, &paused, &values](int value, Continuation<void*>* continuation) -> void* {
                CHECK(*capture == 19);
                values.push_back(value);
                paused = continuation;
                return intrinsics::get_COROUTINE_SUSPENDED();
            }, completion.get());
        capture.reset();
        CHECK(intrinsics::is_coroutine_suspended(result));
        CHECK(values == std::vector<int>{6} && !lifetime.expired());
        CHECK(!completion->resumes && !channel->cancellations);
        auto failure = std::make_exception_ptr(std::runtime_error("resumed action failure"));
        paused->resume_with(fails ? Result<void*>::failure(failure) : Result<void*>::success(nullptr));
        if (!fails) {
            CHECK(values == std::vector<int>({6, 7}) && !completion->resumes);
            CHECK(!channel->cancellations && !lifetime.expired());
            paused->resume_with(Result<void*>::success(nullptr));
        }
        CHECK(completion->resumes == 1 && completion->failure == (fails ? failure : nullptr));
        CHECK(channel->cancellations == 1 && lifetime.expired());
        if (fails) check_wrapped(channel->cancellation_cause, failure);
    }
}

void list_resource_contract() {
    for (bool fails : {false, true}) {
        auto channel = create_channel<std::shared_ptr<int>>(8);
        auto completion = std::make_shared<Completion>();
        auto resource = std::make_shared<int>(29);
        auto identity = resource.get();
        std::weak_ptr<int> lifetime = resource;
        CHECK(channel->try_send(resource).is_success());
        resource.reset();
        CHECK(intrinsics::is_coroutine_suspended(to_list<std::shared_ptr<int>>(channel.get(), completion.get())));
        CHECK(!lifetime.expired() && !completion->resumes);
        auto failure = std::make_exception_ptr(std::runtime_error("resource list failure"));
        channel->close(fails ? failure : nullptr);
        CHECK(completion->resumes == 1 && completion->failure == (fails ? failure : nullptr));
        if (fails) CHECK(lifetime.expired() && !completion->value);
        else {
            std::unique_ptr<std::vector<std::shared_ptr<int>>> values(
                static_cast<std::vector<std::shared_ptr<int>>*>(completion->value));
            CHECK(values->size() == 1 && values->front().get() == identity && *values->front() == 29);
            CHECK(!lifetime.expired());
            values.reset();
            CHECK(lifetime.expired());
        }
    }
}
}

int main() {
    try {
        job_cancellation_equality_contract();
        channel_scope_contract();
        sending_collector_contract();
        combine_contract();
        zip_contract();
        cancellation_contract();
        iteration_contract();
        list_contract();
        suspended_action_contract();
        list_resource_contract();
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
        return 1;
    }
}
