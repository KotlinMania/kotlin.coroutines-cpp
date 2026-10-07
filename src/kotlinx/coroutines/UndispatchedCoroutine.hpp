#pragma once
// port-lint: source kotlinx-coroutines-core/native/src/CoroutineContext.kt
/** Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt */
#include "kotlinx/coroutines/internal/ScopeCoroutine.hpp"

namespace kotlinx::coroutines {

// Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:48-53
// NOTE(port): The template retains the value type required by the public with_context API.
template <typename T>
class UndispatchedCoroutine : public internal::ScopeCoroutine<T> {
public:
    // Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:48-51
    UndispatchedCoroutine(std::shared_ptr<CoroutineContext> context, std::shared_ptr<Continuation<T>> continuation)
        : internal::ScopeCoroutine<T>(std::move(context), std::move(continuation)) {}

protected:
    // Transliterated from: kotlinx-coroutines-core/native/src/CoroutineContext.kt:52-52
    void after_resume(JobState* state) override {
        // NOTE(port): Keep the completing receiver alive while its erased adapter releases ownership.
        auto owner = this->JobSupport::shared_from_this();
        this->erased_completion_.reset();
        this->u_cont->resume_with(recover_result<T>(state, this->u_cont.get()));
    }
};

} // namespace kotlinx::coroutines
