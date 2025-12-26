#pragma once
#include "kotlinx/coroutines/CoroutineContext.hpp"
#include <exception>
#include <iostream>

namespace kotlinx {
namespace coroutines {

/**
 * An optional element in the coroutine context to handle uncaught exceptions.
 * Only one handler can be present in the context.
 */
class CoroutineExceptionHandler : public virtual CoroutineContext::Element {
public:
    // Key for CoroutineExceptionHandler in CoroutineContext
    // Key for CoroutineExceptionHandler in CoroutineContext
    struct KeyType : CoroutineContext::Key {};
    static inline KeyType key_instance;
    static constexpr CoroutineContext::Key* type_key = &key_instance;

    CoroutineContext::Key* key() const override { return type_key; }

    virtual ~CoroutineExceptionHandler() = default;

    /**
     * Handles uncaught [exception] in the given [context].
     * It is invoked if coroutine does not handle exception itself.
     */
    virtual void handle_exception(CoroutineContext& context, std::exception_ptr exception) = 0;
};

// Global helper for handling uncaught exceptions in coroutines
// Kotlin lines 18-32: handleCoroutineException
inline void handle_coroutine_exception(CoroutineContext& context, std::exception_ptr exception) {
    if (!exception) return;

    try {
        auto element = context.get(CoroutineExceptionHandler::type_key);
        if (auto handler = std::dynamic_pointer_cast<CoroutineExceptionHandler>(element)) {
            handler->handle_exception(context, exception);
            return;
        }
    } catch (...) {
        // For now, just fall through to global handler
    }

    // In C++ we just print to stderr (like Native platform behavior)
    try {
        std::rethrow_exception(exception);
    } catch (const std::exception& e) {
        std::cerr << "Uncaught coroutine exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Uncaught coroutine exception: unknown" << std::endl;
    }
}

} // namespace coroutines
} // namespace kotlinx
