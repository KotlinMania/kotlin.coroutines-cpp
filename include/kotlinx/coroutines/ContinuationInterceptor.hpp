#pragma once
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/core_fwd.hpp"

namespace kotlinx {
namespace coroutines {

struct ContinuationInterceptor : public virtual CoroutineContext::Element {
    static constexpr const char* keyStr = "ContinuationInterceptor";
    class Key : public CoroutineContext::KeyTyped<ContinuationInterceptor> {
    public:
        Key() : CoroutineContext::KeyTyped<ContinuationInterceptor>(keyStr) {}
    };
    static Key key_instance;
    static constexpr Key* typeKey = &key_instance;

    ContinuationInterceptor() = default;
    
    virtual CoroutineContext::Key* key() const override { return typeKey; }

    template <typename T>
    std::shared_ptr<Continuation<T>> intercept_continuation(std::shared_ptr<Continuation<T>> continuation) {
        return continuation; // Default implementation? Or abstract? Usually abstract in impls.
    }
    
    virtual void release_intercepted_continuation(std::shared_ptr<ContinuationBase> continuation) = 0;
};

inline ContinuationInterceptor::Key ContinuationInterceptor::key_instance;

} // namespace coroutines
} // namespace kotlinx
