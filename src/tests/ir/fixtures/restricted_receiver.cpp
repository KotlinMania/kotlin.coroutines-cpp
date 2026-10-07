// NOTE(port): Compiler regression for Kotlin restricted receiver roles and
// NativeSuspendFunctionsLowering.getCoroutineBaseClass, not a library port.
#include "kotlinx/coroutines/dsl/Coroutines.hpp"
#include "kotlinx/coroutines/JobImpl.hpp"
#include <cassert>
#include <iostream>
using namespace kotlinx::coroutines;

struct [[kotlinx::restricts_suspension]] Root { int calls = 0; };
struct Left : virtual Root {};
struct Right : virtual Root {};
struct Scope : Left, Right {};
std::shared_ptr<Continuation<void*>> pending;
[[clang::annotate("suspend")]] void* suspend_value(std::shared_ptr<Continuation<void*>> completion) {
    pending = std::move(completion);
    return intrinsics::get_COROUTINE_SUSPENDED();
}
[[suspend]] void* restricted_value([[kotlinx::extension_receiver]] Scope& receiver,
                                  std::shared_ptr<Continuation<void*>> caller) {
    void* value = suspend_value(caller);
    ++receiver.calls;
    return value;
}
[[suspend]] void* ordinary_value(Scope& receiver, std::shared_ptr<Continuation<void*>> caller) {
    void* value = suspend_value(caller);
    ++receiver.calls;
    return value;
}
struct Done final : Continuation<void*> {
    std::shared_ptr<CoroutineContext> context;
    int calls = 0;
    int value = 0;
    explicit Done(std::shared_ptr<CoroutineContext> context) : context(std::move(context)) {}
    std::shared_ptr<CoroutineContext> get_context() const override { return context; }
    void resume_with(Result<void*> result) override {
        std::unique_ptr<int> box(static_cast<int*>(result.get_or_throw()));
        value = *box;
        ++calls;
    }
};
int main() {
    Scope scope;
    auto empty = std::make_shared<Done>(EmptyCoroutineContext::instance());
    assert(intrinsics::is_coroutine_suspended(restricted_value(scope, empty)));
    auto restricted = std::dynamic_pointer_cast<RestrictedContinuationImpl>(std::move(pending));
    assert(restricted && !std::dynamic_pointer_cast<ContinuationImpl>(restricted));
    assert(restricted->get_context() == EmptyCoroutineContext::instance());
    restricted->resume_with(Result<void*>::success(new int(42)));
    assert(scope.calls == 1 && empty->calls == 1 && empty->value == 42);
    auto nonempty = std::make_shared<Done>(JobImpl::create(nullptr));
    bool rejected = false;
    try { restricted_value(scope, nonempty); }
    catch (const std::invalid_argument& error) {
        rejected = true;
        assert(std::string(error.what()) == "Coroutines with restricted suspension must have EmptyCoroutineContext");
    }
    assert(rejected && !pending && scope.calls == 1 && nonempty->calls == 0);
    assert(intrinsics::is_coroutine_suspended(ordinary_value(scope, nonempty)));
    auto ordinary = std::dynamic_pointer_cast<ContinuationImpl>(std::move(pending));
    assert(ordinary && !std::dynamic_pointer_cast<RestrictedContinuationImpl>(ordinary));
    assert(ordinary->get_context() == nonempty->context);
    ordinary->resume_with(Result<void*>::success(new int(43)));
    assert(scope.calls == 2 && nonempty->calls == 1 && nonempty->value == 43);
    std::cout << "restricted receiver:42; ordinary argument:43\n";
}
