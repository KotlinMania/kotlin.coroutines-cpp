#pragma once
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include "kotlinx/coroutines/Continuation.hpp"

namespace kotlinx {
namespace coroutines {

struct ContinuationInterceptor : public virtual CoroutineContext::Element {
    static constexpr const char* key_str = "ContinuationInterceptor";
    inline static CoroutineContext::KeyTyped<ContinuationInterceptor> key_instance{key_str};
    static constexpr CoroutineContext::Key* type_key = &key_instance;

    ContinuationInterceptor() = default;
    
    virtual CoroutineContext::Key* key() const override { return type_key; }
    
    virtual void release_intercepted_continuation(std::shared_ptr<ContinuationBase> continuation) = 0;
};

} // namespace coroutines
} // namespace kotlinx
