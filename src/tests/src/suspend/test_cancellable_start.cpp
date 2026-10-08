// Source contracts: kotlinx-coroutines-core/common/src/intrinsics/Cancellable.kt:33-64;
// kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/intrinsics/IntrinsicsNative.kt:201-202.
#include "kotlinx/coroutines/intrinsics/Cancellable.hpp"
#include "kotlinx/coroutines/CoroutineStart.hpp"
#include "kotlinx/coroutines/AbstractCoroutine.hpp"
#include <vector>
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
// Contracts from Native IntrinsicsNative.kt:142-189,221-263,296-324.
void cold_creation_contract() {
    auto completion = std::make_shared<Completion>();
    int entered = 0;
    intrinsics::ErasedSuspendFunction body = [&](std::shared_ptr<Continuation<void*>> frame) -> void* {
        ++entered;
        CHECK(frame.get() != completion.get());
        CHECK(dynamic_cast<RestrictedContinuationImpl*>(frame.get()));
        return reinterpret_cast<void*>(42);
    };
    auto first = intrinsics::create_coroutine_unintercepted(body, completion);
    auto second = intrinsics::create_coroutine_unintercepted(body, completion);
    CHECK(first != second && !entered);
    CHECK(dynamic_cast<RestrictedContinuationImpl*>(first.get()));
    first->resume_with(Result<void*>::success(nullptr));
    CHECK(entered == 1 && completion->resumes == 1 && completion->value == reinterpret_cast<void*>(42));
    auto original = std::make_exception_ptr(std::runtime_error("initial failure"));
    second->resume_with(Result<void*>::failure(original));
    CHECK(entered == 1 && completion->resumes == 2 && completion->failure == original);
    // A resumed callable-wrapper result follows the actual 1 -> 2 branch.
    first->resume_with(Result<void*>::success(reinterpret_cast<void*>(17)));
    CHECK(entered == 1 && completion->value == reinterpret_cast<void*>(17));
    first->resume_with(Result<void*>::success(nullptr));
    CHECK(bool(completion->failure));

    completion->context = std::make_shared<Dispatcher>();
    auto contextual = intrinsics::create_coroutine_unintercepted(body, completion);
    CHECK(dynamic_cast<ContinuationImpl*>(contextual.get()));
    std::weak_ptr<Continuation<void*>> lifetime = contextual;
    contextual.reset();
    CHECK(lifetime.expired()); // Creating a cold wrapper does not root it.
}

void typed_immediate_and_prototype_contract() {
    int observed = 0;
    int resumes = 0;
    auto completion = make_continuation<int>(EmptyCoroutineContext::instance(), [&](Result<int> result) {
        observed = result.get_or_throw(); ++resumes;
    });
    std::function<void*(int, Continuation<int>*)> receiver_body = [](int receiver, auto* frame) -> void* {
        CHECK(dynamic_cast<internal::ResultBoxCompletion<int>*>(frame));
        return new int(receiver + 1); // The receiving typed adapter unboxes and deletes.
    };
    intrinsics::start_coroutine_cancellable<int, int>(receiver_body, 54, completion);
    CHECK(observed == 55 && resumes == 1);
    intrinsics::start_coroutine<int, int>(receiver_body, 64, completion);
    CHECK(observed == 65 && resumes == 2);
    intrinsics::start_coroutine_undispatched<int, int>(receiver_body, 74, completion);
    CHECK(observed == 75 && resumes == 3);

    int unit_resumes = 0;
    auto unit_completion = make_continuation<Unit>(EmptyCoroutineContext::instance(), [&](Result<Unit> result) {
        result.get_or_throw(); ++unit_resumes;
    });
    std::function<void*(Continuation<Unit>*)> unit_body = [](auto*) -> void* { return nullptr; };
    intrinsics::start_coroutine_cancellable<Unit>(unit_body, unit_completion);
    CHECK(unit_resumes == 1);

    class Prototype final : public ContinuationImpl {
    public:
        bool entered = false;
        int observed = 0;
        Prototype() : ContinuationImpl(std::make_shared<Completion>()) {}
        void* invoke_suspend(Result<void*>) override { throw std::logic_error("prototype is not the fresh frame"); }
        std::shared_ptr<Continuation<void*>> create(std::shared_ptr<Continuation<void*>> completion) override {
            return std::make_shared<Frame>(completion, std::make_shared<int>(8), &entered, &observed);
        }
        std::shared_ptr<Continuation<void*>> create(void* receiver, std::shared_ptr<Continuation<void*>> completion) override {
            return std::make_shared<Frame>(completion, std::make_shared<int>(*static_cast<int*>(receiver)), &entered, &observed);
        }
    } prototype;
    auto erased_completion = std::make_shared<Completion>();
    auto first = intrinsics::create_coroutine_unintercepted(prototype, erased_completion);
    int receiver = 19;
    auto second = intrinsics::create_coroutine_unintercepted(prototype, &receiver, erased_completion);
    CHECK(first != second && !prototype.entered);
    first->resume_with(Result<void*>::success(nullptr));
    CHECK(prototype.entered && prototype.observed == 8);
    second->resume_with(Result<void*>::success(nullptr));
    CHECK(prototype.observed == 19 && erased_completion->resumes == 2);
}

void cold_start_modes_contract() {
    for (int mode : {0, 1, 2}) {
        auto dispatcher = std::make_shared<Dispatcher>();
        auto job = JobImpl::create(nullptr);
        auto completion = std::make_shared<Completion>();
        completion->context = dispatcher->operator+(job);
        auto resource = std::make_shared<int>(91);
        std::weak_ptr<int> lifetime = resource;
        bool entered = false;
        intrinsics::ErasedSuspendFunction body = [resource, &entered](std::shared_ptr<Continuation<void*>> frame) -> void* {
            CHECK(dynamic_cast<ContinuationImpl*>(frame.get()));
            CHECK(*resource == 91);
            entered = true;
            return nullptr;
        };
        if (mode == 0) intrinsics::start_coroutine_cancellable(body, completion);
        if (mode == 1) intrinsics::start_coroutine(body, completion);
        if (mode == 2) intrinsics::start_coroutine_undispatched(body, completion);
        body = nullptr;
        resource.reset();
        if (mode != 2) {
            CHECK(!entered && !lifetime.expired());
            job->cancel(nullptr);
            dispatcher->drain();
        }
        CHECK(entered == (mode != 0) && completion->resumes == 1);
        CHECK(bool(completion->failure) == (mode == 0));
        CHECK(lifetime.expired());
    }

    for (int mode : {0, 1, 2}) {
        auto dispatcher = std::make_shared<Dispatcher>();
        auto original = std::make_exception_ptr(std::runtime_error("start dispatch failure"));
        dispatcher->failure = original;
        auto completion = std::make_shared<Completion>();
        completion->context = dispatcher;
        auto resource = std::make_shared<int>(1);
        std::weak_ptr<int> lifetime = resource;
        intrinsics::ErasedSuspendFunction body = [resource](auto) -> void* { return reinterpret_cast<void*>(31); };
        std::exception_ptr thrown;
        try {
            if (mode == 0) intrinsics::start_coroutine_cancellable(body, completion);
            if (mode == 1) intrinsics::start_coroutine(body, completion);
            if (mode == 2) intrinsics::start_coroutine_undispatched(body, completion);
        } catch (...) { thrown = std::current_exception(); }
        body = nullptr;
        resource.reset();
        if (mode == 0) CHECK(thrown == original && completion->failure == original && completion->resumes == 1);
        if (mode == 1) {
            CHECK(thrown && !completion->resumes);
            try { std::rethrow_exception(thrown); }
            catch (const internal::DispatchException& failure) { CHECK(failure.cause == original); }
        }
        if (mode == 2) CHECK(!thrown && completion->value == reinterpret_cast<void*>(31));
        CHECK(lifetime.expired());
    }
}

void typed_suspension_contract() {
    for (int mode : {0, 1, 2}) for (bool failure : {false, true}) {
        auto dispatcher = std::make_shared<Dispatcher>();
        dispatcher->immediate = true;
        int resumes = 0;
        int observed = 0;
        std::exception_ptr observed_failure;
        auto completion = make_continuation<int>(dispatcher, [&](Result<int> result) {
            ++resumes;
            observed_failure = result.exception_or_null();
            if (!observed_failure) observed = result.get_or_throw();
        });
        std::weak_ptr<Continuation<int>> completion_lifetime = completion;
        Continuation<int>* pending = nullptr;
        std::weak_ptr<Continuation<void*>> frame_lifetime;
        auto resource = std::make_shared<int>(71);
        std::weak_ptr<int> lifetime = resource;
        std::function<void*(Continuation<int>*)> body = [resource, &pending, &frame_lifetime](Continuation<int>* frame) -> void* {
            CHECK(*resource == 71);
            auto projection = dynamic_cast<internal::ResultBoxCompletion<int>*>(frame);
            CHECK(projection);
            frame_lifetime = projection->completion();
            pending = frame;
            return COROUTINE_SUSPENDED;
        };
        if (mode == 0) intrinsics::start_coroutine_cancellable<int>(body, completion);
        if (mode == 1) intrinsics::start_coroutine<int>(body, completion);
        if (mode == 2) intrinsics::start_coroutine_undispatched<int>(body, completion);
        body = nullptr;
        completion.reset();
        resource.reset();
        CHECK(pending && !resumes && !completion_lifetime.expired() && !frame_lifetime.expired() && !lifetime.expired());
        auto original = std::make_exception_ptr(std::runtime_error("resumed body failure"));
        if (failure) pending->resume_with(Result<int>::failure(original));
        else pending->resume_with(Result<int>::success(71));
        CHECK(resumes == 1 && (failure ? observed_failure == original : observed == 71));
        CHECK(completion_lifetime.expired() && frame_lifetime.expired() && lifetime.expired());
    }

    // The same callable can create independent suspended computations.
    std::vector<Continuation<int>*> pending_calls;
    std::function<void*(Continuation<int>*)> shared_body = [&](Continuation<int>* frame) -> void* {
        pending_calls.push_back(frame); return COROUTINE_SUSPENDED;
    };
    auto erased_body = intrinsics::erase_suspend_function<int>(shared_body);
    int first_value = 0, second_value = 0;
    auto first_completion = make_continuation<int>(EmptyCoroutineContext::instance(), [&](Result<int> r) { first_value = r.get_or_throw(); });
    auto second_completion = make_continuation<int>(EmptyCoroutineContext::instance(), [&](Result<int> r) { second_value = r.get_or_throw(); });
    intrinsics::start_coroutine_cancellable(erased_body, to_void_continuation(first_completion));
    intrinsics::start_coroutine_cancellable(erased_body, to_void_continuation(second_completion));
    CHECK(pending_calls.size() == 2 && pending_calls[0] != pending_calls[1]);
    pending_calls[1]->resume_with(Result<int>::success(2));
    pending_calls[0]->resume_with(Result<int>::success(1));
    CHECK(first_value == 1 && second_value == 2);

    Completion borrowed;
    std::function<void*(Continuation<void*>*)> raw_body = [&](Continuation<void*>* frame) -> void* {
        CHECK(frame != &borrowed);
        return reinterpret_cast<void*>(83);
    };
    intrinsics::start_coroutine_cancellable<void*>(raw_body, &borrowed);
    CHECK(borrowed.resumes == 1 && borrowed.value == reinterpret_cast<void*>(83));

    int void_resumes = 0;
    auto void_completion = make_continuation<void>(EmptyCoroutineContext::instance(), [&](Result<void> result) {
        result.get_or_throw(); ++void_resumes;
    });
    std::function<void*(Continuation<void>*)> void_body = [](auto* frame) -> void* {
        frame->resume_with(Result<void>::success());
        return COROUTINE_SUSPENDED; // Inline resume must not reroot a completed frame.
    };
    std::weak_ptr<Continuation<void>> void_lifetime = void_completion;
    intrinsics::start_coroutine_undispatched<void>(void_body, void_completion);
    void_completion.reset(); void_body = nullptr;
    CHECK(void_resumes == 1 && void_lifetime.expired());

    Completion throwing;
    auto original = std::make_exception_ptr(std::runtime_error("completion throws"));
    throwing.resume_failure = original;
    std::exception_ptr thrown;
    try { intrinsics::start_coroutine_undispatched<void*>(raw_body, &throwing); }
    catch (...) { thrown = std::current_exception(); }
    CHECK(thrown == original && throwing.resumes == 1); // Completion invocation lies outside the body catch.
}

// CoroutineStart.kt:356-362 delegates to actual start intrinsics, including
// interception by ordinary ContinuationInterceptor implementations.
void coroutine_start_interceptor_contract() {
    class Interceptor final : public ContinuationInterceptor {
    public:
        int interceptions = 0, resumptions = 0, releases = 0;
        std::weak_ptr<Continuation<void*>> actual_frame;
        class Wrapped final : public Continuation<void*> {
        public:
            Interceptor* interceptor;
            std::shared_ptr<Continuation<void*>> actual;
            Wrapped(Interceptor* interceptor, std::shared_ptr<Continuation<void*>> actual)
                : interceptor(interceptor), actual(std::move(actual)) {}
            std::shared_ptr<CoroutineContext> get_context() const override { return actual->get_context(); }
            void resume_with(Result<void*> result) override { ++interceptor->resumptions; actual->resume_with(std::move(result)); }
        };
        std::shared_ptr<Continuation<void*>> intercept_continuation(std::shared_ptr<Continuation<void*>> frame) override {
            ++interceptions;
            actual_frame = frame;
            return std::make_shared<Wrapped>(this, std::move(frame));
        }
        void release_intercepted_continuation(std::shared_ptr<Continuation<void*>> frame) override {
            CHECK(dynamic_cast<Wrapped*>(frame.get())); ++releases;
        }
    };
    for (auto mode : {CoroutineStart::DEFAULT, CoroutineStart::ATOMIC, CoroutineStart::UNDISPATCHED, CoroutineStart::LAZY}) {
        auto interceptor = std::make_shared<Interceptor>();
        int observed = 0, resumes = 0;
        auto completion = make_continuation<int>(interceptor, [&](Result<int> result) { observed = result.get_or_throw(); ++resumes; });
        bool entered = false;
        invoke(mode, [&](int receiver, std::shared_ptr<Continuation<void*>> frame) -> int {
            CHECK(frame.get() != dynamic_cast<Continuation<void*>*>(completion.get()));
            CHECK(dynamic_cast<ContinuationImpl*>(frame.get()));
            entered = true; return receiver + 4;
        }, 23, completion);
        bool lazy = mode == CoroutineStart::LAZY;
        CHECK(CoroutineStartExtensions::is_lazy(mode) == lazy);
        CHECK(entered == !lazy && resumes == (lazy ? 0 : 1));
        if (!lazy) CHECK(observed == 27);
        int interceptions = mode == CoroutineStart::DEFAULT || mode == CoroutineStart::ATOMIC ? 1 : 0;
        CHECK(interceptor->interceptions == interceptions);
        CHECK(interceptor->resumptions == interceptions && interceptor->releases == interceptions);
        CHECK(interceptor->actual_frame.expired());
    }

    auto dispatcher = std::make_shared<Dispatcher>();
    auto completion = std::make_shared<Completion>();
    completion->context = dispatcher;
    auto original = std::make_exception_ptr(std::runtime_error("strategy dispatch failure"));
    dispatcher->failure = original;
    std::exception_ptr thrown;
    try { invoke(CoroutineStart::DEFAULT, [](Unit) {}, Unit{}, std::static_pointer_cast<Continuation<void*>>(completion)); }
    catch (...) { thrown = std::current_exception(); }
    CHECK(thrown == original && completion->failure == original && completion->resumes == 1);

    auto throwing = std::make_shared<Completion>();
    throwing->resume_failure = original;
    thrown = nullptr;
    try { invoke(CoroutineStart::UNDISPATCHED, [](Unit) -> void* { return reinterpret_cast<void*>(67); }, Unit{},
                 std::static_pointer_cast<Continuation<void*>>(throwing)); }
    catch (...) { thrown = std::current_exception(); }
    CHECK(thrown == original && throwing->resumes == 1 && throwing->value == reinterpret_cast<void*>(67));
}

void coroutine_start_ownership_contract() {
    struct Body {
        std::unique_ptr<std::shared_ptr<int>> resource;
        bool* entered;
        int operator()(int receiver) { *entered = true; return **resource + receiver; }
    };
    for (auto mode : {CoroutineStart::DEFAULT, CoroutineStart::ATOMIC, CoroutineStart::UNDISPATCHED, CoroutineStart::LAZY}) {
        auto dispatcher = std::make_shared<Dispatcher>();
        auto job = JobImpl::create(nullptr);
        int observed = 0, resumes = 0;
        std::exception_ptr failure;
        auto completion = make_continuation<int>(dispatcher->operator+(job), [&](Result<int> result) {
            ++resumes; failure = result.exception_or_null(); if (!failure) observed = result.get_or_throw();
        });
        auto resource = std::make_shared<int>(100);
        std::weak_ptr<int> lifetime = resource;
        bool entered = false;
        Body body{std::make_unique<std::shared_ptr<int>>(resource), &entered};
        resource.reset();
        if (mode == CoroutineStart::UNDISPATCHED) job->cancel(nullptr);
        invoke(mode, std::move(body), 9, completion);
        if (mode == CoroutineStart::LAZY) {
            CHECK(body.resource && !entered && !resumes && dispatcher->queue.empty());
            body.resource.reset();
        } else {
            CHECK(!body.resource);
            if (mode != CoroutineStart::UNDISPATCHED) {
                CHECK(!entered && !resumes && !lifetime.expired());
                job->cancel(nullptr); dispatcher->drain();
            }
            CHECK(resumes == 1 && entered == (mode != CoroutineStart::DEFAULT));
            if (mode == CoroutineStart::DEFAULT) CHECK(failure && is_cancellation_exception(failure));
            else CHECK(!failure && observed == 109);
        }
        CHECK(lifetime.expired());
    }

    // Ordinary C++ noncopyable lvalue receivers stay borrowed, retaining identity.
    auto receiver = std::make_unique<int>(19);
    int observed = 0;
    auto completion = make_continuation<int>(EmptyCoroutineContext::instance(), [&](Result<int> r) { observed = r.get_or_throw(); });
    invoke(CoroutineStart::UNDISPATCHED, [](std::unique_ptr<int>& value) { return *value; }, receiver, completion);
    CHECK(receiver && observed == 19);

    // Public typed compatibility overloads resume through the same native wrappers.
    Continuation<int>* pending = nullptr;
    std::function<void*(int, Continuation<int>*)> body = [&](int value, Continuation<int>* frame) -> void* {
        CHECK(value == 13); pending = frame; return COROUTINE_SUSPENDED;
    };
    CoroutineStartExtensions::invoke(CoroutineStart::DEFAULT, body, 13, completion.get());
    CHECK(pending);
    pending->resume_with(Result<int>::success(29));
    CHECK(observed == 29);
}

// Source contract: CoroutineStart.kt:356-362 supplies the actual block and receiver.
// Ordinary C++ lvalue references remain borrowed through dispatch and suspension.
void coroutine_start_copyable_borrow_contract() {
    struct Receiver {
        int value = 13;
        int* copies;
        explicit Receiver(int* copies) : copies(copies) {}
        Receiver(const Receiver& other) : value(other.value), copies(other.copies) { ++*copies; }
    };
    struct Body {
        Receiver* expected_receiver;
        Body* expected_body = nullptr;
        int* copies;
        int calls = 0;
        Continuation<void*>* pending = nullptr;
        std::weak_ptr<Continuation<void*>> frame_lifetime;
        explicit Body(Receiver* receiver, int* copies) : expected_receiver(receiver), copies(copies) {}
        Body(const Body& other)
            : expected_receiver(other.expected_receiver), expected_body(other.expected_body), copies(other.copies) {
            ++*copies;
        }
        void* operator()(Receiver& receiver, std::shared_ptr<Continuation<void*>> frame) {
            CHECK(this == expected_body && &receiver == expected_receiver);
            ++calls;
            ++receiver.value;
            pending = frame.get();
            frame_lifetime = frame;
            return COROUTINE_SUSPENDED;
        }
    };
    for (auto mode : {CoroutineStart::DEFAULT, CoroutineStart::ATOMIC,
                      CoroutineStart::UNDISPATCHED, CoroutineStart::LAZY}) {
        for (bool cancelled : {false, true}) {
            int copies = 0;
            Receiver receiver(&copies);
            Body body(&receiver, &copies);
            body.expected_body = &body;
            auto dispatcher = std::make_shared<Dispatcher>();
            auto job = JobImpl::create(nullptr);
            int resumes = 0, observed = 0;
            std::exception_ptr failure;
            auto completion = make_continuation<int>(dispatcher->operator+(job), [&](Result<int> result) {
                ++resumes;
                failure = result.exception_or_null();
                if (!failure) observed = result.get_or_throw();
            });
            if (cancelled) job->cancel(nullptr);
            invoke(mode, body, receiver, completion);
            CHECK(copies == 0);
            if (mode == CoroutineStart::DEFAULT || mode == CoroutineStart::ATOMIC) {
                CHECK(body.calls == 0 && receiver.value == 13);
                receiver.value = 21; // Deferred entry must observe the same receiver's updated state.
                dispatcher->drain();
            }
            const bool entered = mode != CoroutineStart::LAZY && !(cancelled && mode == CoroutineStart::DEFAULT);
            CHECK(body.calls == (entered ? 1 : 0));
            if (entered) {
                CHECK(!resumes && body.pending && !body.frame_lifetime.expired());
                CHECK(receiver.value == (mode == CoroutineStart::UNDISPATCHED ? 14 : 22));
                auto original = std::make_exception_ptr(std::runtime_error("borrowed body resumed failure"));
                // The caller keeps both borrowed objects alive until the frame terminates.
                if (cancelled) body.pending->resume_with(Result<void*>::failure(original));
                else body.pending->resume_with(Result<void*>::success(new int(receiver.value)));
                dispatcher->drain();
                CHECK(resumes == 1 && (cancelled ? failure == original : observed == receiver.value));
                CHECK(body.calls == 1 && body.frame_lifetime.expired());
            } else if (mode == CoroutineStart::DEFAULT) {
                CHECK(resumes == 1 && failure && is_cancellation_exception(failure));
            } else {
                CHECK(!resumes && !body.pending && dispatcher->queue.empty());
            }
            CHECK(copies == 0 && body.expected_receiver == &receiver);
        }
    }
}

// AbstractCoroutine.kt:133-135 forwards the actual block/receiver to start.
// Its C++ by-value parameters transfer their ownership into delayed entry.
void abstract_coroutine_owned_start_contract() {
    class Coroutine final : public AbstractCoroutine<int> {
    public:
        explicit Coroutine(std::shared_ptr<CoroutineContext> context) : AbstractCoroutine<int>(context, false, true) {}
        int observed = 0;
        void on_completed(int value) override { observed = value; }
    };
    for (bool erased : {false, true}) {
        auto dispatcher = std::make_shared<Dispatcher>();
        auto coroutine = std::make_shared<Coroutine>(dispatcher);
        std::weak_ptr<int> receiver_lifetime, capture_lifetime;
        int* receiver_identity = nullptr;
        {
            auto receiver = std::make_shared<int>(11);
            auto capture = std::make_shared<int>(17);
            receiver_lifetime = receiver;
            capture_lifetime = capture;
            receiver_identity = receiver.get();
            if (erased) {
                std::function<void*(std::shared_ptr<int>, std::shared_ptr<Continuation<void*>>)> block =
                    [capture, receiver_identity](std::shared_ptr<int> value, auto) -> void* {
                        CHECK(value.get() == receiver_identity);
                        return new int(*value + *capture);
                    };
                coroutine->start(CoroutineStart::DEFAULT, std::move(receiver), std::move(block));
            } else {
                std::function<int(std::shared_ptr<int>)> block =
                    [capture, receiver_identity](std::shared_ptr<int> value) {
                        CHECK(value.get() == receiver_identity);
                        return *value + *capture;
                    };
                coroutine->start(CoroutineStart::DEFAULT, std::move(receiver), std::move(block));
            }
        }
        CHECK(!receiver_lifetime.expired() && !capture_lifetime.expired() && !coroutine->is_completed());
        dispatcher->drain();
        CHECK(coroutine->observed == 28 && coroutine->is_completed());
        CHECK(receiver_lifetime.expired() && capture_lifetime.expired());
    }
}

}

int main() {
    try {
        plain_continuation_contract();
        failure_contract();
        dispatched_contract();
        cold_creation_contract();
        typed_immediate_and_prototype_contract();
        cold_start_modes_contract();
        typed_suspension_contract();
        coroutine_start_interceptor_contract();
        coroutine_start_ownership_contract();
        coroutine_start_copyable_borrow_contract();
        abstract_coroutine_owned_start_contract();
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
        return 1;
    }
}
