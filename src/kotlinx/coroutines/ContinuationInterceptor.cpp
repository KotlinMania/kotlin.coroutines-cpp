// port-lint: source libraries/stdlib/src/kotlin/coroutines/ContinuationInterceptor.kt
/**
 * Transliterated from: libraries/stdlib/src/kotlin/coroutines/ContinuationInterceptor.kt
 */
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"
#include "kotlinx/coroutines/context_impl.hpp"

namespace kotlin::coroutines {
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/ContinuationInterceptor.kt:48-50
void ContinuationInterceptor::release_intercepted_continuation(
    std::shared_ptr<Continuation<void*>> continuation) {}

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/ContinuationInterceptor.kt:52-61
std::shared_ptr<CoroutineContext::Element> ContinuationInterceptor::get(CoroutineContext::Key* key) const {
    // getPolymorphicKey specialized for ContinuationInterceptor key.
    if (auto* polymorphic = dynamic_cast<internal::AbstractCoroutineContextKeyBase*>(key)) {
        if (!polymorphic->is_sub_key(this->key())) return nullptr;
        return polymorphic->try_cast(std::dynamic_pointer_cast<Element>(
            std::const_pointer_cast<CoroutineContext>(shared_from_this())));
    }
    return type_key == key ? std::dynamic_pointer_cast<Element>(
        std::const_pointer_cast<CoroutineContext>(shared_from_this())) : nullptr;
}

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/ContinuationInterceptor.kt:64-71
std::shared_ptr<CoroutineContext> ContinuationInterceptor::minus_key(CoroutineContext::Key* key) const {
    auto element = std::dynamic_pointer_cast<Element>(
        std::const_pointer_cast<CoroutineContext>(shared_from_this()));
    // minusPolymorphicKey specialized for ContinuationInterceptor key.
    if (auto* polymorphic = dynamic_cast<internal::AbstractCoroutineContextKeyBase*>(key)) {
        return polymorphic->is_sub_key(this->key()) && polymorphic->try_cast(element)
            ? std::static_pointer_cast<CoroutineContext>(EmptyCoroutineContext::instance()) : element;
    }
    return type_key == key
        ? std::static_pointer_cast<CoroutineContext>(EmptyCoroutineContext::instance()) : element;
}
} // namespace kotlin::coroutines
