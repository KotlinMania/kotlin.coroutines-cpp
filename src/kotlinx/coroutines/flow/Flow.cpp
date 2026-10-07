// port-lint: source kotlinx-coroutines-core/common/src/flow/Flow.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/Flow.kt
 */
#include "kotlinx/coroutines/flow/Flow.hpp"
#include "kotlinx/coroutines/ContinuationImpl.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"
#include <utility>

namespace kotlinx::coroutines::flow::internal {
namespace {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Flow.kt:223-230
// NOTE(port): This is the lowered frame for the source try/finally. The typed
// bindings retain the actual flow and SafeCollector, without adopting borrowers.
class CollectFrame final : public ContinuationImpl {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Flow.kt:223-230
    CollectFrame(std::function<void*(Continuation<void*>*)> collect_safely,
                 std::function<void()> release_intercepted,
                 Continuation<void*>* completion)
        : ContinuationImpl(kotlinx::coroutines::internal::retain_continuation(completion)),
          collect_safely_(std::move(collect_safely)),
          release_collector_(std::move(release_intercepted)) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Flow.kt:223-230
    // NOTE(port): Kotlin's suspended frame is a GC root. Keep the corresponding
    // C++ frame alive until termination, including exceptional resumption.
    void retain() { self_ref_ = shared_from_this(); }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Flow.kt:223-230
    void* invoke_suspend(Result<void*> result) override {
        try {
            coroutine_begin(this)
            coroutine_yield(this, std::function(collect_safely_)(this));
        } catch (...) {
            release_collector_();
            throw;
        }
        release_collector_();
        coroutine_end(this)
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/Flow.kt:223-230
    // NOTE(port): Terminated spills no longer own the flow or SafeCollector,
    // even when a dispatcher or application still owns this completed frame.
    void release_intercepted() override {
        auto self = std::move(self_ref_);
        collect_safely_ = {};
        release_collector_ = {};
        ContinuationImpl::release_intercepted();
    }

private:
    void* _label = nullptr;
    std::function<void*(Continuation<void*>*)> collect_safely_;
    std::function<void()> release_collector_;
    std::shared_ptr<BaseContinuationImpl> self_ref_;
};

} // namespace

// Transliterated from: kotlinx-coroutines-core/common/src/flow/Flow.kt:223-230
void* collect_abstract_flow(
    std::function<void*(Continuation<void*>*)> collect_safely,
    std::function<void()> release_intercepted,
    Continuation<void*>* completion) {
    auto frame = std::make_shared<CollectFrame>(
        std::move(collect_safely), std::move(release_intercepted), completion);
    frame->retain();
    return frame->start(Result<void*>::success(nullptr));
}

} // namespace kotlinx::coroutines::flow::internal
