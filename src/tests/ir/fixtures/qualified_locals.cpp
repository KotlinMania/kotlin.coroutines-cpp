// NOTE(port): Compiler execution regression for declared C++ cv-qualification,
// ordinary object identity and destruction in Native-derived spill storage.
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include "kotlinx/coroutines/Exceptions.hpp"
#include <cassert>
#include <memory>
#include <stdexcept>
using namespace kotlinx::coroutines;

int alive = 0;
int mode = 0;
int calls = 0;
std::shared_ptr<Continuation<void*>> pending;
std::weak_ptr<Continuation<void*>> frame;
struct QualifiedValue {
    explicit QualifiedValue(int value) : value_(value) { ++alive; }
    QualifiedValue(const QualifiedValue&) = delete;
    ~QualifiedValue() { --alive; }
    int read() & { return -1000; }
    int read() const & { return value_; }
    int read() volatile & { return value_ + 1; }
    int read() const volatile & { return value_ + 2; }
private:
    int value_;
};
const QualifiedValue* fixed_identity = nullptr;
const volatile QualifiedValue* observed_identity = nullptr;
const volatile QualifiedValue* both_identity = nullptr;
int* started_identity = nullptr;
int* finished_identity = nullptr;

[[clang::annotate("suspend")]]
void* await_value(std::shared_ptr<Continuation<void*>> completion) {
    ++calls;
    frame = completion;
    if (mode == 4) throw std::runtime_error("immediate failure");
    if (mode == 0) return new int(7);
    pending = std::move(completion);
    return intrinsics::get_COROUTINE_SUSPENDED();
}

[[clang::annotate("suspend")]]
void* qualified_locals(std::shared_ptr<Continuation<void*>> completion) {
    static int started = 0, finished = 0;
    if (started_identity) {
        assert(started_identity == std::addressof(started));
        assert(finished_identity == std::addressof(finished));
    }
    started_identity = std::addressof(started);
    finished_identity = std::addressof(finished);
    ++started;
    const auto fixed = QualifiedValue(42);
    volatile QualifiedValue observed(43);
    const volatile QualifiedValue both(44);
    fixed_identity = std::addressof(fixed);
    observed_identity = std::addressof(observed);
    both_identity = std::addressof(both);
    int total = 0;
    for (int index = 0; index < 2; ++index) {
        total += fixed.read() + observed.read() + both.read();
        void* raw = dsl::suspend(await_value(completion));
        std::unique_ptr<int> box(static_cast<int*>(raw));
        assert(std::addressof(fixed) == fixed_identity);
        assert(std::addressof(observed) == observed_identity);
        assert(std::addressof(both) == both_identity);
        total += fixed.read() + observed.read() + both.read() + *box;
    }
    ++finished;
    return new int(total);
}
struct Done final : Continuation<void*> {
    int resumes = 0;
    int value = 0;
    bool failed = false;
    bool cancelled = false;
    std::shared_ptr<CoroutineContext> get_context() const override { return EmptyCoroutineContext::instance(); }
    void resume_with(Result<void*> result) override {
        ++resumes;
        try {
            std::unique_ptr<int> box(static_cast<int*>(result.get_or_throw()));
            value = *box;
        } catch (const CancellationException&) { failed = cancelled = true; }
        catch (const std::runtime_error&) { failed = true; }
    }
};
int main() {
    for (mode = 0; mode < 5; ++mode) {
        calls = 0;
        auto done = std::make_shared<Done>();
        try {
            void* result = qualified_locals(done);
            if (intrinsics::is_coroutine_suspended(result)) {
                while (pending) {
                    assert(alive == 3 && done->resumes == 0);
                    auto held = std::move(pending);
                    if (mode == 2 && calls == 2)
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(std::runtime_error("resumed failure"))));
                    else if (mode == 3 && calls == 2)
                        held->resume_with(Result<void*>::failure(std::make_exception_ptr(CancellationException("cancelled"))));
                    else held->resume_with(Result<void*>::success(new int(7)));
                }
            } else done->resume_with(Result<void*>::success(result));
        } catch (...) { done->resume_with(Result<void*>::failure(std::current_exception())); }
        assert(done->resumes == 1 && done->failed == (mode >= 2));
        assert(done->cancelled == (mode == 3));
        assert(done->failed || done->value == 542);
        assert(calls == (mode == 4 ? 1 : 2));
        assert(alive == 0 && !pending && frame.expired());
        assert(*started_identity == mode + 1);
        assert(*finished_identity == (mode == 0 ? 1 : 2));
    }
}
