// Source contracts: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt:33-64;
// kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:201-202.
#include "kotlinx/coroutines/intrinsics/Cancellable.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include <deque>
#include <iostream>

using namespace kotlinx::coroutines;

namespace {
void require(bool value, int line) {
    if (!value) throw std::runtime_error("cancellable start check at " + std::to_string(line));
}
#define CHECK(value) require((value), __LINE__)

class Completion final : public Continuation<void*> {
public:
    int resumes = 0;
    void* value = nullptr;
    std::exception_ptr failure;
    std::exception_ptr resume_failure;
    std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance();
    std::shared_ptr<CoroutineContext> get_context() const override { return context; }
    void resume_with(Result<void*> result) override {
        ++resumes;
        failure = result.exception_or_null();
        if (!failure) value = result.get_or_throw();
        if (resume_failure) std::rethrow_exception(resume_failure);
    }
};

class Dispatcher final : public CoroutineDispatcher {
public:
    bool immediate = false;
    bool fail_query = false;
    std::exception_ptr failure;
    mutable std::deque<std::shared_ptr<Runnable>> queue;
    bool is_dispatch_needed(const CoroutineContext&) const override {
        if (failure && fail_query) std::rethrow_exception(failure);
        return !immediate;
    }
    void dispatch(const CoroutineContext&, std::shared_ptr<Runnable> task) const override {
        if (failure) std::rethrow_exception(failure);
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

class Frame final : public ContinuationImpl {
public:
    std::shared_ptr<int> resource;
    bool* entered;
    int* observed;
    Frame(std::shared_ptr<Continuation<void*>> completion, std::shared_ptr<int> value,
          bool* body_entered, int* body_value)
        : ContinuationImpl(std::move(completion)), resource(std::move(value)),
          entered(body_entered), observed(body_value) {}
    void* invoke_suspend(Result<void*> result) override {
        (void)result.get_or_throw();
        *entered = true;
        *observed = *resource;
        return nullptr;
    }
};

void plain_continuation_contract() {
    Completion actual;
    Completion fatal;
    intrinsics::start_coroutine_cancellable(&actual, &fatal);
    CHECK(actual.resumes == 1 && !actual.failure && !actual.value);
    CHECK(!fatal.resumes);

    auto original = std::make_exception_ptr(std::runtime_error("plain resume failure"));
    actual.resume_failure = original;
    std::exception_ptr thrown;
    try { intrinsics::start_coroutine_cancellable(&actual, &fatal); }
    catch (...) { thrown = std::current_exception(); }
    CHECK(thrown == original && fatal.failure == original && fatal.resumes == 1);
}

void failure_contract() {
    for (bool wrapped : {false, true}) {
        Completion completion;
        auto original = std::make_exception_ptr(std::runtime_error("original failure"));
        auto exception = wrapped ? std::make_exception_ptr(internal::DispatchException(original, nullptr, nullptr)) : original;
        std::exception_ptr thrown;
        try {
            intrinsics::run_safely(&completion, [exception] { std::rethrow_exception(exception); });
        } catch (...) { thrown = std::current_exception(); }
        CHECK(completion.resumes == 1 && completion.failure == original && thrown == original);
    }
    Completion completion;
    bool ran = false;
    intrinsics::run_safely(&completion, [&] { ran = true; });
    CHECK(ran && !completion.resumes);
    auto original = std::make_exception_ptr(std::runtime_error("start failure"));
    auto resume_failure = std::make_exception_ptr(std::runtime_error("fatal completion failure"));
    completion.resume_failure = resume_failure;
    std::exception_ptr thrown;
    try { intrinsics::dispatcher_failure(&completion, original); }
    catch (...) { thrown = std::current_exception(); }
    CHECK(completion.failure == original && thrown == resume_failure);
}

void dispatched_contract() {
    for (bool immediate : {false, true}) for (bool cancelled : {false, true}) {
        auto dispatcher = std::make_shared<Dispatcher>();
        dispatcher->immediate = immediate;
        auto job = JobImpl::create(nullptr);
        auto completion = std::make_shared<Completion>();
        completion->context = dispatcher->operator+(job);
        auto resource = std::make_shared<int>(51);
        std::weak_ptr<int> resource_lifetime = resource;
        bool entered = false;
        int observed = 0;
        auto frame = std::make_shared<Frame>(completion, resource, &entered, &observed);
        std::weak_ptr<Frame> frame_lifetime = frame;
        Completion fatal;
        if (cancelled && immediate) job->cancel(nullptr);
        intrinsics::start_coroutine_cancellable(frame.get(), &fatal);
        frame.reset();
        resource.reset();
        if (!immediate) {
            CHECK(!completion->resumes && !entered && !resource_lifetime.expired());
            if (cancelled) job->cancel(nullptr);
            dispatcher->drain();
        }
        CHECK(!fatal.resumes && completion->resumes == 1);
        CHECK(entered == !cancelled);
        if (cancelled) CHECK(completion->failure && is_cancellation_exception(completion->failure));
        else CHECK(!completion->failure && observed == 51);
        CHECK(frame_lifetime.expired() && resource_lifetime.expired());
    }

    for (bool fail_query : {false, true}) {
        auto dispatcher = std::make_shared<Dispatcher>();
        auto original = std::make_exception_ptr(std::runtime_error("dispatcher failed"));
        dispatcher->failure = original;
        dispatcher->fail_query = fail_query;
        auto completion = std::make_shared<Completion>();
        completion->context = dispatcher;
        auto resource = std::make_shared<int>(61);
        std::weak_ptr<int> lifetime = resource;
        bool entered = false;
        int observed = 0;
        auto frame = std::make_shared<Frame>(completion, resource, &entered, &observed);
        std::weak_ptr<Frame> frame_lifetime = frame;
        Completion fatal;
        std::exception_ptr thrown;
        try { intrinsics::start_coroutine_cancellable(frame.get(), &fatal); }
        catch (...) { thrown = std::current_exception(); }
        CHECK(thrown == original && fatal.failure == original && fatal.resumes == 1);
        CHECK(!entered && !completion->resumes && dispatcher->queue.empty());
        frame.reset();
        resource.reset();
        CHECK(frame_lifetime.expired() && lifetime.expired());
    }
}
}

int main() {
    try {
        plain_continuation_contract();
        failure_contract();
        dispatched_contract();
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
        return 1;
    }
}
