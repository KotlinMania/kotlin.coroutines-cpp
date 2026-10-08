#pragma once
// port-lint: source libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt
/** Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt */
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include <memory>
#include <type_traits>
#include <utility>

namespace kotlin::coroutines {

/**
 * AbstractCoroutineContextElement - convenience base class for context elements.
 *
 * This class provides a simple implementation for elements that store their key.
 * Most concrete context elements will inherit from this class.
 *
 * Usage example:
 * ```cpp
 * class MyElement : public AbstractCoroutineContextElement {
 * public:
 *     static KeyTyped<MyElement> KEY;
 *     MyElement() : AbstractCoroutineContextElement(&KEY) {}
 * };
 * ```
 */
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:17-17
class AbstractCoroutineContextElement : public virtual CoroutineContext::Element {
public:
    Key* key_;

    /**
     * Constructor with key.
     *
     * @param key The key that identifies this element type
     */
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:17-17
    explicit AbstractCoroutineContextElement(Key* key) : key_(key) {}

    /**
     * Returns the key of this element.
     *
     * @return The key provided during construction
     */
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:17-17
    Key* key() const override { return key_; }
};

namespace internal {
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:56-65
// NOTE(port): One erased base represents Kotlin's AbstractCoroutineContextKey<*, *> test
// across C++ template instantiations. Keys remain borrowed, as in existing context APIs.
class AbstractCoroutineContextKeyBase {
public:
    virtual ~AbstractCoroutineContextKeyBase() = default;
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:63-63
    std::shared_ptr<CoroutineContext::Element> try_cast(
        std::shared_ptr<CoroutineContext::Element> element) const;
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:64-64
    bool is_sub_key(const CoroutineContext::Key* key) const;
protected:
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:56-61
    AbstractCoroutineContextKeyBase(CoroutineContext::Key* base_key,
        std::function<std::shared_ptr<CoroutineContext::Element>(
            std::shared_ptr<CoroutineContext::Element>)> safe_cast);
private:
    CoroutineContext::Key* const topmost_key_;
    const std::function<std::shared_ptr<CoroutineContext::Element>(
        std::shared_ptr<CoroutineContext::Element>)> safe_cast_;
};
} // namespace internal

/** Base class for keys associated with polymorphic context elements. */
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:56-65
// NOTE(port): The public generic binding stays in the header; erased algorithms live in .cpp.
template <typename B, typename E>
class AbstractCoroutineContextKey : public CoroutineContext::KeyTyped<E>,
                                    public internal::AbstractCoroutineContextKeyBase {
protected:
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:56-61
    AbstractCoroutineContextKey(CoroutineContext::KeyTyped<B>* base_key,
        std::function<std::shared_ptr<E>(std::shared_ptr<CoroutineContext::Element>)> safe_cast)
        : internal::AbstractCoroutineContextKeyBase(base_key,
            [safe_cast = std::move(safe_cast)](std::shared_ptr<CoroutineContext::Element> element) {
                return safe_cast(std::move(element));
            }) {
        static_assert(std::is_base_of_v<CoroutineContext::Element, B> && std::is_base_of_v<B, E>);
    }
public:
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:63-63
    std::shared_ptr<E> try_cast(std::shared_ptr<CoroutineContext::Element> element) const {
        return std::dynamic_pointer_cast<E>(internal::AbstractCoroutineContextKeyBase::try_cast(std::move(element)));
    }
};

/** Returns this element when its key is associated with the requested key. */
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:81-89
// NOTE(port): This overload supplies the erased context ABI; the template restores E.
std::shared_ptr<CoroutineContext::Element> get_polymorphic_element(
    std::shared_ptr<CoroutineContext::Element> element, CoroutineContext::Key* key);

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:81-89
template <typename E>
std::shared_ptr<E> get_polymorphic_element(std::shared_ptr<CoroutineContext::Element> element,
                                          CoroutineContext::KeyTyped<E>* key) {
    return std::dynamic_pointer_cast<E>(get_polymorphic_element(
        std::move(element), static_cast<CoroutineContext::Key*>(key)));
}

/** Returns the empty context when this element is associated with the requested key. */
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:106-112
std::shared_ptr<CoroutineContext> minus_polymorphic_key(
    std::shared_ptr<CoroutineContext::Element> element, CoroutineContext::Key* key);

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

} // namespace kotlin::coroutines

#include "kotlinx/coroutines/ContextImports.hpp"
