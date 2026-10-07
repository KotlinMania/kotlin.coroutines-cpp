#pragma once
// port-lint: source libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt
/** Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt */
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include <memory>

namespace kotlinx::coroutines {

/** An empty coroutine context. */
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:116-128
class EmptyCoroutineContext final : public CoroutineContext {
public:
    // NOTE(port): A shared singleton represents the Kotlin object in standalone C++.
    static std::shared_ptr<EmptyCoroutineContext> instance();
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:122-122
    std::shared_ptr<Element> get(Key* key) const override;
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:123-123
    void for_each(std::function<void(std::shared_ptr<Element>)> callback) const override;
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:124-124
    std::shared_ptr<CoroutineContext> operator+(std::shared_ptr<CoroutineContext> context) const override;
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:125-125
    std::shared_ptr<CoroutineContext> minus_key(Key* key) const override;
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:127-127
    std::string to_string() const override;
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:126-126
    std::int32_t hash_code() const override;

};

} // namespace kotlinx::coroutines
