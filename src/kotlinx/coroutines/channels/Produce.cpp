// port-lint: source channels/Produce.kt
/** Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt */
#include "kotlinx/coroutines/channels/Produce.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"

namespace kotlinx::coroutines::channels::internal {
namespace {
// Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:60-71
// NOTE(port): The existing Native continuation and injected suspension address
// preserve Kotlin's try/finally across the erased ABI return.
class AwaitCloseContinuation final : public ContinuationImpl {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:60-71
    AwaitCloseContinuation(
        std::function<void(std::function<void(std::exception_ptr)>)> register_close,
        std::function<void()> block, std::shared_ptr<Continuation<void*>> completion)
        : ContinuationImpl(std::move(completion)),
          register_close_(std::move(register_close)), block_(std::move(block)) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:62-71
    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)
            coroutine_yield(this, suspend_cancellable_coroutine<void>(
                [this](CancellableContinuation<void>& cont) {
                    auto retained = dynamic_cast<CancellableContinuationImpl<void>&>(cont).shared_from_this();
                    register_close_([retained = std::move(retained)](std::exception_ptr) mutable {
                        // NOTE(port): A channel invokes its close handler once.
                        // Release the consumed reference from its stored handler.
                        auto continuation = std::move(retained);
                        continuation->resume();
                    });
                }, this));
        } catch (...) {
            block_();
            throw;
        }
        block_();
        coroutine_end(this)
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:62-71
    // NOTE(port): Terminated spills release actual C++ captures and receiver owners.
    void release_intercepted() override {
        register_close_ = {};
        block_ = {};
        ContinuationImpl::release_intercepted();
    }
private:
    void* _label = nullptr;
    std::function<void(std::function<void(std::exception_ptr)>)> register_close_;
    std::function<void()> block_;
};
}

// Transliterated from: kotlinx-coroutines-core/common/src/channels/Produce.kt:60-71
void* await_close_erased(Job* receiver,
    std::function<void(std::function<void(std::exception_ptr)>)> register_close,
    std::function<void()> block, std::shared_ptr<Continuation<void*>> continuation) {
    auto job = std::dynamic_pointer_cast<Job>(continuation->get_context()->get(Job::type_key));
    if (!receiver || job.get() != receiver) {
        throw std::logic_error("awaitClose() can only be invoked from the producer context");
    }
    auto frame = std::make_shared<AwaitCloseContinuation>(
        std::move(register_close), std::move(block), std::move(continuation));
    return frame->start(Result<void*>::success(nullptr));
}
} // namespace kotlinx::coroutines::channels::internal
