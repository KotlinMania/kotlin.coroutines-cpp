#pragma once
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include <exception>

namespace kotlinx {
namespace coroutines {

struct CoroutineExceptionHandler : public AbstractCoroutineContextElement {
    static constexpr const char* keyStr = "CoroutineExceptionHandler";
    
    // Define unique Key for this element type
    struct Key : public CoroutineContext::KeyTyped<CoroutineExceptionHandler> {
        Key() : CoroutineContext::KeyTyped<CoroutineExceptionHandler>(keyStr) {}
    };
    
    // Singleton instance of the key
    static Key key_instance;
    static constexpr Key* key = &key_instance;
    
    CoroutineExceptionHandler() : AbstractCoroutineContextElement(key) {}
    
    virtual void handle_exception(CoroutineContext& context, std::exception_ptr exception) = 0;
};

// Define key_instance (likely needs to be inline or in cpp, but inline for header-only)
inline CoroutineExceptionHandler::Key CoroutineExceptionHandler::key_instance;

// Global helper
inline void handle_coroutine_exception(CoroutineContext& context, std::exception_ptr exception) {
    // Look up exception handler in context
    // If not found, use global or default
    // Stub implementation:
    // auto handler = context.get(CoroutineExceptionHandler::key);
    // if (handler) ...
}

} // namespace coroutines
} // namespace kotlinx
