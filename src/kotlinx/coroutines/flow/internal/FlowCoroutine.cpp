/** Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowCoroutine.kt */
#include "kotlinx/coroutines/flow/internal/FlowCoroutine.hpp"
#include "kotlinx/coroutines/internal/ScopeCoroutine.hpp"

namespace kotlinx::coroutines::flow::internal {
namespace {

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowCoroutine.kt:52-61
class FlowCoroutine final : public kotlinx::coroutines::internal::ScopeCoroutine<void*> {
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowCoroutine.kt:52-55
    FlowCoroutine(std::shared_ptr<CoroutineContext> context, std::shared_ptr<Continuation<void*>> completion)
        : ScopeCoroutine<void*>(std::move(context), std::move(completion)) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowCoroutine.kt:56-59
    bool child_cancelled(std::exception_ptr cause) override {
        try { std::rethrow_exception(cause); }
        catch (const ChildCancelledException&) { return true; }
        catch (...) { return cancel_impl(cause); }
    }

    // NOTE(port): Owning storage replaces Kotlin GC reachability during suspension.
    void retain() { self_ref_ = std::dynamic_pointer_cast<FlowCoroutine>(JobSupport::shared_from_this()); }
    void release() { self_ref_.reset(); }

    // Transliterated from: kotlinx-coroutines-core/common/src/internal/Scopes.kt:21-24
    void after_completion(JobState* state) override {
        auto lifetime = std::move(self_ref_);
        ScopeCoroutine<void*>::after_completion(state);
    }
    // Transliterated from: kotlinx-coroutines-core/common/src/internal/Scopes.kt:33-36
    void after_resume(JobState* state) override {
        auto lifetime = std::move(self_ref_);
        ScopeCoroutine<void*>::after_resume(state);
    }
private:
    std::shared_ptr<FlowCoroutine> self_ref_;
};

} // namespace

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/FlowCoroutine.kt:26-30
void* flow_scope(std::function<void*(CoroutineScope*, Continuation<void*>*)> block,
                 Continuation<void*>* completion) {
    auto u_cont = kotlinx::coroutines::internal::retain_continuation(completion);
    auto coroutine = std::make_shared<FlowCoroutine>(u_cont->get_context(), u_cont);
    coroutine->retain();
    try {
        void* result = coroutine->start_undispatched_or_return(
            [coroutine, block = std::move(block)](Continuation<void*>* continuation) {
                return block(coroutine.get(), continuation);
            });
        if (!intrinsics::is_coroutine_suspended(result)) coroutine->release();
        return result;
    } catch (...) {
        coroutine->release();
        throw;
    }
}

} // namespace kotlinx::coroutines::flow::internal
