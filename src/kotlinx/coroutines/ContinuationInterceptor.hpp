#pragma once
// port-lint: source libraries/stdlib/src/kotlin/coroutines/ContinuationInterceptor.kt
/** Transliterated from: libraries/stdlib/src/kotlin/coroutines/ContinuationInterceptor.kt */
#include "kotlinx/coroutines/CoroutineImports.hpp"
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Continuation.hpp"

namespace kotlin {
namespace coroutines {

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/ContinuationInterceptor.kt:20-72
struct ContinuationInterceptor : public virtual CoroutineContext::Element {
    static constexpr const char* key_str = "ContinuationInterceptor";
    inline static CoroutineContext::KeyTyped<ContinuationInterceptor> key_instance{key_str};
    static constexpr CoroutineContext::Key* type_key = &key_instance;

    ContinuationInterceptor() = default;
    
    virtual CoroutineContext::Key* key() const override { return type_key; }
    
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/ContinuationInterceptor.kt:37-37
    virtual std::shared_ptr<Continuation<void*>> intercept_continuation(
        std::shared_ptr<Continuation<void*>> continuation) = 0;

    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/ContinuationInterceptor.kt:48-50
    virtual void release_intercepted_continuation(
        std::shared_ptr<Continuation<void*>> continuation);

    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/ContinuationInterceptor.kt:52-61
    std::shared_ptr<Element> get(CoroutineContext::Key* key) const override;
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/ContinuationInterceptor.kt:64-71
    std::shared_ptr<CoroutineContext> minus_key(CoroutineContext::Key* key) const override;

};

} // namespace coroutines
} // namespace kotlin
