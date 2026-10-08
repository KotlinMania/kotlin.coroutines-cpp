// Source contracts: kotlinx-coroutines-core/common/src/selects/Select.kt:463-470,488-521,612-617,707-724,824-848.
#include "kotlinx/coroutines/selects/Select.hpp"
#include "kotlinx/coroutines/CoroutineDispatcher.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include <deque>
#include <iostream>
#include <string>
#include <stdexcept>

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
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
