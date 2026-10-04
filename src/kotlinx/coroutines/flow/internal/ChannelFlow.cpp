/**
 * Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt
 * and kotlinx-coroutines-core/common/src/CoroutineScope.kt
 */
#include "kotlinx/coroutines/flow/internal/ChannelFlow.hpp"
#include "kotlinx/coroutines/internal/ScopeCoroutine.hpp"
#include <atomic>

namespace kotlinx::coroutines::flow::internal {
namespace {

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/coroutines/SafeContinuationNative.kt:17-63
class ScopeCompletion final : public Continuation<void*> {
public:
    explicit ScopeCompletion(Continuation<void*>* delegate) : delegate_(delegate) {}

    ~ScopeCompletion() override {
        void* value = result_.load();
        if (value != &UNDECIDED && value != &RESUMED &&
            !intrinsics::is_coroutine_suspended(value)) {
            delete static_cast<Result<void*>*>(value);
        }
    }

    std::shared_ptr<CoroutineContext> get_context() const override {
        return delegate_ ? delegate_->get_context() : EmptyCoroutineContext::instance();
    }

    void resume_with(Result<void*> result) override {
        auto boxed = std::make_unique<Result<void*>>(result);
        while (true) {
            void* current = result_.load();
            if (current == &UNDECIDED) {
                if (result_.compare_exchange_strong(current, boxed.get())) {
                    boxed.release();
                    return;
                }
            } else if (intrinsics::is_coroutine_suspended(current)) {
                if (result_.compare_exchange_strong(current, &RESUMED)) {
                    if (delegate_) delegate_->resume_with(std::move(result));
                    return;
                }
            } else {
                throw std::logic_error("Already resumed");
            }
        }
    }

    void* get_or_throw() {
        void* result = result_.load();
        if (result == &UNDECIDED) {
            if (result_.compare_exchange_strong(result, intrinsics::get_COROUTINE_SUSPENDED())) {
                return intrinsics::get_COROUTINE_SUSPENDED();
            }
        }
        if (result == &RESUMED || intrinsics::is_coroutine_suspended(result)) {
            return intrinsics::get_COROUTINE_SUSPENDED();
        }
        return static_cast<Result<void*>*>(result)->get_or_throw();
    }

private:
    static inline int UNDECIDED = 0;
    static inline int RESUMED = 0;
    Continuation<void*>* delegate_;
    std::atomic<void*> result_{&UNDECIDED};
};

} // namespace

// Transliterated from: kotlinx-coroutines-core/common/src/flow/internal/ChannelFlow.kt:118-121
// and kotlinx-coroutines-core/common/src/CoroutineScope.kt:280-288
void* collect_in_scope(
    std::function<void*(CoroutineScope*, std::shared_ptr<Continuation<void*>>)> block,
    Continuation<void*>* completion) {
    auto safe_completion = std::make_shared<ScopeCompletion>(completion);
    auto scope = std::make_shared<kotlinx::coroutines::internal::ScopeCoroutine<void*>>(
        safe_completion->get_context(), safe_completion);
    scope->start(CoroutineStart::UNDISPATCHED, static_cast<CoroutineScope*>(scope.get()), std::move(block));
    if (!completion && !scope->is_completed()) {
        // NOTE(port): Retain the legacy blocking entry point when no continuation is supplied.
        scope->join_blocking();
    }
    return safe_completion->get_or_throw();
}

} // namespace kotlinx::coroutines::flow::internal
