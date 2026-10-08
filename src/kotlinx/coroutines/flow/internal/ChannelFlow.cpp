// port-lint: source kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt
/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt
 * and kotlinx-coroutines-core/common/src/CoroutineScope.kt
 */
#include "kotlinx/coroutines/flow/internal/ChannelFlow.hpp"
#include "kotlinx/coroutines/internal/ScopeCoroutine.hpp"
#include "kotlinx/coroutines/internal/CoroutineStackFrame.hpp"
#include "kotlinx/coroutines/common/CoroutineContextUtils.hpp"
#include "kotlinx/coroutines/dsl/Suspend.hpp"

namespace kotlinx::coroutines::flow::internal {
namespace {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:228-240
class StackFrameContinuation final : public Continuation<void*>,
                                     public kotlinx::coroutines::internal::CoroutineStackFrame,
                                     public std::enable_shared_from_this<StackFrameContinuation> {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:228-230
    StackFrameContinuation(Continuation<void*>* continuation, std::shared_ptr<CoroutineContext> context)
        : continuation_(kotlinx::coroutines::internal::retain_continuation(continuation)),
          context_(std::move(context)) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:229-229
    std::shared_ptr<CoroutineContext> get_context() const override { return context_; }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:232-233
    kotlinx::coroutines::internal::CoroutineStackFrame* get_caller_frame() const override {
        return dynamic_cast<kotlinx::coroutines::internal::CoroutineStackFrame*>(continuation_.get());
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:235-237
    void resume_with(Result<void*> result) override {
        auto lifetime = std::move(self_ref_);
        continuation_->resume_with(std::move(result));
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:239-239
    kotlinx::coroutines::internal::StackTraceElement* get_stack_trace_element() const override { return nullptr; }

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:215-240
    // NOTE(port): Kotlin GC keeps the continuation alive while its callee is suspended.
    void retain() { self_ref_ = shared_from_this(); }
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:215-240
    void release() { self_ref_.reset(); }

private:
    std::shared_ptr<Continuation<void*>> continuation_;
    std::shared_ptr<CoroutineContext> context_;
    std::shared_ptr<StackFrameContinuation> self_ref_;
};

} // namespace

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:215-225
void* call_with_context_undispatched(
    std::shared_ptr<CoroutineContext> new_context, void* count_or_element,
    std::function<void*(Continuation<void*>*)> block, Continuation<void*>* completion) {
    return with_coroutine_context<void*>(new_context, count_or_element, [&]() -> void* {
        auto frame = std::make_shared<StackFrameContinuation>(completion, new_context);
        frame->retain();
        try {
            void* result = block(frame.get());
            if (!intrinsics::is_coroutine_suspended(result)) frame->release();
            return result;
        } catch (...) {
            frame->release();
            throw;
        }
    });
}

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:118-121
// Transliterated from: kotlinx-coroutines-core/common/src/CoroutineScope.kt:279-288
void* collect_in_scope(
    std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block,
    Continuation<void*>* completion) {
    auto caller = kotlinx::coroutines::internal::retain_continuation(completion);
    auto scope = std::make_shared<kotlinx::coroutines::internal::ScopeCoroutine<void*>>(
        caller->get_context(), caller);
    return scope->start_undispatched_or_return(
        [scope, block = std::move(block)](std::shared_ptr<Continuation<void*>> continuation) {
            return block(static_cast<CoroutineScope*>(scope.get()), std::move(continuation));
        });
}

} // namespace kotlinx::coroutines::flow::internal
