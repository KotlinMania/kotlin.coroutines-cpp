#pragma once
// port-lint: source kotlinx-coroutines-core/common/src/Builders.common.kt
/** Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt */
#include "kotlinx/coroutines/internal/ScopeCoroutine.hpp"
#include <atomic>

namespace kotlinx::coroutines {

// Used by with_context when the dispatcher changes.
// Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:219-267
template <typename T>
class DispatchedCoroutine : public internal::ScopeCoroutine<T> {
    static constexpr int UNDECIDED = 0;
    static constexpr int SUSPENDED = 1;
    static constexpr int RESUMED = 2;
    std::atomic<int> decision_{UNDECIDED};
public:
    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:219-222
    DispatchedCoroutine(std::shared_ptr<CoroutineContext> context, std::shared_ptr<Continuation<T>> continuation)
        : internal::ScopeCoroutine<T>(std::move(context), std::move(continuation)) {}

    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:259-266
    void* get_result() {
        if (try_suspend()) return intrinsics::get_COROUTINE_SUSPENDED();
        auto* state = this->get_state_for_await();
        if (auto* failure = dynamic_cast<CompletedExceptionally*>(state)) std::rethrow_exception(failure->cause);
        auto value = recover_result<T>(state, this->u_cont.get()).get_or_throw();
        // NOTE(port): The receiving caller owns and deletes non-Unit value boxes.
        if constexpr (std::is_same_v<T, void*>) return value;
        else if constexpr (std::is_same_v<T, Unit>) return nullptr;
        else return new T(std::move(value));
    }

protected:
    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:247-251
    void after_completion(JobState* state) override { after_resume(state); }

    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:253-257
    void after_resume(JobState* state) override {
        auto owner = this->JobSupport::shared_from_this();
        this->erased_completion_.reset();
        if (try_resume()) return; // Completed before get_result invocation.
        // Resume cancellably because we have to switch back to the original dispatcher.
        kotlinx::coroutines::resume_cancellable_with(this->intercepted_u_cont(), recover_result<T>(state, this->u_cont.get()));
    }

private:
    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:227-235
    bool try_suspend() {
        while (true) {
            int decision = decision_.load();
            switch (decision) {
                case UNDECIDED:
                    if (decision_.compare_exchange_strong(decision, SUSPENDED)) return true;
                    break;
                case RESUMED: return false;
                default: throw std::logic_error("Already suspended");
            }
        }
    }

    // Transliterated from: kotlinx-coroutines-core/common/src/Builders.common.kt:237-245
    bool try_resume() {
        while (true) {
            int decision = decision_.load();
            switch (decision) {
                case UNDECIDED:
                    if (decision_.compare_exchange_strong(decision, RESUMED)) return true;
                    break;
                case SUSPENDED: return false;
                default: throw std::logic_error("Already resumed");
            }
        }
    }
};

} // namespace kotlinx::coroutines
