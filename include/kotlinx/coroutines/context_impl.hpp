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
        if (right_->key() == key) return right_;
        return left_->get(key);
    }
};

} // namespace coroutines
} // namespace kotlinx
