#pragma once
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Runnable.hpp"
#include "kotlinx/coroutines/Continuation.hpp"
#include "kotlinx/coroutines/ContinuationInterceptor.hpp"
#include <memory>

namespace kotlinx {
namespace coroutines {

class CombinedContext : public CoroutineContext {
public:
    std::shared_ptr<CoroutineContext> left_;
    std::shared_ptr<Element> right_;

    CombinedContext(std::shared_ptr<CoroutineContext> left, std::shared_ptr<Element> right)
        : left_(left), right_(right) {}

    // Minimal implementation required for compilation
    std::shared_ptr<Element> get(Key* key) const override {
        // Search right first (typical convention)
        if (right_->key() == key) return right_;
        // Then search left (recursive)
        return left_->get(key);
    }

    void for_each(std::function<void(std::shared_ptr<Element>)> callback) const override {
        if (left_) left_->for_each(callback);
        if (right_) right_->for_each(callback);
    }

    std::shared_ptr<CoroutineContext> minus_key(Key* key) const override {
        // If element in right matches, return left
        if (right_->key() == key) return left_;
        
        // Use default minus_key from left
        auto new_left = left_->minus_key(key);
        
        // If left didn't change, return this
        if (new_left == left_) return std::const_pointer_cast<CoroutineContext>(shared_from_this());
        
        // If left became empty (nullptr), return right
        if (new_left == nullptr) return right_;
        
        // Reconstruct CombinedContext
        return std::make_shared<CombinedContext>(new_left, right_);
    }
};

class EmptyCoroutineContext : public CoroutineContext {
public:
    static std::shared_ptr<EmptyCoroutineContext> instance() {
        static auto inst = std::make_shared<EmptyCoroutineContext>();
        return inst;
    }

    std::shared_ptr<Element> get(Key*) const override { return nullptr; }
    void for_each(std::function<void(std::shared_ptr<Element>)>) const override {}
    std::shared_ptr<CoroutineContext> minus_key(Key*) const override { return nullptr; }
};

} // namespace coroutines
} // namespace kotlinx
