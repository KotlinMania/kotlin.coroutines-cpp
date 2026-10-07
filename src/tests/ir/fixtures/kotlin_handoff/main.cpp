#include "api.hpp"
#include "actual_api.h"
#include <kotlinx/coroutines/CancellableContinuationImpl.hpp>
#include <cassert>
#include <functional>
#include <iostream>
#include <thread>

struct Session {
    int mode = 0;
    void* pending = nullptr;
    int pending_value = 0;
    int stage = 0;
    int completions = 0;
    int identity_matches = 0;
    std::function<void(Result<int>)> finish;
    std::weak_ptr<Continuation<void*>> cpp_frame;
};
Session session;

extern "C" int suspended(void* owner, void* handle, int value, int stage) {
    auto& state = *static_cast<Session*>(owner);
    if (state.mode == 0) return 1;
    assert(!state.pending);
    state.pending = handle;
    state.pending_value = value + 1;
    state.stage = stage;
    return 0;
}
extern "C" void completed(void* owner, int status, int value, int identical) {
    auto& state = *static_cast<Session*>(owner);
    ++state.completions;
    state.identity_matches = identical;
    auto finish = std::exchange(state.finish, {});
    assert(finish);
    if (status == 0) finish(Result<int>::success(value));
    else if (status == 2) finish(Result<int>::failure(std::make_exception_ptr(CancellationException("Kotlin cancellation"))));
    else finish(Result<int>::failure(std::make_exception_ptr(std::runtime_error("Kotlin failure"))));
}
void* kotlin_value(int value, std::shared_ptr<Continuation<void*>> completion) {
    session.cpp_frame = completion;
    return suspend_cancellable_coroutine<int>([value](CancellableContinuation<int>& continuation) {
        auto& implementation = dynamic_cast<CancellableContinuationImpl<int>&>(continuation);
        auto retained = implementation.shared_from_this();
        session.finish = [retained](Result<int> result) { retained->resume_with(std::move(result)); };
        kxs_kotlin_start(value, &session, reinterpret_cast<void*>(&suspended), reinterpret_cast<void*>(&completed));
    }, std::move(completion));
}
struct Done : Continuation<void*> {
    int calls = 0;
    int value = 0;
    int status = 0;
    std::shared_ptr<CoroutineContext> get_context() const override { return EmptyCoroutineContext::instance(); }
    void resume_with(Result<void*> result) override {
        ++calls;
        try {
            auto boxed = std::unique_ptr<int>(static_cast<int*>(result.get_or_throw()));
            value = *boxed;
        } catch (const CancellationException&) { status = 2; }
        catch (const std::runtime_error&) { status = 1; }
    }
};
int main() {
    for (int mode = 0; mode != 4; ++mode) {
        session = Session{};
        session.mode = mode;
        auto done = std::make_shared<Done>();
        auto result = cpp_value(41, done);
        if (intrinsics::is_coroutine_suspended(result)) {
            assert(done->calls == 0 && session.pending && !session.cpp_frame.expired());
            while (session.pending) {
                auto handle = std::exchange(session.pending, nullptr);
                int status = mode == 2 ? 1 : (mode == 3 && session.stage == 1 ? 2 : 0);
                int value = session.pending_value;
                std::thread worker([=] { kxs_kotlin_resume(handle, value, status); });
                worker.join();
            }
        } else done->resume_with(Result<void*>::success(result));
        assert(done->calls == 1 && session.completions == 1);
        assert(done->status == (mode == 2 ? 1 : mode == 3 ? 2 : 0));
        if (done->status == 0) {
            assert(done->value == 399);
            assert(session.identity_matches == 2);
        }
        assert(kxs_kotlin_outstanding_handles() == 0);
        assert(session.cpp_frame.expired() && !session.pending && !session.finish);
        std::cout << "kotlin-cpp:" << mode << ":" << done->status << ":" << done->value << '\n';
    }
}
