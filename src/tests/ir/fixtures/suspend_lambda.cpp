// NOTE(port): Compiler regression for the restricted-reference metadata path,
// captured field identity and immediate/resumed lambda invocation.
#include "kotlinx/coroutines/dsl/Coroutines.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include <cassert>
#include <iostream>
using namespace kotlinx::coroutines;
struct [[kotlinx::restricts_suspension]] Scope { int calls = 0; };
std::shared_ptr<Continuation<void*>> pending;
[[clang::annotate("suspend")]] void* suspend_value(int mode, std::shared_ptr<Continuation<void*>> completion) {
    if (!mode) return new int(41);
    pending = std::move(completion);
    return intrinsics::get_COROUTINE_SUSPENDED();
}
struct Done final : Continuation<void*> {
    std::shared_ptr<CoroutineContext> context;
    int calls = 0;
    int value = 0;
    bool failed = false;
    explicit Done(std::shared_ptr<CoroutineContext> context) : context(std::move(context)) {}
    std::shared_ptr<CoroutineContext> get_context() const override { return context; }
    void resume_with(Result<void*> result) override {
        ++calls;
        try {
            std::unique_ptr<int> box(static_cast<int*>(result.get_or_throw()));
            value = *box;
        } catch (const std::runtime_error&) { failed = true; }
    }
};
struct Owner {
    int bias = 40;
    auto block() {
        return [this] [[suspend]] (int mode, std::shared_ptr<Continuation<void*>> caller) -> void* {
            void* raw = suspend_value(mode, caller);
            std::unique_ptr<int> box(static_cast<int*>(raw));
            return new int(*box + bias);
        };
    }
};
int main() {
    Scope scope;
    int visits = 0;
    auto block = [&visits, bias = 1] [[suspend]] ([[kotlinx::extension_receiver]] Scope& receiver,
        int mode, std::shared_ptr<Continuation<void*>> caller) -> void* {
        void* raw = suspend_value(mode, caller);
        ++receiver.calls;
        ++visits;
        std::unique_ptr<int> box(static_cast<int*>(raw));
        return new int(*box + bias);
    };
    auto empty = std::make_shared<Done>(EmptyCoroutineContext::instance());
    std::unique_ptr<int> immediate(static_cast<int*>(block(scope, 0, empty)));
    assert(*immediate == 42 && empty->calls == 0 && visits == 1);
    assert(intrinsics::is_coroutine_suspended(block(scope, 1, empty)));
    auto restricted = std::dynamic_pointer_cast<RestrictedContinuationImpl>(std::move(pending));
    assert(restricted && !std::dynamic_pointer_cast<ContinuationImpl>(restricted));
    restricted->resume_with(Result<void*>::success(new int(41)));
    assert(empty->calls == 1 && empty->value == 42 && visits == 2 && scope.calls == 2);
    assert(intrinsics::is_coroutine_suspended(block(scope, 1, empty)));
    auto failure = std::move(pending);
    failure->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("resumed"))));
    assert(empty->calls == 2 && empty->failed && visits == 2 && scope.calls == 2);
    auto nonempty = std::make_shared<Done>(JobImpl::create(nullptr));
    bool rejected = false;
    try { block(scope, 1, nonempty); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected && !pending && nonempty->calls == 0);
    auto ordinary = [] [[suspend]] (Scope& receiver, std::shared_ptr<Continuation<void*>> caller) -> void* {
        void* value = suspend_value(1, caller);
        ++receiver.calls;
        return value;
    };
    assert(intrinsics::is_coroutine_suspended(ordinary(scope, nonempty)));
    auto frame = std::dynamic_pointer_cast<ContinuationImpl>(std::move(pending));
    assert(frame && frame->get_context() == nonempty->context);
    frame->resume_with(Result<void*>::success(new int(43)));
    assert(nonempty->calls == 1 && nonempty->value == 43 && scope.calls == 3);
    Owner owner;
    auto member = owner.block();
    auto member_done = std::make_shared<Done>(EmptyCoroutineContext::instance());
    assert(intrinsics::is_coroutine_suspended(member(1, member_done)));
    owner.bias = 41;
    auto member_frame = std::move(pending);
    member_frame->resume_with(Result<void*>::success(new int(41)));
    assert(member_done->calls == 1 && member_done->value == 82);
    std::cout << "suspend lambda:42; ordinary:43; captured this:82\n";
}
