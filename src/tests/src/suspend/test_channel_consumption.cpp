// Source contracts: kotlinx-coroutines-core/common/src/channels/Channels.common.kt:90-103,159-162,191-202;
// kotlinx-coroutines-core/common/src/flow/Channels.kt:104-108,119-134.
#include "kotlinx/coroutines/channels/Channels.hpp"
#include "kotlinx/coroutines/flow/Channels.hpp"
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
