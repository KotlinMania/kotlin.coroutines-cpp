// NOTE(port): Clang integration regression for ordinary C++ function-address
// dependencies and native catch identity across repeated suspension. This is
// compiler test infrastructure, not a Kotlin source function transliteration.
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include <cassert>
#include <iostream>
#include <variant>
using namespace kotlinx::coroutines;
using namespace kotlinx::coroutines::dsl;

std::shared_ptr<Continuation<void*>> pending;
std::weak_ptr<Continuation<void*>> frame;
int calls = 0;
int alive = 0;
int destroyed = 0;
struct Owned {
    int value = 40;
    Owned() { ++alive; }
    ~Owned() { --alive; ++destroyed; }
};
struct DomainFailure : std::runtime_error {
    DomainFailure() : std::runtime_error("domain") {}
};
template<int Increment> int adjust(int value) { return value + Increment; }
int unbox(void* raw) {
    std::unique_ptr<int> value(static_cast<int*>(raw));
    return *value;
}
[[clang::annotate("suspend")]]
void* wait_value(int mode, std::shared_ptr<Continuation<void*>> completion) {
    ++calls;
    if (mode == 4) throw std::runtime_error("immediate");
    if (!mode) return new int(1);
    frame = completion;
    pending = std::move(completion);
    return intrinsics::get_COROUTINE_SUSPENDED();
}
[[suspend]]
void* addressed(int mode, std::shared_ptr<Continuation<void*>> completion) {
    auto owned = std::make_unique<Owned>();
    try {
        owned->value += unbox(suspend(wait_value(mode, completion)));
        throw DomainFailure();
    } catch (const DomainFailure& error) {
        const auto* identity = &error;
        auto cause = std::current_exception();
        // Both alternatives exercise libc++'s constexpr function dispatch tables.
        std::variant<int, std::exception_ptr> value(owned->value);
        value = cause;
        assert(std::get<std::exception_ptr>(value) == cause);
        value = owned->value;
        constexpr int (*operations[])(int) = {adjust<1>, adjust<2>};
        for (int index = 0; index != 2; ++index) {
            owned->value = operations[index](owned->value);
            owned->value += unbox(suspend(wait_value(mode, completion)));
            assert(&error == identity && std::current_exception() == cause);
            assert(std::get<int>(value) == 41);
        }
        return new int(owned->value);
    }
}
struct Done final : Continuation<void*> {
    int completions = 0;
    int value = 0;
    bool failed = false;
    bool cancelled = false;
    std::shared_ptr<CoroutineContext> get_context() const override { return EmptyCoroutineContext::instance(); }
    void resume_with(Result<void*> result) override {
        ++completions;
        try { value = unbox(result.get_or_throw()); }
        catch (const CancellationException&) { failed = cancelled = true; }
        catch (const std::runtime_error&) { failed = true; }
    }
};
int main() {
    for (int mode = 0; mode != 5; ++mode) {
        calls = destroyed = 0;
        auto done = std::make_shared<Done>();
        try {
            auto result = addressed(mode, done);
            if (!intrinsics::is_coroutine_suspended(result))
                done->resume_with(Result<void*>::success(result));
            else while (pending) {
                assert(alive == 1 && done->completions == 0 && pending != done);
                auto held = std::move(pending);
                if (mode == 2 && calls == 2)
                    held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("resumed"))));
                else if (mode == 3 && calls == 2)
                    held->resume_with(Result<void*>::failure(std::make_exception_ptr(CancellationException("cancelled"))));
                else held->resume_with(Result<void*>::success(new int(1)));
            }
        } catch (const std::runtime_error&) {
            done->resume_with(Result<void*>::failure(std::current_exception()));
        }
        assert(done->completions == 1 && done->failed == (mode >= 2));
        assert(done->cancelled == (mode == 3));
        assert(done->failed || done->value == 46);
        assert(calls == (mode == 4 ? 1 : mode >= 2 ? 2 : 3));
        assert(alive == 0 && destroyed == 1 && !pending && frame.expired());
    }
    std::cout << "function addresses:46; catch identity; repeated suspension; failure; cancellation; cleanup\n";
}
