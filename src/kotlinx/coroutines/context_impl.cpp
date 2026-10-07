// port-lint: source libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt
/**
 * Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContext.kt
 * Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt
 */
#include "kotlinx/coroutines/context_impl.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"
#include <bit>

namespace kotlinx::coroutines {
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:56-61
internal::AbstractCoroutineContextKeyBase::AbstractCoroutineContextKeyBase(
    CoroutineContext::Key* base_key,
    std::function<std::shared_ptr<CoroutineContext::Element>(
        std::shared_ptr<CoroutineContext::Element>)> safe_cast)
    : topmost_key_(dynamic_cast<AbstractCoroutineContextKeyBase*>(base_key)
          ? dynamic_cast<AbstractCoroutineContextKeyBase*>(base_key)->topmost_key_ : base_key),
      safe_cast_(std::move(safe_cast)) {}

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:63-63
std::shared_ptr<CoroutineContext::Element> internal::AbstractCoroutineContextKeyBase::try_cast(
    std::shared_ptr<CoroutineContext::Element> element) const {
    return safe_cast_(std::move(element));
}

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:64-64
bool internal::AbstractCoroutineContextKeyBase::is_sub_key(const CoroutineContext::Key* key) const {
    return key == dynamic_cast<const CoroutineContext::Key*>(this) || topmost_key_ == key;
}

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:81-89
std::shared_ptr<CoroutineContext::Element> get_polymorphic_element(
    std::shared_ptr<CoroutineContext::Element> element, CoroutineContext::Key* key) {
    if (auto* polymorphic = dynamic_cast<internal::AbstractCoroutineContextKeyBase*>(key)) {
        return polymorphic->is_sub_key(element->key()) ? polymorphic->try_cast(element) : nullptr;
    }
    return element->key() == key ? element : nullptr;
}

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:106-112
std::shared_ptr<CoroutineContext> minus_polymorphic_key(
    std::shared_ptr<CoroutineContext::Element> element, CoroutineContext::Key* key) {
    if (auto* polymorphic = dynamic_cast<internal::AbstractCoroutineContextKeyBase*>(key)) {
        return polymorphic->is_sub_key(element->key()) && polymorphic->try_cast(element)
            ? std::static_pointer_cast<CoroutineContext>(EmptyCoroutineContext::instance()) : element;
    }
    return element->key() == key
        ? std::static_pointer_cast<CoroutineContext>(EmptyCoroutineContext::instance()) : element;
}

// Transliterated from: kotlin-native/runtime/src/main/kotlin/kotlin/Any.kt:41-41
// Transliterated from: kotlin-native/runtime/src/main/cpp/Natives.cpp:40-49
std::int32_t CoroutineContext::hash_code() const {
    // NOTE(port): Inline the actual Native identity primitive for ordinary C++ object storage.
    return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(this)));
}

namespace {

// This class is a left-biased list, so plus works naturally.
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:135-199
class CombinedContext final : public CoroutineContext {
    std::shared_ptr<CoroutineContext> left_;
    std::shared_ptr<Element> element_;
public:
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:135-138
    CombinedContext(std::shared_ptr<CoroutineContext> left, std::shared_ptr<Element> element)
        : left_(std::move(left)), element_(std::move(element)) {}

    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:140-151
    std::shared_ptr<Element> get(Key* key) const override {
        auto* current = this;
        while (true) {
            if (auto element = current->element_->get(key)) return element;
            auto next = current->left_;
            if (auto* combined = dynamic_cast<const CombinedContext*>(next.get())) current = combined;
            else return next->get(key);
        }
    }

    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:153-154
    // NOTE(port): Erased for_each supplies Kotlin's public generic fold in CoroutineContext.hpp.
    void for_each(std::function<void(std::shared_ptr<Element>)> operation) const override {
        left_->for_each(operation);
        operation(element_);
    }

    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:156-164
    std::shared_ptr<CoroutineContext> minus_key(Key* key) const override {
        if (element_->get(key)) return left_;
        auto new_left = left_->minus_key(key);
        if (new_left == left_) return std::const_pointer_cast<CoroutineContext>(shared_from_this());
        if (new_left == EmptyCoroutineContext::instance()) return element_;
        return std::make_shared<CombinedContext>(std::move(new_left), element_);
    }

    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:191-192
    bool equals(const CoroutineContext* other) const override {
        if (this == other) return true;
        auto* combined = dynamic_cast<const CombinedContext*>(other);
        return combined && combined->size() == size() && combined->contains_all(*this);
    }

    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:194-194
    std::int32_t hash_code() const override {
        // NOTE(port): Unsigned arithmetic preserves Kotlin Int wraparound without C++ signed overflow.
        return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(left_->hash_code()) +
                                          static_cast<std::uint32_t>(element_->hash_code()));
    }

    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:196-199
    std::string to_string() const override {
        return "[" + fold<std::string>("", [](std::string accumulator, std::shared_ptr<Element> element) {
            return accumulator.empty() ? element->to_string() : accumulator + ", " + element->to_string();
        }) + "]";
    }

private:
    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:166-173
    int size() const {
        auto* current = this;
        int size = 2;
        while (true) {
            auto* left = dynamic_cast<const CombinedContext*>(current->left_.get());
            if (!left) return size;
            current = left;
            ++size;
        }
    }

    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:175-176
    bool contains(const std::shared_ptr<Element>& element) const {
        auto found = get(element->key());
        return found && found->equals(element.get());
    }

    // Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:178-189
    bool contains_all(const CombinedContext& context) const {
        auto* current = &context;
        while (true) {
            if (!contains(current->element_)) return false;
            auto next = current->left_;
            if (auto* combined = dynamic_cast<const CombinedContext*>(next.get())) current = combined;
            else return contains(std::dynamic_pointer_cast<Element>(next));
        }
    }
};

} // namespace

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContext.kt:30-43
std::shared_ptr<CoroutineContext> CoroutineContext::operator+(
    std::shared_ptr<CoroutineContext> other) const {
    if (other == EmptyCoroutineContext::instance())
        return std::const_pointer_cast<CoroutineContext>(shared_from_this());

    return other->fold<std::shared_ptr<CoroutineContext>>(
        std::const_pointer_cast<CoroutineContext>(shared_from_this()),
        [](std::shared_ptr<CoroutineContext> acc, std::shared_ptr<Element> element) {
            auto removed = acc->minus_key(element->key());
            if (removed == EmptyCoroutineContext::instance()) {
                return std::static_pointer_cast<CoroutineContext>(element);
            }
            // Make sure the interceptor is always last in the context, so it is fast to get.
            auto interceptor = removed->get(ContinuationInterceptor::type_key);
            if (!interceptor) return std::static_pointer_cast<CoroutineContext>(
                std::make_shared<CombinedContext>(removed, element));
            auto left = removed->minus_key(ContinuationInterceptor::type_key);
            if (left == EmptyCoroutineContext::instance())
                return std::static_pointer_cast<CoroutineContext>(std::make_shared<CombinedContext>(element, interceptor));
            return std::static_pointer_cast<CoroutineContext>(std::make_shared<CombinedContext>(
                std::make_shared<CombinedContext>(left, element), interceptor));
        });
}


// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContext.kt:72-73
std::shared_ptr<CoroutineContext> CoroutineContext::Element::minus_key(Key* key) const {
    if (this->key() == key) return EmptyCoroutineContext::instance();
    return std::const_pointer_cast<CoroutineContext>(shared_from_this());
}

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:117-119
std::shared_ptr<EmptyCoroutineContext> EmptyCoroutineContext::instance() {
    static auto context = std::make_shared<EmptyCoroutineContext>();
    return context;
}

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:122-122
std::shared_ptr<CoroutineContext::Element> EmptyCoroutineContext::get(Key* key) const { return nullptr; }

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:123-123
void EmptyCoroutineContext::for_each(std::function<void(std::shared_ptr<Element>)> operation) const {}

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:124-124
std::shared_ptr<CoroutineContext> EmptyCoroutineContext::operator+(std::shared_ptr<CoroutineContext> context) const {
    return context;
}

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:125-125
std::shared_ptr<CoroutineContext> EmptyCoroutineContext::minus_key(Key* key) const { return instance(); }

// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:127-127
std::string EmptyCoroutineContext::to_string() const { return "EmptyCoroutineContext"; }

} // namespace kotlinx::coroutines

namespace kotlinx::coroutines {
// Transliterated from: libraries/stdlib/src/kotlin/coroutines/CoroutineContextImpl.kt:126-126
std::int32_t EmptyCoroutineContext::hash_code() const { return 0; }
} // namespace kotlinx::coroutines
