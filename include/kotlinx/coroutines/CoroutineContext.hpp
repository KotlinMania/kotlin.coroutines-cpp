#pragma once
#include <memory>
#include <functional>
#include <string>
#include "core_fwd.hpp"

namespace kotlinx {
namespace coroutines {

class CoroutineContext {
public:
    virtual ~CoroutineContext() = default;

    struct Element;

    class Key {
    public:
        // Optional name for debugging
        const char* name = nullptr;
        Key(const char* name = nullptr) : name(name) {}
        virtual ~Key() = default;
    };

    template <typename E>
    class KeyTyped : public Key {
    public:
        KeyTyped(const char* name = nullptr) : Key(name) {}
    };

    virtual std::shared_ptr<Element> get(Key* key) const { return nullptr; }
    
    // Operator + stub
    std::shared_ptr<CoroutineContext> operator+(std::shared_ptr<CoroutineContext> other) const {
        // dummy
        return nullptr; // In reality, return CombinedContext
    }
    
    std::shared_ptr<CoroutineContext> operator+(CoroutineContext* other) const {
        return nullptr;
    }
    
    // For T* + T* syntax in C++, we usually need free functions, but usage is likely internal or specific
};

struct CoroutineContext::Element : public CoroutineContext {
    virtual Key* key() const = 0;
    
    std::shared_ptr<Element> get(Key* k) const override {
        if (this->key() == k) {
            // return shared_from_this(); // needs enable_shared_from_this
            return nullptr; 
        }
        return nullptr;
    }
};

// AbstractCoroutineContextElement implementation
class AbstractCoroutineContextElement : public virtual CoroutineContext::Element {
public:
    Key* key_;
    AbstractCoroutineContextElement(Key* key) : key_(key) {}

    Key* key() const override { return key_; }
};

} // namespace coroutines
} // namespace kotlinx
