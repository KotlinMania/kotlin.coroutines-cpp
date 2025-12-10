#pragma once
#include <memory>
#include <functional>
#include <string>

namespace kotlinx {
namespace coroutines {

class CoroutineContext {
public:
    virtual ~CoroutineContext() = default;

    struct Element;

    class Key {
    public:
        const char* name = nullptr;
        explicit Key(const char* name = nullptr) : name(name) {}
        virtual ~Key() = default;
    };

    template <typename E>
    class KeyTyped : public Key {
    public:
        explicit KeyTyped(const char* name = nullptr) : Key(name) {}
    };

    virtual std::shared_ptr<Element> get(Key* key) const { return nullptr; }
    
    // Operator + for context composition
    // Typically implemented via fold or CombinedContext
    std::shared_ptr<CoroutineContext> operator+(std::shared_ptr<CoroutineContext> other) const;
    
    template <typename R>
    R fold(R initial, std::function<R(R, std::shared_ptr<Element>)> operation) const;

    std::shared_ptr<CoroutineContext> minusKey(Key* key) const;
};

struct CoroutineContext::Element : public CoroutineContext {
    virtual Key* key() const = 0;
    
    std::shared_ptr<Element> get(Key* k) const override {
        if (this->key() == k) {
             // We need shared_from_this() capability here, typically Element inherits enable_shared_from_this
             // For now, strict interface definition
             return nullptr; 
        }
        return nullptr;
    }
};

class AbstractCoroutineContextElement : public virtual CoroutineContext::Element {
public:
    Key* key_;
    explicit AbstractCoroutineContextElement(Key* key) : key_(key) {}

    Key* key() const override { return key_; }
};

} // namespace coroutines
} // namespace kotlinx
