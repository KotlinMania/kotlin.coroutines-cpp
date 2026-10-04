#pragma once
/** Transliterated from: libraries/stdlib/src/kotlin/coroutines/ContinuationInterceptor.kt */
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Continuation.hpp"

namespace kotlinx {
namespace coroutines {

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/ContinuationInterceptor.kt:8-72
struct ContinuationInterceptor : public virtual CoroutineContext::Element {
    static constexpr const char* key_str = "ContinuationInterceptor";
    inline static CoroutineContext::KeyTyped<ContinuationInterceptor> key_instance{key_str};
    static constexpr CoroutineContext::Key* type_key = &key_instance;

    ContinuationInterceptor() = default;
    
    virtual CoroutineContext::Key* key() const override { return type_key; }
    
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/ContinuationInterceptor.kt:26-37
    virtual std::shared_ptr<Continuation<void*>> intercept_continuation(
        std::shared_ptr<Continuation<void*>> continuation) = 0;

    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/ContinuationInterceptor.kt:39-50
    virtual void release_intercepted_continuation(
        std::shared_ptr<Continuation<void*>> continuation);
};

} // namespace coroutines
} // namespace kotlinx
