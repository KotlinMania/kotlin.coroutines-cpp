// Source contracts: kotlinx-coroutines-core/common/src/selects/Select.kt:463-470,488-521,612-617,707-724,824-848.
#include "kotlinx/coroutines/selects/Select.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include "kotlinx/coroutines/channels/BufferedChannel.hpp"
#include "kotlinx/coroutines/internal/OnUndeliveredElement.hpp"
#include "kotlinx/coroutines/channels/ConflatedBufferedChannel.hpp"
#include "kotlinx/coroutines/channels/Channels.hpp"
#include "kotlinx/coroutines/flow/SharingStarted.hpp"
#include <deque>
#include <iostream>
#include <string>
#include <stdexcept>
#include <any>

using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::selects;
namespace {
void check(bool condition, int line) {
    if (!condition) throw std::runtime_error("select argument check at " + std::to_string(line));
}
#define CHECK(condition) check(condition, __LINE__)
class Completion final : public Continuation<void*> {
public:
    int resumes = 0;
    std::exception_ptr failure;
    std::shared_ptr<CoroutineContext> context = EmptyCoroutineContext::instance();
    std::shared_ptr<CoroutineContext> get_context() const override { return context; }
    void resume_with(Result<void*> result) override { ++resumes; failure = result.exception_or_null(); }
};
struct Parameter {
    std::shared_ptr<int> resource;
    std::string text;
};
struct OpaquePayload {
    std::shared_ptr<int> resource;
};
// Channel.kt:1425-1454: a null handler does not require any element-text operation.
void channel_without_handler_contract() {
    using namespace kotlinx::coroutines::channels;
    for (auto overflow : {BufferOverflow::SUSPEND, BufferOverflow::DROP_OLDEST, BufferOverflow::DROP_LATEST}) {
        auto channel = create_channel<OpaquePayload>(1, overflow);
        auto resource = std::make_shared<int>(102);
        auto* identity = resource.get();
        std::weak_ptr<int> lifetime = resource;
        CHECK(channel->try_send(OpaquePayload{resource}).is_success());
        resource.reset();
        auto result = channel->try_receive().get_or_throw();
        CHECK(result.resource.get() == identity && *result.resource == 102);
        result.resource.reset();
        CHECK(lifetime.expired());
    }
    auto channel = create_channel<std::any>(1);
    CHECK(channel->try_send(std::string("erased payload")).is_success());
    CHECK(std::any_cast<std::string>(channel->try_receive().get_or_throw()) == "erased payload");
}
void parameter_contract(bool wait, bool reregister, bool fail) {
    int object = 9, registrations = 0, calls = 0;
    void* parameter_identity = nullptr;
    SelectInstance<void*>* instance = nullptr;
    std::weak_ptr<int> resource_lifetime;
    auto selection = std::make_shared<SelectImplementation<void*>>(EmptyCoroutineContext::instance());
    SelectBuilder<void*>& builder = *selection;
    auto failure = std::make_exception_ptr(std::runtime_error("select result failure"));
    SelectClause2Impl<Parameter, int*> clause(&object,
        [&](void*, void* select, void* parameter) {
            instance = static_cast<SelectInstance<void*>*>(select);
            ++registrations;
            auto& value = *static_cast<Parameter*>(parameter);
            CHECK(value.text == "actual parameter" && *value.resource == 71);
            if (!parameter_identity) parameter_identity = parameter;
            CHECK(parameter_identity == parameter);
            if (!wait || (reregister && registrations == 2))
                instance->select_in_registration_phase(&object);
        },
        [&](void*, void* parameter, void* result) -> void* {
            CHECK(parameter == parameter_identity && result == &object);
            CHECK(!resource_lifetime.expired());
            if (fail) std::rethrow_exception(failure);
            return result;
        });
    {
        auto resource = std::make_shared<int>(71);
        resource_lifetime = resource;
        builder.invoke<Parameter, int*>(clause, Parameter{resource, "actual parameter"},
            std::function<void*(int*, Continuation<void*>*)>([&](int* result, auto) -> void* {
                CHECK(result == &object && !resource_lifetime.expired());
                ++calls;
                return nullptr;
            }));
    }
    CHECK(!resource_lifetime.expired());
    if (reregister) CHECK(!instance->try_select(&object, &object));
    Completion completion;
    try {
        auto result = selection->do_select(&completion);
        if (wait && !reregister) {
            CHECK(kotlin::coroutines::intrinsics::is_coroutine_suspended(result) && !completion.resumes);
            CHECK(instance->try_select(&object, &object));
            CHECK(completion.resumes == 1 && completion.failure == (fail ? failure : nullptr));
        } else {
            CHECK(!fail && result == nullptr && !completion.resumes);
        }
    } catch (...) {
        CHECK(fail && (!wait || reregister) && std::current_exception() == failure);
    }
    CHECK(registrations == (reregister ? 2 : 1));
    CHECK(calls == (fail ? 0 : 1) && resource_lifetime.expired());
}
class Dispatcher final : public CoroutineDispatcher {
public:
    mutable std::deque<std::shared_ptr<Runnable>> queue;
    void dispatch(const CoroutineContext&, std::shared_ptr<Runnable> task) const override {
        queue.push_back(std::move(task));
    }
    void drain() {
        while (!queue.empty()) {
            auto task = std::move(queue.front()); queue.pop_front(); task->run();
        }
    }
};
void cancellation_parameter_contract() {
    int object = 12, cancellations = 0, blocks = 0;
    auto dispatcher = std::make_shared<Dispatcher>();
    auto job = JobImpl::create(nullptr);
    Completion completion;
    completion.context = dispatcher->operator+(job);
    auto selection = std::make_shared<SelectImplementation<void*>>(completion.context);
    SelectBuilder<void*>& builder = *selection;
    std::weak_ptr<int> lifetime;
    SelectClause2Impl<Parameter, int*> clause(&object,
        [](void*, void*, void*) {},
        [](void*, void*, void* result) { return result; },
        [&](void*, void* parameter, void*) -> OnCancellationAction {
            auto* value = static_cast<Parameter*>(parameter);
            return [&, value](std::exception_ptr cause, void*, auto) {
                CHECK(cause && value->text == "cancellation parameter" && *value->resource == 82);
                ++cancellations;
            };
        });
    {
        auto resource = std::make_shared<int>(82);
        lifetime = resource;
        builder.invoke<Parameter, int*>(clause, Parameter{resource, "cancellation parameter"},
            std::function<void*(int*, Continuation<void*>*)>([&](int*, auto) -> void* {
                ++blocks; return nullptr;
            }));
    }
    CHECK(kotlin::coroutines::intrinsics::is_coroutine_suspended(selection->do_select(&completion)));
    CHECK(selection->try_select(&object, &object));
    CHECK(!lifetime.expired() && !completion.resumes && !dispatcher->queue.empty());
    job->cancel(nullptr);
    dispatcher->drain();
    CHECK(cancellations == 1 && blocks == 0 && completion.resumes == 1 && completion.failure);
    CHECK(lifetime.expired());
}
void value_result_contract(bool concrete) {
    int object = 10;
    for (bool clause_two : {false, true}) {
        auto selection = std::make_shared<SelectImplementation<void*>>(EmptyCoroutineContext::instance());
        SelectBuilder<void*>& builder = *selection;
        auto registration = [&](void*, void* select, void*) {
            static_cast<SelectInstance<void*>*>(select)->select_in_registration_phase(nullptr);
        };
        auto processing = [](void*, void*, void*) -> void* { return new std::string("selected value"); };
        int calls = 0;
        std::function<void*(std::string, Continuation<void*>*)> block = [&](std::string value, auto) -> void* {
            CHECK(value == "selected value"); ++calls; return nullptr;
        };
        SelectClause1Impl<std::string> one(&object, registration, processing);
        SelectClause2Impl<int, std::string> two(&object, registration, processing);
        if (concrete) {
            if (clause_two) selection->invoke<int, std::string>(two, 42, block);
            else selection->invoke<std::string>(one, block);
        } else {
            if (clause_two) builder.invoke<int, std::string>(two, 42, block);
            else builder.invoke<std::string>(one, block);
        }
        Completion completion;
        CHECK(selection->do_select(&completion) == nullptr && calls == 1 && !completion.resumes);
    }
}
// Source contracts: channels/BufferedChannel.kt:241-349,1475-1501.
void channel_send_contract(bool wait, bool cancel, bool closed) {
    using namespace kotlinx::coroutines::channels;
    struct Prefix : virtual SendChannel<std::string> { virtual ~Prefix() = default; int value = 7; };
    struct DerivedChannel final : Prefix, BufferedChannel<std::string> {
        explicit DerivedChannel(int capacity) : BufferedChannel<std::string>(capacity) {}
    };
    DerivedChannel channel(wait ? 0 : 1);
    SendChannel<std::string>* expected = &channel;
    CHECK(static_cast<void*>(expected) != static_cast<void*>(static_cast<BufferedChannel<std::string>*>(&channel)));
    auto job = JobImpl::create(nullptr);
    Completion completion;
    completion.context = job;
    auto selection = std::make_shared<SelectImplementation<void*>>(completion.context);
    SelectBuilder<void*>& builder = *selection;
    int calls = 0;
    auto cause = std::make_exception_ptr(std::runtime_error("closed select channel"));
    if (closed) channel.close(cause);
    builder.invoke<std::string, SendChannel<std::string>*>(channel.on_send(), "channel parameter",
        std::function<void*(SendChannel<std::string>*, Continuation<void*>*)>(
            [&](SendChannel<std::string>* result, auto) -> void* {
                CHECK(result == expected);
                CHECK(!result->is_closed_for_send());
                ++calls;
                return nullptr;
            }));
    if (closed) {
        try { selection->do_select(&completion); CHECK(false); }
        catch (...) { CHECK(std::current_exception() == cause); }
        CHECK(calls == 0 && !completion.resumes);
        return;
    }
    auto result = selection->do_select(&completion);
    if (wait) {
        CHECK(kotlin::coroutines::intrinsics::is_coroutine_suspended(result));
        CHECK(!completion.resumes && calls == 0);
        if (cancel) {
            job->cancel(nullptr);
            CHECK(completion.resumes == 1 && completion.failure && calls == 0);
            CHECK(channel.try_receive().is_failure());
            CHECK(channel.try_send("after cancellation").is_failure());
            return;
        }
    } else {
        CHECK(result == nullptr && calls == 1 && !completion.resumes);
    }
    auto element = channel.try_receive();
    CHECK(element.is_success() && element.get_or_throw() == "channel parameter");
    CHECK(calls == 1);
    CHECK(completion.resumes == (wait ? 1 : 0) && !completion.failure);
}
// Source contracts: channels/BufferedChannel.kt:241-349,1166-1183,1475-1501.
void buffered_channel_resource_contract() {
    using namespace kotlinx::coroutines::channels;
    BufferedChannel<std::shared_ptr<int>> channel(1);
    for (int iteration = 0; iteration < SEGMENT_SIZE + 3; ++iteration) {
        CHECK(channel.try_send(std::make_shared<int>(-1)).is_success());
        Completion completion;
        auto selection = std::make_shared<SelectImplementation<void*>>(completion.context);
        SelectBuilder<void*>& builder = *selection;
        auto resource = std::make_shared<int>(iteration);
        auto* identity = resource.get();
        std::weak_ptr<int> lifetime = resource;
        int calls = 0;
        builder.invoke<std::shared_ptr<int>, SendChannel<std::shared_ptr<int>>*>(channel.on_send(), resource,
            std::function<void*(SendChannel<std::shared_ptr<int>>*, Continuation<void*>*)>(
                [&](auto* result, auto) -> void* {
                    CHECK(result == static_cast<SendChannel<std::shared_ptr<int>>*>(&channel));
                    CHECK(!lifetime.expired());
                    ++calls;
                    return nullptr;
                }));
        resource.reset();
        CHECK(kotlin::coroutines::intrinsics::is_coroutine_suspended(selection->do_select(&completion)));
        CHECK(!completion.resumes && !lifetime.expired());
        CHECK(*channel.try_receive().get_or_throw() == -1);
        CHECK(completion.resumes == 1 && !completion.failure && calls == 1);
        {
            auto received = channel.try_receive().get_or_throw();
            CHECK(received.get() == identity && *received == iteration);
        }
        CHECK(lifetime.expired());
    }
}
// Source contracts: channels/BufferedChannel.kt:875-961,1504-1567.
void channel_receive_contract(bool catching, bool wait, bool closed, bool cancel, bool reregister = false) {
    using namespace kotlinx::coroutines::channels;
    BufferedChannel<std::shared_ptr<int>> channel(wait ? 0 : 1);
    Completion completion;
    auto job = JobImpl::create(nullptr);
    completion.context = job;
    auto selection = std::make_shared<SelectImplementation<void*>>(completion.context);
    SelectBuilder<void*>& builder = *selection;
    auto resource = std::make_shared<int>(93);
    auto* identity = resource.get();
    std::weak_ptr<int> lifetime = resource;
    int calls = 0;
    auto cause = std::make_exception_ptr(std::runtime_error("receive close cause"));
    if (closed) channel.close(cause);
    else if (!wait && !reregister) CHECK(channel.try_send(resource).is_success());
    auto receive = [&](std::shared_ptr<int> result) -> void* {
        CHECK(result.get() == identity && *result == 93);
        ++calls;
        return nullptr;
    };
    if (catching) {
        builder.invoke<ChannelResult<std::shared_ptr<int>>>(channel.on_receive_catching(),
            std::function<void*(ChannelResult<std::shared_ptr<int>>, Continuation<void*>*)>(
                [&](auto result, auto) -> void* {
                    if (closed) {
                        CHECK(result.is_closed() && result.exception_or_null() == cause);
                        ++calls;
                        return nullptr;
                    }
                    return receive(result.get_or_throw());
                }));
    } else {
        builder.invoke<std::shared_ptr<int>>(channel.on_receive(),
            std::function<void*(std::shared_ptr<int>, Continuation<void*>*)>(
                [&](auto result, auto) { return receive(std::move(result)); }));
    }
    if (reregister) CHECK(channel.try_send(resource).is_success());
    try {
        auto result = selection->do_select(&completion);
        CHECK(!closed || catching);
        if (wait && !closed) {
            CHECK(kotlin::coroutines::intrinsics::is_coroutine_suspended(result));
            CHECK(!completion.resumes && calls == 0);
            if (cancel) {
                job->cancel(nullptr);
                CHECK(completion.resumes == 1 && completion.failure && calls == 0);
                CHECK(channel.try_send(std::make_shared<int>(-1)).is_failure());
            } else {
                CHECK(channel.try_send(resource).is_success());
                CHECK(completion.resumes == 1 && !completion.failure && calls == 1);
            }
        } else CHECK(result == nullptr && !completion.resumes && calls == 1);
    } catch (...) {
        if (!closed || catching) throw;
        CHECK(std::current_exception() == cause && calls == 0);
    }
    resource.reset();
    CHECK(lifetime.expired());
}
// Source contract: channels/BufferedChannel.kt:1561-1567.
class ExceptionHandler final : public CoroutineExceptionHandler {
public:
    int calls = 0;
    CoroutineContext* context = nullptr;
    std::exception_ptr failure;
    void handle_exception(CoroutineContext& actual_context, std::exception_ptr exception) override {
        ++calls;
        context = &actual_context;
        failure = exception;
    }
};
class ReceiveFrame final : public ContinuationImpl {
public:
    using ContinuationImpl::ContinuationImpl;
    void* invoke_suspend(Result<void*> result) override { return result.get_or_throw(); }
};
// Source contract: BufferedChannel.kt:1493-1501.
void closed_select_send_handler_context() {
    using namespace kotlinx::coroutines::channels;
    auto closing = std::make_exception_ptr(std::runtime_error("original closed send"));
    auto failure = std::make_exception_ptr(std::runtime_error("closed handler failure"));
    int deliveries = 0;
    BufferedChannel<std::string> channel(1, [&](auto element) {
        CHECK(element == "original element");
        ++deliveries;
        std::rethrow_exception(failure);
    });
    CHECK(channel.close(closing));
    auto exception_handler = std::make_shared<ExceptionHandler>();
    Completion completion;
    completion.context = exception_handler;
    auto selection = std::make_shared<SelectImplementation<void*>>(completion.context);
    SelectBuilder<void*>& builder = *selection;
    builder.invoke<std::string, SendChannel<std::string>*>(channel.on_send(), "original element",
        std::function<void*(SendChannel<std::string>*, Continuation<void*>*)>(
            [](auto, auto) -> void* { CHECK(false); return nullptr; }));
    try { selection->do_select(&completion); CHECK(false); }
    catch (...) { CHECK(std::current_exception() == closing); }
    CHECK(deliveries == 1 && exception_handler->calls == 1 && !completion.resumes);
    CHECK(exception_handler->context == completion.context.get());
    try { std::rethrow_exception(exception_handler->failure); }
    catch (const kotlinx::coroutines::internal::UndeliveredElementException& exception) {
        CHECK(exception.cause() == failure);
    }
}
// Source contracts: BufferedChannel.kt:652-671,708-733,762-776,1649-1672,1707-1720,2767-2793.
void direct_receive_prompt_cancellation(int kind, bool throwing) {
    using namespace kotlinx::coroutines::channels;
    auto dispatcher = std::make_shared<Dispatcher>();
    auto job = JobImpl::create(nullptr);
    auto exception_handler = std::make_shared<ExceptionHandler>();
    auto completion = std::make_shared<Completion>();
    completion->context = dispatcher->operator+(job)->operator+(exception_handler);
    auto frame = std::make_shared<ReceiveFrame>(completion);
    auto failure = std::make_exception_ptr(std::runtime_error("direct undelivered original"));
    int deliveries = 0;
    auto resource = std::make_shared<int>(103);
    auto* identity = resource.get();
    std::weak_ptr<int> lifetime = resource;
    BufferedChannel<std::shared_ptr<int>> channel(0, [&](auto element) {
        CHECK(element.get() == identity && *element == 103);
        ++deliveries;
        if (throwing) std::rethrow_exception(failure);
    });
    std::shared_ptr<ChannelIterator<std::shared_ptr<int>>> iterator;
    void* result;
    if (kind == 0) result = channel.receive(frame.get());
    else if (kind == 1) result = channel.receive_catching(frame.get());
    else {
        iterator = channel.iterator();
        result = iterator->has_next(frame.get());
    }
    CHECK(kotlin::coroutines::intrinsics::is_coroutine_suspended(result));
    CHECK(channel.try_send(resource).is_success());
    resource.reset();
    CHECK(!lifetime.expired() && !completion->resumes && !dispatcher->queue.empty());
    job->cancel(nullptr);
    dispatcher->drain();
    CHECK(completion->resumes == 1 && completion->failure && deliveries == 1);
    CHECK(exception_handler->calls == (throwing ? 1 : 0));
    if (throwing) {
        CHECK(exception_handler->context == completion->context.get());
        try { std::rethrow_exception(exception_handler->failure); }
        catch (const kotlinx::coroutines::internal::UndeliveredElementException& exception) {
            CHECK(exception.cause() == failure);
        }
    }
    iterator.reset();
    frame.reset();
    CHECK(lifetime.expired());
}
void channel_receive_prompt_cancellation(bool catching, bool handler, bool throwing = false) {
    using namespace kotlinx::coroutines::channels;
    int deliveries = 0, calls = 0;
    int* identity = nullptr;
    auto failure = std::make_exception_ptr(std::runtime_error("undelivered original"));
    OnUndeliveredElement<std::shared_ptr<int>> on_undelivered;
    if (handler) on_undelivered = [&](auto element) {
        CHECK(element.get() == identity && *element == 94);
        ++deliveries;
        if (throwing) std::rethrow_exception(failure);
    };
    BufferedChannel<std::shared_ptr<int>> channel(1, on_undelivered);
    auto dispatcher = std::make_shared<Dispatcher>();
    auto job = JobImpl::create(nullptr);
    Completion completion;
    completion.context = dispatcher->operator+(job);
    auto exception_handler = std::make_shared<ExceptionHandler>();
    completion.context = completion.context->operator+(exception_handler);
    auto selection = std::make_shared<SelectImplementation<void*>>(completion.context);
    SelectBuilder<void*>& builder = *selection;
    if (catching) {
        builder.invoke<ChannelResult<std::shared_ptr<int>>>(channel.on_receive_catching(),
            std::function<void*(ChannelResult<std::shared_ptr<int>>, Continuation<void*>*)>(
                [&](auto, auto) -> void* { ++calls; return nullptr; }));
    } else {
        builder.invoke<std::shared_ptr<int>>(channel.on_receive(),
            std::function<void*(std::shared_ptr<int>, Continuation<void*>*)>(
                [&](auto, auto) -> void* { ++calls; return nullptr; }));
    }
    CHECK(kotlin::coroutines::intrinsics::is_coroutine_suspended(selection->do_select(&completion)));
    auto resource = std::make_shared<int>(94);
    identity = resource.get();
    std::weak_ptr<int> lifetime = resource;
    CHECK(channel.try_send(resource).is_success());
    resource.reset();
    CHECK(!lifetime.expired() && !completion.resumes && !dispatcher->queue.empty());
    job->cancel(nullptr);
    dispatcher->drain();
    CHECK(completion.resumes == 1 && completion.failure && calls == 0);
    CHECK(deliveries == (handler ? 1 : 0) && lifetime.expired());
    CHECK(exception_handler->calls == (throwing ? 1 : 0));
    if (throwing) {
        CHECK(exception_handler->context == completion.context.get());
        try { std::rethrow_exception(exception_handler->failure); }
        catch (const kotlinx::coroutines::internal::UndeliveredElementException& exception) { CHECK(exception.cause() == failure); }
    }
}
// Source contract: internal/OnUndeliveredElement.kt:8-23.
void undelivered_exception_contract() {
    auto first_cause = std::make_exception_ptr(std::runtime_error("first cause"));
    auto next_cause = std::make_exception_ptr(std::runtime_error("next cause"));
    kotlinx::coroutines::internal::OnUndeliveredElement<std::string> first_handler = [&](auto) { std::rethrow_exception(first_cause); };
    kotlinx::coroutines::internal::OnUndeliveredElement<std::string> next_handler = [&](auto) { std::rethrow_exception(next_cause); };
    std::unique_ptr<kotlinx::coroutines::internal::UndeliveredElementException> first(
        kotlinx::coroutines::internal::call_undelivered_element_catching_exception(first_handler, std::string("first element")));
    CHECK(first && first->cause() == first_cause);
    CHECK(std::string(first->what()) == "Exception in undelivered element handler for first element");
    CHECK(kotlinx::coroutines::internal::call_undelivered_element_catching_exception(next_handler, std::string("next element"), first.get()) == first.get());
    CHECK(first->suppressed_exceptions().size() == 1 && first->suppressed_exceptions()[0] == next_cause);
    std::unique_ptr<kotlinx::coroutines::internal::UndeliveredElementException> repeated(
        kotlinx::coroutines::internal::call_undelivered_element_catching_exception(first_handler, std::string("repeated element"), first.get()));
    CHECK(repeated.get() != first.get() && repeated->cause() == first_cause);
    CHECK(repeated->suppressed_exceptions().empty());
    kotlinx::coroutines::internal::OnUndeliveredElement<int> non_standard = [](int) { throw 17; };
    std::unique_ptr<kotlinx::coroutines::internal::UndeliveredElementException> other(
        kotlinx::coroutines::internal::call_undelivered_element_catching_exception(non_standard, 96));
    CHECK(other != nullptr);
    try { std::rethrow_exception(other->cause()); }
    catch (int value) { CHECK(value == 17); }
    kotlinx::coroutines::internal::OnUndeliveredElement<Unit> unit_handler = [&](Unit) { std::rethrow_exception(first_cause); };
    std::unique_ptr<kotlinx::coroutines::internal::UndeliveredElementException> unit(
        kotlinx::coroutines::internal::call_undelivered_element_catching_exception(unit_handler, Unit{}));
    CHECK(std::string(unit->what()) == "Exception in undelivered element handler for kotlin.Unit");
    using flow::SharingCommand;
    kotlinx::coroutines::internal::OnUndeliveredElement<SharingCommand> enum_handler = [&](auto) { std::rethrow_exception(first_cause); };
    for (auto command : {SharingCommand::START, SharingCommand::STOP, SharingCommand::STOP_AND_RESET_REPLAY_CACHE}) {
        std::unique_ptr<kotlinx::coroutines::internal::UndeliveredElementException> exception(
            kotlinx::coroutines::internal::call_undelivered_element_catching_exception(enum_handler, command));
        CHECK(std::string(exception->what()) == "Exception in undelivered element handler for " + flow::to_string(command));
    }
}
// Source contract: channels/ConflatedBufferedChannel.kt:31-88.
void conflated_channel_contract() {
    using namespace kotlinx::coroutines::channels;
    Completion completion;
    auto failure = std::make_exception_ptr(std::runtime_error("conflated handler failure"));
    auto closing = std::make_exception_ptr(std::runtime_error("conflated close failure"));
    std::string undelivered;
    ConflatedBufferedChannel<std::string> latest(1, BufferOverflow::DROP_LATEST, [&](auto element) {
        undelivered = element;
        std::rethrow_exception(failure);
    });
    CHECK(latest.try_send("buffered").is_success());
    CHECK(latest.try_send("caller-owned drop").is_success() && undelivered.empty());
    try { latest.send("dropped by send", &completion); CHECK(false); }
    catch (const kotlinx::coroutines::internal::UndeliveredElementException& exception) {
        CHECK(exception.cause() == failure && undelivered == "dropped by send");
    }
    CHECK(latest.try_receive().get_or_throw() == "buffered");
    latest.close(closing);
    try { latest.send("closed original value", &completion); CHECK(false); }
    catch (const kotlinx::coroutines::internal::UndeliveredElementException& exception) {
        CHECK(exception.cause() == failure && undelivered == "closed original value");
        CHECK(exception.suppressed_exceptions().size() == 1 && exception.suppressed_exceptions()[0] == closing);
    }
    ConflatedBufferedChannel<std::shared_ptr<int>> oldest(1, BufferOverflow::DROP_OLDEST,
        [&](auto element) { CHECK(*element == 99); std::rethrow_exception(failure); });
    auto dropped = std::make_shared<int>(99);
    std::weak_ptr<int> dropped_lifetime = dropped;
    CHECK(oldest.try_send(dropped).is_success());
    dropped.reset();
    try { oldest.try_send(std::make_shared<int>(100)); CHECK(false); }
    catch (const kotlinx::coroutines::internal::UndeliveredElementException& exception) { CHECK(exception.cause() == failure); }
    CHECK(dropped_lifetime.expired());
    CHECK(*oldest.try_receive().get_or_throw() == 100);
    for (bool repeated : {false, true}) {
        std::vector<int> called;
        BufferedChannel<int> cancelled(2, [&](int element) {
            called.push_back(element);
            std::rethrow_exception(element == 2 || repeated ? failure : closing);
        });
        CHECK(cancelled.try_send(1).is_success() && cancelled.try_send(2).is_success());
        try { cancelled.cancel(); CHECK(false); }
        catch (const kotlinx::coroutines::internal::UndeliveredElementException& exception) {
            CHECK(exception.cause() == failure);
            CHECK(exception.suppressed_exceptions().size() == (repeated ? 0 : 1));
            if (!repeated) CHECK(exception.suppressed_exceptions()[0] == closing);
        }
        CHECK(called == std::vector<int>({2, 1}));
    }
    for (auto overflow : {BufferOverflow::DROP_OLDEST, BufferOverflow::DROP_LATEST}) {
        ConflatedBufferedChannel<std::shared_ptr<int>> channel(1, overflow);
        auto resource = std::make_shared<int>(97);
        std::weak_ptr<int> lifetime = resource;
        CHECK(channel.try_send(resource).is_success());
        resource.reset();
        auto selection = std::make_shared<SelectImplementation<void*>>(completion.context);
        int calls = 0;
        SelectBuilder<void*>& builder = *selection;
        builder.invoke<std::shared_ptr<int>, SendChannel<std::shared_ptr<int>>*>(channel.on_send(), std::make_shared<int>(98),
            std::function<void*(SendChannel<std::shared_ptr<int>>*, Continuation<void*>*)>(
                [&](auto* result, auto) -> void* { CHECK(result == &channel); ++calls; return nullptr; }));
        CHECK(selection->do_select(&completion) == nullptr && calls == 1 && !completion.resumes);
        auto received = channel.try_receive().get_or_throw();
        CHECK(*received == (overflow == BufferOverflow::DROP_OLDEST ? 98 : 97));
        received.reset();
        CHECK(lifetime.expired());
    }
}
// Source contract: channels/BufferedChannel.kt:1504-1510,1544-1546.
void channel_receive_borrowed_pointer(bool wait) {
    using namespace kotlinx::coroutines::channels;
    const int value = 95;
    BufferedChannel<const int*> channel(wait ? 0 : 1);
    if (!wait) CHECK(channel.try_send(&value).is_success());
    Completion completion;
    auto selection = std::make_shared<SelectImplementation<void*>>(completion.context);
    SelectBuilder<void*>& builder = *selection;
    int calls = 0;
    builder.invoke<const int*>(channel.on_receive(),
        std::function<void*(const int*, Continuation<void*>*)>([&](auto* result, auto) -> void* {
            CHECK(result == &value && *result == 95);
            ++calls;
            return nullptr;
        }));
    auto result = selection->do_select(&completion);
    if (wait) {
        CHECK(kotlin::coroutines::intrinsics::is_coroutine_suspended(result) && calls == 0);
        CHECK(channel.try_send(&value).is_success());
        CHECK(completion.resumes == 1 && !completion.failure);
    } else CHECK(result == nullptr && !completion.resumes);
    CHECK(calls == 1 && value == 95);
}
}
int main() {
    try {
        for (bool fail : {false, true}) {
            parameter_contract(false, false, fail);
            parameter_contract(true, false, fail);
            parameter_contract(true, true, fail);
        }
        value_result_contract(false);
        value_result_contract(true);
        cancellation_parameter_contract();
        channel_send_contract(false, false, false);
        channel_send_contract(true, false, false);
        channel_send_contract(true, true, false);
        channel_send_contract(false, false, true);
        buffered_channel_resource_contract();
        for (bool catching : {false, true}) {
            channel_receive_contract(catching, false, false, false);
            channel_receive_contract(catching, true, false, false);
            channel_receive_contract(catching, true, false, true);
            channel_receive_contract(catching, false, true, false);
            channel_receive_contract(catching, false, false, false, true);
            channel_receive_prompt_cancellation(catching, false);
            channel_receive_prompt_cancellation(catching, true);
            channel_receive_prompt_cancellation(catching, true, true);
        }
        channel_receive_borrowed_pointer(false);
        channel_receive_borrowed_pointer(true);
        closed_select_send_handler_context();
        for (int kind = 0; kind < 3; ++kind)
            for (bool throwing : {false, true}) direct_receive_prompt_cancellation(kind, throwing);
        undelivered_exception_contract();
        conflated_channel_contract();
        channel_without_handler_contract();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
