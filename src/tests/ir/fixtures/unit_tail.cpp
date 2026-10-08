#include <kotlinx/coroutines/ContinuationImpl.hpp>
#include <kotlinx/coroutines/dsl/Suspend.hpp>
#include <cassert>
#include <stdexcept>
using namespace kotlinx::coroutines;
using Completion = std::shared_ptr<Continuation<void*>>;
int mode;
Completion pending;
[[clang::annotate("suspend")]] void* unit_call(Completion caller) {
    if (mode == 3) throw std::runtime_error("failure");
    if (mode) { pending = caller; return intrinsics::get_COROUTINE_SUSPENDED(); }
    return nullptr;
}
[[suspend, clang::annotate("kotlin.ir.UnitReturn")]] void* unit_tail(Completion caller) {
    kotlinx::coroutines::dsl::suspend(unit_call(caller));
    return nullptr;
}
[[suspend, clang::annotate("kotlin.ir.UnitReturn")]] void* conditional_unit_tail(bool condition, Completion caller) {
    condition ? unit_call(caller) : unit_call(caller);
    return nullptr;
}
// NOTE(port): Exercise completed template body delivery through nested owning
// frames. The primary body must wait for its constexpr condition to resolve.
template<int Depth>
[[suspend]] void* recursive_unit(std::shared_ptr<int> owner, Completion caller) {
    if constexpr (Depth == 0) {
        return unit_call(caller);
    } else {
        auto* identity = owner.get();
        auto result = recursive_unit<Depth - 1>(owner, caller);
        assert(owner.get() == identity && *owner == 42);
        return result;
    }
}
struct Done : Continuation<void*> {
    int calls = 0;
    bool failed = false;
    std::shared_ptr<CoroutineContext> get_context() const override { return EmptyCoroutineContext::instance(); }
    void resume_with(Result<void*> result) override {
        ++calls;
        try { assert(result.get_or_throw() == nullptr); }
        catch (const std::runtime_error&) { failed = true; }
    }
};
int main() {
    for (int variant : {0, 1, 2}) for (mode = 0; mode < 4; ++mode) {
        auto done = std::make_shared<Done>();
        auto owner = std::make_shared<int>(42);
        std::weak_ptr<int> retained = owner;
        try {
            auto result = variant == 2 ? recursive_unit<2>(owner, done) :
                variant == 1 ? conditional_unit_tail(mode % 2, done) : unit_tail(done);
            owner.reset();
            if (mode == 1 || mode == 2) {
                assert(intrinsics::is_coroutine_suspended(result));
                assert(pending && done->calls == 0);
                if (variant == 2) assert(!retained.expired() && pending != done);
                else assert(pending == done);
                auto held = std::move(pending);
                if (mode == 1) held->resume_with(Result<void*>::success(nullptr));
                else held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("failure"))));
            } else { assert(result == nullptr); done->resume_with(Result<void*>::success(result)); }
        } catch (const std::runtime_error&) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        owner.reset();
        assert(done->calls == 1 && done->failed == (mode >= 2) && !pending);
        assert(retained.expired());
    }
}
