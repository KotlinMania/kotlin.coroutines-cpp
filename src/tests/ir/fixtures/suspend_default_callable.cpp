// NOTE(port): A default can construct a suspend callable without executing its body.
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include <cassert>
#include <functional>
using namespace kotlinx::coroutines;
using Completion = std::shared_ptr<Continuation<void*>>;
using Callable = std::function<void*(Completion)>;
void invoke(Callable& callable);
[[suspend]] void* source(Completion caller);
void accepts(Callable callable = [] [[suspend]] (Completion caller) -> void* {
    return source(caller);
});
void accepts_resuming(Callable callable = [] [[suspend]] (Completion caller) -> void* {
    void* result = source(caller);
    return result;
});
void accepts_defined(Callable callable = [] [[suspend]] (Completion caller) -> void* {
    void* result = source(caller);
    return result;
}) {
    invoke(callable);
}
namespace defaults {
void accepts(Callable callable = [] [[suspend]] (Completion caller) -> void* {
    void* result = source(caller);
    return result;
});
}
Callable global_callable = [] [[suspend]] (Completion caller) -> void* {
    void* result = source(caller);
    return result;
};
struct DefaultMember {
    Callable callable = [] [[suspend]] (Completion caller) -> void* {
        void* result = source(caller);
        return result;
    };
    void accepts(Callable callable = [] [[suspend]] (Completion caller) -> void* {
        void* result = source(caller);
        return result;
    });
};

#if defined(KXS_TEST_DEFAULT_CALLABLE_RUNTIME)
int mode = 0;
Completion pending;
Completion completion;
void* output = nullptr;

void* source(Completion caller) {
    if (mode == 4) throw std::runtime_error("immediate default callable failure");
    if (mode) {
        pending = std::move(caller);
        return intrinsics::get_COROUTINE_SUSPENDED();
    }
    return new int(42);
}
void invoke(Callable& callable) { output = callable(completion); }
void accepts(Callable callable) { invoke(callable); }
void accepts_resuming(Callable callable) { invoke(callable); }
void defaults::accepts(Callable callable) { invoke(callable); }
void DefaultMember::accepts(Callable callable) { invoke(callable); }

struct Done final : Continuation<void*> {
    int calls = 0;
    int value = 0;
    bool failed = false;
    bool cancelled = false;
    std::shared_ptr<CoroutineContext> get_context() const override {
        return EmptyCoroutineContext::instance();
    }
    void resume_with(Result<void*> result) override {
        ++calls;
        try {
            std::unique_ptr<int> box(static_cast<int*>(result.get_or_throw()));
            value = *box;
        } catch (const CancellationException&) { failed = cancelled = true; }
        catch (const std::runtime_error&) { failed = true; }
    }
};
int main() {
    DefaultMember member;
    for (int variant = 0; variant != 7; ++variant) {
        for (mode = 0; mode != 5; ++mode) {
            auto done = std::make_shared<Done>();
            completion = done;
            output = nullptr;
            try {
                switch (variant) {
                    case 0: accepts(); break;
                    case 1: accepts_resuming(); break;
                    case 2: accepts_defined(); break;
                    case 3: defaults::accepts(); break;
                    case 4: invoke(global_callable); break;
                    case 5: invoke(member.callable); break;
                    case 6: member.accepts(); break;
                }
                if (mode) {
                    assert(intrinsics::is_coroutine_suspended(output));
                    assert(pending && done->calls == 0);
                    assert((pending == done) == (variant == 0));
                    auto held = std::move(pending);
                    if (mode == 1) held->resume_with(Result<void*>::success(new int(42)));
                    else if (mode == 2) held->resume_with(Result<void*>::failure(
                        std::make_exception_ptr(std::runtime_error("resumed default callable failure"))));
                    else held->resume_with(Result<void*>::failure(
                        std::make_exception_ptr(CancellationException("default callable cancellation"))));
                } else done->resume_with(Result<void*>::success(output));
            } catch (const std::runtime_error&) {
                assert(mode == 4);
                done->resume_with(Result<void*>::failure(std::current_exception()));
            }
            completion.reset();
            assert(done->calls == 1 && done->failed == (mode >= 2));
            assert(done->cancelled == (mode == 3) && !pending);
            if (!done->failed) assert(done->value == 42);
        }
    }
}
#endif
