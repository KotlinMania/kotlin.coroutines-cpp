#pragma once
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Continuation.hpp"

namespace kotlinx {
namespace coroutines {

struct ContinuationInterceptor : public virtual CoroutineContext::Element {
    static constexpr const char* key_str = "ContinuationInterceptor";
    class Key : public CoroutineContext::KeyTyped<ContinuationInterceptor> {
    public:
        Key() : CoroutineContext::KeyTyped<ContinuationInterceptor>(key_str) {}
    };
    static Key key_instance;
    static constexpr Key* type_key = &key_instance;

    ContinuationInterceptor() = default;
    
    virtual CoroutineContext::Key* key() const override { return type_key; }
    
    // In C++ interface, we can't have virtual template methods.
    // However, ContinuationInterceptor usually acts as a mixin.
    // The actual interception logic is specific to Dispatchers.
    
    virtual void release_intercepted_continuation(std::shared_ptr<ContinuationBase> continuation) = 0;
};

inline ContinuationInterceptor::Key ContinuationInterceptor::key_instance;

} // namespace coroutines
} // namespace kotlinx
